/*
 *  DataTargetVector.h
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

#include "DataTargetVector.h"
#include "Body.h"
#include "PGDMath.h"
#include "GSUtil.h"
#include "DataFile.h"
#include "HingeJoint.h"
#include "BallJoint.h"
#include "UniversalJoint.h"
#include "PositionReporter.h"
#include "Geom.h"

DataTargetVector::DataTargetVector()
{
    m_VValueList = nullptr;
    m_VValueListLength = -1;
}

DataTargetVector::~DataTargetVector()
{
    if (m_VValueList) delete [] m_VValueList;
}

// note in this case the pointer is to a list of the elements of
// size vectors
void DataTargetVector::SetTargetValues(int size, double *values)
{
    int i;
    if (size != TargetTimeListLength())
    {
        std::cerr << "DataTargetVector::SetTargetValues error: size = " << size << "\n";
        return;
    }
    if (m_VValueListLength != size)
    {
        if (m_VValueList) delete [] m_VValueList;
        m_VValueListLength = size;
        m_VValueList = new pgd::Vector[m_VValueListLength];
    }
    for (i = 0 ; i < m_VValueListLength; i++)
    {
        m_VValueList[i].x = values[i * 3];
        m_VValueList[i].y = values[i * 3 + 1];
        m_VValueList[i].z = values[i * 3 + 2];
    }
}

// note in this case the pointer is to a string which is a list of the elements of
// size vector
void DataTargetVector::SetTargetValues(const char *buf)
{
    int l = strlen(buf);
    char *lBuf = (char *)alloca((l + 1) * sizeof(char));
    char **lBufPtrs = (char **)alloca(l * sizeof(char *));
    int i;

    strcpy(lBuf, buf);
    int size = DataFile::ReturnTokens(lBuf, lBufPtrs, l);

    if (size != TargetTimeListLength() * 3)
    {
        std::cerr << "DataTargetVector::SetTargetValues error: size = " << size << "\n";
        return;
    }

    if (m_VValueListLength != TargetTimeListLength())
    {
        if (m_VValueList) delete [] m_VValueList;
        m_VValueListLength = TargetTimeListLength();
        m_VValueList = new pgd::Vector[m_VValueListLength];
    }
    for (i = 0 ; i < m_VValueListLength; i++)
    {
        m_VValueList[i].x = GSUtil::Double(lBufPtrs[i * 3]);
        m_VValueList[i].y = GSUtil::Double(lBufPtrs[i * 3 + 1]);
        m_VValueList[i].z = GSUtil::Double(lBufPtrs[i * 3 + 2]);
    }
}

// returns the degree of match to the stored values
// in this case this is the euclidean distance between the two vectors
double DataTargetVector::GetError(int valueListIndex)
{
    const double *r;
    Body *body;
    Geom *geom;
    HingeJoint *hingeJoint;
    BallJoint *ballJoint;
    UniversalJoint *universalJoint;
    PositionReporter *positionReporter;
    double err = 0;
    dVector3 v;
    if (valueListIndex < 0) valueListIndex = 0;
    if (valueListIndex >= m_VValueListLength)
    {
        std::cerr << "Warning: DataTargetVector::GetMatchValue valueListIndex out of range\n";
        return 0;
    }

    if ((body = dynamic_cast<Body *>(GetTarget())) != nullptr)
    {
        r = body->GetPosition();
        err = (pgd::Vector(r[0], r[1], r[2]) - m_VValueList[valueListIndex]).Magnitude();
    }
    else if ((geom = dynamic_cast<Geom *>(GetTarget())) != nullptr)
    {
        geom->GetWorldPosition(v);
        err = (pgd::Vector(v[0], v[1], v[2]) - m_VValueList[valueListIndex]).Magnitude();
    }
    else if ((hingeJoint = dynamic_cast<HingeJoint *>(GetTarget())) != nullptr)
    {
        hingeJoint->GetHingeAnchor(v);
        err = (pgd::Vector(v[0], v[1], v[2]) - m_VValueList[valueListIndex]).Magnitude();
   }
    else if ((ballJoint = dynamic_cast<BallJoint *>(GetTarget())) != nullptr)
    {
        ballJoint->GetBallAnchor(v);
        err = (pgd::Vector(v[0], v[1], v[2]) - m_VValueList[valueListIndex]).Magnitude();
   }
    else if ((universalJoint = dynamic_cast<UniversalJoint *>(GetTarget())) != nullptr)
    {
        universalJoint->GetUniversalAnchor(v);
        err = (pgd::Vector(v[0], v[1], v[2]) - m_VValueList[valueListIndex]).Magnitude();
    }
    else if ((positionReporter = dynamic_cast<PositionReporter *>(GetTarget())) != nullptr)
    {
        pgd::Vector vec = positionReporter->GetWorldPosition();
        err = (vec - m_VValueList[valueListIndex]).Magnitude();
    }
    else
    {
        std::cerr << "DataTargetVector target missing error " << GetName() << "\n";
    }
    return err;
}

// returns the degree of match to the stored values
// in this case this is the euclidean distance between the two vectors
double DataTargetVector::GetError(double time)
{
    const double *r;
    Body *body;
    Geom *geom;
    HingeJoint *hingeJoint;
    BallJoint *ballJoint;
    UniversalJoint *universalJoint;
    PositionReporter *positionReporter;
    double err = 0;
    dVector3 v;

    int index = GSUtil::BinarySearchRange(TargetTimeList(), TargetTimeListLength(), time);
    if (index < 0) index = 0;
    if (index >= m_VValueListLength - 1)
    {
        std::cerr << "Warning: DataTargetVector::GetMatchValue index out of range\n";
        return 0;
    }
    int indexNext = index + 1;

    double interpX = GSUtil::Interpolate(TargetTimeList()[index], m_VValueList[index].x, TargetTimeList()[indexNext], m_VValueList[indexNext].x, time);
    double interpY = GSUtil::Interpolate(TargetTimeList()[index], m_VValueList[index].y, TargetTimeList()[indexNext], m_VValueList[indexNext].y, time);
    double interpZ = GSUtil::Interpolate(TargetTimeList()[index], m_VValueList[index].z, TargetTimeList()[indexNext], m_VValueList[indexNext].z, time);
    pgd::Vector interpolatedTarget(interpX, interpY, interpZ);

    if ((body = dynamic_cast<Body *>(GetTarget())) != nullptr)
    {
        r = body->GetPosition();
        err = (pgd::Vector(r[0], r[1], r[2]) - interpolatedTarget).Magnitude();
    }
    else if ((geom = dynamic_cast<Geom *>(GetTarget())) != nullptr)
    {
        geom->GetWorldPosition(v);
        err = (pgd::Vector(v[0], v[1], v[2]) - interpolatedTarget).Magnitude();
    }
    else if ((hingeJoint = dynamic_cast<HingeJoint *>(GetTarget())) != nullptr)
    {
        hingeJoint->GetHingeAnchor(v);
        err = (pgd::Vector(v[0], v[1], v[2]) - interpolatedTarget).Magnitude();
   }
    else if ((ballJoint = dynamic_cast<BallJoint *>(GetTarget())) != nullptr)
    {
        ballJoint->GetBallAnchor(v);
        err = (pgd::Vector(v[0], v[1], v[2]) - interpolatedTarget).Magnitude();
   }
    else if ((universalJoint = dynamic_cast<UniversalJoint *>(GetTarget())) != nullptr)
    {
        universalJoint->GetUniversalAnchor(v);
        err = (pgd::Vector(v[0], v[1], v[2]) - interpolatedTarget).Magnitude();
    }
    else if ((positionReporter = dynamic_cast<PositionReporter *>(GetTarget())) != nullptr)
    {
        pgd::Vector vec = positionReporter->GetWorldPosition();
        err = (vec - interpolatedTarget).Magnitude();
    }
    else
    {
        std::cerr << "DataTargetVector target missing error " << GetName() << "\n";
    }
    return err;
}

void DataTargetVector::Dump()
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
            setDumpStream(new std::ofstream(filename));
            dumpStream()->precision(17);
        }
        if (dumpStream())
        {
            *dumpStream() << "Time\tTargetX\tTargetY\tTargetZ\tActualX\tActualY\tActualZ\tDistance\n";
        }
    }

    Body *body;
    Geom *geom;
    HingeJoint *hingeJoint;
    BallJoint *ballJoint;
    UniversalJoint *universalJoint;
    PositionReporter *positionReporter;
    const double *r = nullptr;
    double err = 0;
    dVector3 v;

    if (dumpStream())
    {
        int valueListIndex = GSUtil::BinarySearchRange(TargetTimeList(), TargetTimeListLength(), simulation()->GetTime());
        if ((body = dynamic_cast<Body *>(GetTarget())) != nullptr)
        {
            r = body->GetPosition();
            err = (pgd::Vector(r[0], r[1], r[2]) - m_VValueList[valueListIndex]).Magnitude();
        }
        else if ((geom = dynamic_cast<Geom *>(GetTarget())) != nullptr)
        {
            geom->GetWorldPosition(v);
            err = (pgd::Vector(v[0], v[1], v[2]) - m_VValueList[valueListIndex]).Magnitude();
            r = v;
        }
        else if ((hingeJoint = dynamic_cast<HingeJoint *>(GetTarget())) != nullptr)
        {
            hingeJoint->GetHingeAnchor(v);
            err = (pgd::Vector(v[0], v[1], v[2]) - m_VValueList[valueListIndex]).Magnitude();
            r = v;
        }
        else if ((ballJoint = dynamic_cast<BallJoint *>(GetTarget())) != nullptr)
        {
            ballJoint->GetBallAnchor(v);
            err = (pgd::Vector(v[0], v[1], v[2]) - m_VValueList[valueListIndex]).Magnitude();
            r = v;
        }
        else if ((universalJoint = dynamic_cast<UniversalJoint *>(GetTarget())) != nullptr)
        {
            universalJoint->GetUniversalAnchor(v);
            err = (pgd::Vector(v[0], v[1], v[2]) - m_VValueList[valueListIndex]).Magnitude();
            r = v;
        }
        else if ((positionReporter = dynamic_cast<PositionReporter *>(GetTarget())) != nullptr)
        {
            pgd::Vector vec = positionReporter->GetWorldPosition();
            v[0] = vec.x; v[1] = vec.y; v[2] = vec.z;
            err = (pgd::Vector(v[0], v[1], v[2]) - m_VValueList[valueListIndex]).Magnitude();
            r = v;
        }

        *dumpStream() << simulation()->GetTime() <<
                "\t" << m_VValueList[valueListIndex].x << "\t" << m_VValueList[valueListIndex].y << "\t" << m_VValueList[valueListIndex].z <<
                "\t" << r[0] << "\t" << r[1] << "\t" << r[2] <<
                "\t" << err <<
                "\n";
    }
}


