/*
 *  GSUtil.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Sat Dec 06 2003.
 *  Copyright (c) 2003 Bill Sellers. All rights reserved.
 *
 *  All the routines I can't think of a better place for
 *
 */

#ifndef GSUTIL_H
#define GSUTIL_H

#include "PGDMath.h"

#include "ode/ode.h"

#include <cmath>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <string>
#include <stdint.h>

#if defined(_WIN32) || defined(WIN32)
#define strcasecmp(s1, s2) _stricmp(s1, s2)
#define strncasecmp(s1, s2, n) _strnicmp(s1, s2, n)
#else
#include <strings.h>
#endif

#define THROWIFZERO(a) if (!(a)) throw __LINE__
#define THROWIF(a) if (a) throw __LINE__
#define RETURNLINEIFZERO(a) if (!(a)) return __LINE__
#define RETURNLINEIF(a) if (a) return __LINE__
#define SQUARE(a) ((a) * (a))
#define CUBE(x) ((x)*(x)*(x))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define ABS(a) ((a) >= 0 ? (a) : -(a))
#define ODD(n) ((n) & 1)
#define SWAP(a,b) { (a) = (a)+(b); (b) = (a)-(b); (a) = (a)-(b); }
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define UNUSED(x) ((void)(x))

class GSUtil {
public:

// calculate cross product (vector product)
inline static void CrossProduct3x1(const double *a, const double *b, double *c)
{
        c[0] = a[1] * b[2] - a[2] * b[1];
        c[1] = a[2] * b[0] - a[0] * b[2];
        c[2] = a[0] * b[1] - a[1] * b[0];
}

// calculate dot product (scalar product)
inline static double DotProduct3x1(const double *a, const double *b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

// calculate length of vector
inline static double Magnitude3x1(const double *a)
{
    return sqrt(SQUARE(a[0]) + SQUARE(a[1]) + SQUARE(a[2]));
}

// calculate distance between two points
inline static double Distance3x1(const double *a, const double *b)
{
    return sqrt(SQUARE(a[0] - b[0]) + SQUARE(a[1] - b[1]) + SQUARE(a[2] - b[2]));
}

// calculate unit vector
inline static void Unit3x1(double *a)
{
    double len = sqrt(SQUARE(a[0]) + SQUARE(a[1]) + SQUARE(a[2]));
    // default fixup for zero length vectors
    if (ABS(len) < 1e-30)
    {
        a[0] = 1;
        a[1] = 0;
        a[2] = 0;
    }
    else
    {
        a[0] /= len;
        a[1] /= len;
        a[2] /= len;
    }
}

// c = a + b vectors
inline static void Add3x1(const double *a, const double *b, double *c)
{
    c[0] = a[0] + b[0];
    c[1] = a[1] + b[1];
    c[2] = a[2] + b[2];
}

// c = a - b vectors
inline static void Subtract3x1(const double *a, const double *b, double *c)
{
    c[0] = a[0] - b[0];
    c[1] = a[1] - b[1];
    c[2] = a[2] - b[2];
}

// c = scalar * a
inline static void ScalarMultiply3x1(const double scalar, const double *a, double *c)
{
    c[0] = a[0] * scalar;
    c[1] = a[1] * scalar;
    c[2] = a[2] * scalar;
}

// b = a
inline static void Copy3x1(const double *a, double *b)
{
    b[0] = a[0];
    b[1] = a[1];
    b[2] = a[2];
}


inline static void ZPlaneRotate(double theta, double *location)
{
    double internal[3];

    // get a local copy

    internal[0] = location[0];
    internal[1] = location[1];
    // internal[2] = location[2];

    // rotation code

    double ctheta = cos(theta);
    double stheta = sin(theta);

    // z planar rotation:
    location[0] = internal[0]*ctheta - internal[1]*stheta;
    location[1] = internal[1]*ctheta + internal[0]*stheta;
}

inline static bool OutRange(double v, double l, double h)
{
    if (v < l)
        return true;
    if (v > h)
        return true;
    return false;
}

template <typename T> static T Clamp(const T& n, const T& lower, const T& upper)
{
    if (n < lower) return lower;
    if (n > upper) return upper;
    return n;
}

inline static double Double(const std::string &buf)
{
    return strtod(buf.c_str(), nullptr);
}

inline static double Double(const char *buf)
{
    return strtod(buf, nullptr);
}

inline static double Double(const unsigned char *buf)
{
    return strtod(reinterpret_cast<const char *>(buf), nullptr);
}

inline static double *Double(const char *buf, int n, double *d)
{
    const char *cptr = buf;
    char *ptr;
    for (int i = 0; i < n; i++)
    {
        d[i] = strtod(cptr, &ptr);
        cptr = ptr;
    }
    return d;
}

inline static double *Double(const unsigned char *buf, int n, double *d)
{
    const char *cptr = reinterpret_cast<const char *>(buf);
    char *ptr;
    for (int i = 0; i < n; i++)
    {
        d[i] = strtod(cptr, &ptr);
        cptr = ptr;
    }
    return d;
}

inline static double *Double(const std::string &buf, int n, double *d)
{
    return Double(buf.c_str(), n, d);
}

inline static std::vector<double> *Double(const std::string &buf, std::vector<double> *d)
{
    const char *cptr = buf.data();
    char *ptr = nullptr;
    double v;
    while (true)
    {
        v = strtod(cptr, &ptr);
        if (ptr == cptr) break; // this is the no conversion condition
        cptr = ptr;
        d->push_back(v);
    }
    return d;
}

inline static int Int(const std::string &buf)
{
    return int(strtol(buf.c_str(), nullptr, 0));
}

inline static int Int(const char *buf)
{
    return int(strtol(buf, nullptr, 0));
}

inline static int Int(const unsigned char *buf)
{
    return int(strtol(reinterpret_cast<const char *>(buf), nullptr, 0));
}

inline static int *Int(const char *buf, int n, int *d)
{
    const char *cptr = buf;
    char *ptr;
    for (int i = 0; i < n; i++)
    {
        d[i] = int(strtol(cptr, &ptr, 0));
        cptr = ptr;
    }
    return d;
}

inline static int *Int(unsigned char *buf, int n, int *d)
{
    const char *cptr = reinterpret_cast<const char *>(buf);
    char *ptr;
    for (int i = 0; i < n; i++)
    {
        d[i] = int(strtol(cptr, &ptr, 0));
        cptr = ptr;
    }
    return d;
}

inline static int *Int(const std::string &buf, int n, int *d)
{
    return Int(buf.c_str(), n, d);
}

inline static std::vector<int> *Int(const std::string &buf, std::vector<int> *d)
{
    const char *cptr = buf.data();
    char *ptr = nullptr;
    int v;
    while (true)
    {
        v = int(strtol(cptr, &ptr, 0));
        if (ptr == cptr) break; // this is the no conversion condition
        cptr = ptr;
        d->push_back(v);
    }
    return d;
}

inline static bool Bool(const std::string &buf)
{
    std::vector<char> vbuf(buf.c_str(), buf.c_str() + buf.size() + 1);
    Strip(vbuf.data());
    if (strcasecmp(vbuf.data(), "false") == 0) return false;
    if (strcasecmp(vbuf.data(), "true") == 0) return true;
    if (strtol(vbuf.data(), nullptr, 0) != 0) return true;
    return false;
}

inline static bool Bool(const char *cbuf)
{
    std::vector<char> vbuf(cbuf, cbuf + strlen(cbuf) + 1);
    Strip(vbuf.data());
    if (strcasecmp(vbuf.data(), "false") == 0) return false;
    if (strcasecmp(vbuf.data(), "true") == 0) return true;
    if (strtol(vbuf.data(), nullptr, 0) != 0) return true;
    return false;
}

inline static bool BoolNoStrip(const char *cbuf)
{
    if (strcasecmp(cbuf, "false") == 0) return false;
    if (strcasecmp(cbuf, "true") == 0) return true;
    if (strtol(cbuf, nullptr, 0) != 0) return true;
    return false;
}

inline static bool Bool(const unsigned char *cbuf)
{
    return Bool(reinterpret_cast<const char *>(cbuf));
}

inline static bool *Bool(char *buf, int n, bool *d)
{
    char *token;
    token = strtok(buf, " \t\n\r");
    d[0] = Bool(token);

    for (int i = 0; i < n; i++)
    {
        token = strtok(nullptr, " \t\n\r");
        if (token) d[i] = BoolNoStrip(token);
        else d[i] = false;
    }
    return d;
}

inline static bool *Bool(unsigned char *buf, int n, bool *d)
{
    char *token;
    token = strtok(reinterpret_cast<char *>(buf), " \t\n\r");
    d[0] = Bool(token);

    for (int i = 0; i < n; i++)
    {
        token = strtok(nullptr, " \t\n\r");
        if (token) d[i] = BoolNoStrip(token);
        else d[i] = false;
    }
    return d;
}

inline static bool *Bool(const std::string &buf, int n, bool *d)
{
    std::vector<char> newBuf(buf.c_str(), buf.c_str() + buf.size() + 1);
    return Bool(newBuf.data(), n, d);
}

// strip out beginning and ending whitespace
// Note modifies string
inline static void Strip(char *str)
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

// Count whitespace delimited tokens (tokens surrounded by double quotes are considered a single token)
inline static int CountTokens(const char *string)
{
    const char *p = string;
    bool inToken = false;
    int count = 0;

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
inline static int CountTokens(const unsigned char *string) { return CountTokens(reinterpret_cast<const char *>(string)); }

// linear interpolate using 2 sets of (x,y) coordinates to define the line
inline static double Interpolate(double x1, double y1, double x2, double y2, double x)
{
    // y - y1 = ( (y2 - y1) / (x2 - x1) ) * (x - x1)
    double delX = x2 - x1;
    if (std::fabs(delX) < DBL_EPSILON) return y1;
    double y =  ( ( (y2 - y1) / (delX) ) * (x - x1) ) + y1;
    return y;
}

// return the index of a matching item in a sorted array
template <class T> inline static int BinarySearchMatch
(T array[ ], int listlen, T item)
{
    int first = 0;
    int last = listlen-1;
    int mid;
    while (first <= last)
    {
        mid = (first + last) / 2;
        if (array[mid] < item) first = mid + 1;
        else if (array[mid] > item) last = mid - 1;
        else return mid;
    }

    return -1;
}

// return the index of a matching item in a sorted array
// special case when I'm searching for a range rather than an exact match
// returns the index of array[index] <= item < array[index+1]
template <class T> inline static int BinarySearchRange
(const T array[ ], int listlen, T item)
{
    int first = 0;
    int last = listlen-1;
    int mid;
    while (first <= last)
    {
        mid = (first + last) / 2;
        if (array[mid + 1] <= item) first = mid + 1;
        else if (array[mid] > item) last = mid - 1;
        else return mid;
    }
    return -1;
}

static void EulerDecompositionXYZ(const double *mRot, double& thetaX, double& thetaY, double& thetaZ);
static void EulerDecompositionXZY(const double *mRot, double& thetaX, double& thetaY, double& thetaZ);
static void EulerDecompositionYXZ(const double *mRot, double& thetaX, double& thetaY, double& thetaZ);
static void EulerDecompositionYZX(const double *mRot, double& thetaX, double& thetaY, double& thetaZ);
static void EulerDecompositionZXY(const double *mRot, double& thetaX, double& thetaY, double& thetaZ);
static void EulerDecompositionZYX(const double *mRot, double& thetaX, double& thetaY, double& thetaZ);

static void FindRotation(const double *R1, const double *R2, dMatrix3 rotMat);
static void DumpMatrix(const double *mRot);

static void Tokenizer(const char *constbuf, std::vector<std::string> &tokens, const char *stopList);

static double *GetQuaternion(char *bufPtrs[], double *q);
static pgd::Quaternion GetQuaternion(const std::vector<std::string> &tokens, size_t startIndex);
static double GetAngle(const char *buf);
static double GetAngle(const std::string &buf);

static double DistanceBetweenTwoLines(pgd::Vector3 p1, pgd::Vector3 d1, pgd::Vector3 p2, pgd::Vector3 d2);
static bool LineLineIntersect(pgd::Vector3 p1, pgd::Vector3 p2,
                              pgd::Vector3 p3, pgd::Vector3 p4,
                              pgd::Vector3 *pa, pgd::Vector3 *pb,
                              double *mua, double *mub);

static unsigned char *AsciiToBitMap(const char *string, int width, int height, char setChar, bool reverseY, unsigned char *bitmap);
static void FindAndReplace( std::string *source, const std::string &find, const std::string &replace );
static void FindBoundsCheck(double *list, double x, int *lowBound, int *highBound); // might be quicker than BinarySearchRange for special case
static void FindBounds(double *list, double x, int *lowBound, int *highBound);
static double GetTime();
static int QuickInt(const char *p);
static double QuickDouble(const char *p);
static double QuickPow(double base, int exp);
static bool BoolRegex(const std::string &buf);
static std::string Wrap(const char *text, size_t line_length);

static std::string *ToString(double v, std::string *output);
static std::string *ToString(float v, std::string *output);
static std::string *ToString(int32_t v, std::string *output);
static std::string *ToString(uint32_t v, std::string *output);
static std::string *ToString(int64_t v, std::string *output);
static std::string *ToString(uint64_t v, std::string *output);
static std::string *ToString(bool v, std::string *output);
static std::string *ToString(const double *v, size_t n, std::string *output);
static std::string *ToString(const float *v, size_t n, std::string *output);
static std::string *ToString(const int32_t *v, size_t n, std::string *output);
static std::string *ToString(const uint32_t *v, size_t n, std::string *output);
static std::string *ToString(const int64_t *v, size_t n, std::string *output);
static std::string *ToString(const uint64_t *v, size_t n, std::string *output);
static std::string *ToString(const bool *v, size_t n, std::string *output);
static std::string *ToString(const pgd::Matrix3x3 &m, std::string *output);
static std::string *ToString(const pgd::Quaternion &v, std::string *output);
static std::string *ToString(const pgd::Vector3 &v, std::string *output);
static std::string *ToString(const pgd::Vector2 &v, std::string *output);
static std::string *ToString(uint32_t address, uint16_t port, std::string *output);

static std::string ToString(double v);
static std::string ToString(float v);
static std::string ToString(int32_t v);
static std::string ToString(int64_t v);
static std::string ToString(uint32_t v);
static std::string ToString(uint64_t v);
static std::string ToString(bool v);
static std::string ToString(const double *v, size_t n);
static std::string ToString(const float *v, size_t n);
static std::string ToString(const int32_t *v, size_t n);
static std::string ToString(const uint32_t *v, size_t n);
static std::string ToString(const int64_t *v, size_t n);
static std::string ToString(const uint64_t *v, size_t n);
static std::string ToString(const bool *v, size_t n);
static std::string ToString(const pgd::Matrix3x3 &m);
static std::string ToString(const pgd::Quaternion &v);
static std::string ToString(const pgd::Vector3 &v);
static std::string ToString(uint32_t address, uint16_t port);

static std::string ToString(const char * const printfFormatString, ...);
static std::string ConvertIPAddressToString(uint32_t address, bool networkOrder);

#if defined(__APPLE__)
static std::string *ToString(size_t v, std::string *output);
static std::string *ToString(const size_t *v, size_t n, std::string *output);
static std::string ToString(const size_t *v, size_t n);
#endif

static void BinaryOutput(std::ostream &stream, int8_t v) { stream.write(reinterpret_cast<const char *>(&v), sizeof(v)); }
static void BinaryOutput(std::ostream &stream, uint8_t v) { stream.write(reinterpret_cast<const char *>(&v), sizeof(v)); }
static void BinaryOutput(std::ostream &stream, int16_t v) { stream.write(reinterpret_cast<const char *>(&v), sizeof(v)); }
static void BinaryOutput(std::ostream &stream, uint16_t v) { stream.write(reinterpret_cast<const char *>(&v), sizeof(v)); }
static void BinaryOutput(std::ostream &stream, int32_t v) { stream.write(reinterpret_cast<const char *>(&v), sizeof(v)); }
static void BinaryOutput(std::ostream &stream, uint32_t v) { stream.write(reinterpret_cast<const char *>(&v), sizeof(v)); }
static void BinaryOutput(std::ostream &stream, int64_t v) { stream.write(reinterpret_cast<const char *>(&v), sizeof(v)); }
static void BinaryOutput(std::ostream &stream, uint64_t v) { stream.write(reinterpret_cast<const char *>(&v), sizeof(v)); }
static void BinaryOutput(std::ostream &stream, bool v) { stream.write(reinterpret_cast<const char *>(&v), sizeof(v)); }
static void BinaryOutput(std::ostream &stream, float v) { stream.write(reinterpret_cast<const char *>(&v), sizeof(v)); }
static void BinaryOutput(std::ostream &stream, double v) { stream.write(reinterpret_cast<const char *>(&v), sizeof(v)); }
static void BinaryOutput(std::ostream &stream, const std::string &v) { size_t l = v.size(); stream.write(reinterpret_cast<const char *>(&l), sizeof(l)); stream.write(v.c_str(), std::streamsize(v.size())); }

static size_t SplitExceptQuoted(const std::string &line, std::vector<std::string> *tokens);
static size_t SplitGeneric(const std::string &line, std::vector<std::string> *tokens, char split, bool quoted, bool allowEmpty);

static std::wstring utf8_to_utf16(const std::string& utf8);

static uint8_t fast_a_to_uint8_t(const char *str);
static uint16_t fast_a_to_uint16_t(const char *str);
static uint32_t fast_a_to_uint32_t(const char *str);
static uint64_t fast_a_to_uint64_t(const char *str);
static int8_t fast_a_to_int8_t(const char *str);
static int16_t fast_a_to_int16_t(const char *str);
static int32_t fast_a_to_int32_t(const char *str);
static int64_t fast_a_to_int64_t(const char *str);
static double fast_a_to_double(const char *nptr, const char *endptr[]);
static uint64_t fast_a_to_uint64_t(const char *nptr, const char *endptr[]);

void Logger(const std::string &file, const std::string &message);

static double ThreeAxisDecompositionScore(double x[] , void *data);
static double ThreeAxisDecomposition(const pgd::Quaternion &target, const pgd::Vector3 &ax1, const pgd::Vector3 &ax2, const pgd::Vector3 &ax3, double *ang1, double *ang2, double *ang3);
static double ThreeAxisDecompositionError(const pgd::Quaternion &target, const pgd::Vector3 &ax1, const pgd::Vector3 &ax2, const pgd::Vector3 &ax3, double ang1, double ang2, double ang3);
static void nelmin ( double fn ( double x[] , void *data ), void *data, int n, double start[], double xmin[],
                     double *ynewlo, double reqmin, double step[], int konvge, int kcount,
                     int *icount, int *numres, int *ifault );
static double zeroin(double ax, double bx, double (*f)(double x, void *info), void *info, double tol);

};

#endif                   // GSUTIL_H__
