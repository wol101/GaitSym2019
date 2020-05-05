/*
 *  Marker.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 22/08/2009.
 *  Copyright 2009 Bill Sellers. All rights reserved.
 *
 */

#include "Marker.h"
#include "PGDMath.h"
#include "DataFile.h"
#include "Simulation.h"
#include "Body.h"
#include "GSUtil.h"

#include "ode/ode.h"

#include "pystring.h"

#include <iostream>
#include <sstream>

using namespace std::string_literals;

Marker::Marker(Body *body)
{
    m_body = body;
}

Marker::~Marker()
{
}

void Marker::SetPosition (double x, double y, double z)
{
    m_position.x = x; m_position.y = y; m_position.z = z;
}

void Marker::SetQuaternion(double qs0, double qx1, double qy2, double qz3)
{
    m_quaternion.n = qs0;
    m_quaternion.v.x = qx1; m_quaternion.v.y = qy2; m_quaternion.v.z = qz3;
}

// parses the position allowing a relative position specified by BODY ID
// x y z - body coordinates
// bodyName x y z - position relative to bodyName local coordinate system
// bodyName can be "World"
std::string *Marker::SetPosition(const std::string &buf)
{
    dVector3 result;
    std::vector<std::string> tokens;
    pystring::split(buf, tokens);
    if (tokens.size() < 3 || tokens.size() > 4)
    {
        setLastError("Marker ID=\""s + name() +"\" Position=\""s + buf + "\" needs 3 or 4 tokens"s);
        return lastErrorPtr();
    }
    if (tokens.size() == 3)
    {
        SetPosition(GSUtil::Double(tokens[0]), GSUtil::Double(tokens[1]), GSUtil::Double(tokens[2]));
        return nullptr;
    }

    if (tokens[0] == "World"s)
    {
        if (m_body)
        {
            dBodyGetPosRelPoint(m_body->GetBodyID(), GSUtil::Double(tokens[1]), GSUtil::Double(tokens[2]), GSUtil::Double(tokens[3]), result); // convert from world to body
            SetPosition(result[0], result[1], result[2]);
        }
        else
        {
            SetPosition(GSUtil::Double(tokens[1]), GSUtil::Double(tokens[2]), GSUtil::Double(tokens[3]));
        }
        return nullptr;
    }

    Body *theBody = simulation()->GetBody(tokens[0]);
    if (!theBody)
    {
        setLastError("Marker ID=\""s + name() +"\" Position=\""s + buf + "\" body not found"s);
        return lastErrorPtr();
    }
    dBodyGetRelPointPos(theBody->GetBodyID(), GSUtil::Double(tokens[1]), GSUtil::Double(tokens[2]), GSUtil::Double(tokens[3]), result); // convert from body to world
    if (m_body)
    {
        dVector3 pos;
        dBodyGetPosRelPoint(m_body->GetBodyID(), result[0], result[1], result[2], pos); // convert from world to body
        SetPosition(pos[0], pos[1], pos[2]);
    }
    else
    {
        SetPosition(result[0], result[1], result[2]);
    }
    return nullptr;
}

// parses the position allowing a relative position specified by BODY ID
// x y z - body coordinates
// bodyName x y z - position relative to bodyName local coordinate system
// bodyName can be "World"
std::string *Marker::SetPosition(const std::string &body, double x, double y, double z)
{
    dVector3 result;
    if (body == "World"s)
    {
        if (m_body)
        {
            dBodyGetPosRelPoint(m_body->GetBodyID(), x, y, z, result); // convert from world to body
            SetPosition(result[0], result[1], result[2]);
        }
        else
        {
            SetPosition(x, y, z);
        }
        return nullptr;
    }

    Body *theBody = simulation()->GetBody(body);
    if (!theBody)
    {
        setLastError("Marker ID=\""s + name() +"\" Position=\""s + body + "\" body not found"s);
        return lastErrorPtr();
    }
    dBodyGetRelPointPos(theBody->GetBodyID(), x, y, z, result); // convert from body to world
    if (m_body)
    {
        dVector3 pos;
        dBodyGetPosRelPoint(m_body->GetBodyID(), result[0], result[1], result[2], pos); // convert from world to body
        SetPosition(pos[0], pos[1], pos[2]);
    }
    else
    {
        SetPosition(result[0], result[1], result[2]);
    }
    return nullptr;
}

std::string *Marker::SetWorldPosition(double x, double y, double z)
{
    dVector3 result;
    if (m_body)
    {
        dBodyGetPosRelPoint(m_body->GetBodyID(), x, y, z, result); // convert from world to body
        SetPosition(result[0], result[1], result[2]);
    }
    else
    {
        SetPosition(x, y, z);
    }
    return nullptr;
}

// parses the quaternion allowing a relative position specified by BODY ID
// note quaternion is (qs,qx,qy,qz)
// s x y z - body coordinates
// bodyName s x y z - position relative to bodyName local coordinate system
// bodyName can be "World"
std::string *Marker::SetQuaternion(const std::string &buf)
{
    std::vector<std::string> tokens;
    pystring::split(buf, tokens);
    if (tokens.size() < 4 || tokens.size() > 5)
    {
        setLastError("Marker ID=\""s + name() +"\" Quaternion=\""s + buf + "\" needs 4 or 5 tokens"s);
        return lastErrorPtr();
    }
    if (tokens.size() == 4)
    {
        pgd::Quaternion q = GSUtil::GetQuaternion(tokens, 0);
        SetQuaternion(q.n, q.v.x, q.v.y, q.v.z);
        return nullptr;
    }

    if (tokens[0] == "World"s)
    {
        if (m_body)
        {
            const double *q = dBodyGetQuaternion(m_body->GetBodyID());
            pgd::Quaternion qBody(q[0], q[1], q[2], q[3]);
            pgd::Quaternion qWorld = GSUtil::GetQuaternion(tokens, 1);
            pgd::Quaternion qLocal = ~qBody * qWorld;
            SetQuaternion(qLocal.n, qLocal.v.x, qLocal.v.y, qLocal.v.z);
        }
        else
        {
            pgd::Quaternion q = GSUtil::GetQuaternion(tokens, 1);
            SetQuaternion(q.n, q.v.x, q.v.y, q.v.z);
        }
        return nullptr;
    }

    Body *theBody = simulation()->GetBody(tokens[0]);
    if (!theBody)
    {
        setLastError("Marker ID=\""s + name() +"\" Quaternion=\""s + buf + "\" body not found"s);
        return lastErrorPtr();
    }

    // first get world quaternion
    const double *q2 = theBody->GetQuaternion();
    pgd::Quaternion qBody1(q2[0], q2[1], q2[2], q2[3]);
    pgd::Quaternion qBody2 = GSUtil::GetQuaternion(tokens, 1);
    pgd::Quaternion qWorld = qBody1 * qBody2;

    // then set the local quaternion
    if (m_body)
    {
        const double *q = dBodyGetQuaternion(m_body->GetBodyID());
        pgd::Quaternion qBody(q[0], q[1], q[2], q[3]);
        pgd::Quaternion qLocal = ~qBody * qWorld;
        SetQuaternion(qLocal.n, qLocal.v.x, qLocal.v.y, qLocal.v.z);
    }
    else
    {
        SetQuaternion(qWorld.n, qWorld.v.x, qWorld.v.y, qWorld.v.z);
    }
    return nullptr;
}

// parses the quaternion allowing a relative position specified by BODY ID
// note quaternion is (qs,qx,qy,qz)
// s x y z - body coordinates
// bodyName s x y z - position relative to bodyName local coordinate system
// bodyName can be "World"
std::string *Marker::SetQuaternion(const std::string &body, double qs0, double qx1, double qy2, double qz3)
{
    if (body == "World"s)
    {
        if (m_body)
        {
            const double *q = dBodyGetQuaternion(m_body->GetBodyID());
            pgd::Quaternion qBody(q[0], q[1], q[2], q[3]);
            pgd::Quaternion qWorld(qs0, qx1, qy2, qz3);
            pgd::Quaternion qLocal = ~qBody * qWorld;
            SetQuaternion(qLocal.n, qLocal.v.x, qLocal.v.y, qLocal.v.z);
        }
        else
        {
            SetQuaternion(qs0, qx1, qy2, qz3);
        }
        return nullptr;
    }

    Body *theBody = simulation()->GetBody(body);
    if (!theBody)
    {
        setLastError("Marker ID=\""s + name() +"\" Quaternion=\""s + body + "\" body not found"s);
        return lastErrorPtr();
    }

    // first get world quaternion
    const double *q2 = theBody->GetQuaternion();
    pgd::Quaternion qBody1(q2[0], q2[1], q2[2], q2[3]);
    pgd::Quaternion qBody2(qs0, qx1, qy2, qz3);
    pgd::Quaternion qWorld = qBody1 * qBody2;

    // then set the local quaternion
    if (m_body)
    {
        const double *q = dBodyGetQuaternion(m_body->GetBodyID());
        pgd::Quaternion qBody(q[0], q[1], q[2], q[3]);
        pgd::Quaternion qLocal = ~qBody * qWorld;
        SetQuaternion(qLocal.n, qLocal.v.x, qLocal.v.y, qLocal.v.z);
    }
    else
    {
        SetQuaternion(qWorld.n, qWorld.v.x, qWorld.v.y, qWorld.v.z);
    }
    return nullptr;
}

std::string *Marker::SetWorldQuaternion(double qs0, double qx1, double qy2, double qz3)
{
    if (m_body)
    {
        const double *q = dBodyGetQuaternion(m_body->GetBodyID());
        pgd::Quaternion qBody(q[0], q[1], q[2], q[3]);
        pgd::Quaternion qWorld(qs0, qx1, qy2, qz3);
        pgd::Quaternion qLocal = ~qBody * qWorld;
        SetQuaternion(qLocal.n, qLocal.v.x, qLocal.v.y, qLocal.v.z);
    }
    else
    {
        SetQuaternion(qs0, qx1, qy2, qz3);
    }
    return nullptr;
}


pgd::Vector Marker::GetPosition() const
{
    return m_position;
}

pgd::Quaternion Marker::GetQuaternion() const
{
    return m_quaternion;
}

pgd::Vector Marker::GetAxis(Marker::Axis axis) const
{
    switch (axis)
    {
    case Marker::Axis::X:
        return pgd::QVRotate(this->GetQuaternion(), pgd::Vector(1, 0, 0));
    case Marker::Axis::Y:
        return pgd::QVRotate(this->GetQuaternion(), pgd::Vector(0, 1, 0));
    case Marker::Axis::Z:
        return pgd::QVRotate(this->GetQuaternion(), pgd::Vector(0, 0, 1));
    }
    return pgd::Vector(1, 0, 0); // just to stop warnings
}

void Marker::GetBasis(pgd::Vector *x, pgd::Vector *y, pgd::Vector *z)
{
    pgd::Matrix3x3 m(this->GetQuaternion());
    x->x = m.e11;
    x->y = m.e21;
    x->z = m.e31;
    y->x = m.e12;
    y->y = m.e22;
    y->z = m.e32;
    z->x = m.e13;
    z->y = m.e23;
    z->z = m.e33;
}

pgd::Vector Marker::GetWorldPosition() const
{
    if (m_body)
    {
        // get the position in world coordinates
        dVector3 p;
        dBodyGetRelPointPos(m_body->GetBodyID(), m_position.x, m_position.y, m_position.z, p);
        return pgd::Vector(p[0], p[1], p[2]);
    }
    else
    {
        return m_position;
    }
}

pgd::Vector Marker::GetWorldPosition(const pgd::Vector &localCoordinates) const
{
    pgd::Vector worldDelta = pgd::QVRotate(GetWorldQuaternion(), localCoordinates);
    return GetWorldPosition() + worldDelta;
}

pgd::Vector Marker::GetPosition(const pgd::Vector &worldCoordinates) const
{
    pgd::Vector worldDelta = worldCoordinates - GetWorldPosition();
    return pgd::QVRotate(~GetWorldQuaternion(), worldDelta);
}

pgd::Vector Marker::GetWorldVector(const pgd::Vector &localVector) const
{
    return pgd::QVRotate(GetWorldQuaternion(), localVector);
}

pgd::Vector Marker::GetVector(const pgd::Vector &worldVector) const
{
    return pgd::QVRotate(~GetWorldQuaternion(), worldVector);
}

pgd::Quaternion Marker::GetWorldQuaternion(const pgd::Quaternion &localQuaternion) const
{
    return GetWorldQuaternion() * localQuaternion;
}

pgd::Quaternion Marker::GetQuaternion(const pgd::Quaternion &worldQuaternion) const

{
    return (~GetWorldQuaternion()) * worldQuaternion;
}

pgd::Vector Marker::GetWorldVelocity()
{
    if (m_body)
    {
        // get the velocity in world coordinates
        dVector3 p;
        dBodyGetRelPointVel(m_body->GetBodyID(), m_position.x, m_position.y, m_position.z, p);
        return pgd::Vector(p[0], p[1], p[2]);
    }
    else
    {
        return pgd::Vector();
    }
}

pgd::Quaternion Marker::GetWorldQuaternion() const
{
    if (m_body)
    {
        const double *bodyRotation = dBodyGetQuaternion(m_body->GetBodyID());
        pgd::Quaternion bodyQuaternion(bodyRotation[0], bodyRotation[1], bodyRotation[2], bodyRotation[3]);
        return bodyQuaternion * m_quaternion;
    }
    else
    {
        return m_quaternion;
    }
}

pgd::Vector Marker::GetWorldAxis(Marker::Axis axis) const
{
    switch (axis)
    {
    case Marker::Axis::X:
        return pgd::QVRotate(this->GetWorldQuaternion(), pgd::Vector(1, 0, 0));
    case Marker::Axis::Y:
        return pgd::QVRotate(this->GetWorldQuaternion(), pgd::Vector(0, 1, 0));
    case Marker::Axis::Z:
        return pgd::QVRotate(this->GetWorldQuaternion(), pgd::Vector(0, 0, 1));
    }
    return pgd::Vector(1, 0, 0); // just to stop warnings
}

void Marker::GetWorldBasis(pgd::Vector *x, pgd::Vector *y, pgd::Vector *z)
{
    pgd::Matrix3x3 m(this->GetWorldQuaternion());
    x->x = m.e11;
    x->y = m.e21;
    x->z = m.e31;
    y->x = m.e12;
    y->y = m.e22;
    y->z = m.e32;
    z->x = m.e13;
    z->y = m.e23;
    z->z = m.e33;
}

std::string Marker::dump()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (getFirstDump())
    {
        setFirstDump(false);
        ss << "Time\tXP\tYP\tZP\tQW\tQX\tQY\tQZ\n";
    }
    pgd::Vector p = GetWorldPosition();
    pgd::Quaternion q = GetWorldQuaternion();

    ss << simulation()->GetTime() << "\t" << p.x << "\t" << p.y << "\t" << p.z <<
          "\t" << q.n << "\t" << q.v.x << "\t" << q.v.y << "\t" << q.v.z << "\n";
    return ss.str();
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *Marker::createFromAttributes()
{
    if (NamedObject::createFromAttributes()) return lastErrorPtr();

    std::string buf;

    if (findAttribute("BodyID"s, &buf) == nullptr) return lastErrorPtr();
    if (buf != "World"s)
    {
        auto it = simulation()->GetBodyList()->find(buf);
        if (it == simulation()->GetBodyList()->end())
        {
            setLastError("Marker ID=\""s + name() +"\" BodyID=\""s + buf + "\" not found"s);
            return lastErrorPtr();
        }
        this->SetBody(it->second.get());
    }
    else
    {
        m_body = nullptr;
    }

    // note quaternion is (qs,qx,qy,qz)
    if (findAttribute("Quaternion"s, &buf) == nullptr) return lastErrorPtr();
    this->SetQuaternion(buf);

    // note position
    if (findAttribute("Position"s, &buf) == nullptr) return lastErrorPtr();
    this->SetPosition(buf);

    return nullptr;
}

// this function copies the data in the object to an xml_node node that it creates internally.
// doc is used to allocate the memory so deletion should be automatic
void Marker::saveToAttributes()
{
    this->setTag("MARKER"s);
    this->clearAttributeMap();
    this->appendToAttributes();
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void Marker::appendToAttributes()
{
    NamedObject::appendToAttributes();
    std::string buf;
    std::string bodyName;
    if (this->GetBody()) bodyName = this->GetBody()->name();
    else bodyName = "World"s;
    setAttribute("BodyID"s, bodyName);
    setAttribute("Quaternion"s, bodyName + " "s + *GSUtil::ToString(m_quaternion, &buf));
    setAttribute("Position"s, bodyName + " "s + *GSUtil::ToString(m_position, &buf));
    setAttribute("WorldQuaternion"s, *GSUtil::ToString(GetWorldQuaternion(), &buf));
    setAttribute("WorldPosition"s, *GSUtil::ToString(GetWorldPosition(), &buf));
}

Body *Marker::GetBody() const
{
    return m_body;
}

void Marker::SetBody(Body *body)
{
    m_body = body;
}

const std::set<NamedObject *> *Marker::dependentList() const
{
    return &m_dependentList;
}

void Marker::addDependent(NamedObject *namedObject)
{
    m_dependentList.insert(namedObject);
}

