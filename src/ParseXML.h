/*
 *  ParseXML.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 28/04/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#ifndef PARSEXML_H
#define PARSEXML_H

#include "NamedObject.h"
#include "rapidxml.hpp"

#include <string>
#include <vector>
#include <memory>

class ParseXML : NamedObject
{
public:
    ParseXML();

    struct XMLElement
    {
        std::string tag;
        std::map<std::string, std::string> attributes;
    };

    void AddElement(const std::string &tag, const std::map<std::string, std::string> &attributeList);

    std::string *LoadModel(const char *buffer, size_t length, const std::string &rootNodeTag);
    std::string SaveModel(const std::string &rootNodeTag, const std::string &comment);

    std::vector<std::unique_ptr<XMLElement>> *elementList();

private:
    rapidxml::xml_attribute<char> *CreateXMLAttribute(rapidxml::xml_node<char> *cur, const std::string &name, const std::string &newValue, bool sorted);
    rapidxml::xml_node<char> *CreateXMLNode(rapidxml::xml_node<char> *parent, const std::string &name);
    rapidxml::xml_node<char> *CreateXMLNode(rapidxml::xml_node<char> *parent, const std::string &name, const std::string newValue);
    bool RemoveXMLAttribute(rapidxml::xml_node<char> *cur, const std::string &name, bool caseSensitive);
    rapidxml::xml_attribute<char> *FindXMLAttribute(rapidxml::xml_node<char> *cur, const std::string &name, bool caseSensitive);
    rapidxml::xml_attribute<char> *GetXMLAttribute(rapidxml::xml_node<char> *cur, const std::string &name, std::string *attributeValue, bool caseSensitive);

    rapidxml::xml_document<char> m_inputConfigDoc;
    rapidxml::xml_document<char> m_ouputConfigDoc;
    std::vector<char> m_inputConfigData;
    std::vector<std::unique_ptr<XMLElement>> m_elementList;
};

#endif // PARSEXML_H
