/*
 *  NamedObject.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 19/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

// Root object that allows some basic data storage such as naming

#include "NamedObject.h"
#include "Simulation.h"
#include "GSUtil.h"

#include "pystring.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <typeinfo>

#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#endif

using namespace std::literals::string_literals;

NamedObject::NamedObject()
{
}

NamedObject::~NamedObject()
{
}

std::string NamedObject::dumpToString()
{
    std::string s;
    if (firstDump()) { s = dumpHelper({"size1"s, "size2"s, "size3"s, "visible"s}); setFirstDump(false); }
    s += dumpHelper({m_size1, m_size2, m_size3, double(m_visible)});
    return s;
}

// returns the value of a named attribute
// using caller provided string
// returns "" if attribute is not found
// also returns the pointer to the attribute or nullptr if not found
std::string *NamedObject::findAttribute(const std::string &name, std::string *attributeValue)
{
    std::map<std::string, std::string>::iterator it;
    it = m_attributeMap.find(name);
    if (it == m_attributeMap.end())
    {
        attributeValue->clear();
        setLastError("Attribute \""s + name + "\" not found in ID=\""s + this->name() + "\""s);
        return nullptr;
    }
    *attributeValue = it->second;
    return attributeValue;
}

// returns the value of a named attribute
// returns "" if attribute is not found
std::string NamedObject::findAttribute(const std::string &name)
{
    std::string attributeValue;
    findAttribute(name, &attributeValue);
    return attributeValue;
}

const std::map<std::string, std::string> &NamedObject::serialise()
{
    saveToAttributes();
    return m_attributeMap;
}

std::string *NamedObject::unserialise(const std::map<std::string, std::string> &serialiseMap)
{
    m_attributeMap = serialiseMap;
    return createFromAttributes();
}

// creates a new attribute and inserts it in alphabetical order
void NamedObject::setAttribute(const std::string &name, const std::string &attributeValue)
{
    m_attributeMap[name] = attributeValue;
}

const std::map<std::string, std::string> &NamedObject::attributeMap()
{
    return m_attributeMap;
}

std::string NamedObject::getTag() const
{
    return m_tag;
}

void NamedObject::setTag(const std::string &tag)
{
    m_tag = tag;
}

std::string NamedObject::lastError() const
{
    return m_lastError;
}

std::string *NamedObject::lastErrorPtr()
{
    return &m_lastError;
}

void NamedObject::setLastError(const std::string &lastError)
{
    m_lastError = lastError;
}

bool NamedObject::visible() const
{
    return m_visible;
}

void NamedObject::setVisible(bool visible)
{
    m_visible = visible;
}

std::string *NamedObject::createFromAttributes()
{
    std::string buf;
    if (findAttribute("ID"s, &buf) == nullptr) return lastErrorPtr();
    this->setName(buf);
    if (findAttribute("Group"s, &buf)) this->setGroup(buf);
    if (findAttribute("Size1"s, &buf)) m_size1 = GSUtil::Double(buf);
    if (findAttribute("Size2"s, &buf)) m_size2 = GSUtil::Double(buf);
    if (findAttribute("Size3"s, &buf)) m_size3 = GSUtil::Double(buf);
    if (findAttribute("Colour1"s, &buf)) m_colour1.SetColour(buf);
    if (findAttribute("Colour2"s, &buf)) m_colour2.SetColour(buf);
    if (findAttribute("Colour3"s, &buf)) m_colour3.SetColour(buf);
    return nullptr;
}

void NamedObject::saveToAttributes()
{
    m_tag = "NAMED_OBJECT"s;
    m_attributeMap.clear();
    this->appendToAttributes();
}

void NamedObject::appendToAttributes()
{
    setAttribute("ID"s, name());
    setAttribute("Group"s, group());
    std::string buf;
    setAttribute("Size1"s, *GSUtil::ToString(m_size1, &buf));
    setAttribute("Size2"s, *GSUtil::ToString(m_size2, &buf));
    setAttribute("Size3"s, *GSUtil::ToString(m_size3, &buf));
    setAttribute("Colour1"s, m_colour1.GetIntColourRGBA());
    setAttribute("Colour2"s, m_colour2.GetIntColourRGBA());
    setAttribute("Colour3"s, m_colour3.GetIntColourRGBA());
}

void NamedObject::createAttributeMap(const std::map<std::string, std::string> &attributeMap)
{
    m_attributeMap = attributeMap;
}

std::string NamedObject::searchNames(const std::map<std::string, std::string> &attributeMap, const std::string &name)
{
    auto it = attributeMap.find(name);
    if (it != attributeMap.end()) return it->second;
    return std::string();
}

std::string NamedObject::dumpHelper(std::initializer_list<std::string> values)
{
    std::stringstream ss;
    auto &&it = values.begin();
    if (it != values.end())
    {
        ss << *it++;
        for (; it != values.end(); it++)
        {
            ss << "\t" << *it;
        }
    }
    ss << "\n";
    return ss.str();
}

std::string NamedObject::dumpHelper(std::initializer_list<double> values)
{
    std::stringstream ss;
    auto &&it = values.begin();
    if (it != values.end())
    {
        ss << std::setprecision(18) << *it++; // this defaults to %.18g format if neither fixed nor scientific is set
        for (; it != values.end(); it++)
        {
            ss << std::setprecision(18) << "\t" << *it;
        }
    }
    ss << "\n";
    return ss.str();
}

std::vector<NamedObject *> *NamedObject::upstreamObjects()
{
    return &m_upstreamObjects;
}

void NamedObject::setUpstreamObjects(const std::vector<NamedObject *> &&upstreamObjects)
{
    m_upstreamObjects = std::move(upstreamObjects);
}

void NamedObject::allUpstreamObjects(std::vector<NamedObject *> *upstreamObjects)
{
    if (m_upstreamObjects.size() == 0) return;
    for (auto &&it : m_upstreamObjects)
    {
        it->allUpstreamObjects(upstreamObjects);
        upstreamObjects->push_back(it);
    }
}

bool NamedObject::isUpstreamObject(NamedObject *findObject)
{
    if (m_upstreamObjects.size() == 0) return false;
    for (auto &&it : m_upstreamObjects)
    {
        if (it == findObject) return true;
        if (it->isUpstreamObject(findObject)) return true;
    }
    return false;
}

void NamedObject::clearAttributeMap()
{
    m_attributeMap.clear();
}

std::string NamedObject::className() const
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

bool NamedObject::firstDump() const
{
    return m_firstDump;
}

void NamedObject::setFirstDump(bool firstDump)
{
    m_firstDump = firstDump;
}

bool NamedObject::redraw() const
{
    return m_redraw;
}

void NamedObject::setRedraw(bool redraw)
{
    m_redraw = redraw;
}

bool NamedObject::dump() const
{
    return m_dump;
}

void NamedObject::setDump(bool dump)
{
    m_dump = dump;
}

Simulation *NamedObject::simulation() const
{
    return m_simulation;
}

void NamedObject::setSimulation(Simulation *simulation)
{
    m_simulation = simulation;
}

Colour NamedObject::colour3() const
{
    return m_colour3;
}

void NamedObject::setColour3(const Colour &colour3)
{
    m_colour3 = colour3;
}

Colour NamedObject::colour2() const
{
    return m_colour2;
}

void NamedObject::setColour2(const Colour &colour2)
{
    m_colour2 = colour2;
}

Colour NamedObject::colour1() const
{
    return m_colour1;
}

void NamedObject::setColour1(const Colour &colour1)
{
    m_colour1 = colour1;
}

double NamedObject::size3() const
{
    return m_size3;
}

void NamedObject::setSize3(double size3)
{
    m_size3 = size3;
}

double NamedObject::size2() const
{
    return m_size2;
}

void NamedObject::setSize2(double size2)
{
    m_size2 = size2;
}

double NamedObject::size1() const
{
    return m_size1;
}

void NamedObject::setSize1(double size1)
{
    m_size1 = size1;
}

void NamedObject::setName(const std::string &name)
{
    m_name = name;
}

std::string NamedObject::name() const // return value optimisation RVO makes via reference unnecessary
{
    return m_name;
}

void NamedObject::setGroup(const std::string &group)
{
    m_group = group;
}

std::string NamedObject::group() const // return value optimisation RVO makes via reference unnecessary
{
    return m_group;
}

void NamedObject::setObjectMessage(const char* message)
{
    m_message = message;
}

void NamedObject::setObjectMessage(const unsigned char* message)
{
    m_message = reinterpret_cast<const char *>(message);
}

void NamedObject::setObjectMessage(const std::string &message)
{
    m_message = message;
}

void NamedObject::setObjectMessage(const std::string *message)
{
    m_message = *message;
}

// const std::& GetMessage() const { return m_Message; }
std::string NamedObject::objectMessage() const // return value optimisation RVO makes via reference unnecessary
{
    return m_message;
}

