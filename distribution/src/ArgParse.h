/*
 *  ArgParse.h
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 22/12/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <string>
#include <unordered_map>
#include <vector>

class ArgParse
{
public:
    ArgParse();

    enum ArgType {String, Int, Bool, Double};

    // this needs to be called first and sets up some of the main information
    void Initialise(int argc, const char **argv, const std::string &appHelpText, size_t maxNumEndArguments, size_t minNumEndArguments);

    // this is used for boolean flags that have no arguments. They return true when set and false otherwise
    void AddArgument(const std::string &shortName, const std::string &longName, const std::string &helpText);

    // this is used for arguments with a fixed number of values
    void AddArgument(const std::string &shortName, const std::string &longName, const std::string &helpText, const std::string &defaultValue, size_t numArgs, bool required, ArgType argType);

    // this is used for arguments with a variable number of values
    void AddArgument(const std::string &shortName, const std::string &longName, const std::string &helpText, const std::string &defaultValue, size_t minArgs, size_t maxArgs, bool required, ArgType argType);

    // this needs to be called after the arguments have been added
    int Parse();

    // this prints out a usage string
    void Usage();

    // these get the values of arguments indexed on their longName values
    bool Get(const std::string &argument, std::vector<std::string> *strings);
    bool Get(const std::string &argument, std::vector<int> *ints);
    bool Get(const std::string &argument, std::vector<bool> *bools);
    bool Get(const std::string &argument, std::vector<double> *doubles);
    bool Get(const std::string &argument, std::string *s);
    bool Get(const std::string &argument, int *i);
    bool Get(const std::string &argument, bool *b);
    bool Get(const std::string &argument, double *d);

    // these get the values of end arguments
    bool Get(std::vector<std::string> *strings);
    bool Get(std::vector<int> *ints);
    bool Get(std::vector<bool> *bools);
    bool Get(std::vector<double> *doubles);
    bool Get(std::string *s);
    bool Get(int *i);
    bool Get(bool *b);
    bool Get(double *d);

    // mostly internal utilities
    static bool IsNumber(const std::string &s);
    static bool IsInt(const std::string &s);
    static bool IsBool(const std::string &s);
    static int ToBool(const std::string &s);
    static double ToDouble(const std::string &buf);
    static int ToInt(const std::string &buf);


private:
    struct Argument
    {
        std::string shortName;
        std::string longName;
        std::string helpText;
        std::string defaultValue;
        size_t minArgs;
        size_t maxArgs;
        bool required;
        ArgType argType;
    };

    static bool ArgumentsOK(const std::string s, const Argument &a);

    std::vector<std::string> m_rawArguments;
    std::vector<Argument> m_argumentList;
    std::unordered_map<std::string, size_t> m_argumentListIndex;
    std::unordered_map<std::string, std::vector<std::string>> m_parsedArguments;
    std::string m_appHelpText;
    std::vector<std::string> m_endArguments;
    size_t m_minNumEndArguments = 0;
    size_t m_maxNumEndArguments = 0;
    std::string m_lastError;
};

#endif // ARGPARSE_H
