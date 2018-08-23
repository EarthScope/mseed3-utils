#include <stdlib.h>
#include <libmseed.h>
#include <unpack.h>
#include <parson.h>
#include <stdbool.h>
#include <xseed-common/cmd_opt.h>
#include <xseed-common/files.h>
#include <xseed-common/xseed_string.h>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <xseed-common/vcs_getopt.h>
#else

#include <unistd.h>
#include <getopt.h>

#endif


//CMD line option structure
static const struct xseed_option_s args[] = {
        {'h', "help",    "   Display usage information", NULL, NO_OPTARG},
        {'v', "verbose", "Verbosity level",              NULL, OPTIONAL_OPTARG},
        {'d', "data",    "   Print data payload",        NULL, OPTIONAL_OPTARG},
        {0,   0,         0, 0,                                 0}};

int print_xseed_2_json(char *file_name, bool print_data, uint8_t verbose);

/*! @brief Program to Print a xSEED file in JSON format
 *
 */
int main(int argc, char **argv)
{


    //vars to store command line options/args
    char *short_opt_string = NULL;
    struct option *long_opt_array = NULL;
    int opt;
    int longindex;
    unsigned char display_usage = 0;
    uint8_t verbose = 0;
    char *file_name = NULL;
    bool print_data = false;

    //parse command line args
    xseed_get_short_getopt_string(&short_opt_string, args);
    xseed_get_long_getopt_array(&long_opt_array, args);

    while (-1 != (opt = getopt_long(argc, argv, short_opt_string, long_opt_array, &longindex)))
    {
        switch (opt)
        {
            //int file_name_size;
            case 'd':
                print_data = true;
                break;
            case 'v':
                if (0 == optarg)
                {
                    verbose++;
                } else
                {
                    verbose = (uint8_t) strlen(optarg) + 1;
                }
                break;
            case 'h':
                display_usage = 1;
                break;
            default:
                //display_usage++;
                break;
        }
        if (display_usage > 0)
        {
            break;
        }

    }


    if (display_usage > 0 || (argc == 1))
    {
        display_help(argv[0], " [options] infile(s)", "Program to Print a xSEED file in JSON format", args);
        return display_usage < 2 ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    free(long_opt_array);
    free(short_opt_string);

    while (argc > optind)
    {
        file_name = argv[optind++];


        if (!xseed_file_exists(file_name))
        {
            printf("Error reading file: %s, File Not Found! \n", file_name);
            continue;
        }


        print_xseed_2_json(file_name, print_data, verbose);
    }

    return 0;
}


int print_xseed_2_json(char *file_name, bool print_data, uint8_t verbose)
{

    if (!xseed_file_exists(file_name))
    {
        printf("Error: input file %s not found!", file_name);
        return EXIT_FAILURE;
    }

    //libmseed vars
    MS3Record *msr = NULL;
    MS3Record *msrOut = NULL;

    //parson vars
    JSON_Status ierr;
    JSON_Value *val = NULL;
    JSON_Object *jsonObj = NULL;
    JSON_Value *extraVal = NULL;
    JSON_Array *payload_arr = NULL;

    //helper vars
    char times[1024];
    char hex[1024];
    uint32_t flags = 0;



    //Set flag to unpack data and check CRC
    flags |= MSF_UNPACKDATA;
    flags |= MSF_VALIDATECRC;

    //loop over all records in xseed file,
    //Add 1 to verbose level as verbose = 1 prints nothing extra
    while ((ms3_readmsr(&msr, file_name, 0, NULL, 0, verbose + 1) == MS_NOERROR))
    {

        if (!msr)
            return EXIT_FAILURE;

        val = json_value_init_object();
        jsonObj = json_value_get_object(val);

        /* Generate a start time string */
        ms_nstime2seedtimestr(msr->starttime, times, 1);


        ierr = json_object_set_string(jsonObj, "tsid", msr->tsid);

        if (ierr == JSONFailure)
        {
            printf("Something went wrong parsing to JSON : tsID");
            return EXIT_FAILURE;
        }

        ierr = json_object_set_number(jsonObj, "pubversion", msr->pubversion);

        if (ierr == JSONFailure)
        {
            printf("Something went wrong parsing to JSON : pubversion");
            return EXIT_FAILURE;
        }

        ierr = json_object_set_number(jsonObj, "reclen", msr->reclen);

        if (ierr == JSONFailure)
        {
            printf("Something went wrong parsing to JSON : reclen");
            return EXIT_FAILURE;
        }

        ierr = json_object_set_number(jsonObj, "reclen", msr->formatversion);

        if (ierr == JSONFailure)
        {
            printf("Something went wrong parsing to JSON : formatversion");
            return EXIT_FAILURE;
        }

        ierr = json_object_set_number(jsonObj, "reclen", msr->formatversion);

        if (ierr == JSONFailure)
        {
            printf("Something went wrong parsing to JSON : formatversion");
            return EXIT_FAILURE;
        }

        ierr = json_object_set_string(jsonObj, "starttime", times);

        if (ierr == JSONFailure)
        {
            printf("Something went wrong parsing to JSON : starttime");
            return EXIT_FAILURE;
        }

        ierr = json_object_set_number(jsonObj, "samplecnt", (double) msr->samplecnt);

        if (ierr == JSONFailure)
        {
            printf("Something went wrong parsing to JSON : formatversion");
            return EXIT_FAILURE;
        }

        ierr = json_object_set_number(jsonObj, "samprate", msr_sampratehz(msr));

        if (ierr == JSONFailure)
        {
            printf("Something went wrong parsing to JSON : formatversion");
            return EXIT_FAILURE;
        }


        ierr = json_object_set_number(jsonObj, "flags", msr->flags);
        if (ierr == JSONFailure)
        {
            printf("Something went wrong parsing to JSON : flags");
            return EXIT_FAILURE;
        }

        sprintf(hex, "0x%0X", (unsigned int) msr->crc);
        ierr = json_object_set_string(jsonObj, "crc", hex);


        if (ierr == JSONFailure)
        {
            printf("Something went wrong parsing to JSON : crc");
            return EXIT_FAILURE;
        }

        ierr = json_object_set_number(jsonObj, "extralength", msr->extralength);

        if (ierr == JSONFailure)
        {
            printf("Something went wrong parsing to JSON : extralength");
            return EXIT_FAILURE;
        }

        ierr = json_object_set_number(jsonObj, "datalength", msr->datalength);

        if (ierr == JSONFailure)
        {
            printf("Something went wrong parsing to JSON : datalength");
            return EXIT_FAILURE;
        }

        ierr = json_object_set_string(jsonObj, "encoding", (char *) ms_encodingstr(msr->encoding));
        if (ierr == JSONFailure)
        {
            printf("Something went wrong parsing to JSON : encoding");
            return EXIT_FAILURE;
        }

        ierr = json_object_set_number(jsonObj, "encodingval", msr->encoding);

        if (ierr == JSONFailure)
        {
            printf("Something went wrong parsing to JSON : encodingval");
            return EXIT_FAILURE;
        }

        if (msr->extralength > 0 && msr->extra)
        {
            extraVal = json_parse_string(msr->extra);
            ierr = json_object_set_value(jsonObj, "extra", extraVal);

            if (ierr == JSONFailure)
            {
                printf("Something went wrong parsing to JSON : extra");
                return EXIT_FAILURE;
            }


        }


        if (print_data)
        {
            if (msr->formatversion == 3)
            {
                if (verbose > 0)
                    printf("Unpacking data for verification\n");
                ierr = msr3_unpack_mseed3(msr->record, msr->reclen, &msrOut, flags, verbose);
                if (ierr != MS_NOERROR)
                {
                    //TODO more verbose error output
                    printf("Error: Format 3 payload parsing failed. ms_unpack_mseed3 returned: %d\n", ierr);
                    return EXIT_FAILURE;
                } else
                {

                    if (verbose > 0)
                        printf("Data unpacked successfully\n");
                }
            } else
            {
                printf("Error: Format version not version 3, read as version: %d", msr->formatversion);
                printf("Attepting to parse as format 2");
                ierr = msr3_unpack_mseed2(msr->record, msr->reclen, &msrOut, flags, verbose);
                if (ierr > 0)
                {
                    printf("Error: Format 2 payload parsing failed. ms_unpack_mseed2 returned: %d\n", ierr);
                    return EXIT_FAILURE;
                }

            }


            if (msrOut->numsamples > 0 && msrOut != NULL)
            {
                int samplesize;
                void *sptr;

                if ((samplesize = ms_samplesize(msrOut->sampletype)) == 0)
                {
                    printf("Unrecognized sample type: '%c'\n", msrOut->sampletype);
                    return EXIT_FAILURE;
                }
                if (msrOut->sampletype == 'a')
                {
                    char *ascii = (char *) msrOut->datasamples;
                    //int length = msrOut->numsamples;

                    ierr = json_object_set_string(jsonObj, "payload", ascii);

                    if (ierr == JSONFailure)
                    {
                        printf("Something went wrong parsing to JSON : payload (ascii)");
                        return EXIT_FAILURE;
                    }


                } else
                {

                    json_object_set_value(jsonObj, "Payload", json_value_init_array());
                    payload_arr = json_object_get_array(jsonObj, "Payload");

                    for (int i = 0; i < msrOut->numsamples; i++)
                    {
                        sptr = (char *) msrOut->datasamples + (i * samplesize);

                        if (msrOut->sampletype == 'i')
                        {
                            json_array_append_number(payload_arr, *(int32_t *) sptr);
                        } else if (msrOut->sampletype == 'f')
                        {
                            json_array_append_number(payload_arr, *(float *) sptr);
                        } else if (msrOut->sampletype == 'd')
                        {
                            json_array_append_number(payload_arr, *(double *) sptr);
                        }

                    }

                    if (ierr == JSONFailure)
                    {
                        printf("Something went wrong parsing to JSON : payload (numeric)");
                        return EXIT_FAILURE;
                    }
                }
            }


            if (msrOut != NULL)
                msr3_free(&msrOut);


        }


        char *full_string = json_serialize_to_string_pretty(val);
        printf("%s", full_string);
        json_free_serialized_string(full_string);
        if (print_data)
        {
            json_array_clear(payload_arr);
        }
        json_value_free(val);


    }


    //if(msr != NULL)
    //    msr3_free(&msr);
    //Required to cleanup globals
    ms3_readmsr(&msr, NULL, 0, 0, 0, 0);


    return EXIT_SUCCESS;
}
