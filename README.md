# C++ Command Line Handling

This is a simple, header only, no dependency, cross platform command line handling library.

More on this library and others like it can be found at [http://wildcoast.nz/open-source](http://wildcoast.nz/open-source).

[Download the latest version directly (right click, download)](https://raw.githubusercontent.com/WildCoastSolutions/CommandLine/master/CommandLine.h).

Travis CI build & test status: [![Build Status](https://api.travis-ci.org/WildCoastSolutions/CommandLine.svg?branch=master)](https://travis-ci.org/WildCoastSolutions/CommandLine)




# Sample Code

```C++
#include "Wild/CommandLine.h"

using namespace Wild::CommandLine;
using namespace std;

// This is a simple video transcoding app example
// Sample command line: transcode input.mp4 -b 1024 -r 1080p output.mp4

int main(int argc, char* argv[])
{
    // Setup args that this application supports, in this case it is a simple video transcoding app
    // It takes an input file, a new bitrate and resolution, and writes to the given output file
    Args args({
        // The order of Flags and Options given on the command line doesn't matter
        // The order of PositionalArgs only matters relative to other PositionalArgs

        // A PositionalArg has no letter or name associated with it, it is assigned 
        // by virtue of where it appears on the command line.
        PositionalArg("input", "Input video file path"),
        PositionalArg("output", "Output video file path"),

        // Flag - name, letter, description
        Flag("version", "v", "Display version information"),
        Flag("debug", "d", "Print debug information"),
        
        // Option - name, letter, description, whether it is required or not
        // An option is generally not required, but this is possible when you want to be name the required option rather
        // than use a positional argument with no name
        Option("bitrate", "b", "Bitrate of output video in Kbps", Is::Required),

        // name, letter, description, possible values, default value (setting a default value means it's optional on the command line)
        Option("resolution", "r", "Resolution of output video", { "720p", "1080p" }, "720p"),
    });

    // E.g. transcode.exe input.mp4 -b 1024 -r 1080p output.mp4
    
    // Args fail to parse with an error message when the syntax is bad
    if (!args.Parse(argc, argv))
    {
        cout << args.Usage(argv[0]);
        return;
    }

    if (args.IsSet("version")) // Use the full name to access
    {
        cout << string(argv[0]) << " 1.0" << endl;
        return;
    }

    // We know that bitrate is set since Parse would have failed otherwise as it is required
    // We know that resolution is set either by the command line or by the default value
    int bitrate = args.GetAsInt("bitrate");  // Use the full name to access
    string resolution = args.Get("resolution");

    // We know that the paths are set since Parse would have failed otherwise as PositionalArgs are required
    string inputPath = args.Get("input");
    string outputPath = args.Get("output");

    // Defined elsewhere
    Transcode(inputPath, outputPath, bitrate, resolution);

```


# Installing

## Download

All you need to use this library is to put `CommandLine.h` in your project and include it in your code.

[Download the latest version here (right click, download)](https://raw.githubusercontent.com/WildCoastSolutions/CommandLine/master/CommandLine.h).

## Namespace

All code is in the `Wild::CommandLine` namespace.

# Using

First you need to specify all the flags and arguments your application is going to support. Each has a name, letter, description, and optionally a list of allowed values, a default value and whether it is required or not.

Once this is established, Parse is called with the given command line (argv). Then you can get the state of args and their values and take action.

**Flag vs Option**

For the purposes of this library, flags are command line options that take no value, and args are options that do take a value.

### Setting up

The Args object is the container for all specified arguments and handles parsing and reporting on the argument values. Construct the args object by passing in as many Arg or Flag objects as necessary. The constructors will throw `invalid_argument` for bad arguments like empty strings or the default value not being in the list of possible values.

```C++
// These are the supported constructors
Args args({
    Flag(name, letter, description),
    Option(name, letter, description, [default_value]),
    Option(name, letter, description, [possible_values], [default_value]),
    Option(name, letter, description, [possible_values], [Is::Required]),
    PositionalArg(name, letter, description, [possible_values], [default_value])
    ...
    });
```

Note that setting a default value and marking an Option as required are mutually exclusive - if it is required then the user must supply a value.

For example: We want to allow the user to supply a colour on the command line, one of red, green and blue, and if they don't supply one we'll just go with red. The Option would look like:

```C++
    Args args({
        Option("colour", "c", "Choose a colour", {"red", "green", "blue"}, "red")
    });
```

## Parsing

Call `args.Parse` with `argc` and `argv` to parse the command line. Parse will print an error message and return false if parsing fails. It can fail for the following reasons:

* an argument/flag was given that wasn't specified in the Args constructor
* a required argument/flag wasn't given
* a value for an argument was given that wasn't in the list of allowed values
* an option was given without a value

It's typical behaviour to print out the usage message when parsing fails, as in the sample code.

## Accessing

Once Parse succeeds, the args object can be queried to find the presence and value of the flags and arguments. This is done using the full name of the argument. E.g.

```C++
    bool showVersion = args.IsSet("version");
    string colour = args.Get("colour");
```

Supported accessors are:

```C++
    Args::IsSet(name);      // true if flag/arg <name> was given on the command line
    // note this is also always true if a default value was specified
    Args::Get(name);        // returns value as a string
    Args::GetAsBool(name);  // returns value as a bool
    Args::GetAsInt(name);   // returns value as an integer
    Args::GetAsFloat(name); // returns value as a float
```


## Compiling And Running Tests

Note that the [Unit Testing](https://github.com/WildCoastSolutions/UnitTesting) library is required alongside this library to compile the tests. E.g.

```
  Wild/
     CommandLine/
     UnitTesting/
```

### Windows

`CommandLine.sln` contains the library and test projects, compilation also runs the tests.

It was built with VS2015 but should work with other versions though it does require C++11 features.

### Linux

The Linux build uses cmake, to test the lib it you can clone it, enter the directory and use

```
cmake .
make
Test\CommandLineTest
```


## Scaling Up

If you need to go beyond what this library offers, boost program_options may be of interest.