/*
 *  Contact.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 09/02/2002.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

#ifndef Contact_h
#define Contact_h

#include "NamedObject.h"

#include "ode/ode.h"

class SimulationWindow;

class Contact:public NamedObject
{
public:
    Contact();

    void SetJointID(dJointID jointID) { m_JointID = jointID; }

    dJointID GetJointID() { return m_JointID; }
    dJointFeedback* GetJointFeedback() { return &m_ContactJointFeedback; }
    double* GetContactPosition() { return m_ContactPosition; }

private:

    dJointID m_JointID = nullptr;
    dJointFeedback m_ContactJointFeedback = {};
    dVector3 m_ContactPosition;

};


#endif

