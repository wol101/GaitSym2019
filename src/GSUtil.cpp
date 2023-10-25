/*
 *  GSUtil.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 12/07/2007.
 *  Copyright 2007 Bill Sellers. All rights reserved.
 *
 */

#if defined(_WIN32) || defined(WIN32)
#include <Windows.h>
#endif

#include "GSUtil.h"
#include "PGDMath.h"

#include "fast_double_parser.h"

#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <stdint.h>
#include <limits>
#include <regex>
#include <cinttypes>
#include <cstdarg>
#include <chrono>

#if !defined(_WIN32) && !defined(WIN32)
#include <sys/time.h>
#include <arpa/inet.h>
#endif

using namespace std::string_literals;

// Euler decomposition examples from Geometric Tools for Computer Graphics, Scheider & Eberly 2003

// matrix format is:
// [r00 r01 r02]
// [r10 r11 r12]
// [r20 r21 r22]

// ode format definition
#define r00 mRot[0*4 + 0]
#define r01 mRot[0*4 + 1]
#define r02 mRot[0*4 + 2]
#define r10 mRot[1*4 + 0]
#define r11 mRot[1*4 + 1]
#define r12 mRot[1*4 + 2]
#define r20 mRot[2*4 + 0]
#define r21 mRot[2*4 + 1]
#define r22 mRot[2*4 + 2]

//#define e11 mRot[0*4 + 0]
//#define e12 mRot[0*4 + 1]
//#define e13 mRot[0*4 + 2]
//#define e21 mRot[1*4 + 0]
//#define e22 mRot[1*4 + 1]
//#define e23 mRot[1*4 + 2]
//#define e31 mRot[2*4 + 0]
//#define e32 mRot[2*4 + 1]
//#define e33 mRot[2*4 + 2]

// these are intrinsic Euler angles (actually Tait-Bryan or Cardan angles according to Wikipedia)
// to do the extrinsic versions you just reverse the XYZ (so intrinsic XYZ = extrinsic ZYX)
// extrinsic is where the rotation axes are fixed
void GSUtil::EulerDecompositionXYZ(const double *mRot, double& thetaX, double& thetaY, double& thetaZ)
{
    thetaY = asin(r02);
    if ( thetaY < M_PI/2 )
    {
        if ( thetaY > -M_PI/2 )
        {
            thetaX = atan2(-r12,r22);
            thetaZ = atan2(-r01,r00);
        }
        else
        {
            // not a unique solution
            thetaX = -atan2(r10,r11);
            thetaZ = 0;
        }
    }
    else
    {
        // not a unique solution
        thetaX = atan2(r10,r11);
        thetaZ = 0;
    }
}

void GSUtil::EulerDecompositionXZY(const double *mRot, double& thetaX, double& thetaY, double& thetaZ)
{
    thetaZ = asin(-r01);
    if ( thetaZ < M_PI/2 )
    {
        if ( thetaZ > -M_PI/2 )
        {
            thetaX = atan2(r21,r11);
            thetaY = atan2(r02,r00);
        }
        else
        {
            // not a unique solution
            thetaX = -atan2(-r20,r22);
            thetaY = 0;
        }
    }
    else
    {
        // not a unique solution
        thetaX = atan2(-r20,r22);
        thetaY = 0;
    }
}

void GSUtil::EulerDecompositionYXZ(const double *mRot, double& thetaX, double& thetaY, double& thetaZ)
{
    thetaX = asin(-r12);
    if ( thetaX < M_PI/2 )
    {
        if ( thetaX > -M_PI/2 )
        {
            thetaY = atan2(r02,r22);
            thetaZ = atan2(r10,r11);
        }
        else
        {
            // not a unique solution
            thetaY = -atan2(-r01,r00);
            thetaZ = 0;
        }
    }
    else
    {
        // not a unique solution
        thetaY = atan2(-r01,r00);
        thetaZ = 0;
    }
}

void GSUtil::EulerDecompositionYZX(const double *mRot, double& thetaX, double& thetaY, double& thetaZ)
{
    thetaZ = asin(r10);
    if ( thetaZ < M_PI/2 )
    {
        if ( thetaZ > -M_PI/2 )
        {
            thetaY = atan2(-r20,r00);
            thetaX = atan2(-r12,r11);
        }
        else
        {
            // not a unique solution
            thetaY = -atan2(r21,r22);
            thetaX = 0;
        }
    }
    else
    {
        // not a unique solution
        thetaY = atan2(r21,r22);
        thetaX = 0;
    }
}

void GSUtil::EulerDecompositionZXY(const double *mRot, double& thetaX, double& thetaY, double& thetaZ)
{
    thetaX = asin(r21);
    if ( thetaX < M_PI/2 )
    {
        if ( thetaX > -M_PI/2 )
        {
            thetaZ = atan2(-r01,r11);
            thetaY = atan2(-r20,r22);
        }
        else
        {
            // not a unique solution
            thetaZ = -atan2(r02,r00);
            thetaY = 0;
        }
    }
    else
    {
        // not a unique solution
        thetaZ = atan2(r02,r00);
        thetaY = 0;
    }
}

void GSUtil::EulerDecompositionZYX(const double *mRot, double& thetaX, double& thetaY, double& thetaZ)
{
    thetaY = asin(-r20);
    if ( thetaY < M_PI/2 )
    {
        if ( thetaY > -M_PI/2 )
        {
            thetaZ = atan2(r10,r00);
            thetaX = atan2(r21,r22);
        }
        else
        {
            // not a unique solution
            thetaZ = -atan2(-r01,r02);
            thetaX = 0;
        }
    }
    else
    {
        // not a unique solution
        thetaZ = atan2(-r01,r02);
        thetaX = 0;
    }
}


// finds the rotation matrix that will transform R1 to R2
// where R1 and R2 are both rotation matrices
void GSUtil::FindRotation(const double *R1, const double *R2, dMatrix3 rotMat)
{
    // theory:
    // X.R1 = R2
    // X.R1.R1' = R2.R1'
    // X = R2.R1'
    // and that's what dMultiply2 does
    // as does dMULTIPLY2_333 which is quicker

    dMultiply2_333(rotMat, R2, R1); // optimised version

}

// matrix format is:
// [r00 r01 r02]
// [r10 r11 r12]
// [r20 r21 r22]
void GSUtil::DumpMatrix(const double *mRot)
{
    fprintf(stderr, "[%9.4f %9.4f %9.4f]\n", r00, r01, r02);
    fprintf(stderr, "[%9.4f %9.4f %9.4f]\n", r10, r11, r12);
    fprintf(stderr, "[%9.4f %9.4f %9.4f]\n", r20, r21, r22);
    fprintf(stderr, "\n");
}

void GSUtil::Tokenizer(const char *constbuf, std::vector<std::string> &tokens, const char *stopList)
{
    char *localBuf = reinterpret_cast<char *>(malloc(strlen(constbuf) + 1));
    char *ptr = localBuf;
    strcpy(ptr, constbuf);
    char *qp;
    char byte;
    char oneChar[2] = {0, 0};
    if (stopList == nullptr) stopList = "{};,=:&|!()+-/*[]'<>^";

     while (*ptr)
     {
         // find non-whitespace
         if (*ptr < 33)
         {
             ptr++;
             continue;
         }

         // is it in stoplist
         if (strchr(stopList, *ptr))
         {
             oneChar[0] = *ptr;
             tokens.push_back(oneChar);
             ptr++;
             continue;
         }

         // is it a double quote?
         if (*ptr == '"')
         {
             ptr++;
             qp = strchr(ptr, '"');
             if (qp)
             {
                 *qp = 0;
                 tokens.push_back(ptr);
                 *qp = '"';
                 ptr = qp + 1;
                 continue;
             }
         }

         qp = ptr;
         while (*qp >= 33 && strchr(stopList, *qp) == nullptr && *qp != '"')
         {
             qp++;
         }
         byte = *qp;
         *qp = 0;
         tokens.push_back(ptr);
         if (byte == 0) break;
         *qp = byte;
         ptr = qp;
     }

     free(localBuf);
}

// function to identify and if necessary convert (angle, axisx, axisy, axiz) representations
// to (qs,qx,qy,qz) quaternions
// angle is identified by a postscript r for radians and d for degrees
// no postscript means that the value is already a quaternion
double *GSUtil::GetQuaternion(char *bufPtrs[], double *q)
{
    int i;
    for (i = 0; i < 4; i++) q[i] = strtod(bufPtrs[i], nullptr);

    char *p;
    p = bufPtrs[0];
    while (*p) p++;
    p--; // pointing to last character of the string

    if (*p == 'r') // radian angle axis
    {
        pgd::Quaternion qq = pgd::MakeQFromAxisAngle(q[1], q[2], q[3], q[0]);
        q[0] = qq.n;
        q[1] = qq.x;
        q[2] = qq.y;
        q[3] = qq.z;
        return q;
    }
    if (*p == 'd') // degee angle axis
    {
        pgd::Quaternion qq = pgd::MakeQFromAxisAngle(q[1], q[2], q[3], pgd::DegreesToRadians(q[0]));
        q[0] = qq.n;
        q[1] = qq.x;
        q[2] = qq.y;
        q[3] = qq.z;
        return q;
    }
    return q;
}

pgd::Quaternion GSUtil::GetQuaternion(const std::vector<std::string> &tokens, size_t startIndex)
{
    double q[4];
    for (size_t i = 0; i < 4; i++) q[i] = strtod(tokens[startIndex + i].c_str(), nullptr);

    char p = tokens[startIndex].back();
    if (p == 'r') // radian angle axis
    {
        pgd::Quaternion qq = pgd::MakeQFromAxisAngle(q[1], q[2], q[3], q[0]);
        return qq;
    }
    if (p == 'd') // degee angle axis
    {
        pgd::Quaternion qq = pgd::MakeQFromAxisAngle(q[1], q[2], q[3], pgd::DegreesToRadians(q[0]));
        return qq;
    }
    return pgd::Quaternion(q[0], q[1], q[2], q[3]);
}

// function to return an angle from a string
// angle is identified by a postscript r for radians and d for degrees
// no postscript means that the value is already in radians
double GSUtil::GetAngle(const char *buf)
{
    double angle = strtod(buf, nullptr);

    const char *p;
    p = buf;
    while (*p) p++;
    p--; // pointing to last character of the string

    if (*p == 'r') // radian angle
    {
        return angle;
    }
    if (*p == 'd') // degee angle
    {
        return angle * M_PI / 180;
    }
    return angle;
}

// function to return an angle from a string
// angle is identified by a postscript r for radians and d for degrees
// no postscript means that the value is already in radians
double GSUtil::GetAngle(const std::string &buf)
{
    double angle = strtod(buf.c_str(), nullptr);

    char p = buf.back();
    if (p == 'r') // radian angle
    {
        return angle;
    }
    if (p == 'd') // degee angle
    {
        return angle * M_PI / 180;
    }
    return angle;
}


// function to return the distance between two lines
// p1 - point on line 1
// d1 - direction of line 1
// p2 - point on line 2
// d2 - direction of line 2
double GSUtil::DistanceBetweenTwoLines(pgd::Vector3 p1, pgd::Vector3 d1, pgd::Vector3 p2, pgd::Vector3 d2)
{
    // first find ther perpendicular to the two vectors
    pgd::Vector3 perpendicular = d1 ^ d2;
    perpendicular.Normalize();

    // now find a vector from l1 to l2
    pgd::Vector3 link = p2 - p1;

    // shortest distance is the component of this vector in the direction of the normal
    double distance = link * perpendicular;

    return distance;
}

/*
   Calculate the line segment PaPb that is the shortest route between
   two lines P1P2 and P3P4. Calculate also the values of mua and mub where
      Pa = P1 + mua (P2 - P1)
      Pb = P3 + mub (P4 - P3)
   Return false if no solution exists.
*/
bool GSUtil::LineLineIntersect(pgd::Vector3 p1, pgd::Vector3 p2,
                            pgd::Vector3 p3, pgd::Vector3 p4,
                            pgd::Vector3 *pa, pgd::Vector3 *pb,
                            double *mua, double *mub)
{
    pgd::Vector3 p13,p43,p21;
    double d1343,d4321,d1321,d4343,d2121;
    double numer,denom;
    const double EPS = DBL_EPSILON;

    p13.x = p1.x - p3.x;
    p13.y = p1.y - p3.y;
    p13.z = p1.z - p3.z;
    p43.x = p4.x - p3.x;
    p43.y = p4.y - p3.y;
    p43.z = p4.z - p3.z;
    if (ABS(p43.x)  < EPS && ABS(p43.y)  < EPS && ABS(p43.z)  < EPS)
        return(false);
    p21.x = p2.x - p1.x;
    p21.y = p2.y - p1.y;
    p21.z = p2.z - p1.z;
    if (ABS(p21.x)  < EPS && ABS(p21.y)  < EPS && ABS(p21.z)  < EPS)
        return(false);

    d1343 = p13.x * p43.x + p13.y * p43.y + p13.z * p43.z;
    d4321 = p43.x * p21.x + p43.y * p21.y + p43.z * p21.z;
    d1321 = p13.x * p21.x + p13.y * p21.y + p13.z * p21.z;
    d4343 = p43.x * p43.x + p43.y * p43.y + p43.z * p43.z;
    d2121 = p21.x * p21.x + p21.y * p21.y + p21.z * p21.z;

    denom = d2121 * d4343 - d4321 * d4321;
    if (ABS(denom) < EPS)
        return(false);
    numer = d1343 * d4321 - d1321 * d4343;

    *mua = numer / denom;
    *mub = (d1343 + d4321 * (*mua)) / d4343;

    pa->x = p1.x + *mua * p21.x;
    pa->y = p1.y + *mua * p21.y;
    pa->z = p1.z + *mua * p21.z;
    pb->x = p3.x + *mub * p43.x;
    pb->y = p3.y + *mub * p43.y;
    pb->z = p3.z + *mub * p43.z;

    return(true);
}

// this routine converts a text string to a bitmap
// the text string ignores whitespace and the zero character
// is defined. Anything not whitespace and not the zero character
// is a set bit. It returns the address of the character array
// of 0 and 1. Optionally the Y can be reversed since by default the
// origin is the top left and often the bottom right is what is wanted
unsigned char *GSUtil::AsciiToBitMap(const char *string, int width, int height, char setChar, bool reverseY, unsigned char *bitmap)
{
    const char *pin = string;
    int outputLen = width * height;
    unsigned char *pout;
    int j;

    // memset(bitmap, 3, outputLen);

    if (reverseY == false)
    {
        pout = bitmap;
        while (outputLen > 0)
        {
            if (*pin > 32)
            {
                if (*pin == setChar) *pout = 1;
                else *pout = 0;
                pout++;
                outputLen--;
            }
            pin++;
        }
    }
    else
    {
        pout = bitmap + outputLen - width;
        j = 0;
        while (outputLen > 0)
        {
            if (*pin > 32)
            {
                if (*pin == setChar) *pout = 1;
                else *pout = 0;
                pout++;
                j++;
                if (j >= width)
                {
                    j = 0;
                    pout -= (2 * width);
                }
                outputLen--;
            }
            pin++;
        }
    }

#if 0
    for (int i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            std::cerr << int(bitmap[j + i * height]);
        }
        std::cerr << "\n";
    }
#endif

    return bitmap;
}

void GSUtil::FindAndReplace( std::string *source, const std::string &find, const std::string &replace )
{
    std::string::size_type j;
    while ((j = source->find( find )) != std::string::npos)
    {
        source->replace( j, find.length(), replace );
    }
}

// finds the indices of the members in a sorted list that bracket a particular value
// intially *lowBound should equal zero, and *highBound should equal the length of the list
void GSUtil::FindBoundsCheck(double *list, double x, int *lowBound, int *highBound)
{
    if (x < list[*lowBound])
    {
        *lowBound = *lowBound - 1;
        *highBound = *lowBound;
        return;
    }
    if (x > list[*highBound])
    {
        *lowBound = *highBound;
        *highBound = *highBound + 1;
        return;
    }
    FindBounds(list, x, lowBound, highBound);
}

void GSUtil::FindBounds(double *list, double x, int *lowBound, int *highBound)
{
    if ((*highBound - *lowBound) <= 1) return; // end condition
    int pivotIndex = (*lowBound + *highBound) / 2;
    double pivot = list[pivotIndex];
    double low = list[*lowBound];
    double high = list[*highBound];
    if (x >= low && x <= pivot) *highBound = pivotIndex;
    else if (x >= pivot && x <= high) *lowBound = pivotIndex;
    FindBounds(list, x, lowBound, highBound);
}

double GSUtil::GetTime()
{
#if defined(_WIN32) || defined(WIN32)
    FILETIME ft;
    uint64_t tmpres = 0; // this is count in 100 nanoseconds intervals
    GetSystemTimeAsFileTime(&ft);

    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    return double(tmpres) / 10000000.0;
#else
    struct timeval tv ;
    gettimeofday(&tv, 0);
    return ((double)tv.tv_sec) + ((double)tv.tv_usec) / 1000000.0;
#endif
}

int GSUtil::QuickInt(const char *p)
{
    int x = 0;
    bool neg = false;
    if (*p == '-')
    {
        neg = true;
        ++p;
    }
    while (*p >= '0' && *p <= '9')
    {
        x = (x*10) + (*p - '0');
        ++p;
    }
    if (neg)
    {
        x = -x;
    }
    return x;
}

double GSUtil::QuickDouble(const char *p)
{
    //double t = atof(p);
    double r = 0.0;
    bool neg = false;
    if (*p == '-')
    {
        neg = true;
        ++p;
    }
    while (*p >= '0' && *p <= '9')
    {
        r = (r*10.0) + (*p - '0');
        ++p;
    }
    if (*p == '.')
    {
        double f = 0.0;
        int n = 0;
        ++p;
        while (*p >= '0' && *p <= '9')
        {
            f = (f*10.0) + (*p - '0');
            ++p;
            ++n;
        }
        r += f / QuickPow(10.0, n);
    }
    if (*p == 'e' || *p == 'E')
    {
        ++p;
        int m = QuickInt(p);
        r *= QuickPow(10.0, m);
    }
    if (neg)
    {
        r = -r;
    }
    //if (fabs(t - r) > 0.01)
    //    std::cerr << "t = " << t << " r = " << r << "\n";
    return r;
}

double GSUtil::QuickPow(double base, int exp)
{
    if (exp >= 0)
    {
        double result = 1;
        while (exp)
        {
            if (exp & 1)
                result *= base;
            exp >>= 1;
            base *= base;
        }

        return result;
    }
    else
    {
        exp = -exp;
        double result = 1;
        while (exp)
        {
            if (exp & 1)
                result *= base;
            exp >>= 1;
            base *= base;
        }

        return 1.0 / result;
    }
}

std::string *GSUtil::ToString(double v, std::string *output)
{
    char buf[32];
    int l = snprintf(buf, sizeof(buf), "%.18g", v); // 17 digits is enough to round trip but 18 is safe
    output->assign(buf, size_t(l));
    return output;
}

std::string *GSUtil::ToString(float v, std::string *output)
{
    char buf[32];
    int l = snprintf(buf, sizeof(buf), "%.10g", double(v)); // 9 is enough to round trip but 10 is safe, the double() is to silence warnings
    output->assign(buf, size_t(l));
    return output;
}

std::string *GSUtil::ToString(int32_t v, std::string *output)
{
    char buf[32];
    int l = snprintf(buf, sizeof(buf), "%d", v);
    output->assign(buf, size_t(l));
    return output;
}

std::string *GSUtil::ToString(uint32_t v, std::string *output)
{
    char buf[32];
    int l = snprintf(buf, sizeof(buf), "%u", v);
    output->assign(buf, size_t(l));
    return output;
}

std::string *GSUtil::ToString(int64_t v, std::string *output)
{
    char buf[32];
    int l = snprintf(buf, sizeof(buf), "%" PRId64, v);
    output->assign(buf, size_t(l));
    return output;
}

std::string *GSUtil::ToString(uint64_t v, std::string *output)
{
    char buf[32];
    int l = snprintf(buf, sizeof(buf), "%" PRIu64, v);
    output->assign(buf, size_t(l));
    return output;
}

std::string *GSUtil::ToString(bool v, std::string *output)
{
    if (v) output->assign("true"s);
    else output->assign("false"s);
    return output;
}

std::string *GSUtil::ToString(const double *v, size_t n, std::string *output)
{
    std::vector<char> buf(32 * n);
    size_t count = 0;
    int l;
    for (size_t i = 0; i < n; i++)
    {
        if (i < n - 1) l = snprintf(&buf[count], 32, "%.18g ", v[i]);
        else l = snprintf(&buf[count], 32, "%.18g", v[i]);
        count += size_t(l);
    }
    output->assign(buf.data(), count);
    return output;
}

std::string *GSUtil::ToString(const float *v, size_t n, std::string *output)
{
    std::vector<char> buf(32 * n);
    size_t count = 0;
    int l;
    for (size_t i = 0; i < n; i++)
    {
        if (i < n - 1) l = snprintf(&buf[count], 32, "%.10g ", double(v[i]));
        else l = snprintf(&buf[count], 32, "%.10g", double(v[i]));
        count += size_t(l);
    }
    output->assign(buf.data(), count);
    return output;
}

std::string *GSUtil::ToString(const int32_t *v, size_t n, std::string *output)
{
    std::vector<char> buf(32 * n);
    size_t count = 0;
    int l;
    for (size_t i = 0; i < n; i++)
    {
        if (i < n - 1) l = snprintf(&buf[count], 32, "%d ", v[i]);
        else l = snprintf(&buf[count], 32, "%d", v[i]);
        count += size_t(l);
    }
    output->assign(buf.data(), count);
    return output;
}

std::string *GSUtil::ToString(const uint32_t *v, size_t n, std::string *output)
{
    std::vector<char> buf(32 * n);
    size_t count = 0;
    int l;
    for (size_t i = 0; i < n; i++)
    {
        if (i < n - 1) l = snprintf(&buf[count], 32, "%u ", v[i]);
        else l = snprintf(&buf[count], 32, "%u", v[i]);
        count += size_t(l);
    }
    output->assign(buf.data(), count);
    return output;
}

std::string *GSUtil::ToString(const int64_t *v, size_t n, std::string *output)
{
    std::vector<char> buf(32 * n);
    size_t count = 0;
    int l;
    for (size_t i = 0; i < n; i++)
    {
        if (i < n - 1) l = snprintf(&buf[count], 32, "%" PRId64 " ", v[i]);
        else l = snprintf(&buf[count], 32, "%" PRId64, v[i]);
        count += size_t(l);
    }
    output->assign(buf.data(), count);
    return output;
}

std::string *GSUtil::ToString(const uint64_t *v, size_t n, std::string *output)
{
    std::vector<char> buf(32 * n);
    size_t count = 0;
    int l;
    for (size_t i = 0; i < n; i++)
    {
        if (i < n - 1) l = snprintf(&buf[count], 32, "%" PRIu64 " ", v[i]);
        else l = snprintf(&buf[count], 32, "%" PRIu64, v[i]);
        count += size_t(l);
    }
    output->assign(buf.data(), count);
    return output;
}

std::string *GSUtil::ToString(const bool *v, size_t n, std::string *output)
{
    std::vector<char> buf(32 * n);
    size_t count = 0;
    int l;
    for (size_t i = 0; i < n; i++)
    {
        if (i < n - 1) { if (v[i]) l = snprintf(&buf[count], 32, "true "); else l = snprintf(&buf[count], 32, "false "); }
        else { if (v[i]) l = snprintf(&buf[count], 32, "true"); else l = snprintf(&buf[count], 32, "false"); }
        count += size_t(l);
    }
    output->assign(buf.data(), count);
    return output;
}

std::string *GSUtil::ToString(const pgd::Quaternion &v, std::string *output)
{
    char buf[32 * 4];
    // note quaternion is (qs,qx,qy,qz)
    int l = snprintf(buf, sizeof(buf), "%.18g %.18g %.18g %.18g", v.n, v.x, v.y, v.z); // 17 digits is enough to round trip
    output->assign(buf, size_t(l));
    return output;
}

std::string *GSUtil::ToString(const pgd::Matrix3x3 &m, std::string *output)
{
    char buf[32 * 9];
    // note quaternion is (qs,qx,qy,qz)
    int l = snprintf(buf, sizeof(buf), "%.18g %.18g %.18g\n%.18g %.18g %.18g\n%.18g %.18g %.18g",
                     m.e11, m.e12, m.e13,
                     m.e21, m.e22, m.e23,
                     m.e31, m.e32, m.e33); // 17 digits is enough to round trip
    output->assign(buf, size_t(l));
    return output;
}

std::string *GSUtil::ToString(const pgd::Vector3 &v, std::string *output)
{
    char buf[32 * 3];
    int l = snprintf(buf, sizeof(buf), "%.18g %.18g %.18g", v.x, v.y, v.z); // 17 digits is enough to round trip
    output->assign(buf, size_t(l));
    return output;
}

std::string *GSUtil::ToString(const pgd::Vector2 &v, std::string *output)
{
    char buf[32 * 2];
    int l = snprintf(buf, sizeof(buf), "%.18g %.18g", v.x, v.y); // 17 digits is enough to round trip
    output->assign(buf, size_t(l));
    return output;
}

std::string *GSUtil::ToString(uint32_t address, uint16_t port, std::string *output)
{
    *output = (std::to_string(address & 0xff) + "."s +
               std::to_string((address >> 8) & 0xff) + "."s +
               std::to_string((address >> 16) & 0xff) + "."s +
               std::to_string(address >> 24) +":"s +
               std::to_string(port));
    return output;
}

std::string GSUtil::ToString(double v)
{
    std::string output;
    return *ToString(v, &output);
}

std::string GSUtil::ToString(float v)
{
    std::string output;
    return *ToString(v, &output);
}

std::string GSUtil::ToString(int32_t v)
{
    std::string output;
    return *ToString(v, &output);
}

std::string GSUtil::ToString(int64_t v)
{
    std::string output;
    return *ToString(v, &output);
}

std::string GSUtil::ToString(uint32_t v)
{
    std::string output;
    return *ToString(v, &output);
}

std::string GSUtil::ToString(uint64_t v)
{
    std::string output;
    return *ToString(v, &output);
}

std::string GSUtil::ToString(bool v)
{
    std::string output;
    return *ToString(v, &output);
}

std::string GSUtil::ToString(const double *v, size_t n)
{
    std::string output;
    return *ToString(v, n, &output);
}

std::string GSUtil::ToString(const float *v, size_t n)
{
    std::string output;
    return *ToString(v, n, &output);
}

std::string GSUtil::ToString(const int32_t *v, size_t n)
{
    std::string output;
    return *ToString(v, n, &output);
}

std::string GSUtil::ToString(const uint32_t *v, size_t n)
{
    std::string output;
    return *ToString(v, n, &output);
}

std::string GSUtil::ToString(const int64_t *v, size_t n)
{
    std::string output;
    return *ToString(v, n, &output);
}

std::string GSUtil::ToString(const uint64_t *v, size_t n)
{
    std::string output;
    return *ToString(v, n, &output);
}

std::string GSUtil::ToString(const bool *v, size_t n)
{
    std::string output;
    return *ToString(v, n, &output);
}

std::string GSUtil::ToString(const pgd::Matrix3x3 &m)
{
    std::string output;
    return *ToString(m, &output);
}

std::string GSUtil::ToString(const pgd::Quaternion &v)
{
    std::string output;
    return *ToString(v, &output);
}

std::string GSUtil::ToString(const pgd::Vector3 &v)
{
    std::string output;
    return *ToString(v, &output);
}

std::string GSUtil::ToString(uint32_t address, uint16_t port)
{
    std::string output;
    return *ToString(address, port, &output);
}

// convert to string using printf style formatting and variable numbers of arguments
std::string GSUtil::ToString(const char * const printfFormatString, ...)
{
    // initialize use of the variable argument array
    va_list vaArgs;
    va_start(vaArgs, printfFormatString);

    // reliably acquire the size
    // from a copy of the variable argument array
    // and a functionally reliable call to mock the formatting
    va_list vaArgsCopy;
    va_copy(vaArgsCopy, vaArgs);
    const int iLen = std::vsnprintf(NULL, 0, printfFormatString, vaArgsCopy);
    va_end(vaArgsCopy);

    // return a formatted string without risking memory mismanagement
    // and without assuming any compiler or platform specific behavior
    std::unique_ptr<char[]> zc = std::make_unique<char[]>(iLen + 1);
    std::vsnprintf(zc.get(), iLen + 1, printfFormatString, vaArgs);
    va_end(vaArgs);
    return std::string(zc.get(), iLen);
}

std::string GSUtil::ConvertIPAddressToString(uint32_t address, bool networkOrder)
{
    if (networkOrder)
    {
        address = ntohl(address);
    }
    std::string hostURL = (std::to_string(address & 0xff) + "."s +
                           std::to_string((address >> 8) & 0xff) + "."s +
                           std::to_string((address >> 16) & 0xff) + "."s +
                           std::to_string(address >> 24));
    return hostURL;
}

#if defined(__APPLE__)
std::string *GSUtil::ToString(size_t v, std::string *output)
{
    return ToString(uint64_t(v), output);
}

std::string *GSUtil::ToString(const size_t *v, size_t n, std::string *output)
{
    std::vector<char> buf(32 * n);
    size_t count = 0;
    int l;
    for (size_t i = 0; i < n; i++)
    {
        if (i < n - 1) l = snprintf(&buf[count], 32, "%" PRIu64 " ", uint64_t(v[i]));
        else l = snprintf(&buf[count], 32, "%" PRIu64, uint64_t(v[i]));
        count += size_t(l);
    }
    output->assign(buf.data(), count);
    return output;
}

std::string GSUtil::ToString(const size_t *v, size_t n)
{
    std::string output;
    return *ToString(v, n, &output);
}
#endif

bool GSUtil::BoolRegex(const std::string &buf)
{
    const std::regex true_regex("^\\s*(true|yes)\\s*$"s, std::regex_constants::icase);
    const std::regex false_regex("^\\s*(false|no)\\s*$"s, std::regex_constants::icase);
    if (std::regex_match(buf, true_regex)) return true;
    if (std::regex_match(buf, false_regex)) return false;
    if (std::strtol(buf.c_str(), nullptr, 0) != 0) return true;
    return false;
}

// wraps a string at white space to a maximum of line length
std::string GSUtil::Wrap(const char *text, size_t line_length)
{
    std::istringstream words(text);
    std::ostringstream wrapped;
    std::string word;

    if (words >> word) {
        wrapped << word;
        size_t space_left = line_length - word.length();
        while (words >> word) {
            if (space_left < word.length() + 1) {
                wrapped << '\n' << word;
                space_left = line_length - word.length();
            } else {
                wrapped << ' ' << word;
                space_left -= word.length() + 1;
            }
        }
    }
    return wrapped.str();
}

size_t GSUtil::SplitExceptQuoted(const std::string &line, std::vector<std::string> *tokens)
{
    tokens->clear();
    std::regex rgx("( |\\\".*?\\\"|'.*?')"); // this regular expression splits on spaces unless contained within quotes
    // The -1 is the key here: when the iterator is constructed the iterator points at the text that precedes the match
    // and after each increment the iterator points at the text that followed the previous match.
    std::sregex_token_iterator iter(line.begin(), line.end(), rgx, -1);
    std::sregex_token_iterator end;
    for ( ; iter != end; ++iter)
        tokens->push_back(*iter);
    return tokens->size();
}

// generic split
// if split is 0 then split on any whitespace
// otherwise split on this character
// strings can be optionally quoted to allow the split character in the string
size_t GSUtil::SplitGeneric(const std::string &line, std::vector<std::string> *tokens, char split, bool quoted, bool allowEmpty)
{
    tokens->clear();
    if (!quoted && split)
    {
        size_t startIndex = 0;
        size_t i = 0;
        for (; i < line.size(); i++)
        {
            if (line[i] == split || (split == 0 && line[i] < 33))
            {
                if (allowEmpty)
                {
                    tokens->push_back(line.substr(startIndex, i - startIndex));
                }
                else
                {
                    if (i - startIndex > 0) tokens->push_back(line.substr(startIndex, i - startIndex));
                }
                startIndex = i + 1;
            }
        }
        if (i > startIndex) tokens->push_back(line.substr(startIndex, i - startIndex));
    }
    else
    {
        bool inQuote = false;
        size_t startIndex = 0;
        size_t i = 0;
        for (; i < line.size(); i++)
        {
            if (inQuote)
            {
                if (line[i] == '"')
                {
                    inQuote = false;
                    if (allowEmpty)
                    {
                        tokens->push_back(line.substr(startIndex, i - startIndex));
                    }
                    else
                    {
                        if (i - startIndex > 0) tokens->push_back(line.substr(startIndex, i - startIndex));
                    }
                    i++; // skips the next character which must be a separator
                }
                continue;
            }
            if (line[i] == '"')
            {
                startIndex = i + 1;
                inQuote = true;
                continue;
            }
            if (line[i] == split || (split == 0 && line[i] < 33))
            {
                if (allowEmpty)
                {
                    tokens->push_back(line.substr(startIndex, i - startIndex));
                }
                else
                {
                    if (i - startIndex > 0) tokens->push_back(line.substr(startIndex, i - startIndex));
                }
                startIndex = i + 1;
            }
        }
        if (i > startIndex) tokens->push_back(line.substr(startIndex, i - startIndex));
    }
    return tokens->size();
}

std::wstring GSUtil::utf8_to_utf16(const std::string& utf8)
{
    std::vector<uint32_t> unicode;
    size_t i = 0;
    while (i < utf8.size())
    {
        uint32_t uni;
        size_t todo;
        unsigned char ch = static_cast<unsigned char>(utf8[i++]);
        if (ch <= 0x7F)
        {
            uni = ch;
            todo = 0;
        }
        else if (ch <= 0xBF)
        {
            throw std::logic_error("not a UTF-8 string");
        }
        else if (ch <= 0xDF)
        {
            uni = ch&0x1F;
            todo = 1;
        }
        else if (ch <= 0xEF)
        {
            uni = ch&0x0F;
            todo = 2;
        }
        else if (ch <= 0xF7)
        {
            uni = ch&0x07;
            todo = 3;
        }
        else
        {
            throw std::logic_error("not a UTF-8 string");
        }
        for (size_t j = 0; j < todo; ++j)
        {
            if (i == utf8.size())
                throw std::logic_error("not a UTF-8 string");
            ch = static_cast<unsigned char>(utf8[i++]);
            if (ch < 0x80 || ch > 0xBF)
                throw std::logic_error("not a UTF-8 string");
            uni <<= 6;
            uni += ch & 0x3F;
        }
        if (uni >= 0xD800 && uni <= 0xDFFF)
            throw std::logic_error("not a UTF-8 string");
        if (uni > 0x10FFFF)
            throw std::logic_error("not a UTF-8 string");
        unicode.push_back(uni);
    }
    std::wstring utf16;
    for (i = 0; i < unicode.size(); ++i)
    {
        uint32_t uni = unicode[i];
        if (uni <= 0xFFFF)
        {
            utf16 += wchar_t(uni);
        }
        else
        {
            uni -= 0x10000;
            utf16 += wchar_t(((uni >> 10) + 0xD800));
            utf16 += wchar_t(((uni & 0x3FF) + 0xDC00));
        }
    }
    return utf16;
}

uint8_t GSUtil::fast_a_to_uint8_t(const char *str)
{
    uint8_t val = 0;
    while(*str) { val = val*10 + (*str++ - '0'); }
    return val;
}

uint16_t GSUtil::fast_a_to_uint16_t(const char *str)
{
    uint16_t val = 0;
    while(*str) { val = val*10 + (*str++ - '0'); }
    return val;
}

uint32_t GSUtil::fast_a_to_uint32_t(const char *str)
{
    uint32_t val = 0;
    while(*str) { val = val*10 + (*str++ - '0'); }
    return val;
}

uint64_t GSUtil::fast_a_to_uint64_t(const char *str)
{
    uint64_t val = 0;
    while(*str) { val = val*10 + (*str++ - '0'); }
    return val;
}

int8_t GSUtil::fast_a_to_int8_t(const char *str)
{
    int8_t neg = 1;
    if (*str == '-') { neg = -1; ++str; }
    int8_t val = 0;
    while(*str) { val = val*10 + (*str++ - '0'); }
    return val * neg;
}

int16_t GSUtil::fast_a_to_int16_t(const char *str)
{
    int16_t neg = 1;
    if (*str == '-') { neg = -1; ++str; }
    int16_t val = 0;
    while(*str) { val = val*10 + (*str++ - '0'); }
    return val * neg;
}

int32_t GSUtil::fast_a_to_int32_t(const char *str)
{
    int32_t neg = 1;
    if (*str == '-') { neg = -1; ++str; }
    int32_t val = 0;
    while(*str) { val = val*10 + (*str++ - '0'); }
    return val * neg;
}

int64_t GSUtil::fast_a_to_int64_t(const char *str)
{
    int64_t neg = 1;
    if (*str == '-') { neg = -1; ++str; }
    int64_t val = 0;
    while(*str) { val = val*10 + (*str++ - '0'); }
    return val * neg;
}

double GSUtil::fast_a_to_double(const char *nptr, const char *endptr[])
{
    double x;
    while (*nptr <= ' ') ++nptr;
    *endptr = fast_double_parser::parse_number(nptr, &x);
    return x;
}

uint64_t GSUtil::fast_a_to_uint64_t(const char *nptr, const char *endptr[])
{
    uint64_t val = 0;
    while (*nptr <= ' ') ++nptr;
    while(*nptr >= '0' && *nptr <= '9') { val = val*10 + (*nptr++ - '0'); }
    *endptr = nptr;
    return val;
}

double GSUtil::ThreeAxisDecompositionScore(double x[] , void *data)
{
    double *ptr = reinterpret_cast<double *>(data);
    pgd::Quaternion target(ptr[0], ptr[1], ptr[2], ptr[3]);
    pgd::Vector3 ax1(ptr[4], ptr[5], ptr[6]);
    pgd::Vector3 ax2(ptr[7], ptr[8], ptr[9]);
    pgd::Vector3 ax3(ptr[10], ptr[11], ptr[12]);
    double ang1 = x[0];
    double ang2 = x[1];
    double ang3 = x[2];
    double error = GSUtil::ThreeAxisDecompositionError(target, ax1, ax2, ax3, ang1, ang2, ang3);
    return error;
}

void GSUtil::Logger(const std::string &file, const std::string &message)
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    try
    {
        std::ofstream logFile;
        logFile.exceptions(std::ios::failbit|std::ios::badbit);
        logFile.open(file.c_str(), std::ios_base::out | std::ios_base::app);
        logFile << std::put_time(std::localtime(&t), "%c") << '\t' << message << '\n'; // consider std::format as a replcement for std::put_time in C++20 since std::localtime is not thread safe
        logFile.close();
    }
    catch (...)
    {
        std::cerr << "Error writing log file\n";
    }
}

double GSUtil::ThreeAxisDecompositionError(const pgd::Quaternion &target, const pgd::Vector3 &ax1, const pgd::Vector3 &ax2, const pgd::Vector3 &ax3, double ang1, double ang2, double ang3)
{
    pgd::Quaternion product = pgd::MakeQFromAxisAngle(ax3, ang3, true) * pgd::MakeQFromAxisAngle(ax2, ang2, true) * pgd::MakeQFromAxisAngle(ax1, ang1, true);
    // now we need a distance metric between product and target. I could just calculate the angle between them:
    // theta = acos(2(q1,q2)^2-1) where (q1,q2) is the inner product (n1n2 + x1x2+ y1y2 + z1z2)
    // but there are other quantities that will do a similar job in less time
    // 1-(q1,q2)^2 should be faster to calculate and is 0 when they are identical and 1 when they are 180 degrees apart
    double innerProduct = target.n * product.n + target.x * product.x + target.y * product.y + target.z * product.z;
    double error = 1 - innerProduct * innerProduct;
    return error;
}

double GSUtil::ThreeAxisDecomposition(const pgd::Quaternion &target, const pgd::Vector3 &ax1, const pgd::Vector3 &ax2, const pgd::Vector3 &ax3, double *ang1, double *ang2, double *ang3)
{
    // we use simplex search to try and solve this

    double userData[13];
    userData[0] = target.n; userData[1] = target.x; userData[2] = target.y; userData[3] = target.z;
    userData[4] = ax1.x; userData[5] = ax1.y; userData[6] = ax1.z;
    userData[7] = ax2.x; userData[8] = ax2.y; userData[9] = ax2.z;
    userData[10] = ax3.x; userData[11] = ax3.y; userData[2] = ax3.z;

    const int N = 3;
    double start[N] = {1, 1, 1};
    double xmin[N] = {0, 0, 0};
    double ynewlo = 0;
    double reqmin = 1.0e-10;
    double step[N] = {1.0, 1.0, 1.0};
    int konvge = 10;
    int kcount = 1000;
    int icount = 0;
    int numres = 0;
    int ifault = 0;
    nelmin (ThreeAxisDecompositionScore, userData, N, start, xmin, &ynewlo, reqmin, step, konvge, kcount, &icount, &numres, &ifault );

    *ang1 = xmin[0];
    *ang2 = xmin[1];
    *ang3 = xmin[2];
    return ynewlo;
}

// wis - void *data added to the original code to allow arbitrary data to be passed to the function
//****************************************************************************80

void GSUtil::nelmin ( double fn ( double x[] , void *data ), void *data, int n, double start[], double xmin[],
                      double *ynewlo, double reqmin, double step[], int konvge, int kcount,
                      int *icount, int *numres, int *ifault )

//****************************************************************************80
//
//  Purpose:
//
//    NELMIN minimizes a function using the Nelder-Mead algorithm.
//
//  Discussion:
//
//    This routine seeks the minimum value of a user-specified function.
//
//    Simplex function minimisation procedure due to Nelder+Mead(1965),
//    as implemented by O'Neill(1971, Appl.Statist. 20, 338-45), with
//    subsequent comments by Chambers+Ertel(1974, 23, 250-1), Benyon(1976,
//    25, 97) and Hill(1978, 27, 380-2)
//
//    The function to be minimized must be defined by a function of
//    the form
//
//      function fn ( x, f )
//      double fn
//      double x(*)
//
//    and the name of this subroutine must be declared EXTERNAL in the
//    calling routine and passed as the argument FN.
//
//    This routine does not include a termination test using the
//    fitting of a quadratic surface.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    27 February 2008
//
//  Author:
//
//    Original FORTRAN77 version by R ONeill.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    John Nelder, Roger Mead,
//    A simplex method for function minimization,
//    Computer Journal,
//    Volume 7, 1965, pages 308-313.
//
//    R ONeill,
//    Algorithm AS 47:
//    Function Minimization Using a Simplex Procedure,
//    Applied Statistics,
//    Volume 20, Number 3, 1971, pages 338-345.
//
//  Parameters:
//
//    Input, double FN ( double x[] ), the name of the routine which evaluates
//    the function to be minimized.
//
//    Input, int N, the number of variables.
//
//    Input/output, double START[N].  On input, a starting point
//    for the iteration.  On output, this data may have been overwritten.
//
//    Output, double XMIN[N], the coordinates of the point which
//    is estimated to minimize the function.
//
//    Output, double YNEWLO, the minimum value of the function.
//
//    Input, double REQMIN, the terminating limit for the variance
//    of function values.
//
//    Input, double STEP[N], determines the size and shape of the
//    initial simplex.  The relative magnitudes of its elements should reflect
//    the units of the variables.
//
//    Input, int KONVGE, the convergence check is carried out
//    every KONVGE iterations.
//
//    Input, int KCOUNT, the maximum number of function
//    evaluations.
//
//    Output, int *ICOUNT, the number of function evaluations
//    used.
//
//    Output, int *NUMRES, the number of restarts.
//
//    Output, int *IFAULT, error indicator.
//    0, no errors detected.
//    1, REQMIN, N, or KONVGE has an illegal value.
//    2, iteration terminated because KCOUNT was exceeded without convergence.
//
{
    double ccoeff = 0.5;
    double del;
    double dn;
    double dnn;
    double ecoeff = 2.0;
    double eps = 0.001;
    int i;
    int ihi;
    int ilo;
    int j;
    int jcount;
    int l;
    int nn;
    double rcoeff = 1.0;
    double rq;
    double x;
    double y2star;
    double ylo;
    double ystar;
    double z;
    //
    //  Check the input parameters.
    //
    if ( reqmin <= 0.0 )
    {
        *ifault = 1;
        return;
    }

    if ( n < 1 )
    {
        *ifault = 1;
        return;
    }

    if ( konvge < 1 )
    {
        *ifault = 1;
        return;
    }

    std::unique_ptr<double []> p = std::make_unique<double []>(size_t(n*(n+1)));
    std::unique_ptr<double []> pstar = std::make_unique<double []>(size_t(n));
    std::unique_ptr<double []> p2star = std::make_unique<double []>(size_t(n));
    std::unique_ptr<double []> pbar = std::make_unique<double []>(size_t(n));
    std::unique_ptr<double []> y = std::make_unique<double []>(size_t(n+1));

    *icount = 0;
    *numres = 0;

    jcount = konvge;
    dn = double ( ( n ) );
    nn = n + 1;
    dnn = double ( ( nn ) );
    del = 1.0;
    rq = reqmin * dn;
    //
    //  Initial or restarted loop.
    //
    for ( ; ; )
    {
        for ( i = 0; i < n; i++ )
        {
            p[size_t(i+n*n)] = start[size_t(i)];
        }
        y[size_t(n)] = fn ( start , data );
        *icount = *icount + 1;

        for ( j = 0; j < n; j++ )
        {
            x = start[size_t(j)];
            start[size_t(j)] = start[size_t(j)] + step[size_t(j)] * del;
            for ( i = 0; i < n; i++ )
            {
                p[size_t(i+j*n)] = start[size_t(i)];
            }
            y[size_t(j)] = fn ( start , data );
            *icount = *icount + 1;
            start[size_t(j)] = x;
        }
        //
        //  The simplex construction is complete.
        //
        //  Find highest and lowest Y values.  YNEWLO = Y(IHI) indicates
        //  the vertex of the simplex to be replaced.
        //
        ylo = y[size_t(0)];
        ilo = 0;

        for ( i = 1; i < nn; i++ )
        {
            if ( y[size_t(i)] < ylo )
            {
                ylo = y[size_t(i)];
                ilo = i;
            }
        }
        //
        //  Inner loop.
        //
        for ( ; ; )
        {
            if ( kcount <= *icount )
            {
                break;
            }
            *ynewlo = y[size_t(0)];
            ihi = 0;

            for ( i = 1; i < nn; i++ )
            {
                if ( *ynewlo < y[size_t(i)] )
                {
                    *ynewlo = y[size_t(i)];
                    ihi = i;
                }
            }
            //
            //  Calculate PBAR, the centroid of the simplex vertices
            //  excepting the vertex with Y value YNEWLO.
            //
            for ( i = 0; i < n; i++ )
            {
                z = 0.0;
                for ( j = 0; j < nn; j++ )
                {
                    z = z + p[size_t(i+j*n)];
                }
                z = z - p[size_t(i+ihi*n)];
                pbar[size_t(i)] = z / dn;
            }
            //
            //  Reflection through the centroid.
            //
            for ( i = 0; i < n; i++ )
            {
                pstar[size_t(i)] = pbar[size_t(i)] + rcoeff * ( pbar[size_t(i)] - p[size_t(i+ihi*n)] );
            }
            ystar = fn ( pstar.get() , data );
            *icount = *icount + 1;
            //
            //  Successful reflection, so extension.
            //
            if ( ystar < ylo )
            {
                for ( i = 0; i < n; i++ )
                {
                    p2star[size_t(i)] = pbar[size_t(i)] + ecoeff * ( pstar[size_t(i)] - pbar[size_t(i)] );
                }
                y2star = fn ( p2star.get() , data );
                *icount = *icount + 1;
                //
                //  Check extension.
                //
                if ( ystar < y2star )
                {
                    for ( i = 0; i < n; i++ )
                    {
                        p[size_t(i+ihi*n)] = pstar[size_t(i)];
                    }
                    y[size_t(ihi)] = ystar;
                }
                //
                //  Retain extension or contraction.
                //
                else
                {
                    for ( i = 0; i < n; i++ )
                    {
                        p[size_t(i+ihi*n)] = p2star[size_t(i)];
                    }
                    y[size_t(ihi)] = y2star;
                }
            }
            //
            //  No extension.
            //
            else
            {
                l = 0;
                for ( i = 0; i < nn; i++ )
                {
                    if ( ystar < y[size_t(i)] )
                    {
                        l = l + 1;
                    }
                }

                if ( 1 < l )
                {
                    for ( i = 0; i < n; i++ )
                    {
                        p[size_t(i+ihi*n)] = pstar[size_t(i)];
                    }
                    y[size_t(ihi)] = ystar;
                }
                //
                //  Contraction on the Y(IHI) side of the centroid.
                //
                else if ( l == 0 )
                {
                    for ( i = 0; i < n; i++ )
                    {
                        p2star[size_t(i)] = pbar[size_t(i)] + ccoeff * ( p[size_t(i+ihi*n)] - pbar[size_t(i)] );
                    }
                    y2star = fn ( p2star.get() , data );
                    *icount = *icount + 1;
                    //
                    //  Contract the whole simplex.
                    //
                    if ( y[size_t(ihi)] < y2star )
                    {
                        for ( j = 0; j < nn; j++ )
                        {
                            for ( i = 0; i < n; i++ )
                            {
                                p[size_t(i+j*n)] = ( p[size_t(i+j*n)] + p[size_t(i+ilo*n)] ) * 0.5;
                                xmin[size_t(i)] = p[size_t(i+j*n)];
                            }
                            y[size_t(j)] = fn ( xmin , data );
                            *icount = *icount + 1;
                        }
                        ylo = y[size_t(0)];
                        ilo = 0;

                        for ( i = 1; i < nn; i++ )
                        {
                            if ( y[size_t(i)] < ylo )
                            {
                                ylo = y[size_t(i)];
                                ilo = i;
                            }
                        }
                        continue;
                    }
                    //
                    //  Retain contraction.
                    //
                    else
                    {
                        for ( i = 0; i < n; i++ )
                        {
                            p[size_t(i+ihi*n)] = p2star[size_t(i)];
                        }
                        y[size_t(ihi)] = y2star;
                    }
                }
                //
                //  Contraction on the reflection side of the centroid.
                //
                else if ( l == 1 )
                {
                    for ( i = 0; i < n; i++ )
                    {
                        p2star[size_t(i)] = pbar[size_t(i)] + ccoeff * ( pstar[size_t(i)] - pbar[size_t(i)] );
                    }
                    y2star = fn ( p2star.get() , data );
                    *icount = *icount + 1;
                    //
                    //  Retain reflection?
                    //
                    if ( y2star <= ystar )
                    {
                        for ( i = 0; i < n; i++ )
                        {
                            p[size_t(i+ihi*n)] = p2star[size_t(i)];
                        }
                        y[size_t(ihi)] = y2star;
                    }
                    else
                    {
                        for ( i = 0; i < n; i++ )
                        {
                            p[size_t(i+ihi*n)] = pstar[size_t(i)];
                        }
                        y[size_t(ihi)] = ystar;
                    }
                }
            }
            //
            //  Check if YLO improved.
            //
            if ( y[size_t(ihi)] < ylo )
            {
                ylo = y[size_t(ihi)];
                ilo = ihi;
            }
            jcount = jcount - 1;

            if ( 0 < jcount )
            {
                continue;
            }
            //
            //  Check to see if minimum reached.
            //
            if ( *icount <= kcount )
            {
                jcount = konvge;

                z = 0.0;
                for ( i = 0; i < nn; i++ )
                {
                    z = z + y[size_t(i)];
                }
                x = z / dnn;

                z = 0.0;
                for ( i = 0; i < nn; i++ )
                {
                    z = z + pow ( y[size_t(i)] - x, 2 );
                }

                if ( z <= rq )
                {
                    break;
                }
            }
        }
        //
        //  Factorial tests to check that YNEWLO is a local minimum.
        //
        for ( i = 0; i < n; i++ )
        {
            xmin[size_t(i)] = p[size_t(i+ilo*n)];
        }
        *ynewlo = y[size_t(ilo)];

        if ( kcount < *icount )
        {
            *ifault = 2;
            break;
        }

        *ifault = 0;

        for ( i = 0; i < n; i++ )
        {
            del = step[size_t(i)] * eps;
            xmin[size_t(i)] = xmin[size_t(i)] + del;
            z = fn ( xmin , data );
            *icount = *icount + 1;
            if ( z < *ynewlo )
            {
                *ifault = 2;
                break;
            }
            xmin[size_t(i)] = xmin[size_t(i)] - del - del;
            z = fn ( xmin , data );
            *icount = *icount + 1;
            if ( z < *ynewlo )
            {
                *ifault = 2;
                break;
            }
            xmin[size_t(i)] = xmin[size_t(i)] + del;
        }

        if ( *ifault == 0 )
        {
            break;
        }
        //
        //  Restart the procedure.
        //
        for ( i = 0; i < n; i++ )
        {
            start[size_t(i)] = xmin[size_t(i)];
        }
        del = eps;
        *numres = *numres + 1;
    }

    return;
}

#ifdef FORTRAN_DERIVED_ZEROIN

/* netlib function zeroin original fortran
      double precision function zeroin(ax,bx,f,tol)
      double precision ax,bx,f,tol
c
c      a zero of the function  f(x)  is computed in the interval ax,bx .
c
c  input..
c
c  ax     left endpoint of initial interval
c  bx     right endpoint of initial interval
c  f      function subprogram which evaluates f(x) for any x in
c         the interval  ax,bx
c  tol    desired length of the interval of uncertainty of the
c         final result (.ge.0.)
c
c  output..
c
c  zeroin abscissa approximating a zero of  f  in the interval ax,bx
c
c      it is assumed  that   f(ax)   and   f(bx)   have  opposite  signs
c  this is checked, and an error message is printed if this is not
c  satisfied.   zeroin  returns a zero  x  in the given interval
c  ax,bx  to within a tolerance  4*macheps*abs(x)+tol, where macheps  is
c  the  relative machine precision defined as the smallest representable
c  number such that  1.+macheps .gt. 1.
c      this function subprogram is a slightly  modified  translation  of
c  the algol 60 procedure  zero  given in  richard brent, algorithms for
c  minimization without derivatives, prentice-hall, inc. (1973).
c
      double precision  a,b,c,d,e,eps,fa,fb,fc,tol1,xm,p,q,r,s
      double precision  dabs, d1mach
   10 eps = d1mach(4)
      tol1 = eps+1.0d0
c
      a=ax
      b=bx
      fa=f(a)
      fb=f(b)
c     check that f(ax) and f(bx) have different signs
      if (fa .eq.0.0d0 .or. fb .eq. 0.0d0) go to 20
      if (fa * (fb/dabs(fb)) .le. 0.0d0) go to 20
         write(6,2500)
2500     format(1x,'f(ax) and f(bx) do not have different signs,',
     1             ' zeroin is aborting')
         return
   20 c=a
      fc=fa
      d=b-a
      e=d
   30 if (dabs(fc).ge.dabs(fb)) go to 40
      a=b
      b=c
      c=a
      fa=fb
      fb=fc
      fc=fa
   40 tol1=2.0d0*eps*dabs(b)+0.5d0*tol
      xm = 0.5d0*(c-b)
      if ((dabs(xm).le.tol1).or.(fb.eq.0.0d0)) go to 150
c
c see if a bisection is forced
c
      if ((dabs(e).ge.tol1).and.(dabs(fa).gt.dabs(fb))) go to 50
      d=xm
      e=d
      go to 110
   50 s=fb/fa
      if (a.ne.c) go to 60
c
c linear interpolation
c
      p=2.0d0*xm*s
      q=1.0d0-s
      go to 70
c
c inverse quadratic interpolation
c
   60 q=fa/fc
      r=fb/fc
      p=s*(2.0d0*xm*q*(q-r)-(b-a)*(r-1.0d0))
      q=(q-1.0d0)*(r-1.0d0)*(s-1.0d0)
   70 if (p.le.0.0d0) go to 80
      q=-q
      go to 90
   80 p=-p
   90 s=e
      e=d
      if (((2.0d0*p).ge.(3.0d0*xm*q-dabs(tol1*q))).or.(p.ge.
     *dabs(0.5d0*s*q))) go to 100
      d=p/q
      go to 110
  100 d=xm
      e=d
  110 a=b
      fa=fb
      if (dabs(d).le.tol1) go to 120
      b=b+d
      go to 140
  120 if (xm.le.0.0d0) go to 130
      b=b+tol1
      go to 140
  130 b=b-tol1
  140 fb=f(b)
      if ((fb*(fc/dabs(fc))).gt.0.0d0) go to 20
      go to 30
  150 zeroin=b
      return
      end
*/

/* Standard C source for D1MACH */
/*
#include <stdio.h>
#include <cfloat>
#include <cmath>
double d1mach_(long *i)
{
    switch(*i){
      case 1: return DBL_MIN;
      case 2: return DBL_MAX;
      case 3: return DBL_EPSILON/FLT_RADIX;
      case 4: return DBL_EPSILON;
      case 5: return log10((double)FLT_RADIX);
      }
    fprintf(stderr, "invalid argument: d1mach(%ld)\n", *i);
    exit(1); return 0;
} */

// zeroin.f -- translated by f2c (version 20031025).
// and subsequently edited to improve readability slightly

double zeroin(double *ax, double *bx, double (*f)(double x, void *info), void *info, double *tol)
{
    /* System generated locals */
    double ret_val = 0, d__1, d__2;

    /* Local variables */
    double a, b, c__, d__, e, p, q, r__, s, fa, fb, fc, xm, eps, tol1;

    /*      a zero of the function  f(x)  is computed in the interval ax,bx . */

    /*  input.. */

    /*  ax     left endpoint of initial interval */
    /*  bx     right endpoint of initial interval */
    /*  f      function subprogram which evaluates f(x) for any x in */
    /*         the interval  ax,bx */
    /*  tol    desired length of the interval of uncertainty of the */
    /*         final result (>= 0.) */

    /*  output.. */

    /*  zeroin abscissa approximating a zero of  f  in the interval ax,bx */

    /*      it is assumed  that   f(ax)   and   f(bx)   have  opposite  signs */
    /*  this is checked, and an error message is printed if this is not */
    /*  satisfied.   zeroin  returns a zero  x  in the given interval */
    /*  ax,bx  to within a tolerance  4*macheps*fabs(x)+tol, where macheps  is */
    /*  the  relative machine precision defined as the smallest representable */
    /*  number such that  1.+macheps > 1. */
    /*      this function subprogram is a slightly  modified  translation  of */
    /*  the algol 60 procedure  zero  given in  richard brent, algorithms for */
    /*  minimization without derivatives, prentice-hall, inc. (1973). */

    /* L10: */
    eps = DBL_EPSILON;
    tol1 = eps + 1.;

    a = *ax;
    b = *bx;
    fa = (*f)(a, info);
    fb = (*f)(b, info);
    /*     check that f(ax) and f(bx) have different signs */
    if (fa == 0. || fb == 0.) {
        goto L20;
    }
    if (fa * (fb / fabs(fb)) <= 0.) {
        goto L20;
    }
    fprintf(stderr, "f(ax) and f(bx) do not have different signs, zeroin is aborting\n");
    return ret_val;
L20:
    c__ = a;
    fc = fa;
    d__ = b - a;
    e = d__;
L30:
    if (fabs(fc) >= fabs(fb)) {
        goto L40;
    }
    a = b;
    b = c__;
    c__ = a;
    fa = fb;
    fb = fc;
    fc = fa;
L40:
    tol1 = eps * 2. * fabs(b) + *tol * .5;
    xm = (c__ - b) * .5;
    if (fabs(xm) <= tol1 || fb == 0.) {
        goto L150;
    }

    /* see if a bisection is forced */

    if (fabs(e) >= tol1 && fabs(fa) > fabs(fb)) {
        goto L50;
    }
    d__ = xm;
    e = d__;
    goto L110;
L50:
    s = fb / fa;
    if (a != c__) {
        goto L60;
    }

    /* linear interpolation */

    p = xm * 2. * s;
    q = 1. - s;
    goto L70;

    /* inverse quadratic interpolation */

L60:
    q = fa / fc;
    r__ = fb / fc;
    p = s * (xm * 2. * q * (q - r__) - (b - a) * (r__ - 1.));
    q = (q - 1.) * (r__ - 1.) * (s - 1.);
L70:
    if (p <= 0.) {
        goto L80;
    }
    q = -q;
    goto L90;
L80:
    p = -p;
L90:
    s = e;
    e = d__;
    if (p * 2. >= xm * 3. * q - (d__1 = tol1 * q, fabs(d__1)) || p >= (d__2 =
                                                                       s * .5 * q, fabs(d__2))) {
        goto L100;
    }
    d__ = p / q;
    goto L110;
L100:
    d__ = xm;
    e = d__;
L110:
    a = b;
    fa = fb;
    if (fabs(d__) <= tol1) {
        goto L120;
    }
    b += d__;
    goto L140;
L120:
    if (xm <= 0.) {
        goto L130;
    }
    b += tol1;
    goto L140;
L130:
    b -= tol1;
L140:
    fb = (*f)(b, info);
    if (fb * (fc / fabs(fc)) > 0.) {
        goto L20;
    }
    goto L30;
L150:
    ret_val = b;
    return ret_val;
} /* zeroin_ */

#else

/*
 ************************************************************************
 *                  C math library
 * function ZEROIN - obtain a function zero within the given range
 *
 * Input
 *  double zeroin(ax,bx,f,tol)
 *  double ax;          Root will be seeked for within
 *  double bx;              a range [ax,bx]
 *  double (*f)(double x);      Name of the function whose zero
 *                  will be seeked for
 *  double tol;         Acceptable tolerance for the root
 *                  value.
 *                  May be specified as 0.0 to cause
 *                  the program to find the root as
 *                  accurate as possible
 *
 * Output
 *  Zeroin returns an estimate for the root with accuracy
 *  4*EPSILON*abs(x) + tol
 *
 * Algorithm
 *  G.Forsythe, M.Malcolm, C.Moler, Computer methods for mathematical
 *  computations. M., Mir, 1980, p.180 of the Russian edition
 *
 *  The function makes use of the bissection procedure combined with
 *  the linear or quadric inverse interpolation.
 *  At every step program operates on three abscissae - a, b, and c.
 *  b - the last and the best approximation to the root
 *  a - the last but one approximation
 *  c - the last but one or even earlier approximation than a that
 *      1) |f(b)| <= |f(c)|
 *      2) f(b) and f(c) have opposite signs, i.e. b and c confine
 *         the root
 *  At every step Zeroin selects one of the two new approximations, the
 *  former being obtained by the bissection procedure and the latter
 *  resulting in the interpolation (if a,b, and c are all different
 *  the quadric interpolation is utilized, otherwise the linear one).
 *  If the latter (i.e. obtained by the interpolation) point is
 *  reasonable (i.e. lies within the current interval [b,c] not being
 *  too close to the boundaries) it is accepted. The bissection result
 *  is used in the other case. Therefore, the range of uncertainty is
 *  ensured to be reduced at least by the factor 1.6
 *
 ************************************************************************
 */

//double zeroin(ax,bx,f,tol)        /* An estimate to the root  */
//double ax;                        /* Left border | of the range   */
//double bx;                        /* Right border| the root is seeked*/
//double (*f)(double x);            /* Function under investigation */
//double tol;                       /* Acceptable tolerance     */

double GSUtil::zeroin(double ax, double bx, double (*f)(double x, void *info), void *info, double tol)
{
    double a,b,c;             /* Abscissae, descr. see above  */
    double fa;                /* f(a)             */
    double fb;                /* f(b)             */
    double fc;                /* f(c)             */

    a = ax;  b = bx;  fa = (*f)(a, info);  fb = (*f)(b, info);
    c = a;   fc = fa;

    for(;;)       /* Main iteration loop  */
    {
        double prev_step = b-a;             /* Distance from the last but one*/
        /* to the last approximation    */
        double tol_act;                     /* Actual tolerance     */
        double p;                           /* Interpolation step is calcu- */
        double q;                           /* lated in the form p/q; divi- */
                                            /* sion operations is delayed   */
                                            /* until the last moment    */
        double new_step;                    /* Step at this iteration       */

        if( fabs(fc) < fabs(fb) )
        {                                   /* Swap data for b to be the    */
            a = b;  b = c;  c = a;          /* best approximation       */
            fa=fb;  fb=fc;  fc=fa;
        }
        tol_act = 2*DBL_EPSILON*fabs(b) + tol/2;
        new_step = (c-b)/2;

        if( fabs(new_step) <= tol_act || fb == 0 )
            return b;                       /* Acceptable approx. is found  */

        /* Decide if the interpolation can be tried */
        if( fabs(prev_step) >= tol_act      /* If prev_step was large enough*/
                && fabs(fa) > fabs(fb) )    /* and was in true direction,   */
        {                                   /* Interpolatiom may be tried   */
            double t1,cb,t2;
            cb = c-b;
            if( a==c )                      /* If we have only two distinct */
            {                               /* points linear interpolation  */
                t1 = fb/fa;                 /* can only be applied      */
                p = cb*t1;
                q = 1.0 - t1;
            }
            else                            /* Quadric inverse interpolation*/
            {
                q = fa/fc;  t1 = fb/fc;  t2 = fb/fa;
                p = t2 * ( cb*q*(q-t1) - (b-a)*(t1-1.0) );
                q = (q-1.0) * (t1-1.0) * (t2-1.0);
            }
            if( p>0 )                       /* p was calculated with the op-*/
                q = -q;                     /* posite sign; make p positive */
            else                            /* and assign possible minus to */
                p = -p;                     /* q                */

            if( p < (0.75*cb*q-fabs(tol_act*q)/2)   /* If b+p/q falls in [b,c]*/
                    && p < fabs(prev_step*q/2) )    /* and isn't too large  */
                new_step = p/q;                     /* it is accepted   */
                                                    /* If p/q is too large then the */
                                                    /* bissection procedure can     */
                                                    /* reduce [b,c] range to more   */
                                                    /* extent           */
        }

        if( fabs(new_step) < tol_act )      /* Adjust the step to be not less*/
        {
            if( new_step > 0 )              /* than tolerance       */
                new_step = tol_act;
            else
                new_step = -tol_act;
        }

        a = b;  fa = fb;                    /* Save the previous approx.    */
        b += new_step;  fb = (*f)(b, info); /* Do step to a new approxim.   */
        if( (fb > 0 && fc > 0) || (fb < 0 && fc < 0) )
        {                                   /* Adjust c for it to have a sign*/
            c = a;  fc = fa;                /* opposite to that of b    */
        }
    }

}

#endif
