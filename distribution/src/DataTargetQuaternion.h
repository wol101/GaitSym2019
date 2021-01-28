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

class DataTargetQuaternion : public DataTarget
{
public:
    DataTargetQuaternion();

    void SetTarget(NamedObject *target);
    NamedObject *GetTarget();

    virtual std::string dumpToString();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

    virtual double calculateError(size_t valueListIndex);
    virtual double calculateError(double time);

private:

    NamedObject *m_Target = nullptr;
    std::vector<pgd::Quaternion> m_QValueList;

};

#endif // DATATARGETQUATERNION_H
