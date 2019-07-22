/*
 *  DataTarget.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Sat May 22 2004.
 *  Copyright (c) 2004 Bill Sellers. All rights reserved.
 *
 */

#ifndef DataTarget_h
#define DataTarget_h

#include "NamedObject.h"

class Body;
class SimulationWindow;

class DataTarget: public NamedObject
{
public:
    DataTarget();
    virtual ~DataTarget();

    enum MatchType { linear, square };

    void SetTarget(NamedObject *target) { m_Target = target; }
    NamedObject *GetTarget() { return m_Target; }

    void SetTargetTimes(int size, double *targetTimes);
    virtual void SetTargetValues(int size, double *values);
    int TargetMatch(double time, double tolerance);
    int GetLastMatchIndex() { return m_LastMatchIndex; }

    void SetIntercept(double intercept) { m_Intercept = intercept; }
    void SetSlope(double slope) { m_Slope = slope; }
    void SetMatchType(MatchType t) { m_MatchType = t; }
    void SetAbortThreshold(double a) { m_AbortThreshold = a; }

    double GetMatchValue(double time);
    double GetMatchValue(int index);
    virtual double GetError(int index) = 0;
    virtual double GetError(double time) = 0;

    double PositiveFunction(double v);

    int TargetTimeListLength() const;

    double *TargetTimeList() const;

    double *ValueList() const;

    int ValueListLength() const;

private:

    int ProtectedTargetMatch(double time, double tolerance);

    NamedObject *m_Target;

    double m_Intercept;
    double m_Slope;
    MatchType m_MatchType;
    double m_AbortThreshold;

    double *m_TargetTimeList;
    int m_TargetTimeListLength;
    double *m_ValueList;
    int m_ValueListLength;

    int m_LastMatchIndex;
};

#endif

