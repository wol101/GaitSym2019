/*
 *  PositionReporter.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 13/12/2010.
 *  Copyright 2010 Bill Sellers. All rights reserved.
 *
 */

#include "PositionReporter.h"
#include "PGDMath.h"
#include "DataFile.h"
#include "Simulation.h"
#include "Body.h"
#include "GSUtil.h"

#include <iostream>
#ifdef MALLOC_H_NEEDED
#include <malloc.h>
#endif
#ifdef ALLOCA_H_NEEDED
#include <alloca.h>
#endif

PositionReporter::PositionReporter()
{
    mBody = 0;
    mQuaternion.n = 1; // rest of the quaternion is set to zero already
}

// parses the position allowing a relative position specified by BODY ID
// x y z - world coordinates
// bodyName x y z - position relative to bodyName local coordinate system
void PositionReporter::SetPosition(const char *buf)
{
    int i;
    int l = strlen(buf);
    char *lBuf = (char *)alloca((l + 1) * sizeof(char));
    char **lBufPtrs = (char **)alloca(l * sizeof(char *));
    dVector3 pos, result;

    strcpy(lBuf, buf);
    int count = DataFile::ReturnTokens(lBuf, lBufPtrs, l);
    if (count < 3)
    {
        std::cerr << "Error in PositionReporter::SetPosition\n";
        return; // error condition
    }

    if (isalpha((int)*lBufPtrs[0]) == 0)
    {
        for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i], 0);
        if (mBody)
        {
            dBodyGetPosRelPoint(mBody->GetBodyID(), pos[0], pos[1], pos[2], result); // convert from world to body
            SetPosition(result[0], result[1], result[2]);
        }
        else
        {
            SetPosition(pos[0], pos[1], pos[2]);
        }
        return;
    }

    if (count < 4)
    {
        std::cerr << "Error in PositionReporter::SetPosition\n";
        return; // error condition
    }
    Body *theBody = simulation()->GetBody(lBufPtrs[0]);
    if (theBody == 0)
    {
        if (strcmp(lBufPtrs[0], "World") == 0)
        {
            for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i + 1], 0);
            if (mBody)
            {
                dBodyGetPosRelPoint(mBody->GetBodyID(), pos[0], pos[1], pos[2], result); // convert from world to body
                SetPosition(result[0], result[1], result[2]);
            }
            else
            {
                SetPosition(pos[0], pos[1], pos[2]);
            }
            return;
        }
        else
        {
            std::cerr << "Error in PositionReporter::SetPosition\n";
            return; // error condition
        }
    }
    for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i + 1], 0);
    dBodyGetRelPointPos(theBody->GetBodyID(), pos[0], pos[1], pos[2], result); // convert from body to world
    if (mBody)
    {
        dBodyGetPosRelPoint(mBody->GetBodyID(), result[0], result[1], result[2], pos); // convert from world to body
        SetPosition(pos[0], pos[1], pos[2]);
    }
    else
    {
        SetPosition(result[0], result[1], result[2]);
    }
}

// parses the quaternion allowing a relative position specified by BODY ID
// note quaternion is (qs,qx,qy,qz)
// s x y z - world coordinates
// bodyName s x y z - position relative to bodyName local coordinate system
void PositionReporter::SetQuaternion(const char *buf)
{
    int i;
    int l = strlen(buf);
    char *lBuf = (char *)alloca((l + 1) * sizeof(char));
    char **lBufPtrs = (char **)alloca(l * sizeof(char *));
    dQuaternion quaternion;
    const double *q;

    strcpy(lBuf, buf);
    int count = DataFile::ReturnTokens(lBuf, lBufPtrs, l);
    if (count < 4)
    {
        std::cerr << "Error in PositionReporter::SetQuaternion\n";
        return; // error condition
    }


    if (isalpha((int)*lBufPtrs[0]) == 0)
    {
        if (mBody)
        {
            q = dBodyGetQuaternion(mBody->GetBodyID());
            pgd::Quaternion qBody(q[0], q[1], q[2], q[3]);
            GSUtil::GetQuaternion(&lBufPtrs[0], quaternion);
            pgd::Quaternion qWorld(quaternion[0], quaternion[1], quaternion[2], quaternion[3]);
            pgd::Quaternion qLocal = ~qBody * qWorld;
            SetQuaternion(qLocal.n, qLocal.v.x, qLocal.v.y, qLocal.v.z);
        }
        else
        {
            GSUtil::GetQuaternion(&lBufPtrs[0], quaternion);
            SetQuaternion(quaternion[0], quaternion[1], quaternion[2], quaternion[3]);
        }
        return;
    }

    if (count < 5)
    {
        std::cerr << "Error in PositionReporter::SetQuaternion\n";
        return; // error condition
    }
    Body *theBody = simulation()->GetBody(lBufPtrs[0]);
    if (theBody == 0)
    {
        if (strcmp(lBufPtrs[0], "World") == 0)
        {
            if (mBody)
            {
                q = dBodyGetQuaternion(mBody->GetBodyID());
                pgd::Quaternion qBody(q[0], q[1], q[2], q[3]);
                GSUtil::GetQuaternion(&lBufPtrs[1], quaternion);
                pgd::Quaternion qWorld(quaternion[0], quaternion[1], quaternion[2], quaternion[3]);
                pgd::Quaternion qLocal = ~qBody * qWorld;
                SetQuaternion(qLocal.n, qLocal.v.x, qLocal.v.y, qLocal.v.z);
            }
            else
            {
                GSUtil::GetQuaternion(&lBufPtrs[0], quaternion);
                SetQuaternion(quaternion[0], quaternion[1], quaternion[2], quaternion[3]);
            }
            return;
        }
        else
        {
            std::cerr << "Error in PositionReporter::SetQuaternion\n";
            return; // error condition
        }
    }
    // first get world quaternion
    const double *q2 = theBody->GetQuaternion();
    pgd::Quaternion qBody1(q2[0], q2[1], q2[2], q2[3]);
    for (i = 0; i < 4; i++) quaternion[i] = strtod(lBufPtrs[i + 1], 0);
    pgd::Quaternion qBody2(quaternion[0], quaternion[1], quaternion[2], quaternion[3]);
    pgd::Quaternion qWorld = qBody1 * qBody2;

    // then set the local quaternion
    if (mBody)
    {
        q = dBodyGetQuaternion(mBody->GetBodyID());
        pgd::Quaternion qBody(q[0], q[1], q[2], q[3]);
        pgd::Quaternion qLocal = ~qBody * qWorld;
        SetQuaternion(qLocal.n, qLocal.v.x, qLocal.v.y, qLocal.v.z);
    }
    else
    {
        SetQuaternion(qWorld.n, qWorld.v.x, qWorld.v.y, qWorld.v.z);
    }
}

pgd::Vector PositionReporter::GetWorldPosition()
{
    if (mBody)
    {
        // get the position in world coordinates
        dVector3 p;
        dBodyGetRelPointPos(mBody->GetBodyID(), mPosition.x, mPosition.y, mPosition.z, p);
        return pgd::Vector(p[0], p[1], p[2]);
    }
    else
    {
        return mPosition;
    }
}

pgd::Quaternion PositionReporter::GetWorldQuaternion()
{
    if (mBody)
    {
        const double *bodyRotation = dBodyGetQuaternion(mBody->GetBodyID());
        pgd::Quaternion bodyQuaternion(bodyRotation[0], bodyRotation[1], bodyRotation[2], bodyRotation[3]);
        return bodyQuaternion * mQuaternion;
    }
    else
    {
        return mQuaternion;
    }
}

pgd::Vector PositionReporter::GetWorldVelocity()
{
    if (mBody)
    {
        // get the velocity in world coordinates
        dVector3 p;
        dBodyGetRelPointVel(mBody->GetBodyID(), mPosition.x, mPosition.y, mPosition.z, p);
        return pgd::Vector(p[0], p[1], p[2]);
    }
    else
    {
        return pgd::Vector();
    }
}

void PositionReporter::Dump()
{
    if (dump() == false) return;

    if (firstDump())
    {
        setFirstDump(false);
        if (dumpStream() == 0)
        {
            if (GetName().size() == 0) std::cerr << "NamedObject::Dump error: can only dump a named object\n";
            std::string filename(GetName());
            filename.append(".dump");
            setDumpStream(new std::ofstream(filename));
            dumpStream()->precision(17);
        }
        if (dumpStream())
        {
            *dumpStream() << "Time\tXP\tYP\tZP\tQW\tQX\tQY\tQZ\n";
        }
    }


    if (dumpStream())
    {
        pgd::Vector p = GetWorldPosition();
        pgd::Quaternion q = GetWorldQuaternion();

        *dumpStream() << simulation()->GetTime() << "\t" << p.x << "\t" << p.y << "\t" << p.z <<
                "\t" << q.n << "\t" << q.v.x << "\t" << q.v.y << "\t" << q.v.z << "\n";
    }
}


