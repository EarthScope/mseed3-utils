# miniSEED3-Utils
**This repository contains command line utilities to verify miniSEED 3 files**
- miniSEED 3 Validator
  - Validates a single or collection of miniSEED 3 files
- mseed2text
  - Prints the contents of a selected miniSEED 3 file in text format to the terminal
- mseed2json
  - Prints the contents of a selected miniSEED 3 file in JSON format to the terminal

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
- ```git clone https://github.com/iris-edu/mseed3-utils.git```
- ```cd mseed3-utils```
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
  - Run ```MSBuild mseed3-utils.sln```


## miniSEED 3 Validator
Checks miniSEED 3 file for:
1. Valid fixed header
2. Valid payload
3. Valid extra header via user provided JSON schema (optional)

All information on the miniSEED file is printed to the terminal

**Usage:**
```
Usage: ./mseed3-validator [options] infile(s)

         ## Options ##
	 -h help    Display usage information
	 -j json    Json schema
	 -v verbose Verbosity level
	 -d data    Print data payload
	 -W         Option flag  *e.g* -W error,skip-payload
         -V version Print program version
```


## mseed2text
Prints the contents of a selected miniSEED file in text format to the terminal

**Usage:**

```
Program to print an miniSEED file in human readable format:

Usage: ./mseed2text [options] infile(s)

         ## Options ##
     -h help    Display usage information
     -v verbose Verbosity level
     -d data    Print data payload
     -V version Print program version
```

## mseed2json
Prints the contents of a selected miniSEED file in JSON format to the terminal

**Usage:**

```
Program to print an miniSEED file in JSON format:

Usage: ./mseed2json [options] infile(s)

         ## Options ##
     -h help    Display usage information
     -v verbose Verbosity level
     -d data    Print data payload
     -V version Print program version
```
