/*
 *  TegotaeDriver.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 08/01/2017.
 *  Copyright 2017 Bill Sellers. All rights reserved.
 *
 */

#ifndef TEGOTAEDRIVER_H
#define TEGOTAEDRIVER_H

#include "Driver.h"
#include "PGDMath.h"
#include "Marker.h"

class Body;
class Geom;

class TegotaeDriver : public Driver
{
public:
    TegotaeDriver();

    void Initialise(double omega, double sigma, double A, double Aprime, double B, double phi,
                    Marker *tegotaeCentre, Marker *tegotaeRim, Marker *errorOutput, Marker *forceDirection,
                    const std::vector<Geom *> &contactGeomList);

    virtual void Update();
    virtual void SendData();

    void UpdateReactionForce();

    virtual std::string dumpToString();

    double omega() const;
    double sigma() const;
    double A() const;
    double Aprime() const;
    double B() const;
    double phi() const;
    double X() const;
    double Y() const;
    double N() const;

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

    pgd::Vector3 worldErrorVector() const;
    pgd::Vector3 localErrorVector() const;

private:

    // these are the parameters from the paper Owaki D, Kano T, Nagasawa K, Tero A, Ishiguro A. 2012 Simple robot suggests physical interlimb communication is essential for quadruped walking. J R Soc Interface 20120669
    double m_omega = 0;         // rad s-1     intrinsic angular velocity
    double m_sigma = 0;         // rad s-1 N-1 magnitude of sensory feedback
    double m_A = 0;             // m           positive y-direction amplitude of leg motion in a swing phase
    double m_Aprime = 0;        // m           negative y-direction amplitude of leg motion in a stance phase
    double m_B = 0;             // m           x-direction amplitude of leg motion
    double m_phi = 0;           // rad         initial (and current) phase on LF leg's oscillator

    // and these are the outputs
    double m_X = 0;             // m           x-direction target of leg motion (+ve forward)
    double m_Y = 0;             // m           y-direction target of leg motion (+ve further from body)
    double m_N = 0;             // N           ground reaction force
    double m_phi_dot = 0;       // rad/s       the instantaneous change in phi

    // and these are for my implementation
    Marker *m_tegotaeCentre = nullptr;                // this marker defines the centre position and local coordinate system for the controller
    Marker *m_tegotaeRim = nullptr;                   // this marks the position of the tegotae target and is moved by the controller automatically
    Marker *m_errorOutput = nullptr;                  // this marker defines the position but not direction used to output the error system
    Marker *m_forceDirection = nullptr;               // this marker X axis defines the direction for the contact force feedback
    std::vector<Geom *> m_contactGeomList;            // these are the geoms that are used for the contact force feedback

//    pgd::Vector3 m_localPosition;                      // tegotae target position in m_tegotaeCentre coordinates
//    pgd::Vector3 m_worldPosition;                      // tegotae target position in world coordinates
//    pgd::Vector3 m_targetPosition;                     // m_errorOutput position in world coordinates
    pgd::Vector3 m_worldErrorVector;                   // error vector in world coordinates
    pgd::Vector3 m_localErrorVector;                   // error vector in m_errorOutput coordinates

    std::map<std::string, Drivable *> m_targetList1;  // target list for the Y component of the error
    std::map<std::string, Drivable *> m_targetList2;  // target list for the Z component of the error

    // these optional drivers allow me to alter the inputs
    Driver *m_omegaDriver = nullptr;
    Driver *m_sigmaDriver = nullptr;
    Driver *m_ADriver = nullptr;
    Driver *m_AprimeDriver = nullptr;
    Driver *m_BDriver = nullptr;

    // this value is required when the driver needs to be mirrored
    // the standard driver moves the robot in the -X direction so
    // mirroring is required even when the axis marker is rotated
    // the effect is simply to reverse the sign from
    // m_phi_dot = m_omega - m_sigma * m_N * std::cos(m_phi);
    // to
    // m_phi_dot = m_omega + m_sigma * m_N * std::cos(m_phi);
    bool m_mirror = false;
    bool m_allow_negative_phi_dot = false;

};

#endif // TEGOTAEDRIVER_H
