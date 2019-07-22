/*
 *  Geom.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 28/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

// Wrapper class to hold ODE geom



#include "Geom.h"
#include "PGDMath.h"
#include "DataFile.h"
#include "Simulation.h"
#include "Body.h"
#include "GSUtil.h"
#include "Contact.h"
#include "Marker.h"
#include "GSUtil.h"
#include "PlaneGeom.h"



#include <iostream>
#ifdef MALLOC_H_NEEDED
#include <malloc.h>
#endif
#ifdef ALLOCA_H_NEEDED
#include <alloca.h>
#endif

using namespace std::string_literals;


Geom::Geom()
{
}

Geom::~Geom()
{
    if (GetGeomID()) dGeomDestroy(GetGeomID());
}


// these functions set the geom position relative to its body
// these now use the geom offset functions
void Geom::SetBody(dBodyID setBody)
{
    dGeomSetBody(GetGeomID(), setBody);
}

dBodyID Geom::GetBody()
{
    return dGeomGetBody(GetGeomID());
}

void Geom::SetPosition (double x, double y, double z)
{
    dGeomSetOffsetPosition(GetGeomID(), x, y, z);
}

const double *Geom::GetPosition()
{
    return dGeomGetOffsetPosition(GetGeomID());
}

void Geom::GetWorldPosition(dVector3 p)
{
    const double *relPosition = dGeomGetOffsetPosition(GetGeomID());
    dBodyGetRelPointPos(dGeomGetBody(GetGeomID()), relPosition[0], relPosition[1], relPosition[2], p);
    return;
}

void Geom::SetQuaternion(double q0, double q1, double q2, double q3)
{
    dQuaternion q;
    q[0] = q0; q[1] = q1; q[2] = q2; q[3] = q3;
    dGeomSetOffsetQuaternion(GetGeomID(), q);
}

void Geom::setGeomMarker(Marker *geomMarker)
{
    m_geomMarker = geomMarker;
    if (m_geomMarker->GetBody())
    {
        this->SetGeomLocation(Geom::body);
        this->SetBody(m_geomMarker->GetBody()->GetBodyID());
    }
    else
    {
        this->SetBody(nullptr);
        this->SetGeomLocation(Geom::environment);

    }
    if (dynamic_cast<PlaneGeom *>(this)) return; // do not try to place non-placeable geoms

    pgd::Vector p = geomMarker->GetPosition();
    this->SetPosition(p.x, p.y, p.z);
    pgd::Quaternion q = geomMarker->GetQuaternion();
    this->SetQuaternion(q.n, q.v.x, q.v.y, q.v.z);
    m_geomMarker->addDependent(this);
}

void Geom::GetQuaternion(dQuaternion q)
{
    dGeomGetOffsetQuaternion(GetGeomID(), q);
}

void Geom::GetWorldQuaternion(dQuaternion q)
{
    const double *bodyRotation = dBodyGetQuaternion(dGeomGetBody(GetGeomID()));
    dQuaternion relRotation;
    dGeomGetOffsetQuaternion(GetGeomID(), relRotation);
    //combine the body rotation with the cylinder rotation to get combined rotation from world coordinates
    dQMultiply0 (q, bodyRotation, relRotation);
}

void Geom::SetSpringDamp(double springConstant, double dampingConstant, double integrationStep)
{
    m_ERP = integrationStep * springConstant/(integrationStep * springConstant + dampingConstant);
    m_CFM = 1/(integrationStep * springConstant + dampingConstant);
    m_SpringConstant = springConstant;
    m_DampingConstant = springConstant;
}

void Geom::SetSpringERP(double springConstant, double ERP, double integrationStep)
{
    m_ERP = ERP;
    m_CFM = ERP / (integrationStep * springConstant);
    m_SpringConstant = springConstant;
    m_DampingConstant = (1.0 - m_ERP) / m_CFM;
}

void Geom::SetSpringCFM(double springConstant, double CFM, double integrationStep)
{
    m_CFM = CFM;
    m_SpringConstant = springConstant;
    m_DampingConstant = 1.0 / m_CFM - integrationStep * m_SpringConstant;
    m_ERP = integrationStep * m_SpringConstant/(integrationStep * m_SpringConstant + m_DampingConstant);
}

void Geom::SetCFMERP(double CFM, double ERP, double integrationStep)
{
    m_ERP = ERP;
    m_CFM = CFM;
    m_SpringConstant = m_ERP / (m_CFM * integrationStep);
    m_DampingConstant = (1.0 - m_ERP) / m_CFM;
}

void Geom::SetCFMDamp(double CFM, double dampingConstant, double integrationStep)
{
    m_CFM = CFM;
    m_DampingConstant = dampingConstant;
    m_SpringConstant = (1.0 / m_CFM - m_DampingConstant) / integrationStep;
    m_ERP = integrationStep * m_SpringConstant/(integrationStep * m_SpringConstant + m_DampingConstant);
}

void Geom::SetERPDamp(double ERP, double dampingConstant, double integrationStep)
{
    m_ERP = ERP;
    m_DampingConstant = dampingConstant;
    m_SpringConstant = m_DampingConstant / (integrationStep / m_ERP - integrationStep);
    m_CFM = 1.0/(integrationStep * m_SpringConstant + m_DampingConstant);
}

void Geom::Dump()
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
            setDumpStream(new std::ofstream(filename));
            dumpStream()->precision(17);
        }
        if (dumpStream())
        {
            *dumpStream() << "Time\tXP\tYP\tZP\tQW\tQX\tQY\tQZ\tNContacts\tBody1\tBody2\tXC\tYC\tZC\tFX1\tFY1\tFZ1\tTX1\tTY1\tTZ1\tFX2\tFY2\tFZ2\tTX2\tTY2\tTZ2\n";
        }
    }


    if (dumpStream())
    {

        dQuaternion bodyRotation;
        dGeomGetQuaternion(GetGeomID(), bodyRotation);
        const double *relPosition = dGeomGetOffsetPosition(GetGeomID());
        dQuaternion relRotation;
        dGeomGetOffsetQuaternion(GetGeomID(), relRotation);

        dVector3 p;
        dQuaternion q;

        // get the position in world coordinates
        dBodyGetRelPointPos(dGeomGetBody(GetGeomID()), relPosition[0], relPosition[1], relPosition[2], p);

        //combine the body rotation with the cylinder rotation to get combined rotation from world coordinates
        dQMultiply0 (q, bodyRotation, relRotation);

        *dumpStream() << simulation()->GetTime() << "\t" << p[0] << "\t" << p[1] << "\t" << p[2] <<
                "\t" << q[0] << "\t" << q[1] << "\t" << q[2] << "\t" << q[3] << "\t" <<
                m_ContactList.size();

        dJointFeedback *jointFeedback;
        dBodyID bodyID;
        for (unsigned int i = 0; i < m_ContactList.size(); i++)
        {
            bodyID = dJointGetBody(m_ContactList[i]->GetJointID(), 0);
            if (bodyID == GetBody()) // put them the normal way round
            {
                if (bodyID == nullptr) *dumpStream() << "\tStatic_Environment\t";
                else *dumpStream() << "\t" << static_cast<Body *>(dBodyGetData(bodyID))->GetName() << "\t";
                bodyID = dJointGetBody(m_ContactList[i]->GetJointID(), 1);
                if (bodyID == nullptr) *dumpStream() << "Static_Environment\t";
                else *dumpStream() << static_cast<Body *>(dBodyGetData(bodyID))->GetName() << "\t";

                *dumpStream() << (m_ContactList[i]->GetContactPosition())[0] << "\t" <<
                        (m_ContactList[i]->GetContactPosition())[1] << "\t" <<
                        (m_ContactList[i]->GetContactPosition())[2] << "\t";

                jointFeedback = m_ContactList[i]->GetJointFeedback();
                *dumpStream() <<
                        jointFeedback->f1[0] << "\t" << jointFeedback->f1[1] << "\t" << jointFeedback->f1[2] << "\t" <<
                        jointFeedback->t1[0] << "\t" << jointFeedback->t1[1] << "\t" << jointFeedback->t1[2] << "\t" <<
                        jointFeedback->f2[0] << "\t" << jointFeedback->f2[1] << "\t" << jointFeedback->f2[2] << "\t" <<
                        jointFeedback->t2[0] << "\t" << jointFeedback->t2[1] << "\t" << jointFeedback->t2[2];
            }
            else // reverse the order since our body is second
            {
                bodyID = dJointGetBody(m_ContactList[i]->GetJointID(), 1);
                if (bodyID == nullptr) *dumpStream() << "Static_Environment\t";
                else *dumpStream() << static_cast<Body *>(dBodyGetData(bodyID))->GetName() << "\t";
                bodyID = dJointGetBody(m_ContactList[i]->GetJointID(), 0);
                if (bodyID == nullptr) *dumpStream() << "\tStatic_Environment\t";
                else *dumpStream() << "\t" << static_cast<Body *>(dBodyGetData(bodyID))->GetName() << "\t";

                *dumpStream() << (m_ContactList[i]->GetContactPosition())[0] << "\t" <<
                        (m_ContactList[i]->GetContactPosition())[1] << "\t" <<
                        (m_ContactList[i]->GetContactPosition())[2] << "\t";

                jointFeedback = m_ContactList[i]->GetJointFeedback();
                *dumpStream() <<
                        jointFeedback->f2[0] << "\t" << jointFeedback->f2[1] << "\t" << jointFeedback->f2[2] << "\t" <<
                        jointFeedback->t2[0] << "\t" << jointFeedback->t2[1] << "\t" << jointFeedback->t2[2] << "\t" <<
                        jointFeedback->f1[0] << "\t" << jointFeedback->f1[1] << "\t" << jointFeedback->f1[2] << "\t" <<
                        jointFeedback->t1[0] << "\t" << jointFeedback->t1[1] << "\t" << jointFeedback->t1[2];
        }
        }
        *dumpStream() << "\n";
    }
}

Marker *Geom::geomMarker() const
{
    return m_geomMarker;
}

void Geom::setGeomID(const dGeomID &GeomID)
{
    m_GeomID = GeomID;
}

dGeomID Geom::GeomID() const
{
    return m_GeomID;
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *Geom::CreateFromAttributes()
{
    if (NamedObject::CreateFromAttributes()) return lastErrorPtr();
    std::string buf, buf2;
    if (GetAttribute("MarkerID"s, &buf) == nullptr) return lastErrorPtr();
    auto it = simulation()->GetMarkerList()->find(buf);
    if (it == simulation()->GetMarkerList()->end())
    {
        setLastError("GEOM ID=\""s + GetName() +"\" Marker not found"s);
        return lastErrorPtr();
    }
    this->setGeomMarker(it->second);

    // can specify ERP & CFM; SpringConstant & DampingConstant; SpringConstant & ERP; SpringConstant & CFM; DampingConstant & ERP; DampingConstant & CFM
    double stepSize = simulation()->GetTimeIncrement();
    if (GetAttribute("ERP", &buf) && GetAttribute("CFM", &buf2))
    {
        m_ERP = GSUtil::Double(buf);
        m_CFM = GSUtil::Double(buf2);
        m_SpringConstant = m_ERP / (m_CFM * stepSize);
        m_DampingConstant = (1.0 - m_ERP) / m_CFM;
    }
    else if (GetAttribute("ERP", &buf) && GetAttribute("SpringConstant", &buf2))
    {
        m_ERP = GSUtil::Double(buf);
        m_SpringConstant = GSUtil::Double(buf2);
        m_DampingConstant = stepSize * (m_SpringConstant / m_ERP - m_SpringConstant);
        m_CFM = 1.0/(stepSize * m_SpringConstant + m_DampingConstant);
    }
    else if (GetAttribute("ERP", &buf) && GetAttribute("DampingConstant", &buf2))
    {
        m_ERP = GSUtil::Double(buf);
        m_DampingConstant = GSUtil::Double(buf2);
        m_SpringConstant = m_DampingConstant / (stepSize / m_ERP - stepSize);
        m_CFM = 1.0/(stepSize * m_SpringConstant + m_DampingConstant);
    }
    else if (GetAttribute("CFM", &buf) && GetAttribute("DampingConstant", &buf2))
    {
        m_CFM = GSUtil::Double(buf);
        m_DampingConstant = GSUtil::Double(buf2);
        m_SpringConstant = (1.0 / m_CFM - m_DampingConstant) / stepSize;
        m_ERP = stepSize * m_SpringConstant/(stepSize * m_SpringConstant + m_DampingConstant);
    }
    else if (GetAttribute("CFM", &buf) && GetAttribute("SpringConstant", &buf2))
    {
        m_CFM = GSUtil::Double(buf);
        m_SpringConstant = GSUtil::Double(buf2);
        m_DampingConstant = 1.0 / m_CFM - stepSize * m_SpringConstant;
        m_ERP = stepSize * m_SpringConstant/(stepSize * m_SpringConstant + m_DampingConstant);
    }
    else if (GetAttribute("DampingConstant", &buf) && GetAttribute("SpringConstant", &buf2))
    {
        m_DampingConstant = GSUtil::Double(buf);
        m_SpringConstant = GSUtil::Double(buf2);
        m_CFM = 1.0/(stepSize * m_SpringConstant + m_DampingConstant);
        m_ERP = stepSize * m_SpringConstant/(stepSize * m_SpringConstant + m_DampingConstant);
    }
    else
    {
        setLastError("GEOM ID=\""s + GetName() +"\" 2 of DampingConstant, SpringConstant, CFM, or ERP must be provided"s);
        return lastErrorPtr();
    }

    if (GetAttribute("Bounce"s, &buf) == nullptr) return lastErrorPtr();
    this->SetContactBounce(GSUtil::Double(buf));
    if (GetAttribute("Mu"s, &buf) == nullptr) return lastErrorPtr();
    this->SetContactMu(GSUtil::Double(buf));
    if (GetAttribute("Abort"s, &buf) == nullptr) return lastErrorPtr();
    this->SetAbort(GSUtil::Bool(buf));
    if (GetAttribute("Adhesion"s, &buf) == nullptr) return lastErrorPtr();
    this->SetAdhesion(GSUtil::Bool(buf));

    return nullptr;
}

// this function copies the data in the object to an xml_node node that it creates internally.
// doc is used to allocate the memory so deletion should be automatic
void Geom::SaveToAttributes()
{
    this->setTag("GEOM"s);
    this->AppendToAttributes();
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void Geom::AppendToAttributes()
{
    NamedObject::AppendToAttributes();
    std::string buf;
    setAttribute("MarkerID"s, m_geomMarker->GetName());
    setAttribute("SpringConstant"s, *GSUtil::ToString(m_SpringConstant, &buf));
    setAttribute("DampingConstant"s, *GSUtil::ToString(m_DampingConstant, &buf));
    setAttribute("Bounce"s, *GSUtil::ToString(m_Bounce, &buf));
    setAttribute("Mu"s, *GSUtil::ToString(m_Mu, &buf));
    setAttribute("Abort"s, *GSUtil::ToString(m_Abort, &buf));
    setAttribute("Adhesion"s, *GSUtil::ToString(m_Adhesion, &buf));
}

