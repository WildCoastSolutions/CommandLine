//
// Author       Wild Coast Solutions
//              David Hamilton
//
// This file is distributed under the MIT License. See the accompanying file
// LICENSE.txt or http://www.opensource.org/licenses/mit-license.php for terms
// and conditions.
//
// This file contains an implementation of a simple command line argument 
// handling library for console applications. 

#ifndef WILD_COMMANDLINE_COMMANDLINE_H
#define WILD_COMMANDLINE_COMMANDLINE_H

#include <string>
#include <initializer_list>
#include <list>
#include <set>
#include <map>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <algorithm>

namespace Wild
{
	namespace CommandLine
	{
        // Represents a single argument
        // e.g. -v, --version
        class Arg
        {
        public:
            Arg(){};

            // Specify the name, letter and description of a command line flag
            // Not specifying possible values means that this is a flag not an argument
            Arg(const std::string &name, const std::string &letter, const std::string &description) :
                name(name), letter(letter), description(description), isFlag(true)
            {}

            // Set the name, letter and description and possible values of a command line argument
            // If possible values is empty, all values are possible
            Arg(const std::string &name, const std::string &letter, const std::string &description, std::initializer_list<std::string> possibleValues) :
                name(name), letter(letter), description(description), possibleValues(possibleValues), isFlag(false)
            {}

            // Determines if a value passed to a command line argument is in the list of possible values
            // If the list of possible values is empty the answer is yes
            bool IsValidValue(const std::string &value)
            {
                // If no set was specified that means we allow any value for this argument
                if (possibleValues.size() == 0) return true;
                return (std::find(possibleValues.begin(), possibleValues.end(), value) != possibleValues.end());
            }

            // Full name of the command e.g. "version", used as "--version"
            std::string name;

            // Letter for the command e.g. "v", used as "-v"
            std::string letter;

            // Descriptions, used when printing the usage
            std::string description;

            // Possible values, if empty anything is allowed
            std::set<std::string> possibleValues;

            // True if this arg is a flag i.e. is just present or absent on the command line and doesn't need a value
            bool isFlag;
        };

        // Represents all arguments the command line supports
        // All supported args are initialised in the constructor, then Parse is called with argc and argv
        // If Parse is successful Args can be queried to get command line argument values
        //
        // e.g.
        //
        //  Args args({
        //      Arg("verbose",      "v",        "Display version information"),
        //      Arg("another-flag", "a",        "Another flag"),
        //      Arg("colour",       "c",        "Colour",       { "red", "green", "blue" }),
        //  });
        //  
        //  e.g. with command line "-v --colour red -a"
        //  if(!args.Parse(argc, argv)){ fail & print usage };
        //
        //  if(args.IsSet("verbose")){ set verbosity }
        //  string colour = args.Get("colour");
        //
        //
        //
        class Args
        {
        public:
            Args(const std::initializer_list<Arg> &args)
            {
                for (auto arg : args)
                {
                    m_args[arg.name] = arg;
                    m_argLookup[arg.letter] = arg.name;
                }
            }

            bool Parse(int argc, char* argv[])
            {
                std::list<std::string> args(argv + 1, argv + argc);
                return Parse(args);
            }

            bool Parse(const std::list<std::string> &commandLine)
            {
                try
                {
                    // Clear values in case Parse is called more than once
                    m_argValues.clear();

                    // An empty command line is ok
                    if (commandLine.size() == 0) return true;

                    auto i = commandLine.begin();
                    auto j = commandLine.begin();
                    j++;

                    Arg arg;
                    std::string name;
                    std::string value;
                    while (i != commandLine.end())
                    {
                        name = StripDashes(*i);
                        if (!LookupArg(name, arg)) throw std::invalid_argument("couldn't find " + *i + " in specified list of arguments");

                        if (arg.isFlag)
                        {
                            m_argValues[name] = "";
                        }
                        else
                        {
                            if (j == commandLine.end()) throw std::invalid_argument("argument " + *i + " given without a value");
                            value = *j;
                            if (!arg.IsValidValue(value)) throw std::invalid_argument("value " + value + " for argument " + *i + " isn't one of the options");
                            m_argValues[name] = value;
                            i++;
                            if (i == commandLine.end()) break;
                            j++;
                        }
                        i++;
                        if (i == commandLine.end()) break;
                        j++;
                        
                    }
                }
                catch (std::invalid_argument &e)
                {
                    std::cout << "Parsing command line failed, details: " << e.what() << std::endl;
                    return false;
                }

                return true;
            }

            bool IsSet(const std::string &name)
            {
                return m_argValues.count(name) > 0;
            }

            std::string Get(const std::string &name)
            {
                return m_argValues[name];
            }

            int GetAsInt(const std::string &name)
            {
                int i;
                std::istringstream(m_argValues[name]) >> i;
                return i;
            }

            bool GetAsBool(const std::string &name)
            {
                bool b;
                std::istringstream(m_argValues[name]) >> std::boolalpha >> b;
                return b;
            }

            float GetAsFloat(const std::string &name)
            {
                float f;
                std::istringstream(m_argValues[name]) >> f;
                return f;
            }

            std::string Usage()
            {

            }

        private:

            bool LookupArg(std::string &name, Arg &arg)
            {
                if (m_argLookup.count(name) > 0)
                    name = m_argLookup[name];

                if (m_args.count(name) > 0)
                {
                    arg = m_args[name];
                    return true;
                }
                else
                    return false;
            }

            std::string StripDashes(const std::string &s)
            {
                if (s.size() < 2) throw std::invalid_argument("argument needs to be at least two characters including -");

                std::string first = s.substr(0, 1);
                if (first != "-") throw std::invalid_argument("argument needs to start with -");

                std::string second = s.substr(1, 1);

                std::string name;
                if (second == "-")
                    name = s.substr(2);
                else
                    name = s.substr(1);

                return name;
            }

            std::map<std::string, Arg> m_args;
            std::map<std::string, std::string> m_argLookup;
            std::map<std::string, std::string> m_argValues;
        };
	}
}

#endif // #ifndef WILD_COMMANDLINE_COMMANDLINE_H
