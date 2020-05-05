/*
 *  Global.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 11/11/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#ifndef GLOBAL_H
#define GLOBAL_H

#include "NamedObject.h"
#include "PGDMath.h"
#include "SmartEnum.h"

#include <string>

using namespace std::string_literals;

class Global: public NamedObject
{
public:
    Global();
//    Global(const Global &global);
    virtual ~Global();

//    Global& operator=(const Global&);

    SMART_ENUM(StepType, stepTypeStrings, stepTypeCount, World, Quick);
    SMART_ENUM(FitnessType, fitnessTypeStrings, fitnessTypeCount, DistanceTravelled, KinematicMatch, KinematicMatchMiniMax, KinematicMatchContinuous, KinematicMatchContinuousMiniMax);

//    enum StepType
//    {
//        WorldStep = 0,
//        QuickStep = 1,
//        StepTypeMax = QuickStep
//    };
//    const char *StepTypeNames[StepType::StepTypeMax + 1] = {"World", "Quick"};

//    enum FitnessType
//    {
//        DistanceTravelled = 0,
//        KinematicMatch = 1,
//        KinematicMatchMiniMax = 2,
//        KinematicMatchContinuous = 3,
//        KinematicMatchContinuousMiniMax = 4,
//        ClosestWarehouse = 5,
//        FitnessTypeMax = ClosestWarehouse
//    };
//    const char *FitnessTypeNames[FitnessTypeMax + 1] = {"DistanceTravelled", "KinematicMatch", "KinematicMatchMiniMax", "KinematicMatchContinuous", "KinematicMatchContinuousMiniMax", "ClosestWarehouse"};

    virtual std::string *createFromAttributes();
    virtual void saveToAttributes();
    virtual void appendToAttributes();

    FitnessType fitnessType() const;
    void setFitnessType(FitnessType fitnessType);

    StepType stepType() const;
    void setStepType(StepType stepType);

    bool AllowConnectedCollisions() const;
    void setAllowConnectedCollisions(bool AllowConnectedCollisions);

    bool AllowInternalCollisions() const;
    void setAllowInternalCollisions(bool AllowInternalCollisions);

    pgd::Vector Gravity() const;
    void setGravity(const pgd::Vector &gravity);
    void setGravity(double gravityX, double gravityY, double gravityZ);

    double BMR() const;
    void setBMR(double BMR);

    double CFM() const;
    void setCFM(double CFM);

    double ContactMaxCorrectingVel() const;
    void setContactMaxCorrectingVel(double ContactMaxCorrectingVel);

    double ContactSurfaceLayer() const;
    void setContactSurfaceLayer(double ContactSurfaceLayer);

    double ERP() const;
    void setERP(double ERP);

    double MechanicalEnergyLimit() const;
    void setMechanicalEnergyLimit(double MechanicalEnergyLimit);

    double MetabolicEnergyLimit() const;
    void setMetabolicEnergyLimit(double MetabolicEnergyLimit);

    double StepSize() const;
    void setStepSize(double StepSize);

    double TimeLimit() const;
    void setTimeLimit(double TimeLimit);

    double WarehouseDecreaseThresholdFactor() const;
    void setWarehouseDecreaseThresholdFactor(double WarehouseDecreaseThresholdFactor);

    double WarehouseFailDistanceAbort() const;
    void setWarehouseFailDistanceAbort(double WarehouseFailDistanceAbort);

    double WarehouseUnitIncreaseDistanceThreshold() const;
    void setWarehouseUnitIncreaseDistanceThreshold(double WarehouseUnitIncreaseDistanceThreshold);

    std::string CurrentWarehouseFile() const;
    void setCurrentWarehouseFile(const std::string &CurrentWarehouseFile);

    std::string DistanceTravelledBodyIDName() const;
    void setDistanceTravelledBodyIDName(const std::string &DistanceTravelledBodyIDName);

    double SpringConstant() const;
    void setSpringConstant(double SpringConstant);

    double DampingConstant() const;
    void setDampingConstant(double DampingConstant);

    std::string MeshSearchPath() const;
    void setMeshSearchPath(const std::string &MeshSearchPath);

    double LinearDamping() const;
    void setLinearDamping(double LinearDamping);

    double AngularDamping() const;
    void setAngularDamping(double AngularDamping);

private:
    FitnessType m_FitnessType = KinematicMatch;
    StepType m_StepType = World;
    bool m_AllowConnectedCollisions = false;
    bool m_AllowInternalCollisions = false;
    pgd::Vector m_Gravity = {0, 0, -9.81};
    double m_BMR = 0;
    double m_CFM = 1e-10;
    double m_ContactMaxCorrectingVel = 100;
    double m_ContactSurfaceLayer = 0.001;
    double m_DampingConstant = 0;
    double m_ERP = 0.2;
    double m_MechanicalEnergyLimit = 0;
    double m_MetabolicEnergyLimit = 0;
    double m_SpringConstant = 0;
    double m_StepSize = 1e-4;
    double m_TimeLimit = 10;
    double m_WarehouseDecreaseThresholdFactor = 0.5;
    double m_WarehouseFailDistanceAbort = 0.5;
    double m_WarehouseUnitIncreaseDistanceThreshold = 0.5;
    double m_LinearDamping = 0;
    double m_AngularDamping = 0;
    std::string m_CurrentWarehouseFile;
    std::string m_DistanceTravelledBodyIDName;
    std::string m_OutputModelStateFile;
    std::string m_MeshSearchPath = "."s;
};

#endif // GLOBAL_H
