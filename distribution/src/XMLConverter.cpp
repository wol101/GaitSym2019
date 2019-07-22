/*
 *  XMLConverter.cc
 *  GA
 *
 *  Created by Bill Sellers on Fri Dec 12 2003.
 *  Copyright (c) 2003 Bill Sellers. All rights reserved.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include <cfloat>
#include <cmath>
#include <assert.h>
#include <iostream>

#include "XMLConverter.h"
#include "DataFile.h"

#include "exprtk.hpp"

XMLConverter::XMLConverter()
{
}

XMLConverter::~XMLConverter()
{
    unsigned int i;
    if (m_SmartSubstitutionTextBuffer) delete m_SmartSubstitutionTextBuffer;
    for (i = 0; i < m_SmartSubstitutionTextComponents.size(); i++)
        delete m_SmartSubstitutionTextComponents[i];
    for (i = 0; i < m_SmartSubstitutionParserText.size(); i++)
        delete m_SmartSubstitutionParserText[i];
}

// load the base file for smart substitution file
int XMLConverter::LoadBaseXMLFile(const char *filename)
{
    DataFile smartSubstitutionBaseXMLFile;
    if (smartSubstitutionBaseXMLFile.ReadFile(filename)) return 1;
    LoadBaseXMLString(smartSubstitutionBaseXMLFile.GetRawData());
    return 0;
}

// load the base XML for smart substitution file
int XMLConverter::LoadBaseXMLString(char *dataPtr)
{
    unsigned int i;
    if (m_SmartSubstitutionTextBuffer) delete m_SmartSubstitutionTextBuffer;
    for (i = 0; i < m_SmartSubstitutionTextComponents.size(); i++)
        delete m_SmartSubstitutionTextComponents[i];
    for (i = 0; i < m_SmartSubstitutionParserText.size(); i++)
        delete m_SmartSubstitutionParserText[i];
    m_SmartSubstitutionTextComponents.clear();
    m_SmartSubstitutionParserText.clear();
    m_SmartSubstitutionValues.clear();

    char *ptr1 = dataPtr;
    std::string *s;
    std::string *expressionParserText;
    int length = strlen(dataPtr);

    char *ptr2 = strstr(ptr1, "[[");
    while (ptr2)
    {
        *ptr2 = 0;
        s = new std::string(ptr1);
        m_SmartSubstitutionTextComponents.push_back(s);

        ptr2 += 2;
        ptr1 = strstr(ptr2, "]]");
        if (ptr2 == 0)
        {
            std::cerr << "Error: could not find matching ]]\n";
            exit(1);
        }
        expressionParserText = new std::string(ptr2, static_cast<int>(ptr1 - ptr2));
        m_SmartSubstitutionParserText.push_back(expressionParserText);
        m_SmartSubstitutionValues.push_back(0); // dummy values
        ptr1 += 2;
        ptr2 = strstr(ptr1, "[[");
    }
    s = new std::string(ptr1);
    m_SmartSubstitutionTextComponents.push_back(s);

    // make the text buffer plenty big enough
    m_SmartSubstitutionTextBuffer = new char[length + m_SmartSubstitutionValues.size() * 64]; // this assumes that the numbers produced for each substition are less than 64 bytes long

    m_SmartSubstitutionFlag = true;

    // get the vector brackets in the right format for exprtk if necessary
    ConvertVectorBrackets();

    return 0;
}

char *XMLConverter::GetFormattedXML(int *docTxtLen)
{
    if (m_SmartSubstitutionTextBuffer == 0) return 0;
    char *ptr = m_SmartSubstitutionTextBuffer;
    unsigned int i;
    int chars;
    for (i = 0; i < m_SmartSubstitutionValues.size(); i++)
    {
        chars = sprintf(ptr, "%s", m_SmartSubstitutionTextComponents[i]->c_str());
        ptr += chars;
        chars = sprintf(ptr, "%.17e", m_SmartSubstitutionValues[i]);
        ptr += chars;
    }
    chars = sprintf(ptr, "%s", m_SmartSubstitutionTextComponents[i]->c_str());
    ptr += chars;
    *docTxtLen = (int)(ptr - m_SmartSubstitutionTextBuffer);
    return m_SmartSubstitutionTextBuffer;
}

// this needs to be customised depending on how the genome interacts with
// the XML file specifying the simulation
int XMLConverter::ApplyGenome(int genomeSize, double *genomeData)
{
    exprtk::symbol_table<double> *symbol_table;
    exprtk::expression<double> *expression;
    exprtk::parser<double> *parser;

    for (unsigned int i = 0; i < m_SmartSubstitutionParserText.size(); i++)
    {
        // set up the genome as a function g(locus) and evaluate

        symbol_table = new exprtk::symbol_table<double>();
        expression = new exprtk::expression<double>();
        parser = new exprtk::parser<double>();

        symbol_table->add_vector("g", genomeData, genomeSize);
        symbol_table->add_constants();

        expression->register_symbol_table(*symbol_table);

//        std::cerr << "substitution text " << i << ": " << *m_SmartSubstitutionParserText[i] << "\n";
        bool success = parser->compile(*m_SmartSubstitutionParserText[i], *expression);

        if (success)
        {
            m_SmartSubstitutionValues[i] = expression->value();
//            std::cerr << "substitution value " << i << " = " << m_SmartSubstitutionValues[i] << "\n";
        }
        else
        {
            std::cerr << "Error: XMLConverter::ApplyGenome m_SmartSubstitutionParserComponents[" << i << "] does not evaluate to a number\n";
            std::cerr << "Applying standard fix up and setting to zero\n";
            m_SmartSubstitutionValues[i] = 0;
        }

        delete parser;
        delete expression;
        delete symbol_table;
    }

    return 0;
}

// exprtk requires [] around vector indices whereas my parser used ()
// this routine converts the brackets around the g vector
void XMLConverter::ConvertVectorBrackets()
{
    std::string *s;
    int pCount;
    unsigned int j;
    for (unsigned int i = 0; i < m_SmartSubstitutionParserText.size(); i++)
    {
        s =  m_SmartSubstitutionParserText[i];
        j = 0;
        while (j < s->size())
        {
            if (s->at(j) == 'g')
            {
                j++;
                if (j >= s->size()) break;
                while (j < s->size())
                {
                    if (s->at(j) < 33) j++; // this just skips any whitespace
                    else break;
                }
                if (j >= s->size()) break;
                if (s->at(j) == '(')
                {
                    s->at(j) = '[';
                    pCount = 1;
                    while (j < s->size())
                    {
                        if (s->at(j) == '(') pCount++;
                        if (s->at(j) == ')') pCount--;
                        if (pCount <= 0)
                        {
                            s->at(j) = ']';
                            break;
                        }
                        j++;
                    }
                }
            }
            j++;
        }
    }
}



