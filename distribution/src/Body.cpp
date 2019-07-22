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
#include <math.h>
#include <algorithm>

#ifdef MALLOC_H_NEEDED
#include <malloc.h>
#endif
#ifdef ALLOCA_H_NEEDED
#include <alloca.h>
#endif

using namespace std::string_literals;

// length of vector a
#define LENGTHOF(a) \
        sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2])

#define LENGTH2OF(a) \
        (a[0]*a[0]+a[1]*a[1]+a[2]*a[2])

#define OUTSIDERANGE(x, minX, maxX) \
        ((x) < (minX) || (x) > (maxX))

#define PSWAP(a,b,t) {t tmp; tmp=a; a=b; b=tmp;}

Body::Body(dWorldID worldID)
{
    m_bodyID = dBodyCreate (worldID);
    dBodySetData(m_bodyID, this);
    m_worldID = worldID;
}

Body::~Body()
{
    dBodyDestroy (m_bodyID);
}

void Body::SetPosition(double x, double y, double z)
{
    dBodySetPosition(m_bodyID, x, y, z);
}

void Body::SetQuaternion(double q0, double q1, double q2, double q3)
{
    dQuaternion q;
    q[0] = q0;
    q[1] = q1;
    q[2] = q2;
    q[3] = q3;
    dBodySetQuaternion(m_bodyID, q);
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
            setLastError("Body ID=\""s + GetName() +"\" Position=\""s + buf + "\" marker not found"s);
            return lastErrorPtr();
        }
        pgd::Vector wp = marker->GetWorldPosition();
        this->SetPosition(wp.x, wp.y, wp.z);
        return nullptr;
    }

    if (tokens.size() == 2) // two marker definition
    {
        Marker *marker1 = simulation()->GetMarker(tokens[0]);
        Marker *marker2 = simulation()->GetMarker(tokens[1]);
        if (marker1 == nullptr || marker2 == nullptr)
        {
            setLastError("Body ID=\""s + GetName() +"\" Position=\""s + buf + "\" markers not found"s);
            return lastErrorPtr();
        }
        // find the marker that is relative to this body and make it marker1
        if (marker2->GetBody()->GetBodyID() == m_bodyID) PSWAP(marker1, marker2, Marker *);
        if (marker2->GetBody()->GetBodyID() == m_bodyID)
        {
            setLastError("Body ID=\""s + GetName() +"\" Position=\""s + buf + "\" Only one marker of pair must be relative to the body"s);
            return lastErrorPtr();
        }
        if (marker1->GetBody()->GetBodyID() != m_bodyID)
        {
            setLastError("Body ID=\""s + GetName() +"\" Position=\""s + buf + "\" Only one marker of pair must be relative to the body"s);
            return lastErrorPtr();
        }

        pgd::Vector target = marker2->GetWorldPosition();
        pgd::Vector current = marker2->GetWorldPosition();
        pgd::Vector difference = target - current;
        this->SetPosition(difference.x, difference.y, difference.z);
        return nullptr;
    }

    if (tokens.size() == 3)
    {
        this->SetPosition(std::stod(tokens[0]), std::stod(tokens[1]), std::stod(tokens[2]));
        return nullptr;
    }

    if (tokens.size() == 4)
    {
        Body *theBody = simulation()->GetBody(tokens[0]);
        if (theBody == nullptr)
        {
            if (tokens[0] == "World"s)
            {
                this->SetPosition(std::stod(tokens[1]), std::stod(tokens[2]), std::stod(tokens[3]));
                return nullptr;
            }
            else
            {
                setLastError("Body ID=\""s + GetName() +"\" Position=\""s + buf + "\" reference body not found"s);
                return lastErrorPtr();
            }
        }
        else
        {
            dVector3 result;
            dBodyGetRelPointPos (theBody->GetBodyID(), std::stod(tokens[1]), std::stod(tokens[2]), std::stod(tokens[3]), result);
            this->SetPosition(result[0], result[1], result[2]);
            return nullptr;
        }
    }
    if (tokens.size() == 7)
    {
        Body *theBody = simulation()->GetBody(tokens[0]);
        if (!theBody)
        {
            setLastError("Body ID=\""s + GetName() +"\" Position=\""s + buf + "\" reference body not found"s);
            return lastErrorPtr();
        }
        // get world coordinates of x1,y1,z1
        dVector3 world1, world2, pos;
        dBodyGetRelPointPos (theBody->GetBodyID(), std::stod(tokens[1]), std::stod(tokens[2]), std::stod(tokens[3]), world1);
        dBodyGetRelPointPos (m_bodyID, std::stod(tokens[4]), std::stod(tokens[5]), std::stod(tokens[6]), world2);
        // add the error to the current position
        const double *p = dBodyGetPosition(m_bodyID);
        for (size_t i = 0; i < 3; i++) pos[i] = p[i] + (world1[i] - world2[i]);
        this->SetPosition(pos[0], pos[1], pos[2]);

        // for checking
        // dBodyGetRelPointPos (m_BodyID, local2[0], local2[1], local2[2], world2);
        // for (i = 0; i < 3; i++) std::cerr << world1[i] << " " << world2[i] << "\n";
        return nullptr;
    }

    setLastError("Body ID=\""s + GetName() +"\" Position=\""s + buf + "\" too many tokens"s);
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
            setLastError("Body ID=\""s + GetName() +"\" Quaternion=\""s + buf + "\" Cannot find marker"s);
            return lastErrorPtr();
        }
        pgd::Quaternion wq = marker->GetWorldQuaternion();
        this->SetQuaternion(wq.n, wq.v.x, wq.v.y, wq.v.z);
        return nullptr;
    }

    if (tokens.size() == 4)
    {
        pgd::Quaternion wq = GSUtil::GetQuaternion(tokens, 0);
        this->SetQuaternion(wq.n, wq.v.x, wq.v.y, wq.v.z);
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
                this->SetQuaternion(wq.n, wq.v.x, wq.v.y, wq.v.z);
                return nullptr;
            }
            else
            {
                setLastError("Body ID=\""s + GetName() +"\" Quaternion=\""s + buf + "\" Cannot find body"s);
                return lastErrorPtr();
            }
        }
        const double *q = theBody->GetQuaternion();
        pgd::Quaternion qBody(q[0], q[1], q[2], q[3]);
        pgd::Quaternion qIn = GSUtil::GetQuaternion(tokens, 1);
        pgd::Quaternion qNew = qBody * qIn;
        this->SetQuaternion(qNew.n, qNew.v.x, qNew.v.y, qNew.v.z);
        return nullptr;
    }
    setLastError("Body ID=\""s + GetName() +"\" Position=\""s + buf + "\" wrong number of tokens"s);
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
        this->SetLinearVelocity(std::stod(tokens[0]), std::stod(tokens[1]), std::stod(tokens[2]));
        return nullptr;
    }

    if (tokens.size() == 4)
    {
        Body *theBody = simulation()->GetBody(tokens[0]);
        if (theBody == nullptr)
        {
            if (tokens[0] == "World"s)
            {
                this->SetLinearVelocity(std::stod(tokens[1]), std::stod(tokens[2]), std::stod(tokens[3]));
                return nullptr;
            }
            else
            {
                setLastError("Body ID=\""s + GetName() +"\" LinearVelocity=\""s + buf + "\" Cannot find body"s);
                return lastErrorPtr();
            }
        }
        else
        {
            dVector3 result;
            dBodyVectorToWorld(theBody->GetBodyID(), std::stod(tokens[1]), std::stod(tokens[2]), std::stod(tokens[3]), result);
            const double *vRel = dBodyGetLinearVel(theBody->GetBodyID());
            SetLinearVelocity(result[0] + vRel[0], result[1] + vRel[1], result[2] + vRel[2]);
            return nullptr;
        }
    }
    setLastError("Body ID=\""s + GetName() +"\" LinearVelocity=\""s + buf + "\" wrong number of tokens"s);
    return lastErrorPtr();
}

double Body::GetLinearKineticEnergy()
{
    // linear KE = 0.5 m v^2
    dMass mass;
    dBodyGetMass(m_bodyID, &mass);

    const double *v = dBodyGetLinearVel(m_bodyID);
    double linearKE = 0.5 * mass.mass * LENGTH2OF(v);

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
    dMULTIPLY0_331(o1, mass.I, o);
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
        SetAngularVelocity(std::stod(tokens[0]), std::stod(tokens[1]), std::stod(tokens[2]));
        return nullptr;
    }

    if (tokens.size() == 4)
    {
        Body *theBody = simulation()->GetBody(tokens[0]);
        if (theBody == nullptr)
        {
            if (tokens[0] == "World"s)
            {
                SetAngularVelocity(std::stod(tokens[1]), std::stod(tokens[2]), std::stod(tokens[3]));
                return nullptr;
            }
            else
            {
                setLastError("Body ID=\""s + GetName() +"\" AngularVelocity=\""s + buf + "\" body not found"s);
                return lastErrorPtr();
            }
        }
        else
        {
            dVector3 result;
            dBodyVectorToWorld(theBody->GetBodyID(), std::stod(tokens[1]), std::stod(tokens[2]), std::stod(tokens[3]), result);
            const double *vARel = dBodyGetAngularVel(theBody->GetBodyID());
            SetAngularVelocity(result[0] + vARel[0], result[1] + vARel[1], result[2] + vARel[2]);
            return nullptr;
        }
    }
    setLastError("Body ID=\""s + GetName() +"\" AngularVelocity=\""s + buf + "\" wrong number of tokens"s);
    return lastErrorPtr();

}

void Body::SetMass(const dMass *mass)
{
    dBodySetMass(m_bodyID, mass);
}

const double *Body::GetPosition()
{
    return dBodyGetPosition(m_bodyID);
}

const double *Body::GetQuaternion()
{
    return dBodyGetQuaternion(m_bodyID);
}

const double *Body::GetRotation()
{
    return dBodyGetRotation (m_bodyID);
}

const double *Body::GetLinearVelocity()
{
    return dBodyGetLinearVel(m_bodyID);
}

const double *Body::GetAngularVelocity()
{
    return dBodyGetAngularVel(m_bodyID);
}

void Body::GetRelativePosition(Body *rel, pgd::Vector *pos)
{
    dVector3 result;
    const double *p = GetPosition();
    if (rel)
    {
        dBodyGetPosRelPoint(rel->GetBodyID(), p[0], p[1], p[2], result);
        *pos = pgd::Vector(result[0], result[1], result[2]);
    }
    else
    {
        *pos = pgd::Vector(p[0], p[1], p[2]);
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

void Body::GetRelativeRotation(Body *rel, pgd::Matrix3x3 *rot)
{
    const double *rW = GetRotation();
    if (rel)
    {
        const double *rR = rel->GetRotation();
        pgd::Matrix3x3 rWorld(rW[0], rW[1], rW[2], rW[4], rW[5], rW[6], rW[8], rW[9], rW[10]);
        pgd::Matrix3x3 rRelBody(rR[0], rR[1], rR[2], rR[4], rR[5], rR[6], rR[8], rR[9], rR[10]);
        *rot = rRelBody.Inverse() * rWorld;
    }
    else
    {
        *rot = pgd::Matrix3x3(rW[0], rW[1], rW[2], rW[4], rW[5], rW[6], rW[8], rW[9], rW[10]);
    }
}

void Body::GetRelativeLinearVelocity(Body *rel, pgd::Vector *vel)
{
    const double *v = GetLinearVelocity();
    if (rel)
    {
        const double *qR = rel->GetQuaternion();
        const double *vR = rel->GetLinearVelocity();
        pgd::Vector worldV(v[0], v[1], v[2]);
        pgd::Vector relV(vR[0], vR[1], vR[2]);
        pgd::Quaternion qRelBody(qR[0], qR[1], qR[2], qR[3]);
        *vel = QVRotate(~qRelBody, worldV - relV);
    }
    else
    {
        *vel = pgd::Vector(v[0], v[1], v[2]);
    }
}

void Body::GetRelativeAngularVelocity(Body *rel, pgd::Vector *rVel)
{
    const double *vR = GetAngularVelocity();
    if (rel)
    {
        const double *qR = rel->GetQuaternion();
        const double *vRR = rel->GetAngularVelocity();
        pgd::Vector worldVR(vR[0], vR[1], vR[2]);
        pgd::Vector relVR(vRR[0], vRR[1], vRR[2]);
        pgd::Quaternion qRelBody(qR[0], qR[1], qR[2], qR[3]);
        *rVel = QVRotate(~qRelBody, worldVR - relVR);
    }
    else
    {
        *rVel = pgd::Vector(vR[0], vR[1], vR[2]);
    }
}

double Body::GetMass()
{
    dMass mass;
    dBodyGetMass(m_bodyID, &mass);
    return mass.mass;
}

void Body::GetMass(dMass *mass)
{
    dBodyGetMass(m_bodyID, mass);
    return;
}

Body::LimitTestResult Body::TestLimits()
{
    const double *p = dBodyGetPosition(m_bodyID);
    if (!std::isfinite(p[0])) return NumericalError;
    if (!std::isfinite(p[1])) return NumericalError;
    if (!std::isfinite(p[2])) return NumericalError;
    if (OUTSIDERANGE(p[0], m_positionLowBound[0], m_positionHighBound[0])) return XPosError;
    if (OUTSIDERANGE(p[1], m_positionLowBound[1], m_positionHighBound[1])) return YPosError;
    if (OUTSIDERANGE(p[2], m_positionLowBound[2], m_positionHighBound[2])) return ZPosError;

    const double *v = dBodyGetLinearVel(m_bodyID);
    if (!std::isfinite(v[0])) return NumericalError;
    if (!std::isfinite(v[1])) return NumericalError;
    if (!std::isfinite(v[2])) return NumericalError;
    if (OUTSIDERANGE(v[0], m_linearVelocityLowBound[0], m_linearVelocityHighBound[0])) return XVelError;
    if (OUTSIDERANGE(v[1], m_linearVelocityLowBound[1], m_linearVelocityHighBound[1])) return YVelError;
    if (OUTSIDERANGE(v[2], m_linearVelocityLowBound[2], m_linearVelocityHighBound[2])) return ZVelError;

    return WithinLimits;
}

void Body::Dump()
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
            *dumpStream() << "Time\tXP\tYP\tZP\tXV\tYV\tZV\tQW\tQX\tQY\tQZ\tRVX\tRVY\tRVZ\tLKEX\tLKEY\tLKEZ\tRKE\tGPE\n";
        }
    }


    if (dumpStream())
    {
        const double *p = GetPosition();
        const double *v = GetLinearVelocity();
        const double *q = GetQuaternion();
        const double *rv = GetAngularVelocity();
        dVector3 ke;
        GetLinearKineticEnergy(ke);

        *dumpStream() << simulation()->GetTime() << "\t" << p[0] << "\t" << p[1] << "\t" << p[2] <<
                "\t" << v[0] << "\t" << v[1] << "\t" << v[2] <<
                "\t" << q[0] << "\t" << q[1] << "\t" << q[2] << "\t" << q[3] <<
                "\t" << rv[0] << "\t" << rv[1] << "\t" << rv[2] <<
                "\t" << ke[0] << "\t" << ke[1] << "\t" << ke[2] <<
                "\t" << GetRotationalKineticEnergy() << "\t" << GetGravitationalPotentialEnergy() <<
                "\n";
    }
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
int Body::SanityCheck(Body *otherBody, Simulation::AxisType axis, const std::string & /*sanityCheckLeft */, const std::string & /* sanityCheckRight */)
{
    const double epsilon = 1e-10;
    const double *p1 = this->GetPosition();
    const double *p2 = otherBody->GetPosition();

    switch (axis)
    {
    case Simulation::XAxis:
        if (fabs(p1[0] + p2[0]) > epsilon) return __LINE__;
        if (fabs(p1[1] - p2[1]) > epsilon) return __LINE__;
        if (fabs(p1[2] - p2[2]) > epsilon) return __LINE__;
        break;

    case Simulation::YAxis:
        if (fabs(p1[0] - p2[0]) > epsilon) return __LINE__;
        if (fabs(p1[1] + p2[1]) > epsilon) return __LINE__;
        if (fabs(p1[2] - p2[2]) > epsilon) return __LINE__;
        break;

    case Simulation::ZAxis:
        if (fabs(p1[0] - p2[0]) > epsilon) return __LINE__;
        if (fabs(p1[1] - p2[1]) > epsilon) return __LINE__;
        if (fabs(p1[2] + p2[2]) > epsilon) return __LINE__;
        break;
    }

    int i;
    dMass mass1, mass2;
    dBodyGetMass(m_bodyID, &mass1);
    dBodyGetMass(otherBody->GetBodyID(), &mass2);

    if (fabs(mass1.mass - mass2.mass) > epsilon) return __LINE__;
    for (i=0; i<3; i++) if (fabs(mass1.I[i] - mass2.I[i]) > epsilon) return __LINE__;
    for (i=4; i<7; i++) if (fabs(mass1.I[i] - mass2.I[i]) > epsilon) return __LINE__;
    for (i=8; i<11; i++) if (fabs(mass1.I[i] - mass2.I[i]) > epsilon) return __LINE__;

    return 0;
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


// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *Body::CreateFromAttributes()
{
    if (NamedObject::CreateFromAttributes()) return lastErrorPtr();

    std::string buf;
    double doubleList[6];

    // initial position/quaternion first then construction position/quaternion
    if (GetAttribute("Position"s, &buf) == nullptr) return lastErrorPtr();
    this->SetPosition(buf);
    std::copy_n(this->GetPosition(), 4, m_initialPosition);

    if (GetAttribute("ConstructionPosition"s, &buf) == nullptr) return lastErrorPtr();
    this->SetPosition(buf); // set the position first and copy the values
    std::copy_n(this->GetPosition(), 4, m_constructionPosition);

    if (GetAttribute("Quaternion"s, &buf) == nullptr) return lastErrorPtr(); // note quaternion is (qs,qx,qy,qz)
    this->SetQuaternion(buf);
    std::copy_n(this->GetQuaternion(), 4, m_initialQuaternion);
    this->SetQuaternion(1, 0, 0, 0); // construction quaternion is always zero

    if (GetAttribute("LinearVelocity"s, &buf) == nullptr) return lastErrorPtr();
    this->SetLinearVelocity(buf);

    if (GetAttribute("AngularVelocity"s, &buf) == nullptr) return lastErrorPtr();
    this->SetAngularVelocity(buf);

    // and now the mass properties
    // (remember the origin is always at the centre of mass)

    if (GetAttribute("Mass"s, &buf) == nullptr) return lastErrorPtr();
    double theMass = GSUtil::Double(buf);

    if (GetAttribute("MOI"s, &buf) == nullptr) return lastErrorPtr();
    GSUtil::Double(buf, 6, doubleList);

    dMass mass;
    // note: inertial matrix is as follows
    // [ I11 I12 I13 ]
    // [ I12 I22 I23 ]
    // [ I13 I23 I33 ]
//    double I11, I22, I33, I12, I13, I23;
//    I11 = doubleList[0];
//    I22 = doubleList[1];
//    I33 = doubleList[2];
//    I12 = doubleList[3];
//    I13 = doubleList[4];
//    I23 = doubleList[5];
//    dMassSetParameters(&mass, theMass, 0, 0, 0, I11, I22, I33, I12, I13, I23);
    dMassSetParameters(&mass, theMass, 0, 0, 0, doubleList[0], doubleList[1], doubleList[2], doubleList[3], doubleList[4], doubleList[5]);
    this->SetMass(&mass);

    // get limits if available
    if (GetAttribute("PositionLowBound"s, &buf))
    {
        GSUtil::Double(buf, 3, doubleList);
        this->SetPositionLowBound(doubleList[0], doubleList[1], doubleList[2]);
    }
    if (GetAttribute("PositionHighBound"s, &buf))
    {
        GSUtil::Double(buf, 3, doubleList);
        this->SetPositionHighBound(doubleList[0], doubleList[1], doubleList[2]);
    }
    if (GetAttribute("LinearVelocityLowBound"s, &buf))
    {
        GSUtil::Double(buf, 3, doubleList);
        this->SetLinearVelocityLowBound(doubleList[0], doubleList[1], doubleList[2]);
    }
    if (GetAttribute("LinearVelocityHighBound"s, &buf))
    {
        GSUtil::Double(buf, 3, doubleList);
        this->SetLinearVelocityHighBound(doubleList[0], doubleList[1], doubleList[2]);
    }

    if (GetAttribute("ConstructionDensity"s, &buf) == nullptr) return lastErrorPtr();
    this->SetConstructionDensity(GSUtil::Double(buf));

    // set damping if necessary
    if (GetAttribute("LinearDamping"s, &buf)) this->SetLinearDamping(GSUtil::Double(buf));
    if (GetAttribute("AngularDamping"s, &buf)) this->SetAngularDamping(GSUtil::Double(buf));
    if (GetAttribute("LinearDampingThreshold"s, &buf)) this->SetLinearDampingThreshold(GSUtil::Double(buf));
    if (GetAttribute("AngularDampingThreshold"s, &buf)) this->SetAngularDampingThreshold(GSUtil::Double(buf));
    if (GetAttribute("MaxAngularSpeed"s, &buf)) this->SetMaxAngularSpeed(GSUtil::Double(buf));


    if (GetAttribute("GraphicFile1"s, &buf)) this->SetGraphicFile1(buf);
    if (GetAttribute("GraphicFile2"s, &buf)) this->SetGraphicFile2(buf);
    if (GetAttribute("GraphicFile3"s, &buf)) this->SetGraphicFile3(buf);

    return nullptr;
}

// this function copies the data in the object to an xml_node node that it creates internally.
// doc is used to allocate the memory so deletion should be automatic
void Body::SaveToAttributes()
{
    this->setTag("BODY"s);
    this->AppendToAttributes();
}

void Body::AppendToAttributes()
{
    NamedObject::AppendToAttributes();
    std::string buf;

    dMass mass;
    dBodyGetMass (m_bodyID, &mass);
    setAttribute("Mass"s, *GSUtil::ToString(mass.mass, &buf));
    double I11 = mass.I[(0)*4+(0)];
    double I22 = mass.I[(1)*4+(1)];
    double I33 = mass.I[(2)*4+(2)];
    double I12 = mass.I[(0)*4+(1)];
    double I13 = mass.I[(0)*4+(2)];
    double I23 = mass.I[(1)*4+(2)];
    double MOI[6] = {I11, I22, I33, I12, I13, I23};
    setAttribute("MOI"s, *GSUtil::ToString(MOI, 6, &buf));
    if (m_LinearDamping >= 0) setAttribute("LinearDamping"s, *GSUtil::ToString(dBodyGetLinearDamping(m_bodyID), &buf));
    if (m_AngularDamping >= 0) setAttribute("AngularDamping"s, *GSUtil::ToString(dBodyGetAngularDamping(m_bodyID), &buf));
    if (m_LinearDampingThreshold >= 0) setAttribute("LinearDampingThreshold"s, *GSUtil::ToString(dBodyGetLinearDampingThreshold(m_bodyID), &buf));
    if (m_AngularDampingThreshold >= 0) setAttribute("AngularDampingThreshold"s, *GSUtil::ToString(dBodyGetAngularDampingThreshold(m_bodyID), &buf));
    if (m_MaxAngularSpeed >= 0) setAttribute("MaxAngularSpeed"s, *GSUtil::ToString(dBodyGetMaxAngularSpeed(m_bodyID), &buf));

    const dReal *q = dBodyGetQuaternion(m_bodyID);
    setAttribute("Quaternion"s, *GSUtil::ToString(q, 4, &buf)); // note quaternion is (qs,qx,qy,qz)
    const dReal *p = dBodyGetPosition(m_bodyID);
    setAttribute("Position"s, *GSUtil::ToString(p, 3, &buf));
    const dReal *v = dBodyGetLinearVel(m_bodyID);
    setAttribute("LinearVelocity"s, *GSUtil::ToString(v, 3, &buf));
    const dReal *a = dBodyGetAngularVel(m_bodyID);
    setAttribute("AngularVelocity"s, *GSUtil::ToString(a, 3, &buf));

    setAttribute("ConstructionPosition"s, *GSUtil::ToString(m_constructionPosition, 3, &buf));
    setAttribute("ConstructionDensity"s, *GSUtil::ToString(m_constructionDensity, &buf));

    setAttribute("PositionLowBound"s, *GSUtil::ToString(m_positionLowBound, 3, &buf));
    setAttribute("PositionHighBound"s, *GSUtil::ToString(m_positionHighBound, 3, &buf));
    setAttribute("LinearVelocityLowBound"s, *GSUtil::ToString(m_linearVelocityLowBound, 3, &buf));
    setAttribute("LinearVelocityHighBound"s, *GSUtil::ToString(m_linearVelocityHighBound, 3, &buf));

    setAttribute("GraphicFile1"s, m_graphicFile1);
    setAttribute("GraphicFile2"s, m_graphicFile2);
    setAttribute("GraphicFile3"s, m_graphicFile3);

}

void Body::LateInitialisation()
{
    this->SetPosition(m_initialPosition[0], m_initialPosition[1], m_initialPosition[2]);
    this->SetQuaternion(m_initialQuaternion[0], m_initialQuaternion[1], m_initialQuaternion[2], m_initialQuaternion[3]);
}

void Body::EnterConstructionMode()
{
    this->SetPosition(m_constructionPosition[0], m_constructionPosition[1], m_constructionPosition[2]);
    this->SetQuaternion(m_constructionQuaternion[0], m_constructionQuaternion[1], m_constructionQuaternion[2], m_constructionQuaternion[3]);

}

void Body::EnterRunMode()
{
    this->SetPosition(m_initialPosition[0], m_initialPosition[1], m_initialPosition[2]);
    this->SetQuaternion(m_initialQuaternion[0], m_initialQuaternion[1], m_initialQuaternion[2], m_initialQuaternion[3]);

}

