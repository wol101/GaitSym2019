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

#define CUBE(x) ((x)*(x)*(x))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

using namespace std::string_literals;

static double CalculateForceError (double lce, void *params);
// static double zeroin(double *ax, double *bx, double (*f)(double x, void *info), void *info, double *tol);
static double zeroin(double ax, double bx, double (*f)(double x, void *info), void *info, double tol);

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
//                    r = zeroin(&ax, &bx, &CalculateForceError, &m_Params, &tol);
                    r = zeroin(ax, bx, &CalculateForceError, &m_Params, tol);
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
    if (findAttribute("InitialFibreLength"s, &buf) == nullptr) return lastErrorPtr();
    m_initialFibreLength = GSUtil::Double(buf);
    this->SetInitialFibreLength(m_initialFibreLength);
    if (findAttribute("ActivationRate"s, &buf) == nullptr) return lastErrorPtr();
    this->SetActivationRate(GSUtil::Double(buf));
    if (findAttribute("StartActivation"s, &buf) == nullptr) return lastErrorPtr();
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

std::string MAMuscleComplete::dump()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (getFirstDump())
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

#ifdef FORTRAN_DERIVED_ZEROIN

/* netlib function zeroin original fortran
      double precision function zeroin(ax,bx,f,tol)
      double precision ax,bx,f,tol
c
c      a zero of the function  f(x)  is computed in the interval ax,bx .
c
c  input..
c
c  ax     left endpoint of initial interval
c  bx     right endpoint of initial interval
c  f      function subprogram which evaluates f(x) for any x in
c         the interval  ax,bx
c  tol    desired length of the interval of uncertainty of the
c         final result (.ge.0.)
c
c  output..
c
c  zeroin abscissa approximating a zero of  f  in the interval ax,bx
c
c      it is assumed  that   f(ax)   and   f(bx)   have  opposite  signs
c  this is checked, and an error message is printed if this is not
c  satisfied.   zeroin  returns a zero  x  in the given interval
c  ax,bx  to within a tolerance  4*macheps*abs(x)+tol, where macheps  is
c  the  relative machine precision defined as the smallest representable
c  number such that  1.+macheps .gt. 1.
c      this function subprogram is a slightly  modified  translation  of
c  the algol 60 procedure  zero  given in  richard brent, algorithms for
c  minimization without derivatives, prentice-hall, inc. (1973).
c
      double precision  a,b,c,d,e,eps,fa,fb,fc,tol1,xm,p,q,r,s
      double precision  dabs, d1mach
   10 eps = d1mach(4)
      tol1 = eps+1.0d0
c
      a=ax
      b=bx
      fa=f(a)
      fb=f(b)
c     check that f(ax) and f(bx) have different signs
      if (fa .eq.0.0d0 .or. fb .eq. 0.0d0) go to 20
      if (fa * (fb/dabs(fb)) .le. 0.0d0) go to 20
         write(6,2500)
2500     format(1x,'f(ax) and f(bx) do not have different signs,',
     1             ' zeroin is aborting')
         return
   20 c=a
      fc=fa
      d=b-a
      e=d
   30 if (dabs(fc).ge.dabs(fb)) go to 40
      a=b
      b=c
      c=a
      fa=fb
      fb=fc
      fc=fa
   40 tol1=2.0d0*eps*dabs(b)+0.5d0*tol
      xm = 0.5d0*(c-b)
      if ((dabs(xm).le.tol1).or.(fb.eq.0.0d0)) go to 150
c
c see if a bisection is forced
c
      if ((dabs(e).ge.tol1).and.(dabs(fa).gt.dabs(fb))) go to 50
      d=xm
      e=d
      go to 110
   50 s=fb/fa
      if (a.ne.c) go to 60
c
c linear interpolation
c
      p=2.0d0*xm*s
      q=1.0d0-s
      go to 70
c
c inverse quadratic interpolation
c
   60 q=fa/fc
      r=fb/fc
      p=s*(2.0d0*xm*q*(q-r)-(b-a)*(r-1.0d0))
      q=(q-1.0d0)*(r-1.0d0)*(s-1.0d0)
   70 if (p.le.0.0d0) go to 80
      q=-q
      go to 90
   80 p=-p
   90 s=e
      e=d
      if (((2.0d0*p).ge.(3.0d0*xm*q-dabs(tol1*q))).or.(p.ge.
     *dabs(0.5d0*s*q))) go to 100
      d=p/q
      go to 110
  100 d=xm
      e=d
  110 a=b
      fa=fb
      if (dabs(d).le.tol1) go to 120
      b=b+d
      go to 140
  120 if (xm.le.0.0d0) go to 130
      b=b+tol1
      go to 140
  130 b=b-tol1
  140 fb=f(b)
      if ((fb*(fc/dabs(fc))).gt.0.0d0) go to 20
      go to 30
  150 zeroin=b
      return
      end
*/

/* Standard C source for D1MACH */
/*
#include <stdio.h>
#include <cfloat>
#include <cmath>
double d1mach_(long *i)
{
    switch(*i){
      case 1: return DBL_MIN;
      case 2: return DBL_MAX;
      case 3: return DBL_EPSILON/FLT_RADIX;
      case 4: return DBL_EPSILON;
      case 5: return log10((double)FLT_RADIX);
      }
    fprintf(stderr, "invalid argument: d1mach(%ld)\n", *i);
    exit(1); return 0;
} */

// zeroin.f -- translated by f2c (version 20031025).
// and subsequently edited to improve readability slightly

double zeroin(double *ax, double *bx, double (*f)(double x, void *info), void *info, double *tol)
{
    /* System generated locals */
    double ret_val = 0, d__1, d__2;

    /* Local variables */
    double a, b, c__, d__, e, p, q, r__, s, fa, fb, fc, xm, eps, tol1;

    /*      a zero of the function  f(x)  is computed in the interval ax,bx . */

    /*  input.. */

    /*  ax     left endpoint of initial interval */
    /*  bx     right endpoint of initial interval */
    /*  f      function subprogram which evaluates f(x) for any x in */
    /*         the interval  ax,bx */
    /*  tol    desired length of the interval of uncertainty of the */
    /*         final result (>= 0.) */

    /*  output.. */

    /*  zeroin abscissa approximating a zero of  f  in the interval ax,bx */

    /*      it is assumed  that   f(ax)   and   f(bx)   have  opposite  signs */
    /*  this is checked, and an error message is printed if this is not */
    /*  satisfied.   zeroin  returns a zero  x  in the given interval */
    /*  ax,bx  to within a tolerance  4*macheps*fabs(x)+tol, where macheps  is */
    /*  the  relative machine precision defined as the smallest representable */
    /*  number such that  1.+macheps > 1. */
    /*      this function subprogram is a slightly  modified  translation  of */
    /*  the algol 60 procedure  zero  given in  richard brent, algorithms for */
    /*  minimization without derivatives, prentice-hall, inc. (1973). */

    /* L10: */
    eps = DBL_EPSILON;
    tol1 = eps + 1.;

    a = *ax;
    b = *bx;
    fa = (*f)(a, info);
    fb = (*f)(b, info);
    /*     check that f(ax) and f(bx) have different signs */
    if (fa == 0. || fb == 0.) {
        goto L20;
    }
    if (fa * (fb / fabs(fb)) <= 0.) {
        goto L20;
    }
    fprintf(stderr, "f(ax) and f(bx) do not have different signs, zeroin is aborting\n");
    return ret_val;
L20:
    c__ = a;
    fc = fa;
    d__ = b - a;
    e = d__;
L30:
    if (fabs(fc) >= fabs(fb)) {
        goto L40;
    }
    a = b;
    b = c__;
    c__ = a;
    fa = fb;
    fb = fc;
    fc = fa;
L40:
    tol1 = eps * 2. * fabs(b) + *tol * .5;
    xm = (c__ - b) * .5;
    if (fabs(xm) <= tol1 || fb == 0.) {
        goto L150;
    }

    /* see if a bisection is forced */

    if (fabs(e) >= tol1 && fabs(fa) > fabs(fb)) {
        goto L50;
    }
    d__ = xm;
    e = d__;
    goto L110;
L50:
    s = fb / fa;
    if (a != c__) {
        goto L60;
    }

    /* linear interpolation */

    p = xm * 2. * s;
    q = 1. - s;
    goto L70;

    /* inverse quadratic interpolation */

L60:
    q = fa / fc;
    r__ = fb / fc;
    p = s * (xm * 2. * q * (q - r__) - (b - a) * (r__ - 1.));
    q = (q - 1.) * (r__ - 1.) * (s - 1.);
L70:
    if (p <= 0.) {
        goto L80;
    }
    q = -q;
    goto L90;
L80:
    p = -p;
L90:
    s = e;
    e = d__;
    if (p * 2. >= xm * 3. * q - (d__1 = tol1 * q, fabs(d__1)) || p >= (d__2 =
                                                                       s * .5 * q, fabs(d__2))) {
        goto L100;
    }
    d__ = p / q;
    goto L110;
L100:
    d__ = xm;
    e = d__;
L110:
    a = b;
    fa = fb;
    if (fabs(d__) <= tol1) {
        goto L120;
    }
    b += d__;
    goto L140;
L120:
    if (xm <= 0.) {
        goto L130;
    }
    b += tol1;
    goto L140;
L130:
    b -= tol1;
L140:
    fb = (*f)(b, info);
    if (fb * (fc / fabs(fc)) > 0.) {
        goto L20;
    }
    goto L30;
L150:
    ret_val = b;
    return ret_val;
} /* zeroin_ */

#else

/*
 ************************************************************************
 *                  C math library
 * function ZEROIN - obtain a function zero within the given range
 *
 * Input
 *  double zeroin(ax,bx,f,tol)
 *  double ax;          Root will be seeked for within
 *  double bx;              a range [ax,bx]
 *  double (*f)(double x);      Name of the function whose zero
 *                  will be seeked for
 *  double tol;         Acceptable tolerance for the root
 *                  value.
 *                  May be specified as 0.0 to cause
 *                  the program to find the root as
 *                  accurate as possible
 *
 * Output
 *  Zeroin returns an estimate for the root with accuracy
 *  4*EPSILON*abs(x) + tol
 *
 * Algorithm
 *  G.Forsythe, M.Malcolm, C.Moler, Computer methods for mathematical
 *  computations. M., Mir, 1980, p.180 of the Russian edition
 *
 *  The function makes use of the bissection procedure combined with
 *  the linear or quadric inverse interpolation.
 *  At every step program operates on three abscissae - a, b, and c.
 *  b - the last and the best approximation to the root
 *  a - the last but one approximation
 *  c - the last but one or even earlier approximation than a that
 *      1) |f(b)| <= |f(c)|
 *      2) f(b) and f(c) have opposite signs, i.e. b and c confine
 *         the root
 *  At every step Zeroin selects one of the two new approximations, the
 *  former being obtained by the bissection procedure and the latter
 *  resulting in the interpolation (if a,b, and c are all different
 *  the quadric interpolation is utilized, otherwise the linear one).
 *  If the latter (i.e. obtained by the interpolation) point is
 *  reasonable (i.e. lies within the current interval [b,c] not being
 *  too close to the boundaries) it is accepted. The bissection result
 *  is used in the other case. Therefore, the range of uncertainty is
 *  ensured to be reduced at least by the factor 1.6
 *
 ************************************************************************
 */

//double zeroin(ax,bx,f,tol)        /* An estimate to the root  */
//double ax;                        /* Left border | of the range   */
//double bx;                        /* Right border| the root is seeked*/
//double (*f)(double x);            /* Function under investigation */
//double tol;                       /* Acceptable tolerance     */

double zeroin(double ax, double bx, double (*f)(double x, void *info), void *info, double tol)
{
    double a,b,c;             /* Abscissae, descr. see above  */
    double fa;                /* f(a)             */
    double fb;                /* f(b)             */
    double fc;                /* f(c)             */

    a = ax;  b = bx;  fa = (*f)(a, info);  fb = (*f)(b, info);
    c = a;   fc = fa;

    for(;;)       /* Main iteration loop  */
    {
        double prev_step = b-a;             /* Distance from the last but one*/
        /* to the last approximation    */
        double tol_act;                     /* Actual tolerance     */
        double p;                           /* Interpolation step is calcu- */
        double q;                           /* lated in the form p/q; divi- */
                                            /* sion operations is delayed   */
                                            /* until the last moment    */
        double new_step;                    /* Step at this iteration       */

        if( fabs(fc) < fabs(fb) )
        {                                   /* Swap data for b to be the    */
            a = b;  b = c;  c = a;          /* best approximation       */
            fa=fb;  fb=fc;  fc=fa;
        }
        tol_act = 2*DBL_EPSILON*fabs(b) + tol/2;
        new_step = (c-b)/2;

        if( fabs(new_step) <= tol_act || fb == 0 )
            return b;                       /* Acceptable approx. is found  */

        /* Decide if the interpolation can be tried */
        if( fabs(prev_step) >= tol_act      /* If prev_step was large enough*/
                && fabs(fa) > fabs(fb) )    /* and was in true direction,   */
        {                                   /* Interpolatiom may be tried   */
            double t1,cb,t2;
            cb = c-b;
            if( a==c )                      /* If we have only two distinct */
            {                               /* points linear interpolation  */
                t1 = fb/fa;                 /* can only be applied      */
                p = cb*t1;
                q = 1.0 - t1;
            }
            else                            /* Quadric inverse interpolation*/
            {
                q = fa/fc;  t1 = fb/fc;  t2 = fb/fa;
                p = t2 * ( cb*q*(q-t1) - (b-a)*(t1-1.0) );
                q = (q-1.0) * (t1-1.0) * (t2-1.0);
            }
            if( p>0 )                       /* p was calculated with the op-*/
                q = -q;                     /* posite sign; make p positive */
            else                            /* and assign possible minus to */
                p = -p;                     /* q                */

            if( p < (0.75*cb*q-fabs(tol_act*q)/2)   /* If b+p/q falls in [b,c]*/
                    && p < fabs(prev_step*q/2) )    /* and isn't too large  */
                new_step = p/q;                     /* it is accepted   */
                                                    /* If p/q is too large then the */
                                                    /* bissection procedure can     */
                                                    /* reduce [b,c] range to more   */
                                                    /* extent           */
        }

        if( fabs(new_step) < tol_act )      /* Adjust the step to be not less*/
        {
            if( new_step > 0 )              /* than tolerance       */
                new_step = tol_act;
            else
                new_step = -tol_act;
        }

        a = b;  fa = fb;                    /* Save the previous approx.    */
        b += new_step;  fb = (*f)(b, info); /* Do step to a new approxim.   */
        if( (fb > 0 && fc > 0) || (fb < 0 && fc < 0) )
        {                                   /* Adjust c for it to have a sign*/
            c = a;  fc = fa;                /* opposite to that of b    */
        }
    }

}

#endif

