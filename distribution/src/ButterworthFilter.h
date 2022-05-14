/*
 *  ButterworthFilter.h
 *  GaitSym2016
 *
 *  Created by Bill Sellers on Sat Jun 4 2016.
 *  Copyright (c) 2016 Bill Sellers. All rights reserved.
 *
 *  Implements a 2nd Order Butterworth Low Pass IIR Filter
 *
 */

#ifndef BUTTERWORTHFILTER_H
#define BUTTERWORTHFILTER_H

#include "Filter.h"

class ButterworthFilter : public Filter
{
public:
    ButterworthFilter();
    ButterworthFilter(double cutoffFrequency, double samplingFrequency);

    virtual void AddNewSample(double x);
    virtual double Output();

    void CalculateCoefficients(double cutoffFrequency, double samplingFrequency);

    double yn() const;

    double cutoffFrequency() const;
    double samplingFrequency() const;

private:

    double m_cutoffFrequency;
    double m_samplingFrequency;
    double m_b0;
    double m_b1;
    double m_b2;
    double m_a1;
    double m_a2;
    double m_xnminus1;
    double m_xnminus2;
    double m_yn;
    double m_ynminus1;
    double m_ynminus2;
};

#ifdef NON_THREAD_SAFE_OK
class SharedButterworthFilter : public Filter
{
public:
    SharedButterworthFilter();
    SharedButterworthFilter(double cutoffFrequency, double samplingFrequency);

    virtual void AddNewSample(double x);
    virtual double Output();

    static void CalculateCoefficients(double cutoffFrequency, double samplingFrequency);

    double yn() const;

    double cutoffFrequency() const;
    double samplingFrequency() const;

private:

    static double m_cutoffFrequency;
    static double m_samplingFrequency;
    static double m_b0;
    static double m_b1;
    static double m_b2;
    static double m_a1;
    static double m_a2;
    double m_xnminus1;
    double m_xnminus2;
    double m_yn;
    double m_ynminus1;
    double m_ynminus2;
};
#endif

#endif // BUTTERWORTHFILTER_H
