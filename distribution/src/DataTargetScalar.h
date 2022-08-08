/*
 *  DataTargetScalar.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Tue July 14 2009.
 *  Copyright (c) 1009 Bill Sellers. All rights reserved.
 *
 */

#ifndef DATATARGETSCALAR_H
#define DATATARGETSCALAR_H

#include "DataTarget.h"
#include "SmartEnum.h"

#include <string>
#include <set>

class NamedObject;

class DataTargetScalar: public DataTarget
{
public:
    DataTargetScalar();
    ~DataTargetScalar();

    SMART_ENUM(DataType, dataTypeStrings, dataTypeCount, XP, YP, ZP, Q0, Q1, Q2, Q3, XV, YV, ZV,
               XRV, YRV, ZRV, Angle, MetabolicEnergy, MechanicalEnergy, DriverError, Time, DeltaTime,
               XF, YF, ZF, Force);


    void SetTarget(NamedObject *target);
    NamedObject *GetTarget();

    void SetDataType(DataType dataType) { m_DataType = dataType; }
    DataType GetDataType() { return m_DataType; }

    virtual std::string dumpToString() override;

    virtual std::string *createFromAttributes() override;
    virtual void appendToAttributes() override;

    virtual double calculateError(double time) override;
    virtual double calculateError(size_t index) override;

private:
    double calculateErrorScore(double value);

    NamedObject *m_Target = nullptr;
    DataType m_DataType = XP;
    std::set<DataType> m_noTargetList = {MetabolicEnergy, MechanicalEnergy, Time, DeltaTime};

    std::vector<double> m_ValueList;
    double m_errorScore = 0;
};

#endif // DATATARGETSCALAR_H
