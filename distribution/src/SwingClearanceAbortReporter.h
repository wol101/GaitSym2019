/*
 *  SwingClearanceAbortReporter.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 26/04/2011.
 *  Copyright 2011 Bill Sellers. All rights reserved.
 *
 */

#ifndef SWINGCLEARANCEABORTREPORTER_H
#define SWINGCLEARANCEABORTREPORTER_H

#include "Marker.h"
#include "PGDMath.h"

class SwingClearanceAbortReporter : public Marker
{
public:
    SwingClearanceAbortReporter();

    void SetHeightThreshold(double heightThreshold) { m_heightThreshold = heightThreshold; }
    void SetVelocityThreshold(double velocityThreshold) { m_velocityThreshold = velocityThreshold; }
    void SetUpAxis(const char *upAxis);
    void SetUpAxis(Marker::Axis upAxis) { m_upAxis = upAxis; }
    void SetDirectionAxis(double x, double y, double z);
    void SetDirectionAxis(const char *buf);

    virtual bool ShouldAbort();
    virtual std::string dump();

private:

    double m_heightThreshold;
    double m_velocityThreshold;
    Marker::Axis m_upAxis;
    bool m_useDirectionAxis;
    pgd::Vector m_directionAxis;
    double m_velocity;
    double m_height;


};

#endif // SWINGCLEARANCEABORTREPORTER_H
