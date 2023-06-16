/*
 *  NPointStrap.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 27/10/2007.
 *  Copyright 2007 Bill Sellers. All rights reserved.
 *
 */

#include "NPointStrap.h"
#include "Body.h"
#include "PGDMath.h"
#include "Simulation.h"
#include "GSUtil.h"
#include "Marker.h"

#include "ode/ode.h"

#include "pystring.h"

#include <cmath>
#include <string.h>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std::string_literals;

NPointStrap::NPointStrap(): Strap()
{
}

//void NPointStrap::SetOrigin(Body *body, const dVector3 point)
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

//void NPointStrap::SetInsertion(Body *body, const dVector3 point)
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

//void NPointStrap::GetOrigin(const Body **body, dVector3 origin) const
//{
//    *body = m_originBody;
//    origin[0] = m_origin[0];
//    origin[1] = m_origin[1];
//    origin[2] = m_origin[2];
//}

//void NPointStrap::GetInsertion(const Body **body, dVector3 insertion) const
//{
//    *body = m_insertionBody;
//    insertion[0] = m_insertion[0];
//    insertion[1] = m_insertion[1];
//    insertion[2] = m_insertion[2];
//}

void NPointStrap::SetOrigin(Marker *originMarker)
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

void NPointStrap::SetInsertion(Marker *insertionMarker)
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

//void NPointStrap::SetViaPoints(std::vector<Body *> *bodyList, std::vector<pgd::Vector3> *pointList)
//{
//    if (pointList->size() != bodyList->size())
//    {
//        std::cerr << __FILE__ << " " << __LINE__ << " Error in SetViaPoints\n";
//        return;
//    }
//    GetPointForceList()->reserve(pointList->size());
//    m_ViaBodyList.reserve(pointList->size());
//    m_ViaPointList.reserve(pointList->size());
//    for (unsigned int i = 0; i < pointList->size(); i++)
//    {
//        std::unique_ptr<PointForce> viaPointForce = std::make_unique<PointForce>();
//        viaPointForce->body = (*bodyList)[i];
//        m_ViaBodyList.push_back(viaPointForce->body);
//        m_ViaPointList.push_back((*pointList)[i]);
//        GetPointForceList()->push_back(std::move(viaPointForce));
//    }
//}

void NPointStrap::SetViaPoints(std::vector<Marker *> *viaPointMarkerList)
{
    m_ViaBodyList.clear();
    m_ViaPointList.clear();
    m_ViaPointMarkerList.clear();
    GetPointForceList()->reserve(viaPointMarkerList->size() + 2);
    m_ViaBodyList.reserve(viaPointMarkerList->size());
    m_ViaPointList.reserve(viaPointMarkerList->size());
    m_ViaPointMarkerList.reserve(viaPointMarkerList->size());
    for (size_t i = 0; i < viaPointMarkerList->size(); i++)
    {
        std::unique_ptr<PointForce> viaPointForce = std::make_unique<PointForce>();
        viaPointForce->body = viaPointMarkerList->at(i)->GetBody();
        m_ViaBodyList.push_back(viaPointForce->body);
        m_ViaPointList.push_back(viaPointMarkerList->at(i)->GetPosition());
        m_ViaPointMarkerList.push_back(viaPointMarkerList->at(i));
        GetPointForceList()->push_back(std::move(viaPointForce));
    }
}

const std::vector<pgd::Vector3> *NPointStrap::GetViaPoints() const
{
    return &m_ViaPointList;
}

const std::vector<Body *> *NPointStrap::GetViaPointBodies() const
{
    return &m_ViaBodyList;
}

const std::vector<Marker *> *NPointStrap::GetViaPointMarkers() const
{
    return &m_ViaPointMarkerList;
}

Marker *NPointStrap::GetOriginMarker() const
{
    return m_originMarker;
}

Marker *NPointStrap::GetInsertionMarker() const
{
    return m_insertionMarker;
}

void NPointStrap::Calculate()
{
    PointForce *theOrigin = (*GetPointForceList())[0].get();
    PointForce *theInsertion = (*GetPointForceList())[1].get();
    unsigned int i;
    pgd::Vector3 v;

    // calculate the world positions
    pgd::Vector3 origin = m_originMarker->GetWorldPosition();
    theOrigin->point[0] = origin.x;
    theOrigin->point[1] = origin.y;
    theOrigin->point[2] = origin.z;
    pgd::Vector3 insertion = m_insertionMarker->GetWorldPosition();
    theInsertion->point[0] = insertion.x;
    theInsertion->point[1] = insertion.y;
    theInsertion->point[2] = insertion.z;
    for (i = 0; i < m_ViaPointMarkerList.size(); i++)
    {
        v = m_ViaPointMarkerList[i]->GetWorldPosition();
        (*GetPointForceList())[i + 2]->point[0] = v.x;
        (*GetPointForceList())[i + 2]->point[1] = v.y;
        (*GetPointForceList())[i + 2]->point[2] = v.z;
    }

    std::unique_ptr<unsigned int[]> mapping = std::make_unique<unsigned int[]>(GetPointForceList()->size());
    for (i = 0; i < GetPointForceList()->size(); i++)
    {
        if (i == 0) mapping[i] = 0;
        else
            if (i == GetPointForceList()->size() - 1) mapping[i] = 1;
            else mapping[i] = i + 1;
    }

    pgd::Vector3 line, line2;
    double totalLength = 0;
    double len;
    for (i = 0; i < GetPointForceList()->size(); i++)
    {
        if (i == 0)
        {
            line.x = (*GetPointForceList())[mapping[i + 1]]->point[0] - (*GetPointForceList())[mapping[i]]->point[0];
            line.y = (*GetPointForceList())[mapping[i + 1]]->point[1] - (*GetPointForceList())[mapping[i]]->point[1];
            line.z = (*GetPointForceList())[mapping[i + 1]]->point[2] - (*GetPointForceList())[mapping[i]]->point[2];
            len = line.Magnitude();
            totalLength += len;
            line /= len;
        }
        else if (i == GetPointForceList()->size() - 1)
        {
            line.x = (*GetPointForceList())[mapping[i - 1]]->point[0] - (*GetPointForceList())[mapping[i]]->point[0];
            line.y = (*GetPointForceList())[mapping[i - 1]]->point[1] - (*GetPointForceList())[mapping[i]]->point[1];
            line.z = (*GetPointForceList())[mapping[i - 1]]->point[2] - (*GetPointForceList())[mapping[i]]->point[2];
            line.Normalize();
        }
        else
        {
            line.x = (*GetPointForceList())[mapping[i + 1]]->point[0] - (*GetPointForceList())[mapping[i]]->point[0];
            line.y = (*GetPointForceList())[mapping[i + 1]]->point[1] - (*GetPointForceList())[mapping[i]]->point[1];
            line.z = (*GetPointForceList())[mapping[i + 1]]->point[2] - (*GetPointForceList())[mapping[i]]->point[2];
            len = line.Magnitude();
            totalLength += len;
            line /= len;
            line2.x = (*GetPointForceList())[mapping[i - 1]]->point[0] - (*GetPointForceList())[mapping[i]]->point[0];
            line2.y = (*GetPointForceList())[mapping[i - 1]]->point[1] - (*GetPointForceList())[mapping[i]]->point[1];
            line2.z = (*GetPointForceList())[mapping[i - 1]]->point[2] - (*GetPointForceList())[mapping[i]]->point[2];
            line2.Normalize();
            line += line2;
        }

        (*GetPointForceList())[mapping[i]]->vector[0] = line.x;
        (*GetPointForceList())[mapping[i]]->vector[1] = line.y;
        (*GetPointForceList())[mapping[i]]->vector[2] = line.z;
    }

    if (Length() >= 0 && simulation() && simulation()->GetTimeIncrement() > 0) setVelocity((totalLength - Length()) / simulation()->GetTimeIncrement());
    else setVelocity(0);
    setLength(totalLength);

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

//int NPointStrap::SanityCheck(Strap *otherStrap, Simulation::AxisType axis, const std::string &sanityCheckLeft, const std::string &sanityCheckRight)
//{
//    const double epsilon = 1e-10;
//    unsigned int i;

//    NPointStrap *other = dynamic_cast<NPointStrap *>(otherStrap);
//    if (other == nullptr) return __LINE__;

//    if (this->m_ViaPointList.size() != other->m_ViaPointList.size()) return __LINE__;

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
//        for (i = 0; i < m_ViaPointList.size(); i++)
//        {
//            if (fabs(this->m_ViaPointList[i].x + other->m_ViaPointList[i].x) > epsilon) return __LINE__;
//            if (fabs(this->m_ViaPointList[i].y - other->m_ViaPointList[i].y) > epsilon) return __LINE__;
//            if (fabs(this->m_ViaPointList[i].z - other->m_ViaPointList[i].z) > epsilon) return __LINE__;
//        }
//        break;

//    case Simulation::YAxis:
//        if (fabs(this->m_origin[0] - other->m_origin[0]) > epsilon) return __LINE__;
//        if (fabs(this->m_origin[1] + other->m_origin[1]) > epsilon) return __LINE__;
//        if (fabs(this->m_origin[2] - other->m_origin[2]) > epsilon) return __LINE__;
//        if (fabs(this->m_insertion[0] - other->m_insertion[0]) > epsilon) return __LINE__;
//        if (fabs(this->m_insertion[1] + other->m_insertion[1]) > epsilon) return __LINE__;
//        if (fabs(this->m_insertion[2] - other->m_insertion[2]) > epsilon) return __LINE__;
//        for (i = 0; i < m_ViaPointList.size(); i++)
//        {
//            if (fabs(this->m_ViaPointList[i].x - other->m_ViaPointList[i].x) > epsilon) return __LINE__;
//            if (fabs(this->m_ViaPointList[i].y + other->m_ViaPointList[i].y) > epsilon) return __LINE__;
//            if (fabs(this->m_ViaPointList[i].z - other->m_ViaPointList[i].z) > epsilon) return __LINE__;
//        }
//        break;

//    case Simulation::ZAxis:
//        if (fabs(this->m_origin[0] - other->m_origin[0]) > epsilon) return __LINE__;
//        if (fabs(this->m_origin[1] - other->m_origin[1]) > epsilon) return __LINE__;
//        if (fabs(this->m_origin[2] + other->m_origin[2]) > epsilon) return __LINE__;
//        if (fabs(this->m_insertion[0] - other->m_insertion[0]) > epsilon) return __LINE__;
//        if (fabs(this->m_insertion[1] - other->m_insertion[1]) > epsilon) return __LINE__;
//        if (fabs(this->m_insertion[2] + other->m_insertion[2]) > epsilon) return __LINE__;
//        for (i = 0; i < m_ViaPointList.size(); i++)
//        {
//            if (fabs(this->m_ViaPointList[i].x - other->m_ViaPointList[i].x) > epsilon) return __LINE__;
//            if (fabs(this->m_ViaPointList[i].y - other->m_ViaPointList[i].y) > epsilon) return __LINE__;
//            if (fabs(this->m_ViaPointList[i].z + other->m_ViaPointList[i].z) > epsilon) return __LINE__;
//        }
//        break;
//    }

//    // now check for left to right crossover errors
//    if (this->name().find(sanityCheckLeft) != std::string::npos)
//    {
//        if (m_originBody->name().find(sanityCheckRight) != std::string::npos) return __LINE__;
//        if (m_insertionBody->name().find(sanityCheckRight) != std::string::npos) return __LINE__;
//        for (i = 0; i < m_ViaPointList.size(); i++)
//            if (m_ViaBodyList[i]->name().find(sanityCheckRight) != std::string::npos) return __LINE__;
//    }
//    if (this->name().find(sanityCheckRight) != std::string::npos)
//    {
//        if (m_originBody->name().find(sanityCheckLeft) != std::string::npos) return __LINE__;
//        if (m_insertionBody->name().find(sanityCheckLeft) != std::string::npos) return __LINE__;
//        for (i = 0; i < m_ViaPointList.size(); i++)
//            if (m_ViaBodyList[i]->name().find(sanityCheckLeft) != std::string::npos) return __LINE__;
//    }

//    return 0;
//}

std::string *NPointStrap::createFromAttributes()
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

    if (findAttribute("ViaPointMarkerIDList"s, &buf) == nullptr) return lastErrorPtr();
    std::vector<std::string> result;
    pystring::split(buf, result);
    if (result.empty())
    {
        setLastError("STRAP ID=\""s + name() +"\" ViaPointMarkerID list empty"s);
        return lastErrorPtr();
    }
    std::vector<Marker *> viaPointMarkerList;
    viaPointMarkerList.reserve(result.size());
    for (size_t i = 0; i < result.size(); i++)
    {
        auto viaPointMarker = simulation()->GetMarkerList()->find(result[i]);
        if (viaPointMarker == simulation()->GetMarkerList()->end())
        {
            setLastError("STRAP ID=\""s + name() +"\" via point marker \""s + result[i] +"\" not found"s);
            return lastErrorPtr();
        }
        viaPointMarkerList.push_back(viaPointMarker->second.get());
    }
    this->SetViaPoints(&viaPointMarkerList);

    std::vector<NamedObject *> upstreamObjects;
    upstreamObjects.reserve(viaPointMarkerList.size() + 2);
    upstreamObjects.push_back(m_originMarker);
    upstreamObjects.push_back(m_insertionMarker);
    for (auto &&it : viaPointMarkerList) upstreamObjects.push_back(it);
    setUpstreamObjects(std::move(upstreamObjects));
    return nullptr;
}

void NPointStrap::appendToAttributes()
{
    Strap::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "NPoint"s);
    setAttribute("OriginMarkerID"s, m_originMarker->name());
    setAttribute("InsertionMarkerID"s, m_insertionMarker->name());
    std::vector<std::string> markerNames;
    markerNames.reserve(m_ViaPointMarkerList.size());
    for (size_t i = 0; i < m_ViaPointMarkerList.size(); i++) markerNames.push_back(m_ViaPointMarkerList[i]->name());
    std::string viaPointMarkerList = pystring::join(" "s, markerNames);
    setAttribute("ViaPointMarkerIDList"s, viaPointMarkerList);
}


