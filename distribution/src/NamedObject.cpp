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

using namespace std::literals::string_literals;

NamedObject::NamedObject()
{
}

NamedObject::~NamedObject()
{
    if (dumpStream())
    {
        dumpStream()->close();
        delete(dumpStream());
    }
}

void NamedObject::Dump()
{
    if (dump() == false) return;

    if (firstDump())
    {
        setFirstDump(false);
        if (dumpStream() == nullptr)
        {
            if (GetName().size() == 0) std::cerr << "NamedObject::Dump error: can only dump a named object\n";
            std::string filename(GetName());
            filename.append(".dump");
            setDumpStream(new std::ofstream(filename));
            dumpStream()->precision(17);
        }
        if (dumpStream())
        {
            *dumpStream() << "Name\tm_Visible\n";
        }
    }

    if (dumpStream())
    {
        *dumpStream() << GetName() << "\n";
    }
}

// returns the value of a named attribute
// using caller provided string
// returns "" if attribute is not found
// also returns the pointer to the attribute or nullptr if not found
std::string *NamedObject::GetAttribute(const std::string &name, std::string *attributeValue)
{
    std::map<std::string, std::string>::iterator it;
    it = m_attributeMap.find(name);
    if (it == m_attributeMap.end())
    {
        attributeValue->clear();
        setLastError("Attribute \""s + name + "\" not found in ID=\""s + GetName() + "\""s);
        return nullptr;
    }
    *attributeValue = it->second;
    return attributeValue;
}

// returns the value of a named attribute
// returns "" if attribute is not found
std::string NamedObject::GetAttribute(const std::string &name)
{
    std::string attributeValue;
    GetAttribute(name, &attributeValue);
    return attributeValue;
}

// creates a new attribute and inserts it in alphabetical order
void NamedObject::setAttribute(const std::string &name, const std::string &attributeValue)
{
    m_attributeMap[name] = attributeValue;
}

const std::map<std::string, std::string> &NamedObject::getAttributeMap()
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

std::ofstream *NamedObject::dumpStream() const
{
    return m_dumpStream;
}

void NamedObject::setDumpStream(std::ofstream *dumpStream)
{
    m_dumpStream = dumpStream;
}

bool NamedObject::firstDump() const
{
    return m_firstDump;
}

void NamedObject::setFirstDump(bool firstDump)
{
    m_firstDump = firstDump;
}

bool NamedObject::dump() const
{
    return m_dump;
}

void NamedObject::setDump(bool dump)
{
   m_dump = dump;
}

bool NamedObject::visible() const
{
    return m_visible;
}

void NamedObject::setVisible(bool visible)
{
    m_visible = visible;
}

std::string *NamedObject::CreateFromAttributes()
{
    std::string buf;
    if (GetAttribute("ID"s, &buf) == nullptr) return lastErrorPtr();
    this->SetName(buf);
    if (GetAttribute("Group"s, &buf)) this->SetGroup(buf);
    if (GetAttribute("Size1"s, &buf)) m_size1 = GSUtil::Double(buf);
    if (GetAttribute("Size2"s, &buf)) m_size2 = GSUtil::Double(buf);
    if (GetAttribute("Size3"s, &buf)) m_size3 = GSUtil::Double(buf);
    if (GetAttribute("Colour1"s, &buf)) m_colour1.SetColour(buf);
    if (GetAttribute("Colour2"s, &buf)) m_colour2.SetColour(buf);
    if (GetAttribute("Colour3"s, &buf)) m_colour3.SetColour(buf);
    return nullptr;
}

void NamedObject::SaveToAttributes()
{
    m_tag = "NAMED_OBJECT"s;
    m_attributeMap.clear();
    this->AppendToAttributes();
}

void NamedObject::AppendToAttributes()
{
    setAttribute("ID"s, GetName());
    setAttribute("Group"s, GetGroup());
    std::string buf;
    setAttribute("Size1"s, *GSUtil::ToString(m_size1, &buf));
    setAttribute("Size2"s, *GSUtil::ToString(m_size2, &buf));
    setAttribute("Size3"s, *GSUtil::ToString(m_size3, &buf));
    setAttribute("Colour1"s, m_colour1.GetColourString());
    setAttribute("Colour2"s, m_colour2.GetColourString());
    setAttribute("Colour3"s, m_colour3.GetColourString());
}

void NamedObject::CreateAttributeMap(const std::vector<const char *> &names, const std::vector<size_t> &name_sizes, const std::vector<const char *> &values, const std::vector<size_t> &value_sizes)
{
    m_attributeMap.clear();
    for (size_t i = 0; i < names.size(); i++)
        setAttribute(std::string(names[i], name_sizes[i]), std::string(values[i], value_sizes[i]));
}

void NamedObject::CreateAttributeMap(const std::vector<std::string> &names, const std::vector<std::string> &values)
{
    m_attributeMap.clear();
    for (size_t i = 0; i < names.size(); i++)
        setAttribute(names[i], values[i]);
}

std::string NamedObject::SearchNames(const std::vector<const char *> &names, const std::vector<size_t> &name_sizes, const std::vector<const char *> &values, const std::vector<size_t> &value_sizes,
                                     const std::string &name, bool caseSensitive)
{
    for (size_t i = 0; i < names.size(); i++)
    {
        if (caseSensitive && strncmp(names[i], name.c_str(), name_sizes[i]) == 0) return std::string(values[i], value_sizes[i]);
        if (strncasecmp(names[i], name.c_str(), name_sizes[i]) == 0) return std::string(values[i], value_sizes[i]);
    }
    return std::string();
}

std::string NamedObject::SearchNames(const std::vector<std::string> &names, const std::vector<std::string> &values,
                                     const std::string &name, bool caseSensitive)
{
    for (size_t i = 0; i < names.size(); i++)
    {
        if (caseSensitive && strncmp(names[i].c_str(), name.c_str(), names[i].size()) == 0) return values[i];
        if (strncasecmp(names[i].c_str(), name.c_str(), names[i].size()) == 0) return values[i];
    }
    return std::string();
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

void NamedObject::SetName(const std::string &name)
{
    m_lastError.clear();
    if (!m_simulation)
    {
        m_lastError = "Warning: NamedObject::SetName(\""s + name + "\") called before NamedObject::setSimulation()"s;
        std::cerr << m_lastError << "\n";
    }
    else
    {
        auto it = m_simulation->GetObjectList()->find(name);
        if (it != m_simulation->GetObjectList()->end())
        {
            m_lastError = "Warning: NamedObject::SetName(\""s + name + "\") when name already exists"s;
            std::cerr << m_lastError << "\n";
        }
        else
        {
            (*m_simulation->GetObjectList())[name] = this;
        }
    }
    m_name = name;
}

std::string NamedObject::GetName() const // return value optimisation RVO makes via reference unnecessary
{
    return m_name;
}

void NamedObject::SetGroup(const std::string &group)
{
    m_lastError.clear();
    if (!m_simulation)
    {
        m_lastError = "Warning: NamedObject::SetGroup(\""s + group + "\") called before NamedObject::setSimulation()"s;
        std::cerr << m_lastError << "\n";
    }
    else
    {
        if (m_name.size() == 0)
        {
            m_lastError = "Warning: NamedObject::SetGroup(\""s + group + "\") called before NamedObject::SetName()"s;
            std::cerr << m_lastError << "\n";
        }
        else
        {
            if (group.size())
            {
                auto it = m_simulation->GetGroupList()->find(group);
                if (it != m_simulation->GetGroupList()->end())
                {
                    if (it->second.insert(m_name).second == false)
                    {
                        m_lastError = "Error: NamedObject::SetGroup(\""s + group + "\") already contains name \""s + m_name + "\"";
                        std::cerr << m_lastError << "\n";
                    }
                }
                else
                {
                    std::set<std::string> set = { m_name };
                    (*m_simulation->GetGroupList())[group] = std::move(set);
                }
            }
        }
    }
    m_group = group;
}

std::string NamedObject::GetGroup() const // return value optimisation RVO makes via reference unnecessary
{
    return m_group;
}

bool NamedObject::GetDump() const
{
    return m_dump;
}

void NamedObject::SetDump(bool v)
{
    m_dump = v;
}

void NamedObject::SetMessage(const char* message)
{
    m_message = message;
}

void NamedObject::SetMessage(const unsigned char* message)
{
    m_message = reinterpret_cast<const char *>(message);
}

void NamedObject::SetMessage(const std::string &message)
{
    m_message = message;
}

void NamedObject::SetMessage(const std::string *message)
{
    m_message = *message;
}

// const std::& GetMessage() const { return m_Message; }
std::string NamedObject::GetMessage() const // return value optimisation RVO makes via reference unnecessary
{
    return m_message;
}

