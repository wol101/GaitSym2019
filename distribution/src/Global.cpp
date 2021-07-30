/*
 *  Global.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 11/11/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#include "Global.h"
#include "GSUtil.h"
#include "Simulation.h"

#include "pystring.h"

#include <string>
#include <algorithm>

using namespace std::string_literals;

Global::Global()
{
    m_SpringConstant = m_ERP / (m_CFM * m_StepSize);
    m_DampingConstant = (1.0 - m_ERP) / m_CFM;
}

Global::~Global()
{
}

double Global::SpringConstant() const
{
    return m_SpringConstant;
}

void Global::setSpringConstant(double SpringConstant)
{
    m_SpringConstant = SpringConstant;
}

double Global::DampingConstant() const
{
    return m_DampingConstant;
}

void Global::setDampingConstant(double DampingConstant)
{
    m_DampingConstant = DampingConstant;
}

std::vector<std::string> *Global::MeshSearchPath()
{
    return &m_MeshSearchPath;
}

const std::vector<std::string> *Global::ConstMeshSearchPath() const
{
    return &m_MeshSearchPath;
}

void Global::MeshSearchPathAddToFront(const std::string &meshSearchPath)
{
    if (m_MeshSearchPath[0] != meshSearchPath)
    {
        MeshSearchPathRemove(meshSearchPath);
        m_MeshSearchPath.insert(m_MeshSearchPath.begin(), meshSearchPath); // because there is no push_front in a vector
    }
}

void Global::MeshSearchPathAddToBack(const std::string &meshSearchPath)
{
    if (m_MeshSearchPath[m_MeshSearchPath.size() - 1] != meshSearchPath)
    {
        MeshSearchPathRemove(meshSearchPath);
        m_MeshSearchPath.push_back(meshSearchPath);
    }
}

bool Global::MeshSearchPathRemove(const std::string &meshSearchPath)
{
    bool altered = false;
    for (auto it = m_MeshSearchPath.begin(); it != m_MeshSearchPath.end();)
    {
        if ((*it) == meshSearchPath)
        {
            it = m_MeshSearchPath.erase(it);
            altered = true;
        }
        else
        {
            it++;
        }
    }
    return altered;
}

double Global::LinearDamping() const
{
    return m_LinearDamping;
}

void Global::setLinearDamping(double LinearDamping)
{
    m_LinearDamping = LinearDamping;
}

double Global::AngularDamping() const
{
    return m_AngularDamping;
}

void Global::setAngularDamping(double AngularDamping)
{
    m_AngularDamping = AngularDamping;
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *Global::createFromAttributes()
{
    if (NamedObject::createFromAttributes()) return lastErrorPtr();

    std::string buf;
    std::string buf2;
    double m_DoubleList[3];
    size_t i;

    // gravity
    if (findAttribute("GravityVector", &buf) == nullptr) return lastErrorPtr();
    GSUtil::Double(buf, 3, m_DoubleList);
    m_Gravity.Set(m_DoubleList);

    // set the simulation integration step size
    if (findAttribute("IntegrationStepSize", &buf) == nullptr) return lastErrorPtr();
    m_StepSize = GSUtil::Double(buf);
    if (m_StepSize <= 0.0) { setLastError("Error: GLOBAL IntegrationStepSize must be > 0"s); return lastErrorPtr(); }


    // can specify ERP & CFM; SpringConstant & DampingConstant; SpringConstant & ERP; SpringConstant & CFM; DampingConstant & ERP; DampingConstant & CFM
    if (findAttribute("ERP", &buf) && findAttribute("CFM", &buf2))
    {
        m_ERP = GSUtil::Double(buf);
        m_CFM = GSUtil::Double(buf2);
        if (m_ERP <= 0.0) { setLastError("Error: GLOBAL ERP must be > 0"s); return lastErrorPtr(); }
        if (m_CFM <= 0.0) { setLastError("Error: GLOBAL CFM must be > 0"s); return lastErrorPtr(); }
        m_SpringConstant = m_ERP / (m_CFM * m_StepSize);
        m_DampingConstant = (1.0 - m_ERP) / m_CFM;
    }
    else if (findAttribute("ERP", &buf) && findAttribute("SpringConstant", &buf2))
    {
        m_ERP = GSUtil::Double(buf);
        m_SpringConstant = GSUtil::Double(buf2);
        if (m_ERP <= 0.0) { setLastError("Error: GLOBAL ERP must be > 0"s); return lastErrorPtr(); }
        if (m_SpringConstant <= 0.0) { setLastError("Error: GLOBAL SpringConstant must be > 0"s); return lastErrorPtr(); }
        m_DampingConstant = m_StepSize * (m_SpringConstant / m_ERP - m_SpringConstant);
        m_CFM = 1.0/(m_StepSize * m_SpringConstant + m_DampingConstant);
    }
    else if (findAttribute("ERP", &buf) && findAttribute("DampingConstant", &buf2))
    {
        m_ERP = GSUtil::Double(buf);
        m_DampingConstant = GSUtil::Double(buf2);
        if (m_ERP <= 0.0) { setLastError("Error: GLOBAL ERP must be > 0"s); return lastErrorPtr(); }
        if (m_DampingConstant <= 0.0) { setLastError("Error: GLOBAL DampingConstant must be > 0"s); return lastErrorPtr(); }
        m_SpringConstant = m_DampingConstant / (m_StepSize / m_ERP - m_StepSize);
        m_CFM = 1.0/(m_StepSize * m_SpringConstant + m_DampingConstant);
    }
    else if (findAttribute("CFM", &buf) && findAttribute("DampingConstant", &buf2))
    {
        m_CFM = GSUtil::Double(buf);
        m_DampingConstant = GSUtil::Double(buf2);
        if (m_CFM <= 0.0) { setLastError("Error: GLOBAL CFM must be > 0"s); return lastErrorPtr(); }
        if (m_DampingConstant <= 0.0) { setLastError("Error: GLOBAL DampingConstant must be > 0"s); return lastErrorPtr(); }
        m_SpringConstant = (1.0 / m_CFM - m_DampingConstant) / m_StepSize;
        m_ERP = m_StepSize * m_SpringConstant/(m_StepSize * m_SpringConstant + m_DampingConstant);
    }
    else if (findAttribute("CFM", &buf) && findAttribute("SpringConstant", &buf2))
    {
        m_CFM = GSUtil::Double(buf);
        m_SpringConstant = GSUtil::Double(buf2);
        if (m_CFM <= 0.0) { setLastError("Error: GLOBAL CFM must be > 0"s); return lastErrorPtr(); }
        if (m_SpringConstant <= 0.0) { setLastError("Error: GLOBAL SpringConstant must be > 0"s); return lastErrorPtr(); }
        m_DampingConstant = 1.0 / m_CFM - m_StepSize * m_SpringConstant;
        m_ERP = m_StepSize * m_SpringConstant/(m_StepSize * m_SpringConstant + m_DampingConstant);
    }
    else if (findAttribute("DampingConstant", &buf) && findAttribute("SpringConstant", &buf2))
    {
        m_DampingConstant = GSUtil::Double(buf);
        m_SpringConstant = GSUtil::Double(buf2);
        m_CFM = 1.0/(m_StepSize * m_SpringConstant + m_DampingConstant);
        m_ERP = m_StepSize * m_SpringConstant/(m_StepSize * m_SpringConstant + m_DampingConstant);
    }
    else
    {
        setLastError("Error: GLOBAL needs one of these pairs ERP & CFM; SpringConstant & DampingConstant; SpringConstant & ERP; SpringConstant & CFM; DampingConstant & ERP; DampingConstant & CFM"s);
        return lastErrorPtr();
    }

    if (findAttribute("ContactMaxCorrectingVel", &buf) == nullptr) return lastErrorPtr();
    m_ContactMaxCorrectingVel = GSUtil::Double(buf);
    if (m_ContactMaxCorrectingVel < 0.0) { setLastError("Error: GLOBAL ContactMaxCorrectingVel must be >= 0"s); return lastErrorPtr(); }

    if (findAttribute("ContactSurfaceLayer", &buf) == nullptr) return lastErrorPtr();
    m_ContactSurfaceLayer = GSUtil::Double(buf);
    if (m_ContactSurfaceLayer < 0.0) { setLastError("Error: GLOBAL ContactSurfaceLayer must be >= 0"s); return lastErrorPtr(); }

    // get the stepper required
    // WorldStep, accurate but slow
    // QuickStep, faster but less accurate
    findAttribute("StepType", &buf);
    for (i = 0; i < stepTypeCount; i++)
    {
        if (strcmp(buf.c_str(), stepTypeStrings(i)) == 0)
        {
            m_StepType = StepType(i);
            break;
        }
    }
    if (i > 1)
    {
        setLastError("GLOBAL: Unrecognised StepType=\""s + buf + "\""s);
        return lastErrorPtr();
    }

    // allow internal collisions
    if (findAttribute("AllowInternalCollisions", &buf) == nullptr) return lastErrorPtr();
    m_AllowInternalCollisions = GSUtil::Bool(buf);

    // allow collisions for objects connected by a joint
    findAttribute("AllowConnectedCollisions", &buf);
    if (buf.size()) m_AllowConnectedCollisions = GSUtil::Bool(buf);

    if (findAttribute("LinearDamping"s, &buf)) this->setLinearDamping(GSUtil::Double(buf));
    if (findAttribute("AngularDamping"s, &buf)) this->setAngularDamping(GSUtil::Double(buf));

    // now some run parameters

    if (findAttribute("BMR", &buf) == nullptr) return lastErrorPtr();
    m_BMR = GSUtil::Double(buf);

    if (findAttribute("TimeLimit", &buf) == nullptr) return lastErrorPtr();
    m_TimeLimit = GSUtil::Double(buf);
    if (findAttribute("MechanicalEnergyLimit", &buf) == nullptr) return lastErrorPtr();
    m_MechanicalEnergyLimit = GSUtil::Double(buf);
    if (findAttribute("MetabolicEnergyLimit", &buf) == nullptr) return lastErrorPtr();
    m_MetabolicEnergyLimit = GSUtil::Double(buf);
    if (findAttribute("DistanceTravelledBodyID", &buf) == nullptr) return lastErrorPtr();
    m_DistanceTravelledBodyIDName = buf;
    if (findAttribute("FitnessType", &buf) == nullptr) return lastErrorPtr();
    for (i = 0; i < fitnessTypeCount; i++)
    {
        if (strcmp(buf.c_str(), fitnessTypeStrings(i)) == 0)
        {
            m_FitnessType = FitnessType(i);
            break;
        }
    }
    if (i >= fitnessTypeCount)
    {
        setLastError("Error GLOBAL: Unrecognised FitnessType=\""s + buf + "\""s);
        return lastErrorPtr();
    }

    if (findAttribute("PermittedNumericalErrors", &buf)) m_PermittedNumericalErrors = GSUtil::Int(buf);
    if (findAttribute("NumericalErrorsScore", &buf)) m_NumericalErrorsScore = GSUtil::Double(buf);

    findAttribute("WarehouseFailDistanceAbort", &buf);
    if (buf.size()) m_WarehouseFailDistanceAbort = GSUtil::Double(buf);

    findAttribute("WarehouseUnitIncreaseDistanceThreshold", &buf);
    if (buf.size()) m_WarehouseUnitIncreaseDistanceThreshold = GSUtil::Double(buf);

    findAttribute("WarehouseDecreaseThresholdFactor", &buf);
    if (buf.size()) m_WarehouseDecreaseThresholdFactor = GSUtil::Double(buf);

    findAttribute("CurrentWarehouse", &buf);
    if (buf.size()) m_CurrentWarehouseFile = buf;

    m_MeshSearchPath.clear();
    findAttribute("MeshSearchPath", &buf);
    std::vector<std::string> encodedMeshSearchPath;
    if (buf.size())
    {
        pystring::split(buf, encodedMeshSearchPath, ":"s);
        for (size_t i = 0; i < encodedMeshSearchPath.size(); i++) m_MeshSearchPath.push_back(percentDecode(encodedMeshSearchPath[i]));
    }

    return nullptr;
}

// this function copies the data in the object to an xml_node node that it creates internally.
// doc is used to allocate the memory so deletion should be automatic
void Global::saveToAttributes()
{
    this->setTag("GLOBAL"s);
    this->clearAttributeMap();
    this->appendToAttributes();
}

void Global::appendToAttributes()
{
    NamedObject::appendToAttributes();
    std::string buf;

    setAttribute("AllowConnectedCollisions", *GSUtil::ToString(m_AllowConnectedCollisions, &buf));
    setAttribute("AllowInternalCollisions", *GSUtil::ToString(m_AllowInternalCollisions, &buf));
    setAttribute("BMR", *GSUtil::ToString(m_BMR, &buf));
    setAttribute("CFM", *GSUtil::ToString(m_CFM, &buf));
    setAttribute("ContactMaxCorrectingVel", *GSUtil::ToString(m_ContactMaxCorrectingVel, &buf));
    setAttribute("ContactSurfaceLayer", *GSUtil::ToString(m_ContactSurfaceLayer, &buf));
    setAttribute("DistanceTravelledBodyID", m_DistanceTravelledBodyIDName);
    setAttribute("ERP", *GSUtil::ToString(m_ERP, &buf));
    setAttribute("FitnessType", fitnessTypeStrings(m_FitnessType));
    setAttribute("LinearDamping", *GSUtil::ToString(m_LinearDamping, &buf));
    setAttribute("AngularDamping", *GSUtil::ToString(m_AngularDamping, &buf));
    setAttribute("GravityVector", *GSUtil::ToString(m_Gravity, &buf));
    setAttribute("IntegrationStepSize", *GSUtil::ToString(m_StepSize, &buf));
    setAttribute("MechanicalEnergyLimit", *GSUtil::ToString(m_MechanicalEnergyLimit, &buf));
    setAttribute("MetabolicEnergyLimit", *GSUtil::ToString(m_MetabolicEnergyLimit, &buf));
    setAttribute("StepType", stepTypeStrings(m_StepType));
    setAttribute("TimeLimit", *GSUtil::ToString(m_TimeLimit, &buf));
    setAttribute("NumericalErrorsScore", *GSUtil::ToString(m_NumericalErrorsScore, &buf));
    setAttribute("PermittedNumericalErrors", *GSUtil::ToString(m_PermittedNumericalErrors, &buf));
    setAttribute("CurrentWarehouse", m_CurrentWarehouseFile);
    setAttribute("WarehouseDecreaseThresholdFactor", *GSUtil::ToString(m_WarehouseDecreaseThresholdFactor, &buf));
    setAttribute("WarehouseFailDistanceAbort", *GSUtil::ToString(m_WarehouseFailDistanceAbort, &buf));
    setAttribute("WarehouseUnitIncreaseDistanceThreshold", *GSUtil::ToString(m_WarehouseUnitIncreaseDistanceThreshold, &buf));

    std::vector<std::string> encodedMeshSearchPath;
    for (size_t i = 0; i < m_MeshSearchPath.size(); i++) encodedMeshSearchPath.push_back(percentEncode(m_MeshSearchPath[i], "%:"s));
    setAttribute("MeshSearchPath", pystring::join(":"s, encodedMeshSearchPath));
}

std::string Global::percentEncode(const std::string &input, const std::string &encodeList)
{
    // this routine encodes the characters in encodeList

//    UTF-8 cheat sheet
//    Binary    Hex          Comments
//    0xxxxxxx  0x00..0x7F   Only byte of a 1-byte character encoding
//    10xxxxxx  0x80..0xBF   Continuation bytes (1-3 continuation bytes)
//    110xxxxx  0xC0..0xDF   First byte of a 2-byte character encoding
//    1110xxxx  0xE0..0xEF   First byte of a 3-byte character encoding
//    11110xxx  0xF0..0xF4   First byte of a 4-byte character encoding

    // what this means is that no UTF-8 character looks like a 1-byte colon or 1-byte percent
    std::string output;
    static const std::string digits = "0123456789ABCDEF"s;
    for (size_t i = 0; i < input.size(); i++)
    {
        if (encodeList.find(input[i]) == std::string::npos)
        {
            output.push_back(input[i]);
            continue;
        }
        output.push_back('%');
        uint8_t quotient = uint8_t(input[i]) / uint8_t(16);
        uint8_t remainder = uint8_t(input[i]) % uint8_t(16);
        output.push_back(digits[quotient]);
        output.push_back(digits[remainder]);
    }
    return output;
}

std::string Global::percentDecode(const std::string &input)
{
    // this routine decodes everything that is percent encoded in the string (%XX)
    // if the encoding is poorly formed it assumes that no encoding is present

//    UTF-8 cheat sheet
//    Binary    Hex          Comments
//    0xxxxxxx  0x00..0x7F   Only byte of a 1-byte character encoding
//    10xxxxxx  0x80..0xBF   Continuation bytes (1-3 continuation bytes)
//    110xxxxx  0xC0..0xDF   First byte of a 2-byte character encoding
//    1110xxxx  0xE0..0xEF   First byte of a 3-byte character encoding
//    11110xxx  0xF0..0xF4   First byte of a 4-byte character encoding

    // what this means is that no UTF-8 character looks like a 1-byte percent
    std::string output;
    static const std::map<char, uint8_t> characterMap =
    {
        {'0', 0}, {'1', 1},
        {'2', 2}, {'3', 3},
        {'4', 4}, {'5', 5},
        {'6', 6}, {'7', 7},
        {'8', 8}, {'9', 9},
        {'A', 10}, {'B', 11},
        {'C', 12}, {'D', 13},
        {'E', 14}, {'F', 15},
        {'a', 10}, {'b', 11},
        {'c', 12}, {'d', 13},
        {'e', 14}, {'f', 15}
    };
    for (size_t i = 0; i < input.size(); i++)
    {
        if (input[i] != '%')
        {
            output.push_back(input[i]);
            continue;
        }
        if (i >= input.size() - 2) { output.push_back(input[i]); continue; }
        auto it1 = characterMap.find(input[i + 1]);
        auto it2 = characterMap.find(input[i + 2]);
        if (it1 == characterMap.end() || it2 == characterMap.end()) { output.push_back(input[i]); continue; }
        uint8_t decodedChar = it1->second * uint8_t(16) + it2->second;
        output.push_back(char(decodedChar));
        i += 2;
    }
    return output;
}

int Global::PermittedNumericalErrors() const
{
    return m_PermittedNumericalErrors;
}

void Global::setPermittedNumericalErrors(int PermittedNumericalErrors)
{
    m_PermittedNumericalErrors = PermittedNumericalErrors;
}

double Global::NumericalErrorsScore() const
{
    return m_NumericalErrorsScore;
}

void Global::setNumericalErrorsScore(double NumericalErrorsScore)
{
    m_NumericalErrorsScore = NumericalErrorsScore;
}

Global::FitnessType Global::fitnessType() const
{
    return m_FitnessType;
}

void Global::setFitnessType(FitnessType fitnessType)
{
    m_FitnessType = fitnessType;
}

Global::StepType Global::stepType() const
{
    return m_StepType;
}

void Global::setStepType(Global::StepType stepType)
{
    m_StepType = stepType;
}

bool Global::AllowConnectedCollisions() const
{
    return m_AllowConnectedCollisions;
}

void Global::setAllowConnectedCollisions(bool AllowConnectedCollisions)
{
    m_AllowConnectedCollisions = AllowConnectedCollisions;
}

bool Global::AllowInternalCollisions() const
{
    return m_AllowInternalCollisions;
}

void Global::setAllowInternalCollisions(bool AllowInternalCollisions)
{
    m_AllowInternalCollisions = AllowInternalCollisions;
}

pgd::Vector3 Global::Gravity() const
{
    return m_Gravity;
}

void Global::setGravity(const pgd::Vector3 &gravity)
{
    m_Gravity = gravity;
}

void Global::setGravity(double gravityX, double gravityY, double gravityZ)
{
    m_Gravity.Set(gravityX, gravityY, gravityZ);
}

double Global::BMR() const
{
    return m_BMR;
}

void Global::setBMR(double BMR)
{
    m_BMR = BMR;
}

double Global::CFM() const
{
    return m_CFM;
}

void Global::setCFM(double CFM)
{
    m_CFM = CFM;
}

double Global::ContactMaxCorrectingVel() const
{
    return m_ContactMaxCorrectingVel;
}

void Global::setContactMaxCorrectingVel(double ContactMaxCorrectingVel)
{
    m_ContactMaxCorrectingVel = ContactMaxCorrectingVel;
}

double Global::ContactSurfaceLayer() const
{
    return m_ContactSurfaceLayer;
}

void Global::setContactSurfaceLayer(double ContactSurfaceLayer)
{
    m_ContactSurfaceLayer = ContactSurfaceLayer;
}

double Global::ERP() const
{
    return m_ERP;
}

void Global::setERP(double ERP)
{
    m_ERP = ERP;
}

double Global::MechanicalEnergyLimit() const
{
    return m_MechanicalEnergyLimit;
}

void Global::setMechanicalEnergyLimit(double MechanicalEnergyLimit)
{
    m_MechanicalEnergyLimit = MechanicalEnergyLimit;
}

double Global::MetabolicEnergyLimit() const
{
    return m_MetabolicEnergyLimit;
}

void Global::setMetabolicEnergyLimit(double MetabolicEnergyLimit)
{
    m_MetabolicEnergyLimit = MetabolicEnergyLimit;
}

double Global::StepSize() const
{
    return m_StepSize;
}

void Global::setStepSize(double StepSize)
{
    m_StepSize = StepSize;
}

double Global::TimeLimit() const
{
    return m_TimeLimit;
}

void Global::setTimeLimit(double TimeLimit)
{
    m_TimeLimit = TimeLimit;
}

double Global::WarehouseDecreaseThresholdFactor() const
{
    return m_WarehouseDecreaseThresholdFactor;
}

void Global::setWarehouseDecreaseThresholdFactor(double WarehouseDecreaseThresholdFactor)
{
    m_WarehouseDecreaseThresholdFactor = WarehouseDecreaseThresholdFactor;
}

double Global::WarehouseFailDistanceAbort() const
{
    return m_WarehouseFailDistanceAbort;
}

void Global::setWarehouseFailDistanceAbort(double WarehouseFailDistanceAbort)
{
    m_WarehouseFailDistanceAbort = WarehouseFailDistanceAbort;
}

double Global::WarehouseUnitIncreaseDistanceThreshold() const
{
    return m_WarehouseUnitIncreaseDistanceThreshold;
}

void Global::setWarehouseUnitIncreaseDistanceThreshold(double WarehouseUnitIncreaseDistanceThreshold)
{
    m_WarehouseUnitIncreaseDistanceThreshold = WarehouseUnitIncreaseDistanceThreshold;
}

std::string Global::CurrentWarehouseFile() const
{
    return m_CurrentWarehouseFile;
}

void Global::setCurrentWarehouseFile(const std::string &CurrentWarehouse)
{
    m_CurrentWarehouseFile = CurrentWarehouse;
}

std::string Global::DistanceTravelledBodyIDName() const
{
    return m_DistanceTravelledBodyIDName;
}

void Global::setDistanceTravelledBodyIDName(const std::string &DistanceTravelledBodyIDName)
{
    m_DistanceTravelledBodyIDName = DistanceTravelledBodyIDName;
}



