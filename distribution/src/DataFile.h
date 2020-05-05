/*
 *  DataFile.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 24/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

// DataFile.h - utility class to read in various sorts of data files

#ifndef DataFile_h
#define DataFile_h

#include <stddef.h>
#include <stdint.h>
#include <string>

class DataFile
{
public:
    DataFile();
    virtual ~DataFile();

    // initialise by reading the file
    // this routine allocates a buffer and reads in the whole file
    // filenames are assumed to be UTF-8 and internal conversion happens
    // for Windows where the file system needs wide characters
    bool ReadFile(const std::string &name);
    // write the data out to a file
    bool WriteFile(const std::string &name, bool binary = false);

    // retrieve basic types
    // the file is searched for the parameter and then the next token is read
    bool RetrieveParameter(const char * const param, int *val, bool searchFromStart = true);
    bool RetrieveParameter(const char * const param, unsigned int *val, bool searchFromStart = true);
    bool RetrieveParameter(const char * const param, double *val, bool searchFromStart = true);
    bool RetrieveParameter(const char * const param, bool *val, bool searchFromStart = true);
    bool RetrieveParameter(const char * const param, char *val, size_t size, bool searchFromStart = true);
    bool RetrieveParameter(const char * const param, char **val, size_t *size, bool searchFromStart = true);
    bool RetrieveParameter(const char * const param, std::string *val, bool searchFromStart = true);
    bool RetrieveQuotedStringParameter(const char * const param, char *val, size_t size, bool searchFromStart = true);
    bool RetrieveQuotedStringParameter(const char * const param, char **val, size_t *size, bool searchFromStart = true);
    bool RetrieveQuotedStringParameter(const char * const param, std::string *val, bool searchFromStart = true);

    // ranged functions
    // the file is searched for the parameter and then the next token is read
    bool RetrieveRangedParameter(const char * const param, double *val, bool searchFromStart = true);
    void SetRangeControl(double r) { m_RangeControl = r; }

    // line reading functions
    // optional comment character and can ignore empty lines
    bool ReadNextLine(char *line, size_t size, bool ignoreEmpty, char commentChar = 0, char continuationChar = 0);
    bool ReadNextLine2(char *line, size_t size, bool ignoreEmpty, const char *commentString, const char *continuationString = nullptr);

    // retrieve arrays
    // the file is searched for the parameter and then the next token is read
    bool RetrieveParameter(const char * const param, size_t n, int *val, bool searchFromStart = true);
    bool RetrieveParameter(const char * const param, size_t n, double *val, bool searchFromStart = true);
    bool RetrieveRangedParameter(const char * const param, size_t n, double *val, bool searchFromStart = true);

    // utility settings
    void SetExitOnError(bool flag) { m_ExitOnErrorFlag = flag; }
    bool GetExitOnError() { return m_ExitOnErrorFlag; }
    char *GetRawData() { return m_FileData; }
    void SetRawData(const char *string, size_t stringLen);
    void ResetIndex() { m_Index = m_FileData; }
    size_t GetSize() { return m_Size; }
    void ClearData();
    size_t Replace(const char *oldString, const char *newString);
    char *GetIndex() { return m_Index; }
    void SetIndex(char *index) { m_Index = index; }
    std::string GetPathName() { return m_PathName; }

    // probably mostly for internal use
    // read the next ASCII token from the current index and bump index
    bool FindParameter(const char * const param, bool searchFromStart = true);
    bool ReadNext(int *val);
    bool ReadNext(double *val);
    bool ReadNext(char *val, size_t size);
    bool ReadNext(char **val, size_t *size);
    bool ReadNext(std::string *val);
    bool ReadNextQuotedString(char *val, size_t size);
    bool ReadNextQuotedString(char **val, size_t *size);
    bool ReadNextQuotedString(std::string *val);
    bool ReadNextLine(char *line, size_t size);
    bool ReadNextRanged(double *val);

    // handy statics
    static bool EndOfLineTest(char **p);
    static size_t CountTokens(const char *string);
    static size_t ReturnTokens(char *string, char *ptrs[], size_t size);
    static size_t CountTokens(const char *string, const char *separators);
    static size_t ReturnTokens(char *string, char *ptrs[], size_t size, const char *separators);
    static size_t CountLines(const char *string);
    static size_t ReturnLines(char *string, char *ptrs[], size_t size);
    static void Strip(char *str);
    static bool StringEndsWith(const char * str, const char * suffix);
    static bool StringStartsWith(const char * str, const char * suffix);
    static double Double(const char *buf);
    static void Double(const char *buf, size_t n, double *d);
    static int Int(const char *buf);
    static void Int(const char *buf, size_t n, int *d);
    static bool Bool(const char *buf);
    static bool EndsWith(const char *str, const char *suffix);

    static std::wstring ConvertUTF8ToWide(const std::string& str);
    static std::string ConvertWideToUTF8(const std::wstring& wstr);

    // binary file operators
    // read the next binary value from current index and bump index
    bool ReadNextBinary(int *val);
    bool ReadNextBinary(float *val);
    bool ReadNextBinary(double *val);
    bool ReadNextBinary(char *val);
    bool ReadNextBinary(bool *val);
    bool ReadNextBinary(int *val, size_t n);
    bool ReadNextBinary(float *val, size_t n);
    bool ReadNextBinary(double *val, size_t n);
    bool ReadNextBinary(char *val, size_t n);
    bool ReadNextBinary(bool *val, size_t n);

    // file writing operators
    bool WriteParameter(const char * const param, int val);
    bool WriteParameter(const char * const param, double val);
    bool WriteParameter(const char * const param, bool val);
    bool WriteParameter(const char * const param, const char * const val);
    bool WriteQuotedStringParameter(const char * const param, const char * const val);
    bool WriteParameter(const char * const param, size_t n, int *val);
    bool WriteParameter(const char * const param, size_t n, double *val);
    bool WriteParameter(const char * const param, size_t n, bool *val);
    bool WriteNext(int val, char after = 0);
    bool WriteNext(double val, char after = 0);
    bool WriteNext(bool val, char after = 0);
    bool WriteNext(const char * const val, char after = 0);
    bool WriteNextQuotedString(const char * const val, char after = 0);

private:

    char *m_FileData = nullptr;
    char *m_Index = nullptr;
    bool m_ExitOnErrorFlag = false;
    double m_RangeControl = false;
    size_t m_Size = 0;
    std::string m_PathName;
#if defined(_WIN32) || defined(WIN32)
    // provide Windows specific wchar versions
    bool ReadFile(const std::wstring &name);
    bool WriteFile(const std::wstring &name, bool binary = false);
    std::wstring GetWPathName() { return m_WPathName; }
    std::wstring m_WPathName;
#endif
};

#endif

