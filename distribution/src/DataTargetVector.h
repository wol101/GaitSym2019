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

namespace irr { namespace scene { class IMeshSceneNode; } }

class DataTargetVector : public DataTarget
{
public:
    DataTargetVector();
    virtual ~DataTargetVector();

    virtual void SetTargetValues(int size, double *values);
    void SetTargetValues(const char *buf);
    virtual double GetError(int valueListIndex);
    virtual double GetError(double time);

    virtual void Dump();

private:

    pgd::Vector *m_VValueList;
    int m_VValueListLength;

};

#endif // DATATARGETVECTOR_H
