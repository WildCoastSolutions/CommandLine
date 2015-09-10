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
    // Setup args that this application supports, in this case the app adds or subtracts using the two numbers provided
    Args args({
        // name, letter, description
        Flag("version", "v", "Display version information"),
        Flag("please", "p", "The magic word", Flag::Is::Required),
        
        // name, letter, description, whether required
        Arg("number-a", "a", "First number", Arg::Is::Required),

        // name, letter, description, default value
        Arg("number-b", "b", "Second number", "4"),

        // name, letter, description, possible values, default value (setting a default value means it's optional on the command line)
        Arg("operation", "o", "Operation to use", { "add", "subtract" }, "add")
    });

    cout << args.Usage("maths.exe") << endl;

    // maths.exe -a 3 -b 4 --operation add
    const int argc = 8;
    char *argv[argc] = { "maths.exe", "-p", "-a", "3", "-b", "5", "--operation", "add" };

    // Args fail to parse with an error message when the syntax is bad
    if (!args.Parse(argc, argv))
    {
        string usage = "usage: " + string(argv[0]) + "-a <n1> -b <n2> -o <add|subtract> [-v]"
            "Adds or subtracts two numbers";
        return;
    }

    if (args.IsSet("version")) // Use the full name to access
    {
        cout << string(argv[0]) << " 1.0" << endl;
        return;
    }

    // We know that number-a is set since Parse would have failed otherwise as it is required
    // We know that number-b is set either by the command line or by the default value
    int a = args.GetAsInt("number-a");  // Use the full name to access
    int b = args.GetAsInt("number-b");

    // We know that operation at least has a default value if it hasn't been set
    string operation = args.Get("operation");
    if (operation == "add")
        cout << a << " + " << b << " = " << a + b << endl;
    else if (operation == "subtract")
        cout << a << " - " << b << " = " << a - b << endl;

}

void TestConstruction()
{
    AssertThrows(Args args({ }), invalid_argument);

    AssertThrows(Flag("", "", ""), invalid_argument);
    AssertThrows(Flag("version", "", ""), invalid_argument);
    AssertThrows(Flag("", "v", ""), invalid_argument);
    AssertThrows(Flag("v", "v", ""), invalid_argument);
    AssertThrows(Flag("version", "vr", ""), invalid_argument);

    AssertThrows(Arg("", "", ""), invalid_argument);
    AssertThrows(Arg("version", "", ""), invalid_argument);
    AssertThrows(Arg("", "v", ""), invalid_argument);
    AssertThrows(Arg("v", "v", ""), invalid_argument);
    AssertThrows(Arg("version", "vr", ""), invalid_argument);

    // Can't set the default to something not in the list of possibles
    AssertThrows(Arg("colour", "c", "Colour", { "red", "blue" }, "green"), invalid_argument);

}


int main(int argc, char* argv[])
{
    //ReadmeSampleCode();
    TestConstruction();

    Args args({
        Flag("version", "v", "Display version information", Flag::Is::Required),
        Flag("another-flag", "a", "Another flag for some reason"),
        Arg("colour", "c", "Colour", { "red", "green", "blue" }, Arg::Is::Required),
        Arg("number", "n", "Number of things", "5"),
        Arg("string", "s", "Some text"),
        Arg("float", "f", "A float"),
        Arg("bool", "b", "A boolean", {"true", "false"}, "false")
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

    AssertPrints(
        AssertTrue(!args.Parse({ "-c", "red" })),
        "Parsing command line failed, details: version is required but was not set\n");

    AssertPrints(
        AssertTrue(!args.Parse({ "-v" })),
        "Parsing command line failed, details: colour is required but was not set\n");

    AssertTrue(args.Parse({}));
    AssertTrue(args.Parse({ "-v", "-c", "red" }));
    AssertTrue(args.Parse({ "-v", "-c", "red", "--number", "5", "--another-flag", "-f", "1.456", "--bool", "true"}));
    AssertTrue(args.Parse({ "-v", "-c", "red" }));
    // Check args clears values on Parse
    AssertFalse(args.IsSet("string"));

    // Check default values are present
    AssertTrue(args.IsSet("bool"));
    AssertFalse(args.GetAsBool("bool"));

    AssertTrue(args.IsSet("number"));
    AssertEquals(5, args.GetAsInt("number"));

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

    AssertTrue(args.Parse({ "--bool", "false", "-v", "-c", "red" }));
    AssertFalse(args.GetAsBool("bool"));

    const int argcTest = 9;
    char *argvTest[argcTest] = { "programName", "-v", "-c", "red", "--number", "5", "-a", "-s", "foo bar" };

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

