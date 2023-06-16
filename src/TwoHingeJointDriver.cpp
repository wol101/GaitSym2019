/*
 *  TwoHingeJointDriver.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 14/06/2022.
 *  Copyright 2020 Bill Sellers. All rights reserved.
 *
 */

#include "TwoHingeJointDriver.h"
#include "Simulation.h"
#include "HingeJoint.h"
#include "UniversalJoint.h"
#include "BallJoint.h"
#include "Marker.h"
#include "GSUtil.h"
#include "PGDMath.h"
#include "Muscle.h"
#include "Body.h"
#include "TwoPointStrap.h"
#include "TwoCylinderWrapStrap.h"
#include "CylinderWrapStrap.h"
#include "NPointStrap.h"
#include "PIDMuscleLengthController.h"

#include "pystring.h"

#include <cassert>
#include <algorithm>
#include <utility>
#include <limits>

using namespace std::string_literals;

TwoHingeJointDriver::TwoHingeJointDriver()
{
}

void TwoHingeJointDriver::Update()
{
    assert(simulation()->GetStepCount() == lastStepCount() + 1);
    setLastStepCount(simulation()->GetStepCount());

    // set the desired distance
    pgd::Vector3 proximalJointPositionWorld = m_proximalJoint->body1Marker()->GetWorldPosition();
    pgd::Vector3 targetPositionWorld = m_targetMarker->GetWorldPosition();
    m_desiredLength = (targetPositionWorld - proximalJointPositionWorld).Magnitude();

    // now find the zero of the CalculateLengthDifference to get the angle fraction that achieves this length
    m_angleFraction = GSUtil::zeroin(0, 1, &CalculateLengthDifference, this, m_tolerance);

    // that sorts out the angles on the distal joints- lets see where that takes us
#ifndef NDEBUG
    double testAngleFraction = m_angleFraction;
    CalculateLength(m_angleFraction); // not mormally needed because zeroin will have called this with the returned angleFraction as its last operation
    if (std::abs(testAngleFraction - m_angleFraction) >= std::numeric_limits<double>::epsilon())
        std::cerr << "Warning zeroin calculated m_angleFraction does not match\n";
#endif

    pgd::Vector3 targetVector = m_targetMarker->GetPosition() - m_proximalJoint->body1Marker()->GetPosition(); // these will both be on the same body
    // need to find the rotations about m_proximalJoint->body1Marker()->GetAxis(Marker::X) and m_proximalJoint->body1Marker()->GetAxis(Marker::Y)
    // that rotate m_distalBodyMarkerPositionWRTProxJoint to targetVector
    targetVector.Normalize();
    pgd::Vector3 startVector = m_distalBodyMarkerPositionWRTProxJoint;
    startVector.Normalize();

    // start by projecting the two vectors onto a plane defined by the first axis as a normal
    // from https://www.euclideanspace.com/maths/geometry/elements/plane/lineOnPlane/index.htm
    // using the formula B cross (A cross B / |B|) / |B|
    // where A is the vector and B is the normal of the plane
    // for completeness this distance along the normal is
    // A dot B * B/|B|**2
    // however we can use some optimisation because we just want the direction in the plane which is
    // B cross (A cross B)
    // which we then normalise anyway
    pgd::Vector3 normal1 = m_proximalJoint->body1Marker()->GetAxis(Marker::X);
    pgd::Vector3 startDirection1 = normal1 ^ (startVector ^ normal1);
    startDirection1.Normalize();
    pgd::Vector3 endDirection1 = normal1 ^ (targetVector ^ normal1);
    endDirection1.Normalize();
    // now we can find the angle in the plane using
    // dot = x1*x2 + y1*y2 + z1*z2
    // det = x1*y2*zn + x2*yn*z1 + xn*y1*z2 - z1*y2*xn - z2*yn*x1 - zn*y1*x2
    // angle = atan2(det, dot)
    // [alternative for det is det = n dot (v1 cross v2)]
    double dot = startDirection1 * endDirection1;
    double det = normal1 * (startDirection1 ^ endDirection1);
    double angle1 = atan2(det, dot);
    pgd::Quaternion q1 = pgd::MakeQFromAxisAngle(normal1, angle1);

    // now work on the second rotation
    pgd::Vector3 rotatedStartVector = pgd::QVRotate(q1, startVector);
#ifdef USE_UNROTATED_SECOND_AXIS
    pgd::Vector3 normal2 =m_proximalJoint->body1Marker()->GetAxis(Marker::Y);
#else // using the rotated seconds axis is what is required to mimic a universal joint
    pgd::Vector3 normal2 = pgd::QVRotate(q1, m_proximalJoint->body1Marker()->GetAxis(Marker::Y));
#endif
    pgd::Vector3 startDirection2 = normal2 ^ (rotatedStartVector ^ normal2);
    startDirection2.Normalize();
    pgd::Vector3 endDirection2 = normal2 ^ (targetVector ^ normal2);
    endDirection2.Normalize();
    dot = startDirection2 * endDirection2;
    det = normal2 * (startDirection2 ^ endDirection2);
    double angle2 = atan2(det, dot);

#ifdef CHECK_VECTOR_MATH
    // check what we get by rotating the intial vector by the two quaternions
    pgd::Quaternion q2 = pgd::MakeQFromAxisAngle(normal2, angle2);
    pgd::Vector3 checkVector = pgd::QVRotate(q2, pgd::QVRotate(q1, startVector));
    std::cerr << "targetVector = " << targetVector << "\n";
    std::cerr << "checkVector = " << checkVector << "\n";
    std::cerr << "difference = " << targetVector - checkVector << "\n";
#endif

    while (true)
    {
        HingeJoint *hingeJoint = dynamic_cast<HingeJoint *>(m_proximalJoint);
        if (hingeJoint)
        {
            m_proximalJointAxis1 = normal1;
            m_proximalJointAngle1 = -angle1; // note that the angle is negated because ODE calculates hinge joint angle wrt body 2 and this is a rotation wrt body 1
            m_proximalAngleFraction1 = (m_proximalJointAngle1 - m_proximalJointRange[0]) / (m_proximalJointRange[1] - m_proximalJointRange[0]);
            if (m_proximalAngleFraction1 < 0 || m_proximalAngleFraction1 > 1)
            {
                m_proximalAngleFraction1 = GSUtil::Clamp(m_proximalAngleFraction1, 0.0, 1.0);
                m_proximalJointAngle1 = m_proximalAngleFraction1 * (m_proximalJointRange[1] - m_proximalJointRange[0]) + m_proximalJointRange[0];
            }
            m_proximalJointRotation = pgd::MakeQFromAxisAngle(m_proximalJointAxis1, -m_proximalJointAngle1); // note that the angle is negated because ODE calculates hinge joint angle wrt body 2 and this is a rotation wrt body 1
            break;
        }
        UniversalJoint *universalJoint = dynamic_cast<UniversalJoint *>(m_proximalJoint);
        if (universalJoint)
        {
            m_proximalJointAxis1 = normal1;
            m_proximalJointAngle1 = -angle1; // note that the angle is negated because ODE calculates hinge joint angle wrt body 2 and this is a rotation wrt body 1
            m_proximalAngleFraction1 = (m_proximalJointAngle1 - m_proximalJointRange[0]) / (m_proximalJointRange[1] - m_proximalJointRange[0]);
            if (m_proximalAngleFraction1 < 0 || m_proximalAngleFraction1 > 1)
            {
                m_proximalAngleFraction1 = GSUtil::Clamp(m_proximalAngleFraction1, 0.0, 1.0);
                m_proximalJointAngle1 = m_proximalAngleFraction1 * (m_proximalJointRange[1] - m_proximalJointRange[0]) + m_proximalJointRange[0];
            }
            m_proximalJointRotation = pgd::MakeQFromAxisAngle(m_proximalJointAxis1, -m_proximalJointAngle1); // note that the angle is negated again to put it back to the correct sign
            m_proximalJointAxis2 = normal2;
            m_proximalJointAngle2 = angle2;
            m_proximalJointRotation =  pgd::MakeQFromAxisAngle(m_proximalJointAxis2, m_proximalJointAngle2) * m_proximalJointRotation;
            break;
        }
        BallJoint *ballJoint = dynamic_cast<BallJoint *>(m_proximalJoint);
        if (ballJoint)
        {
            m_proximalJointAxis1 = normal1;
            m_proximalJointAngle1 = -angle1; // note that the angle is negated because ODE calculates hinge joint angle wrt body 2 and this is a rotation wrt body 1
            m_proximalAngleFraction1 = (m_proximalJointAngle1 - m_proximalJointRange[0]) / (m_proximalJointRange[1] - m_proximalJointRange[0]);
            if (m_proximalAngleFraction1 < 0 || m_proximalAngleFraction1 > 1)
            {
                m_proximalAngleFraction1 = GSUtil::Clamp(m_proximalAngleFraction1, 0.0, 1.0);
                m_proximalJointAngle1 = m_proximalAngleFraction1 * (m_proximalJointRange[1] - m_proximalJointRange[0]) + m_proximalJointRange[0];
            }
            m_proximalJointRotation = pgd::MakeQFromAxisAngle(m_proximalJointAxis1, -m_proximalJointAngle1); // note that the angle is negated again to put it back to the correct sign
            m_proximalJointAxis2 = normal2;
            m_proximalJointAngle2 = angle2;
            m_proximalJointRotation =  pgd::MakeQFromAxisAngle(m_proximalJointAxis2, m_proximalJointAngle2) * m_proximalJointRotation;
            break;
        }
        std::cerr << "TwoHingeJointDriver::Update(): unrecognised proximal joint type\n";
        break;
    }

//    pgd::Vector3 targetVector = m_targetMarker->GetPosition() - m_proximalJoint->body1Marker()->GetPosition(); // these will both be on the same body
//    pgd::Quaternion targetRotation = pgd::FindRotation(m_distalBodyMarkerPositionWRTProxJoint, targetVector);
//#if 1
//    // need to decompose the rotation into the roations about the first 2 axes of the joint marker
//    m_proximalJointAxis1 = m_proximalJoint->body1Marker()->GetAxis(Marker::X);
//    pgd::Quaternion swing, twist;
//    pgd::SwingTwistDecomposition(targetRotation, m_proximalJointAxis1, &swing, &twist);
//    pgd::Vector3 twistAxis;
//    pgd::MakeAxisAngleFromQ(twist, &twistAxis.x, &twistAxis.y, &twistAxis.z, &m_proximalJointAngle1);
//    if (pgd::Dot(twistAxis, m_proximalJointAxis1) < 0)
//        m_proximalJointAngle1 = -m_proximalJointAngle1; // MakeAxisAngleFromQ can reverse the axis direction
//    // swing is the remaining rotation and we need to resolve this around the second axis
//    // in fact swing should be around an axis that is parallel or antiparallel to Marker::Y
//    pgd::Vector3 swingAxis;
//    double swingAngle;
//    pgd::MakeAxisAngleFromQ(swing, &swingAxis.x, &swingAxis.y, &swingAxis.z, &swingAngle);
//    m_proximalJointAxis2 = m_proximalJoint->body1Marker()->GetAxis(Marker::Y);
//    pgd::SwingTwistDecomposition(swing, m_proximalJointAxis2, &swing, &twist);
//    pgd::MakeAxisAngleFromQ(twist, &twistAxis.x, &twistAxis.y, &twistAxis.z, &m_proximalJointAngle2);
//    if (pgd::Dot(twistAxis, m_proximalJointAxis2) < 0)
//        m_proximalJointAngle2 = -m_proximalJointAngle2; // MakeAxisAngleFromQ can reverse the axis direction
//    // can now assemble the target rotation
//    while (true)
//    {
//        HingeJoint *hingeJoint = dynamic_cast<HingeJoint *>(m_proximalJoint);
//        if (hingeJoint)
//        {
//            m_proximalJointAngle1 = -m_proximalJointAngle1;
//            m_proximalAngleFraction1 = (m_proximalJointAngle1 - m_proximalJointRange[0]) / (m_proximalJointRange[1] - m_proximalJointRange[0]);
//            if (m_proximalAngleFraction1 < 0 || m_proximalAngleFraction1 > 1)
//            {
//                m_proximalAngleFraction1 = GSUtil::Clamp(m_proximalAngleFraction1, 0.0, 1.0);
//                m_proximalJointAngle1 = m_proximalAngleFraction1 * (m_proximalJointRange[1] - m_proximalJointRange[0]) + m_proximalJointRange[0];
//            }
//            m_proximalJointRotation = pgd::MakeQFromAxisAngle(m_proximalJointAxis1, -m_proximalJointAngle1); // note that the angle is negated because ODE calculates hinge joint angle wrt body 2 and this is a rotation wrt body 1
//            break;
//        }
//        UniversalJoint *universalJoint = dynamic_cast<UniversalJoint *>(m_proximalJoint);
//        if (universalJoint)
//        {
//            m_proximalJointAngle1 = -m_proximalJointAngle1; // note that the angle is negated because ODE calculates hinge joint angle wrt body 2 and this is a rotation wrt body 1
//            m_proximalAngleFraction1 = (m_proximalJointAngle1 - m_proximalJointRange[0]) / (m_proximalJointRange[1] - m_proximalJointRange[0]);
//            if (m_proximalAngleFraction1 < 0 || m_proximalAngleFraction1 > 1)
//            {
//                m_proximalAngleFraction1 = GSUtil::Clamp(m_proximalAngleFraction1, 0.0, 1.0);
//                m_proximalJointAngle1 = m_proximalAngleFraction1 * (m_proximalJointRange[1] - m_proximalJointRange[0]) + m_proximalJointRange[0];
//            }
//            m_proximalJointRotation = pgd::MakeQFromAxisAngle(m_proximalJointAxis1, -m_proximalJointAngle1); // note that the angle is negated again to put it back to the correct sign
//            m_proximalJointRotation =  pgd::MakeQFromAxisAngle(m_proximalJointAxis2, m_proximalJointAngle2) * m_proximalJointRotation;
//            break;
//        }
//        BallJoint *ballJoint = dynamic_cast<BallJoint *>(m_proximalJoint);
//        if (ballJoint)
//        {
//            m_proximalJointAngle1 = -m_proximalJointAngle1; // note that the angle is negated because ODE calculates hinge joint angle wrt body 2 and this is a rotation wrt body 1
//            m_proximalAngleFraction1 = (m_proximalJointAngle1 - m_proximalJointRange[0]) / (m_proximalJointRange[1] - m_proximalJointRange[0]);
//            if (m_proximalAngleFraction1 < 0 || m_proximalAngleFraction1 > 1)
//            {
//                m_proximalAngleFraction1 = GSUtil::Clamp(m_proximalAngleFraction1, 0.0, 1.0);
//                m_proximalJointAngle1 = m_proximalAngleFraction1 * (m_proximalJointRange[1] - m_proximalJointRange[0]) + m_proximalJointRange[0];
//            }
//            m_proximalJointRotation = pgd::MakeQFromAxisAngle(m_proximalJointAxis1, -m_proximalJointAngle1); // note that the angle is negated again to put it back to the correct sign
//            m_proximalJointRotation =  pgd::MakeQFromAxisAngle(m_proximalJointAxis2, m_proximalJointAngle2) * m_proximalJointRotation;
//            break;
//        }
//        std::cerr << "TwoHingeJointDriver::Update(): unrecognised proximal joint type\n";
//        break;
//    }
//#else
//    HingeJoint *hingeJoint = dynamic_cast<HingeJoint *>(m_proximalJoint);
//    if (hingeJoint)
//    {
//        m_proximalJointAxis = m_proximalJoint->body1Marker()->GetAxis(Marker::X);
//        pgd::Quaternion swing, twist;
//        pgd::SwingTwistDecomposition(targetRotation, m_proximalJointAxis, &swing, &twist);
//        // an alternative would be to create 2 planes: 1 m_proximalJointAxis & targetVector; 2 m_proximalJointAxis & m_distalBodyMarkerPositionWRTProxJoint
//        // you can then find the normals to the planes and the angle between the normals would be the twist angle directly
//        pgd::Vector3 twistAxis;
//        pgd::MakeAxisAngleFromQ(twist, &twistAxis.x, &twistAxis.y, &twistAxis.z, &m_proximalJointAngle);
//        if (pgd::Dot(twistAxis, m_proximalJointAxis) < 0)
//            m_proximalJointAngle = -m_proximalJointAngle; // MakeAxisAngleFromQ can reverse the axis direction
//        // m_proximalJointAngle is currently wrt body 1 (which I prefer) but the joint ranges are in ODE convention
//        // need to negate so it is consistent with m_intermediateJointAngle and m_distalJointAngle
//        m_proximalJointAngle = -m_proximalJointAngle;
//        m_proximalAngleFraction = (m_proximalJointAngle - m_proximalJointRange[0]) / (m_proximalJointRange[1] - m_proximalJointRange[0]);
//        if (m_proximalAngleFraction < 0 || m_proximalAngleFraction > 1)
//        {
//            m_proximalAngleFraction = GSUtil::Clamp(m_proximalAngleFraction, 0.0, 1.0);
//            m_proximalJointAngle = m_proximalAngleFraction * (m_proximalJointRange[1] - m_proximalJointRange[0]) + m_proximalJointRange[0];
//        }
//        m_proximalJointRotation = pgd::MakeQFromAxisAngle(m_proximalJointAxis, -m_proximalJointAngle); // note that the angle is negated because ODE calculates hinge joint angle wrt body 2 and this is a rotation wrt body 1
//    }
//    UniversalJoint *universalJoint = dynamic_cast<UniversalJoint *>(m_proximalJoint);
//    if (universalJoint)
//    {
//        pgd::Vector3 eulerAngles = GetEulerAngles(*m_proximalJoint, *m_proximalJoint->body1Marker(), false);
//        pgd::Vector3 axis0, axis1, axis2;
//        m_proximalJoint->body1Marker()->GetBasis(&axis0, &axis1, &axis2);
//        m_proximalJointRotation = pgd::MakeQFromAxisAngle(axis1, eulerAngles[1]) * pgd::MakeQFromAxisAngle(axis0, eulerAngles[0]);
//    }
//    BallJoint *ballJoint = dynamic_cast<BallJoint *>(m_proximalJoint);
//    if (ballJoint)
//    {
//        // might want to do this if I am worried about joint limits
//#ifdef USE_EULER_JOINT_LIMITS
//        pgd::Vector3 eulerAngles = GetEulerAngles(*m_proximalJoint, *m_proximalJoint->body1Marker(), false);
//        pgd::Vector3 axis0, axis1, axis2;
//        m_proximalJoint->body1Marker()->GetBasis(&axis0, &axis1, &axis2);
//        m_proximalJointRotation = pgd::MakeQFromAxisAngle(axis2, eulerAngles[2]) * pgd::MakeQFromAxisAngle(axis1, eulerAngles[1]) * pgd::MakeQFromAxisAngle(axis0, eulerAngles[0]);
//#else
//        m_proximalJointRotation = targetRotation;
//#endif
//    }
//#endif

    // now assemble a fake limb that uses the calculated angles
    m_proximalBody->SetQuaternion(m_proximalJointRotation.n, m_proximalJointRotation.x, m_proximalJointRotation.y, m_proximalJointRotation.z);
    pgd::Quaternion qDistalBody = m_distalJointRotation * m_proximalJointRotation;
    m_distalBody->SetQuaternion(qDistalBody.n, qDistalBody.x, qDistalBody.y, qDistalBody.z);

    // and move them around so the markers line up
    pgd::Vector3 negDelta = m_proximalJointMarker1->GetWorldPosition() - m_proximalJointMarker2->GetWorldPosition();
    m_proximalBody->SetPositionDelta(negDelta.x, negDelta.y, negDelta.z);
    negDelta = m_distalJointMarker1->GetWorldPosition() - m_distalJointMarker2->GetWorldPosition();
    m_distalBody->SetPositionDelta(negDelta.x, negDelta.y, negDelta.z);

    // and calculate all the straps
    for (auto &&it : m_localStrapList)
    {
        it.second->Calculate();
//        std::cerr << it.first << " " << it.second->Length() << "\n";
    }
}

pgd::Vector3 TwoHingeJointDriver::GetEulerAngles(const Joint &joint, const Marker &basisMarker, bool reverseBodyOrderInCalculations)
{
    // returns the Euler angles using marker axes as the basis

    // Theory:
    // Since the axes form an orthonormal basis A (each axis is a column of the matrix),
    // you can decompose a rotation R on the three axes by transforming R into the
    // basis A and then do Euler Angle decomposition on the three main axes:
    // R = A*R'*A^t = A*X*Y*Z*A^t = (A*X*A^t)*(A*Y*A^t)*(A*Z*A^t)
    // This translates into the following algorithm:
    // Compute R' = A^t*R*A
    // Decompose R' into Euler Angles around main axes to obtain angles X, Y, Z
    // convert these into their matrix rotations
    // Compute the three rotations around the given axes:
    //    X' = A*X*A^t
    //    Y' = A*Y*A^t
    //    Z' = A*Y*A^t

    pgd::Quaternion body1MarkerWorld = joint.body1Marker()->GetWorldQuaternion();
    pgd::Quaternion body2MarkerWorld = joint.body2Marker()->GetWorldQuaternion();
    pgd::Matrix3x3 R;
    if (reverseBodyOrderInCalculations)
    {
        // angle is calculated with respect to body 1
        pgd::Quaternion body1ToBody2 = body2MarkerWorld * (~body1MarkerWorld);
        R = pgd::Matrix3x3(body1ToBody2);
    }
    else
    {
        // angle is calculated with respect to body 2
        pgd::Quaternion body2ToBody1 = body1MarkerWorld * (~body2MarkerWorld);
        R = pgd::Matrix3x3(body2ToBody1);
    }
    pgd::Matrix3x3 A = pgd::MakeMFromQ(basisMarker.GetWorldQuaternion());
    pgd::Matrix3x3 At = A.Transpose();
    pgd::Matrix3x3 Rp = At * R * A;
    pgd::Vector3 euler = pgd::MakeEulerAnglesFromQRadian(pgd::MakeQfromM(Rp));
    pgd::Matrix3x3 Xp = A * pgd::MakeMFromQ(pgd::MakeQFromAxisAngle(1, 0, 0, (euler.x), true)) * At;
    pgd::Matrix3x3 Yp = A * pgd::MakeMFromQ(pgd::MakeQFromAxisAngle(0, 1, 0, (euler.y), true)) * At;
    pgd::Matrix3x3 Zp = A * pgd::MakeMFromQ(pgd::MakeQFromAxisAngle(0, 0, 1, (euler.z), true)) * At;
    pgd::Quaternion Xpq = pgd::MakeQfromM(Xp);
    pgd::Quaternion Ypq = pgd::MakeQfromM(Yp);
    pgd::Quaternion Zpq = pgd::MakeQfromM(Zp);

    // we can't just convert the quaternions into axis angle to get the angles because the axes might be reversed, or might be arbitrary if close to zero
    double xMag = Xpq.GetVector().Magnitude();
    double yMag = Ypq.GetVector().Magnitude();
    double zMag = Zpq.GetVector().Magnitude();
    double dotX = 0, dotY = 0, dotZ = 0;
    pgd::Vector3 axisX, axisY, axisZ;
    double angle0 = 0, angle1 = 0, angle2 = 0;
    const double epsilon = std::numeric_limits<double>::epsilon();
    if (xMag < epsilon || Xpq.n >= 1 || Xpq.n <= -1)
    {
        angle0 = 0;
    }
    else
    {
        angle0 = 2 * std::acos(Xpq.n); // acos produces a value from 0 to pi, so this angle is 0 to 2*pi
        if (angle0 > M_PI) angle0 -= 2 * M_PI; // so this means the angle is restricted to the more usual -pi to pi
        axisX = Xpq.GetVector() / xMag;
        dotX =pgd::Dot(axisX, pgd::Vector3(A.e11, A.e21, A.e31));
        if (dotX < 0)
            angle0 = - angle0;
    }
    if (yMag < epsilon || Ypq.n >= 1 || Ypq.n <= -1)
    {
        angle1 = 0;
    }
    else
    {
        angle1 = 2 * std::acos(Ypq.n);
        if (angle1 > M_PI) angle1 -= 2 * M_PI;
        axisY = Ypq.GetVector() / yMag;
        dotY =pgd::Dot(axisY, pgd::Vector3(A.e11, A.e21, A.e31));
        if (dotY < 0)
            angle1 = - angle1;
    }
    if (zMag < epsilon || Zpq.n >= 1 || Zpq.n <= -1)
    {
        angle2 = 0;
    }
    else
    {
        angle2 = 2 * std::acos(Zpq.n);
        if (angle2 > M_PI) angle2 -= 2 * M_PI;
        axisZ = Zpq.GetVector() / zMag;
        dotZ =pgd::Dot(axisZ, pgd::Vector3(A.e11, A.e21, A.e31));
        if (dotZ < 0)
            angle2 = - angle2;
    }
    return pgd::Vector3(angle0, angle1, angle2);
}

void TwoHingeJointDriver::SendData()
{
    // and set the required strap lengths
    for (auto &&it : *targetList())
    {
        PIDMuscleLengthController *pidMuscleLengthController = dynamic_cast<PIDMuscleLengthController *>(it.second);
        if (!pidMuscleLengthController) continue;
        Strap *remoteStrap = pidMuscleLengthController->muscle()->GetStrap();
        auto localStrap = m_localStrapList.find(remoteStrap->name());
        if (localStrap == m_localStrapList.end()) continue;
        double length = localStrap->second->Length();
        // now set the target length
        pidMuscleLengthController->ReceiveData(Clamp(length), simulation()->GetStepCount());
    }
}


// this funtion calculates the distance from the proximal joint to the distal Body Marker
// dependent on the fraction of the joint limits for the interconnecting hinge joints
void TwoHingeJointDriver::CalculateLength(double angleFraction)
{
    // now calculate the rotations at the joints in a consistent coordinate frame (and this can be the local frame because at contruction nothing is rotated)
    m_distalJointAngle = angleFraction * (m_distalJointRange[1] - m_distalJointRange[0]) + m_distalJointRange[0];
    m_distalJointAxis = m_distalJoint->body1Marker()->GetAxis(Marker::X);
    m_distalJointRotation = pgd::MakeQFromAxisAngle(m_distalJointAxis, -m_distalJointAngle); // note that the angle is negated because ODE calculates hinge joint angle wrt body 2 and this is a rotation wrt body 1

    // now sum the vectors to get the position of the end point
    m_distalBodyMarkerPositionWRTProxJoint = m_proximalBodyVector + pgd::QVRotate(m_distalJointRotation, m_distalBodyVector);
    m_actualLength = m_distalBodyMarkerPositionWRTProxJoint.Magnitude();
//    std::cerr << "angleFraction " << angleFraction << " actualLength " << m_actualLength << "\n";
}

// this funtion calculates the difference between the desired length and the length calculated from the angle fraction
double TwoHingeJointDriver::CalculateLengthDifference(double angleFraction, void *data)
{
    TwoHingeJointDriver *twoHingeJointController = static_cast<TwoHingeJointDriver *>(data);
    twoHingeJointController->CalculateLength(angleFraction);
    double lengthError = twoHingeJointController->actualLength() - twoHingeJointController->desiredLength();
    return lengthError;
}


Marker *TwoHingeJointDriver::createLocalMarkerCopy(const Marker *marker)
{
    auto it = m_localBodyList.find(marker->GetBody()->name());
    if (it == m_localBodyList.end()) return nullptr;
    std::unique_ptr<Marker> localMarker = std::make_unique<Marker>(nullptr);
    Marker *localMarkerPtr = localMarker.get();
    localMarker->setName(marker->name());
    localMarker->SetBody(it->second.get());
    pgd::Vector3 p = marker->GetPosition();
    localMarker->SetPosition(p.x, p.y, p.z);
    pgd::Quaternion q = marker->GetQuaternion();
    localMarker->SetQuaternion(q.n, q.x, q.y, q.z);
    m_localMarkerList[localMarker->name()] = std::move(localMarker);
    return localMarkerPtr;
}

// test for monotonicity
// returns 0 if not monotonic
// returns +1 if increasing monotonically
// returns -1 if decreasing monotonically
// returns +2 if does not vary at all
int TwoHingeJointDriver::monotonicTest(double (*f)(double x, void *info), double a, double b, double eps, void *info)
{
    double x = a;
    double y1 = f(x, info);
    double y2 = f(x + eps, info);
    double d = y2 - y1;
    double s = d < 0 ? -1. : (d > 0 ? 1. : 0.);
    while( x < b )
    {
        x += eps;
        y1 = y2;
        y2 = f(x + eps, info);
        d = y2 - y1;
        if( s == 0. )
        {
            s = d < 0 ? -1. : (d > 0 ? 1. : 0.);
        }
        if( s * d < 0 )
        {
            return 0;
        }
    }
    if (s > 0) return +1;
    if (s < 0) return -1;
    return +2;
}

std::string TwoHingeJointDriver::dumpToString()
{
    std::string s;
    if (firstDump())
    {
        setFirstDump(false);
        if (m_dumpExtensionCurve)
        {
            s += dumpHelper({"AngleFraction", "Length"s});
            for (int i = 0; i < 1001; i++)
            {
                double v = double(i) * 0.001;
                if (i == 1000) v = 1.0; // fix for likely rounding error
                double le = CalculateLengthDifference(v, this);
                s +=  dumpHelper({v, le});
            }
            CalculateLength(m_angleFraction); // neeeded because CalculateLengthDifference changes m_distalJointAngle
        }
        s += dumpHelper({"Time", "MarkerDistance"s, "DesiredLength"s, "AngleFraction"s, "ProximalAngleFraction1"s,
                         "ProximalJointAngle1"s, "ProximalJointAngle2"s, "DistalJointAngle"s,
                         "proximalJointMarker1Position.x"s, "proximalJointMarker1Position.y"s, "proximalJointMarker1Position.z"s,
                         "distalJointMarker1Position.x"s, "distalJointMarker1Position.y"s, "distalJointMarker1Position.z"s,
                         "distalBodyMarkerLocalPosition.x"s, "distalBodyMarkerLocalPosition.y"s, "distalBodyMarkerLocalPosition.z"s});

    }
    pgd::Vector3 m_proximalJointMarker1Position = m_proximalJointMarker1->GetWorldPosition();
    pgd::Vector3 m_distalJointMarker1Position = m_distalJointMarker1->GetWorldPosition();
    pgd::Vector3 m_distalBodyMarkerLocalPosition = m_distalBodyMarkerLocal->GetWorldPosition();
    double markerDistance = (m_distalBodyMarker->GetWorldPosition() - m_proximalJoint->body1Marker()->GetWorldPosition()).Magnitude();
    s += dumpHelper({simulation()->GetTime(), markerDistance, m_desiredLength, m_angleFraction, m_proximalAngleFraction1,
                     m_proximalJointAngle1, m_proximalJointAngle2, m_distalJointAngle,
                     m_proximalJointMarker1Position.x, m_proximalJointMarker1Position.y, m_proximalJointMarker1Position.z,
                     m_distalJointMarker1Position.x, m_distalJointMarker1Position.y, m_distalJointMarker1Position.z,
                     m_distalBodyMarkerLocalPosition.x, m_distalBodyMarkerLocalPosition.y, m_distalBodyMarkerLocalPosition.z});
    return s;
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *TwoHingeJointDriver::createFromAttributes()
{
    if (Driver::createFromAttributes()) return lastErrorPtr();
    std::string buf;
    if (findAttribute("TargetMarkerID"s, &buf) == nullptr) return lastErrorPtr();
    m_targetMarker = simulation()->GetMarker(buf);
    if (!m_targetMarker)
    {
        setLastError("TwoHingeJointDriver ID=\""s + name() + "\" TargetMarkerID marker not found \""s + buf + "\"");
        return lastErrorPtr();
    }
    if (findAttribute("DistalBodyMarkerID"s, &buf) == nullptr) return lastErrorPtr();
    m_distalBodyMarker = simulation()->GetMarker(buf);
    if (!m_distalBodyMarker)
    {
        setLastError("TwoHingeJointDriver ID=\""s + name() + "\" DistalBodyMarkerID marker not found \""s + buf + "\"");
        return lastErrorPtr();
    }
    if (findAttribute("ProximalJointID"s, &buf) == nullptr) return lastErrorPtr();
    m_proximalJoint = dynamic_cast<HingeJoint *>(simulation()->GetJoint(buf));
    if (!m_proximalJoint) m_proximalJoint = dynamic_cast<UniversalJoint *>(simulation()->GetJoint(buf));
    if (!m_proximalJoint) m_proximalJoint = dynamic_cast<BallJoint *>(simulation()->GetJoint(buf));
    if (!m_proximalJoint)
    {
        setLastError("TwoHingeJointDriver ID=\""s + name() + "\" ProximalJointID joint not found or not Hinge, Universal or Ball\""s + buf + "\"");
        return lastErrorPtr();
    }
    if (findAttribute("DistalJointID"s, &buf) == nullptr) return lastErrorPtr();
    m_distalJoint = dynamic_cast<HingeJoint *>(simulation()->GetJoint(buf));
    if (!m_distalJoint)
    {
        setLastError("TwoHingeJointDriver ID=\""s + name() + "\" DistalJointID joint not found \""s + buf + "\"");
        return lastErrorPtr();
    }
    for (auto &&it: *targetList())
    {
        PIDMuscleLengthController *pidMuscleLengthController = dynamic_cast<PIDMuscleLengthController *>(it.second);
        if (!pidMuscleLengthController)
        {
            setLastError("Driver ID=\""s + name() +"\" \" TargetList error: \"" + it.first + "\" not found or not a PIDMuscleLength controller"s);
            return lastErrorPtr();
        }
    }
    if (findAttribute("ProximalJointRange"s, &buf) == nullptr) return lastErrorPtr();
    GSUtil::Double(buf, 2, m_proximalJointRange.data());
    if (findAttribute("DistalJointRange"s, &buf) == nullptr) return lastErrorPtr();
    GSUtil::Double(buf, 2, m_distalJointRange.data());

    if (findAttribute("Tolerance"s, &buf)) m_tolerance = GSUtil::Double(buf);

    // check for consistency
    if (m_proximalJoint->body2Marker()->GetBody() != m_distalJoint->body1Marker()->GetBody())
    {
        setLastError("Driver ID=\""s + name() +"\" proximal joint body2 != distal joint body1"s);
        return lastErrorPtr();
    }
    if (m_distalJoint->body2Marker()->GetBody() != m_distalBodyMarker->GetBody())
    {
        setLastError("Driver ID=\""s + name() +"\" distal joint body2 != distal marker body"s);
        return lastErrorPtr();
    }
    if (m_proximalJoint->body1Marker()->GetBody() != m_targetMarker->GetBody())
    {
        setLastError("Driver ID=\""s + name() +"\" proximal joint body1 != target marker body"s);
        return lastErrorPtr();
    }

    // during contruction the bodies are not rotated, so the body vectors are the contruction vectors
    m_proximalBodyVector = m_distalJoint->body1Marker()->GetPosition() - m_proximalJoint->body2Marker()->GetPosition();
    m_distalBodyVector = m_distalBodyMarker->GetPosition() - m_distalJoint->body2Marker()->GetPosition();

    // we need to find the best ordering for the distal joint ranges
    // so that we get a monotonically increasing function
    double eps = 1.0 / 1000;
    int monotonic = monotonicTest(CalculateLengthDifference, 0.0, 1.0 + eps / 2, eps, this);
    if (monotonic != +1 && monotonic != -1)
    {
        std::string message = "Driver ID=\""s + name() +"\" selected DistalJointRange does not produce a monotonic length change\n"s;
        message += "\" and DistalJointRange=\""s + *GSUtil::ToString(m_distalJointRange, &buf) + "\""s;
        if (monotonic != +1 && monotonic != -1) message += " fails\n"s;
        else message += " succeeds\n"s;
        std::swap(m_distalJointRange.x, m_distalJointRange.y);
        monotonic = monotonicTest(CalculateLengthDifference, 0.0, 1.0 + eps / 2, eps, this);
        message += "\" and DistalJointRange=\""s + *GSUtil::ToString(m_distalJointRange, &buf) + "\""s;
        if (monotonic != +1 && monotonic != -1) message += " fails\n"s;
        else message += " succeeds\n"s;
        setLastError(message);
        return lastErrorPtr();
    }

    // assemble the local copies of bodies, markers and joints
    std::unique_ptr<Body> baseBody = std::make_unique<Body>(nullptr);
    baseBody->setName(m_proximalJoint->body1Marker()->GetBody()->name());
    m_baseBody = baseBody.get();
    std::unique_ptr<Body> proximalBody = std::make_unique<Body>(nullptr);
    proximalBody->setName(m_proximalJoint->body2Marker()->GetBody()->name());
    m_proximalBody = proximalBody.get();
    std::unique_ptr<Body> distalBody = std::make_unique<Body>(nullptr);
    distalBody->setName(m_distalJoint->body2Marker()->GetBody()->name());
    m_distalBody = distalBody.get();
    m_localBodyList[baseBody->name()] = std::move(baseBody);
    m_localBodyList[proximalBody->name()] = std::move(proximalBody);
    m_localBodyList[distalBody->name()] = std::move(distalBody);

    m_proximalJointMarker1 = createLocalMarkerCopy(m_proximalJoint->body1Marker());
    m_proximalJointMarker2 = createLocalMarkerCopy(m_proximalJoint->body2Marker());
    m_distalJointMarker1 = createLocalMarkerCopy(m_distalJoint->body1Marker());
    m_distalJointMarker2 = createLocalMarkerCopy(m_distalJoint->body2Marker());
    m_distalBodyMarkerLocal = createLocalMarkerCopy(m_distalBodyMarker);

    for (auto &&it : *targetList())
    {
        PIDMuscleLengthController *pidMuscleLengthController = dynamic_cast<PIDMuscleLengthController *>(it.second);
        if (!pidMuscleLengthController) continue;
        Strap *strapPtr = pidMuscleLengthController->muscle()->GetStrap();
        TwoPointStrap *twoPointStrap = dynamic_cast<TwoPointStrap *>(strapPtr);
        if (twoPointStrap)
        {
            Marker *originMarker = createLocalMarkerCopy(twoPointStrap->GetOriginMarker());
            Marker *insertionMarker = createLocalMarkerCopy(twoPointStrap->GetInsertionMarker());
            if (!originMarker || !insertionMarker)
            {
                setLastError("Driver ID=\""s + name() +"\" cannot create local strap=\""s + strapPtr->name() + "\""s);
                return lastErrorPtr();
            }
            std::unique_ptr<TwoPointStrap> strap = std::make_unique<TwoPointStrap>();
            strap->setName(strapPtr->name());
            strap->SetOrigin(originMarker);
            strap->SetInsertion(insertionMarker);
            m_localStrapList[strap->name()] = std::move(strap);
            continue;
        }
        NPointStrap *nPointStrap = dynamic_cast<NPointStrap *>(strapPtr);
        if (nPointStrap)
        {
            Marker *originMarker = createLocalMarkerCopy(nPointStrap->GetOriginMarker());
            Marker *insertionMarker = createLocalMarkerCopy(nPointStrap->GetInsertionMarker());
            std::vector<Marker *> viaPointList;
            for (auto &&it : *nPointStrap->GetViaPointMarkers()) viaPointList.push_back(createLocalMarkerCopy(it));
            if (!originMarker || !insertionMarker)
            {
                setLastError("Driver ID=\""s + name() +"\" cannot create local strap=\""s + strapPtr->name() + "\""s);
                return lastErrorPtr();
            }
            std::unique_ptr<NPointStrap> strap = std::make_unique<NPointStrap>();
            strap->setName(strapPtr->name());
            strap->SetOrigin(originMarker);
            strap->SetInsertion(insertionMarker);
            strap->SetViaPoints(&viaPointList);
            m_localStrapList[strap->name()] = std::move(strap);
            continue;
        }
        CylinderWrapStrap *cylinderWrapStrap = dynamic_cast<CylinderWrapStrap *>(strapPtr);
        if (cylinderWrapStrap)
        {
            Marker *originMarker = createLocalMarkerCopy(cylinderWrapStrap->GetOriginMarker());
            Marker *insertionMarker = createLocalMarkerCopy(cylinderWrapStrap->GetInsertionMarker());
            Marker *cylinderMarker = createLocalMarkerCopy(cylinderWrapStrap->GetCylinderMarker());
            if (!originMarker || !insertionMarker || !cylinderMarker)
            {
                setLastError("Driver ID=\""s + name() +"\" cannot create local strap=\""s + strapPtr->name() + "\""s);
                return lastErrorPtr();
            }
            std::unique_ptr<CylinderWrapStrap> strap = std::make_unique<CylinderWrapStrap>();
            strap->setName(strapPtr->name());
            strap->SetOrigin(originMarker);
            strap->SetInsertion(insertionMarker);
            strap->SetCylinder(cylinderMarker);
            strap->SetCylinderRadius(cylinderWrapStrap->cylinderRadius());
            m_localStrapList[strap->name()] = std::move(strap);
            continue;
        }
        TwoCylinderWrapStrap *twoCylinderWrapStrap = dynamic_cast<TwoCylinderWrapStrap *>(strapPtr);
        if (twoCylinderWrapStrap)
        {
            Marker *originMarker = createLocalMarkerCopy(twoCylinderWrapStrap->GetOriginMarker());
            Marker *insertionMarker = createLocalMarkerCopy(twoCylinderWrapStrap->GetInsertionMarker());
            Marker *cylinder1Marker = createLocalMarkerCopy(twoCylinderWrapStrap->GetCylinder1Marker());
            Marker *cylinder2Marker = createLocalMarkerCopy(twoCylinderWrapStrap->GetCylinder2Marker());
            if (!originMarker || !insertionMarker || !cylinder1Marker || !cylinder2Marker)
            {
                setLastError("Driver ID=\""s + name() +"\" cannot create local strap=\""s + strapPtr->name() + "\""s);
                return lastErrorPtr();
            }
            std::unique_ptr<TwoCylinderWrapStrap> strap = std::make_unique<TwoCylinderWrapStrap>();
            strap->setName(strapPtr->name());
            strap->SetOrigin(originMarker);
            strap->SetInsertion(insertionMarker);
            strap->SetCylinder1(cylinder1Marker);
            strap->SetCylinder2(cylinder2Marker);
            strap->SetCylinder1Radius(twoCylinderWrapStrap->Cylinder1Radius());
            strap->SetCylinder2Radius(twoCylinderWrapStrap->Cylinder2Radius());
            m_localStrapList[strap->name()] = std::move(strap);
            continue;
        }
    }

    return nullptr;
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void TwoHingeJointDriver::appendToAttributes()
{
    Driver::appendToAttributes();
    std::string buf;
    buf.reserve(64);
    setAttribute("Type"s, "TwoHingeJoint"s);
    setAttribute("TargetMarkerID"s, m_targetMarker->name());
    setAttribute("DistalBodyMarkerID"s, m_distalBodyMarker->name());
    setAttribute("ProximalJointID"s, m_proximalJoint->name());
    setAttribute("DistalJointID"s, m_distalJoint->name());
    setAttribute("ProximalJointRange"s, *GSUtil::ToString(m_proximalJointRange, &buf));
    setAttribute("DistalJointRange"s, *GSUtil::ToString(m_distalJointRange, &buf));
    setAttribute("Tolerance"s, *GSUtil::ToString(m_tolerance, &buf));
}


Marker *TwoHingeJointDriver::targetMarker() const
{
    return m_targetMarker;
}

void TwoHingeJointDriver::setTargetMarker(Marker *targetMarker)
{
    m_targetMarker = targetMarker;
}

Marker *TwoHingeJointDriver::distalBodyMarker() const
{
    return m_distalBodyMarker;
}

void TwoHingeJointDriver::setDistalBodyMarker(Marker *distalBodyMarker)
{
    m_distalBodyMarker = distalBodyMarker;
}

Joint *TwoHingeJointDriver::proximalJoint() const
{
    return m_proximalJoint;
}

void TwoHingeJointDriver::setProximalJoint(Joint *proximalJoint)
{
    m_proximalJoint = proximalJoint;
}

HingeJoint *TwoHingeJointDriver::distalJoint() const
{
    return m_distalJoint;
}

void TwoHingeJointDriver::setDistalJoint(HingeJoint *distalJoint)
{
    m_distalJoint = distalJoint;
}

double TwoHingeJointDriver::desiredLength() const
{
    return m_desiredLength;
}

void TwoHingeJointDriver::setDesiredLength(double desiredLength)
{
    m_desiredLength = desiredLength;
}

double TwoHingeJointDriver::actualLength() const
{
    return m_actualLength;
}

