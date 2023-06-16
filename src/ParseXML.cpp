/*
 *  ParseXML.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 28/04/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#include "ParseXML.h"
#include "GSUtil.h"

#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#include "pystring.h"

using namespace std::literals::string_literals;

ParseXML::ParseXML()
{
}

std::string *ParseXML::LoadModel(const char *buffer, size_t length, const std::string &rootNodeTag) // note buffer must be a null terminated string (total length length + 1)
{
    m_inputConfigDoc.clear();
    m_elementList.clear();
    m_inputConfigData.assign(buffer, buffer + length + 1);
    rapidxml::xml_node<char> *cur;

    // do the basic XML parsing
    try
    {
        m_inputConfigDoc.parse<rapidxml::parse_no_data_nodes | rapidxml::parse_no_element_values>(m_inputConfigData.data());
        cur = m_inputConfigDoc.first_node();
        if (cur == nullptr)
        {
            setLastError("Error: Simulation::LoadModel - document empty"s);
            return lastErrorPtr();
        }
    }
    catch (const std::runtime_error& e)
    {
        setLastError("Error: Simulation::LoadModel runtime error: "s + std::string(e.what()));
        return lastErrorPtr();
    }
    catch (const rapidxml::parse_error& e)
    {
        std::string whereString(e.where<char>());
        setLastError("Error: Simulation::LoadModel parse error: "s + std::string(e.what()) + "\n"s + whereString.substr(0, 256));
        return lastErrorPtr();
    }
    catch (const std::exception& e)
    {
        setLastError("Error: Simulation::LoadModel exception: "s + std::string(e.what()));
        return lastErrorPtr();
    }
    catch (...)
    {
        setLastError("Error: Simulation::LoadModel undefined exception"s);
        return lastErrorPtr();
    }


    if (rootNodeTag != std::string(cur->name(), cur->name_size()))
    {
        setLastError("Error: Simulation::LoadModel - Document of the wrong type, root node != \""s + rootNodeTag + "\""s);
        return lastErrorPtr();
    }

    // now parse the elements in the file
    cur = cur->first_node();
    while (cur)
    {
        lastErrorPtr()->clear();
        auto xmlElement = std::make_unique<XMLElement>();
        xmlElement->tag.assign(cur->name(), cur->name_size());
        for (rapidxml::xml_attribute<char> *attr = cur->first_attribute(); attr; attr = attr->next_attribute())
        {
            xmlElement->attributes[std::string(attr->name(), attr->name_size())] = std::string(attr->value(), attr->value_size());
        }
        m_elementList.push_back(std::move(xmlElement));
        cur = cur->next_sibling();
    }
    return nullptr;
}

std::string ParseXML::SaveModel(const std::string &rootNodeTag, const std::string &comment)
{
    std::string xmlString;
    m_ouputConfigDoc.clear();

    // declaration first
    rapidxml::xml_node<char> *declarationNode = m_ouputConfigDoc.allocate_node(rapidxml::node_declaration);
    m_ouputConfigDoc.append_node(declarationNode); // must append the node before we start creating attributes
    CreateXMLAttribute(declarationNode, "version"s, "1.0"s, false); // note important that these are kept in this order
    CreateXMLAttribute(declarationNode, "encoding"s, "UTF-8"s, false);
    rapidxml::print(std::back_inserter(xmlString), *declarationNode);

    // now add the comment
    if (comment.size())
    {
        xmlString.append("<!--");
        xmlString.append(comment);
        xmlString.append("-->\n");
    }

    // create the root node
    rapidxml::xml_node<char> *rootNode = CreateXMLNode(&m_ouputConfigDoc, rootNodeTag);

    // create all the child nodes
    for (auto &&element : m_elementList)
    {
        rapidxml::xml_node<char> *node = CreateXMLNode(rootNode, element->tag);
        for (auto &&attribute : element->attributes)
        {
            CreateXMLAttribute(node, attribute.first, attribute.second, false);
        }
    }

    // convert to string and output
    rapidxml::print(std::back_inserter(xmlString), *rootNode, rapidxml::print_lf_after_attrib | rapidxml::print_indent_with_spaces);
    return xmlString;
}

// adds an element (tag plus attributes) to the internal list
void ParseXML::AddElement(const std::string &tag, const std::map<std::string, std::string> &attributeList)
{
    auto xmlElement = std::make_unique<XMLElement>();
    xmlElement->tag.assign(tag);
    xmlElement->attributes = attributeList;
    m_elementList.push_back(std::move(xmlElement));
}

// creates a new attribute and inserts it in alphabetical order
// returns a pointer to the attribute
rapidxml::xml_attribute<char> *ParseXML::CreateXMLAttribute(rapidxml::xml_node<char> *cur, const std::string &name, const std::string &newValue, bool sorted)
{
    lastErrorPtr()->clear();
    rapidxml::xml_attribute<char> *ptr = nullptr;
    if (sorted)
    {
        int res;
        rapidxml::xml_attribute<char> *attr = cur->first_attribute();
        while (attr)
        {
            res = strcmp(name.c_str(), attr->name());
            if (res == 0)
            {
                rapidxml::xml_attribute<char> *removeMe = attr;
                attr = attr->next_attribute();
                cur->remove_attribute(removeMe);
                continue;
            }
            if (res < 0)
            {
                char *allocatedName = cur->document()->allocate_string(name.data(), name.size());
                char *allocatedValue = cur->document()->allocate_string(newValue.data(), newValue.size());
                ptr = cur->document()->allocate_attribute(allocatedName, allocatedValue, name.size(), newValue.size());
                cur->insert_attribute(attr, ptr);
                break;
            }
            attr = attr->next_attribute();
        }
        if (!ptr)
        {
            char *allocatedName = cur->document()->allocate_string(name.data(), name.size());
            char *allocatedValue = cur->document()->allocate_string(newValue.data(), newValue.size());
            ptr = cur->document()->allocate_attribute(allocatedName, allocatedValue, name.size(), newValue.size());
            cur->append_attribute(ptr);
        }
    }
    else
    {
        char *allocatedName = cur->document()->allocate_string(name.data(), name.size());
        char *allocatedValue = cur->document()->allocate_string(newValue.data(), newValue.size());
        ptr = cur->document()->allocate_attribute(allocatedName, allocatedValue, name.size(), newValue.size());
        cur->append_attribute(ptr);
    }
    return ptr;
}

rapidxml::xml_node<char> *ParseXML::CreateXMLNode(rapidxml::xml_node<char> *parent, const std::string &name)
{
    char *allocatedName = parent->document()->allocate_string(name.data(), name.size());
    rapidxml::xml_node<char> *node = parent->document()->allocate_node(rapidxml::node_element, allocatedName, nullptr, name.size(), 0);
    parent->append_node(node);
    return node;
}

rapidxml::xml_node<char> *ParseXML::CreateXMLNode(rapidxml::xml_node<char> *parent, const std::string &name, const std::string newValue)
{
    char *allocatedName = parent->document()->allocate_string(name.data(), name.size());
    char *allocatedValue = parent->document()->allocate_string(newValue.data(), newValue.size());
    rapidxml::xml_node<char> *node = parent->document()->allocate_node(rapidxml::node_element, allocatedName, allocatedValue, name.size(), newValue.size());
    parent->append_node(node);
    return node;
}

// removes a named attribute if it exists
// returns true if an attribute is removed
bool ParseXML::RemoveXMLAttribute(rapidxml::xml_node<char> *cur, const std::string &name, bool caseSensitive)
{
    rapidxml::xml_attribute<char> *ptr = FindXMLAttribute(cur, name, caseSensitive);
    if (ptr)
    {
        cur->remove_attribute(ptr);
        return true;
    }
    else
    {
        return false;
    }
}

// returns a pointer to an attribute if it exists
rapidxml::xml_attribute<char> *ParseXML::FindXMLAttribute(rapidxml::xml_node<char> *cur, const std::string &name, bool caseSensitive)
{
    int res;
    rapidxml::xml_attribute<char> *ptr = nullptr;
    for (rapidxml::xml_attribute<char> *attr = cur->first_attribute(); attr; attr = attr->next_attribute())
    {
        if (caseSensitive) res = strcmp(name.c_str(), attr->name());
        else res = strcasecmp(name.c_str(), attr->name());
        if (res == 0)
        {
            ptr = attr;
            goto found;
        }
    }
    setLastError("Attribute \""s + name + "\" not found in ID=\""s + this->name() + "\""s);
    return ptr;

found:
    lastErrorPtr()->clear();
    return ptr;
}

// returns the value of a named attribute
// using caller provided string
// returns "" if attribute is not found
// also returns the pointer to the attribute or nullptr if not found
rapidxml::xml_attribute<char> *ParseXML::GetXMLAttribute(rapidxml::xml_node<char> *cur, const std::string &name, std::string *attributeValue, bool caseSensitive)
{
    rapidxml::xml_attribute<char> *ptr = FindXMLAttribute(cur, name, caseSensitive);
    if (ptr)
    {
        attributeValue->assign(ptr->value(), ptr->value_size());
    }
    else
    {
        attributeValue->clear();
    }
    return ptr;
}

std::vector<std::unique_ptr<ParseXML::XMLElement>> *ParseXML::elementList()
{
    return &m_elementList;
}

#if defined(RAPIDXML_NO_EXCEPTIONS)
// this is the required rapidxml error handler when RAPIDXML_NO_EXCEPTIONS is used to disable exceptions
void rapidxml::parse_error_handler(const char *what, void *where)
{
    std::cerr << "rapidxml::parse_error_handler Parse error (what) " << what << "\n";
    std::cerr << "(where) " << where << "\n";
}
#endif


