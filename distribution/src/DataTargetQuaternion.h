/*
 *  DataTargetQuaternion.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Tue July 14 2009.
 *  Copyright (c) 1009 Bill Sellers. All rights reserved.
 *
 */

#ifndef DATATARGETQUATERNION_H
#define DATATARGETQUATERNION_H

#include "DataTarget.h"
#include "PGDMath.h"

#include <iostream>

class DataTargetQuaternion : public DataTarget
{
public:
    DataTargetQuaternion();
    virtual ~DataTargetQuaternion();

    void SetTarget(NamedObject *target);
    NamedObject *GetTarget();

    virtual void SetTargetValues(int size, double *values);

    virtual double GetError(int valueListIndex);
    virtual double GetError(double time);

    virtual std::string dump();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

private:

    NamedObject *m_Target = nullptr;
    std::vector<pgd::Quaternion> m_QValueList;
    int m_QValueListLength = -1;

};

#endif // DATATARGETQUATERNION_H
