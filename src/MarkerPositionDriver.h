/*
 *  MarkerPositionDriver.h
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 11th May 2022.
 *  Copyright (c) 2022 Bill Sellers. All rights reserved.
 *
 *  Sets the values of the marker position at specified times
 *
 */

#ifndef MarkerPositionDriver_h
#define MarkerPositionDriver_h

#include "Driver.h"

class Marker;

class MarkerPositionDriver: public Driver
{
public:
    MarkerPositionDriver();
    virtual ~MarkerPositionDriver();

    virtual void Update();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

private:

    std::vector<double> m_xPositionList;
    std::vector<double> m_yPositionList;
    std::vector<double> m_zPositionList;
    std::vector<double> m_changeTimes;
    std::vector<Marker *> m_targetMarkerList;
    Marker *m_referenceMarker = nullptr;

    size_t m_index = 0;
};

#endif
