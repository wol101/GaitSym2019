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

#ifdef MALLOC_H_NEEDED
#include <malloc.h>
#endif
#ifdef ALLOCA_H_NEEDED
#include <alloca.h>
#endif

using namespace std::string_literals;

TwoPointStrap::TwoPointStrap()
{
}

TwoPointStrap::~TwoPointStrap()
{
}

void TwoPointStrap::SetOrigin(Body *body, const dVector3 point)
{
    m_OriginBody = body;
    m_Origin[0] = point[0];
    m_Origin[1] = point[1];
    m_Origin[2] = point[2];
    if (PointForceList()->size() == 0)
    {
        PointForce *origin = new PointForce();
        origin->body = m_OriginBody;
        PointForceList()->push_back(origin);
    }
    else
    {
        PointForceList()->at(0)->body = m_OriginBody;
    }
}

// parses the position allowing a relative position specified by BODY ID
// x y z - world coordinates
// bodyName x y z - position relative to bodyName local coordinate system
void
TwoPointStrap::SetOrigin(Body *body, const char *buf)
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
        std::cerr << "Error in TwoPointStrap::SetOrigin\n";
        return; // error condition
    }


        if (isalpha((int)*lBufPtrs[0]) == 0)
        {
                for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i], 0);
        dBodyGetPosRelPoint(body->GetBodyID(), pos[0], pos[1], pos[2], result); // convert from world to body
        SetOrigin(body, result);
                return;
        }

        if (count < 4)
    {
        std::cerr << "Error in TwoPointStrap::SetOrigin\n";
        return; // error condition
    }
        Body *theBody = simulation()->GetBody(lBufPtrs[0]);
        if (theBody == 0)
    {
        if (strcmp(lBufPtrs[0], "World") == 0)
        {
            for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i + 1], 0);
            dBodyGetPosRelPoint(body->GetBodyID(), pos[0], pos[1], pos[2], result); // convert from world to body
            SetOrigin(body, result);
            return;
        }
        else
        {
            std::cerr << "Error in TwoPointStrap::SetOrigin\n";
            return; // error condition
        }
    }
    for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i + 1], 0);
    dBodyGetRelPointPos(theBody->GetBodyID(), pos[0], pos[1], pos[2], result); // convert from body to world
    dBodyGetPosRelPoint(body->GetBodyID(), result[0], result[1], result[2], pos); // convert from world to body
    SetOrigin(body, pos);
}

void TwoPointStrap::SetInsertion(Body *body, const dVector3 point)
{
    m_InsertionBody = body;
    m_Insertion[0] = point[0];
    m_Insertion[1] = point[1];
    m_Insertion[2] = point[2];
    if (PointForceList()->size() <= 1)
    {
        PointForce *insertion = new PointForce();
        insertion->body = m_InsertionBody;
        PointForceList()->push_back(insertion);
    }
    else
    {
        PointForceList()->at(1)->body = m_InsertionBody;
    }
}

// parses the position allowing a relative position specified by BODY ID
// x y z - world coordinates
// bodyName x y z - position relative to bodyName local coordinate system
void
TwoPointStrap::SetInsertion(Body *body, const char *buf)
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
        std::cerr << "Error in TwoPointStrap::SetInsertion\n";
        return; // error condition
    }

        if (isalpha((int)*lBufPtrs[0]) == 0)
        {
                for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i], 0);
        dBodyGetPosRelPoint(body->GetBodyID(), pos[0], pos[1], pos[2], result); // convert from world to body
        SetInsertion(body, result);
                return;
        }

        if (count < 4)
    {
        std::cerr << "Error in TwoPointStrap::SetInsertion\n";
        return; // error condition
    }
        Body *theBody = simulation()->GetBody(lBufPtrs[0]);
        if (theBody == 0)
    {
        if (strcmp(lBufPtrs[0], "World") == 0)
        {
            for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i + 1], 0);
            dBodyGetPosRelPoint(body->GetBodyID(), pos[0], pos[1], pos[2], result); // convert from world to body
            SetInsertion(body, result);
            return;
        }
        else
        {
            std::cerr << "Error in TwoPointStrap::SetInsertion\n";
            return; // error condition
        }
    }
    for (i = 0; i < 3; i++) pos[i] = strtod(lBufPtrs[i + 1], 0);
    dBodyGetRelPointPos(theBody->GetBodyID(), pos[0], pos[1], pos[2], result); // convert from body to world
    dBodyGetPosRelPoint(body->GetBodyID(), result[0], result[1], result[2], pos); // convert from world to body
    SetInsertion(body, pos);
}

void TwoPointStrap::SetOrigin(Marker *originMarker)
{
    m_originMarker = originMarker;
    this->SetOrigin(originMarker->GetBody(), originMarker->GetPosition().data());
}

void TwoPointStrap::SetInsertion(Marker *insertionMarker)
{
    m_insertionMarker = insertionMarker;
    this->SetInsertion(insertionMarker->GetBody(), insertionMarker->GetPosition().data());
}

void TwoPointStrap::Calculate(double simulationTime)
{
    PointForce *theOrigin = (*PointForceList())[0];
    PointForce *theInsertion = (*PointForceList())[1];

    // calculate the world positions
    dBodyGetRelPointPos(OriginBody()->GetBodyID(), Origin()[0], Origin()[1], Origin()[2],
                        theOrigin->point);
    dBodyGetRelPointPos(InsertionBody()->GetBodyID(), Insertion()[0], Insertion()[1], Insertion()[2],
                        theInsertion->point);

    // calculate the vector from the origin to the insertion
    dVector3 line;
    line[0] = theInsertion->point[0] - theOrigin->point[0];
    line[1] = theInsertion->point[1] - theOrigin->point[1];
    line[2] = theInsertion->point[2] - theOrigin->point[2];

    // calculate the length and velocity
    setLength(sqrt(line[0]*line[0] + line[1]*line[1] + line[2]*line[2]), simulationTime);

    // normalise the direction vector
    line[0] /= Length();
    line[1] /= Length();
    line[2] /= Length();

    std::copy_n(line, dV3E__MAX, theOrigin->vector);

    // simply reverse the direction for the insertion
    line[0] = -line[0];
    line[1] = -line[1];
    line[2] = -line[2];

    std::copy_n(line, dV3E__MAX, theInsertion->vector);
}

int TwoPointStrap::SanityCheck(Strap *otherStrap, Simulation::AxisType axis, const std::string &sanityCheckLeft, const std::string &sanityCheckRight)
{
    const double epsilon = 1e-10;

    TwoPointStrap *other = dynamic_cast<TwoPointStrap *>(otherStrap);
    if (other == 0) return __LINE__;

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
        break;

    case Simulation::YAxis:
        if (fabs(this->Origin()[0] - other->Origin()[0]) > epsilon) return __LINE__;
        if (fabs(this->Origin()[1] + other->Origin()[1]) > epsilon) return __LINE__;
        if (fabs(this->Origin()[2] - other->Origin()[2]) > epsilon) return __LINE__;
        if (fabs(this->Insertion()[0] - other->Insertion()[0]) > epsilon) return __LINE__;
        if (fabs(this->Insertion()[1] + other->Insertion()[1]) > epsilon) return __LINE__;
        if (fabs(this->Insertion()[2] - other->Insertion()[2]) > epsilon) return __LINE__;
        break;

    case Simulation::ZAxis:
        if (fabs(this->Origin()[0] - other->Origin()[0]) > epsilon) return __LINE__;
        if (fabs(this->Origin()[1] - other->Origin()[1]) > epsilon) return __LINE__;
        if (fabs(this->Origin()[2] + other->Origin()[2]) > epsilon) return __LINE__;
        if (fabs(this->Insertion()[0] - other->Insertion()[0]) > epsilon) return __LINE__;
        if (fabs(this->Insertion()[1] - other->Insertion()[1]) > epsilon) return __LINE__;
        if (fabs(this->Insertion()[2] + other->Insertion()[2]) > epsilon) return __LINE__;
        break;
    }

    // now check for left to right crossover errors
    if (this->GetName().find(sanityCheckLeft) != std::string::npos)
    {
        if (OriginBody()->GetName().find(sanityCheckRight) != std::string::npos) return __LINE__;
        if (InsertionBody()->GetName().find(sanityCheckRight) != std::string::npos) return __LINE__;
    }
    if (this->GetName().find(sanityCheckRight) != std::string::npos)
    {
        if (OriginBody()->GetName().find(sanityCheckLeft) != std::string::npos) return __LINE__;
        if (InsertionBody()->GetName().find(sanityCheckLeft) != std::string::npos) return __LINE__;
    }

    return 0;
}

std::set<Marker *> *TwoPointStrap::updateDependentMarkers()
{
    dependentMarkers()->clear();
    dependentMarkers()->insert(m_originMarker);
    dependentMarkers()->insert(m_insertionMarker);
    for (auto it : *dependentMarkers()) it->addDependent(this);
    return dependentMarkers();
}

std::string *TwoPointStrap::CreateFromAttributes()
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

    return nullptr;
}

void TwoPointStrap::AppendToAttributes()
{
    Strap::AppendToAttributes();
    std::string buf;
    setAttribute("Type"s, "TwoPoint"s);
    setAttribute("OriginMarkerID"s, m_originMarker->GetName());
    setAttribute("InsertionMarkerID"s, m_insertionMarker->GetName());
}

Body *TwoPointStrap::OriginBody() const
{
    return m_OriginBody;
}

double *TwoPointStrap::Origin()
{
    return m_Origin;
}

Body *TwoPointStrap::InsertionBody() const
{
    return m_InsertionBody;
}

double *TwoPointStrap::Insertion()
{
    return m_Insertion;
}

Marker *TwoPointStrap::originMarker() const
{
    return m_originMarker;
}

Marker *TwoPointStrap::insertionMarker() const
{
    return m_insertionMarker;
}


