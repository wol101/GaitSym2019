/*
 *  DataFile.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 24/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

// DataFile.cpp - utility class to read in various sorts of data files

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <iostream>
#include <locale>
#include <codecvt>

#ifdef STRINGS_H_NEEDED
#include <strings.h>
#endif

#include "DataFile.h"

#ifdef _WIN32
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#include<Windows.h>
#endif

//#ifdef USE_STRCMPI
//#define strcasecmp strcmpi
//#define strncasecmp strcmp
//#endif

const static size_t kStorageIncrement = 65536;

// default constructor
DataFile::DataFile()
{
}

// default destructor
DataFile::~DataFile()
{
    if (m_FileData) delete [] m_FileData;
}

// set the raw data adding a terminating 0
void DataFile::SetRawData(const char *string, size_t stringLen)
{
    if (m_FileData) delete [] m_FileData;
    if (stringLen == 0) m_Size = strlen(string) + 1;
    else m_Size = stringLen + 1;
    m_FileData = new char [m_Size];
    memcpy(m_FileData, string, m_Size);
    m_FileData[stringLen] = 0;
    m_Index = m_FileData;
}

void DataFile::ClearData()
{
    if (m_FileData) delete [] m_FileData;
    m_Size = 0;
    m_FileData = nullptr;
    m_Index = m_FileData;
}

// preforms a global search and replace
size_t DataFile::Replace(const char *oldString, const char *newString)
{
    size_t count = 0;
    size_t oldLen = strlen(oldString);
    size_t newLen = strlen(newString);
    char *startPtr = m_FileData;
    char *foundPtr;
    char *endPtr;
    size_t size;
    char *newBuffer;
    size_t i;
    char **segment = new char *[1 + m_Size / oldLen]; // bound to be big enough

    do
    {
        foundPtr = strstr(startPtr, oldString);
        if (foundPtr)
        {
            size = size_t(foundPtr - startPtr); // always positive
            segment[count] = new char[size + 1];
            memcpy(segment[count], startPtr, size);
            segment[count][size] = 0;
            count ++;
            startPtr = foundPtr + oldLen;
            endPtr = startPtr;
        }
    } while (foundPtr);

    if (count) // safe but slow version - would be quicker with memcpy and lots of string length storage
    {
        newBuffer = new char[m_Size + (newLen - oldLen) * count + 1];
        *newBuffer = 0;
        for (i = 0; i < count; i++)
        {
            strcat(newBuffer, segment[i]);
            delete segment[i];
            strcat(newBuffer, newString);
        }
        strcat(newBuffer, endPtr);
        delete [] m_FileData;
        m_Size = strlen(newBuffer) + 1;
        m_FileData = newBuffer;
        m_Index = m_FileData;
    }

    delete [] segment;
    return count;
}

// read the named file
// returns true on error
// note if used on windows this routine calls the wide character versions after converting the string
bool DataFile::ReadFile(const std::string &name)
{
#if (defined(_WIN32) || defined(WIN32)) && !defined(__MINGW32__)
    m_PathName = name;
    return ReadFile(ConvertUTF8ToWide(name));
#else
    struct stat fileStat;
    FILE *in;
    size_t count = 0;
    size_t index, read_block;
    size_t max_read_block = 256LL * 256LL * 256LL * 64LL;
    int error;

    m_PathName = name;

    if (m_FileData) delete [] m_FileData;
    m_FileData = nullptr;

    error = stat(name.c_str(), &fileStat);
    if (error && m_ExitOnErrorFlag)
    {
        std::cerr << "Error: DataFile::ReadFile(" << name << ") - Cannot stat file\n";
        exit(1);
    }
    if (error) return true;
    m_FileData = new char[size_t(fileStat.st_size) + 1];
    if (m_FileData == nullptr && m_ExitOnErrorFlag)
    {
        std::cerr << "Error: DataFile::ReadFile(" << name << ") - Cannot allocate m_FileData\n";
        exit(1);
    }
    if (m_FileData == nullptr) return true;
    m_Index = m_FileData;
    m_Size = size_t(fileStat.st_size);
    m_FileData[m_Size] = 0;

    in = fopen(name.c_str(), "rb");
    if (in == nullptr && m_ExitOnErrorFlag)
    {
        std::cerr << "Error: DataFile::ReadFile(" << name << ") - Cannot open file\n";
        exit(1);
    }
    if (in == nullptr) return true;
    for (index = 0; index < size_t(fileStat.st_size); index += max_read_block)
    {
        read_block = (size_t(fileStat.st_size) - index);
        if (read_block > max_read_block) read_block = max_read_block;
        count = read_block;
        count = fread(m_FileData + index, count, 1, in);
        if (count != 1 && m_ExitOnErrorFlag)
        {
            std::cerr << "Error: DataFile::ReadFile(" << name << ") - Cannot read file\n";
            exit(1);
        }
    }
    fclose(in);

    return false;

#endif
}

// write the data to a file
// if binary is true, the whole of the buffer (excluding terminating zero) is written
// otherwise it is just the string until the first terminating zero
bool DataFile::WriteFile(const std::string &name, bool binary)
{
#if (defined(_WIN32) || defined(WIN32)) && !defined(__MINGW32__)
    return WriteFile(ConvertUTF8ToWide(name), binary);
#else
    FILE *out;
    size_t count;

    out = fopen(name.c_str(), "wb");

    if (out == nullptr)
    {
        if (m_ExitOnErrorFlag)
        {
            std::cerr << "Error: DataFile::WriteFile(" << name << ") - Cannot open file\n";
            exit(1);
        }
        else return true;
    }

    // write file
    if (binary) count = fwrite(m_FileData, m_Size - 1, 1, out);
    else count = fwrite(m_FileData, strlen(m_FileData), 1, out);

    if (count != 1)
    {
        if (m_ExitOnErrorFlag)
        {
            std::cerr << "Error: DataFile::WriteFile(" << name << ") - Cannot write file\n";
            exit(1);
        }
        else return true;
    }

    if (fclose(out))
    {
        if (m_ExitOnErrorFlag)
        {
            std::cerr << "Error: DataFile::WriteFile(" << name << ") - Cannot close file\n";
            exit(1);
        }
        else return true;
    }

    return false;
#endif
}

#if defined(_WIN32) || defined(WIN32)
// provide Windows specific wchar versions
// read the named file
// returns true on error
bool DataFile::ReadFile(const std::wstring &name)
{
    struct _stat64 fileStat;
    FILE *in;
    size_t count = 0;
    size_t index, read_block;
    size_t max_read_block = 256LL * 256LL * 256LL * 64LL;
    int error;

    m_WPathName = name;

    if (m_FileData) delete [] m_FileData;
    m_FileData = nullptr;

    error = _wstat64(name.c_str(), &fileStat);
    if (error && m_ExitOnErrorFlag)
    {
        std::wcerr << L"Error: DataFile::ReadFile(" << name << L") - Cannot stat file\n";
        exit(1);
    }
    if (error) return true;
    m_FileData = new char[size_t(fileStat.st_size) + 1];
    if (m_FileData == nullptr && m_ExitOnErrorFlag)
    {
        std::wcerr << L"Error: DataFile::ReadFile(" << name << L") - Cannot allocate m_FileData\n";
        exit(1);
    }
    if (m_FileData == nullptr) return true;
    m_Index = m_FileData;
    m_Size = size_t(fileStat.st_size);
    m_FileData[m_Size] = 0;

    in = _wfopen(name.c_str(), L"rb");
    if (in == nullptr && m_ExitOnErrorFlag)
    {
        std::wcerr << L"Error: DataFile::ReadFile(" << name << L") - Cannot open file\n";
        exit(1);
    }
    if (in == nullptr) return true;
    for (index = 0; index < size_t(fileStat.st_size); index += max_read_block)
    {
        read_block = (size_t(fileStat.st_size) - index);
        if (read_block > max_read_block) read_block = max_read_block;
        count = read_block;
        count = fread(m_FileData + index, count, 1, in);
        if (count != 1 && m_ExitOnErrorFlag)
        {
            std::wcerr << L"Error: DataFile::ReadFile(" << name << L") - Cannot read file\n";
            exit(1);
        }
    }
    fclose(in);

    return false;
}

// write the data to a file
// if binary is true, the whole of the buffer (excluding terminating zero) is written
// otherwise it is just the string until the first terminating zero
bool DataFile::WriteFile(const std::wstring &name, bool binary)
{
    FILE *out;
    size_t count;

    out = _wfopen(name.c_str(), L"wb");

    if (out == nullptr)
    {
        if (m_ExitOnErrorFlag)
        {
            std::wcerr << L"Error: DataFile::WriteFile(" << name << L") - Cannot open file\n";
            exit(1);
        }
        else return true;
    }

    // write file
    if (binary) count = fwrite(m_FileData, m_Size - 1, 1, out);
    else count = fwrite(m_FileData, strlen(m_FileData), 1, out);

    if (count != 1)
    {
        if (m_ExitOnErrorFlag)
        {
            std::wcerr << L"Error: DataFile::WriteFile(" << name << L") - Cannot write file\n";
            exit(1);
        }
        else return true;
    }

    if (fclose(out))
    {
        if (m_ExitOnErrorFlag)
        {
            std::wcerr << L"Error: DataFile::WriteFile(" << name << L") - Cannot close file\n";
            exit(1);
        }
        else return true;
    }

    return false;
}
#endif

// read an integer parameter
// returns false on success
bool DataFile::RetrieveParameter(const char * const param, int *val, bool searchFromStart)
{
    char buffer[64];

    if (RetrieveParameter(param, buffer, sizeof(buffer), searchFromStart)) return true;

    *val = strtol(buffer, nullptr, 10);

    return false;
}

// read an unsigned integer parameter
// returns false on success
bool DataFile::RetrieveParameter(const char * const param, unsigned int *val, bool searchFromStart)
{
    char buffer[64];

    if (RetrieveParameter(param, buffer, sizeof(buffer), searchFromStart)) return true;

    *val = strtoul(buffer, nullptr, 10);

    return false;
}


// read a double parameter
// returns false on success
bool DataFile::RetrieveParameter(const char * const param, double *val, bool searchFromStart)
{
    char buffer[64];

    if (RetrieveParameter(param, buffer, sizeof(buffer), searchFromStart)) return true;

    *val = strtod(buffer, nullptr);

    return false;
}

// read a bool parameter
// returns false on success
bool DataFile::RetrieveParameter(const char * const param, bool *val, bool searchFromStart)
{
    char buffer[64];

    if (RetrieveParameter(param, buffer, sizeof(buffer), searchFromStart)) return true;

    if (strcmp(buffer, "true") == 0 || strcmp(buffer, "TRUE") == 0 || strcmp(buffer, "1") == 0)
    {
        *val = true;
        return false;
    }
    if (strcmp(buffer, "false") == 0 || strcmp(buffer, "FALSE") == 0 || strcmp(buffer, "0") == 0)
    {
        *val = false;
        return false;
    }

    return true;
}

// read a string parameter - up to (size - 1) bytes
// returns false on success
bool DataFile::RetrieveParameter(const char * const param, char *val, size_t size, bool searchFromStart)
{
    if (FindParameter(param, searchFromStart)) return true;

    return (ReadNext(val, size));
}

// read a string parameter as a ptr and length (no copying)
// returns false on success
bool DataFile::RetrieveParameter(const char * const param, char **val, size_t *size, bool searchFromStart)
{
    if (FindParameter(param, searchFromStart)) return true;

    return (ReadNext(val, size));
}

// read a std::string parameter
// returns false on success
bool DataFile::RetrieveParameter(const char * const param, std::string *val, bool searchFromStart)
{
    if (FindParameter(param, searchFromStart)) return true;

    return (ReadNext(val));
}

// read a quoted string parameter - up to (size - 1) bytes
// returns false on success
bool DataFile::RetrieveQuotedStringParameter(const char * const param, char *val, size_t size, bool searchFromStart)
{
    if (FindParameter(param, searchFromStart)) return true;

    return (ReadNextQuotedString(val, size));
}

// read a quoted string parameter -as a ptr and length (no copying)
// returns false on success
bool DataFile::RetrieveQuotedStringParameter(const char * const param, char **val, size_t *size, bool searchFromStart)
{
    if (FindParameter(param, searchFromStart)) return true;

    return (ReadNextQuotedString(val, size));
}

// read a quoted std::string parameter
// returns false on success
bool DataFile::RetrieveQuotedStringParameter(const char * const param, std::string *val, bool searchFromStart)
{
    if (FindParameter(param, searchFromStart)) return true;

    return (ReadNextQuotedString(val));
}


// return a parameter selected from a range of values
bool DataFile::RetrieveRangedParameter(const char * const param, double *val, bool searchFromStart)
{
    if (FindParameter(param, searchFromStart)) return true;
    if (ReadNextRanged(val)) return true;
    return false;
}

// find a parameter and move index to just after parameter
// NB can't have whitespace in parameter (might work but not guaranteed)
// in fact there are lots of ways this can be confused
// I'm assuming that the system strstr function is more efficient
// than anything I might come up with
bool DataFile::FindParameter(const char * const param,
                             bool searchFromStart)
{
    char *p;
    size_t len = strlen(param);

    if (searchFromStart) p = m_FileData;
    else p = m_Index;

    while (1)
    {
        p = strstr(p, param);
        if (p == nullptr) break; // not found at all
        if (p == m_FileData) // at beginning of file
        {
            if (*(p + len) < 33) // ends with whitespace
            {
                m_Index = p + len;
                return false;
            }
        }
        else
        {
            if (*(p - 1) < 33) // character before is whitespace
            {
                if (*(p + len) < 33) // ends with whitespace
                {
                    m_Index = p + len;
                    return false;
                }
            }
        }
        p += len;
    }
    if (m_ExitOnErrorFlag)
    {
        std::cerr << "Error: DataFile::FindParameter(" << param
        << " - could not find parameter\n";
        exit(1);
    }
    return true;
}

// read the next whitespace delimited token - up to (size - 1) characters
// automatically copes with quote delimited strings
bool DataFile::ReadNext(char *val, size_t size)
{
    size_t len = 0;

    // find non-whitespace
    while (*m_Index < 33)
    {
        if (*m_Index == 0 && m_ExitOnErrorFlag)
        {
            std::cerr << "Error: DataFile::ReadNext no non-whitespace found\n";
            exit(1);
        }
        if (*m_Index == 0) return true;
        m_Index++;
    }

    if (*m_Index == '\"') return ReadNextQuotedString(val, size);

    // copy until whitespace
    while (*m_Index > 32)
    {
        *val = *m_Index;
        val++;
        m_Index++;
        len++;
        if (len == size - 1) break;
    }
    *val = 0;
    return false;
}

// read the next whitespace delimited token - up to (size - 1) characters
// automatically copes with quote delimited strings
// returns the start pointer and length
bool DataFile::ReadNext(char **val, size_t *size)
{
    *size = 0;

    // find non-whitespace
    while (*m_Index < 33)
    {
        if (*m_Index == 0 && m_ExitOnErrorFlag)
        {
            std::cerr << "Error: DataFile::ReadNext no non-whitespace found\n";
            exit(1);
        }
        if (*m_Index == 0) return true;
        m_Index++;
    }

    if (*m_Index == '\"') return ReadNextQuotedString(val, size);

    *val = m_Index;
    // count until whitespace
    while (*m_Index > 32)
    {
        m_Index++;
        (*size)++;
    }
    return false;
}

// read the next whitespace delimited token
// automatically copes with quote delimited strings
bool DataFile::ReadNext(std::string *val)
{
    // find non-whitespace
    while (*m_Index < 33)
    {
        if (*m_Index == 0 && m_ExitOnErrorFlag)
        {
            std::cerr << "Error: DataFile::ReadNext no non-whitespace found\n";
            exit(1);
        }
        if (*m_Index == 0) return true;
        m_Index++;
    }

    if (*m_Index == '\"') return ReadNextQuotedString(val);

    // copy until whitespace
    val->clear();
    while (*m_Index > 32)
    {
        *val += *m_Index++;
    }
    return false;
}

// read a quoted string parameter - up to (size - 1) bytes
// returns false on success
bool DataFile::ReadNextQuotedString(char *val, size_t size)
{
    char *start;
    char *end;
    size_t len;

    start = strstr(m_Index, "\"");
    if (start == nullptr && m_ExitOnErrorFlag)
    {
        std::cerr << "Error: DataFile::ReadNextQuotedString could not find opening \"\n";
        exit(1);
    }
    if (start == nullptr) return true;

    end = strstr(start + 1, "\"");
    if (end == nullptr && m_ExitOnErrorFlag)
    {
        std::cerr << "Error: DataFile::ReadNextQuotedString could not find closing \"\n";
        exit(1);
    }
    if (end == nullptr) return true;

    len = size_t(end - start - 1); // this is always greater than 0
    if (len >= size) len = size - 1;
    m_Index = start + len + 2;
    memcpy(val, start + 1, len);
    val[len] = 0;

    return false;
}

// read a quoted string parameter - up to (size - 1) bytes
// returns false on success
// returns the start pointer and length
bool DataFile::ReadNextQuotedString(char **val, size_t *size)
{
    char *start;
    char *end;

    start = strstr(m_Index, "\"");
    if (start == nullptr && m_ExitOnErrorFlag)
    {
        std::cerr << "Error: DataFile::ReadNextQuotedString could not find opening \"\n";
        exit(1);
    }
    if (start == nullptr) return true;

    end = strstr(start + 1, "\"");
    if (end == nullptr && m_ExitOnErrorFlag)
    {
        std::cerr << "Error: DataFile::ReadNextQuotedString could not find closing \"\n";
        exit(1);
    }
    if (end == nullptr) return true;

    *size = size_t(end - start - 1); // this is always greater than 0
    m_Index = start + *size + 2;
    *val = start + 1;

    return false;
}

// read a quoted string parameter - up to (size - 1) bytes
// returns false on success
bool DataFile::ReadNextQuotedString(std::string *val)
{
    char *start;
    char *end;
    size_t len;

    start = strstr(m_Index, "\"");
    if (start == nullptr && m_ExitOnErrorFlag)
    {
        std::cerr << "Error: DataFile::ReadNextQuotedString could not find opening \"\n";
        exit(1);
    }
    if (start == nullptr) return true;

    end = strstr(start + 1, "\"");
    if (end == nullptr && m_ExitOnErrorFlag)
    {
        std::cerr << "Error: DataFile::ReadNextQuotedString could not find closing \"\n";
        exit(1);
    }
    if (end == nullptr) return true;

    len = size_t(end - start - 1); // this is always greater than 0
    m_Index = start + len + 2;
    val->clear();
    std::copy(start + 1, start + 1 + len, std::back_inserter(*val));

    return false;
}

// read the next integer
bool DataFile::ReadNext(int *val)
{
    char buffer[64];

    if (ReadNext(buffer, sizeof(buffer))) return true;

    *val = strtol(buffer, nullptr, 10);

    return false;
}

// read the next double
bool DataFile::ReadNext(double *val)
{
    char buffer[64];

    if (ReadNext(buffer, sizeof(buffer))) return true;

    *val = strtod(buffer, nullptr);

    return false;
}

// return the next ranged parameter
bool DataFile::ReadNextRanged(double *val)
{
    double low, high;
    if (ReadNext(&low)) return true;
    if (ReadNext(&high)) return true;

    // m_RangeControl is normally from 0 to 1.0
    *val = low + m_RangeControl * (high - low);
    return false;
}

// read an array of ints
bool DataFile::RetrieveParameter(const char * const param, size_t n, int *val, bool searchFromStart)
{
    size_t i;
    if (FindParameter(param, searchFromStart)) return true;

    for (i = 0; i < n; i++)
    {
        if (ReadNext(&(val[i]))) return true;
    }

    return false;
}

// read an array of doubles
bool DataFile::RetrieveParameter(const char * const param, size_t n, double *val, bool searchFromStart)
{
    size_t i;
    if (FindParameter(param, searchFromStart)) return true;

    for (i = 0; i < n; i++)
    {
        if (ReadNext(&(val[i]))) return true;
    }

    return false;
}

// read an array of ranged doubles
bool DataFile::RetrieveRangedParameter(const char * const param, size_t n, double *val, bool searchFromStart)
{
    size_t i;
    if (FindParameter(param, searchFromStart)) return true;

    for (i = 0; i < n; i++)
    {
        if (ReadNextRanged(&(val[i]))) return true;
    }

    return false;
}

// read a line, optionally ignoring blank lines and comments
// (comment string to end of line)
// returns true on error
bool DataFile::ReadNextLine2(char *line, size_t size, bool ignoreEmpty, const char *commentString, const char *continuationString)
{
    char *c;
    bool loopFlag = true;
    bool openQuotes = false;

    while (loopFlag)
    {
        if (ReadNextLine(line, size)) return true;

        if (commentString)
        {
            c = line;
            while (*c)
            {
                if (*c == '"') openQuotes = !openQuotes;
                if (strncmp(c, commentString, strlen(commentString)) == 0 && openQuotes == false)
                {
                    *c = 0;
                    break;
                }
                c++;
            }
        }

        if (ignoreEmpty)
        {
            c = line;
            while (*c)
            {
                if (*c > 32)
                {
                    loopFlag = false;
                    break;
                }
                c++;
            }
        }
        else loopFlag = false;

        if (continuationString)
        {
            c = line;
            if (StringEndsWith(c, continuationString))
            {
                loopFlag = true;
                size_t len = size_t(strlen(c)) - size_t(strlen(continuationString));
                line = line + len;
                size = size - len; // always positive
            }
        }
    }
    return false;
}


// read a line, optionally ignoring blank lines and comments
// (comment string to end of line)
// returns true on error
bool DataFile::ReadNextLine(char *line, size_t size, bool ignoreEmpty, char commentChar, char continuationChar)
{
    char *c;
    bool loopFlag = true;
    bool openQuotes = false;

    while (loopFlag)
    {
        if (ReadNextLine(line, size)) return true;

        if (commentChar)
        {
            c = line;
            while (*c)
            {
                if (*c == '"') openQuotes = !openQuotes;
                if (*c == commentChar && openQuotes == false)
                {
                    *c = 0;
                    break;
                }
                c++;
            }
        }

        if (ignoreEmpty)
        {
            c = line;
            while (*c)
            {
                if (*c > 32)
                {
                    loopFlag = false;
                    break;
                }
                c++;
            }
        }
        else loopFlag = false;

        if (continuationChar)
        {
            c = line;
            if (*c)
            {
                while (*c)
                {
                    c++;
                    size--;
                }
                c--;
                if (*c == continuationChar)
                {
                    loopFlag = true;
                    size++;
                    line = c;
                }
            }
        }
    }
    return false;
}


// read a line
// returns true on error
bool DataFile::ReadNextLine(char *line, size_t size)
{
    char *p = m_Index;
    char *c = line;
    size_t count = 0;
    size--; // needs to be shrunk to make room for the zero

    if (*p == 0) return true; // at end of file

    while (EndOfLineTest(&p) == false)
    {
        if (count >= size)
        {
            *c = 0;
            if (m_ExitOnErrorFlag)
            {
                std::cerr << "Error: DataFile::ReadNextLine line longer than string\n";
                exit(1);
            }
            else return true;
        }

        *c = *p;
        count++;
        c++;
        p++;
    }
    m_Index = p;
    *c = 0;
    return false;
}

// tests for end of line and bumps pointer
// note takes a pointer to a pointer
bool DataFile::EndOfLineTest(char **p)
{
    if (**p == 0) return true; // don't bump past end of string
    if (**p == 10) // must be Unix style linefeed
    {
        (*p)++;
        return true;
    }
    if (**p == 13) // Mac or DOS
    {
        (*p)++;
        if (**p == 10) (*p)++; // DOS
        return true;
    }
    return false;
}

// Count token utility
size_t DataFile::CountTokens(const char *string)
{
    const char *p = string;
    bool inToken = false;
    size_t count = 0;

    while (*p != 0)
    {
        if (inToken == false && *p > 32)
        {
            inToken = true;
            count++;
            if (*p == '"')
            {
                p++;
                while (*p != '"')
                {
                    p++;
                    if (*p == 0) return count;
                }
            }
        }
        else if (inToken == true && *p <= 32)
        {
            inToken = false;
        }
        p++;
    }
    return count;
}

// Return tokens utility
// note string is altered by this routine
// if returned count is >= size then there are still tokens
// (this is probably an error status)
// recommend that tokens are counted first
size_t DataFile::ReturnTokens(char *string, char *ptrs[], size_t size)
{
    char *p = string;
    bool inToken = false;
    size_t count = 0;

    while (*p != 0)
    {
        if (inToken == false && *p > 32)
        {
            inToken = true;
            if (count >= size) return count;
            ptrs[count] = p;
            count++;
            if (*p == '"')
            {
                p++;
                ptrs[count - 1] = p;
                while (*p != '"')
                {
                    p++;
                    if (*p == 0) return count;
                }
                *p = 0;
            }
        }
        else if (inToken == true && *p <= 32)
        {
            inToken = false;
            *p = 0;
        }
        p++;
    }
    return count;
}

// Count token utility
size_t DataFile::CountTokens(const char *string, const char *separators)
{
    const char *p = string;
    bool inToken = false;
    size_t count = 0;

    while (*p != 0)
    {
        if (inToken == false && strchr(separators, *p) == nullptr)
        {
            inToken = true;
            count++;
            if (*p == '"')
            {
                p++;
                while (*p != '"')
                {
                    p++;
                    if (*p == 0) return count;
                }
            }
        }
        else if (inToken == true && strchr(separators, *p) != nullptr)
        {
            inToken = false;
        }
        p++;
    }
    return count;
}

// Return tokens utility
// note string is altered by this routine
// if returned count is >= size then there are still tokens
// (this is probably an error status)
// recommend that tokens are counted first
size_t DataFile::ReturnTokens(char *string, char *ptrs[], size_t size, const char *separators)
{
    char *p = string;
    bool inToken = false;
    size_t count = 0;

    while (*p != 0)
    {
        if (inToken == false && strchr(separators, *p) == nullptr)
        {
            inToken = true;
            if (count >= size) return count;
            ptrs[count] = p;
            count++;
            if (*p == '"')
            {
                p++;
                ptrs[count - 1] = p;
                while (*p != '"')
                {
                    p++;
                    if (*p == 0) return count;
                }
                *p = 0;
            }
        }
        else if (inToken == true && strchr(separators, *p) != nullptr)
        {
            inToken = false;
            *p = 0;
        }
        p++;
    }
    return count;
}

// Count lines utility
size_t DataFile::CountLines(const char *string)
{
    const char *p = string;
    size_t count = 0;

    while (*p != 0)
    {
        if (*p == 10) // lf style line ending
        {
            count++;
            p++;
            continue;
        }

        if (*p == 13) // cr style line ending
        {
            count++;
            p++;
            if (*p == 10) // cr+lf style line ending
            {
                p++;
            }
            continue;
        }

        p++;
    }
    return count;
 }

// Return lines utility
// note string is altered by this routine
// if returned count is >= size then there are still tokens
// (this is probably an error status)
// recommend that tokens are counted first
size_t DataFile::ReturnLines(char *string, char *ptrs[], size_t size)
{
    char *p = string;
    size_t count = 0;
    ptrs[0] = p;

    while (*p != 0)
    {
        if (*p == 10) // lf style line ending
        {
            *p = 0; // replace the end of line with an end of string
            count++;
            if (count >= size) return count;
            p++;
            ptrs[count] = p;
            continue;
        }

        if (*p == 13) // cr style line ending
        {
            *p = 0; // replace the end of line with an end of string
            count++;
            if (count >= size) return count;
            p++;
            if (*p == 10) // cr+lf style line ending
            {
                p++;
            }
            ptrs[count] = p;
            continue;
        }

        p++;
    }
    return count;
}

// read the next integer
bool DataFile::ReadNextBinary(int *val)
{
    if (size_t(m_Index - m_FileData) > m_Size - sizeof(int)) return true;
    memcpy(reinterpret_cast<void *>((val)), m_Index, sizeof(int));
    m_Index += sizeof(int);
    return false;
}

// read the next float
bool DataFile::ReadNextBinary(float *val)
{
    if (size_t(m_Index - m_FileData) > m_Size - sizeof(float)) return true;
    memcpy(reinterpret_cast<void *>((val)), m_Index, sizeof(float));
    m_Index += sizeof(float);
    return false;
}

// read the next double
bool DataFile::ReadNextBinary(double *val)
{
    if (size_t(m_Index - m_FileData) > m_Size - sizeof(double)) return true;
    memcpy(reinterpret_cast<void *>((val)), m_Index, sizeof(double));
    m_Index += sizeof(double);
    return false;
}

// read the next char
bool DataFile::ReadNextBinary(char *val)
{
    if (size_t(m_Index - m_FileData) > m_Size - sizeof(char)) return true;
    *val = *m_Index;
    m_Index += sizeof(char);
    return false;
}

// read the next bool
bool DataFile::ReadNextBinary(bool *val)
{
    if (size_t(m_Index - m_FileData) > m_Size - sizeof(bool)) return true;
    memcpy(reinterpret_cast<void *>((val)), m_Index, sizeof(bool));
    m_Index += sizeof(bool);
    return false;
}

// read the next integer array
bool DataFile::ReadNextBinary(int *val, size_t n)
{
    if (size_t(m_Index - m_FileData) > m_Size - sizeof(int) * n) return true;
    memcpy(reinterpret_cast<void *>((val)), m_Index, sizeof(int) * n);
    m_Index += sizeof(int) * n;
    return false;
}

// read the next float array
bool DataFile::ReadNextBinary(float *val, size_t n)
{
    if (size_t(m_Index - m_FileData) > m_Size - sizeof(float) * n) return true;
    memcpy(reinterpret_cast<void *>((val)), m_Index, sizeof(float) * n);
    m_Index += sizeof(float) * n;
    return false;
}

// read the next double array
bool DataFile::ReadNextBinary(double *val, size_t n)
{
    if (size_t(m_Index - m_FileData) > m_Size - sizeof(double) * n) return true;
    memcpy(reinterpret_cast<void *>((val)), m_Index, sizeof(double) * n);
    m_Index += sizeof(double) * n;
    return false;
}

// read the next character array
bool DataFile::ReadNextBinary(char *val, size_t n)
{
    if (size_t(m_Index - m_FileData) > m_Size - sizeof(char) * n) return true;
    memcpy(reinterpret_cast<void *>((val)), m_Index, sizeof(char) * n);
    m_Index += sizeof(char) * n;
    return false;
}

// read the next bool array
bool DataFile::ReadNextBinary(bool *val, size_t n)
{
    if (size_t(m_Index - m_FileData) > m_Size - sizeof(bool) * n) return true;
    memcpy(reinterpret_cast<void *>((val)), m_Index, sizeof(bool) * n);
    m_Index += sizeof(bool) * n;
    return false;
}

// write an integer parameter
// returns false on success
bool DataFile::WriteParameter(const char * const param, int val)
{
    char buffer[64];

    sprintf(buffer, "%d", val);
    if (WriteParameter(param, buffer)) return true;

    return false;
}


// write a double parameter
// returns false on success
bool DataFile::WriteParameter(const char * const param, double val)
{
    char buffer[64];

    sprintf(buffer, "%.18g", val);
    if (WriteParameter(param, buffer)) return true;

    return false;
}

// write a bool parameter
// returns false on success
bool DataFile::WriteParameter(const char * const param, bool val)
{
    char buffer[64];

    if (val) strcpy(buffer, "true");
    else strcpy(buffer, "false");
    if (WriteParameter(param, buffer)) return true;

    return true;
}

// write a string parameter
// returns false on success
bool DataFile::WriteParameter(const char * const param, const char * const val)
{
    if (WriteNext(param, '\t')) return true;
    if (WriteNext(val, '\n')) return true;

    return false;
}

// write a quoted string parameter
// returns false on success
bool DataFile::WriteQuotedStringParameter(const char * const param, const char * const val)
{
    if (WriteNext(param, '\t')) return true;
    if (WriteNextQuotedString(val, '\n')) return true;

    return false;
}

// write an integer parameter array
// returns false on success
bool DataFile::WriteParameter(const char * const param, size_t n, int *val)
{
    size_t i;
    if (n <= 0) return true;

    if (WriteNext(param, '\t')) return true;

    for (i = 0; i < n - 1; i++)
        if (WriteNext(val[i], '\t')) return true;

    if (WriteNext(val[i], '\n')) return true;
    return false;
}


// write a double parameter array
// returns false on success
bool DataFile::WriteParameter(const char * const param, size_t n, double *val)
{
    size_t i;
    if (n <= 0) return true;

    if (WriteNext(param, '\t')) return true;

    for (i = 0; i < n - 1; i++)
        if (WriteNext(val[i], '\t')) return true;

    if (WriteNext(val[i], '\n')) return true;
    return false;
}

// write a bool parameter array
// returns false on success
bool DataFile::WriteParameter(const char * const param, size_t n, bool *val)
{
    size_t i;
    if (n <= 0) return true;

    if (WriteNext(param, '\t')) return true;

    for (i = 0; i < n - 1; i++)
        if (WriteNext(val[i], '\t')) return true;

    if (WriteNext(val[i], '\n')) return true;
    return false;
}

// write an integer
// returns false on success
bool DataFile::WriteNext(int val, char after)
{
    char buffer[64];

    sprintf(buffer, "%d", val);
    if (WriteNext(buffer, after)) return true;

    return false;
}

// write a double
// returns false on success
bool DataFile::WriteNext(double val, char after)
{
    char buffer[64];

    sprintf(buffer, "%.18g", val);
    if (WriteNext(buffer, after)) return true;

    return false;
}

// write a bool
// returns false on success
bool DataFile::WriteNext(bool val, char after)
{
    char buffer[64];

    if (val) strcpy(buffer, "true");
    else strcpy(buffer, "false");
    if (WriteNext(buffer, after)) return true;

    return true;
}

// write a string
// returns false on success
// note string must be shorter than kStorageIncrement
bool DataFile::WriteNext(const char * const val, char after)
{
    char *p;
    const char *cp;
    bool needQuotes = false;
    size_t size = 0;

    // check for whitespace and measure actual size
    cp = val;
    while (*cp)
    {
        if (*cp < 33) needQuotes = true;
        cp++;
        size++;
    }

    if (m_Index + size + 16 >= m_FileData + m_Size)
    {
        p = new char[m_Size + kStorageIncrement];
        if (p == nullptr)
        {
            if (m_ExitOnErrorFlag)
            {
                std::cerr << "Error: DataFile::WriteNext(" << val
                << ") - could not allocate memory\n";
                exit(1);
            }
            else
            {
                return true;
            }
        }
        memcpy(p, m_FileData, m_Size);
        m_Index = p + (m_Index - m_FileData);
        delete [] m_FileData;
        m_FileData = p;
        m_Size += kStorageIncrement;
    }

    if (needQuotes) *m_Index++ = '"';
    memcpy(m_Index, val, size);
    m_Index += size;
    if (needQuotes) *m_Index++ = '"';
    if (after) *m_Index++ = after;
    *m_Index = 0;

    return false;
}

// write a string
// returns false on success
// note string must be shorter than kStorageIncrement
bool DataFile::WriteNextQuotedString(const char * const val, char after)
{
    char *p;
    const char *cp;
    size_t size = 0;

    // check for whitespace and measure actual size
    cp = val;
    while (*cp)
    {
        cp++;
        size++;
    }

    if (m_Index + size + 16 >= m_FileData + m_Size)
    {
        p = new char[m_Size + kStorageIncrement];
        if (p == nullptr)
        {
            if (m_ExitOnErrorFlag)
            {
                std::cerr << "Error: DataFile::WriteNext(" << val
                << ") - could not allocate memory\n";
                exit(1);
            }
            else
            {
                return true;
            }
        }
        memcpy(p, m_FileData, m_Size);
        m_Index = p + (m_Index - m_FileData);
        delete [] m_FileData;
        m_FileData = p;
        m_Size += kStorageIncrement;
    }

    *m_Index++ = '"';
    memcpy(m_Index, val, size);
    m_Index += size;
    *m_Index++ = '"';
    if (after) *m_Index++ = after;
    *m_Index = 0;

    return false;
}

// strip out beginning and ending whitespace
void DataFile::Strip(char *str)
{
    char *p1, *p2;

    if (*str == 0) return;

    // heading whitespace
    if (*str <= ' ')
    {
        p1 = str;
        while (*p1)
        {
            if (*p1 > ' ') break;
            p1++;
        }
        p2 = str;
        while (*p1)
        {
            *p2 = *p1;
            p1++;
            p2++;
        }
        *p2 = 0;
    }

    if (*str == 0) return;

    // tailing whitespace
    p1 = str;
    while (*p1)
    {
        p1++;
    }
    p1--;
    while (*p1 <= ' ')
    {
        p1--;
    }
    p1++;
    *p1 = 0;

    return;
}

/*  returns true iff str starts with suffix  */
bool DataFile::StringStartsWith(const char * str, const char * suffix)
{
    if (str == nullptr || suffix == nullptr)
        return false;

    size_t suffix_len = strlen(suffix);
    if (strncmp(str, suffix, suffix_len) == 0) return true;
    return false;
}

/*  returns true iff str ends with suffix  */
bool DataFile::StringEndsWith(const char * str, const char * suffix)
{
    if (str == nullptr || suffix == nullptr)
        return false;

    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);

    if (suffix_len > str_len)
        return false;

    if (strncmp(str + str_len - suffix_len, suffix, suffix_len) == 0) return true;
    return false;
}

// more handy statics
double DataFile::Double(const char *buf)
{
    return strtod(buf, nullptr);
}

void DataFile::Double(const char *buf, size_t n, double *d)
{
    char *ptr;
    d[0] = strtod(buf, &ptr);
    for (size_t i = 1; i < n; i++)
        d[i] = strtod(ptr, &ptr);
}

int DataFile::Int(const char *buf)
{
    return strtol(buf, nullptr, 10);
}

void DataFile::Int(const char *buf, size_t n, int *d)
{
    char *ptr;
    d[0] = strtol(buf, &ptr, 10);
    for (size_t i = 1; i < n; i++)
        d[i] = strtol(ptr, &ptr, 10);
}

bool DataFile::Bool(const char *buf)
{
    size_t l = strlen(buf);
    const char *pstart = buf;
    const char *pend = buf + l;
    while (*pstart)
    {
        if (*pstart > 32) break;
        pstart++;
    }
    while (pend > pstart)
    {
        pend--;
        if (*pend > 32) break;
    }
    l = size_t(pend - pstart);
    if (l == 5)
        if (strcasecmp(pstart, "false") == 0) return false;
    if (l == 4)
        if (strcasecmp(pstart, "true") == 0) return true;
    if (strtol(pstart, nullptr, 10) != 0) return true;
    return false;
}

bool DataFile::EndsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}



std::wstring DataFile::ConvertUTF8ToWide(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conv;
    return conv.from_bytes(str);
//    using convert_typeX = std::codecvt_utf8<wchar_t>;
//    std::wstring_convert<convert_typeX, wchar_t> converterX;
//    return converterX.from_bytes(str);
}

std::string DataFile::ConvertWideToUTF8(const std::wstring& wstr)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conv;
    return conv.to_bytes(wstr);
//    using convert_typeX = std::codecvt_utf8<wchar_t>;
//    std::wstring_convert<convert_typeX, wchar_t> converterX;
//    return converterX.to_bytes(wstr);
}



