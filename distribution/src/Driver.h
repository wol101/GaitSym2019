/*
 *  Driver.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Sat May 22 2004.
 *  Copyright (c) 2004 Bill Sellers. All rights reserved.
 *
 * Virtual class that all drivers descend from
 */

#ifndef Driver_h
#define Driver_h

#include "NamedObject.h"

#include <map>
#include <string>

class Drivable;

class Driver : public NamedObject
{
public:
    Driver();
    virtual ~Driver() ;

    int AddTarget(Drivable *target);
    Drivable *GetTarget(const std::string & name);
    double Clamp(double value);

    virtual void Update() = 0;
    virtual void SendData();

    virtual std::string dumpToString();
    virtual std::string *createFromAttributes();
    virtual void saveToAttributes();
    virtual void appendToAttributes();

    double MinValue() const;
    void setMinValue(double MinValue);

    double MaxValue() const;
    void setMaxValue(double MaxValue);

    bool Interp() const;
    void setInterp(bool Interp);

    const std::map<std::string, Drivable *> *targetList() const;

    int64_t lastStepCount() const;
    void setLastStepCount(const int64_t &lastStepCount);

    double value() const;
    void setValue(double value);

private:

    std::map<std::string, Drivable *> m_targetList;
    double m_minValue = 0;
    double m_maxValue = 1;
    bool m_interp = false;
    int64_t m_lastStepCount = -1;
    double m_value = 0;
};

#endif
