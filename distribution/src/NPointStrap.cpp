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

NPointStrap::NPointStrap(): TwoPointStrap()
{
}

NPointStrap::~NPointStrap()
{
    for (size_t i = 0; i < m_ViaPointList.size(); i++) delete [] m_ViaPointList[i];
}

void NPointStrap::SetViaPoints(std::vector<Body *> *bodyList, std::vector<double *> *pointList)
{
    if (pointList->size() != bodyList->size())
    {
        std::cerr << __FILE__ << " " << __LINE__ << " Error in SetViaPoints\n";
        return;
    }
    PointForceList()->reserve(pointList->size());
    m_ViaBodyList.reserve(pointList->size());
    m_ViaPointList.reserve(pointList->size());
    for (unsigned int i = 0; i < pointList->size(); i++)
    {
        PointForce *viaPointForce = new PointForce();
        viaPointForce->body = (*bodyList)[i];
        PointForceList()->push_back(viaPointForce);
        m_ViaBodyList.push_back(viaPointForce->body);
        double *point = new double[dV3E__MAX];
        std::copy_n((*pointList)[i], dV3E__MAX, point);
        m_ViaPointList.push_back(point);
    }
}

void NPointStrap::SetViaPoints(std::vector<Marker *> *viaPointMarkerList)
{
    m_ViaBodyList.clear();
    m_ViaPointList.clear();
    m_ViaPointMarkerList.clear();
    PointForceList()->reserve(viaPointMarkerList->size() + 2);
    m_ViaBodyList.reserve(viaPointMarkerList->size());
    m_ViaPointList.reserve(viaPointMarkerList->size());
    m_ViaPointMarkerList.reserve(viaPointMarkerList->size());
    pgd::Vector v;
    for (size_t i = 0; i < viaPointMarkerList->size(); i++)
    {
        PointForce *viaPointForce = new PointForce();
        viaPointForce->body = viaPointMarkerList->at(i)->GetBody();
        PointForceList()->push_back(viaPointForce);
        m_ViaBodyList.push_back(viaPointForce->body);
        double *point = new double[dV3E__MAX];
        v = viaPointMarkerList->at(i)->GetPosition();
        point[0] = v.x;
        point[1] = v.y;
        point[2] = v.z;
        m_ViaPointList.push_back(point);
        m_ViaPointMarkerList.push_back(viaPointMarkerList->at(i));
    }
}

void NPointStrap::Calculate(double simulationTime)
{
    PointForce *theOrigin = (*PointForceList())[0];
    PointForce *theInsertion = (*PointForceList())[1];
    unsigned int i;
    double *ptr;

    // calculate the world positions
    dBodyGetRelPointPos(OriginBody()->GetBodyID(), Origin()[0], Origin()[1], Origin()[2],
                        theOrigin->point);
    dBodyGetRelPointPos(InsertionBody()->GetBodyID(), Insertion()[0], Insertion()[1], Insertion()[2],
                        theInsertion->point);
    for (i = 0; i < m_ViaPointList.size(); i++)
    {
        ptr = m_ViaPointList[i];
        dBodyGetRelPointPos(m_ViaBodyList[i]->GetBodyID(), ptr[0], ptr[1], ptr[2],
                        (*PointForceList())[i + 2]->point);
    }

    unsigned int *mapping = new unsigned int[PointForceList()->size()];
    for (i = 0; i < PointForceList()->size(); i++)
    {
        if (i == 0) mapping[i] = 0;
        else
            if (i == PointForceList()->size() - 1) mapping[i] = 1;
            else mapping[i] = i + 1;
    }

    pgd::Vector line, line2;
    double totalLength = 0;
    double len;
    for (i = 0; i < PointForceList()->size(); i++)
    {
        if (i == 0)
        {
            line.x = (*PointForceList())[mapping[i + 1]]->point[0] - (*PointForceList())[mapping[i]]->point[0];
            line.y = (*PointForceList())[mapping[i + 1]]->point[1] - (*PointForceList())[mapping[i]]->point[1];
            line.z = (*PointForceList())[mapping[i + 1]]->point[2] - (*PointForceList())[mapping[i]]->point[2];
            len = line.Magnitude();
            totalLength += len;
            line /= len;
        }
        else if (i == PointForceList()->size() - 1)
        {
            line.x = (*PointForceList())[mapping[i - 1]]->point[0] - (*PointForceList())[mapping[i]]->point[0];
            line.y = (*PointForceList())[mapping[i - 1]]->point[1] - (*PointForceList())[mapping[i]]->point[1];
            line.z = (*PointForceList())[mapping[i - 1]]->point[2] - (*PointForceList())[mapping[i]]->point[2];
            line.Normalize();
        }
        else
        {
            line.x = (*PointForceList())[mapping[i + 1]]->point[0] - (*PointForceList())[mapping[i]]->point[0];
            line.y = (*PointForceList())[mapping[i + 1]]->point[1] - (*PointForceList())[mapping[i]]->point[1];
            line.z = (*PointForceList())[mapping[i + 1]]->point[2] - (*PointForceList())[mapping[i]]->point[2];
            len = line.Magnitude();
            totalLength += len;
            line /= len;
            line2.x = (*PointForceList())[mapping[i - 1]]->point[0] - (*PointForceList())[mapping[i]]->point[0];
            line2.y = (*PointForceList())[mapping[i - 1]]->point[1] - (*PointForceList())[mapping[i]]->point[1];
            line2.z = (*PointForceList())[mapping[i - 1]]->point[2] - (*PointForceList())[mapping[i]]->point[2];
            line2.Normalize();
            line += line2;
        }

        (*PointForceList())[mapping[i]]->vector[0] = line.x;
        (*PointForceList())[mapping[i]]->vector[1] = line.y;
        (*PointForceList())[mapping[i]]->vector[2] = line.z;
    }

    setLength(totalLength, simulationTime);

    delete [] mapping;
}

int NPointStrap::SanityCheck(Strap *otherStrap, Simulation::AxisType axis, const std::string &sanityCheckLeft, const std::string &sanityCheckRight)
{
    const double epsilon = 1e-10;
    unsigned int i;

    NPointStrap *other = dynamic_cast<NPointStrap *>(otherStrap);
    if (other == nullptr) return __LINE__;

    if (this->m_ViaPointList.size() != other->m_ViaPointList.size()) return __LINE__;

    // first check attachment errors
    switch (axis)
    {
    case Simulation::XAxis:
        if (fabs(this->Origin()[0] + other->Origin()[0]) > epsilon) return __LINE__;
        if (fabs(this->Origin()[1] - other->Origin()[1]) > epsilon) return __LINE__;
        if (fabs(this->Origin()[2] - other->Origin()[2]) > epsilon) return __LINE__;
        if (fabs(this->Insertion()[0] + other->Insertion()[0]) > epsilon) return __LINE__;
        if (fabs(this->Insertion()[1] - other->Insertion()[1]) > epsilon) return __LINE__;
        if (fabs(this->Insertion()[2] - other->Insertion()[2]) > epsilon) return __LINE__;
        for (i = 0; i < m_ViaPointList.size(); i++)
        {
            if (fabs(this->m_ViaPointList[i][0] + other->m_ViaPointList[i][0]) > epsilon) return __LINE__;
            if (fabs(this->m_ViaPointList[i][1] - other->m_ViaPointList[i][1]) > epsilon) return __LINE__;
            if (fabs(this->m_ViaPointList[i][2] - other->m_ViaPointList[i][2]) > epsilon) return __LINE__;
        }
        break;

    case Simulation::YAxis:
        if (fabs(this->Origin()[0] - other->Origin()[0]) > epsilon) return __LINE__;
        if (fabs(this->Origin()[1] + other->Origin()[1]) > epsilon) return __LINE__;
        if (fabs(this->Origin()[2] - other->Origin()[2]) > epsilon) return __LINE__;
        if (fabs(this->Insertion()[0] - other->Insertion()[0]) > epsilon) return __LINE__;
        if (fabs(this->Insertion()[1] + other->Insertion()[1]) > epsilon) return __LINE__;
        if (fabs(this->Insertion()[2] - other->Insertion()[2]) > epsilon) return __LINE__;
        for (i = 0; i < m_ViaPointList.size(); i++)
        {
            if (fabs(this->m_ViaPointList[i][0] - other->m_ViaPointList[i][0]) > epsilon) return __LINE__;
            if (fabs(this->m_ViaPointList[i][1] + other->m_ViaPointList[i][1]) > epsilon) return __LINE__;
            if (fabs(this->m_ViaPointList[i][2] - other->m_ViaPointList[i][2]) > epsilon) return __LINE__;
        }
        break;

    case Simulation::ZAxis:
        if (fabs(this->Origin()[0] - other->Origin()[0]) > epsilon) return __LINE__;
        if (fabs(this->Origin()[1] - other->Origin()[1]) > epsilon) return __LINE__;
        if (fabs(this->Origin()[2] + other->Origin()[2]) > epsilon) return __LINE__;
        if (fabs(this->Insertion()[0] - other->Insertion()[0]) > epsilon) return __LINE__;
        if (fabs(this->Insertion()[1] - other->Insertion()[1]) > epsilon) return __LINE__;
        if (fabs(this->Insertion()[2] + other->Insertion()[2]) > epsilon) return __LINE__;
        for (i = 0; i < m_ViaPointList.size(); i++)
        {
            if (fabs(this->m_ViaPointList[i][0] - other->m_ViaPointList[i][0]) > epsilon) return __LINE__;
            if (fabs(this->m_ViaPointList[i][1] - other->m_ViaPointList[i][1]) > epsilon) return __LINE__;
            if (fabs(this->m_ViaPointList[i][2] + other->m_ViaPointList[i][2]) > epsilon) return __LINE__;
        }
        break;
    }

    // now check for left to right crossover errors
    if (this->GetName().find(sanityCheckLeft) != std::string::npos)
    {
        if (OriginBody()->GetName().find(sanityCheckRight) != std::string::npos) return __LINE__;
        if (InsertionBody()->GetName().find(sanityCheckRight) != std::string::npos) return __LINE__;
        for (i = 0; i < m_ViaPointList.size(); i++)
            if (m_ViaBodyList[i]->GetName().find(sanityCheckRight) != std::string::npos) return __LINE__;
    }
    if (this->GetName().find(sanityCheckRight) != std::string::npos)
    {
        if (OriginBody()->GetName().find(sanityCheckLeft) != std::string::npos) return __LINE__;
        if (InsertionBody()->GetName().find(sanityCheckLeft) != std::string::npos) return __LINE__;
        for (i = 0; i < m_ViaPointList.size(); i++)
            if (m_ViaBodyList[i]->GetName().find(sanityCheckLeft) != std::string::npos) return __LINE__;
    }

    return 0;
}

std::set<Marker *> *NPointStrap::updateDependentMarkers()
{
    dependentMarkers()->clear();
    dependentMarkers()->insert(originMarker());
    dependentMarkers()->insert(insertionMarker());
    for (size_t i = 0; i < m_ViaPointMarkerList.size(); i++) dependentMarkers()->insert(m_ViaPointMarkerList[i]);
    for (auto it : *dependentMarkers()) it->addDependent(this);
    return dependentMarkers();
}

std::string *NPointStrap::CreateFromAttributes()
{
    if (Strap::CreateFromAttributes()) return lastErrorPtr();

    std::string buf;

    if (GetAttribute("OriginMarkerID"s, &buf) == nullptr) return lastErrorPtr();
    auto originMarker = simulation()->GetMarkerList()->find(buf);
    if (originMarker == simulation()->GetMarkerList()->end())
    {
        setLastError("STRAP ID=\""s + GetName() +"\" OriginMarker not found"s);
        return lastErrorPtr();
    }
    this->SetOrigin(originMarker->second);
    if (GetAttribute("InsertionMarkerID"s, &buf) == nullptr) return lastErrorPtr();
    auto insertionMarker = simulation()->GetMarkerList()->find(buf);
    if (insertionMarker == simulation()->GetMarkerList()->end())
    {
        setLastError("STRAP ID=\""s + GetName() +"\" InsertionMarker not found"s);
        return lastErrorPtr();
    }
    this->SetInsertion(insertionMarker->second);

    if (GetAttribute("ViaPointMarkerIDList"s, &buf) == nullptr) return lastErrorPtr();
    std::vector<std::string> result;
    pystring::split(buf, result);
    if (result.empty())
    {
        setLastError("STRAP ID=\""s + GetName() +"\" ViaPointMarkerID list empty"s);
        return lastErrorPtr();
    }
    std::vector<Marker *> viaPointMarkerList;
    viaPointMarkerList.reserve(result.size());
    for (size_t i = 0; i < result.size(); i++)
    {
        auto viaPointMarker = simulation()->GetMarkerList()->find(result[i]);
        if (viaPointMarker == simulation()->GetMarkerList()->end())
        {
            setLastError("STRAP ID=\""s + GetName() +"\" via point marker \""s + result[i] +"\" not found"s);
            return lastErrorPtr();
        }
        viaPointMarkerList.push_back(viaPointMarker->second);
    }
    this->SetViaPoints(&viaPointMarkerList);

    return nullptr;
}

void NPointStrap::AppendToAttributes()
{
    Strap::AppendToAttributes();
    std::string buf;
    setAttribute("Type"s, "NPoint"s);
    setAttribute("OriginMarkerID"s, originMarker()->GetName());
    setAttribute("InsertionMarkerID"s, insertionMarker()->GetName());
    std::vector<std::string> markerNames;
    markerNames.reserve(m_ViaPointMarkerList.size());
    for (size_t i = 0; i < m_ViaPointMarkerList.size(); i++) markerNames.push_back(m_ViaPointMarkerList[i]->GetName());
    std::string viaPointMarkerList = pystring::join(" "s, markerNames);
    setAttribute("ViaPointMarkerIDList"s, viaPointMarkerList);
}


