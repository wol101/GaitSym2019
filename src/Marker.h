/*
 *  Marker.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 22/08/2009.
 *  Copyright 2009 Bill Sellers. All rights reserved.
 *
 */

#ifndef Marker_h
#define Marker_h

#include "NamedObject.h"
#include "PGDMath.h"
#include "SmartEnum.h"

#include <set>

class Body;

class Marker: public NamedObject
{
public:

    Marker(Body *body);

    // these functions set the geom position relative to its body
    void SetPosition(double x, double y, double z);
    void SetQuaternion(double qs0, double qx1, double qy2, double qz3);
    std::string *SetPosition (const std::string &buf);
    std::string *SetPosition(const std::string &body, double x, double y, double z);
    std::string *SetWorldPosition(double x, double y, double z);
    std::string *SetQuaternion(const std::string &buf);
    std::string *SetQuaternion(const std::string &body, double qs0, double qx1, double qy2, double qz3);
    std::string *SetWorldQuaternion(double qs0, double qx1, double qy2, double qz3);
    void OffsetPosition(double x, double y, double z);

    SMART_ENUM(Axis, axisStrings, axisCount, X, Y, Z);
//    enum Axis { X = 0, Y = 1, Z = 2 };

    pgd::Vector3 GetPosition() const;
    pgd::Quaternion GetQuaternion() const;
    pgd::Vector3 GetAxis(Marker::Axis axis) const;
    void GetBasis(pgd::Vector3 *x, pgd::Vector3 *y, pgd::Vector3 *z);
    pgd::Vector3 GetWorldPosition() const;
    pgd::Quaternion GetWorldQuaternion() const;
    pgd::Vector3 GetWorldAxis(Marker::Axis axis) const;
    void GetWorldBasis(pgd::Vector3 *x, pgd::Vector3 *y, pgd::Vector3 *z);
    pgd::Vector3 GetWorldLinearVelocity();
    pgd::Vector3 GetWorldAngularVelocity();

    // these functions get things into and out of marker based coordinates
    pgd::Vector3 GetPosition(const pgd::Vector3 &worldCoordinates) const;
    pgd::Vector3 GetWorldPosition(const pgd::Vector3 &localCoordinates) const;
    pgd::Vector3 GetVector(const pgd::Vector3 &worldVector) const;
    pgd::Vector3 GetWorldVector(const pgd::Vector3 &localVector) const;
    pgd::Quaternion GetWorldQuaternion(const pgd::Quaternion &localQuaternion) const;
    pgd::Quaternion GetQuaternion(const pgd::Quaternion &worldQuaternion) const;


    virtual std::string dumpToString();
    virtual std::string *createFromAttributes();
    virtual void saveToAttributes();
    virtual void appendToAttributes();

    Body *GetBody() const;
    void SetBody(Body *body);

private:

    Body *m_body = nullptr; // if nullptr then this is the World, otherwise a pre-existing body
    pgd::Vector3 m_position; // this is the position with respect to m_body (which can be World)
    pgd::Quaternion m_quaternion = {1, 0, 0, 0}; // this is the orientation with respect to m_body (which can be World)
};


#endif
