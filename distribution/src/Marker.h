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

#include <set>

class Body;

class Marker: public NamedObject
{
public:

    Marker(Body *body);
    virtual ~Marker();

    // these functions set the geom position relative to its body
    void SetPosition (double x, double y, double z);
    void SetQuaternion(double qs0, double qx1, double qy2, double qz3);
    std::string *SetPosition (const std::string &buf);
    std::string *SetPosition(const std::string &body, double x, double y, double z);
    std::string *SetWorldPosition(double x, double y, double z);
    std::string *SetQuaternion(const std::string &buf);
    std::string *SetQuaternion(const std::string &body, double qs0, double qx1, double qy2, double qz3);
    std::string *SetWorldQuaternion(double qs0, double qx1, double qy2, double qz3);

    enum Axis { X, Y, Z };

    pgd::Vector GetPosition() const;
    pgd::Quaternion GetQuaternion() const;
    pgd::Vector GetAxis(Marker::Axis axis) const;
    void GetBasis(pgd::Vector *x, pgd::Vector *y, pgd::Vector *z);
    pgd::Vector GetWorldPosition() const;
    pgd::Quaternion GetWorldQuaternion() const;
    pgd::Vector GetWorldAxis(Marker::Axis axis) const;
    void GetWorldBasis(pgd::Vector *x, pgd::Vector *y, pgd::Vector *z);

    virtual void Dump();
    virtual std::string *CreateFromAttributes();
    virtual void SaveToAttributes();
    virtual void AppendToAttributes();

    Body *GetBody() const;
    void SetBody(Body *body);

    void addDependent(NamedObject *namedObject);
    const std::set<NamedObject *> *dependentList() const;

private:

    Body *m_body = nullptr; // if nullptr then this is the World, otherwise a pre-existing body
    pgd::Vector m_position; // this is the position with respect to m_body (which can be World)
    pgd::Quaternion m_quaternion = {1, 0, 0, 0}; // this is the orientation with respect to m_body (which can be World)
    std::set<NamedObject *> m_dependentList;
};


#endif
