/*
 *  MAMuscleComplete.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 03/03/2007.
 *  Copyright 2006 Bill Sellers. All rights reserved.
 *
 */

// MAMuscleComplete - implementation of an Minetti & Alexander style
// muscle based on the StrapForce class

// Minetti & Alexander, J. theor Biol (1997) 186, 467-476

// Added extra terms to allow damped spring element
// plus length tension stuff too

#include "Strap.h"
#include "MAMuscleComplete.h"
#include "Simulation.h"
#include "GSUtil.h"
#include "TwoPointStrap.h"
#include "NPointStrap.h"
#include "CylinderWrapStrap.h"
#include "TwoCylinderWrapStrap.h"

#include "ode/ode.h"

#include <sstream>
#include <cmath>

using namespace std::string_literals;

static double CalculateForceError (double lce, void *params);

// constructor

MAMuscleComplete::MAMuscleComplete(): Muscle()
{
}

// destructor
MAMuscleComplete::~MAMuscleComplete()
{
}

// set the muscle elastic properties
void MAMuscleComplete::SetSerialElasticProperties(double serialStrainAtFmax, double serialStrainRateAtFmax, double tendonLength, MAMuscleComplete::StrainModel serialStrainModel)
{
    double serialDampingConstant, serialElasticConstant;
    m_serialStrainAtFmax = serialStrainAtFmax;
    m_serialStrainRateAtFmax = serialStrainRateAtFmax;
    m_Params.smse = serialStrainModel; // strain model for serial element
    m_Params.sse = tendonLength; // slack length serial element (m)

    if (tendonLength != -1) // if tendonLength == -1 this has to be done later once the tendonLength can be calculated
    {
        switch (serialStrainModel)
        {
        case MAMuscleComplete::linear:
            serialElasticConstant = m_Params.fmax/(serialStrainAtFmax * tendonLength);
            break;
        case MAMuscleComplete::square:
            serialElasticConstant = m_Params.fmax/(SQUARE(serialStrainAtFmax * tendonLength));
            break;
        }
        if (serialStrainRateAtFmax == 0) serialDampingConstant = 0;
        else serialDampingConstant = m_Params.fmax/(serialStrainRateAtFmax * tendonLength);
        m_Params.ese = serialElasticConstant; // elastic constant serial element (N/m)
        m_Params.dse = serialDampingConstant; // damping constant serial element (Ns/m)
    }
}

void MAMuscleComplete::SetParallelElasticProperties(double parallelStrainAtFmax, double parallelStrainRateAtFmax, double parallelElementLength, MAMuscleComplete::StrainModel parallelStrainModel)
{
    double parallelDampingConstant, parallelElasticConstant;

    m_parallelStrainAtFmax = parallelStrainAtFmax;
    m_parallelStrainRateAtFmax = parallelStrainRateAtFmax;

    if (parallelStrainAtFmax == 0) parallelElasticConstant = 0;
    else
    {
        switch (parallelStrainModel)
        {
        case MAMuscleComplete::linear:
            parallelElasticConstant = m_Params.fmax/(parallelStrainAtFmax * parallelElementLength);
            break;
        case MAMuscleComplete::square:
            parallelElasticConstant = m_Params.fmax/(SQUARE(parallelStrainAtFmax * parallelElementLength));
            break;
        }
    }
    if (parallelStrainRateAtFmax == 0) parallelDampingConstant = 0;
    else parallelDampingConstant = m_Params.fmax/(parallelStrainRateAtFmax * parallelElementLength);

    m_Params.spe = parallelElementLength; // slack length parallel element (m)
    m_Params.epe = parallelElasticConstant; // elastic constant parallel element (N/m)
    m_Params.dpe = parallelDampingConstant; // damping constant serial element (Ns/m)
    m_Params.smpe = parallelStrainModel; // strain model for serial element
}

// set the muscle contractile properties
void MAMuscleComplete::SetMuscleProperties(double vMax, double Fmax, double K, double Width)
{
    m_Params.k = K; // shape constant
    m_Params.vmax = vMax; // maximum shortening velocity (m/s)
    m_Params.fmax = Fmax; // isometric force
    m_Params.width = Width; // relative width of length/tension peak
}

// set the activation kinetics
void MAMuscleComplete::SetActivationKinetics(bool activationKinetics,
                                             double akFastTwitchProportion,
                                             double akTActivationA, double akTActivationB,
                                             double akTDeactivationA, double akTDeactivationB)
{
    m_ActivationKinetics = activationKinetics;

    // original code
    //double ft = 0.5; // arbitrary set activation kinetics as 50% fast twitch
    //double tact = 80e-3 - 0.47e-3 * ft; // Umberger et al 2003 eq 4
    //double tdeact = 90e-3 - 0.56e-3 * ft; // Umberger et al 2003 eq 4

    m_ft = akFastTwitchProportion;
    m_tact = akTActivationA + akTActivationB * m_ft;
    m_tdeact = akTDeactivationA + akTDeactivationB * m_ft;
}

// do any intialisation that relies on the strap being set up properly

void MAMuscleComplete::LateInitialisation()
{
    Muscle::LateInitialisation();
    m_Params.len = GetStrap()->GetLength();

    // lastlpe perhaps not set to anything useful

    // handle sse == -1 which means I need to calculate this value internally and reset the derived values
    if (m_Params.sse == -1)
    {
        m_Params.sse = m_Params.len - m_Params.spe; // set the slack serial length

        double serialDampingConstant, serialElasticConstant;
        switch (m_Params.smse)
        {
        case MAMuscleComplete::linear:
            serialElasticConstant = m_Params.fmax/(m_serialStrainAtFmax * m_Params.sse);
            break;
        case MAMuscleComplete::square:
            serialElasticConstant = m_Params.fmax/(SQUARE(m_serialStrainAtFmax * m_Params.sse));
            break;
        }
        if (m_serialStrainRateAtFmax == 0) serialDampingConstant = 0;
        else serialDampingConstant = m_Params.fmax/(m_serialStrainRateAtFmax * m_Params.sse);
        m_Params.ese = serialElasticConstant; // elastic constant serial element (N/m)
        m_Params.dse = serialDampingConstant; // damping constant serial element (Ns/m)
    }

    double minlpe = m_Params.spe - (m_Params.spe * m_Params.width / 2);
    if (minlpe < 0) minlpe = 0;
    double maxlpe = m_Params.len - m_Params.sse; // this would be right with no damping
    if (maxlpe < minlpe) maxlpe = minlpe;
    if (m_Params.lastlpe == -1)
    {
        // just setting lastlpe to something sensible should be enough
        m_Params.lastlpe = (maxlpe + minlpe) / 2;
    }
    m_Params.lpe = m_Params.lastlpe; // needed so that we get a sensible t=0 OutputProgramState

    return;

}

// set the proportion of muscle fibres that are active
// calculates the tension in the strap

void MAMuscleComplete::SetActivation()
{
    double activation = dataSum();
    // set variable input parameters

    if (activation < m_MinimumActivation) activation = m_MinimumActivation;
    else if (activation > 1) activation = 1;
    m_Stim = activation;

    m_Params.timeIncrement = simulation()->GetTimeIncrement();
    if (m_ActivationKinetics || m_ActivationRate != 0)
    {
        if (m_Params.alpha == -1) // special case for first run through if I just want disable rate
        {
            m_Params.alpha = m_Stim;
        }
        else
        {
            if (m_ActivationKinetics)
            {
                // using activation kinetics from UGM model
                double t2 = 1 / m_tdeact;
                double t1 = 1 / m_tact - t2;
                // Nagano & Gerritsen 2001 A2
                double qdot = (m_Stim - m_Params.alpha) * (t1 * m_Stim + t2);
                m_Params.alpha += qdot * m_Params.timeIncrement;
                // I think I should allow this to fall - it won't make any difference and it maintains
                // continuity in the differentials
                // if (m_Params.alpha < 0.001) m_Params.alpha = 0.001; // m_Act never drops to zero in practice
            }
            else // this is if the activation rate is limited
            {
                double delAct = m_ActivationRate * m_Params.timeIncrement;
                if (m_Stim > m_Params.alpha)
                {
                    m_Params.alpha += delAct;
                    if (m_Params.alpha > m_Stim) m_Params.alpha = m_Stim;
                }
                else if (m_Stim < m_Params.alpha)
                {
                    m_Params.alpha -= delAct;
                    if (m_Params.alpha < m_Stim) m_Params.alpha = m_Stim;
                }
            }
        }
    }
    else
    {
        m_Params.alpha = m_Stim;
    }

    m_Params.len = GetStrap()->GetLength();
    m_Params.v = GetStrap()->GetVelocity();

    double minlpe = m_Params.spe - (m_Params.spe * m_Params.width / 2);
    if (minlpe < 0) minlpe = 0;
    // double maxlpe = m_Params.len - m_Params.sse; // this would be right with no damping
    // if (maxlpe < minlpe) maxlpe = minlpe;

    // now calculate output parameters

    // need to do some checks here for being slack (and also silly extension/contraction rates???)
    double minlen;
    minlen = m_Params.sse + minlpe;
    if (m_Params.len <= minlen)
    {
        m_Params.lastlpe = minlpe;

        m_Params.fce = 0; // contractile force (N)
        m_Params.lpe = minlpe; // contractile and parallel length (m)
        m_Params.fpe = 0; // parallel element force (N)
        m_Params.lse = m_Params.sse; // serial length (m)
        m_Params.fse = 0; // serial element force (N)
        m_Params.vce = 0; // contractile element velocity (m/s)
        m_Params.vse = 0; // serial element velocity (m/s)
        m_Params.targetFce = 0; // fce calculated from elastic elements (N)
        m_Params.f0 = 0; // length corrected fmax (N)
        m_Params.err = 0; // error term in lpe (m)
    }
    else
    {
        if (m_Params.ese == 0) // special case - easy to calculate
        {
            m_Params.err = CalculateForceError (m_Params.len - m_Params.sse, &m_Params);
            m_Params.lastlpe = m_Params.lpe;
        }
        else
        {

            // now solve the activation function so the contractile and elastic elements are consistent

            // we have a previous value for lce that is probably a good estimate of the new values
            double currentEstimate = m_Params.lastlpe;
            if (currentEstimate < 0) currentEstimate = 0;
            if (currentEstimate > m_Params.len) currentEstimate = m_Params.len;
            double flast = CalculateForceError(currentEstimate, &m_Params);
            if (fabs(flast) <= m_Tolerance)
            {
                m_Params.err = flast;
            }
            else
            {
                double ax = -DBL_MAX, bx = DBL_MAX, r, tol;
                // double range = maxlpe - minlpe; // this doesn't quite work because of damping
                double range = m_Params.len; // this should be bigger than necessary
                int nInc = 100;
                double inc = range / nInc;
                double high_target, low_target, err;
                int i;
                for (i = 1; i <= nInc; i++)
                {
                    high_target = currentEstimate + i * inc;
                    low_target = currentEstimate - i * inc;
                    if (high_target > m_Params.len) high_target = m_Params.len;
                    if (low_target < 0) low_target = 0;

                    if (high_target <= m_Params.len) // maxlpe might be expected to work but is too small
                    {
                        err = CalculateForceError(high_target, &m_Params);
                        if (std::signbit(err) != std::signbit(flast))
                        {
                            ax = currentEstimate + (i - 1) * inc;
                            bx = high_target;
                            break;
                        }
                    }
                    if (low_target >= 0) // minlpe might be expected to work but is too big
                    {
                        err = CalculateForceError(low_target, &m_Params);
                        if (std::signbit(err) != std::signbit(flast))
                        {
                            ax = currentEstimate - (i - 1) * inc;
                            bx = low_target;
                            break;
                        }
                    }
                    if (high_target >= m_Params.len && low_target <= 0) i = nInc + 1;
                }
                if (i > nInc)
                {
                    std::cerr << "MAMuscleComplete::SetActivation Error: Unable to solve lpe " << name() << "\n";
                    m_Params.err = CalculateForceError (currentEstimate, &m_Params); // couldn't find anything better
                    m_Params.lastlpe = currentEstimate;
                }
                else
                {
                    tol = m_Tolerance;
                    r = GSUtil::zeroin(ax, bx, &CalculateForceError, &m_Params, tol);
                    m_Params.err = CalculateForceError (r, &m_Params); // this sets m_Params with all the correct values
                    m_Params.lastlpe = r;
                }
            }
        }
    }

    GetStrap()->SetTension(m_Params.fse);
}

// calculate the metabolic power of the muscle

double MAMuscleComplete::GetMetabolicPower()
{
    // m_Velocity is negative when muscle shortening
    // we need the sign the other way round
    double relV = -m_Params.vce / m_Params.vmax;

    // limit relV
    if (relV > 1) relV = 1;
    else if (relV < -1) relV = -1;

    double relVSquared = relV * relV;
    double relVCubed = relVSquared * relV;

    double sigma = (0.054 + 0.506 * relV + 2.46 * relVSquared) /
        (1 - 1.13 * relV + 12.8 * relVSquared - 1.64 * relVCubed);

    double power = m_Params.alpha * m_Params.f0 * m_Params.vmax * sigma;

    return (power);
}

// note that with damping this is the energy stored but not the energy that will be returned
double MAMuscleComplete::GetESE() // energy serial element
{
    if (m_Params.lse <= m_Params.sse) return 0;

    double energy;
    double extension = m_Params.lse - m_Params.sse;
    // serial element model
    switch (m_Params.smpe)
    {
    case MAMuscleComplete::linear:
        energy = (0.5 * SQUARE(extension) * m_Params.ese);
        break;

    case MAMuscleComplete::square:
        energy = ((1.0/3.0) * CUBE(extension) * m_Params.ese);
        break;
    }

    return energy;
}

// note that with damping this is the energy stored but not the energy that will be returned
double MAMuscleComplete::GetEPE() // energy serial element
{
    if (m_Params.lpe <= m_Params.spe) return 0;

    double energy;
    double extension = m_Params.lpe - m_Params.spe;
    // serial element model
    switch (m_Params.smpe)
    {
    case MAMuscleComplete::linear:
        energy = (0.5 * SQUARE(extension) * m_Params.epe);
        break;

    case MAMuscleComplete::square:
        energy = ((1.0/3.0) * CUBE(extension) * m_Params.epe);
        break;
    }

    return energy;
}

// we can calculate the force from the length of the contractile element in two independent ways:
// (1) from the elastic properties of the serial and elastic elements
// (2) from the contractile element physiology and its activation
// this function calculates the error for the force as calculated by the elastic elements and the contractile elements
// this value should be zero for everything to work as expected and that's what the solver will attempt to achieve
double CalculateForceError (double lce, void *params)
{
    MAMuscleComplete::CalculateForceErrorParams *p = reinterpret_cast<MAMuscleComplete::CalculateForceErrorParams *>(params);

    // The elastic elements each generate a force and fce = fse - fpe

    p->lpe = lce;
    p->lse = p->len - p->lpe;
    p->vce = (p->lpe - p->lastlpe) / p->timeIncrement;
    p->vse = p->v - p->vce;

    // parallel element
    if (p->lpe <= p->spe) p->fpe = 0;
    else
    {
        switch (p->smpe)
        {
        case MAMuscleComplete::linear:
            p->fpe = p->epe * (p->lpe - p->spe) + p->dpe * p->vce; // positive sign because force gets higher as speed of extension increases
            break;

        case MAMuscleComplete::square:
            p->fpe = p->epe * SQUARE(p->lpe - p->spe) + p->dpe * p->vce; // positive sign because force gets higher as speed of extension increases
            break;
        }
        if (p->fpe < 0) p->fpe = 0;
    }


    // serial element
    if (p->lse <= p->sse) p->fse = 0;
    else
    {
        switch (p->smpe)
        {
        case MAMuscleComplete::linear:
            p->fse = p->ese * (p->lse - p->sse) + p->dse * p->vse; // positive sign because force gets higher as speed of extension increases
            break;

        case MAMuscleComplete::square:
            p->fse = p->ese * SQUARE(p->lse - p->sse) + p->dse * p->vse; // positive sign because force gets higher as speed of extension increases
            break;
        }
        if (p->fse < 0) p->fse = 0;
    }


    // can calculate the fce based on the elastic elements
    p->targetFce = p->fse - p->fpe;

    // contractile element based fce calculate

    // use the width to calculate the maximum force for the ce
    p->f0 = p->fmax * (1 - (4 * SQUARE(-1 + p->lpe/p->spe))/p->width);
    if (p->f0 <= 0)
    {
        p->f0 = 0;
        p->fce = 0;
    }
    else
    {
        if (p->alpha == 0)
        {
            p->fce = 0;
        }
        else
        {
            double localvce = p->vce;
            if (localvce > p->vmax) localvce = p->vmax; // velocity sanity limits
            if (localvce < -p->vmax) localvce = -p->vmax; // velocity sanity limits

            if (localvce > 0) // eccentric
            {
                p->fce = p->alpha * p->f0 * (1.8 + (0.8 * p->k*(localvce - 1.0 * p->vmax)) / (7.56 * localvce + p->k * p->vmax));
            }
            else // concentric
            {
                p->fce = (p->alpha * p->f0 * p->k * (localvce + p->vmax)) / (-localvce + p->k * p->vmax);
            }

        }
    }

    double err = p->fce - p->targetFce;
    // std::cerr << "lce = " << lce << " Error = " << err << "\n";
    return err;

}

std::string *MAMuscleComplete::createFromAttributes()
{
    if (Muscle::createFromAttributes()) return lastErrorPtr();
    std::string buf;

    if (findAttribute("ForcePerUnitArea"s, &buf) == nullptr) return lastErrorPtr();
    m_forcePerUnitArea = GSUtil::Double(buf);
    if (findAttribute("VMaxFactor"s, &buf) == nullptr) return lastErrorPtr();
    m_vMaxFactor = GSUtil::Double(buf);
    if (findAttribute("PCA"s, &buf) == nullptr) return lastErrorPtr();
    m_pca = GSUtil::Double(buf);
    double f0 = m_pca * m_forcePerUnitArea;
    if (findAttribute("FibreLength"s, &buf) == nullptr) return lastErrorPtr();
    m_fibreLength = GSUtil::Double(buf);
    double vMax = m_fibreLength * m_vMaxFactor;
    if (findAttribute("ActivationK"s, &buf) == nullptr) return lastErrorPtr();
    m_activationK = GSUtil::Double(buf);
    if (findAttribute("Width"s, &buf) == nullptr) return lastErrorPtr();
    m_width = GSUtil::Double(buf);
    this->SetMuscleProperties(vMax, f0, m_activationK, m_width);

    if (findAttribute("TendonLength"s, &buf) == nullptr) return lastErrorPtr();
    m_tendonLength = GSUtil::Double(buf);
    if (findAttribute("SerialStrainAtFmax"s, &buf) == nullptr) return lastErrorPtr();
    double serialStrainAtFmax = GSUtil::Double(buf);
    if (findAttribute("SerialStrainRateAtFmax"s, &buf) == nullptr) return lastErrorPtr();
    double serialStrainRateAtFmax = GSUtil::Double(buf);
    if (findAttribute("SerialStrainModel"s, &buf) == nullptr) return lastErrorPtr();
    if (buf == "Linear"s) m_serialStrainModel = MAMuscleComplete::linear;
    else if (buf == "Square"s) m_serialStrainModel = MAMuscleComplete::square;
    else { setLastError("MUSCLE ID=\""s + name() + "\": Invalid SerialStrainModel"s); return lastErrorPtr(); }
    if (findAttribute("ParallelStrainAtFmax"s, &buf) == nullptr) return lastErrorPtr();
    double parallelStrainAtFmax = GSUtil::Double(buf);
    if (findAttribute("ParallelStrainRateAtFmax"s, &buf) == nullptr) return lastErrorPtr();
    double parallelStrainRateAtFmax = GSUtil::Double(buf);
    if (findAttribute("ParallelStrainModel"s, &buf) == nullptr) return lastErrorPtr();
    if (buf == "Linear"s) m_parallelStrainModel = MAMuscleComplete::linear;
    else if (buf == "Square"s) m_parallelStrainModel = MAMuscleComplete::square;
    else { setLastError("MUSCLE ID=\""s + name() + "\": Invalid ParallelStrainModel"s); return lastErrorPtr(); }
    this->SetSerialElasticProperties(serialStrainAtFmax, serialStrainRateAtFmax, m_tendonLength, m_serialStrainModel);
    this->SetParallelElasticProperties(parallelStrainAtFmax, parallelStrainRateAtFmax, m_fibreLength, m_parallelStrainModel);

    if (findAttribute("ActivationKinetics"s, &buf) == nullptr) return lastErrorPtr();
    bool activationKinetics = GSUtil::Bool(buf);
    if (activationKinetics)
    {
        if (findAttribute("FastTwitchProportion"s, &buf) == nullptr) return lastErrorPtr();
        m_akFastTwitchProportion = GSUtil::Double(buf);
        if (findAttribute("TActivationA"s, &buf) == nullptr) return lastErrorPtr();
        m_akTActivationA = GSUtil::Double(buf);
        if (findAttribute("TActivationB"s, &buf) == nullptr) return lastErrorPtr();
        m_akTActivationB = GSUtil::Double(buf);
        if (findAttribute("TDeactivationA"s, &buf) == nullptr) return lastErrorPtr();
        m_akTDeactivationA = GSUtil::Double(buf);
        if (findAttribute("TDeactivationB"s, &buf) == nullptr) return lastErrorPtr();
        m_akTDeactivationB = GSUtil::Double(buf);
        this->SetActivationKinetics(activationKinetics, m_akFastTwitchProportion, m_akTActivationA, m_akTActivationB, m_akTDeactivationA, m_akTDeactivationB);
    }
    if (findAttribute("InitialFibreLength"s, &buf) == nullptr) return lastErrorPtr(); // FIX ME - InitialFibreLength is currently not used
    m_initialFibreLength = GSUtil::Double(buf);
    this->SetInitialFibreLength(m_initialFibreLength);
    if (findAttribute("ActivationRate"s, &buf) == nullptr) return lastErrorPtr();
    this->SetActivationRate(GSUtil::Double(buf));
    if (findAttribute("StartActivation"s, &buf) == nullptr) return lastErrorPtr(); // FIX ME - StartActivation is currently not used
    m_startActivation = GSUtil::Double(buf);
    this->SetStartActivation(m_startActivation);
    if (findAttribute("MinimumActivation"s, &buf) == nullptr) return lastErrorPtr();
    this->SetMinimumActivation(GSUtil::Double(buf));

    return nullptr;
}

void MAMuscleComplete::appendToAttributes()
{
    Muscle::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "MinettiAlexanderComplete"s);
    setAttribute("ForcePerUnitArea"s, *GSUtil::ToString(m_forcePerUnitArea, &buf));
    setAttribute("VMaxFactor"s, *GSUtil::ToString(m_vMaxFactor, &buf));
    setAttribute("PCA"s, *GSUtil::ToString(m_pca, &buf));
    setAttribute("FibreLength"s, *GSUtil::ToString(m_fibreLength, &buf));
    setAttribute("ActivationK"s, *GSUtil::ToString(m_activationK, &buf));
    setAttribute("Width"s, *GSUtil::ToString(m_width, &buf));

    setAttribute("TendonLength"s, *GSUtil::ToString(m_tendonLength, &buf));
    setAttribute("SerialStrainAtFmax"s, *GSUtil::ToString(m_serialStrainAtFmax, &buf));
    setAttribute("SerialStrainRateAtFmax"s, *GSUtil::ToString(m_serialStrainRateAtFmax, &buf));
    switch (m_serialStrainModel)
    {
    case MAMuscleComplete::linear:
        setAttribute("SerialStrainModel"s, "Linear"s);
        break;
    case MAMuscleComplete::square:
        setAttribute("SerialStrainModel"s, "Square"s);
        break;
    }
    setAttribute("ParallelStrainAtFmax"s, *GSUtil::ToString(m_parallelStrainAtFmax, &buf));
    setAttribute("ParallelStrainRateAtFmax"s, *GSUtil::ToString(m_parallelStrainRateAtFmax, &buf));
    switch (m_parallelStrainModel)
    {
    case MAMuscleComplete::linear:
        setAttribute("ParallelStrainModel"s, "Linear"s);
        break;
    case MAMuscleComplete::square:
        setAttribute("ParallelStrainModel"s, "Square"s);
        break;
    }

    setAttribute("ActivationKinetics"s, *GSUtil::ToString(m_ActivationKinetics, &buf));
    if (m_ActivationKinetics)
    {
        setAttribute("FastTwitchProportion"s, *GSUtil::ToString(m_akFastTwitchProportion, &buf));
        setAttribute("TActivationA"s, *GSUtil::ToString(m_akTActivationA, &buf));
        setAttribute("TActivationB"s, *GSUtil::ToString(m_akTActivationB, &buf));
        setAttribute("TDeactivationA"s, *GSUtil::ToString(m_akTDeactivationA, &buf));
        setAttribute("TDeactivationB"s, *GSUtil::ToString(m_akTDeactivationB, &buf));
    }
    setAttribute("InitialFibreLength"s, *GSUtil::ToString(m_initialFibreLength, &buf));
    setAttribute("ActivationRate"s, *GSUtil::ToString(m_ActivationRate, &buf));
    setAttribute("StartActivation"s, *GSUtil::ToString(m_startActivation, &buf));
    setAttribute("MinimumActivation"s, *GSUtil::ToString(m_MinimumActivation, &buf));

}

double MAMuscleComplete::fibreLength() const
{
    return m_fibreLength;
}

double MAMuscleComplete::tendonLength() const
{
    return m_tendonLength;
}

double MAMuscleComplete::pca() const
{
    return m_pca;
}

double MAMuscleComplete::forcePerUnitArea() const
{
    return m_forcePerUnitArea;
}

void MAMuscleComplete::setForcePerUnitArea(double forcePerUnitArea)
{
    m_forcePerUnitArea = forcePerUnitArea;
}

double MAMuscleComplete::vMaxFactor() const
{
    return m_vMaxFactor;
}

void MAMuscleComplete::setVMaxFactor(double vMaxFactor)
{
    m_vMaxFactor = vMaxFactor;
}

void MAMuscleComplete::setPca(double pca)
{
    m_pca = pca;
}

void MAMuscleComplete::setFibreLength(double fibreLength)
{
    m_fibreLength = fibreLength;
}

double MAMuscleComplete::activationK() const
{
    return m_activationK;
}

void MAMuscleComplete::setActivationK(double activationK)
{
    m_activationK = activationK;
}

double MAMuscleComplete::width() const
{
    return m_width;
}

void MAMuscleComplete::setWidth(double width)
{
    m_width = width;
}

void MAMuscleComplete::setTendonLength(double tendonLength)
{
    m_tendonLength = tendonLength;
}

double MAMuscleComplete::akFastTwitchProportion() const
{
    return m_akFastTwitchProportion;
}

void MAMuscleComplete::setAkFastTwitchProportion(double akFastTwitchProportion)
{
    m_akFastTwitchProportion = akFastTwitchProportion;
}

double MAMuscleComplete::akTActivationA() const
{
    return m_akTActivationA;
}

void MAMuscleComplete::setAkTActivationA(double akTActivationA)
{
    m_akTActivationA = akTActivationA;
}

double MAMuscleComplete::akTActivationB() const
{
    return m_akTActivationB;
}

void MAMuscleComplete::setAkTActivationB(double akTActivationB)
{
    m_akTActivationB = akTActivationB;
}

double MAMuscleComplete::akTDeactivationA() const
{
    return m_akTDeactivationA;
}

void MAMuscleComplete::setAkTDeactivationA(double akTDeactivationA)
{
    m_akTDeactivationA = akTDeactivationA;
}

double MAMuscleComplete::akTDeactivationB() const
{
    return m_akTDeactivationB;
}

void MAMuscleComplete::setAkTDeactivationB(double akTDeactivationB)
{
    m_akTDeactivationB = akTDeactivationB;
}

double MAMuscleComplete::initialFibreLength() const
{
    return m_initialFibreLength;
}

void MAMuscleComplete::setInitialFibreLength(double initialFibreLength)
{
    m_initialFibreLength = initialFibreLength;
}

double MAMuscleComplete::startActivation() const
{
    return m_startActivation;
}

void MAMuscleComplete::setStartActivation(double startActivation)
{
    m_startActivation = startActivation;
}

MAMuscleComplete::StrainModel MAMuscleComplete::serialStrainModel() const
{
    return m_serialStrainModel;
}

void MAMuscleComplete::setSerialStrainModel(const StrainModel &serialStrainModel)
{
    m_serialStrainModel = serialStrainModel;
}

void MAMuscleComplete::setSerialStrainModel(const std::string &serialStrainModel)
{
    if (serialStrainModel == "Linear"s) m_serialStrainModel = MAMuscleComplete::linear;
    if (serialStrainModel == "Square"s) m_serialStrainModel = MAMuscleComplete::square;
}

MAMuscleComplete::StrainModel MAMuscleComplete::parallelStrainModel() const
{
    return m_parallelStrainModel;
}

void MAMuscleComplete::setParallelStrainModel(const StrainModel &parallelStrainModel)
{
    m_parallelStrainModel = parallelStrainModel;
}

void MAMuscleComplete::setParallelStrainModel(const std::string &parallelStrainModel)
{
    if (parallelStrainModel == "Linear"s) m_parallelStrainModel = MAMuscleComplete::linear;
    if (parallelStrainModel == "Square"s) m_parallelStrainModel = MAMuscleComplete::square;
}

std::string MAMuscleComplete::dumpToString()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (firstDump())
    {
        setFirstDump(false);
        ss << "Time\tm_Stim\talpha\tlen\tv\tlastlpe\tfce\tlpe\tfpe\tlse\tfse\tvce\tvse\ttargetFce\tf0\terr\tESE\tEPE\tPSE\tPPE\tPCE\ttension\tlength\tvelocity\tPMECH\tPMET\n";
    }
    ss << simulation()->GetTime() << "\t" <<
          m_Stim << "\t" << m_Params.alpha << "\t" << m_Params.len << "\t" << m_Params.v << "\t" << m_Params.lastlpe << "\t" <<
          m_Params.fce << "\t" << m_Params.lpe << "\t" << m_Params.fpe << "\t" << m_Params.lse << "\t" << m_Params.fse << "\t" <<
          m_Params.vce << "\t" << m_Params.vse << "\t" << m_Params.targetFce << "\t" << m_Params.f0 << "\t" << m_Params.err << "\t" <<
          GetESE() << "\t" << GetEPE() << "\t" << GetPSE() << "\t" << GetPPE() << "\t" << GetPCE() << "\t" <<
          GetTension() << "\t" << GetLength() << "\t" << GetVelocity() << "\t" <<
          GetPower() << "\t" << GetMetabolicPower() <<
          "\n";
    return ss.str();
}



