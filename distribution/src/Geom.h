/*
 *  Geom.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 28/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

// Wrapper class to hold ODE geom

#ifndef Geom_h
#define Geom_h

#include "NamedObject.h"
#include "SmartEnum.h"

#include "ode/ode.h"

#include <vector>

class Contact;
class SimulationWindow;
class Marker;

class Geom: public NamedObject
{
public:

    Geom();
    virtual ~Geom();

    SMART_ENUM(GeomLocation, GeomLocationStrings, GeomLocationCount, environment, body);
//    enum GeomLocation { environment, body };

    void SetBody(dBodyID body);
    dBodyID GetBody();
    dGeomID GetGeomID() { return m_GeomID; }

    // these functions set the geom position relative to its body
    void SetPosition (double x, double y, double z);
    void SetQuaternion(double q0, double q1, double q2, double q3);

    // return body local values
    const double *GetPosition();
    void GetQuaternion(dQuaternion q);
    // return world values
    void GetWorldPosition(dVector3 p);
    void GetWorldQuaternion(dQuaternion q);


    void SetGeomLocation(GeomLocation l) { m_GeomLocation = l; }
    GeomLocation GetGeomLocation() { return m_GeomLocation; }

    double GetContactSoftCFM() { return m_CFM; }
    double GetContactSoftERP() { return m_ERP; }
    void SetContactMu(double mu) { m_Mu = mu; }
    double GetContactMu() { return m_Mu; }
    void SetContactBounce(double bounce) { m_Bounce = bounce; }
    double GetContactBounce() { return m_Bounce; }
    void SetRho(double rho) { m_Rho = rho; }
    double GetRho() { return m_Rho; }

    void SetSpringDamp(double springConstant, double dampingConstant, double integrationStep);
    void SetSpringERP(double springConstant, double ERP, double integrationStep);
    void SetSpringCFM(double springConstant, double CFM, double integrationStep);
    void SetCFMERP(double CFM, double ERP, double integrationStep);
    void SetCFMDamp(double CFM, double dampingConstant, double integrationStep);
    void SetERPDamp(double ERP, double dampingConstant, double integrationStep);

    void SetAbort(bool abort) { m_Abort = abort; }
    bool GetAbort() { return m_Abort; }

    void SetAdhesion(bool adhesion) { m_Adhesion = adhesion; }
    bool GetAdhesion() { return m_Adhesion; }

    void AddContact(Contact *contact) { m_ContactList.push_back(contact); }
    std::vector<Contact *> *GetContactList() { return &m_ContactList; }
    void ClearContacts() { m_ContactList.clear(); }

    std::vector<Geom *> *GetExcludeList() { return &m_ExcludeList; }

    virtual std::string dumpToString();
    virtual std::string *createFromAttributes();
    virtual void saveToAttributes();
    virtual void appendToAttributes();

    void setGeomMarker(Marker *geomMarker); // virtual because this is specialised for some geoms e.g. plane
    Marker *geomMarker() const;

    void setGeomID(const dGeomID &GeomID);
    dGeomID GeomID() const;

private:

    dGeomID m_GeomID = {nullptr};

    GeomLocation m_GeomLocation = {GeomLocation::environment};

    double m_CFM = -1;
    double m_ERP = -1;
    double m_Mu = dInfinity;
    double m_Bounce = -1;
    double m_Rho = -1;

    bool m_Abort = false;
    bool m_Adhesion = false;

    std::vector<Contact *> m_ContactList;

    Marker *m_geomMarker = nullptr;

    std::vector<Geom *> m_ExcludeList;

    // used for XMLSave
    double m_SpringConstant = 0;
    double m_DampingConstant = 0;
};


#endif
