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
#include "SmartEnum.h"

#include <float.h>

class Body;
class SimulationWindow;

class DataTarget: public NamedObject
{
public:
    DataTarget();
    virtual ~DataTarget();

    SMART_ENUM(MatchType, matchTypeStrings, matchTypeCount, Linear, Square);
//    enum MatchType { linear, square };

    void SetTargetTimes(int size, double *targetTimes);
    virtual void SetTargetValues(int size, double *values) = 0;

    int TargetMatch(double time, double tolerance);
    int GetLastMatchIndex();

    void SetIntercept(double intercept);
    void SetSlope(double slope);
    void SetMatchType(MatchType t);
    void SetAbortThreshold(double a);

    double GetMatchValue(double time);
    double GetMatchValue(int index);

    virtual double GetError(int index) = 0;
    virtual double GetError(double time) = 0;

    double PositiveFunction(double v);

    int TargetTimeListLength() const;

    const double *TargetTimeList() const;

    virtual std::string dump();
    virtual std::string *createFromAttributes();
    virtual void saveToAttributes();
    virtual void appendToAttributes();

private:

    int ProtectedTargetMatch(double time, double tolerance);

    double m_Intercept = 0;
    double m_Slope = -1;
    MatchType m_MatchType = MatchType::Linear;
    double m_AbortThreshold = -DBL_MAX;

    std::vector<double> m_TargetTimeList;
    int m_TargetTimeListLength = -1;

    int m_LastMatchIndex = -1;


};

#endif

