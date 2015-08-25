

#include "CommandLine.h"
#include "UnitTesting.h"

using namespace Wild::CommandLine;
using namespace std;

int main(int argc, char* argv[])
{

    Args args({
        Arg("version", "v", "Display version information"),
        Arg("another-flag", "a", "Another flag for some reason"),
        Arg("colour", "c", "Colour", {"red", "green", "blue"}),
        Arg("number", "n", "Number of things", {}),
        Arg("string", "s", "Some text", {}),
        Arg("float", "f", "A float", {}),
        Arg("bool", "b", "A boolean", {"true", "false", "0", "1"})
    });

    AssertPrints(
        AssertTrue(!args.Parse({""})),
        "Parsing command line failed, details: argument needs to be at least two characters including -\n");

    AssertPrints(
        AssertTrue(!args.Parse({ "foo" })),
        "Parsing command line failed, details: argument needs to start with -\n");

    AssertPrints(
        AssertTrue(!args.Parse({ "-x" })),
        "Parsing command line failed, details: couldn't find -x in specified list of arguments\n");

    AssertPrints(
        AssertTrue(!args.Parse({ "-c" })),
        "Parsing command line failed, details: argument -c given without a value\n");

    AssertPrints(
        AssertTrue(!args.Parse({ "-c", "mauve"})),
        "Parsing command line failed, details: value mauve for argument -c isn't one of the options\n");

    AssertTrue(args.Parse({}));
    AssertTrue(args.Parse({ "-v" }));
    AssertTrue(args.Parse({ "-v", "-c", "red", "--number", "5", "--another-flag", "-f", "1.456", "--bool", "true"}));
    AssertTrue(args.Parse({ "-v" }));
    // Check args clears values on Parse
    AssertFalse(args.IsSet("colour"));

    AssertTrue(args.Parse({ "-v", "-c", "red", "--number", "5", "--another-flag", "-f", "1.456", "--bool", "true" }));

    AssertTrue(args.IsSet("version"));
    AssertTrue(args.IsSet("another-flag"));
    AssertTrue(args.IsSet("colour"));
    AssertTrue(args.IsSet("number"));
    AssertTrue(!args.IsSet("string"));
    AssertTrue(args.IsSet("float"));
    AssertTrue(args.IsSet("bool"));

    AssertEquals("red", args.Get("colour"));
    AssertEquals("5", args.Get("number"));
    AssertEquals(5, args.GetAsInt("number"));
    AssertEquals(1.456f, args.GetAsFloat("float"));
    AssertTrue(args.GetAsBool("bool"));

    AssertTrue(args.Parse({"--bool", "false" }));
    AssertFalse(args.GetAsBool("bool"));

    const int argcTest = 9;
    char *argvTest[argcTest];
    argvTest[0] = "programName";
    argvTest[1] = "-v";
    argvTest[2] = "-c";
    argvTest[3] = "red";
    argvTest[4] = "--number";
    argvTest[5] = "5";
    argvTest[6] = "-a";
    argvTest[7] = "-s";
    argvTest[8] = "foo bar";

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

    EndTest
}

