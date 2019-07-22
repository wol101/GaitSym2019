/*
 *  SwingClearanceAbortReporter.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 26/04/2011.
 *  Copyright 2011 Bill Sellers. All rights reserved.
 *
 */

#include "ode/ode.h"

#ifdef MALLOC_H_NEEDED
#include <malloc.h>
#endif
#ifdef ALLOCA_H_NEEDED
#include <alloca.h>
#endif

#include "SwingClearanceAbortReporter.h"
#include "GSUtil.h"
#include "Body.h"
#include "Simulation.h"

SwingClearanceAbortReporter::SwingClearanceAbortReporter()
{
    m_heightThreshold = 0;
    m_velocityThreshold = 0;
    m_upAxis = z;
    m_useDirectionAxis = false;
    m_height = 0;
    m_velocity = 0;
}

bool SwingClearanceAbortReporter::ShouldAbort()
{
    if (GetBody() == nullptr) return false;

    // get the position in world coordinates
    dVector3 p;
    dBodyGetRelPointPos(GetBody()->GetBodyID(), GetPosition().x, GetPosition().y, GetPosition().z, p);
    switch (m_upAxis)
    {
    case x:
        m_height = p[0];
        break;
    case y:
        m_height = p[1];
        break;
    case z:
        m_height = p[2];
        break;
    }

    // get the velocity in world coordinates
    dVector3 v;
    dBodyGetRelPointVel(GetBody()->GetBodyID(), GetPosition().x, GetPosition().y, GetPosition().z, v);

    if (m_useDirectionAxis == false)
    {
        switch (m_upAxis)
        {
        case x:
            m_velocity = sqrt(v[1] * v[1] + v[2] * v[2]);
            break;
        case y:
            m_velocity = sqrt(v[2] * v[2] + v[0] * v[0]);
            break;
        case z:
            m_velocity = sqrt(v[0] * v[0] + v[1] * v[1]);
            break;
        }
    }
    else
    {
        m_velocity = pgd::Vector(v[0], v[1], v[2]) * m_directionAxis;
    }

    if (m_height > m_heightThreshold) return false;
    if (m_velocity < m_velocityThreshold) return false;

    return true;
}

void SwingClearanceAbortReporter::SetUpAxis(const char *upAxis)
{
    if (strcasecmp(upAxis, "x") == 0) SetUpAxis(x);
    else if (strcasecmp(upAxis, "y") == 0) SetUpAxis(y);
    else if (strcasecmp(upAxis, "z") == 0) SetUpAxis(z);
}

void SwingClearanceAbortReporter::SetDirectionAxis(double x, double y, double z)
{
    m_directionAxis.Set(x, y, z);
    m_directionAxis.Normalize();
    m_useDirectionAxis = true;
}

// parses the direction axis allowing a relative position specified by BODY ID
// x y z - world coordinates
// bodyName x y z - position relative to bodyName local coordinate system
void SwingClearanceAbortReporter::SetDirectionAxis(const char *buf)
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
        std::cerr << "Error in SwingClearanceAbortReporter::SetDirectionAxis\n";
        return; // error condition
    }

    if (isalpha((int)*lBufPtrs[0]) == 0)
    {
        for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i], 0);
        SetDirectionAxis(pos[0], pos[1], pos[2]);
        return;
    }

    if (count < 4)
    {
        std::cerr << "Error in SwingClearanceAbortReporter::SetDirectionAxis\n";
        return; // error condition
    }
    Body *theBody = simulation()->GetBody(lBufPtrs[0]);
    if (theBody == 0)
    {
        if (strcmp(lBufPtrs[0], "World") == 0)
        {
            for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i + 1], 0);
            SetDirectionAxis(pos[0], pos[1], pos[2]);
            return;
        }
        else
        {
            std::cerr << "Error in SwingClearanceAbortReporter::SetDirectionAxis\n";
            return; // error condition
        }
    }
    for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i + 1], 0);
    dBodyVectorToWorld(theBody->GetBodyID(), pos[0], pos[1], pos[2], result);
    SetDirectionAxis(result[0], result[1], result[2]);
}

void SwingClearanceAbortReporter::Dump()
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
            *dumpStream() << "Time\tXP\tYP\tZP\tXV\tYV\tZV\theight\tvelocity\n";
        }
    }


    if (dumpStream())
    {
        pgd::Vector p = GetWorldPosition();
        // pgd::Quaternion q = GetWorldQuaternion();
        pgd::Vector v = GetWorldVelocity();
        ShouldAbort(); // this is needed because otherwise these values are out of sync

        *dumpStream() << simulation()->GetTime() << "\t" << p.x << "\t" << p.y << "\t" << p.z <<
                         "\t" << v.x << "\t" << v.y << "\t" << v.z <<
                         "\t" << m_height << "\t" << m_velocity << "\n";
    }
}

