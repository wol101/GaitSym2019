/*
 *  Body.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 19/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

// this class is a wrapper for the ODE body

#include "Body.h"
#include "Simulation.h"
#include "PGDMath.h"
#include "GSUtil.h"
#include "Marker.h"

#include "ode/ode.h"

#include "pystring.h"

#include <iostream>
#include <string>
#include <string.h>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <assert.h>

using namespace std::string_literals;

#define OUTSIDERANGE(x, minX, maxX) \
        ((x) < (minX) || (x) > (maxX))

Body::Body(dWorldID worldID)
{
    m_worldID = worldID;
    if (m_worldID)
    {
        m_bodyID = dBodyCreate(worldID);
        dBodySetData(m_bodyID, this);
    }
}

Body::~Body()
{
    if (m_bodyID) dBodyDestroy(m_bodyID);
}

void Body::SetPosition(double x, double y, double z)
{
    m_currentPosition[0] = x;
    m_currentPosition[1] = y;
    m_currentPosition[2] = z;
    if (m_bodyID) dBodySetPosition(m_bodyID, x, y, z);
}

void Body::SetQuaternion(double n, double x, double y, double z)
{
    m_currentQuaternion[0] = n;
    m_currentQuaternion[1] = x;
    m_currentQuaternion[2] = y;
    m_currentQuaternion[3] = z;
    if (m_bodyID) dBodySetQuaternion(m_bodyID, m_currentQuaternion);
}

// parses the position allowing a relative position specified by BODY ID
// x y z - world coordinates
// bodyName x y z - position relative to bodyName local coordinate system
// bodyName x1 y1 z1 x2 y2 z2 - position such that x1,y1,z1 on bodyName has same world coordinates as x2,y2,z2 on local body
std::string *Body::SetPosition(const std::string &buf)
{
    std::vector<std::string> tokens;
    pystring::split(buf, tokens);

    if (tokens.size() == 1) // it must be a marker
    {
        Marker *marker = simulation()->GetMarker(tokens[0]);
        if (marker == nullptr)
        {
            setLastError("Body ID=\""s + name() +"\" Position=\""s + buf + "\" marker not found"s);
            return lastErrorPtr();
        }
        pgd::Vector3 wp = marker->GetWorldPosition();
        this->SetPosition(wp.x, wp.y, wp.z);
        return nullptr;
    }

    if (tokens.size() == 2) // two marker definition
    {
        Marker *marker1 = simulation()->GetMarker(tokens[0]);
        Marker *marker2 = simulation()->GetMarker(tokens[1]);
        if (marker1 == nullptr || marker2 == nullptr)
        {
            setLastError("Body ID=\""s + name() +"\" Position=\""s + buf + "\" markers not found"s);
            return lastErrorPtr();
        }
        // find the marker that is relative to this body and make it marker1
        if (marker2->GetBody()->GetBodyID() == m_bodyID) std::swap(marker1, marker2);
        if (marker2->GetBody()->GetBodyID() == m_bodyID)
        {
            setLastError("Body ID=\""s + name() +"\" Position=\""s + buf + "\" Only one marker of pair must be relative to the body"s);
            return lastErrorPtr();
        }
        if (marker1->GetBody()->GetBodyID() != m_bodyID)
        {
            setLastError("Body ID=\""s + name() +"\" Position=\""s + buf + "\" Only one marker of pair must be relative to the body"s);
            return lastErrorPtr();
        }

        pgd::Vector3 target = marker2->GetWorldPosition();
        pgd::Vector3 current = marker2->GetWorldPosition();
        pgd::Vector3 difference = target - current;
        this->SetPosition(difference.x, difference.y, difference.z);
        return nullptr;
    }

    if (tokens.size() == 3)
    {
        this->SetPosition(GSUtil::Double(tokens[0]), GSUtil::Double(tokens[1]), GSUtil::Double(tokens[2]));
        return nullptr;
    }

    if (tokens.size() == 4)
    {
        Body *theBody = simulation()->GetBody(tokens[0]);
        if (theBody == nullptr)
        {
            if (tokens[0] == "World"s)
            {
                this->SetPosition(GSUtil::Double(tokens[1]), GSUtil::Double(tokens[2]), GSUtil::Double(tokens[3]));
                return nullptr;
            }
            else
            {
                setLastError("Body ID=\""s + name() +"\" Position=\""s + buf + "\" reference body not found"s);
                return lastErrorPtr();
            }
        }
        else
        {
            dVector3 result;
            dBodyGetRelPointPos (theBody->GetBodyID(), GSUtil::Double(tokens[1]), GSUtil::Double(tokens[2]), GSUtil::Double(tokens[3]), result);
            this->SetPosition(result[0], result[1], result[2]);
            return nullptr;
        }
    }
    if (tokens.size() == 7)
    {
        Body *theBody = simulation()->GetBody(tokens[0]);
        if (!theBody)
        {
            setLastError("Body ID=\""s + name() +"\" Position=\""s + buf + "\" reference body not found"s);
            return lastErrorPtr();
        }
        // get world coordinates of x1,y1,z1
        dVector3 world1, world2, pos;
        dBodyGetRelPointPos (theBody->GetBodyID(), GSUtil::Double(tokens[1]), GSUtil::Double(tokens[2]), GSUtil::Double(tokens[3]), world1);
        dBodyGetRelPointPos (m_bodyID, GSUtil::Double(tokens[4]), GSUtil::Double(tokens[5]), GSUtil::Double(tokens[6]), world2);
        // add the error to the current position
        const double *p = dBodyGetPosition(m_bodyID);
        for (size_t i = 0; i < 3; i++) pos[i] = p[i] + (world1[i] - world2[i]);
        this->SetPosition(pos[0], pos[1], pos[2]);

        // for checking
        // dBodyGetRelPointPos (m_BodyID, local2[0], local2[1], local2[2], world2);
        // for (i = 0; i < 3; i++) std::cerr << world1[i] << " " << world2[i] << "\n";
        return nullptr;
    }

    setLastError("Body ID=\""s + name() +"\" Position=\""s + buf + "\" too many tokens"s);
    return lastErrorPtr();
}

// parses the quaternion allowing a relative position specified by BODY ID
// note quaternion is (qs,qx,qy,qz)
// s x y z - world coordinates
// bodyName s x y z - position relative to bodyName local coordinate system
std::string *Body::SetQuaternion(const std::string &buf)
{
    std::vector<std::string> tokens;
    pystring::split(buf, tokens);

    if (tokens.size() == 1) // it must be a marker
    {
        Marker *marker = simulation()->GetMarker(tokens[0]);
        if (marker == nullptr)
        {
            setLastError("Body ID=\""s + name() +"\" Quaternion=\""s + buf + "\" Cannot find marker"s);
            return lastErrorPtr();
        }
        pgd::Quaternion wq = marker->GetWorldQuaternion();
        this->SetQuaternion(wq.n, wq.x, wq.y, wq.z);
        return nullptr;
    }

    if (tokens.size() == 4)
    {
        pgd::Quaternion wq = GSUtil::GetQuaternion(tokens, 0);
        this->SetQuaternion(wq.n, wq.x, wq.y, wq.z);
        return nullptr;
    }

    if (tokens.size() == 5)
    {
        Body *theBody = simulation()->GetBody(tokens[0]);
        if (theBody == nullptr)
        {
            if (tokens[0] == "World"s)
            {
                pgd::Quaternion wq = GSUtil::GetQuaternion(tokens, 1);
                this->SetQuaternion(wq.n, wq.x, wq.y, wq.z);
                return nullptr;
            }
            else
            {
                setLastError("Body ID=\""s + name() +"\" Quaternion=\""s + buf + "\" Cannot find body"s);
                return lastErrorPtr();
            }
        }
        const double *q = theBody->GetQuaternion();
        pgd::Quaternion qBody(q[0], q[1], q[2], q[3]);
        pgd::Quaternion qIn = GSUtil::GetQuaternion(tokens, 1);
        pgd::Quaternion qNew = qBody * qIn;
        this->SetQuaternion(qNew.n, qNew.x, qNew.y, qNew.z);
        return nullptr;
    }
    setLastError("Body ID=\""s + name() +"\" Position=\""s + buf + "\" wrong number of tokens"s);
    return lastErrorPtr();
}

void Body::SetLinearVelocity(double x, double y, double z)
{
    dBodySetLinearVel(m_bodyID, x, y, z);
}

// parses the linear velocity allowing a relative velocity specified by BODY ID
// x y z - world coordinates
// bodyName x y z - position relative to bodyName local coordinate system
std::string *Body::SetLinearVelocity(const std::string &buf)
{
    std::vector<std::string> tokens;
    pystring::split(buf, tokens);

    if (tokens.size() == 3)
    {
        this->SetLinearVelocity(GSUtil::Double(tokens[0]), GSUtil::Double(tokens[1]), GSUtil::Double(tokens[2]));
        return nullptr;
    }

    if (tokens.size() == 4)
    {
        Body *theBody = simulation()->GetBody(tokens[0]);
        if (theBody == nullptr)
        {
            if (tokens[0] == "World"s)
            {
                this->SetLinearVelocity(GSUtil::Double(tokens[1]), GSUtil::Double(tokens[2]), GSUtil::Double(tokens[3]));
                return nullptr;
            }
            else
            {
                setLastError("Body ID=\""s + name() +"\" LinearVelocity=\""s + buf + "\" Cannot find body"s);
                return lastErrorPtr();
            }
        }
        else
        {
            dVector3 result;
            dBodyVectorToWorld(theBody->GetBodyID(), GSUtil::Double(tokens[1]), GSUtil::Double(tokens[2]), GSUtil::Double(tokens[3]), result);
            const double *vRel = dBodyGetLinearVel(theBody->GetBodyID());
            SetLinearVelocity(result[0] + vRel[0], result[1] + vRel[1], result[2] + vRel[2]);
            return nullptr;
        }
    }
    setLastError("Body ID=\""s + name() +"\" LinearVelocity=\""s + buf + "\" wrong number of tokens"s);
    return lastErrorPtr();
}

void Body::SetPositionDelta(double x, double y, double z)
{
    m_currentPosition[0] += x;
    m_currentPosition[1] += y;
    m_currentPosition[2] += z;
    if (m_bodyID)
    {
        const double *p = dBodyGetPosition(m_bodyID);
        dBodySetPosition(m_bodyID, p[0] + x, p[1] + y, p[2] + z);
    }
}

void Body::SetQuaternionDelta(double n, double x, double y, double z)
{
    dQuaternion qb = {n, x, y, z};
    dQuaternion qa;
    dQMultiply0(qa, qb, m_currentQuaternion);
    m_currentQuaternion[0] = qa[0];
    m_currentQuaternion[1] = qa[1];
    m_currentQuaternion[2] = qa[2];
    m_currentQuaternion[3] = qa[3];
    if (m_bodyID)
    {
        const double *q = dBodyGetQuaternion(m_bodyID);
        dQMultiply0(qa, qb, q);
        dBodySetQuaternion(m_bodyID, qa);
    }
}

double Body::GetLinearKineticEnergy()
{
    // linear KE = 0.5 m v^2
    dMass mass;
    dBodyGetMass(m_bodyID, &mass);

    const double *v = dBodyGetLinearVel(m_bodyID);
    double linearKE = 0.5 * mass.mass * (v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);

    return linearKE;
}

void Body::GetLinearKineticEnergy(dVector3 ke)
{
    // linear KE = 0.5 m v^2
    dMass mass;
    dBodyGetMass(m_bodyID, &mass);

    const double *v = dBodyGetLinearVel(m_bodyID);
    ke[0] =  0.5 * mass.mass * v[0] * v[0];
    ke[1] =  0.5 * mass.mass * v[1] * v[1];
    ke[2] =  0.5 * mass.mass * v[2] * v[2];

    return;
}

double Body::GetRotationalKineticEnergy()
{

    // rotational KE = 0.5 * o(t) * I * o
    // where o is rotational velocity vector and o(t) is the same but transposed

    dMass mass;
    dBodyGetMass(m_bodyID, &mass);

    const double *ow = dBodyGetAngularVel(m_bodyID);
    dVector3 o;
    dBodyVectorFromWorld (m_bodyID, ow[0], ow[1], ow[2], o);
    dVector3 o1;
    dMultiply0_331(o1, mass.I, o);
    double rotationalKE = 0.5 * (o[0]*o1[0] + o[1]*o1[1] + o[2]*o1[2]);

    return rotationalKE;
}

double Body::GetGravitationalPotentialEnergy()
{
    dMass mass;
    dBodyGetMass(m_bodyID, &mass);
    dVector3 g;
    dWorldGetGravity (m_worldID, g);
    const double *p = dBodyGetPosition(m_bodyID);

    // gravitational PE = mgh
    double gravitationalPotentialEnergy = - mass.mass * (g[0]*p[0] + g[1]*p[1] + g[2]*p[2]);

    return gravitationalPotentialEnergy;
}

void Body::SetAngularVelocity(double x, double y, double z)
{
    dBodySetAngularVel(m_bodyID, x, y, z);
}

// parses the angular velocity allowing a relative angular velocity specified by BODY ID
// x y z - world coordinates
// bodyName x y z - position relative to bodyName local coordinate system
std::string *Body::SetAngularVelocity(const std::string &buf)
{
    std::vector<std::string> tokens;
    pystring::split(buf, tokens);

    if (tokens.size() == 3)
    {
        SetAngularVelocity(GSUtil::Double(tokens[0]), GSUtil::Double(tokens[1]), GSUtil::Double(tokens[2]));
        return nullptr;
    }

    if (tokens.size() == 4)
    {
        Body *theBody = simulation()->GetBody(tokens[0]);
        if (theBody == nullptr)
        {
            if (tokens[0] == "World"s)
            {
                SetAngularVelocity(GSUtil::Double(tokens[1]), GSUtil::Double(tokens[2]), GSUtil::Double(tokens[3]));
                return nullptr;
            }
            else
            {
                setLastError("Body ID=\""s + name() +"\" AngularVelocity=\""s + buf + "\" body not found"s);
                return lastErrorPtr();
            }
        }
        else
        {
            dVector3 result;
            dBodyVectorToWorld(theBody->GetBodyID(), GSUtil::Double(tokens[1]), GSUtil::Double(tokens[2]), GSUtil::Double(tokens[3]), result);
            const double *vARel = dBodyGetAngularVel(theBody->GetBodyID());
            SetAngularVelocity(result[0] + vARel[0], result[1] + vARel[1], result[2] + vARel[2]);
            return nullptr;
        }
    }
    setLastError("Body ID=\""s + name() +"\" AngularVelocity=\""s + buf + "\" wrong number of tokens"s);
    return lastErrorPtr();

}

dBodyID Body::GetBodyID() const
{
    return m_bodyID;
}

void Body::SetMass(const dMass *mass)
{
    dBodySetMass(m_bodyID, mass);
}

const double *Body::GetPosition()
{
    if (m_bodyID)
        return dBodyGetPosition(m_bodyID);
    else
        return m_currentPosition;
}

const double *Body::GetQuaternion()
{
    if (m_bodyID)
        return dBodyGetQuaternion(m_bodyID);
    else
        return m_currentQuaternion;
}

const double *Body::GetLinearVelocity()
{
    return dBodyGetLinearVel(m_bodyID);
}

const double *Body::GetAngularVelocity()
{
    return dBodyGetAngularVel(m_bodyID);
}

void Body::GetPosition(pgd::Vector3 *pos)
{
    const double *p = GetPosition();
    *pos = pgd::Vector3(p[0], p[1], p[2]);
}

void Body::GetQuaternion(pgd::Quaternion *quat)
{
    const double *qW = GetQuaternion();
    *quat = pgd::Quaternion(qW[0], qW[1], qW[2], qW[3]);
}

void Body::GetRelativePosition(Body *rel, pgd::Vector3 *pos)
{
    dVector3 result;
    const double *p = GetPosition();
    if (rel)
    {
        dBodyGetPosRelPoint(rel->GetBodyID(), p[0], p[1], p[2], result);
        *pos = pgd::Vector3(result[0], result[1], result[2]);
    }
    else
    {
        *pos = pgd::Vector3(p[0], p[1], p[2]);
    }
}

void Body::GetRelativeQuaternion(Body *rel, pgd::Quaternion *quat)
{
    const double *qW = GetQuaternion();
    if (rel)
    {
        const double *qR = rel->GetQuaternion();
        pgd::Quaternion qWorld(qW[0], qW[1], qW[2], qW[3]);
        pgd::Quaternion qRelBody(qR[0], qR[1], qR[2], qR[3]);
        *quat = ~qRelBody * qWorld;
    }
    else
    {
        *quat = pgd::Quaternion(qW[0], qW[1], qW[2], qW[3]);
    }
}

void Body::GetRelativeLinearVelocity(Body *rel, pgd::Vector3 *vel)
{
    const double *v = GetLinearVelocity();
    if (rel)
    {
        const double *qR = rel->GetQuaternion();
        const double *vR = rel->GetLinearVelocity();
        pgd::Vector3 worldV(v[0], v[1], v[2]);
        pgd::Vector3 relV(vR[0], vR[1], vR[2]);
        pgd::Quaternion qRelBody(qR[0], qR[1], qR[2], qR[3]);
        *vel = QVRotate(~qRelBody, worldV - relV);
    }
    else
    {
        *vel = pgd::Vector3(v[0], v[1], v[2]);
    }
}

void Body::GetRelativeAngularVelocity(Body *rel, pgd::Vector3 *rVel)
{
    const double *vR = GetAngularVelocity();
    if (rel)
    {
        const double *qR = rel->GetQuaternion();
        const double *vRR = rel->GetAngularVelocity();
        pgd::Vector3 worldVR(vR[0], vR[1], vR[2]);
        pgd::Vector3 relVR(vRR[0], vRR[1], vRR[2]);
        pgd::Quaternion qRelBody(qR[0], qR[1], qR[2], qR[3]);
        *rVel = QVRotate(~qRelBody, worldVR - relVR);
    }
    else
    {
        *rVel = pgd::Vector3(vR[0], vR[1], vR[2]);
    }
}

double Body::GetMass() const
{
    dMass mass;
    dBodyGetMass(m_bodyID, &mass);
    return mass.mass;
}

void Body::GetMass(dMass *mass) const
{
    dBodyGetMass(m_bodyID, mass);
    return;
}

Body::LimitTestResult Body::TestLimits()
{
    const double *p = dBodyGetPosition(m_bodyID);
    if (std::fpclassify(p[0]) != FP_NORMAL && std::fpclassify(p[0]) != FP_ZERO) return NumericalError;
    if (std::fpclassify(p[1]) != FP_NORMAL && std::fpclassify(p[1]) != FP_ZERO) return NumericalError;
    if (std::fpclassify(p[2]) != FP_NORMAL && std::fpclassify(p[2]) != FP_ZERO) return NumericalError;
    if (OUTSIDERANGE(p[0], m_positionLowBound[0], m_positionHighBound[0])) return XPosError;
    if (OUTSIDERANGE(p[1], m_positionLowBound[1], m_positionHighBound[1])) return YPosError;
    if (OUTSIDERANGE(p[2], m_positionLowBound[2], m_positionHighBound[2])) return ZPosError;

    const double *v = dBodyGetLinearVel(m_bodyID);
    if (std::fpclassify(v[0]) != FP_NORMAL && std::fpclassify(v[0]) != FP_ZERO) return NumericalError;
    if (std::fpclassify(v[1]) != FP_NORMAL && std::fpclassify(v[1]) != FP_ZERO) return NumericalError;
    if (std::fpclassify(v[2]) != FP_NORMAL && std::fpclassify(v[2]) != FP_ZERO) return NumericalError;
    if (OUTSIDERANGE(v[0], m_linearVelocityLowBound[0], m_linearVelocityHighBound[0])) return XVelError;
    if (OUTSIDERANGE(v[1], m_linearVelocityLowBound[1], m_linearVelocityHighBound[1])) return YVelError;
    if (OUTSIDERANGE(v[2], m_linearVelocityLowBound[2], m_linearVelocityHighBound[2])) return ZVelError;

    const double *a = dBodyGetAngularVel(m_bodyID);
    if (std::fpclassify(a[0]) != FP_NORMAL && std::fpclassify(a[0]) != FP_ZERO) return NumericalError;
    if (std::fpclassify(a[1]) != FP_NORMAL && std::fpclassify(a[1]) != FP_ZERO) return NumericalError;
    if (std::fpclassify(a[2]) != FP_NORMAL && std::fpclassify(a[2]) != FP_ZERO) return NumericalError;
    if (OUTSIDERANGE(a[0], m_angularVelocityLowBound[0], m_angularVelocityHighBound[0])) return XAVelError;
    if (OUTSIDERANGE(a[1], m_angularVelocityLowBound[1], m_angularVelocityHighBound[1])) return YAVelError;
    if (OUTSIDERANGE(a[2], m_angularVelocityLowBound[2], m_angularVelocityHighBound[2])) return ZAVelError;

    return WithinLimits;
}

double Body::GetProjectedAngle(const pgd::Vector3 &planeNormal, const pgd::Vector3 &vector1, const pgd::Vector3 &vector2)
{
    // calculate the projected angle from vector1 to vector2 projected onto the plane defined by a normal vector
    // this angle gives the proper -pi to +pi value to rotate the direction for vector1 to vector2
    // around planeNormal

    // start by projecting the two vectors onto a plane defined by the first axis as a normal
    // from https://www.euclideanspace.com/maths/geometry/elements/plane/lineOnPlane/index.htm
    // using the formula B cross (A cross B / |B|) / |B|
    // where A is the vector and B is the normal of the plane
    // for completeness this distance along the normal is
    // A dot B * B/|B|**2
    // however we can use some optimisation because we just want the direction in the plane which is
    // B cross (A cross B)
    // which we then normalise anyway
    pgd::Vector3 startDirection1 = planeNormal ^ (vector1 ^ planeNormal);
    startDirection1.Normalize();
    pgd::Vector3 endDirection1 = planeNormal ^ (vector2 ^ planeNormal);
    endDirection1.Normalize();
    // now we can find the angle in the plane using
    // dot = x1*x2 + y1*y2 + z1*z2
    // det = x1*y2*zn + x2*yn*z1 + xn*y1*z2 - z1*y2*xn - z2*yn*x1 - zn*y1*x2
    // angle = atan2(det, dot)
    // [alternative for det is det = n dot (v1 cross v2)]
    double dot = startDirection1 * endDirection1;
    double det = planeNormal * (startDirection1 ^ endDirection1);
    double angle = atan2(det, dot);
    return angle;
}

std::string Body::dumpToString()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (firstDump())
    {
        setFirstDump(false);
        ss << "Time\tXP\tYP\tZP\tXV\tYV\tZV\tQW\tQX\tQY\tQZ\tRVX\tRVY\tRVZ\tLKEX\tLKEY\tLKEZ\tRKE\tGPE\n";
    }
    const double *p = GetPosition();
    const double *v = GetLinearVelocity();
    const double *q = GetQuaternion();
    const double *rv = GetAngularVelocity();
    dVector3 ke;
    GetLinearKineticEnergy(ke);

    ss << simulation()->GetTime() << "\t" << p[0] << "\t" << p[1] << "\t" << p[2] <<
          "\t" << v[0] << "\t" << v[1] << "\t" << v[2] <<
          "\t" << q[0] << "\t" << q[1] << "\t" << q[2] << "\t" << q[3] <<
          "\t" << rv[0] << "\t" << rv[1] << "\t" << rv[2] <<
          "\t" << ke[0] << "\t" << ke[1] << "\t" << ke[2] <<
          "\t" << GetRotationalKineticEnergy() << "\t" << GetGravitationalPotentialEnergy() <<
          "\n";
    return ss.str();
}

// a utility function to calculate moments of interia given an arbitrary translation and rotation
// assumes starting point is the moment of inertia at the centre of mass
// #define _I(i,j) I[(i)*4+(j)]
// regex _I\(([0-9]+),([0-9]+)\) to I[(\1)*4+(\2)]
void Body::ParallelAxis(dMass *massProperties, const double *translation, const double *quaternion, dMass *newMassProperties)
{
    double x, y, z; // transformation from centre of mass to new location (m)
    double mass; // mass (kg)
    double ixx,  iyy,  izz,  ixy,  iyz,  izx; // moments of inertia kgm2
    double ang; // rotation angle (radians)
    double ax, ay, az; // axis of rotation
    double ixxp, iyyp, izzp, ixyp, iyzp, izxp; // transformed moments of inertia about new coordinate system)

    x = translation[0];
    y = translation[1];
    z = translation[2];
    mass = massProperties->mass;
//    ixx = massProperties->_I(0,0);
//    iyy = massProperties->_I(1,1);
//    izz = massProperties->_I(2,2);
//    ixy = massProperties->_I(0,1);
//    izx = massProperties->_I(0,2);
//    iyz = massProperties->_I(1,2);
    ixx = massProperties->I[(0)*4+(0)];
    iyy = massProperties->I[(1)*4+(1)];
    izz = massProperties->I[(2)*4+(2)];
    ixy = massProperties->I[(0)*4+(1)];
    izx = massProperties->I[(0)*4+(2)];
    iyz = massProperties->I[(1)*4+(2)];

    ang = 2*acos(quaternion[0]);
    double magnitude = sqrt(SQUARE(quaternion[1]) + SQUARE(quaternion[2]) + SQUARE(quaternion[3]));
    if (magnitude <= 1e-10)
    {
        std::cerr << "Vector magnitude too low in Body::ParallelAxis\n";
    }
    ax = quaternion[1] / magnitude;
    ay = quaternion[2] / magnitude;
    az = quaternion[3] / magnitude;

    ParallelAxis(x, y, z, mass, ixx, iyy, izz, ixy, iyz, izx, ang, ax, ay, az, &ixxp, &iyyp, &izzp, &ixyp, &iyzp, &izxp);

    dMassSetParameters (newMassProperties, mass, 0, 0, 0, ixxp, iyyp, izzp, ixyp, izxp, iyzp);
}

// a utility function to calculate moments of interia given an arbitrary translation and rotation
void Body::ParallelAxis(double x, double y, double z, // transformation from centre of mass to new location (m)
                        double mass, // mass (kg)
                        double ixx, double iyy, double izz, double ixy, double iyz, double izx, // moments of inertia kgm2
                        double ang, // rotation angle (radians)
                        double ax, double ay, double az, // axis of rotation - must be unit length
                        double *ixxp, double *iyyp, double *izzp, double *ixyp, double *iyzp, double *izxp) // transformed moments of inertia about new coordinate system
{
    double cosang = cos(ang);
    double sinang = sin(ang);

    *ixxp = -(mass*(-(y*y) - (z*z))) + ((ax*ax)*(1 - cosang) + cosang)*
            (ixx*((ax*ax)*(1 - cosang) + cosang) + izx*(ax*az*(1 - cosang) + ay*sinang) +
             ixy*(ax*ay*(1 - cosang) - az*sinang)) + (ax*ay*(1 - cosang) - az*sinang)*
            (ixy*((ax*ax)*(1 - cosang) + cosang) + iyz*(ax*az*(1 - cosang) + ay*sinang) +
             iyy*(ax*ay*(1 - cosang) - az*sinang)) + (ax*az*(1 - cosang) + ay*sinang)*
            (izx*((ax*ax)*(1 - cosang) + cosang) + izz*(ax*az*(1 - cosang) + ay*sinang) +
             iyz*(ax*ay*(1 - cosang) - az*sinang));

    *iyyp = -(mass*(-(x*x) - (z*z))) + (ax*ay*(1 - cosang) + az*sinang)*
            (ixy*((ay*ay)*(1 - cosang) + cosang) + izx*(ay*az*(1 - cosang) - ax*sinang) +
             ixx*(ax*ay*(1 - cosang) + az*sinang)) + ((ay*ay)*(1 - cosang) + cosang)*
            (iyy*((ay*ay)*(1 - cosang) + cosang) + iyz*(ay*az*(1 - cosang) - ax*sinang) +
             ixy*(ax*ay*(1 - cosang) + az*sinang)) + (ay*az*(1 - cosang) - ax*sinang)*
            (iyz*((ay*ay)*(1 - cosang) + cosang) + izz*(ay*az*(1 - cosang) - ax*sinang) +
             izx*(ax*ay*(1 - cosang) + az*sinang));

    *izzp = -(mass*(-(x*x) - (y*y))) + (ax*az*(1 - cosang) - ay*sinang)*
            (izx*((az*az)*(1 - cosang) + cosang) + ixy*(ay*az*(1 - cosang) + ax*sinang) +
             ixx*(ax*az*(1 - cosang) - ay*sinang)) + (ay*az*(1 - cosang) + ax*sinang)*
            (iyz*((az*az)*(1 - cosang) + cosang) + iyy*(ay*az*(1 - cosang) + ax*sinang) +
             ixy*(ax*az*(1 - cosang) - ay*sinang)) + ((az*az)*(1 - cosang) + cosang)*
            (izz*((az*az)*(1 - cosang) + cosang) + iyz*(ay*az*(1 - cosang) + ax*sinang) +
             izx*(ax*az*(1 - cosang) - ay*sinang));

    *ixyp = -(mass*x*y) + (ax*ay*(1 - cosang) + az*sinang)*
            (ixx*((ax*ax)*(1 - cosang) + cosang) + izx*(ax*az*(1 - cosang) + ay*sinang) +
             ixy*(ax*ay*(1 - cosang) - az*sinang)) + ((ay*ay)*(1 - cosang) + cosang)*
            (ixy*((ax*ax)*(1 - cosang) + cosang) + iyz*(ax*az*(1 - cosang) + ay*sinang) +
             iyy*(ax*ay*(1 - cosang) - az*sinang)) + (ay*az*(1 - cosang) - ax*sinang)*
            (izx*((ax*ax)*(1 - cosang) + cosang) + izz*(ax*az*(1 - cosang) + ay*sinang) +
             iyz*(ax*ay*(1 - cosang) - az*sinang));

    *iyzp = -(mass*y*z) + (ax*az*(1 - cosang) - ay*sinang)*
            (ixy*((ay*ay)*(1 - cosang) + cosang) + izx*(ay*az*(1 - cosang) - ax*sinang) +
             ixx*(ax*ay*(1 - cosang) + az*sinang)) + (ay*az*(1 - cosang) + ax*sinang)*
            (iyy*((ay*ay)*(1 - cosang) + cosang) + iyz*(ay*az*(1 - cosang) - ax*sinang) +
             ixy*(ax*ay*(1 - cosang) + az*sinang)) + ((az*az)*(1 - cosang) + cosang)*
            (iyz*((ay*ay)*(1 - cosang) + cosang) + izz*(ay*az*(1 - cosang) - ax*sinang) +
             izx*(ax*ay*(1 - cosang) + az*sinang));

    *izxp = -(mass*x*z) + (ax*az*(1 - cosang) - ay*sinang)*
            (ixx*((ax*ax)*(1 - cosang) + cosang) + izx*(ax*az*(1 - cosang) + ay*sinang) +
             ixy*(ax*ay*(1 - cosang) - az*sinang)) + (ay*az*(1 - cosang) + ax*sinang)*
            (ixy*((ax*ax)*(1 - cosang) + cosang) + iyz*(ax*az*(1 - cosang) + ay*sinang) +
             iyy*(ax*ay*(1 - cosang) - az*sinang)) + ((az*az)*(1 - cosang) + cosang)*
            (izx*((ax*ax)*(1 - cosang) + cosang) + izz*(ax*az*(1 - cosang) + ay*sinang) +
             iyz*(ax*ay*(1 - cosang) - az*sinang));
}

// returns zero if position values are simply mirror images of each other
// also checks mass properties
//int Body::SanityCheck(Body *otherBody, Simulation::AxisType axis, const std::string & /*sanityCheckLeft */, const std::string & /* sanityCheckRight */)
//{
//    const double epsilon = 1e-10;
//    const double *p1 = this->GetPosition();
//    const double *p2 = otherBody->GetPosition();

//    switch (axis)
//    {
//    case Simulation::XAxis:
//        if (fabs(p1[0] + p2[0]) > epsilon) return __LINE__;
//        if (fabs(p1[1] - p2[1]) > epsilon) return __LINE__;
//        if (fabs(p1[2] - p2[2]) > epsilon) return __LINE__;
//        break;

//    case Simulation::YAxis:
//        if (fabs(p1[0] - p2[0]) > epsilon) return __LINE__;
//        if (fabs(p1[1] + p2[1]) > epsilon) return __LINE__;
//        if (fabs(p1[2] - p2[2]) > epsilon) return __LINE__;
//        break;

//    case Simulation::ZAxis:
//        if (fabs(p1[0] - p2[0]) > epsilon) return __LINE__;
//        if (fabs(p1[1] - p2[1]) > epsilon) return __LINE__;
//        if (fabs(p1[2] + p2[2]) > epsilon) return __LINE__;
//        break;
//    }

//    int i;
//    dMass mass1, mass2;
//    dBodyGetMass(m_bodyID, &mass1);
//    dBodyGetMass(otherBody->GetBodyID(), &mass2);

//    if (fabs(mass1.mass - mass2.mass) > epsilon) return __LINE__;
//    for (i=0; i<3; i++) if (fabs(mass1.I[i] - mass2.I[i]) > epsilon) return __LINE__;
//    for (i=4; i<7; i++) if (fabs(mass1.I[i] - mass2.I[i]) > epsilon) return __LINE__;
//    for (i=8; i<11; i++) if (fabs(mass1.I[i] - mass2.I[i]) > epsilon) return __LINE__;

//    return 0;
//}

std::string Body::MassCheck(const dMass *m)
{
    if (m->mass <= 0)
    {
        return "mass must be > 0"s;
    }
    if (!dIsPositiveDefinite (m->I,3))
    {
        return "inertia must be positive definite"s;
    }

    // verify that the center of mass position is consistent with the mass
    // and inertia matrix. this is done by checking that the inertia around
    // the center of mass is also positive definite. from the comment in
    // dMassTranslate(), if the body is translated so that its center of mass
    // is at the point of reference, then the new inertia is:
    //   I + mass*crossmat(c)^2
    // note that requiring this to be positive definite is exactly equivalent
    // to requiring that the spatial inertia matrix
    //   [ mass*eye(3,3)   M*crossmat(c)^T ]
    //   [ M*crossmat(c)   I               ]
    // is positive definite, given that I is PD and mass>0. see the theorem
    // about partitioned PD matrices for proof.

    dMatrix3 I2,chat;
    dSetZero (chat,12);
    dSetCrossMatrixPlus (chat,m->c,4);
    dMultiply0_333 (I2,chat,chat);
    for (int i=0; i<3; i++) I2[i] = m->I[i] + m->mass*I2[i];
    for (int i=4; i<7; i++) I2[i] = m->I[i] + m->mass*I2[i];
    for (int i=8; i<11; i++) I2[i] = m->I[i] + m->mass*I2[i];
    if (!dIsPositiveDefinite (I2,3))
    {
        return "center of mass inconsistent with mass parameters"s;
    }
    return ""s;
}

void Body::SetLinearDamping(double linearDamping)
{
    m_LinearDamping = linearDamping;
    dBodySetLinearDamping(m_bodyID, linearDamping);
}

void Body::SetAngularDamping(double angularDamping)
{
    m_AngularDamping = angularDamping;
    dBodySetAngularDamping(m_bodyID, angularDamping);
}

void Body::SetLinearDampingThreshold(double linearDampingThreshold)
{
    m_LinearDampingThreshold = linearDampingThreshold;
    dBodySetLinearDampingThreshold(m_bodyID, linearDampingThreshold);
}

void Body::SetAngularDampingThreshold(double angularDampingThreshold)
{
    m_AngularDampingThreshold = angularDampingThreshold;
    dBodySetAngularDampingThreshold(m_bodyID, angularDampingThreshold);
}

void Body::SetMaxAngularSpeed(double maxAngularSpeed)
{
    m_MaxAngularSpeed = maxAngularSpeed;
    dBodySetMaxAngularSpeed(m_bodyID, maxAngularSpeed);
}

#ifdef EXPERIMENTAL

void Body::SetCylinderDragParameters(DragControl dragAxis, double dragFluidDensity, double dragCylinderMin, double dragCylinderMax, double dragCylinderRadius, double dragCylinderCoefficient)
{
    // drag parameters
    m_dragControl = dragAxis;
    m_dragFluidDensity = dragFluidDensity;
    m_dragCylinderMin = dragCylinderMin;
    m_dragCylinderLength = dragCylinderMax - dragCylinderMin;
    m_dragCylinderRadius = dragCylinderRadius;
    m_dragCylinderCoefficient = dragCylinderCoefficient;
}

void Body::SetDirectDragCoefficients(double linearDragCoefficientX, double linearDragCoefficientY, double linearDragCoefficientZ,
                                     double rotationalDragCoefficientX, double rotationalDragCoefficientY, double rotationalDragCoefficientZ)
{
    m_dragControl = DragCoefficients;
    m_dragCoefficients[0] = rotationalDragCoefficientX;
    m_dragCoefficients[1] = rotationalDragCoefficientY;
    m_dragCoefficients[2] = rotationalDragCoefficientZ;
    m_dragCoefficients[3] = linearDragCoefficientX;
    m_dragCoefficients[4] = linearDragCoefficientY;
    m_dragCoefficients[5] = linearDragCoefficientZ;
}

// this routine modifed from Dynamechs 4.0 by Scott McMillan
void Body::ComputeDrag()
{
    const double gqx[4] = {0.069431844,     // Gauss-quadrature constants.
                           0.330009478,
                           0.669990521,
                           0.930568155};
    const double gqk[4] = {0.1739274225687,
                           0.3260725774312,
                           0.3260725774312,
                           0.1739274225687};

    if (m_dragControl == NoDrag) return;
    int k;

    double f_D[6] = {0, 0, 0, 0, 0, 0}; // angular follwed by linear
    double v_rel[6]; // angular follwed by linear

    const double *aVel = dBodyGetAngularVel(m_bodyID);
    dBodyVectorFromWorld (m_bodyID, aVel[0], aVel[1], aVel[2], v_rel);

    const double *lVel = dBodyGetLinearVel(m_bodyID);
    dBodyVectorFromWorld (m_bodyID, lVel[0], lVel[1], lVel[2], &(v_rel[3]));


    if (m_dragControl == DragCoefficients)
    {
        for (k = 0; k < 6; k++)
        {
            f_D[k] = -m_dragCoefficients[k]*v_rel[k]*fabs(v_rel[k]);
        }
    }
    else
    {

        double area = M_PI * m_dragCylinderRadius * m_dragCylinderRadius;
        double C2 = m_dragFluidDensity * m_dragCylinderCoefficient;
        double Ca = 0.5 * C2 * area;
        double C2rl = C2 * m_dragCylinderRadius * m_dragCylinderLength;
        double vn[3], tem[3];
        double x, tmp, vn_mag;

        if (m_dragControl == DragCylinderX)
        {
            // x-axis moment and force
            f_D[3] = -Ca*v_rel[3]*fabs(v_rel[3]);

            // y,z components - using Gauss-quadrature
            for (k = 0; k < 4; k++)
            {
                x = m_dragCylinderMin + m_dragCylinderLength*gqx[k];
                vn[1] = v_rel[4] + v_rel[2]*x;
                vn[2] = v_rel[5] - v_rel[1]*x;

                vn_mag = sqrt(vn[1]*vn[1] + vn[2]*vn[2]);

                tmp = gqk[k]*vn_mag;
                tem[1] = tmp*vn[1];
                tem[2] = tmp*vn[2];

                f_D[1] += (-x*tem[2]);
                f_D[2] += x*tem[1];
                f_D[4] += tem[1];
                f_D[5] += tem[2];
            }
            f_D[1] *= -C2rl;
            f_D[2] *= -C2rl;
            f_D[4] *= -C2rl;
            f_D[5] *= -C2rl;
        }

        else if (m_dragControl == DragCylinderY)
        {
            // y-axis moment and force
            f_D[4] = -Ca*v_rel[4]*fabs(v_rel[4]);

            // z,x components
            for (k = 0; k < 4; k++)
            {
                x = m_dragCylinderMin + m_dragCylinderLength*gqx[k];
                vn[0] = v_rel[3] - v_rel[2]*x;
                vn[2] = v_rel[5] + v_rel[0]*x;

                vn_mag = sqrt(vn[2]*vn[2] + vn[0]*vn[0]);

                tmp = gqk[k]*vn_mag;
                tem[0] = tmp*vn[0];
                tem[2] = tmp*vn[2];

                f_D[0] += (x*tem[2]);
                f_D[2] += -x*tem[0];
                f_D[3] += tem[0];
                f_D[5] += tem[2];
            }
            f_D[0] *= -C2rl;
            f_D[2] *= -C2rl;
            f_D[3] *= -C2rl;
            f_D[5] *= -C2rl;

        }

        else if (m_dragControl == DragCylinderZ)
        {
            // z-axis moment and force
            f_D[5] = -Ca*v_rel[5]*fabs(v_rel[5]);

            // y,x components
            for (k = 0; k < 4; k++)
            {
                x = m_dragCylinderMin + m_dragCylinderLength*gqx[k];
                vn[0] = v_rel[3] + v_rel[1]*x;
                vn[1] = v_rel[4] - v_rel[0]*x;

                vn_mag = sqrt(vn[1]*vn[1] + vn[0]*vn[0]);

                tmp = gqk[k]*vn_mag;
                tem[0] = tmp*vn[0];
                tem[1] = tmp*vn[1];

                f_D[0] += (-x*tem[1]);
                f_D[1] += x*tem[0];
                f_D[3] += tem[0];
                f_D[4] += tem[1];
            }
            f_D[0] *= -C2rl;
            f_D[1] *= -C2rl;
            f_D[3] *= -C2rl;
            f_D[4] *= -C2rl;

        }

    }

    // drag forces are now in f_D (3 rotational followed by 3 linear)
    // need to add them to the body
    dBodyAddRelTorque(m_bodyID, f_D[0], f_D[1], f_D[2]);
    dBodyAddRelForce(m_bodyID, f_D[3], f_D[4], f_D[5]);

}

#endif

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *Body::createFromAttributes()
{
    if (NamedObject::createFromAttributes()) return lastErrorPtr();

    bool constructionMode = m_constructionMode;
    if (constructionMode) EnterRunMode();

    std::string buf;
    double doubleList[6];

    // initial position/quaternion first then construction position/quaternion
    if (findAttribute("Position"s, &buf) == nullptr) return lastErrorPtr();
    this->SetPosition(buf);
    std::copy_n(this->GetPosition(), 4, m_initialPosition);

    if (findAttribute("ConstructionPosition"s, &buf) == nullptr) return lastErrorPtr();
    this->SetPosition(buf); // set the position first and copy the values
    std::copy_n(this->GetPosition(), 4, m_constructionPosition);

    if (findAttribute("Quaternion"s, &buf) == nullptr) return lastErrorPtr(); // note quaternion is (qs,qx,qy,qz)
    this->SetQuaternion(buf);
    std::copy_n(this->GetQuaternion(), 4, m_initialQuaternion);
    this->SetQuaternion(1, 0, 0, 0); // construction quaternion is always zero

    if (findAttribute("LinearVelocity"s, &buf) == nullptr) return lastErrorPtr();
    this->SetLinearVelocity(buf);

    if (findAttribute("AngularVelocity"s, &buf) == nullptr) return lastErrorPtr();
    this->SetAngularVelocity(buf);

    // and now the mass properties
    // (remember the origin is always at the centre of mass)

    if (findAttribute("Mass"s, &buf) == nullptr) return lastErrorPtr();
    double theMass = GSUtil::Double(buf);

    if (findAttribute("MOI"s, &buf) == nullptr) return lastErrorPtr();
    GSUtil::Double(buf, 6, doubleList);

    dMass mass;
    // note: inertial matrix is as follows
    // [ I11 I12 I13 ]   [ xx xy xz ]
    // [ I12 I22 I23 ] = [ yx yy yz ]
    // [ I13 I23 I33 ]   [ zx zy zz ]
//    double I11, I22, I33, I12, I13, I23;
//    I11 = doubleList[0];
//    I22 = doubleList[1];
//    I33 = doubleList[2];
//    I12 = doubleList[3];
//    I13 = doubleList[4];
//    I23 = doubleList[5];
//    dMassSetParameters(&mass, theMass, 0, 0, 0, I11, I22, I33, I12, I13, I23);
    dMassSetParameters(&mass, theMass, 0, 0, 0, doubleList[0], doubleList[1], doubleList[2], doubleList[3], doubleList[4], doubleList[5]);
    buf = MassCheck(&mass);
    if (buf.size())
    {
        setLastError("BODY ID=\""s + name() +"\" mass property error: " + buf);
        return lastErrorPtr();
    }
    this->SetMass(&mass);

    // get limits if available
    if (findAttribute("PositionLowBound"s, &buf))
    {
        GSUtil::Double(buf, 3, doubleList);
        this->SetPositionLowBound(doubleList[0], doubleList[1], doubleList[2]);
    }
    if (findAttribute("PositionHighBound"s, &buf))
    {
        GSUtil::Double(buf, 3, doubleList);
        this->SetPositionHighBound(doubleList[0], doubleList[1], doubleList[2]);
    }
    if (findAttribute("LinearVelocityLowBound"s, &buf))
    {
        GSUtil::Double(buf, 3, doubleList);
        this->SetLinearVelocityLowBound(doubleList[0], doubleList[1], doubleList[2]);
    }
    if (findAttribute("LinearVelocityHighBound"s, &buf))
    {
        GSUtil::Double(buf, 3, doubleList);
        this->SetLinearVelocityHighBound(doubleList[0], doubleList[1], doubleList[2]);
    }
    if (findAttribute("AngularVelocityLowBound"s, &buf))
    {
        GSUtil::Double(buf, 3, doubleList);
        this->SetAngularVelocityLowBound(doubleList[0], doubleList[1], doubleList[2]);
    }
    if (findAttribute("AngularVelocityHighBound"s, &buf))
    {
        GSUtil::Double(buf, 3, doubleList);
        this->SetAngularVelocityHighBound(doubleList[0], doubleList[1], doubleList[2]);
    }

    if (findAttribute("ConstructionDensity"s, &buf) == nullptr) return lastErrorPtr();
    this->SetConstructionDensity(GSUtil::Double(buf));

    // set damping if necessary
    if (findAttribute("LinearDamping"s, &buf)) this->SetLinearDamping(GSUtil::Double(buf));
    if (findAttribute("AngularDamping"s, &buf)) this->SetAngularDamping(GSUtil::Double(buf));
    if (findAttribute("LinearDampingThreshold"s, &buf)) this->SetLinearDampingThreshold(GSUtil::Double(buf));
    if (findAttribute("AngularDampingThreshold"s, &buf)) this->SetAngularDampingThreshold(GSUtil::Double(buf));
    if (findAttribute("MaxAngularSpeed"s, &buf)) this->SetMaxAngularSpeed(GSUtil::Double(buf));

#ifdef EXPERIMENTAL
    if (findAttribute("DragControl"s, &buf))
    {
        if (buf == "NoDrag"s) m_dragControl = NoDrag;
        else if (buf == "DragCoefficients"s) m_dragControl = DragCoefficients;
        else if (buf == "DragCylinderX"s) m_dragControl = DragCylinderX;
        else if (buf == "DragCylinderY"s) m_dragControl = DragCylinderY;
        else if (buf == "DragCylinderZ"s) m_dragControl = DragCylinderZ;
        else
        {
            setLastError("BODY ID=\""s + name() +"\" Unrecognised DragControl \""s + buf + "\""s);
            return lastErrorPtr();
        }
    }
    if (m_dragControl == DragCoefficients)
    {
        if (findAttribute("DirectDragCoefficients"s, &buf) == nullptr) return lastErrorPtr();
        GSUtil::Double(buf, 6, doubleList);
        this->SetDirectDragCoefficients(doubleList[0], doubleList[1], doubleList[2], doubleList[3], doubleList[4], doubleList[5]); // 3 rotational then 3 linear
    }
    else if (m_dragControl == DragCylinderX || m_dragControl == DragCylinderY || m_dragControl == DragCylinderZ)
    {
        if (findAttribute("DragFluidDensity"s, &buf) == nullptr) return lastErrorPtr();
        double dragFluidDensity = GSUtil::Double(buf);
        if (findAttribute("DragCylinderMin"s, &buf) == nullptr) return lastErrorPtr();
        double dragCylinderMin = GSUtil::Double(buf);
        if (findAttribute("DragCylinderMax"s, &buf) == nullptr) return lastErrorPtr();
        double dragCylinderMax = GSUtil::Double(buf);
        if (findAttribute("DragCylinderRadius"s, &buf) == nullptr) return lastErrorPtr();
        double dragCylinderRadius = GSUtil::Double(buf);
        if (findAttribute("DragCylinderCoefficient"s, &buf) == nullptr) return lastErrorPtr();
        double dragCylinderCoefficient = GSUtil::Double(buf);
        this->SetCylinderDragParameters(m_dragControl, dragFluidDensity, dragCylinderMin, dragCylinderMax, dragCylinderRadius, dragCylinderCoefficient);
    }

#endif

    if (findAttribute("GraphicFile1"s, &buf)) this->SetGraphicFile1(buf);
    if (findAttribute("GraphicFile2"s, &buf)) this->SetGraphicFile2(buf);
    if (findAttribute("GraphicFile3"s, &buf)) this->SetGraphicFile3(buf);

    if (constructionMode) EnterConstructionMode();

    return nullptr;
}

// this function copies the data in the object to an xml_node node that it creates internally.
// doc is used to allocate the memory so deletion should be automatic
void Body::saveToAttributes()
{
    this->setTag("BODY"s);
    this->clearAttributeMap();
    this->appendToAttributes();
}

void Body::appendToAttributes()
{
    NamedObject::appendToAttributes();
    std::string buf;

    bool constructionMode = m_constructionMode;
    if (constructionMode) EnterRunMode();

    dMass mass;
    dBodyGetMass (m_bodyID, &mass);
    setAttribute("Mass"s, *GSUtil::ToString(mass.mass, &buf));
    // note: inertial matrix is as follows
    // [ I11 I12 I13 ]   [ xx xy xz ]
    // [ I12 I22 I23 ] = [ yx yy yz ]
    // [ I13 I23 I33 ]   [ zx zy zz ]
    double I11 = mass.I[(0)*4+(0)];
    double I22 = mass.I[(1)*4+(1)];
    double I33 = mass.I[(2)*4+(2)];
    double I12 = mass.I[(0)*4+(1)];
    double I13 = mass.I[(0)*4+(2)];
    double I23 = mass.I[(1)*4+(2)];
    double MOI[6] = {I11, I22, I33, I12, I13, I23}; // xx, yy, zz, xy, xz, yz
    setAttribute("MOI"s, *GSUtil::ToString(MOI, 6, &buf));
    if (m_LinearDamping >= 0) setAttribute("LinearDamping"s, *GSUtil::ToString(dBodyGetLinearDamping(m_bodyID), &buf));
    if (m_AngularDamping >= 0) setAttribute("AngularDamping"s, *GSUtil::ToString(dBodyGetAngularDamping(m_bodyID), &buf));
    if (m_LinearDampingThreshold >= 0) setAttribute("LinearDampingThreshold"s, *GSUtil::ToString(dBodyGetLinearDampingThreshold(m_bodyID), &buf));
    if (m_AngularDampingThreshold >= 0) setAttribute("AngularDampingThreshold"s, *GSUtil::ToString(dBodyGetAngularDampingThreshold(m_bodyID), &buf));
    if (m_MaxAngularSpeed >= 0) setAttribute("MaxAngularSpeed"s, *GSUtil::ToString(dBodyGetMaxAngularSpeed(m_bodyID), &buf));

    const double *q = dBodyGetQuaternion(m_bodyID);
    setAttribute("Quaternion"s, *GSUtil::ToString(q, 4, &buf)); // note quaternion is (qs,qx,qy,qz)
    const double *p = dBodyGetPosition(m_bodyID);
    setAttribute("Position"s, *GSUtil::ToString(p, 3, &buf));

    const double *v = dBodyGetLinearVel(m_bodyID);
    setAttribute("LinearVelocity"s, *GSUtil::ToString(v, 3, &buf));
    const double *a = dBodyGetAngularVel(m_bodyID);
    setAttribute("AngularVelocity"s, *GSUtil::ToString(a, 3, &buf));

    setAttribute("ConstructionPosition"s, *GSUtil::ToString(m_constructionPosition, 3, &buf));
    setAttribute("ConstructionDensity"s, *GSUtil::ToString(m_constructionDensity, &buf));

    setAttribute("PositionLowBound"s, *GSUtil::ToString(m_positionLowBound, 3, &buf));
    setAttribute("PositionHighBound"s, *GSUtil::ToString(m_positionHighBound, 3, &buf));
    setAttribute("LinearVelocityLowBound"s, *GSUtil::ToString(m_linearVelocityLowBound, 3, &buf));
    setAttribute("LinearVelocityHighBound"s, *GSUtil::ToString(m_linearVelocityHighBound, 3, &buf));
    setAttribute("AngularVelocityLowBound"s, *GSUtil::ToString(m_angularVelocityLowBound, 3, &buf));
    setAttribute("AngularVelocityHighBound"s, *GSUtil::ToString(m_angularVelocityHighBound, 3, &buf));

    setAttribute("GraphicFile1"s, m_graphicFile1);
    setAttribute("GraphicFile2"s, m_graphicFile2);
    setAttribute("GraphicFile3"s, m_graphicFile3);

    #ifdef EXPERIMENTAL
    if (m_dragControl == NoDrag) setAttribute("DragControl"s, "NoDrag"s);
    else if (m_dragControl == DragCoefficients) setAttribute("DragControl"s, "DragCoefficients"s);
    else if (m_dragControl == DragCylinderX) setAttribute("DragControl"s, "DragCylinderX"s);
    else if (m_dragControl == DragCylinderY) setAttribute("DragControl"s, "DragCylinderY"s);
    else if (m_dragControl == DragCylinderZ) setAttribute("DragControl"s, "DragCylinderZ"s);
    if (m_dragControl != NoDrag)
    {
        setAttribute("DragCoefficients"s, *GSUtil::ToString(m_dragCoefficients, 6, &buf));
        setAttribute("DragFluidDensity"s, *GSUtil::ToString(m_dragFluidDensity, &buf));
        setAttribute("DragCylinderMin"s, *GSUtil::ToString(m_dragCylinderMin, &buf));
        setAttribute("DragCylinderLength"s, *GSUtil::ToString(m_dragCylinderLength, &buf));
        setAttribute("DragCylinderRadius"s, *GSUtil::ToString(m_dragCylinderRadius, &buf));
        setAttribute("DragCylinderCoefficient"s, *GSUtil::ToString(m_dragCylinderCoefficient, &buf));
    }
    #endif

    if (constructionMode) EnterConstructionMode();
}

void Body::LateInitialisation()
{
    this->SetPosition(m_initialPosition[0], m_initialPosition[1], m_initialPosition[2]);
    this->SetQuaternion(m_initialQuaternion[0], m_initialQuaternion[1], m_initialQuaternion[2], m_initialQuaternion[3]);
}

void Body::EnterConstructionMode()
{
    m_constructionMode = true;
    this->SetPosition(m_constructionPosition[0], m_constructionPosition[1], m_constructionPosition[2]);
    this->SetQuaternion(m_constructionQuaternion[0], m_constructionQuaternion[1], m_constructionQuaternion[2], m_constructionQuaternion[3]);
}

void Body::EnterRunMode()
{
    m_constructionMode = false;
    this->SetPosition(m_initialPosition[0], m_initialPosition[1], m_initialPosition[2]);
    this->SetQuaternion(m_initialQuaternion[0], m_initialQuaternion[1], m_initialQuaternion[2], m_initialQuaternion[3]);
}

void Body::SetInitialPosition(double x, double y, double z)
{
    m_initialPosition[0] = x;
    m_initialPosition[1] = y;
    m_initialPosition[2] = z;
}

void Body::SetInitialQuaternion(double n, double x, double y, double z)
{
    m_initialQuaternion[0] = n;
    m_initialQuaternion[1] = x;
    m_initialQuaternion[2] = y;
    m_initialQuaternion[3] = z;
}

const double *Body::GetInitialPosition()
{
    return m_initialPosition;
}

const double *Body::GetInitialQuaternion()
{
    return m_initialQuaternion;
}
