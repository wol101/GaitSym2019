/*
 *  MarkerEllipseDriver.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 08/01/2017.
 *  Copyright 2017 Bill Sellers. All rights reserved.
 *
 */

#ifndef MARKERELLIPSEDRIVER_H
#define MARKERELLIPSEDRIVER_H

#include "DataTarget.h"
#include "Driver.h"
#include "Marker.h"
#include "ButterworthFilter.h"

class Body;
class Geom;

class MarkerEllipseDriver : public Driver
{
public:
    MarkerEllipseDriver();

    void Initialise(double omega, double sigma, double XR, double YR, double phi, Marker *markerEllipseCentre, Marker *markerEllipseRim, DataTarget *phaseControlInput);

    virtual void Update();
    virtual void SendData();

    virtual std::string dumpToString();

    double omega() const;
    double sigma() const;
    double phi() const;
    double phi_dot() const;
    double XR() const;
    double YR() const;
    double X() const;
    double Y() const;

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

private:
    int detectSignChange(double value); // 0 is no change, +1 is switching to decreasing (phase = pi/2), -1 is switching to increasing (phase = 3pi/2)

    double m_omega = 0;         // rad s-1     intrinsic angular velocity
    double m_sigma = 0;         //             gain for the phase correction
    double m_XR = 0;            // m           x-direction radius
    double m_YR = 0;            // m           y-direction radius
    double m_phi = 0;           // rad         initial (and current) phase on LF leg's oscillator
    double m_phiDot = 0;       // rad/s       the instantaneous change in phi
    double m_X = 0;             // current local X position
    double m_Y = 0;             // current local Y position
    Marker *m_markerEllipseCentre = nullptr;                // this marker defines the centre position and local coordinate system for the controller
    Marker *m_markerEllipseRim = nullptr;                   // this marks the position of the markerEllipse target and is moved by the controller automatically
    DataTarget *m_phaseControlInput = nullptr;              // this can be used as an imput driver that alters the phase of the circular motion (much like a phase-locked-loop PLL)

    // these optional drivers allow me to alter the inputs
    Driver *m_omegaDriver = nullptr;
    Driver *m_sigmaDriver = nullptr;
    Driver *m_XRDriver = nullptr;
    Driver *m_YRDriver = nullptr;

    // and this is the phase detection login
    bool m_phaseStateIncreasing = false; // false is decreasing, true is decreasing
    size_t m_phaseStateChangeCount = 0;
    size_t m_phaseStateCountThreshold = 0;
    double m_lastPhaseChangeTime = 0;
    double m_phaseOffset = 0;
    double m_maxPhiDot = 0;
    ButterworthFilter m_butterworthFilter;
    double m_halfPeriod = 0;
    double m_periodMultiplier = 2.0;
    double m_wantedPhi = 0;
    double m_delPhi = 0;
    int m_valueChangeDirection = 0;
};

#endif // MARKERELLIPSEDRIVER_H
