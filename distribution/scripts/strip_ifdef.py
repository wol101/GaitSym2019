#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import string
import re

def strip_ifdef():

    usage = """strip_ifdef DEFINE in_file out_file"""

    if len(sys.argv) != 4:
        sys.stderr.write(usage + "\n")
        sys.exit(1)

    define = sys.argv[1]
    in_file = sys.argv[2]
    out_file = sys.argv[3]

    inf = open(in_file, "r")
    lines = inf.readlines()
    inf.close()

    outf = open(out_file, "w")

    i = 0;
    while i < len(lines):
        tokens = ConvertToListOfTokensIgnoreQuotes(lines[i])
        if len(tokens) > 0:
            if tokens[0] == "#ifdef":
                if tokens[1] == define:
                    define_count = 1
                    i = i + 1
                    in_else_clause = 0
                    while i < len(lines):
                        tokens = ConvertToListOfTokensIgnoreQuotes(lines[i])
                        if len(tokens) > 0:
                            if tokens[0] == "#ifdef" or tokens[0] == "#ifdefined" or tokens[0] == "#if" or tokens[0] == "#ifndef":
                                define_count = define_count + 1
                                if in_else_clause:
                                    outf.write(lines[i])
                                i = i + 1
                                continue
                            if tokens[0] == "#endif":
                                define_count = define_count - 1
                                if in_else_clause and define_count >= 1:
                                    outf.write(lines[i])
                                i = i + 1
                                if define_count <= 0:
                                    break
                            if tokens[0] == "#else" and define_count == 1:
                                in_else_clause = 1
                                i = i + 1
                                continue
                        if in_else_clause:
                            outf.write(lines[i])
                        i = i + 1
        if (i < len(lines)):
            outf.write(lines[i])
        i = i + 1

    outf.close();

def CopyFile(theInputFileName, theOutputFileName):
    """Copies the contents of a file"""
    theInput = open(theInputFileName, 'r')
    theData = theInput.read()
    theInput.close()
    theOutput = open(theOutputFileName, 'w')
    theOutput.write(theData)
    theOutput.close()

def StartsWith(theString, thePrefix):
    if theString[0: len(thePrefix)] == thePrefix:
        return 1
    return 0

def EndsWith(theString, theSuffix):
    if theString[len(theString) - len(theSuffix):] == theSuffix:
        return 1
    return 0

def PrintExit(value):
    """exits with error message"""
    sys.stderr.write(str(value) + "\n");
    sys.exit(1)

def WriteTokenList(filename, tokenList):
    theOutput = open(filename, 'w')
    for i in range(0, len(tokenList)):
        theOutput.write(str(tokenList[i]) + "\n")
    theOutput.close()


def ConvertToListOfTokens(charList):
    """converts a string into a list of tokens delimited by whitespace"""

    i = 0
    tokenList = []
    while i < len(charList):
        byte = charList[i]
        if byte in string.whitespace:
            i = i + 1
            continue
        word = ""
        if byte == '"':
            i = i + 1
            byte = charList[i]
            while byte != '"':
                word = word + byte
                i = i + 1
                if i >= len(charList): PrintExit("Unmatched \" error")
                byte = charList[i]
        else:
            while (byte in string.whitespace) == 0:
                word = word + byte
                i = i + 1
                if i >= len(charList): break
                byte = charList[i]

        if len(word) > 0: tokenList.append(word)
        i = i + 1

    return tokenList

def ConvertFileToTokens(filename):
    """reads in a file and converts to a list of whitespace delimited tokens"""

    input = open(filename, 'r')
    theBuffer = input.read()
    input.close()

    return ConvertToListOfTokens(theBuffer)

def ConvertToListOfTokensIgnoreQuotes(charList):
    """converts a string into a list of tokens delimited by whitespace"""

    i = 0
    tokenList = []
    while i < len(charList):
        byte = charList[i]
        if byte in string.whitespace:
            i = i + 1
            continue
        word = ""
        while (byte in string.whitespace) == 0:
            word = word + byte
            i = i + 1
            if i >= len(charList): break
            byte = charList[i]

        if len(word) > 0: tokenList.append(word)
        i = i + 1

    return tokenList

def ConvertFileToTokensIgnoreQuotes(filename):
    """reads in a file and converts to a list of whitespace delimited tokens"""

    input = open(filename, 'r')
    theBuffer = input.read()
    input.close()

    return ConvertToListOfTokensIgnoreQuotes(theBuffer)

def GetIndexFromTokenList(tokenList, match):
    """returns the index pointing to the value in the tokenList (match + 1)
       returns -1 on error"""
    for i in range(0, len(tokenList) - 1):
        if tokenList[i] == match: return i + 1

    return -1

def GetValueFromTokenList(tokenList, match):
    """returns value after the matching token in the tokenList (match + 1)
       returns empty string on error"""
    for i in range(0, len(tokenList) - 1):
        if tokenList[i] == match: return tokenList[i + 1]

    return ""

def GetNumberListFromTokenList(tokenList, match):
    """returns value after the matching token in the tokenList (match + 1)
       returns empty list on error"""
    l = []
    for i in range(0, len(tokenList) - 1):
        if tokenList[i] == match:
            for j in range(i + 1, len(tokenList)):
                if IsANumber(tokenList[j]):
                    l.append(float(tokenList[j]))
                else:
                    return l
            return l

    return []

def IsANumber(theString):
    """checks to see whether a string is a valid number"""
    if re.match('([+-]?)(?=\d|\.\d)\d*(\.\d*)?([Ee]([+-]?\d+))?', theString) == None: return 0
    return 1

# program starts here
if __name__ == "__main__":
    strip_ifdef()

