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

class FacetedObject;
class Simulation;

namespace rapidxml { template<class Ch> class xml_node; }

class NamedObject
{
public:

    NamedObject();
    virtual ~NamedObject();

    void SetName(const std::string &name);
    std::string GetName() const; // return value optimisation RVO makes returning via reference unnecessary

    void SetGroup(const std::string &group);
    std::string GetGroup() const; // return value optimisation RVO makes returning via reference unnecessary

    bool GetDump() const;
    void SetDump(bool v);

    void SetMessage(const char* message);
    void SetMessage(const unsigned char* message);
    void SetMessage(const std::string &message);
    void SetMessage(const std::string *message);
    std::string GetMessage() const; // return value optimisation RVO makes via reference unnecessary

    virtual void Dump();
    void CreateAttributeMap(const std::vector<const char *> &names, const std::vector<size_t> &name_sizes, const std::vector<const char *> &values, const std::vector<size_t> &value_sizes);
    void CreateAttributeMap(const std::vector<std::string> &names, const std::vector<std::string> &values);
    virtual std::string *CreateFromAttributes();
    virtual void SaveToAttributes();
    virtual void AppendToAttributes();
    std::string GetAttribute(const std::string &name);

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


    bool dump() const;
    void setDump(bool dump);

    bool firstDump() const;
    void setFirstDump(bool firstDump);

    std::ofstream *dumpStream() const;
    void setDumpStream(std::ofstream *dumpStream);

    std::string lastError() const;
    std::string *lastErrorPtr();
    void setLastError(const std::string &lastError);

    std::string getTag() const;
    void setTag(const std::string &tag);

    const std::map<std::string, std::string> &getAttributeMap();

    // utility function
    static std::string SearchNames(const std::vector<const char *> &names, const std::vector<size_t> &name_sizes, const std::vector<const char *> &values, const std::vector<size_t> &value_sizes,
                                   const std::string &name, bool caseSensitive);
    static std::string SearchNames(const std::vector<std::string> &names, const std::vector<std::string> &values,
                                   const std::string &name, bool caseSensitive);


protected:
    std::string *GetAttribute(const std::string &name, std::string *attributeValue);
    void setAttribute(const std::string &name, const std::string &attributeValue);

private:

    std::string m_name;
    std::string m_group;
    std::string m_message;
    std::string m_lastError;

    bool m_dump = false;
    bool m_firstDump = true;
    std::ofstream *m_dumpStream = nullptr;

    Simulation *m_simulation = nullptr;

    double m_size1 = 0;
    double m_size2 = 0;
    double m_size3 = 0;
    Colour m_colour1;
    Colour m_colour2;
    Colour m_colour3;
    bool m_visible = true;

    std::map<std::string, std::string> m_attributeMap;
    std::string m_tag;
};

#endif
