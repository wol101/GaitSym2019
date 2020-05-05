/*
 *  DataTargetVector.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Tue July 14 2009.
 *  Copyright (c) 1009 Bill Sellers. All rights reserved.
 *
 */

#ifndef DATATARGETVECTOR_H
#define DATATARGETVECTOR_H

#include "DataTarget.h"
#include "PGDMath.h"

#include <iostream>

class DataTargetVector : public DataTarget
{
public:
    DataTargetVector();
    virtual ~DataTargetVector();

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
    std::vector<pgd::Vector> m_VValueList;
    int m_VValueListLength = -1;

};

#endif // DATATARGETVECTOR_H
