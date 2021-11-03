/*
 *  TwoPointStrap.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 29/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

#include "TwoPointStrap.h"
#include "Body.h"
#include "PGDMath.h"
#include "DataFile.h"
#include "Simulation.h"
#include "Marker.h"
#include "GSUtil.h"

#include "ode/ode.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <string.h>
#include <algorithm>

using namespace std::string_literals;

TwoPointStrap::TwoPointStrap()
{
}

//void TwoPointStrap::SetOrigin(Body *body, const dVector3 point)
//{
//    m_originBody = body;
//    m_origin[0] = point[0];
//    m_origin[1] = point[1];
//    m_origin[2] = point[2];
//    if (GetPointForceList()->size() == 0)
//    {
//        std::unique_ptr<PointForce> origin = std::make_unique<PointForce>();
//        origin->body = m_originBody;
//        GetPointForceList()->push_back(std::move(origin));
//    }
//    else
//    {
//        GetPointForceList()->at(0)->body = m_originBody;
//    }
//}

//void TwoPointStrap::SetInsertion(Body *body, const dVector3 point)
//{
//    m_insertionBody = body;
//    m_insertion[0] = point[0];
//    m_insertion[1] = point[1];
//    m_insertion[2] = point[2];
//    if (GetPointForceList()->size() <= 1)
//    {
//        std::unique_ptr<PointForce> insertion = std::make_unique<PointForce>();
//        insertion->body = m_insertionBody;
//        GetPointForceList()->push_back(std::move(insertion));
//    }
//    else
//    {
//        GetPointForceList()->at(1)->body = m_insertionBody;
//    }
//}

//void TwoPointStrap::GetOrigin(const Body **body, dVector3 origin) const
//{
//    *body = m_originBody;
//    origin[0] = m_origin[0];
//    origin[1] = m_origin[1];
//    origin[2] = m_origin[2];
//}

//void TwoPointStrap::GetInsertion(const Body **body, dVector3 insertion) const
//{
//    *body = m_insertionBody;
//    insertion[0] = m_insertion[0];
//    insertion[1] = m_insertion[1];
//    insertion[2] = m_insertion[2];
//}

void TwoPointStrap::SetOrigin(Marker *originMarker)
{
    m_originMarker = originMarker;
//    this->SetOrigin(originMarker->GetBody(), originMarker->GetPosition().data());
    if (GetPointForceList()->size() == 0)
    {
        std::unique_ptr<PointForce> origin = std::make_unique<PointForce>();
        origin->body = m_originMarker->GetBody();
        GetPointForceList()->push_back(std::move(origin));
    }
    else
    {
        GetPointForceList()->at(0)->body = m_originMarker->GetBody();
    }
}

void TwoPointStrap::SetInsertion(Marker *insertionMarker)
{
    m_insertionMarker = insertionMarker;
//    this->SetInsertion(insertionMarker->GetBody(), insertionMarker->GetPosition().data());
    if (GetPointForceList()->size() <= 1)
    {
        std::unique_ptr<PointForce> insertion = std::make_unique<PointForce>();
        insertion->body = m_insertionMarker->GetBody();
        GetPointForceList()->push_back(std::move(insertion));
    }
    else
    {
        GetPointForceList()->at(1)->body =  m_insertionMarker->GetBody();
    }
}

void TwoPointStrap::Calculate()
{
    PointForce *theOrigin = (*GetPointForceList())[0].get();
    PointForce *theInsertion = (*GetPointForceList())[1].get();

    // calculate the world positions
//    dBodyGetRelPointPos(m_originBody->GetBodyID(), m_origin[0], m_origin[1], m_origin[2], theOrigin->point);
//    dBodyGetRelPointPos(m_insertionBody->GetBodyID(), m_insertion[0], m_insertion[1], m_insertion[2], theInsertion->point);
    pgd::Vector3 origin = m_originMarker->GetWorldPosition();
    theOrigin->point[0] = origin.x;
    theOrigin->point[1] = origin.y;
    theOrigin->point[2] = origin.z;
    pgd::Vector3 insertion = m_insertionMarker->GetWorldPosition();
    theInsertion->point[0] = insertion.x;
    theInsertion->point[1] = insertion.y;
    theInsertion->point[2] = insertion.z;

    // calculate the vector from the origin to the insertion
    dVector3 line;
    line[0] = theInsertion->point[0] - theOrigin->point[0];
    line[1] = theInsertion->point[1] - theOrigin->point[1];
    line[2] = theInsertion->point[2] - theOrigin->point[2];

    // calculate the length and velocity
    double length = std::sqrt(line[0]*line[0] + line[1]*line[1] + line[2]*line[2]);
    if (Length() >= 0 && simulation() && simulation()->GetTimeIncrement() > 0) setVelocity((length - Length()) / simulation()->GetTimeIncrement());
    else setVelocity(0);
    setLength(length);

    // normalise the direction vector
    line[0] /= Length();
    line[1] /= Length();
    line[2] /= Length();

    theOrigin->vector[0] = line[0];
    theOrigin->vector[1] = line[1];
    theOrigin->vector[2] = line[2];

    // simply reverse the direction for the insertion
    theInsertion->vector[0] = -line[0];
    theInsertion->vector[1] = -line[1];
    theInsertion->vector[2] = -line[2];

    // check that we don't have any non-normal values for directions which can occur if points co-locate
    for (size_t i = 0; i < GetPointForceList()->size(); i++)
    {
        if ((std::isfinite((*GetPointForceList())[i]->vector[0]) && std::isfinite((*GetPointForceList())[i]->vector[1]) && std::isfinite((*GetPointForceList())[i]->vector[2])) == false)
        {
            (*GetPointForceList())[i]->vector[0] = 0.0;
            (*GetPointForceList())[i]->vector[1] = 0.0;
            (*GetPointForceList())[i]->vector[2] = 0.0;
            std::cerr << "Warning: point force direction in \"" << name() << "\" is invalid so applying standard fixup\n";
        }
    }
}

//int TwoPointStrap::SanityCheck(Strap *otherStrap, Simulation::AxisType axis, const std::string &sanityCheckLeft, const std::string &sanityCheckRight)
//{
//    const double epsilon = DBL_EPSILON;

//    TwoPointStrap *other = dynamic_cast<TwoPointStrap *>(otherStrap);
//    if (other == nullptr) return __LINE__;

//    // first check attachment errors
//    switch (axis)
//    {
//    case Simulation::XAxis:
//        if (fabs(this->m_origin[0] + other->m_origin[0]) > epsilon) return __LINE__;
//        if (fabs(this->m_origin[1] - other->m_origin[1]) > epsilon) return __LINE__;
//        if (fabs(this->m_origin[2] - other->m_origin[2]) > epsilon) return __LINE__;
//        if (fabs(this->m_insertion[0] + other->m_insertion[0]) > epsilon) return __LINE__;
//        if (fabs(this->m_insertion[1] - other->m_insertion[1]) > epsilon) return __LINE__;
//        if (fabs(this->m_insertion[2] - other->m_insertion[2]) > epsilon) return __LINE__;
//        break;

//    case Simulation::YAxis:
//        if (fabs(this->m_origin[0] - other->m_origin[0]) > epsilon) return __LINE__;
//        if (fabs(this->m_origin[1] + other->m_origin[1]) > epsilon) return __LINE__;
//        if (fabs(this->m_origin[2] - other->m_origin[2]) > epsilon) return __LINE__;
//        if (fabs(this->m_insertion[0] - other->m_insertion[0]) > epsilon) return __LINE__;
//        if (fabs(this->m_insertion[1] + other->m_insertion[1]) > epsilon) return __LINE__;
//        if (fabs(this->m_insertion[2] - other->m_insertion[2]) > epsilon) return __LINE__;
//        break;

//    case Simulation::ZAxis:
//        if (fabs(this->m_origin[0] - other->m_origin[0]) > epsilon) return __LINE__;
//        if (fabs(this->m_origin[1] - other->m_origin[1]) > epsilon) return __LINE__;
//        if (fabs(this->m_origin[2] + other->m_origin[2]) > epsilon) return __LINE__;
//        if (fabs(this->m_insertion[0] - other->m_insertion[0]) > epsilon) return __LINE__;
//        if (fabs(this->m_insertion[1] - other->m_insertion[1]) > epsilon) return __LINE__;
//        if (fabs(this->m_insertion[2] + other->m_insertion[2]) > epsilon) return __LINE__;
//        break;
//    }

//    // now check for left to right crossover errors
//    if (this->name().find(sanityCheckLeft) != std::string::npos)
//    {
//        if (m_originBody->name().find(sanityCheckRight) != std::string::npos) return __LINE__;
//        if (m_insertionBody->name().find(sanityCheckRight) != std::string::npos) return __LINE__;
//    }
//    if (this->name().find(sanityCheckRight) != std::string::npos)
//    {
//        if (m_originBody->name().find(sanityCheckLeft) != std::string::npos) return __LINE__;
//        if (m_insertionBody->name().find(sanityCheckLeft) != std::string::npos) return __LINE__;
//    }

//    return 0;
//}

std::string *TwoPointStrap::createFromAttributes()
{
    if (Strap::createFromAttributes()) return lastErrorPtr();

    std::string buf;

    if (findAttribute("OriginMarkerID"s, &buf) == nullptr) return lastErrorPtr();
    auto originMarker = simulation()->GetMarkerList()->find(buf);
    if (originMarker == simulation()->GetMarkerList()->end())
    {
        setLastError("STRAP ID=\""s + name() +"\" OriginMarker not found"s);
        return lastErrorPtr();
    }
    this->SetOrigin(originMarker->second.get());
    if (findAttribute("InsertionMarkerID"s, &buf) == nullptr) return lastErrorPtr();
    auto insertionMarker = simulation()->GetMarkerList()->find(buf);
    if (insertionMarker == simulation()->GetMarkerList()->end())
    {
        setLastError("STRAP ID=\""s + name() +"\" InsertionMarker not found"s);
        return lastErrorPtr();
    }
    this->SetInsertion(insertionMarker->second.get());

    setUpstreamObjects({m_originMarker, m_insertionMarker});
    return nullptr;
}

void TwoPointStrap::appendToAttributes()
{
    Strap::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "TwoPoint"s);
    setAttribute("OriginMarkerID"s, m_originMarker->name());
    setAttribute("InsertionMarkerID"s, m_insertionMarker->name());
}

Marker *TwoPointStrap::GetOriginMarker() const
{
    return m_originMarker;
}

Marker *TwoPointStrap::GetInsertionMarker() const
{
    return m_insertionMarker;
}



