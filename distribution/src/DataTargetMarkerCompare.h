/*
 *  DataTargetMarkerCompare.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Thu Dec 24 2020.
 *  Copyright (c) 2020 Bill Sellers. All rights reserved.
 *
 */

#ifndef DATATARGETMARKERCOMPARE_H
#define DATATARGETMARKERCOMPARE_H

#include "DataTarget.h"
#include "SmartEnum.h"

class Marker;

class DataTargetMarkerCompare : public DataTarget
{
public:
    DataTargetMarkerCompare();

    SMART_ENUM(Comparison, comparisonStrings, comparisonCount, XWP, YWP, ZWP, XAD, YAD, ZAD, Distance, Angle, LinearVelocity, AngularVelocity);

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

    virtual double calculateError(double time);
    virtual double calculateError(size_t index);

private:
    Marker *m_marker1 = nullptr;
    Marker *m_marker2 = nullptr;
    Comparison m_marker1Comparison = XWP;
    Comparison m_marker2Comparison = XWP;

    std::vector<double> m_ValueList;
    double m_errorScore = 0;
};

#endif // DATATARGETMARKERCOMPARE_H
