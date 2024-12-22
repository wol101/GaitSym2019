#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import string
import array
import os
import re
import time
import math
import gzip

def find_best_log():
    usageString = "Usage:\n%s logfile1 logfile2 ..." % sys.argv[0]
    
    if len(sys.argv) < 2: PrintExit(usageString)

    fileList = sys.argv[1:]

    scores = []
    files = []
    for i in range(0, len(fileList)):
        lines = ConvertFileToLines(fileList[i])
        # now need to parse the various options
        lastLine = lines[-1]
        tokens = ConvertToListOfTokens(lastLine)
        
        # check we have a line of numbers
        allNumbers = 1
        for t in tokens:
            if IsANumber(t) == 0:
                allNumbers = 0
                break
        
        if allNumbers != 0:
            if (len(tokens) == 5):
                scores.append(float(tokens[0]))
                files.append(fileList[i])
            if (len(tokens) == 12):
                scores.append(float(tokens[11]))
                files.append(fileList[i])
 
    scoresAndFiles = list(zip(scores, files))
    scoresAndFiles.sort()
    #scoresAndFiles.reverse()
    sortedScores, sortedFiles = list(zip(*scoresAndFiles))
    
    for i in range(0, len(sortedScores)):
        print("%7f %s" % (sortedScores[i], sortedFiles[i]))

def find_best_log():
    usageString = "Usage:\n%s logfile1 logfile2 ..." % sys.argv[0]
    
    if len(sys.argv) < 2: PrintExit(usageString)

    fileList = sys.argv[1:]

    scores = []
    files = []
    for i in range(0, len(fileList)):
        lines = ConvertFileToLines(fileList[i])
        # now need to parse the various options
        lastLine = lines[-1]
        tokens = ConvertToListOfTokens(lastLine)
        
        # check we have a line of numbers
        allNumbers = 1
        for t in tokens:
            if IsANumber(t) == 0:
                allNumbers = 0
                break
        
        if allNumbers != 0:
            if (len(tokens) == 5):
                scores.append(float(tokens[0]))
                files.append(fileList[i])
            if (len(tokens) == 12):
                scores.append(float(tokens[11]))
                files.append(fileList[i])
 
    scoresAndFiles = list(zip(scores, files))
    scoresAndFiles.sort()
    #scoresAndFiles.reverse()
    sortedScores, sortedFiles = list(zip(*scoresAndFiles))
    
    for i in range(0, len(sortedScores)):
        print("%7f %s" % (sortedScores[i], sortedFiles[i]))
    

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

def ConvertFileToLines(filename):
    """reads in a file and converts to a list of lines"""
    
    if filename.endswith('.gz'):
        input = gzip.open(filename, 'r')
        theLines = input.readlines()
        input.close()
    else:
        input = open(filename, 'r')
        theLines = input.readlines()
        input.close()
    
    return theLines 
    
    
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
    if re.match(r'([+-]?)(?=\d|\.\d)\d*(\.\d*)?([Ee]([+-]?\d+))?', theString) == None: return 0
    return 1

# program starts here

if __name__ == '__main__':
    find_best_log()   
   

