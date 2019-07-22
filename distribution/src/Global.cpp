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
#ifdef USE_QT
#include "Preferences.h"
#endif



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

// assignment operator function
//Global& Global::operator=(const Global &global)
//{
//    if(this != &global)
//    {
//        m_Fitness = global.m_Fitness;
//        m_Step = global.m_Step;
//        m_AllowConnectedCollisions = global.m_AllowConnectedCollisions;
//        m_AllowInternalCollisions = global.m_AllowInternalCollisions;
//        m_Gravity = global.m_Gravity;
//        m_BMR = global.m_BMR;
//        m_CFM = global.m_CFM;
//        m_ContactMaxCorrectingVel = global.m_ContactMaxCorrectingVel;
//        m_ContactSurfaceLayer = global.m_ContactSurfaceLayer;
//        m_DampingConstant = global.m_DampingConstant;
//        m_ERP = global.m_ERP;
//        m_MechanicalEnergyLimit = global.m_MechanicalEnergyLimit;
//        m_MetabolicEnergyLimit = global.m_MetabolicEnergyLimit;
//        m_SpringConstant = global.m_SpringConstant;
//        m_StepSize = global.m_StepSize;
//        m_TimeLimit = global.m_TimeLimit;
//        m_WarehouseDecreaseThresholdFactor = global.m_WarehouseDecreaseThresholdFactor;
//        m_WarehouseFailDistanceAbort = global.m_WarehouseFailDistanceAbort;
//        m_WarehouseUnitIncreaseDistanceThreshold = global.m_WarehouseUnitIncreaseDistanceThreshold;
//        m_CurrentWarehouseFile = global.m_CurrentWarehouseFile;
//        m_DistanceTravelledBodyIDName = global.m_DistanceTravelledBodyIDName;
//        m_OutputModelStateFile = global.m_OutputModelStateFile;
//        m_MeshSearchPath = global.m_MeshSearchPath;
//    }
//    return *this;
//}

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

std::string Global::MeshSearchPath() const
{
    return m_MeshSearchPath;
}

void Global::setMeshSearchPath(const std::string &MeshSearchPath)
{
    m_MeshSearchPath = MeshSearchPath;
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *Global::CreateFromAttributes()
{
    if (NamedObject::CreateFromAttributes()) return lastErrorPtr();

    std::string buf;
    std::string buf2;
    double m_DoubleList[3];
    size_t i;

    // gravity
    if (GetAttribute("GravityVector", &buf) == nullptr) return lastErrorPtr();
    GSUtil::Double(buf, 3, m_DoubleList);
    m_Gravity.Set(m_DoubleList);

    // set the simulation integration step size
    if (GetAttribute("IntegrationStepSize", &buf) == nullptr) return lastErrorPtr();
    m_StepSize = GSUtil::Double(buf);

    // can specify ERP & CFM; SpringConstant & DampingConstant; SpringConstant & ERP; SpringConstant & CFM; DampingConstant & ERP; DampingConstant & CFM
    if (GetAttribute("ERP", &buf) && GetAttribute("CFM", &buf2))
    {
        m_ERP = GSUtil::Double(buf);
        m_CFM = GSUtil::Double(buf2);
        m_SpringConstant = m_ERP / (m_CFM * m_StepSize);
        m_DampingConstant = (1.0 - m_ERP) / m_CFM;
    }
    else if (GetAttribute("ERP", &buf) && GetAttribute("SpringConstant", &buf2))
    {
        m_ERP = GSUtil::Double(buf);
        m_SpringConstant = GSUtil::Double(buf2);
        m_DampingConstant = m_StepSize * (m_SpringConstant / m_ERP - m_SpringConstant);
        m_CFM = 1.0/(m_StepSize * m_SpringConstant + m_DampingConstant);
    }
    else if (GetAttribute("ERP", &buf) && GetAttribute("DampingConstant", &buf2))
    {
        m_ERP = GSUtil::Double(buf);
        m_DampingConstant = GSUtil::Double(buf2);
        m_SpringConstant = m_DampingConstant / (m_StepSize / m_ERP - m_StepSize);
        m_CFM = 1.0/(m_StepSize * m_SpringConstant + m_DampingConstant);
    }
    else if (GetAttribute("CFM", &buf) && GetAttribute("DampingConstant", &buf2))
    {
        m_CFM = GSUtil::Double(buf);
        m_DampingConstant = GSUtil::Double(buf2);
        m_SpringConstant = (1.0 / m_CFM - m_DampingConstant) / m_StepSize;
        m_ERP = m_StepSize * m_SpringConstant/(m_StepSize * m_SpringConstant + m_DampingConstant);
    }
    else if (GetAttribute("CFM", &buf) && GetAttribute("SpringConstant", &buf2))
    {
        m_CFM = GSUtil::Double(buf);
        m_SpringConstant = GSUtil::Double(buf2);
        m_DampingConstant = 1.0 / m_CFM - m_StepSize * m_SpringConstant;
        m_ERP = m_StepSize * m_SpringConstant/(m_StepSize * m_SpringConstant + m_DampingConstant);
    }
    else if (GetAttribute("DampingConstant", &buf) && GetAttribute("SpringConstant", &buf2))
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

    if (GetAttribute("ContactMaxCorrectingVel", &buf) == nullptr) return lastErrorPtr();
    m_ContactMaxCorrectingVel = GSUtil::Double(buf);

    if (GetAttribute("ContactSurfaceLayer", &buf) == nullptr) return lastErrorPtr();
    m_ContactSurfaceLayer = GSUtil::Double(buf);


    // get the stepper required
    // WorldStep, accurate but slow
    // QuickStep, faster but less accurate
    GetAttribute("StepType", &buf);
    for (i = 0; i < 2; i++)
    {
        if (strcmp(buf.c_str(), StepTypeNames[i]) == 0)
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
    if (GetAttribute("AllowInternalCollisions", &buf) == nullptr) return lastErrorPtr();
    m_AllowInternalCollisions = GSUtil::Bool(buf);

    // allow collisions for objects connected by a joint
    GetAttribute("AllowConnectedCollisions", &buf);
    if (buf.size()) m_AllowConnectedCollisions = GSUtil::Bool(buf);

    // now some run parameters

    if (GetAttribute("BMR", &buf) == nullptr) return lastErrorPtr();
    m_BMR = GSUtil::Double(buf);

    if (GetAttribute("TimeLimit", &buf) == nullptr) return lastErrorPtr();
    m_TimeLimit = GSUtil::Double(buf);
    if (GetAttribute("MechanicalEnergyLimit", &buf) == nullptr) return lastErrorPtr();
    m_MechanicalEnergyLimit = GSUtil::Double(buf);
    if (GetAttribute("MetabolicEnergyLimit", &buf) == nullptr) return lastErrorPtr();
    m_MetabolicEnergyLimit = GSUtil::Double(buf);
    if (GetAttribute("DistanceTravelledBodyID", &buf) == nullptr) return lastErrorPtr();
    m_DistanceTravelledBodyIDName = buf;
    if (GetAttribute("FitnessType", &buf) == nullptr) return lastErrorPtr();
    for (i = 0; i < 6; i++)
    {

        if (strcmp(buf.c_str(), FitnessTypeNames[i]) == 0)
        {
            m_FitnessType = FitnessType(i);
            break;
        }
    }
    if (i > 5)
    {
        setLastError("Error GLOBAL: Unrecognised FitnessType=\""s + buf + "\""s);
        return lastErrorPtr();
    }

    if (m_FitnessType == DistanceTravelled && m_DistanceTravelledBodyIDName.size() == 0)
    {
        setLastError("Error GLOBAL: must specify DistanceTravelledBodyIDName with FitnessType=\"DistanceTravelled\""s);
        return lastErrorPtr();
    }

    GetAttribute("MeshSearchPath", &buf);
    if (buf.size()) m_MeshSearchPath = buf;

    GetAttribute("WarehouseFailDistanceAbort", &buf);
    if (buf.size()) m_WarehouseFailDistanceAbort = GSUtil::Double(buf);

    GetAttribute("WarehouseUnitIncreaseDistanceThreshold", &buf);
    if (buf.size()) m_WarehouseUnitIncreaseDistanceThreshold = GSUtil::Double(buf);

    GetAttribute("WarehouseDecreaseThresholdFactor", &buf);
    if (buf.size()) m_WarehouseDecreaseThresholdFactor = GSUtil::Double(buf);

    GetAttribute("CurrentWarehouse", &buf);
    if (buf.size()) m_CurrentWarehouseFile = buf;

    return nullptr;
}

// this function copies the data in the object to an xml_node node that it creates internally.
// doc is used to allocate the memory so deletion should be automatic
void Global::SaveToAttributes()
{
    this->setTag("GLOBAL"s);
    this->AppendToAttributes();
}

void Global::AppendToAttributes()
{
    NamedObject::AppendToAttributes();
    std::string buf;

    // generated using regexp r'.*m_(.*);' to r'setAttribute("\1", *GSUtil::ToString(m_\1, &buf));'
    setAttribute("AllowConnectedCollisions", *GSUtil::ToString(m_AllowConnectedCollisions, &buf));
    setAttribute("AllowInternalCollisions", *GSUtil::ToString(m_AllowInternalCollisions, &buf));
    setAttribute("BMR", *GSUtil::ToString(m_BMR, &buf));
    setAttribute("CFM", *GSUtil::ToString(m_CFM, &buf));
    setAttribute("ContactMaxCorrectingVel", *GSUtil::ToString(m_ContactMaxCorrectingVel, &buf));
    setAttribute("ContactSurfaceLayer", *GSUtil::ToString(m_ContactSurfaceLayer, &buf));
    setAttribute("DistanceTravelledBodyID", m_DistanceTravelledBodyIDName);
    setAttribute("ERP", *GSUtil::ToString(m_ERP, &buf));
    setAttribute("FitnessType", FitnessTypeNames[int(m_FitnessType)]);
    setAttribute("GravityVector", *GSUtil::ToString(m_Gravity, &buf));
    setAttribute("IntegrationStepSize", *GSUtil::ToString(m_StepSize, &buf));
    setAttribute("MechanicalEnergyLimit", *GSUtil::ToString(m_MechanicalEnergyLimit, &buf));
    setAttribute("MetabolicEnergyLimit", *GSUtil::ToString(m_MetabolicEnergyLimit, &buf));
    setAttribute("StepType", StepTypeNames[int(m_StepType)]);
    setAttribute("TimeLimit", *GSUtil::ToString(m_TimeLimit, &buf));
    setAttribute("MeshSearchPath", m_MeshSearchPath);
    setAttribute("CurrentWarehouse", m_CurrentWarehouseFile);
    setAttribute("WarehouseDecreaseThresholdFactor", *GSUtil::ToString(m_WarehouseDecreaseThresholdFactor, &buf));
    setAttribute("WarehouseFailDistanceAbort", *GSUtil::ToString(m_WarehouseFailDistanceAbort, &buf));
    setAttribute("WarehouseUnitIncreaseDistanceThreshold", *GSUtil::ToString(m_WarehouseUnitIncreaseDistanceThreshold, &buf));
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

pgd::Vector Global::Gravity() const
{
    return m_Gravity;
}

void Global::setGravity(const pgd::Vector &gravity)
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



