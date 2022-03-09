/*
 *  SwingClearanceAbortReporter.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 26/04/2011.
 *  Copyright 2011 Bill Sellers. All rights reserved.
 *
 */

#include "SwingClearanceAbortReporter.h"
#include "GSUtil.h"
#include "Body.h"
#include "Simulation.h"

#include <sstream>

SwingClearanceAbortReporter::SwingClearanceAbortReporter() : Marker(nullptr)
{
    m_heightThreshold = 0;
    m_velocityThreshold = 0;
    m_upAxis = Z;
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
    case X:
        m_height = p[0];
        break;
    case Y:
        m_height = p[1];
        break;
    case Z:
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
        case X:
            m_velocity = sqrt(v[1] * v[1] + v[2] * v[2]);
            break;
        case Y:
            m_velocity = sqrt(v[2] * v[2] + v[0] * v[0]);
            break;
        case Z:
            m_velocity = sqrt(v[0] * v[0] + v[1] * v[1]);
            break;
        }
    }
    else
    {
        m_velocity = pgd::Vector3(v[0], v[1], v[2]) * m_directionAxis;
    }

    if (m_height > m_heightThreshold) return false;
    if (m_velocity < m_velocityThreshold) return false;

    return true;
}

void SwingClearanceAbortReporter::SetUpAxis(const char *upAxis)
{
    if (strcasecmp(upAxis, "X") == 0) SetUpAxis(X);
    else if (strcasecmp(upAxis, "Y") == 0) SetUpAxis(Y);
    else if (strcasecmp(upAxis, "Z") == 0) SetUpAxis(Z);
}

void SwingClearanceAbortReporter::SetDirectionAxis(double x, double y, double z)
{
    m_directionAxis.Set(x, y, z);
    m_directionAxis.Normalize();
    m_useDirectionAxis = true;
}

std::string SwingClearanceAbortReporter::dumpToString()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (firstDump())
    {
        setFirstDump(false);

        ss << "Time\tXP\tYP\tZP\tXV\tYV\tZV\theight\tvelocity\n";
    }

    pgd::Vector3 p = GetWorldPosition();
    // pgd::Quaternion q = GetWorldQuaternion();
    pgd::Vector3 v = GetWorldLinearVelocity();
    ShouldAbort(); // this is needed because otherwise these values are out of sync

    ss << simulation()->GetTime() << "\t" << p.x << "\t" << p.y << "\t" << p.z <<
          "\t" << v.x << "\t" << v.y << "\t" << v.z <<
          "\t" << m_height << "\t" << m_velocity << "\n";
    return ss.str();
}

