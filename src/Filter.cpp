/*
 *  Filter.cpp
 *  GaitSym2016
 *
 *  Created by Bill Sellers on Sat Jun 19 2016.
 *  Copyright (c) 2016 Bill Sellers. All rights reserved.
 *
 *  Base class for digital filters
 *
 */

#include "Filter.h"

Filter::Filter()
{
    m_xn = 0;
}

Filter::~Filter()
{

}

void Filter::AddNewSample(double x)
{
    m_xn = x;
}

double Filter::Output()
{
    return m_xn;
}

double Filter::xn() const
{
    return m_xn;
}

void Filter::setXn(double xn)
{
    m_xn = xn;
}


