/*
 *  FixedJoint.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 20/09/2008.
 *  Copyright 2008 Bill Sellers. All rights reserved.
 *
 */

#ifndef FixedJoint_h
#define FixedJoint_h

#include "Joint.h"
#include "SmartEnum.h"
#include "PGDMath.h"

class ButterworthFilter;
class MovingAverage;
class Filter;

class FixedJoint: public Joint
{
    public:

    FixedJoint(dWorldID worldID);
    virtual ~FixedJoint();


    SMART_ENUM(StressCalculationType, stressCalculationTypeStrings, stressCalculationTypeCount, none, beam, spring);
//    enum StressCalculationType { none = 0, beam, spring };
    SMART_ENUM(LowPassType, lowPassTypeStrings, lowPassTypeCount, NoLowPass, MovingAverageLowPass, Butterworth2ndOrderLowPass);
//    enum LowPassType { NoLowPass = 0, MovingAverageLowPass, Butterworth2ndOrderLowPass };



    void SetFixed();

    void SetCrossSection(unsigned char *stiffness, int nx, int ny, double dx, double dy);
    void SetStressOrigin(double x, double y, double z);
    void SetStressOrientation(double q0, double q1, double q2, double q3);

    void SetStressCalculationType(StressCalculationType type) { m_stressCalculationType = type; }

    pgd::Vector GetStressOrigin() { return m_StressOrigin; }
    pgd::Quaternion GetStressOrientation() { return m_StressOrientation; }

    double GetMaxStress() { return m_maxStress; }
    double GetMinStress() { return m_minStress; }

    void SetStressLimit(double stressLimit) { m_stressLimit = stressLimit; }
    bool CheckStressAbort();

    void SetLowPassType(LowPassType lowPassType) { m_lowPassType = lowPassType; }
    LowPassType GetLowPassType() { return m_lowPassType; }
    void SetWindow(int window);
    void SetCutoffFrequency(double cutoffFrequency);
    double GetLowPassMinStress() { return m_lowPassMinStress; }
    double GetLowPassMaxStress() { return m_lowPassMaxStress; }

    double *GetStress() { return m_stress; }

    virtual void Update();
    virtual std::string dump();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();

    double maxStress() const;

private:

    void CalculateStress();

    // these are used for the stress/strain calculations
    unsigned char *m_stiffness = nullptr;
    double *m_stress = nullptr;
    double *m_xDistances = nullptr;
    double *m_yDistances = nullptr;
    int m_nx = 0;
    int m_ny = 0;
    int m_nActivePixels = 0;
    double m_dx = 0;
    double m_dy = 0;
    double m_Ix = 0;
    double m_Iy = 0;
    double m_Ixy = 0;
    double m_area = 0;
    double m_xOrigin = 0;
    double m_yOrigin = 0;
    double m_minStress = 0;
    double m_maxStress = 0;
    StressCalculationType m_stressCalculationType = StressCalculationType::none;

    pgd::Vector m_StressOrigin;
    pgd::Quaternion m_StressOrientation;
    pgd::Vector m_torqueStressCoords;
    pgd::Vector m_forceStressCoords;
    pgd::Vector m_torqueAxis;
    double m_torqueScalar = 0;

    double m_stressLimit = -1;
    Filter **m_filteredStress = nullptr;
    double m_lowPassMinStress = 0;
    double m_lowPassMaxStress = 0;
    LowPassType m_lowPassType = LowPassType::NoLowPass;
    double m_cutoffFrequency = 0;
    int m_window = 0;

    pgd::Vector *m_vectorList = nullptr;

};


#endif

