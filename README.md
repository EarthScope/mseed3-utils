# xSEED-Utils
**This repository contains command line utilities to verify xSEED standard files**
- XSEED Validator
  - Validates a single or collection of xSEED files
- XSEED2Text
  - Prints the contents of a selected xSEED file in text format to the terminal
- XSEED2JSON
   - Prints the contents of a selected xSEED file in JSON format to the terminal

### Dependencies
1. cmake >= 2.8.0
2. libmseed >= 3.0
3. WJElement > 1.3  

NOTE: libmseed and WJElement are automatically installed locally via make 

### Supported Platforms
- Linux
- MacOS
- Windows


## Clone - Configure - Build - Install
**Clone & Configure Project**
- ```git clone https://github.com/iris-edu/xseed-utils.git```
- ```cd xseed-utils```
- ```mkdir build/```
- ```cd build/```
- Run ```cmake ..```
  - To specify install prefix use:
            ```cmake -DCMAKE_INSTALL_PREFIX:PATH={user specified path} ..```

**Build/Install**
- Linux/MacOS
  - Run ```make``` --NOTE: Internet connection required to pull and build supporting libraries (see Dependencies)
  - (optional) Run ```make install``` to install in system or location specified by ```-DCMAKE_INSTALL_PREFIX:PATH```

- Windows
  - Run ```MSBuild xseed-utils.sln```


## xSEED Validator
Checks xSEED file for:
1. Valid fixed header
2. Valid payload
3. Valid extra header via user provided JSON schema (optional)

All information on the xSEED file is printed to the terminal

**Usage:**
```
Usage: ./xseed-validator [options] infile(s)

         ## Options ##
	 -h help    Display usage information
	 -j json    Json schema
	 -v verbose Verbosity level
	 -d data    Print data payload
	 -W         Option flag  *e.g* -W error,skip-payload
         -V version Print program version
```


## xSEED2text
Prints the contents of a selected xSEED file in text format to the terminal

**Usage:**

```
Program to print an xSEED file in human readable format:

Usage: ./xseed2text [options] infile(s)

         ## Options ##
     -h help    Display usage information
     -v verbose Verbosity level
     -d data    Print data payload
     -V version Print program version
```

## xSEED2JSON
Prints the contents of a selected xSEED file in JSON format to the terminal

**Usage:**

```
Program to print an xSEED file in JSON format:

Usage: ./xseed2json [options] infile(s)

         ## Options ##
     -h help    Display usage information
     -v verbose Verbosity level
     -d data    Print data payload
     -V version Print program version
```


