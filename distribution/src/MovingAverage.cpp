/*
 *  MovingAverage.cpp
 *  GaitSym2016
 *
 *  Created by Bill Sellers on Sat Jun 4 2016.
 *  Copyright (c) 2016 Bill Sellers. All rights reserved.
 *
 *  Implements a Moving Average FIR Filter
 *
 */

#include "MovingAverage.h"

#include <algorithm>

MovingAverage::MovingAverage() : Filter()
{
}

MovingAverage::MovingAverage(int window)  : Filter()
{
    m_window = window;
    m_buffer.resize(size_t(m_window));
    m_index = 0;
    m_sum = 0;
    m_average = 0;
}

void MovingAverage::InitialiseBuffer(int window)
{
    m_window = window;
    m_buffer.clear();
    m_buffer.resize(size_t(m_window));
    m_index = 0;
    m_sum = 0;
    m_average = 0;
}

void MovingAverage::AddNewSample(double x)
{
    m_index = ++m_index % m_window; // increment and modulus implementing ring buffer
    m_sum -= m_buffer[size_t(m_index)];
    m_buffer[size_t(m_index)] = x;
    m_sum += x;
    m_average = m_sum / m_window;
}

double MovingAverage::Output()
{
    return m_average;
}

double MovingAverage::sum() const
{
    return m_sum;
}

double MovingAverage::average() const
{
    return m_average;
}

int MovingAverage::window() const
{
    return m_window;
}
