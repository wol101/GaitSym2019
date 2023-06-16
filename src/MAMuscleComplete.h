/*
 *  MAMuscleComplete.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 03/03/2007.
 *  Copyright 2006 Bill Sellers. All rights reserved.
 *
 */

// MAMuscleComplete - implementation of an Minetti & Alexander style
// muscle based on the StrapForce class

// Minetti & Alexander, J. theor Biol (1997) 186, 467-476

// Added extra terms to allow a serial and parallel spring element
// plus length tension stuff too


#ifndef MAMuscleComplete_h
#define MAMuscleComplete_h

#include "Muscle.h"
#include "SmartEnum.h"

class Strap;
class MAMuscle;
class DampedSpringMuscle;
class SimpleStrap;
class Filter;

class MAMuscleComplete : public Muscle
{
public:
    SMART_ENUM(StrainModel, StrainModelStrings, StrainModelCount, linear, square);
//    enum StrainModel { linear, square };

    // this struct contains all the paramers required for the CalculateForceError function
    struct CalculateForceErrorParams
    {
        // fixed input parameters
        double spe = 0; // slack length parallel element (m)
        double epe = 0; // elastic constant parallel element (N/m)
        double dpe = 0; // damping constant parallel element (N/m)
        StrainModel smpe = StrainModel::linear; // strain model for parallel element
        double sse = 0; // slack length serial element (m)
        double ese = 0; // elastic constant serial element (N/m)
        double dse = 0; // damping constant serial element (N/m)
        StrainModel smse = StrainModel::linear; // strain model for serial element
        double k = 0; // shape constant
        double vmax = 0; // maximum shortening velocity (m/s)
        double fmax = 0; // maximum isometric force (N)
        double width = 0; // relative width of length/tension peak

        // variable input parameters
        double alpha = 0; // proportion of muscle activated
        double timeIncrement = 0; // inegration step size for simulation (s)
        double len = 0; // length of the whole element (m)
        double v = 0; // contraction velocity of the whole element (m/s)
        double lastlpe = -1; // last calculated lpe value (m)

        // output parameters (set to dummy values)
        double fce = -2; // contractile force (N)
        double lpe = -2; // contractile and parallel length (m)
        double fpe = -2; // parallel element force (N)
        double lse = -2; // serial length (m)
        double fse = -2; // serial element force (N)
        double vce = -2; // contractile element velocity (m/s)
        double vse = -2; // serial element velocity (m/s)
        double targetFce = -2; // fce calculated from elastic elements (N)
        double f0 = -2; // length corrected fmax (N)
        double err = -2; // error term in lpe (m)
    };

    MAMuscleComplete();
    virtual ~MAMuscleComplete();

    void SetSerialElasticProperties(double serialStrainAtFmax, double serialStrainRateAtFmax, double tendonLength, MAMuscleComplete::StrainModel serialStrainModel);
    void SetParallelElasticProperties(double parallelStrainAtFmax, double parallelStrainRateAtFmax, double parallelElementLength, MAMuscleComplete::StrainModel parallelStrainModel);
    void SetMuscleProperties(double vMax, double Fmax, double K, double Width);
    void SetActivationKinetics(bool activationKinetics, double akFastTwitchProportion, double akTActivationA, double akTActivationB, double akTDeactivationA, double akTDeactivationB);
    void SetInitialFibreLength(double initialFibreLength) { m_Params.lastlpe = initialFibreLength; }
    void SetActivationRate(double activationRate) { m_ActivationRate = activationRate; }
    void SetStartActivation(double startActivation) { m_Params.alpha = startActivation; }
    void SetMinimumActivation(double minimumActivation) { m_MinimumActivation = minimumActivation; }

    virtual double GetMetabolicPower();

    virtual void SetActivation();
    virtual double GetActivation() { return m_Params.alpha; }
    virtual double GetElasticEnergy() { return GetESE(); }

    double GetStimulation() { return m_Stim; }

    double GetFCE() { return m_Params.fce; } // contractile force (N)
    double GetLPE() { return m_Params.lpe; } // contractile and parallel length (m)
    double GetFPE() { return m_Params.fpe; } // parallel element force (N)
    double GetLSE() { return m_Params.lse; } // serial length (m)
    double GetFSE() { return m_Params.fse; } // serial element force (N)
    double GetVCE() { return m_Params.vce; } // contractile element velocity (m/s)
    double GetVPE() { return m_Params.vce; } // parallel element velocity (m/s)
    double GetVSE() { return m_Params.vse; } // serial element velocity (m/s)
    double GetESE(); // energy serial element
    double GetEPE(); // energy parallel element
    double GetPSE() { return GetVSE() * -m_Params.fse; } // power serial element
    double GetPPE() { return GetVPE() * -m_Params.fpe; } // power parallel element
    double GetPCE() { return GetVCE() * -m_Params.fce; } // power contractile element
    double GetSSE() { return m_Params.sse; }
    double GetSPE() { return m_Params.spe; }

    virtual std::string dumpToString();
    virtual void LateInitialisation();

    virtual std::string *createFromAttributes();
    virtual void appendToAttributes();




    double forcePerUnitArea() const;
    void setForcePerUnitArea(double forcePerUnitArea);

    double vMaxFactor() const;
    void setVMaxFactor(double vMaxFactor);

    double pca() const;
    void setPca(double pca);

    double fibreLength() const;
    void setFibreLength(double fibreLength);

    double activationK() const;
    void setActivationK(double activationK);

    double width() const;
    void setWidth(double width);

    double tendonLength() const;
    void setTendonLength(double tendonLength);

    double akFastTwitchProportion() const;
    void setAkFastTwitchProportion(double akFastTwitchProportion);

    double akTActivationA() const;
    void setAkTActivationA(double akTActivationA);

    double akTActivationB() const;
    void setAkTActivationB(double akTActivationB);

    double akTDeactivationA() const;
    void setAkTDeactivationA(double akTDeactivationA);

    double akTDeactivationB() const;
    void setAkTDeactivationB(double akTDeactivationB);

    double initialFibreLength() const;
    void setInitialFibreLength(double initialFibreLength);

    double startActivation() const;
    void setStartActivation(double startActivation);

    StrainModel serialStrainModel() const;
    void setSerialStrainModel(const StrainModel &serialStrainModel);
    void setSerialStrainModel(const std::string &serialStrainModel);

    StrainModel parallelStrainModel() const;
    void setParallelStrainModel(const StrainModel &parallelStrainModel);
    void setParallelStrainModel(const std::string &parallelStrainModel);

private:

    double m_Stim = 0;
    bool m_ActivationKinetics = false;
    double m_ft = 0;
    double m_tact = 0;
    double m_tdeact = 0;
    double m_ActivationRate = 0;
    double m_serialStrainAtFmax = 0;
    double m_serialStrainRateAtFmax = 0;
    double m_parallelStrainAtFmax = 0;
    double m_parallelStrainRateAtFmax = 0;
    double m_MinimumActivation = 0.001; // arbitrary value so that we avoid some numerical issues

    CalculateForceErrorParams m_Params;
    double m_Tolerance = 1e-8; // solution tolerance (m) - small because the serial tendons are quite stiff

    // these values are only used for loading and saving
    StrainModel m_serialStrainModel = StrainModel::linear;
    StrainModel m_parallelStrainModel = StrainModel::linear;
    double m_forcePerUnitArea = 0;
    double m_vMaxFactor = 0;
    double m_pca = 0;
    double m_fibreLength = 0;
    double m_activationK = 0;
    double m_width = 0;
    double m_tendonLength = 0;
    double m_akFastTwitchProportion = 0;
    double m_akTActivationA = 0;
    double m_akTActivationB = 0;
    double m_akTDeactivationA = 0;
    double m_akTDeactivationB = 0;
    double m_initialFibreLength = 0;
    double m_startActivation = 0;
};






#endif // MAMuscleComplete_h
