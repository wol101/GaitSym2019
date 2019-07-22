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

class Driver: public NamedObject
{
public:
    Driver();
    virtual ~Driver() ;

    void AddTarget(Drivable *target);
    Drivable *GetTarget(const std::string & name);
    double Clamp(double value);

    virtual double GetValue(double time) = 0;

    virtual void Dump();
    virtual std::string *CreateFromAttributes();
    virtual void SaveToAttributes();
    virtual void AppendToAttributes();


    double MinValue() const;
    void setMinValue(double MinValue);

    double MaxValue() const;
    void setMaxValue(double MaxValue);

    double LastTime() const;
    void setLastTime(double LastTime);

    double LastValue() const;
    void setLastValue(double LastValue);

    bool Interp() const;
    void setInterp(bool Interp);

    const std::map<std::string, Drivable *> *targetList() const;

private:

    std::map<std::string, Drivable *> m_targetList;
    double m_minValue = 0;
    double m_maxValue = 1;
    bool m_interp = false;
    double m_lastTime = -1;
    double m_lastValue = 0;

};

#endif
