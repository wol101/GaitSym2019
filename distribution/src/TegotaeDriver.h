/*
 *  TegotaeDriver.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 08/01/2017.
 *  Copyright 2017 Bill Sellers. All rights reserved.
 *
 */

#ifndef QUADRUPEDALTEGOTAECONTROLLER_H
#define QUADRUPEDALTEGOTAECONTROLLER_H

#include "Driver.h"

#include "PGDMath.h"

#include "ode/ode.h"

class Body;
class Geom;

#ifdef USE_QT
class SimulationWindow;
#endif

class TegotaeDriver : public Driver
{
public:
    TegotaeDriver();

    void Initialise(double omega, double sigma, double A, double Aprime, double B, double phi,
                    Body *referenceBody, Geom *contactGeom, const pgd::Vector &contactOffset, bool outputVertical);

    void UpdateReactionForce();

    virtual double GetValue(double time);

    virtual void Dump();

    double omega() const;
    double sigma() const;
    double A() const;
    double Aprime() const;
    double B() const;
    double phi() const;
    double X() const;
    double Y() const;
    double N() const;

private:

    // these are the parameters from the paper Owaki D, Kano T, Nagasawa K, Tero A, Ishiguro A. 2012 Simple robot suggests physical interlimb communication is essential for quadruped walking. J R Soc Interface 20120669
    double m_omega;         // rad s-1     intrinsic angular velocity
    double m_sigma;         // rad s-1 N-1 magnitude of sensory feedback
    double m_A;             // m           positive y-direction amplitude of leg motion in a swing phase
    double m_Aprime;        // m           negative y-direction amplitude of leg motion in a stance phase
    double m_B;             // m           x-direction amplitude of leg motion
    double m_phi;           // rad         initial (and current) phase on LF leg's oscillator

    // and these are the outputs
    double m_X;             // m           x-direction target of leg motion (+ve forward)
    double m_Y;             // m           y-direction target of leg motion (+ve further from body)
    double m_N;             // N           ground reaction force
    double m_phi_dot;       // rad/s       the instantaneous change in phi

    // and these are for my implementation
    Body *m_referenceBody;              // this is the body used to set the coordinate system
    Geom *m_contactGeom;                // this is the geom that is used for the contact position
    pgd::Vector m_contactOffset;        // this is the neutral offset of the geom from the reference
    dVector3 m_worldGeomPosition;       // this is the current position of the contact GEOM in world coordinates
    dVector3 m_worldTargetPosition;     // this is the current position of the tegotae target in world coordinates
    dVector3 m_referenceBodyGeomPosition;   // this is the current position of the contact GEOM in reference body coordinates
    dVector3 m_referenceBodyTargetPosition;   // this is the current position of the tegotae target in reference body coordinates
    double m_output;                    // the output value of the controller

    // and these are to make the driver work as expected
    bool m_outputVertical;          // this driver will output the horizontal error

#ifdef USE_QT
    double m_Radius;
#endif
};

#endif // QUADRUPEDALTEGOTAECONTROLLER_H
