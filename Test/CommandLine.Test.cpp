//
// Author       Wild Coast Solutions
//              David Hamilton
//
// This file is distributed under the MIT License. See the accompanying file
// LICENSE.txt or http://www.opensource.org/licenses/mit-license.php for terms
// and conditions.
//
// This file contains tests for the command line handling library in
// CommandLine.h
// 
// Testing is done by the unit testing library from
// https://github.com/WildCoastSolutions/UnitTesting

#include "CommandLine.h"
#include "UnitTesting.h"

using namespace Wild::CommandLine;
using namespace std;

void ReadmeSampleCode()
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

    cout << args.Usage("transcode.exe") << endl;

    // transcode.exe input.mp4 -b 1024 -r 1080p output.mp4
    const int argc = 7;
    char *argv[argc] = { "transcode.exe", "input.mp4", "-b", "1024", "-r", "1080p", "output.mp4" };

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

    //Transcode(inputPath, outputPath, bitrate, resolution);

}

void TestConstruction()
{
    AssertThrows(Flag("", "", ""), invalid_argument);
    AssertThrows(Flag("", "v", ""), invalid_argument);
    AssertThrows(Flag("v", "v", ""), invalid_argument);
    AssertThrows(Flag("version", "vr", ""), invalid_argument);

    AssertThrows(Option("", "", ""), invalid_argument);
    AssertThrows(Option("", "v", ""), invalid_argument);
    AssertThrows(Option("v", "v", ""), invalid_argument);
    AssertThrows(Option("version", "vr", ""), invalid_argument);

    AssertThrows(PositionalArg("", ""), invalid_argument);

    // Can't set the default to something not in the list of possibles
    AssertThrows(Option("colour", "c", "Colour", { "red", "blue" }, "green"), invalid_argument);

}


int main(int argc, char* argv[])
{
    //ReadmeSampleCode();
    TestConstruction();

    Args args(
    {
        PositionalArg("positional-arg-1", "Positional arg 1"),
        PositionalArg("positional-arg-2", "Positional arg 2"),
        Flag("version", "v", "Display version information"),
        Flag("another-flag", "a", "Another flag for some reason"),
        Option("colour", "c", "Colour", { "red", "green", "blue" }, Is::Required),
        Option("number", "n", "Number of things", "5"),
        Option("string", "s", "Some text"),
        Option("float", "f", "A float"),
        Option("bool", "b", "A boolean", {"true", "false"}, "false"),
    });


    AssertPrints(
        AssertTrue(!args.Parse({""})),
        "Parsing command line failed, details: argument needs to be at least one character\n");

    AssertPrints(
        AssertTrue(!args.Parse({ "foo" })),
        "Parsing command line failed, details: colour is required but was not set\n");

    AssertPrints(
        AssertTrue(!args.Parse({ "-x" })),
        "Parsing command line failed, details: colour is required but was not set\n");

    AssertPrints(
        AssertTrue(!args.Parse({ "-c" })),
        "Parsing command line failed, details: argument -c given without a value\n");

    AssertPrints(
        AssertTrue(!args.Parse({ "-c", "mauve"})),
        "Parsing command line failed, details: value mauve for argument -c isn't one of the options\n");

    AssertPrints(
        AssertTrue(!args.Parse({ "-v" })),
        "Parsing command line failed, details: colour is required but was not set\n");

    AssertPrints(
        AssertTrue(!args.Parse({})),
        "Parsing command line failed, details: colour is required but was not set\n");

    AssertPrints(
        AssertTrue(!args.Parse({"1", "2"})),
        "Parsing command line failed, details: colour is required but was not set\n");

    AssertPrints(
        AssertTrue(!args.Parse({ "-v", "-c", "red", "1", "2", "-x" })),
        "Parsing command line failed, details: couldn't find -x in specified list of arguments\n");

    AssertTrue(args.Parse({ "-v", "-c", "red", "1", "2" }));
    AssertTrue(args.Parse({ "-v", "-c", "red", "--number", "5", "--another-flag", "-f", "1.456", "--bool", "true", "1", "2" }));
    AssertTrue(args.Parse({ "-v", "-c", "red", "1", "2" }));
    // Check args clears values on Parse
    AssertFalse(args.IsSet("string"));

    // Check default values are present
    AssertTrue(args.IsSet("bool"));
    AssertFalse(args.GetAsBool("bool"));

    AssertTrue(args.IsSet("number"));
    AssertEquals(5, args.GetAsInt("number"));

    AssertTrue(args.Parse({ "-v", "-c", "red", "--number", "5", "--another-flag", "-f", "1.456", "--bool", "true", "1", "2" }));

    AssertTrue(args.IsSet("version"));
    AssertTrue(args.IsSet("another-flag"));
    AssertTrue(args.IsSet("colour"));
    AssertTrue(args.IsSet("number"));
    AssertTrue(!args.IsSet("string"));
    AssertTrue(args.IsSet("float"));
    AssertTrue(args.IsSet("bool"));
    AssertTrue(args.IsSet("positional-arg-1"));
    AssertTrue(args.IsSet("positional-arg-2"));

    AssertEquals("red", args.Get("colour"));
    AssertEquals("5", args.Get("number"));
    AssertEquals(5, args.GetAsInt("number"));
    AssertEquals(1.456f, args.GetAsFloat("float"));
    AssertEquals(1, args.GetAsInt("positional-arg-1"));
    AssertEquals(2, args.GetAsInt("positional-arg-2"))
    AssertTrue(args.GetAsBool("bool"));

    AssertTrue(args.Parse({ "1", "2", "--bool", "false", "-v", "-c", "red" }));
    AssertFalse(args.GetAsBool("bool"));

    const int argcTest = 11;
    char *argvTest[argcTest] = { "programName", "1", "-v", "-c", "red", "--number", "5", "-a", "-s", "foo bar", "2" };

    AssertTrue(args.Parse(argcTest, argvTest));

    AssertTrue(args.IsSet("version"));
    AssertTrue(args.IsSet("another-flag"));
    AssertTrue(args.IsSet("colour"));
    AssertTrue(args.IsSet("number"));
    AssertTrue(args.IsSet("string"));
    AssertFalse(args.IsSet("float"));

    AssertEquals("red", args.Get("colour"));
    AssertEquals("5", args.Get("number"));
    AssertEquals(5, args.GetAsInt("number"));
    AssertEquals("foo bar", args.Get("string"));
    AssertEquals("1", args.Get("positional-arg-1"));
    AssertEquals("2", args.Get("positional-arg-2"));

    EndTest
}

