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
//
// Project url: https://github.com/WildCoastSolutions/CommandLine

#ifndef WILD_COMMANDLINE_COMMANDLINE_H
#define WILD_COMMANDLINE_COMMANDLINE_H

#include <string>
#include <initializer_list>
#include <list>
#include <set>
#include <map>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace Wild
{
    namespace CommandLine
    {
        // Whether an arg or flag is optional or required
        enum class Ordinality
        {
            Required,
            Optional
        };

        // But "ordinality" is not a common concept, so simplify for reading
        typedef Ordinality Is;

        // Represents a single argument
        // e.g. -v, --version
        class Arg
        {
        public:

            Arg(){};

            // Specify the name, letter, description and whether required for a command line arg
            // Using this constructor without possible values means that any value is accepted
            Arg(
                const std::string &name,
                const std::string &letter,
                const std::string &description,
                Ordinality ordinality = Is::Optional) :
                name(name),
                letter(letter),
                description(description),
                defaultValue(""),
                defaultValueSet(false),
                ordinality(ordinality),
                isFlag(false)
            {
                CheckValidity();
            }

            // Specify the name, letter, description and whether required for a command line arg
            // Using this constructor without possible values means that any value is accepted
            Arg(
                const std::string &name,
                const std::string &letter,
                const std::string &description,
                const std::string &defaultValue) :
                name(name),
                letter(letter),
                description(description),
                defaultValue(defaultValue),
                defaultValueSet(true),
                ordinality(Is::Optional),
                isFlag(false)
            {
                CheckValidity();
            }

            // Set the name, letter and description, possible values and default value.
            // If a default value is given then by definition the arg is optional
            Arg(
                const std::string &name,
                const std::string &letter,
                const std::string &description,
                std::initializer_list<std::string> possibleValues,
                const std::string &defaultValue) :
                name(name),
                letter(letter),
                description(description),
                possibleValues(possibleValues),
                defaultValue(defaultValue),
                defaultValueSet(true),
                ordinality(Is::Optional),
                isFlag(false)
            {
                if (!IsValidValue(defaultValue)) throw std::invalid_argument("default value " + defaultValue + " is not present in allowed values");
                CheckValidity();
            }

            // Set the name, letter and description, possible values, default value 
            // and whether required for a command line argument
            Arg(
                const std::string &name,
                const std::string &letter,
                const std::string &description,
                std::initializer_list<std::string> possibleValues,
                Ordinality ordinality = Is::Optional) :
                name(name),
                letter(letter),
                description(description),
                possibleValues(possibleValues),
                defaultValue(""),
                defaultValueSet(false),
                ordinality(ordinality),
                isFlag(false)
            {
                CheckValidity();
            }

            // Determines if a value passed to a command line argument is in the list of possible values
            // If the list of possible values is empty the answer is yes
            bool IsValidValue(const std::string &value)
            {
                // If no set was specified that means we allow any value for this argument
                if (possibleValues.size() == 0) return true;
                return (possibleValues.find(value) != possibleValues.end());
            }

            bool IsFlag() { return isFlag; }

            bool IsRequired() { return ordinality == Is::Required; }

            // Full name of the command e.g. "version", used as "--version"
            std::string name;

            // Letter for the command e.g. "v", used as "-v"
            std::string letter;

            // Descriptions, used when printing the usage
            std::string description;

            // Possible values
            std::set<std::string> possibleValues;

            std::string defaultValue;
            bool defaultValueSet;

            // Required or Optional
            Ordinality ordinality;

            bool isFlag = false;

            bool isPositional = false;

        protected:
            virtual void CheckValidity()
            {
                if (name.size() < 2) throw std::invalid_argument("argument name must be two or more letters");
                if (!(letter.size() == 1 || letter.size() == 0))
                    throw std::invalid_argument("argument letter must be one letter or blank");
            }
        };

        class PositionalArg : public Arg
        {
        public:
            PositionalArg(const std::string &name, const std::string &description, Ordinality ordinality = Is::Required) :
                Arg(name, "", description, ordinality)
            {
                isPositional = true;
            }
        };

        class Flag : public Arg
        {
        public:

            Flag(
                const std::string &name,
                const std::string &letter,
                const std::string &description) :
                Arg(name, letter, description, Is::Optional)
            {
                isFlag = true;
                CheckValidity();
            }
        };

        typedef Arg Option;


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
                    if(m_args.count(arg.name) > 0)
                        throw std::invalid_argument("cannot have two arguments with the same name");

                    //if (arg.isPositional &&)

                    m_args[arg.name] = arg;
                    m_insertionOrder.push_back(arg.name);
                    if (arg.letter.size() > 0)
                    {
                        if (m_argLookup.count(arg.letter) > 0)
                            throw std::invalid_argument("cannot have two arguments with the same letter");

                        m_argLookup[arg.letter] = arg.name;
                    }
                    else
                    {
                        m_orderedArgNames.push_back(arg.name);
                    }

                    ResetValues();
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
                    ResetValues();

                    // An empty command line is ok as long as no args were required
                    if (commandLine.size() == 0)
                    {
                        for (auto i : m_args)
                        {
                            if (i.second.IsRequired())
                                throw std::invalid_argument(i.second.name + " is required but was not set");
                        }
                        return true;
                    }

                    auto i = commandLine.begin();
                    auto j = commandLine.begin();
                    j++;

                    Arg arg;
                    std::string nameOrValue;
                    std::string name;
                    std::string value;
                    while (i != commandLine.end())
                    {
                        nameOrValue = StripDashes(*i);
                        if (!LookupArg(nameOrValue, arg))
                        {
                            // If arg doesn't have a name, it could be an ordered arg, where it's order is used to match with the name
                            if(m_currentOrderedArgIndex >= m_orderedArgNames.size())
                                throw std::invalid_argument("couldn't find " + *i + " in specified list of arguments");
 
                            std::string name = m_orderedArgNames[m_currentOrderedArgIndex];
                            m_argValues[name] = nameOrValue;
                            m_currentOrderedArgIndex++;
                        }
                        else
                        {
                            name = nameOrValue;
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
                        }

                        i++;
                        if (i == commandLine.end()) break;
                        j++;
                    }

                    // Check all required flags and args have been set
                    for (auto i : m_args)
                    {
                        if (i.second.IsRequired())
                            if (!IsSet(i.second.name))
                            {
                                throw std::invalid_argument(i.second.name + " is required but was not set");
                            }
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


            // Uses what we know about the args to construct a readable usage string
            std::string Usage(const std::string &appName, bool removeNewLines = false, int numSpacesBeforeDescription = 20)
            {
                std::stringstream s;
                std::stringstream line;
                line << "usage: " << appName << " ";

                for (auto i : m_insertionOrder)
                {
                    Arg a = m_args[i];

                    s << std::setw(numSpacesBeforeDescription) << std::left << std::setfill(' ') << a.letter + (a.letter.size() > 0 ? ", " : "") + a.name;
                    s << std::setw(numSpacesBeforeDescription) << std::left << std::setfill(' ') << a.description;
                    s << std::endl;
                    if (a.possibleValues.size() > 0)
                    {
                        s << std::setw(numSpacesBeforeDescription) << std::left << std::setfill(' ') << "\t";
                        s << "options: ";
                        std::string values;
                        for (auto v : a.possibleValues)
                            values += "|" + v;
                        s << values.substr(1) << std::endl;
                    }
                    if (a.defaultValueSet)
                    {
                        s << std::setw(numSpacesBeforeDescription) << std::left << std::setfill(' ') << "\t";
                        s << "default: " << a.defaultValue << std::endl;
                    }
                    if (a.IsRequired())
                    {
                        s << std::setw(numSpacesBeforeDescription) << std::left << std::setfill(' ') << "\t";
                        s << "required" << std::endl;
                    }


                    // Create first line of usage
                    if (a.IsFlag())
                    {
                        if (a.IsRequired())
                            line << "-" << a.letter << " ";
                        else
                            line << "[-" << a.letter << "] ";
                    }
                    else
                    {
                        if (a.IsRequired())
                            if(a.letter.size() == 0)
                                line << "<" << a.name << "> ";
                            else
                                line << "-" << a.letter << " <" << a.name << "> ";
                        else
                            line << "[-" << a.letter << " " << a.name << "] ";
                    }

                    if (!removeNewLines) s << std::endl;
                }
                return line.str() + "\n\n" + s.str();
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
                if (s.size() == 0) throw std::invalid_argument("argument needs to be at least one character");

                std::string first = s.substr(0, 1);
                if (first != "-") return s;  // could be ordered arg

                std::string second = s.substr(1, 1);

                std::string name;
                if (second == "-")
                    name = s.substr(2);
                else
                    name = s.substr(1);

                return name;
            }

            void ResetValues()
            {
                m_argValues.clear();
                m_currentOrderedArgIndex = 0;
                for (auto i : m_args)
                {
                    if (i.second.defaultValueSet)
                        m_argValues[i.second.name] = i.second.defaultValue;
                }
            }

            std::map<std::string, Arg> m_args;
            std::vector<std::string> m_insertionOrder;
            std::vector<std::string> m_orderedArgNames;
            std::size_t m_currentOrderedArgIndex = 0;
            std::map<std::string, std::string> m_argLookup;
            std::map<std::string, std::string> m_argValues;
            // Afer an optional positional arg, all remaining ones must be optional as well
            bool m_seenOptionalPositionalArg = false;
        };
    }
}

#endif // #ifndef WILD_COMMANDLINE_COMMANDLINE_H
