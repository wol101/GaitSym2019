/*
 *  MovingAverage.h
 *  GaitSym2016
 *
 *  Created by Bill Sellers on Sat Jun 4 2016.
 *  Copyright (c) 2016 Bill Sellers. All rights reserved.
 *
 *  Implements a Moving Average FIR Filter
 *
 */

#ifndef MOVINGAVERAGE_H
#define MOVINGAVERAGE_H

#include "Filter.h"

#include <vector>

class MovingAverage : public Filter
{
public:
    MovingAverage();
    MovingAverage(int window);

    virtual void AddNewSample(double x);
    virtual double Output();

    void InitialiseBuffer(int window);

    double sum() const;
    double average() const;
    int window() const;

private:
    int m_window = 0;
    int m_index = 0;
    std::vector<double> m_buffer;
    double m_sum = 0;
    double m_average = 0;
};

#endif // MOVINGAVERAGE_H
