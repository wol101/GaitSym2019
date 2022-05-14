/*
 *  FixedJoint.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 20/09/2008.
 *  Copyright 2008 Bill Sellers. All rights reserved.
 *
 */

#include "FixedJoint.h"
#include "DataFile.h"
#include "Body.h"
#include "Simulation.h"
#include "GSUtil.h"
#include "ButterworthFilter.h"
#include "MovingAverage.h"
#include "Marker.h"

#include <sstream>

using namespace std::string_literals;

FixedJoint::FixedJoint(dWorldID worldID) : Joint()
{
    setJointID(dJointCreateFixed(worldID, nullptr));
    dJointSetData(JointID(), this);
    dJointSetFeedback(JointID(), JointFeedback());
}

void FixedJoint::SetFixed()
{
    dJointSetFixed(JointID());
}

void FixedJoint::LateInitialisation()
{
    if (m_lateFix) SetFixed();
}

// this is the part where we calculate the stress map
// SetCrossSection needs to be called first
// note for this to work the centre of the fixed joint needs to be the centroid of the cross section area
void FixedJoint::CalculateStress()
{
    // first of all we need to convert the forces and torques into the joint local coordinate system

    // the force feedback is at the CM for fixed joints
    // first we need to move it to the joint position

    // calculate the offset of the stress position from the CM
    dVector3 result;
    dBodyVectorToWorld (this->GetBody1()->GetBodyID(), m_StressOrigin.x, m_StressOrigin.y, m_StressOrigin.z, result);
    pgd::Vector3 worldStressOffset(result[0], result[1], result[2]);

    // now the linear components of JointFeedback() will generate a torque if applied at this position
    // torque = r x f
    pgd::Vector3 forceCM(JointFeedback()->f1[0], JointFeedback()->f1[1], JointFeedback()->f1[2]);
    pgd::Vector3 addedTorque = worldStressOffset ^ forceCM;

    pgd::Vector3 torqueCM(JointFeedback()->t1[0], JointFeedback()->t1[1], JointFeedback()->t1[2]);
    pgd::Vector3 torqueStressOrigin = torqueCM - addedTorque;

    // now rotate new values to stress based coordinates
    const double *q = dBodyGetQuaternion (this->GetBody1()->GetBodyID());
    pgd::Quaternion bodyOrientation(q[0], q[1], q[2], q[3]);
    m_torqueStressCoords = pgd::QVRotate(m_StressOrientation, pgd::QVRotate(bodyOrientation, torqueStressOrigin));
    m_forceStressCoords = pgd::QVRotate(m_StressOrientation, pgd::QVRotate(bodyOrientation, forceCM));

    if (m_stressCalculationType == beam)
    {

        // torques x and y correspond to bending and z corresponds to torsion
        // forces x and y correspond to shear and z corresponds to compressions/tension

        // now we use the cross section bitmap to calculate the loading

        // first linear

        double linearStress = m_forceStressCoords.z / m_area;

        // now add rotational

        // Stress in a beam
        // The general form of the classic bending formula for a beam in co-ordinate system having origin located at the neutral axis of the beam is (Pilkey 2002, p. 17):
        // Pilkey, Walter D. (2002). Analysis and Design of Elastic Beams. John Wiley & Sons, Inc.. ISBN 0-471-38152-7

        // sigma = -[(MyIx + MxIxy)/(IxIy - Ixy^2)]x + [(MxIy + MyIxy)/(IxIy - Ixy^2)]y

        // sigma = the normal stress in the beam due to bending
        // x = the perpendicular distance to the centroidal y-axis
        // y = the perpendicular distance to the centroidal x-axis
        // My = the bending moment about the y-axis
        // Mx = the bending moment about the x-axis
        // Ix = the second moment of area about x-axis
        // Iy = the second moment of area about y-axis
        // Ixy = the product moment of area
        // If the coordinate system is chosen to give a product moment of area equal to zero, the formula simplifies to:

        // sigma = -(My/Iy)x + (Mx/Ix)y

        // If additionally the beam is only subjected to bending about one axis, the formula simplifies further:

        // sigma = (Mx/Ix)y

        // for the x and y (bending) components
        // for each element in the stiffness array
        // stress = (torque / second moment of area) * moment arm

        double My = m_torqueStressCoords.y;
        double Mx = m_torqueStressCoords.x;
        // precalculate invariant bits of the formula
        double t1 = (My * m_Ix + Mx * m_Ixy)/(m_Ix * m_Iy - m_Ixy * m_Ixy);
        double t2 = (Mx * m_Iy + My * m_Ixy)/(m_Ix * m_Iy - m_Ixy * m_Ixy);
        double *xDistancePtr = m_xDistances.data();
        double *yDistancePtr = m_yDistances.data();

        double *stressPtr = m_stress.data();
        m_minStress = DBL_MAX;
        m_maxStress = -DBL_MAX;
        for (size_t i = 0; i < m_nActivePixels; i++)
        {
            *stressPtr = -t1 * (*xDistancePtr) + t2 * (*yDistancePtr) + linearStress;
            if (*stressPtr > m_maxStress) m_maxStress = *stressPtr;
            if (*stressPtr < m_minStress) m_minStress = *stressPtr;
            stressPtr++;
            xDistancePtr++;
            yDistancePtr++;
        }
    }
    else if (m_stressCalculationType == spring)
    {
        m_torqueScalar = m_torqueStressCoords.Magnitude();
        m_torqueAxis = m_torqueStressCoords / m_torqueScalar;

        // assuming all the springs are the same then
        pgd::Vector3 forcePerSpring1 = m_forceStressCoords / double(m_nActivePixels);

        // but for the torque we need to find the total torsional springiness
        double *xDistancePtr = m_xDistances.data();
        double *yDistancePtr = m_yDistances.data();
        double totalNominalTorque = 0;

        for (size_t i = 0; i < m_nActivePixels; i++)
        {
            pgd::Vector3 p(*xDistancePtr, *yDistancePtr, 0);
            pgd::Vector3 closestPoint = m_torqueAxis * (m_torqueAxis * p);
            pgd::Vector3 r = p - closestPoint;
            double distance2 = r.Magnitude2();
            double distance = sqrt(distance2);
            if (distance2 > 1e-10)
            {
                pgd::Vector3 direction = m_torqueAxis ^ r;
                direction.Normalize();
                pgd::Vector3 forcePerSpring2 =  direction * distance; // force per spring is proportional to the perpendicular distance
                totalNominalTorque += distance2; // but the torque per spring is proportional to the perpendicular distance squared
                m_vectorList[i] = forcePerSpring2;
            }
            else
            {
                m_vectorList[i] = pgd::Vector3(0, 0, 0);
            }
            xDistancePtr++;
            yDistancePtr++;
        }

        double torqueScale = m_torqueScalar / totalNominalTorque; // this will make the total torque produced by the springs add up to the actual torque
        m_minStress = DBL_MAX;
        m_maxStress = -DBL_MAX;
        double *stressPtr = m_stress.data();
        double dArea = m_dx * m_dy;
        for (size_t i = 0; i < m_nActivePixels; i++)
        {
            //std::cerr << m_vectorList[i].x << " " << m_vectorList[i].y << " " << m_vectorList[i].z << "\n";
            m_vectorList[i] = (m_vectorList[i] * torqueScale) + forcePerSpring1;
            *stressPtr = m_vectorList[i].Magnitude() / dArea;
            if (*stressPtr > m_maxStress) m_maxStress = *stressPtr;
            if (*stressPtr < m_minStress) m_minStress = *stressPtr;
            stressPtr++;
        }

//#define SANITY_CHECK
#ifdef SANITY_CHECK
        // check that my forces and my torqes add up
        pgd::Vector3 totalForce;
        pgd::Vector3 totalTorque;
        size_t i = 0;
        unsigned char *ptr = m_stiffness.data();
        for (size_t iy = 0; iy < m_ny; iy++)
        {
            for (size_t ix = 0; ix < m_nx; ix++)
            {
                if (*ptr)
                {
                    totalForce += m_vectorList[i];

                    pgd::Vector3 p(((ix) + 0.5) * m_dx - m_xOrigin, ((iy) + 0.5) * m_dy - m_yOrigin, 0);
                    pgd::Vector3 closestPoint = m_torqueAxis * (m_torqueAxis * p);
                    pgd::Vector3 r = p - closestPoint;
                    pgd::Vector3 torque = r ^ m_vectorList[i];
                    std::cerr << "torque " << torque.x << " " << torque.y << " " << torque.z << "\n";
                    totalTorque += torque;
                    i++;
                }
                ptr++;
            }
        }
        std::cerr << "m_forceStressCoords " << m_forceStressCoords.x << " " << m_forceStressCoords.y << " " << m_forceStressCoords.z << "\n";
        std::cerr << "totalForce " << totalForce.x << " " << totalForce.y << " " << totalForce.z << "\n";
        std::cerr << "m_torqueStressCoords " << m_torqueStressCoords.x << " " << m_torqueStressCoords.y << " " << m_torqueStressCoords.z << "\n";
        std::cerr << "totalTorque " << totalTorque.x << " " << totalTorque.y << " " << totalTorque.z << "\n";
        std::cerr << "m_torqueStressCoords.Magnitude " << m_torqueStressCoords.Magnitude() << " " << "totalTorque.Magnitude " << totalTorque.Magnitude() << "\n";
#endif
    }

    // update the stress list
    switch (m_lowPassType)
    {
    case NoLowPass:
        m_lowPassMinStress = m_minStress;
        m_lowPassMaxStress = m_maxStress;
        break;

    case MovingAverageLowPass:
    case Butterworth2ndOrderLowPass:
        m_lowPassMinStress = DBL_MAX;
        m_lowPassMaxStress = -DBL_MAX;
        for (size_t i = 0; i < m_nActivePixels; i++)
        {
            m_filteredStress[i]->AddNewSample(m_stress[i]);
            if (m_filteredStress[i]->Output() > m_lowPassMaxStress)
                m_lowPassMaxStress = m_filteredStress[i]->Output();
            if (m_filteredStress[i]->Output() < m_lowPassMinStress)
                m_lowPassMinStress = m_filteredStress[i]->Output();
        }
        break;
    }
}

const std::vector<unsigned char> &FixedJoint::pixMap() const
{
    return m_pixMap;
}

double FixedJoint::highRange() const
{
    return m_highRange;
}

void FixedJoint::setHighRange(double highRange)
{
    m_highRange = highRange;
}

double FixedJoint::lowRange() const
{
    return m_lowRange;
}

void FixedJoint::setLowRange(double lowRange)
{
    m_lowRange = lowRange;
}

const std::vector<unsigned char> &FixedJoint::stiffness() const
{
    return m_stiffness;
}

size_t FixedJoint::ny() const
{
    return m_ny;
}

size_t FixedJoint::nx() const
{
    return m_nx;
}

double FixedJoint::yOrigin() const
{
    return m_yOrigin;
}

double FixedJoint::xOrigin() const
{
    return m_xOrigin;
}

double FixedJoint::height() const
{
    return m_height;
}

double FixedJoint::width() const
{
    return m_width;
}

// this is where we set the cross section and precalculate the second moment of area
// the cross section array is a unsigned char image with origin at bottom left (standard raster origin will need to have y reversed but this matches the OpenGL standard)
// it scans row first and then vertically
// nx, ny are the dimensions of the array
// dx, dy are the real world sizes of each pixel in the array
// stiffness array is copied
// the edge of the mesh corresponds to the edge of the pixel not the centre
void FixedJoint::SetCrossSection(const std::vector<unsigned char> &stiffness, size_t nx, size_t ny, double dx, double dy)
{
    size_t ix, iy;
    unsigned char *ptr;
    double xsum = 0;
    double ysum = 0;

    m_dx = dx;
    m_dy = dy;
    m_nx = nx;
    m_ny = ny;
    m_width = m_nx * m_dx;
    m_height = m_ny * m_dy;


    m_stiffness = stiffness;
    ptr = m_stiffness.data();

    // calculate centre of area
    m_nActivePixels = 0;
    for (iy = 0; iy < m_ny; iy++)
    {
        for (ix = 0; ix < m_nx; ix++)
        {
            if (*ptr)
            {
                m_nActivePixels++;
                xsum += (double(ix) + 0.5) * m_dx;
                ysum += (double(iy) + 0.5) * m_dy;
            }
            ptr++;
        }
    }
    m_xOrigin = xsum / m_nActivePixels;
    m_yOrigin = ysum / m_nActivePixels;

    // now calculate second moment of area and distances from the origin

    ptr = m_stiffness.data();
    double dArea = m_dx * m_dy;
    m_area = m_nActivePixels * dArea;
    m_xDistances.clear();
    m_xDistances.resize(m_nActivePixels);
    m_yDistances.clear();
    m_yDistances.resize(m_nActivePixels);
    double *xDistancePtr = m_xDistances.data();
    double *yDistancePtr = m_yDistances.data();
    m_Ix = 0;
    m_Iy = 0;
    m_Ixy = 0;
    for (iy = 0; iy < m_ny; iy++)
    {
        for (ix = 0; ix < m_nx; ix++)
        {
            if (*ptr)
            {
                *xDistancePtr = (double(ix) + 0.5) * m_dx - m_xOrigin;
                *yDistancePtr = (double(iy) + 0.5) * m_dy - m_yOrigin;
                // second moments of area and product moment of area
                // Pilkey, Walter D. (2002). Analysis and Design of Elastic Beams. John Wiley & Sons, Inc.. ISBN 0-471-38152-7
                // Ix = sum(y2 * area)
                // Iy = sum(x2 * area)
                // Ixy = sum(xy * area)
                m_Ix += (*yDistancePtr) * (*yDistancePtr) * dArea;
                m_Iy += (*xDistancePtr) * (*xDistancePtr) * dArea;
                m_Ixy += (*xDistancePtr) * (*yDistancePtr) * dArea;
                xDistancePtr++;
                yDistancePtr++;
            }
            ptr++;
        }
    }

    // allocate the vector list
    m_vectorList.clear();
    m_vectorList.resize(m_nActivePixels);
    m_stress.clear();
    m_stress.resize(m_nActivePixels);
}

// note: m_StressOrigin is in Body1 local coordinates
void FixedJoint::SetStressOrigin(double x, double y, double z)
{
    m_StressOrigin.x = x;
    m_StressOrigin.y = y;
    m_StressOrigin.z = z;
}

// note: m_StressOrientation is in Body1 local coordinates
void FixedJoint::SetStressOrientation(double q0, double q1, double q2, double q3)
{
    m_StressOrientation.n = q0;
    m_StressOrientation.x = q1;
    m_StressOrientation.y = q2;
    m_StressOrientation.z = q3;
    m_StressOrientation.Normalize(); // this is the safest option
}


void FixedJoint::SetWindow(size_t window)
{
//    m_minStressMovingAverage = new MovingAverage(window);
//    m_maxStressMovingAverage = new MovingAverage(window);
    m_window = window;
    m_lowPassType = MovingAverageLowPass;
    m_filteredStress.clear();
    m_filteredStress.reserve(m_nActivePixels);
    for (size_t i = 0; i < m_nActivePixels; i++) m_filteredStress.push_back(std::make_unique<MovingAverage>(int(window)));
}

void FixedJoint::SetCutoffFrequency(double cutoffFrequency)
{
    m_cutoffFrequency = cutoffFrequency;
    double samplingFrequency = 1.0 / simulation()->GetTimeIncrement();
//    m_minStressButterworth = new ButterworthFilter(cutoffFrequency, samplingFrequency);
//    m_maxStressButterworth = new ButterworthFilter(cutoffFrequency, samplingFrequency);
    m_lowPassType = Butterworth2ndOrderLowPass;
    m_filteredStress.clear();
    m_filteredStress.reserve(m_nActivePixels);
#ifdef NON_THREAD_SAFE_OK
    for (size_t i = 0; i < m_nActivePixels; i++) m_filteredStress.push_back(std::make_unique<SharedButterworthFilter>());
    SharedButterworthFilter::CalculateCoefficients(cutoffFrequency, samplingFrequency);
#else
    for (size_t i = 0; i < m_nActivePixels; i++) m_filteredStress.push_back(std::make_unique<ButterworthFilter>(cutoffFrequency, samplingFrequency));
#endif
}

bool FixedJoint::CheckStressAbort()
{
    if (m_stressCalculationType == none) return false;
    if (m_lowPassMaxStress > m_stressLimit) return true;
    if (m_lowPassMinStress < -m_stressLimit) return true;
    return false;
}

void FixedJoint::Update()
{
    if (m_stressCalculationType != none) CalculateStress();
}

std::string *FixedJoint::createFromAttributes()
{
    if (Joint::createFromAttributes()) return lastErrorPtr();
    std::string buf;
    buf.reserve(1000000);

    pgd::Vector3 position = body1Marker()->GetPosition();
    this->SetStressOrigin(position.x, position.y, position.z);
    pgd::Quaternion quaternion = body1Marker()->GetQuaternion();
    this->SetStressOrientation(quaternion.n, quaternion.x, quaternion.y, quaternion.z);

    SetFixed();
    if (CFM() >= 0) dJointSetFixedParam (JointID(), dParamCFM, CFM());
    if (ERP() >= 0) dJointSetFixedParam (JointID(), dParamERP, ERP());
    if (findAttribute("LateFix"s, &buf)) m_lateFix = GSUtil::Bool(buf);

    if (findAttribute("StressCalculationType"s, &buf) == nullptr) return lastErrorPtr();
    if (buf == "None"s) this->SetStressCalculationType(FixedJoint::none);
    else if (buf == "Beam"s) this->SetStressCalculationType(FixedJoint::beam);
    else if (buf == "Spring"s) this->SetStressCalculationType(FixedJoint::spring);
    else { setLastError("Joint ID=\""s + name() +"\" unrecognised StressCalculationTypes"s); return lastErrorPtr(); }

    if (m_stressCalculationType != FixedJoint::none)
    {
        if (findAttribute("LowPassType"s, &buf) == nullptr) return lastErrorPtr();
        if (buf == "Butterworth2ndOrderLowPass"s) this->SetLowPassType(FixedJoint::Butterworth2ndOrderLowPass);
        else if (buf == "MovingAverageLowPass"s) this->SetLowPassType(FixedJoint::MovingAverageLowPass);
        else if (buf == "NoLowPass"s) this->SetLowPassType(FixedJoint::NoLowPass);
        else { setLastError("Joint ID=\""s + name() +"\" unrecognised LowPassType"s); return lastErrorPtr(); }

        if (findAttribute("StressLimit"s, &buf) == nullptr) return lastErrorPtr();
        this->SetStressLimit(GSUtil::Double(buf));

        double doubleList[2];
        if (findAttribute("StressBitmapPixelSize"s, &buf) == nullptr) return lastErrorPtr();
        GSUtil::Double(buf, 2, doubleList);
        double dx = doubleList[0];
        double dy = doubleList[1];
        if (findAttribute("StressBitmapDimensions"s, &buf) == nullptr) return lastErrorPtr();
        GSUtil::Double(buf, 2, doubleList);
        int nx = int(doubleList[0] + 0.5);
        int ny = int(doubleList[1] + 0.5);
        if (findAttribute("StressBitmap"s, &buf) == nullptr) return lastErrorPtr();
        this->SetCrossSection(AsciiToBitMap(buf, nx, ny, '1', true), nx, ny, dx, dy);

        switch (m_lowPassType)
        {
        case FixedJoint::NoLowPass:
            break;
        case FixedJoint::Butterworth2ndOrderLowPass:
            if (findAttribute("CutoffFrequency"s, &buf) == nullptr) return lastErrorPtr();
            this->SetCutoffFrequency(GSUtil::Double(buf));
            break;
        case FixedJoint::MovingAverageLowPass:
            if (findAttribute("Window"s, &buf) == nullptr) return lastErrorPtr();
            this->SetWindow(GSUtil::Int(buf));
            break;
        }

        if (findAttribute("StressBitmapDisplayRange"s, &buf))
        {
            GSUtil::Double(buf, 2, doubleList);
            setLowRange(doubleList[0]);
            setHighRange(doubleList[1]);
        }


    }

    return nullptr;
}

void FixedJoint::appendToAttributes()
{
    Joint::appendToAttributes();
    std::string buf;
    buf.reserve(1000000);
    setAttribute("Type"s, "Fixed"s);
    setAttribute("LateFix"s, *GSUtil::ToString(m_lateFix, &buf));
    switch (m_stressCalculationType)
    {
    case FixedJoint::none:
        setAttribute("StressCalculationType"s, "None"s);
        break;
    case FixedJoint::beam:
        setAttribute("StressCalculationType"s, "Beam"s);
        break;
    case FixedJoint::spring:
        setAttribute("StressCalculationType"s, "Spring"s);
        break;
    }
    if (m_stressCalculationType != FixedJoint::none)
    {
        switch (m_lowPassType)
        {
        case FixedJoint::Butterworth2ndOrderLowPass:
            setAttribute("LowPassType"s, "Butterworth2ndOrderLowPass"s);
            setAttribute("CutoffFrequency"s, *GSUtil::ToString(m_cutoffFrequency, &buf));
            break;
        case FixedJoint::MovingAverageLowPass:
            setAttribute("LowPassType"s, "MovingAverageLowPass"s);
            setAttribute("Window"s, *GSUtil::ToString(m_window, &buf));
            break;
        case FixedJoint::NoLowPass:
            setAttribute("LowPassType"s, "NoLowPass"s);
            break;
        }
        setAttribute("StressLimit"s, *GSUtil::ToString(m_stressLimit, &buf));
        double doubleList[2] = { m_dx, m_dy };
        setAttribute("StressBitmapPixelSize"s, *GSUtil::ToString(doubleList, 2, &buf));
        size_t intList[2] = { m_nx, m_ny };
        setAttribute("StressBitmapDimensions"s, *GSUtil::ToString(intList, 2, &buf));
        std::string bitmap;
        bitmap.reserve(size_t((m_nx + 2) * m_ny));
        for (size_t iy = m_ny - 1; iy < m_ny; iy--)
        {
            bitmap.append("\n");
            for (size_t ix = 0; ix < m_nx; ix++)
            {
                if (m_stiffness[iy * m_nx + ix]) bitmap.append("1");
                else  bitmap.append("0");
            }
        }
        bitmap.append("\n");
        setAttribute("StressBitmap"s, bitmap);
        double doubleList2[2] = { m_lowRange, m_highRange };
        setAttribute("StressBitmapDisplayRange"s, *GSUtil::ToString(doubleList2, 2, &buf));
    }
}

std::string FixedJoint::dumpToString()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (firstDump())
    {
        setFirstDump(false);
        if (m_stiffness.size() == 0)
        {
            ss << "Time\tXP\tYP\tZP\tFX1\tFY1\tFZ1\tTX1\tTY1\tTZ1\tFX2\tFY2\tFZ2\tTX2\tTY2\tTZ2\n";
        }
        else
        {
            if (m_stressCalculationType == beam)
            {
                ss << "XOrigin\tYOrigin\tArea\tIx\tIy\tIxy\n";
                ss << m_xOrigin << "\t" << m_yOrigin << "\t" << m_area <<
                                 "\t" << m_Ix << "\t" << m_Iy << "\t" << m_Ixy <<
                                 "\n";
                ss << "Time\tXP\tYP\tZP\tFX1\tFY1\tFZ1\tTX1\tTY1\tTZ1\tFXS1\tFYS1\tFZS1\tTXS1\tTYS1\tTZS1\tMinStress\tMaxStress\tLowPassMinStress\tLowPassMaxStress\n";
            }
            else if (m_stressCalculationType == spring)
            {
                ss << "XOrigin\tYOrigin\tArea\tIx\tIy\tIxy\n";
                ss << m_xOrigin << "\t" << m_yOrigin << "\t" << m_area <<
                                 "\t" << m_Ix << "\t" << m_Iy << "\t" << m_Ixy <<
                                 "\n";
                ss << "Time\tXP\tYP\tZP\tFX1\tFY1\tFZ1\tTX1\tTY1\tTZ1\tFXS1\tFYS1\tFZS1\tTXSA1\tTYSA1\tTZSA1\tTorqueScalar\tMinStress\tMaxStress\tLowPassMinStress\tLowPassMaxStress\n";
            }
        }
    }
    if (m_stiffness.size() == 0)
    {
        const double *p;
        if (this->GetBody1()) p = dBodyGetPosition(this->GetBody1()->GetBodyID());
        else p = dBodyGetPosition(this->GetBody2()->GetBodyID());

        ss << simulation()->GetTime() << "\t" << p[0] << "\t" << p[1] << "\t" << p[2] << "\t" <<
              JointFeedback()->f1[0] << "\t" << JointFeedback()->f1[1] << "\t" << JointFeedback()->f1[2] << "\t" <<
              JointFeedback()->t1[0] << "\t" << JointFeedback()->t1[1] << "\t" << JointFeedback()->t1[2] << "\t" <<
              JointFeedback()->f2[0] << "\t" << JointFeedback()->f2[1] << "\t" << JointFeedback()->f2[2] << "\t" <<
              JointFeedback()->t2[0] << "\t" << JointFeedback()->t2[1] << "\t" << JointFeedback()->t2[2] << "\t" <<
              "\n";
    }
    else
    {
        if (m_stressCalculationType == beam)
        {
            dVector3 r;
            if (this->GetBody1()) dBodyGetRelPointPos (this->GetBody1()->GetBodyID(), m_StressOrigin.x, m_StressOrigin.y, m_StressOrigin.z, r);
            else dBodyGetRelPointPos (this->GetBody2()->GetBodyID(), m_StressOrigin.x, m_StressOrigin.y, m_StressOrigin.z, r);
            ss << simulation()->GetTime() << "\t" << r[0] << "\t" << r[1] << "\t" << r[2] << "\t" <<
                  JointFeedback()->f1[0] << "\t" << JointFeedback()->f1[1] << "\t" << JointFeedback()->f1[2] << "\t" <<
                  JointFeedback()->t1[0] << "\t" << JointFeedback()->t1[1] << "\t" << JointFeedback()->t1[2] << "\t" <<
                  m_forceStressCoords.x << "\t" << m_forceStressCoords.y << "\t" << m_forceStressCoords.z << "\t" <<
                  m_torqueStressCoords.x << "\t" << m_torqueStressCoords.y << "\t" << m_torqueStressCoords.z << "\t" <<
                  m_minStress << "\t" << m_maxStress << "\t" << m_lowPassMinStress << "\t" << m_lowPassMaxStress << "\t" <<
                  "\n";
        }
        else if (m_stressCalculationType == spring)
        {
            dVector3 r;
            if (this->GetBody1()) dBodyGetRelPointPos (this->GetBody1()->GetBodyID(), m_StressOrigin.x, m_StressOrigin.y, m_StressOrigin.z, r);
            else dBodyGetRelPointPos (this->GetBody2()->GetBodyID(), m_StressOrigin.x, m_StressOrigin.y, m_StressOrigin.z, r);
            ss << simulation()->GetTime() << "\t" << r[0] << "\t" << r[1] << "\t" << r[2] << "\t" <<
                  JointFeedback()->f1[0] << "\t" << JointFeedback()->f1[1] << "\t" << JointFeedback()->f1[2] << "\t" <<
                  JointFeedback()->t1[0] << "\t" << JointFeedback()->t1[1] << "\t" << JointFeedback()->t1[2] << "\t" <<
                  m_forceStressCoords.x << "\t" << m_forceStressCoords.y << "\t" << m_forceStressCoords.z << "\t" <<
                  m_torqueAxis.x << "\t" << m_torqueAxis.y << "\t" << m_torqueAxis.z << "\t" <<
                  m_torqueScalar << "\t" << m_minStress << "\t" << m_maxStress << "\t" << m_lowPassMinStress << "\t" << m_lowPassMaxStress << "\t" <<
                  "\n";
        }
    }
    return ss.str();
}

void FixedJoint::CalculatePixmap()
{
    if (!m_colourMap.size())
    {
        Colour mappedColour;
        m_colourMap.reserve(256 * 4);
        for (size_t i = 0; i < 256; i++)
        {
            float r = ((float)i / 255.0f);
            Colour::SetColourFromMap(r, Colour::JetColourMap, &mappedColour, false);
            m_colourMap.push_back(static_cast<unsigned char>(255.0 * mappedColour.r()));
            m_colourMap.push_back(static_cast<unsigned char>(255.0 * mappedColour.g()));
            m_colourMap.push_back(static_cast<unsigned char>(255.0 * mappedColour.b()));
            m_colourMap.push_back(static_cast<unsigned char>(255.0 * mappedColour.alpha()));
        }
    }

    if (!m_pixMap.size())
    {
        m_pixMap.resize(m_nx * m_ny * 4);
    }

    if (m_lastDisplayTime != simulation()->GetTime())
    {
        m_lastDisplayTime = simulation()->GetTime();
        size_t backgroundColourIndex4 = 0;
        size_t foregroundColourIndex4 = 255 * 4;
        unsigned char *stiffnessPtr = m_stiffness.data();
        size_t i = 0;
        if (simulation()->GetTime() <= 0) // set texture from stiffness bitmap (0 or 1)
        {
            for (size_t iy = 0; iy < m_ny; iy++)
            {
                for (size_t ix = 0; ix < m_nx; ix++)
                {
                    if (*stiffnessPtr++)
                    {
                        m_pixMap[i++] = m_colourMap[foregroundColourIndex4];
                        m_pixMap[i++] = m_colourMap[foregroundColourIndex4 + 1];
                        m_pixMap[i++] = m_colourMap[foregroundColourIndex4 + 2];
                        m_pixMap[i++] = m_colourMap[foregroundColourIndex4 + 3];
                    }
                    else
                    {
                        m_pixMap[i++] = m_colourMap[backgroundColourIndex4];
                        m_pixMap[i++] = m_colourMap[backgroundColourIndex4 + 1];
                        m_pixMap[i++] = m_colourMap[backgroundColourIndex4 + 2];
                        m_pixMap[i++] = m_colourMap[backgroundColourIndex4 + 3];
                    }
                }
            }
        }
        else
        {
            double *stressPtr = m_stress.data();
            size_t filteredStressIndex = 0;
            double v;
            for (size_t iy = 0; iy < m_ny; iy++)
            {
                for (size_t ix = 0; ix < m_nx; ix++)
                {
                    if (*stiffnessPtr++)
                    {
                        switch (m_lowPassType)
                        {
                        case NoLowPass:
                            if (m_lowRange != m_highRange)
                            {
                                v = (*stressPtr - m_lowRange) / (m_highRange - m_lowRange);
                            }
                            else
                            {
                                if (m_minStress != m_maxStress)
                                    v = (*stressPtr - m_minStress) / (m_maxStress - m_minStress);
                                else
                                    v = 0;
                            }
                            stressPtr++;
                            break;

                        case MovingAverageLowPass:
                        case Butterworth2ndOrderLowPass:
                            if (m_lowRange != m_highRange)
                            {
                                v = (m_filteredStress[filteredStressIndex]->Output() - m_lowRange) / (m_highRange - m_lowRange);
                            }
                            else
                            {
                                if (m_minStress != m_maxStress)
                                    v = (m_filteredStress[filteredStressIndex]->Output() - m_minStress) / (m_maxStress - m_minStress);
                                else
                                    v = 0;
                            }
                            filteredStressIndex++;
                            break;
                        }

                        int idx = (int)(255.0 * v) * 4;
                        m_pixMap[i++] = m_colourMap[idx++];
                        m_pixMap[i++] = m_colourMap[idx++];
                        m_pixMap[i++] = m_colourMap[idx++];
                        m_pixMap[i++] = m_colourMap[idx++];
                    }
                    else
                    {
                        m_pixMap[i++] = m_colourMap[backgroundColourIndex4];
                        m_pixMap[i++] = m_colourMap[backgroundColourIndex4 + 1];
                        m_pixMap[i++] = m_colourMap[backgroundColourIndex4 + 2];
                        m_pixMap[i++] = m_colourMap[backgroundColourIndex4 + 3];
                    }
                }
            }
        }
    }
}

bool FixedJoint::CalculatePixmapNeeded()
{
    if (m_stressCalculationType == none || !simulation() || m_lastDisplayTime == simulation()->GetTime()) return false;
    return true;
}


std::vector<unsigned char> FixedJoint::AsciiToBitMap(const std::string &buffer, size_t width, size_t height, char setChar, bool reverseY)
{
    std::vector<unsigned char> bitmap(width * height);
    size_t bufferIndex = 0;
    size_t bitmapIndex = 0;
    if (reverseY == false)
    {
        while (bufferIndex < buffer.size() && bitmapIndex < bitmap.size())
        {
            if (buffer[bufferIndex] > 32)
            {
                if (buffer[bufferIndex] == setChar) bitmap[bitmapIndex] = 1;
                bitmapIndex++;
            }
            bufferIndex++;
        }
    }
    else
    {
        bitmapIndex = width * (height - 1);
        size_t j = 0;
        while (bufferIndex < buffer.size() && bitmapIndex < bitmap.size())
        {
            if (buffer[bufferIndex] > 32)
            {
                if (buffer[bufferIndex] == setChar) bitmap[bitmapIndex] = 1;
                bitmapIndex++;
                j++;
                if (j >= width)
                {
                    j = 0;
                    bitmapIndex -= (2 * width);
                }
            }
            bufferIndex++;
        }
    }
    return bitmap;
}

bool FixedJoint::lateFix() const
{
    return m_lateFix;
}

void FixedJoint::setLateFix(bool lateFix)
{
    m_lateFix = lateFix;
}



