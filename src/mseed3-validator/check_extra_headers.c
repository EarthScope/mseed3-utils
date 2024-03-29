#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wjelement.h>
#include <mseed3-common/files.h>

#include <libmseed.h>

#include "validator.h"
#include "warnings.h"

#define SCHEMA_BUFFER_SIZE 1024u

static void schema_error_func (void *client, const char *format, ...);
static WJElement load_schema_func (const char *name, void *client, const char *file, const int line);
static void schema_free (WJElement schema, void *client);



/*! @brief Check extra header using WJElement against a user provided schema
 *
 *  @param[in] options -W cmd line warn options (currently not implemented)
 *  @param[in] schema path to provided json schema file
 *  @param[in] file_len overall binary length
 *  @param[in] extra_header_len Extra header length in bytes
 *  @param[in] recordNum number of current record being processed
 *  @param[in] verbose verbosity level
 *
 */
/*TODO future improvement pass back stuff from extra_headers to validate payloads*/
bool
check_extra_headers (struct extra_options_s *options, char *schema, FILE *input,
                     uint16_t extra_header_len, uint32_t recordNum, uint8_t verbose)
{
  WJElement document_element;
  char *extraHeaderStr;
  bool valid_extra_header = true;
  is_valid_gbl            = valid_extra_header;

  char schema_buffer[SCHEMA_BUFFER_SIZE];
  char *buffer = (char *)calloc (extra_header_len + 1, sizeof (char));

  if (buffer == NULL)
  {
    printf ("Fatal Error! Record: %d --- could not calloc buffer\n", recordNum);
    free (buffer);
    return false;
  }

  if (extra_header_len > fread (buffer, sizeof (char), extra_header_len, input))
  {
    printf ("Fatal Error! Record: %d --- EOF reached reading extra headers into buffer, please double check input record\n",
            recordNum);
    free (buffer);
    valid_extra_header = false;

    return valid_extra_header;

  }

  if (extra_header_len == 0)
  {
    if (verbose > 1)
      printf ("Record: %d --- This record does not contain an extra header\n", recordNum);

    free (buffer);
    return true;
  }
  else
  {
    /* Parse extra headers to validate integrity */
    document_element = WJEParse (buffer);

    if (document_element != NULL)
    {
      if (verbose > 3)
      {
        //TODO make optional
        extraHeaderStr = WJEToString (document_element, true);
        printf ("Record: %d --- Extra header output:\n%s\n\n", recordNum, extraHeaderStr);
        free (extraHeaderStr);
      }
    }
    else
    {
      printf ("Error! Record: %d ---  Failed to parse Extra Header from Record!\n", recordNum);
      valid_extra_header = false;
      free (buffer);
      return valid_extra_header;
    }
  }

  /* If schema file is provided, attempt to validate */
  if (schema && mseed3_file_exists (schema))
  {
    /* Get user provided schema and parge into WJElement */
    FILE *schema_file        = fopen (schema, "r");
    WJReader schema_reader   = WJROpenFILEDocument (schema_file, schema_buffer, SCHEMA_BUFFER_SIZE);
    WJElement schema_element = WJEOpenDocument (schema_reader, NULL, NULL, NULL);

    char *path    = mseed3_get_dirname (schema);
    char *pattern = mseed3_cat_strings (path, "/%s");

    WJEErrCB errFunc            = &schema_error_func;
    WJESchemaLoadCB load_schema = &load_schema_func;
    WJESchemaFreeCB free_schema = &schema_free;

    /* Validate extra headers against schema */
    XplBool isValid = WJESchemaValidate (schema_element, document_element, errFunc, load_schema, free_schema, pattern);

    if ((!isValid) || (!is_valid_gbl))
    {
      printf ("Error! Record: %d ---  Schema validation failed!\n", recordNum);
      valid_extra_header = false;

      if (options->treat_as_errors)
      {
        free (buffer);
        WJECloseDocument (schema_element);
        WJRCloseDocument (schema_reader);
        fclose (schema_file);
        free (pattern);
        free (path);
        return valid_extra_header;
      }
    }
    else
    {

      if (verbose > 2)
        printf ("Record: %d --- JSON Schema validation success!\n", recordNum);
    }

    WJECloseDocument (schema_element);
    WJRCloseDocument (schema_reader);
    fclose (schema_file);
    free (pattern);
    free (path);

  } // if no schema file provided
  else
  {
    if (verbose > 1 && extra_header_len > 0)
      printf ("Record: %d --- No json schema file provided, skipping Extra Header check\n", recordNum);
  }
  /*TODO other checks */

  if (buffer)
  {
    free (buffer);
  }

  if (extra_header_len != 0)
    WJECloseDocument (document_element);

  return valid_extra_header;
}

/* Helper function used with WJElement for error reporting */
static void
schema_error_func (void *client, const char *format, ...)
{
  //TODO format to other log messages
  va_list ap;
  va_start (ap, format);
  fprintf (stderr, "Error in Schema Validation- ");
  vfprintf (stderr, format, ap);
  va_end (ap);
  fprintf (stderr, "\n");
  is_valid_gbl = false;
}

/* Callback for loading additional schemas */
static WJElement
load_schema_func (const char *name, void *client,
                  const char *file, const int line)
{
  char *format;
  char *path;
  FILE *schemafile;
  WJReader readschema;
  WJElement schema;
  schema = NULL;
  if (client && name)
  {

    format = (char *)client;
    path   = malloc (strlen (format) + strlen (name));
    sprintf (path, format, name);

    if ((schemafile = fopen (path, "r")))
    {
      if ((readschema = WJROpenFILEDocument (schemafile, NULL, 0)))
      {
        schema = WJEOpenDocument (readschema, NULL, NULL, NULL);
      }
      else
      {
        fprintf (stderr, "json document failed to open: '%s'\n", path);
      }
      fclose (schemafile);
    }
    else
    {
      fprintf (stderr, "json file not found: '%s'\n", path);
    }
    free (path);
  }
  //WJEDump(schema);
  return schema;
}

/* Callback to free additional schemas */
static void
schema_free (WJElement schema, void *client)
{
  WJECloseDocument (schema);
  return;
}
