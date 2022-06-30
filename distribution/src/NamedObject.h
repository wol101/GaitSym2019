/*
 *  NamedObject.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 19/08/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

// Root object that allows some basic data storage such as naming

#ifndef NamedObject_h
#define NamedObject_h

#include "Colour.h"

#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <initializer_list>

class Simulation;

namespace rapidxml { template<class Ch> class xml_node; }

class NamedObject
{
public:

    NamedObject();
    virtual ~NamedObject();

    // destructor might be needed to clean up resources but I don't need the rest of the rule of 5
    NamedObject(const NamedObject&) = delete;
    NamedObject& operator=(const NamedObject&) = delete;
    NamedObject(NamedObject&&) = delete;
    NamedObject& operator=(NamedObject&&) = delete;

    void setName(const std::string &name);
    std::string name() const; // return value optimisation RVO makes returning via reference unnecessary

    void setGroup(const std::string &group);
    std::string group() const; // return value optimisation RVO makes returning via reference unnecessary

    void setObjectMessage(const char* message);
    void setObjectMessage(const unsigned char* message);
    void setObjectMessage(const std::string &message);
    void setObjectMessage(const std::string *message);
    std::string objectMessage() const; // return value optimisation RVO makes via reference unnecessary

    std::string className() const; // return value optimisation RVO makes via reference unnecessary

    virtual std::string dumpToString();
    void createAttributeMap(const std::map<std::string, std::string> &attributeMap);
    virtual std::string *createFromAttributes();
    virtual void saveToAttributes();
    virtual void appendToAttributes();
    std::string findAttribute(const std::string &name);
    virtual const std::map<std::string, std::string> &serialise();
    virtual std::string *unserialise(const std::map<std::string, std::string> &serialiseMap);

    std::vector<NamedObject *> *upstreamObjects();
    void setUpstreamObjects(const std::vector<NamedObject *> &&upstreamObjects);
    void allUpstreamObjects(std::vector<NamedObject *> *upstreamObjects);
    bool isUpstreamObject(NamedObject *findObject);

    Simulation *simulation() const;
    void setSimulation(Simulation *simulation);

    double size1() const;
    void setSize1(double size1);

    double size2() const;
    void setSize2(double size2);

    double size3() const;
    void setSize3(double size3);

    Colour colour1() const;
    void setColour1(const Colour &colour1);

    Colour colour2() const;
    void setColour2(const Colour &colour2);

    Colour colour3() const;
    void setColour3(const Colour &colour3);

    bool visible() const;
    void setVisible(bool visible);

    std::string lastError() const;
    std::string *lastErrorPtr();
    void setLastError(const std::string &lastError);

    std::string getTag() const;
    void setTag(const std::string &tag);

    const std::map<std::string, std::string> &attributeMap();

    bool dump() const;
    void setDump(bool dumpToString);
    bool firstDump() const;

    bool redraw() const;
    void setRedraw(bool redraw);

    // utility function
    static std::string searchNames(const std::map<std::string, std::string> &attributeMap, const std::string &name);
    static std::string dumpHelper(std::initializer_list<std::string> values);
    static std::string dumpHelper(std::initializer_list<double> values);


protected:
    std::string *findAttribute(const std::string &name, std::string *attributeValue);
    void setAttribute(const std::string &name, const std::string &attributeValue);
    void clearAttributeMap();
    void setFirstDump(bool firstDump);

private:

    std::string m_name;
    std::string m_group;
    std::string m_message;
    std::string m_lastError;

    Simulation *m_simulation = nullptr;

    double m_size1 = 0;
    double m_size2 = 0;
    double m_size3 = 0;
    Colour m_colour1;
    Colour m_colour2;
    Colour m_colour3;
    bool m_visible = true;
    bool m_redraw = true;

    bool m_dump = false;
    bool m_firstDump = true;

    std::map<std::string, std::string> m_attributeMap;
    std::string m_tag;
    std::vector<NamedObject *> m_upstreamObjects;
};

#endif
