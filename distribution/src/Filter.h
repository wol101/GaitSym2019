/*
 *  Filter.h
 *  GaitSym2016
 *
 *  Created by Bill Sellers on Sat Jun 19 2016.
 *  Copyright (c) 2016 Bill Sellers. All rights reserved.
 *
 *  Base class for digital filters
 *
 */

#ifndef FILTER_H
#define FILTER_H


class Filter
{
public:
    Filter();
    virtual ~Filter();

    virtual void AddNewSample(double x);
    virtual double Output();


    double xn() const;
    void setXn(double xn);

private:
    double m_xn;
};

#endif // FILTER_H
