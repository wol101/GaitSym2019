/*
 *  DataTargetQuaternion.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Tue July 14 2009.
 *  Copyright (c) 1009 Bill Sellers. All rights reserved.
 *
 */

#include <iostream>
#ifdef MALLOC_H_NEEDED
#include <malloc.h>
#endif
#ifdef ALLOCA_H_NEEDED
#include <alloca.h>
#endif

#include "ode/ode.h"

#include "DataTargetQuaternion.h"
#include "Body.h"
#include "PGDMath.h"
#include "GSUtil.h"
#include "DataFile.h"
#include "Geom.h"

DataTargetQuaternion::DataTargetQuaternion()
{
    m_QValueList = nullptr;
    m_QValueListLength = -1;
}

DataTargetQuaternion::~DataTargetQuaternion()
{
    if (m_QValueList) delete [] m_QValueList;
}

// note in this case the pointer is to a list of the elements of
// size quaternions
// note quaternion is (qs,qx,qy,qz)
void DataTargetQuaternion::SetTargetValues(int size, double *values)
{
    int i;
    if (size != TargetTimeListLength())
    {
        std::cerr << "DataTargetQuaternion::SetTargetValues error: size = " << size << "\n";
        return;
    }
    if (m_QValueListLength != size)
    {
        if (m_QValueList) delete [] m_QValueList;
        m_QValueListLength = size;
        m_QValueList = new pgd::Quaternion[m_QValueListLength];
    }
    for (i = 0 ; i < m_QValueListLength; i++)
    {
        m_QValueList[i].n = values[i * 4];
        m_QValueList[i].v.x = values[i * 4 + 1];
        m_QValueList[i].v.y = values[i * 4 + 2];
        m_QValueList[i].v.z = values[i * 4 + 3];
        m_QValueList[i].Normalize(); // always do this on input.
    }
}

// note in this case the pointer is to a string which is a list of the elements of
// size quaternions (or angle axis orientations following the standard d or r postscript
// convention
// note quaternion is (qs,qx,qy,qz)
void DataTargetQuaternion::SetTargetValues(const char *buf)
{
    int l = strlen(buf);
    char *lBuf = (char *)alloca((l + 1) * sizeof(char));
    char **lBufPtrs = (char **)alloca(l * sizeof(char *));
    dQuaternion quaternion;
    int i;

    strcpy(lBuf, buf);
    int size = DataFile::ReturnTokens(lBuf, lBufPtrs, l);

    if (size != TargetTimeListLength() * 4)
    {
        std::cerr << "DataTargetQuaternion::SetTargetValues error: size = " << size << "\n";
        return;
    }

    if (m_QValueListLength != TargetTimeListLength())
    {
        if (m_QValueList) delete [] m_QValueList;
        m_QValueListLength = TargetTimeListLength();
        m_QValueList = new pgd::Quaternion[m_QValueListLength];
    }
    for (i = 0 ; i < m_QValueListLength; i++)
    {
        GSUtil::GetQuaternion(&lBufPtrs[i * 4], quaternion);
        m_QValueList[i].n = quaternion[0];
        m_QValueList[i].v.x = quaternion[1];
        m_QValueList[i].v.y = quaternion[2];
        m_QValueList[i].v.z = quaternion[3];
    }
}

// returns the degree of match to the stored values
// in this case this is the angle between the two quaternions
double DataTargetQuaternion::GetError(int valueListIndex)
{
    const double *r;
    Body *body;
    Geom *geom;
    dQuaternion q;
    double angle = 0;
    if (valueListIndex < 0) valueListIndex = 0;
    if (valueListIndex >= m_QValueListLength)
    {
        std::cerr << "Warning: DataTargetQuaternion::GetMatchValue valueListIndex out of range\n";
        return 0;
    }

    if ((body = dynamic_cast<Body *>(GetTarget())) != nullptr)
    {
        r = body->GetQuaternion();
        angle = pgd::FindAngle(m_QValueList[valueListIndex], pgd::Quaternion(r[0], r[1], r[2], r[3]));
    }
    else if ((geom = dynamic_cast<Geom *>(GetTarget())) != nullptr)
    {
        geom->GetWorldQuaternion(q);
        angle = pgd::FindAngle(m_QValueList[valueListIndex], pgd::Quaternion(q[0], q[1], q[2], q[3]));
    }
    else
    {
        std::cerr << "DataTargetQuaternion target missing error " << GetName() << "\n";
    }
    return angle;
}

// returns the degree of match to the stored values
// in this case this is the angle between the two quaternions
double DataTargetQuaternion::GetError(double time)
{
    const double *r;
    Body *body;
    Geom *geom;
    dQuaternion q;
    double angle = 0;

    int index = GSUtil::BinarySearchRange(TargetTimeList(), TargetTimeListLength(), time);
    if (index < 0) index = 0;
    if (index >= m_QValueListLength - 1)
    {
        std::cerr << "Warning: DataTargetVector::GetMatchValue index out of range\n";
        return 0;
    }
    int indexNext = index + 1;

    // do a slerp interpolation between the target quaternions
    double interpolationFraction = (time - TargetTimeList()[index]) / (TargetTimeList()[indexNext] - TargetTimeList()[index]);
    pgd::Quaternion interpolatedTarget = pgd::slerp(m_QValueList[index], m_QValueList[indexNext], interpolationFraction);

    if ((body = dynamic_cast<Body *>(GetTarget())) != nullptr)
    {
        r = body->GetQuaternion();
        angle = pgd::FindAngle(interpolatedTarget, pgd::Quaternion(r[0], r[1], r[2], r[3]));
    }
    else if ((geom = dynamic_cast<Geom *>(GetTarget())) != nullptr)
    {
        geom->GetWorldQuaternion(q);
        angle = pgd::FindAngle(interpolatedTarget, pgd::Quaternion(q[0], q[1], q[2], q[3]));
    }
    else
    {
        std::cerr << "DataTargetQuaternion target missing error " << GetName() << "\n";
    }
    return angle;
}

void DataTargetQuaternion::Dump()
{
    if (dump() == false) return;

    if (firstDump())
    {
        setFirstDump(false);
        if (dumpStream() == nullptr)
        {
            if (GetName().size() == 0) std::cerr << "NamedObject::Dump error: can only dump a named object\n";
            std::string filename(GetName());
            filename.append(".dump");
            setDumpStream(new std::ofstream(filename.c_str()));
            dumpStream()->precision(17);
        }
        if (dumpStream())
        {
            *dumpStream() << "Time\tTargetQW\tTargetQX\tTargetQY\tTargetQZ\tActualQW\tActualQX\tActualQY\tActualQZ\tAngle\n";
        }
    }

    Body *body;
    Geom *geom;
    const double *r;
    double angle = 0;
    dQuaternion q;

    if (dumpStream())
    {
        int valueListIndex = GSUtil::BinarySearchRange(TargetTimeList(), TargetTimeListLength(), simulation()->GetTime());
        if ((body = dynamic_cast<Body *>(GetTarget())) != nullptr)
        {
            r = body->GetQuaternion();
            angle = pgd::FindAngle(m_QValueList[valueListIndex], pgd::Quaternion(r[0], r[1], r[2], r[3]));
        }
        else if ((geom = dynamic_cast<Geom *>(GetTarget())) != nullptr)
        {
            geom->GetWorldQuaternion(q);
            angle = pgd::FindAngle(m_QValueList[valueListIndex], pgd::Quaternion(q[0], q[1], q[2], q[3]));
        }

        *dumpStream() << simulation()->GetTime() <<
                "\t" << m_QValueList[valueListIndex].n << "\t" << m_QValueList[valueListIndex].v.x << "\t" << m_QValueList[valueListIndex].v.y << "\t" << m_QValueList[valueListIndex].v.z <<
                "\t" << q[0] << "\t" << q[1] << "\t" << q[2] << "\t" << q[3] <<
                "\t" << angle <<
                "\n";
    }
}

