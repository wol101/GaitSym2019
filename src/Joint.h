/*
 *  Joint.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 19/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

// wrapper class for a joint

#ifndef Joint_h
#define Joint_h

#include "NamedObject.h"

#include "ode/ode.h"

class Body;
class SimulationWindow;
class Marker;

class Joint: public NamedObject
{
public:

    // Note constructor in derived functions need to set m_JointID
    // m_JointFeedback only needs to be set if GetFeedback is used (good idea though)

    Joint();
    virtual ~Joint();

    virtual void Attach(Body *body1, Body *body2);
    virtual void Attach();
    Body *GetBody1() { return m_Body1; }
    Body *GetBody2() { return m_Body2; }
    void setBody1(Body *Body1);
    void setBody2(Body *Body2);

    dJointFeedback *GetFeedback();

    // some joints (particularly those with motors) need to do something before the simulation step
    virtual void Update() {}

    // this might be needed at some point in the future
    virtual void LateInitialisation() {}

    Marker *body1Marker() const;
    void setBody1Marker(Marker *body1Marker);

    Marker *body2Marker() const;
    void setBody2Marker(Marker *body2Marker);

    virtual std::string *createFromAttributes();
    virtual void saveToAttributes();
    virtual void appendToAttributes();

    dJointID JointID() const;
    void setJointID(const dJointID &JointID);

    dJointFeedback *JointFeedback();
    void setJointFeedback(const dJointFeedback &JointFeedback);


    double CFM() const;
    void setCFM(double CFM);

    double ERP() const;
    void setERP(double ERP);

private:

    Body *m_Body1 = nullptr;
    Body *m_Body2 = nullptr;
    dJointID m_JointID = nullptr;
    dJointFeedback m_JointFeedback = {};
    Marker *m_body1Marker = nullptr;
    Marker *m_body2Marker = nullptr;
    double m_CFM = -1;
    double m_ERP = -1;
};

#endif
