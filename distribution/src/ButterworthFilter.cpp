/*
 *  ButterworthFilter.cpp
 *  GaitSym2016
 *
 *  Created by Bill Sellers on Sat Jun 4 2016.
 *  Copyright (c) 2016 Bill Sellers. All rights reserved.
 *
 *  Implements a 2nd Order Butterworth Low Pass IIR Filter
 *
 */

#include "ButterworthFilter.h"
#include <cmath>

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif
#ifndef M_SQRT2
#define M_SQRT2    1.41421356237309504880
#endif


ButterworthFilter::ButterworthFilter() : Filter()
{
    m_cutoffFrequency = 0;
    m_samplingFrequency = 0;
    m_b0 = 0;
    m_b1 = 0;
    m_b2 = 0;
    m_a1 = 0;
    m_a2 = 0;
    m_xnminus1 = 0;
    m_xnminus2 = 0;
    m_yn = 0;
    m_ynminus1 = 0;
    m_ynminus2 = 0;
}

ButterworthFilter::ButterworthFilter(double cutoffFrequency, double samplingFrequency) : Filter()
{
    setXn(0);
    m_xnminus1 = 0;
    m_xnminus2 = 0;
    m_yn = 0;
    m_ynminus1 = 0;
    m_ynminus2 = 0;
    CalculateCoefficients(cutoffFrequency, samplingFrequency);
}

double ButterworthFilter::yn() const
{
    return m_yn;
}

double ButterworthFilter::cutoffFrequency() const
{
    return m_cutoffFrequency;
}

double ButterworthFilter::samplingFrequency() const
{
    return m_samplingFrequency;
}

void ButterworthFilter::AddNewSample(double x)
{
    // shift all the current values
    m_xnminus2 = m_xnminus1;
    m_xnminus1 = xn();
    setXn(x);
    m_ynminus2 = m_ynminus1;
    m_ynminus1 = m_yn;

    // and calculate the new yn value
    m_yn  =  m_b0*xn() + m_b1*m_xnminus1 + m_b2*m_xnminus2 + m_a1*m_ynminus1 + m_a2*m_ynminus2;
}

double ButterworthFilter::Output()
{
    return m_yn;
}

void ButterworthFilter::CalculateCoefficients(double cutoffFrequency, double samplingFrequency)
{
    // calculate the 2nd Order Butterworth Low Pass Filter coefficients for the IIR filter
    // y(n)  =  b0*x(n) + b1*x(n-1) + b2*x(n-2) + a1*y(n-1) + a2*y(n-2)
    m_cutoffFrequency = cutoffFrequency;
    m_samplingFrequency = samplingFrequency;
    double ff = m_cutoffFrequency / m_samplingFrequency;
    const double ita = 1.0 / tan(M_PI * ff);
    const double q = M_SQRT2;
    m_b0 = 1.0 / (1.0 + q*ita + ita*ita);
    m_b1 = 2 * m_b0;
    m_b2 = m_b0;
    m_a1 = 2.0 * (ita * ita - 1.0) * m_b0;
    m_a2 = -(1.0 - q * ita + ita * ita) * m_b0;
}

double SharedButterworthFilter::m_cutoffFrequency;
double SharedButterworthFilter::m_samplingFrequency;
double SharedButterworthFilter::m_b0;
double SharedButterworthFilter::m_b1;
double SharedButterworthFilter::m_b2;
double SharedButterworthFilter::m_a1;
double SharedButterworthFilter::m_a2;

SharedButterworthFilter::SharedButterworthFilter() : Filter()
{
    m_cutoffFrequency = 0;
    m_samplingFrequency = 0;
    m_b0 = 0;
    m_b1 = 0;
    m_b2 = 0;
    m_a1 = 0;
    m_a2 = 0;
    m_xnminus1 = 0;
    m_xnminus2 = 0;
    m_yn = 0;
    m_ynminus1 = 0;
    m_ynminus2 = 0;
}

SharedButterworthFilter::SharedButterworthFilter(double cutoffFrequency, double samplingFrequency) : Filter()
{
    setXn(0);
    m_xnminus1 = 0;
    m_xnminus2 = 0;
    m_yn = 0;
    m_ynminus1 = 0;
    m_ynminus2 = 0;
    CalculateCoefficients(cutoffFrequency, samplingFrequency);
}

double SharedButterworthFilter::yn() const
{
    return m_yn;
}

double SharedButterworthFilter::cutoffFrequency() const
{
    return m_cutoffFrequency;
}

double SharedButterworthFilter::samplingFrequency() const
{
    return m_samplingFrequency;
}

void SharedButterworthFilter::AddNewSample(double x)
{
    // shift all the current values
    m_xnminus2 = m_xnminus1;
    m_xnminus1 = Filter::xn();
    setXn(x);
    m_ynminus2 = m_ynminus1;
    m_ynminus1 = m_yn;

    // and calculate the new yn value
    m_yn  =  m_b0*xn() + m_b1*m_xnminus1 + m_b2*m_xnminus2 + m_a1*m_ynminus1 + m_a2*m_ynminus2;
}

double SharedButterworthFilter::Output()
{
    return m_yn;
}

void SharedButterworthFilter::CalculateCoefficients(double cutoffFrequency, double samplingFrequency)
{
    // calculate the 2nd Order Butterworth Low Pass Filter coefficients for the IIR filter
    // y(n)  =  b0*x(n) + b1*x(n-1) + b2*x(n-2) + a1*y(n-1) + a2*y(n-2)
    m_cutoffFrequency = cutoffFrequency;
    m_samplingFrequency = samplingFrequency;
    double ff = m_cutoffFrequency / m_samplingFrequency;
    const double ita = 1.0 / tan(M_PI * ff);
    const double q = M_SQRT2;
    m_b0 = 1.0 / (1.0 + q*ita + ita*ita);
    m_b1 = 2 * m_b0;
    m_b2 = m_b0;
    m_a1 = 2.0 * (ita * ita - 1.0) * m_b0;
    m_a2 = -(1.0 - q * ita + ita * ita) * m_b0;
}


