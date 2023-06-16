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

#include <typeinfo>

#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#endif

#if defined(GAITSYM_DEBUG_BUILD) && defined(GAITSYM_MEMORY_ALLOCATION_DEBUG)
uint64_t Drawable::m_objectCount = 0;
#endif

Drawable::Drawable()
{
//    this doesn't work because during the constructor we know nothing about derived classes
//#if defined(GAITSYM_DEBUG_BUILD) && defined(GAITSYM_MEMORY_ALLOCATION_DEBUG)
//    m_objectCountAtCreation = m_objectCount++;
//    std::cerr << m_objectCountAtCreation << " " << className() << " constructed\n";;
//#endif
}

Drawable::~Drawable()
{
//    this doesn't work because during the destructor we know nothing about derived classes
//#if defined(GAITSYM_DEBUG_BUILD) && defined(GAITSYM_MEMORY_ALLOCATION_DEBUG)
//    std::cerr << m_objectCountAtCreation << " " << className() << " destructed\n";;
//#endif
}

void Drawable::SetDisplayPosition(double x, double y, double z)
{
    for (auto &&iter : m_facetedObjectList) iter->SetDisplayPosition(x, y, z);
}

void Drawable::SetDisplayScale(double x, double y, double z)
{
    for (auto &&iter : m_facetedObjectList) iter->SetDisplayScale(x, y, z);
}

void Drawable::SetDisplayRotation(const dMatrix3 R)
{
    for (auto &&iter : m_facetedObjectList) iter->SetDisplayRotation(R);
}

void Drawable::SetDisplayRotationFromQuaternion(const dQuaternion q)
{
    for (auto &&iter : m_facetedObjectList) iter->SetDisplayRotationFromQuaternion(q);
}

void Drawable::setVisible(bool visible)
{
    for (auto &&iter : m_facetedObjectList) iter->setVisible(visible);
}

std::string Drawable::className() const
{
    std::string className(typeid(*this).name());
#ifdef __GNUG__
    int status = -4;
    std::unique_ptr<char, void(*)(void*)> res
    {
        abi::__cxa_demangle(className.c_str(), NULL, NULL, &status),
        std::free
    };
    if (status == 0) className.assign(res.get());
#endif
    return className;
}

const std::vector<FacetedObject *> &Drawable::facetedObjectList() const
{
    return m_facetedObjectList;
}




