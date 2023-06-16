/*
 *  ArgParse.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 22/12/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#include "ArgParse.h"

#include "pystring.h"

#include <iostream>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <regex>

using namespace std::string_literals;

ArgParse::ArgParse()
{
}

void ArgParse::Initialise(int argc, const char **argv, const std::string &appHelpText, size_t maxNumEndArguments, size_t minNumEndArguments)
{
    m_rawArguments.clear();
    for (size_t i = 1; i < size_t(argc); i++) m_rawArguments.push_back(std::string(argv[i]));
    m_appHelpText = appHelpText;
    m_minNumEndArguments = minNumEndArguments;
    m_maxNumEndArguments = maxNumEndArguments;
    AddArgument("-h"s, "--help"s, "Display help text"s);
    AddArgument("-v"s, "--verbose"s, "Displays more information"s);
}

void ArgParse::AddArgument(const std::string &shortName, const std::string &longName, const std::string &helpText)
{
    AddArgument(shortName, longName, helpText, "false"s, 0, 0, false, ArgParse::Bool);
}

void ArgParse::AddArgument(const std::string &shortName, const std::string &longName, const std::string &helpText, const std::string &defaultValue, size_t numArgs, bool required, ArgType argType)
{
    AddArgument(shortName, longName, helpText, defaultValue, numArgs, numArgs, required, argType);
}

void ArgParse::AddArgument(const std::string &shortName, const std::string &longName, const std::string &helpText, const std::string &defaultValue, size_t minArgs, size_t maxArgs, bool required, ArgType argType)
{
    if (m_argumentListIndex.count(shortName))
    {
        std::cerr << "Error: " << shortName << " not unique\n";
        std::exit(1);
    }
    if (m_argumentListIndex.count(longName))
    {
        std::cerr << "Error: " << longName << " not unique\n";
        std::exit(1);
    }
    m_argumentListIndex[shortName] = m_argumentList.size();
    m_argumentListIndex[longName] = m_argumentList.size();
    if (defaultValue.size()) m_parsedArguments[longName] = { defaultValue };
    Argument argument;
    argument.shortName = shortName;
    argument.longName = longName;
    argument.helpText = helpText;
    argument.defaultValue = defaultValue;
    argument.minArgs = minArgs;
    argument.maxArgs = maxArgs;
    argument.required = required;
    argument.argType = argType;
    m_argumentList.push_back(argument);
}


int ArgParse::Parse()
{
    // get the argument locations
    std::vector<std::string> arguments;
    std::vector<size_t> locations;
    for (size_t i = 0; i < m_rawArguments.size(); i++)
    {
        if (pystring::startswith(m_rawArguments[i], "-"s))
        {
            arguments.push_back(m_rawArguments[i]);
            locations.push_back(i);
        }
    }
    locations.push_back(m_rawArguments.size());
    // check required arguments are present
    for (auto &&it: m_argumentList)
    {
        if (it.required)
        {
            auto findShort = std::find(arguments.begin(), arguments.end(), it.shortName);
            if (findShort != arguments.end()) continue;
            auto findLong = std::find(arguments.begin(), arguments.end(), it.longName);
            if (findLong == arguments.end())
            {
                m_lastError = it.shortName + " "s + it.longName + " required in argument list"s;
                return __LINE__;
            }
        }
    }
    // check the arguments exist and are the right sizes
    for (size_t i = 0; i < arguments.size(); i++)
    {
        auto it = m_argumentListIndex.find(arguments[i]);
        if (it == m_argumentListIndex.end())
        {
            m_lastError = arguments[i] + " not in argument list"s;
            return __LINE__;
        }
        size_t argsAvail = locations[i + 1] - locations[i] - 1;
        size_t minArgs = m_argumentList[it->second].minArgs;
        size_t maxArgs = m_argumentList[it->second].maxArgs;
        if (i < arguments.size() - 1)
        {
            std::vector<std::string> argumentList;
            if (argsAvail < minArgs || argsAvail > maxArgs)
            {
                m_lastError = arguments[i] + " requires between "s + std::to_string(minArgs) + " and "s + std::to_string(maxArgs) + " arguments"s;
                return __LINE__;
            }
            if (locations[i + 1] - (locations[i] + 1) > 0)
            {
                for (size_t j = locations[i] + 1; j < locations[i + 1]; j++)
                {
                    if (ArgumentsOK(m_rawArguments[j], m_argumentList[it->second]) == false)
                    {
                        m_lastError = m_rawArguments[j] + " is wrong type"s;
                        return __LINE__;
                    }
                    argumentList.push_back(m_rawArguments[j]);
                }
            }
            else
            {
                argumentList.push_back("true"s);
            }
            m_parsedArguments[m_argumentList[it->second].longName] = argumentList;
        }
        else
        {
            std::vector<std::string> argumentList;
            if (argsAvail < minArgs)
            {
                m_lastError = arguments[i] + " requires between "s + std::to_string(minArgs) + " and "s + std::to_string(maxArgs) + " arguments"s;
                return __LINE__;
            }
            size_t endArgsStart = std::min(locations[i] + maxArgs + 1, locations[i + 1]);
            if (endArgsStart - (locations[i] + 1) > 0)
            {
                for (size_t j = locations[i] + 1; j < endArgsStart; j++)
                {
                    argumentList.push_back(m_rawArguments[j]);
                }
            }
            else
            {
                argumentList.push_back("true"s);
            }
            m_parsedArguments[m_argumentList[it->second].longName] = argumentList;
            for (size_t j = endArgsStart; j < locations[i + 1]; j++)
            {
                m_endArguments.push_back(m_rawArguments[j]);
            }
            if (m_endArguments.size() < m_minNumEndArguments || m_endArguments.size() > m_maxNumEndArguments)
            {
                if (m_minNumEndArguments == m_maxNumEndArguments)
                    m_lastError = "Exactly "s + std::to_string(m_minNumEndArguments) + " end arguments required: "s + std::to_string(m_endArguments.size()) + " found"s;
                else
                    m_lastError = "Between "s + std::to_string(m_minNumEndArguments) + " and "s + std::to_string(m_maxNumEndArguments) + " end arguments required: "s + std::to_string(m_endArguments.size()) + " found."s;
                return __LINE__;
            }
        }
    }
    bool helpFlag = false;
    Get("--help"s, &helpFlag);
    if (helpFlag)
    {
        Usage();
        exit(1);
    }
    bool verboseFlag = false;
    Get("--verbose"s, &verboseFlag);
    if (verboseFlag)
    {
        for (auto &&it : m_argumentList)
        {
            auto findIt = m_parsedArguments.find(it.longName);
            if (findIt == m_parsedArguments.end())
            {
                std::cerr << it.longName << " " << it.defaultValue << " [default]\n";
            }
            else
            {
                if (findIt->second.size() == 0)
                    std::cerr << it.longName << " true\n";
                else
                {
                    if (findIt->second.size() == 1)
                        std::cerr << it.longName << " " << findIt->second[0] << "\n";
                    else
                    {
                        std::cerr << it.longName << "\n";
                        for (size_t i = 0; i < findIt->second.size(); i++)
                        {
                            std::cerr << "    " << findIt->second[i] << "\n";
                        }
                    }
                }
            }
        }
    }
    return 0;
}

void ArgParse::Usage()
{
    if (m_lastError.size())
    {
        std::cerr << pystring::join(" "s, m_rawArguments) << "\n\n";
        std::cerr << "ERROR\n" << m_lastError << "\n\n";
    }

    std::cerr << m_appHelpText << "\n";
    std::map<std::string, Argument> required;
    for (size_t i = 0; i < m_argumentList.size(); i++)
    {
        if (m_argumentList[i].required)
        {
            required[m_argumentList[i].longName] = m_argumentList[i];
        }
    }
    std::map<std::string, Argument> optional;
    for (size_t i = 0; i < m_argumentList.size(); i++)
    {
        if (m_argumentList[i].required == false)
        {
            optional[m_argumentList[i].longName] = m_argumentList[i];
        }
    }
    if (required.size())
    {
        std::cerr << "Required Arguments\n";
        for (auto &&it: required)
        {
            std::cerr << it.second.longName << ", " << it.second.shortName << "\n";
            std::cerr << it.second.helpText << "\n";
            if (it.second.minArgs == it.second.maxArgs)
            {
                switch (it.second.argType)
                {
                case ArgType::String:
                    std::cerr << "Exactly " << it.second.minArgs << " string(s) required\n";
                    break;
                case ArgType::Int:
                    std::cerr << "Exactly " << it.second.minArgs << " integer(s) required\n";
                    break;
                case ArgType::Bool:
                    std::cerr << "Exactly " << it.second.minArgs << " boolean(s) required\n";
                    break;
                case ArgType::Double:
                    std::cerr << "Exactly " << it.second.minArgs << " double(s) required\n";
                    break;
                }
            }
            else
            {
                switch (it.second.argType)
                {
                case ArgType::String:
                    std::cerr << "Between " << it.second.minArgs << " and " << it.second.maxArgs << " strings required\n";
                    break;
                case ArgType::Int:
                    std::cerr << "Between " << it.second.minArgs << " and " << it.second.maxArgs << " integers required\n";
                    break;
                case ArgType::Bool:
                    std::cerr << "Between " << it.second.minArgs << " and " << it.second.maxArgs << " booleans required\n";
                    break;
                case ArgType::Double:
                    std::cerr << "Between " << it.second.minArgs << " and " << it.second.maxArgs << " doubles required\n";
                    break;
                }
            }
        }
        std::cerr << "\n";
    }
    if (optional.size())
    {
        std::cerr << "Optional Arguments\n";
        for (auto &&it: optional)
        {
            std::cerr << it.second.longName << ", " << it.second.shortName << "\n";
            std::cerr << it.second.helpText << "\n";
            std::cerr << "Default value is: \"" << it.second.defaultValue << "\"\n";
            if (it.second.minArgs == it.second.maxArgs)
            {
                switch (it.second.argType)
                {
                case ArgType::String:
                    std::cerr << "Exactly " << it.second.minArgs << " string(s) required\n";
                    break;
                case ArgType::Int:
                    std::cerr << "Exactly " << it.second.minArgs << " integer(s) required\n";
                    break;
                case ArgType::Bool:
                    std::cerr << "Exactly " << it.second.minArgs << " boolean(s) required\n";
                    break;
                case ArgType::Double:
                    std::cerr << "Exactly " << it.second.minArgs << " double(s) required\n";
                    break;
                }
            }
            else
            {
                switch (it.second.argType)
                {
                case ArgType::String:
                    std::cerr << "Between " << it.second.minArgs << " and " << it.second.maxArgs << " strings required\n";
                    break;
                case ArgType::Int:
                    std::cerr << "Between " << it.second.minArgs << " and " << it.second.maxArgs << " integers required\n";
                    break;
                case ArgType::Bool:
                    std::cerr << "Between " << it.second.minArgs << " and " << it.second.maxArgs << " booleans required\n";
                    break;
                case ArgType::Double:
                    std::cerr << "Between " << it.second.minArgs << " and " << it.second.maxArgs << " doubles required\n";
                    break;
                }
            }
        }
        std::cerr << "\n";
    }
    if (m_minNumEndArguments)
    {
        std::cerr << "Between " << m_minNumEndArguments << " and " << m_maxNumEndArguments << " end arguments(s) required\n";
    }
}

bool ArgParse::Get(const std::string &argument, std::vector<std::string> *strings)
{
    auto it = m_parsedArguments.find(argument);
    if (it == m_parsedArguments.end()) return false;
    *strings = it->second;
    return true;
}

bool ArgParse::Get(const std::string &argument, std::vector<int> *ints)
{
    auto it = m_parsedArguments.find(argument);
    if (it == m_parsedArguments.end()) return false;
    ints->clear();
    for (size_t i = 0; i < it->second.size(); i++) ints->push_back(ToInt(it->second[i]));
    return true;
}

bool ArgParse::Get(const std::string &argument, std::vector<bool> *bools)
{
    auto it = m_parsedArguments.find(argument);
    if (it == m_parsedArguments.end()) return false;
    bools->clear();
    for (size_t i = 0; i < it->second.size(); i++) bools->push_back(ToBool(it->second[i]));
    return true;
}

bool ArgParse::Get(const std::string &argument, std::vector<double> *doubles)
{
    auto it = m_parsedArguments.find(argument);
    if (it == m_parsedArguments.end()) return false;
    doubles->clear();
    for (size_t i = 0; i < it->second.size(); i++) doubles->push_back(ToDouble(it->second[i]));
    return true;
}

bool ArgParse::Get(const std::string &argument, std::string *s)
{
    auto it = m_parsedArguments.find(argument);
    if (it == m_parsedArguments.end()) return false;
    *s = it->second[0];
    return true;
}

bool ArgParse::Get(const std::string &argument, int *i)
{
    auto it = m_parsedArguments.find(argument);
    if (it == m_parsedArguments.end()) return false;
    *i = ToInt(it->second[0]);
    return true;
}

bool ArgParse::Get(const std::string &argument, bool *b)
{
    auto it = m_parsedArguments.find(argument);
    if (it == m_parsedArguments.end()) return false;
    *b = ToBool(it->second[0]);
    return true;
}

bool ArgParse::Get(const std::string &argument, double *d)
{
    auto it = m_parsedArguments.find(argument);
    if (it == m_parsedArguments.end()) return false;
    *d = ToDouble(it->second[0]);
    return true;
}

bool ArgParse::Get(std::vector<std::string> *strings)
{
    if (m_endArguments.size() == 0) return false;
    *strings = m_endArguments;
    return true;
}

bool ArgParse::Get(std::vector<int> *ints)
{
    if (m_endArguments.size() == 0) return false;
    ints->clear();
    for (size_t i = 0; i < m_endArguments.size(); i++) ints->push_back(ToInt(m_endArguments[i]));
    return true;
}

bool ArgParse::Get(std::vector<bool> *bools)
{
    if (m_endArguments.size() == 0) return false;
    bools->clear();
    for (size_t i = 0; i < m_endArguments.size(); i++) bools->push_back(ToBool(m_endArguments[i]));
    return true;
}

bool ArgParse::Get(std::vector<double> *doubles)
{
    if (m_endArguments.size() == 0) return false;
    doubles->clear();
    for (size_t i = 0; i < m_endArguments.size(); i++) doubles->push_back(ToDouble(m_endArguments[i]));
    return true;
}

bool ArgParse::Get(std::string *s)
{
    if (m_endArguments.size() == 0) return false;
    *s = m_endArguments[0];
    return true;
}

bool ArgParse::Get(int *i)
{
    if (m_endArguments.size() == 0) return false;
    *i = ToInt(m_endArguments[0]);
    return true;
}

bool ArgParse::Get(bool *b)
{
    if (m_endArguments.size() == 0) return false;
    *b = ToBool(m_endArguments[0]);
    return true;
}

bool ArgParse::Get(double *d)
{
    if (m_endArguments.size() == 0) return false;
    *d = ToDouble(m_endArguments[0]);
    return true;
}

bool ArgParse::IsNumber(const std::string &s)
{
    std::regex e("^([+-]?)(?=[0-9]|\\.[0-9])[0-9]*(\\.[0-9]*)?([Ee]([+-]?[0-9]+))?$");
    return std::regex_match (s, e);
}

bool ArgParse::IsInt(const std::string &s)
{
    std::regex e("^(?:(0[xX][a-fA-F0-9]+(?:[uU](?:ll|LL|[lL])?|(?:ll|LL|[lL])[uU]?)?)$"           // Hexadecimal
                 "|^([1-9][0-9]*(?:[uU](?:ll|LL|[lL])?|(?:ll|LL|[lL])[uU]?)?)$"                    // Decimal
                 "|^(0[0-7]*(?:[uU](?:ll|LL|[lL])?|(?:ll|LL|[lL])[uU]?)?))$"s);                   // Octal
    return std::regex_match (s, e);
}

bool ArgParse::IsBool(const std::string &s)
{
    std::string lower = pystring::lower(s);
    if (lower == "false"s || lower == "true") return true;
    return false;
}

int ArgParse::ToBool(const std::string &s)
{
    std::string lower = pystring::lower(s);
    if (lower == "false"s) return 0;
    if (lower == "true"s) return 1;
    return -1;
}

double ArgParse::ToDouble(const std::string &buf)
{
    return std::strtod(buf.c_str(), nullptr); // note: not using std::stod because I do not want exceptions to be thrown
}

 int ArgParse::ToInt(const std::string &buf)
{
    return int(std::strtol(buf.c_str(), nullptr, 0)); // note: not using std::stoi because I do not want exceptions to be thrown
}

bool ArgParse::ArgumentsOK(const std::string s, const Argument &a)
{
    switch (a.argType)
    {
    case ArgType::String:
        return true;
    case ArgType::Int:
        return IsInt(s);
    case ArgType::Bool:
        return IsBool(s);
    case ArgType::Double:
        return IsNumber(s);
    }
    return false;
}
