/*
 *  Drawable.cpp
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 08/10/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#include "Drawable.h"
#include "FacetedObject.h"

#include <memory>

Drawable::Drawable()
{

}

Drawable::~Drawable()
{

}

void Drawable::SetDisplayPosition(double x, double y, double z)
{
    for (auto iter : m_facetedObjectList) iter->SetDisplayPosition(x, y, z);
}

void Drawable::SetDisplayScale(double x, double y, double z)
{
    for (auto iter : m_facetedObjectList) iter->SetDisplayScale(x, y, z);
}

void Drawable::SetDisplayRotation(const dMatrix3 R)
{
    for (auto iter : m_facetedObjectList) iter->SetDisplayRotation(R);
}

void Drawable::SetDisplayRotationFromQuaternion(const dQuaternion q)
{
    for (auto iter : m_facetedObjectList) iter->SetDisplayRotationFromQuaternion(q);
}

void Drawable::setVisible(bool visible)
{
    for (auto iter : m_facetedObjectList) iter->setVisible(visible);
}

const std::vector<FacetedObject *> &Drawable::facetedObjectList() const
{
    return m_facetedObjectList;
}




