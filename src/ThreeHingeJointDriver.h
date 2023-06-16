/*
 *  ThreeHingeJointDriver.h
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 22/05/2020.
 *  Copyright 2020 Bill Sellers. All rights reserved.
 *
 */

#ifndef THREEHINGEJOINTDRIVER_H
#define THREEHINGEJOINTDRIVER_H

#include "Driver.h"
#include "PGDMath.h"

#include <memory>
#include <map>

class Marker;
class HingeJoint;
class Strap;
class Controller;
class Body;
class Joint;

class ThreeHingeJointDriver : public Driver
{
public:
    ThreeHingeJointDriver();

    virtual void Update();
    virtual void SendData();

    void CalculateLength(double angleFraction);
    static double CalculateLengthDifference(double angleFraction, void *data);

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

    virtual std::string dumpToString();

    Marker *targetMarker() const;
    void setTargetMarker(Marker *targetMarker);

    Marker *distalBodyMarker() const;
    void setDistalBodyMarker(Marker *distalBodyMarker);

    Joint *proximalJoint() const;
    void setProximalJoint(Joint *proximalJoint);

    HingeJoint *intermediateJoint() const;
    void setIntermediateJoint(HingeJoint *intermediateJoint);

    HingeJoint *distalJoint() const;
    void setDistalJoint(HingeJoint *distalJoint);

    double desiredLength() const;
    void setDesiredLength(double desiredLength);

    double actualLength() const;

private:
    Marker *m_targetMarker = nullptr;
    Marker *m_distalBodyMarker = nullptr;
    Joint *m_proximalJoint = nullptr;
    HingeJoint *m_intermediateJoint = nullptr;
    HingeJoint *m_distalJoint = nullptr;
    pgd::Vector2 m_proximalJointRange;
    pgd::Vector2 m_intermediateJointRange;
    pgd::Vector2 m_distalJointRange;
    double m_intermediateJointAngleGamma = 1.0;
    double m_distalJointAngleGamma = 1.0;
    double m_tolerance  =1.0e-6;
    bool m_dumpExtensionCurve = true;

    double m_proximalJointAngle1 = 0;
    pgd::Vector3 m_proximalJointAxis1;
    double m_proximalJointAngle2 = 0;
    pgd::Vector3 m_proximalJointAxis2;
    pgd::Quaternion m_proximalJointRotation;
    double m_intermediateJointAngle = 0;
    pgd::Vector3 m_intermediateJointAxis;
    pgd::Quaternion m_intermediateJointRotation;
    double m_distalJointAngle = 0;
    pgd::Vector3 m_distalJointAxis;
    pgd::Quaternion m_distalJointRotation;
    pgd::Vector3 m_distalBodyMarkerPositionWRTProxJoint;
    double m_desiredLength = 0;
    double m_actualLength = 0;
    double m_angleFraction = 0;
    double m_proximalAngleFraction1 = 0;
    double m_proximalAngleFraction2 = 0;

    // during contruction the bodies are not rotated, so the body vectors are the contruction vectors
    pgd::Vector3 m_proximalBodyVector;
    pgd::Vector3 m_intermediateBodyVector;
    pgd::Vector3 m_distalBodyVector;

    Body *m_baseBody = nullptr;
    Body *m_proximalBody = nullptr;
    Body *m_intermediateBody = nullptr;
    Body *m_distalBody = nullptr;
    Marker *m_proximalJointMarker1 = nullptr;
    Marker *m_proximalJointMarker2 = nullptr;
    Marker *m_intermediateJointMarker1 = nullptr;
    Marker *m_intermediateJointMarker2 = nullptr;
    Marker *m_distalJointMarker1 = nullptr;
    Marker *m_distalJointMarker2 = nullptr;
    Marker *m_distalBodyMarkerLocal = nullptr;

    std::map<std::string, std::unique_ptr<Body>> m_localBodyList;
    std::map<std::string, std::unique_ptr<Marker>> m_localMarkerList;
    std::map<std::string, std::unique_ptr<Strap>> m_localStrapList;

    Marker *createLocalMarkerCopy(const Marker *marker);
    static int monotonicTest(double (*f)(double x, void *info), double a, double b, double eps, void *info);
    static pgd::Vector3 GetEulerAngles(const Joint &joint, const Marker &basisMarker, bool reverseBodyOrderInCalculations);

};

#endif // THREEHINGEJOINTDRIVER_H
