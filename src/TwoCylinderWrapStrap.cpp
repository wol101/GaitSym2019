/*
 *  TwoCylinderWrapStrap.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 19/12/2012.
 *  Copyright 2012 Bill Sellers. All rights reserved.
 *
 */

#include "TwoCylinderWrapStrap.h"
#include "Body.h"
#include "PGDMath.h"
#include "DataFile.h"
#include "Simulation.h"
#include "GSUtil.h"
#include "Marker.h"

#include <cmath>
#include <string.h>

using namespace std::string_literals;

TwoCylinderWrapStrap::TwoCylinderWrapStrap()
{
}

TwoCylinderWrapStrap::~TwoCylinderWrapStrap()
{
}

//void TwoCylinderWrapStrap::SetOrigin(Body *body, const dVector3 point)
//{
//    m_originBody = body;
//    m_originPosition.x = point[0];
//    m_originPosition.y = point[1];
//    m_originPosition.z = point[2];
//    if (GetPointForceList()->size() == 0)
//    {
//        std::unique_ptr<PointForce> origin = std::make_unique<PointForce>();
//        origin->body = m_originBody;
//        GetPointForceList()->push_back(std::move(origin));
//    }
//    else
//    {
//        GetPointForceList()->at(0)->body = m_originBody;
//    }
//}

//void TwoCylinderWrapStrap::SetInsertion(Body *body, const dVector3 point)
//{
//    m_insertionBody = body;
//    m_insertionPosition.x = point[0];
//    m_insertionPosition.y = point[1];
//    m_insertionPosition.z = point[2];
//    if (GetPointForceList()->size() <= 1)
//    {
//        std::unique_ptr<PointForce> insertion = std::make_unique<PointForce>();
//        insertion->body = m_insertionBody;
//        GetPointForceList()->push_back(std::move(insertion));
//    }
//    else
//    {
//        GetPointForceList()->at(1)->body = m_insertionBody;
//    }
//}

void TwoCylinderWrapStrap::SetOrigin(Marker *originMarker)
{
    m_originMarker = originMarker;
//    this->SetOrigin(originMarker->GetBody(), originMarker->GetPosition().data());
    if (GetPointForceList()->size() == 0)
    {
        std::unique_ptr<PointForce> origin = std::make_unique<PointForce>();
        origin->body = m_originMarker->GetBody();
        GetPointForceList()->push_back(std::move(origin));
    }
    else
    {
        GetPointForceList()->at(0)->body = m_originMarker->GetBody();
    }
}

void TwoCylinderWrapStrap::SetInsertion(Marker *insertionMarker)
{
    m_insertionMarker = insertionMarker;
//    this->SetInsertion(insertionMarker->GetBody(), insertionMarker->GetPosition().data());
    if (GetPointForceList()->size() <= 1)
    {
        std::unique_ptr<PointForce> insertion = std::make_unique<PointForce>();
        insertion->body = m_insertionMarker->GetBody();
        GetPointForceList()->push_back(std::move(insertion));
    }
    else
    {
        GetPointForceList()->at(1)->body =  m_insertionMarker->GetBody();
    }
}

//void TwoCylinderWrapStrap::SetCylinder1Body(Body *body)
//{
//    m_cylinder1Body = body;
//    if (GetPointForceList()->size() <= 2)
//    {
//        std::unique_ptr<PointForce> cylinder1 = std::make_unique<PointForce>();
//        cylinder1->body = m_cylinder1Body;
//        GetPointForceList()->push_back(std::move(cylinder1));
//    }
//    else
//    {
//        GetPointForceList()->at(2)->body = m_cylinder1Body;
//    }
//}

void TwoCylinderWrapStrap::SetCylinder1Radius(double radius)
{
    m_cylinder1Radius = radius;
}

//void TwoCylinderWrapStrap::SetCylinder1Position(double x, double y, double z)
//{
//    m_cylinder1Position.x = x;
//    m_cylinder1Position.y = y;
//    m_cylinder1Position.z = z;
//}

//void TwoCylinderWrapStrap::GetCylinder1(const Body **body, dVector3 position, double *radius, dQuaternion q) const
//{
//    *body = m_cylinder1Body;
//    position[0] = m_cylinder1Position.x;
//    position[1] = m_cylinder1Position.y;
//    position[2] = m_cylinder1Position.z;
//    *radius = m_cylinder1Radius;
//    q[0] = m_cylinderQuaternion.n;
//    q[1] = m_cylinderQuaternion.x;
//    q[2] = m_cylinderQuaternion.y;
//    q[3] = m_cylinderQuaternion.z;
//}

void TwoCylinderWrapStrap::SetCylinder1(Marker *cylinder1Marker)
{
    m_cylinder1Marker = cylinder1Marker;
//    this->SetCylinder1Body(cylinder1Marker->GetBody());
//    pgd::Vector3 pos = cylinder1Marker->GetPosition();  // Cylinder Position is set in Body relative coordinates
//    this->SetCylinder1Position(pos.x, pos.y, pos.z);
//    pgd::Vector3 axis = cylinder1Marker->GetAxis(Marker::Axis::X);  // Cylinder Axis is set in Body relative coordinates
//    this->SetCylinderAxis(axis.x, axis.y, axis.z);
    if (GetPointForceList()->size() <= 2)
    {
        std::unique_ptr<PointForce> cylinder1 = std::make_unique<PointForce>();
        cylinder1->body = GetCylinder1Marker()->GetBody();
        GetPointForceList()->push_back(std::move(cylinder1));
    }
    else
    {
        GetPointForceList()->at(2)->body = GetCylinder1Marker()->GetBody();
    }
}

void TwoCylinderWrapStrap::SetCylinder2(Marker *cylinder2Marker)
{
    m_cylinder2Marker = cylinder2Marker;
//    this->SetCylinder2Body(cylinder2Marker->GetBody());
//    pgd::Vector3 pos = cylinder2Marker->GetPosition();  // Cylinder Position is set in Body relative coordinates
//    this->SetCylinder2Position(pos.x, pos.y, pos.z);
    if (GetPointForceList()->size() <= 3)
    {
        std::unique_ptr<PointForce> cylinder2 = std::make_unique<PointForce>();
        cylinder2->body = GetCylinder2Marker()->GetBody();
        GetPointForceList()->push_back(std::move(cylinder2));
    }
    else
    {
        GetPointForceList()->at(2)->body = GetCylinder2Marker()->GetBody();
    }
}

//void TwoCylinderWrapStrap::SetCylinderAxis(double x, double y, double z)
//{
//    pgd::Vector3 v2(x, y, z); // this is the target direction
//    pgd::Vector3 v1(0, 0, 1); // and this is the Z axis we need to rotate

////    this is easy to explain but quite slow
////    // cross product will get us the rotation axis
////    pgd::Vector3 axis = v1 ^ v2;
////
////    // Use atan2 for a better angle.  If you use only cos or sin, you only get
////    // half the possible angles, and you can end up with rotations that flip around near
////    // the poles.
////
////    // cos angle obtained from dot product formula
////    // cos(a) = (s . e) / (||s|| ||e||)
////    double cosAng = v1 * v2; // (s . e)
////    double ls = v1.Magnitude();
////    ls = 1. / ls; // 1 / ||s||
////    double le = v2.Magnitude();
////    le = 1. / le; // 1 / ||e||
////    cosAng = cosAng * ls * le;
////
////    // sin angle obtained from cross product formula
////    // sin(a) = ||(s X e)|| / (||s|| ||e||)
////    double sinAng = axis.Magnitude(); // ||(s X e)||;
////    sinAng = sinAng * ls * le;
////    double angle = atan2(sinAng, cosAng); // rotations are in radians.
////
////    m_CylinderQuaternion = pgd::MakeQFromAxis(axis.x, axis.y, axis.z, angle);
//    m_cylinderQuaternion = pgd::FindRotation(v1, v2);
//}


//void TwoCylinderWrapStrap::SetCylinderQuaternion(double q0, double q1, double q2, double q3)
//{
//    m_cylinderQuaternion.n = q0;
//    m_cylinderQuaternion.x = q1;
//    m_cylinderQuaternion.y = q2;
//    m_cylinderQuaternion.z = q3;
//    m_cylinderQuaternion.Normalize(); // this is the safest option
//}

//void TwoCylinderWrapStrap::SetCylinder2Body(Body *body)
//{
//    m_cylinder2Body = body;
//    if (GetPointForceList()->size() <= 3)
//    {
//        std::unique_ptr<PointForce> cylinder2 = std::make_unique<PointForce>();
//        cylinder2->body = m_cylinder2Body;
//        GetPointForceList()->push_back(std::move(cylinder2));
//    }
//    else
//    {
//        GetPointForceList()->at(2)->body = m_cylinder2Body;
//    }
//}

void  TwoCylinderWrapStrap::SetCylinder2Radius(double radius)
{
    m_cylinder2Radius = radius;
}

//void TwoCylinderWrapStrap::SetCylinder2Position(double x, double y, double z)
//{
//    m_cylinder2Position.x = x;
//    m_cylinder2Position.y = y;
//    m_cylinder2Position.z = z;
//}

void TwoCylinderWrapStrap::SetNumWrapSegments(int numWrapSegments)
{
    m_numWrapSegments = numWrapSegments;
    m_pathCoordinates.reserve(size_t(m_numWrapSegments) * 2 + 6);
}

//void TwoCylinderWrapStrap::GetCylinder2(const Body **body, dVector3 position, double *radius, dQuaternion q) const
//{
//    *body = m_cylinder2Body;
//    position[0] = m_cylinder2Position.x;
//    position[1] = m_cylinder2Position.y;
//    position[2] = m_cylinder2Position.z;
//    *radius = m_cylinder2Radius;
//    q[0] = m_cylinderQuaternion.n;
//    q[1] = m_cylinderQuaternion.x;
//    q[2] = m_cylinderQuaternion.y;
//    q[3] = m_cylinderQuaternion.z;
//}

//void TwoCylinderWrapStrap::GetOrigin(const Body **body, dVector3 pos) const
//{
//    *body = m_originBody;
//    pos[0] = m_originPosition.x;
//    pos[1] = m_originPosition.y;
//    pos[2] = m_originPosition.z;
//}

//void TwoCylinderWrapStrap::GetInsertion(const Body **body, dVector3 pos) const
//{
//    *body = m_insertionBody;
//    pos[0] = m_insertionPosition.x;
//    pos[1] = m_insertionPosition.y;
//    pos[2] = m_insertionPosition.z;
//}

Marker *TwoCylinderWrapStrap::GetOriginMarker() const
{
    return m_originMarker;
}

Marker *TwoCylinderWrapStrap::GetInsertionMarker() const
{
    return m_insertionMarker;
}

Marker *TwoCylinderWrapStrap::GetCylinder1Marker() const
{
    return m_cylinder1Marker;
}

Marker *TwoCylinderWrapStrap::GetCylinder2Marker() const
{
    return m_cylinder2Marker;
}

int TwoCylinderWrapStrap::GetNumWrapSegments()
{
    return m_numWrapSegments;
}

void TwoCylinderWrapStrap::Calculate()
{
    // get the necessary body orientations and positions
//    const double *q;
//    q = dBodyGetQuaternion(m_OriginBody->GetBodyID());
//    pgd::Quaternion qOriginBody(q[0], q[1], q[2], q[3]);
//    q = dBodyGetPosition(m_OriginBody->GetBodyID());
//    pgd::Vector3 vOriginBody(q[0], q[1], q[2]);
//    q = dBodyGetQuaternion(m_InsertionBody->GetBodyID());
//    pgd::Quaternion qInsertionBody(q[0], q[1], q[2], q[3]);
//    q = dBodyGetPosition(m_InsertionBody->GetBodyID());
//    pgd::Vector3 vInsertionBody(q[0], q[1], q[2]);
//    q = dBodyGetQuaternion(m_Cylinder1Body->GetBodyID());
//    pgd::Quaternion qCylinder1Body(q[0], q[1], q[2], q[3]);
//    q = dBodyGetPosition(m_Cylinder1Body->GetBodyID());
//    pgd::Vector3 vCylinder1Body(q[0], q[1], q[2]);
//    q = dBodyGetQuaternion(m_Cylinder2Body->GetBodyID());
//    pgd::Quaternion qCylinder2Body(q[0], q[1], q[2], q[3]);
//    q = dBodyGetPosition(m_Cylinder2Body->GetBodyID());
//    pgd::Vector3 vCylinder2Body(q[0], q[1], q[2]);
    pgd::Quaternion qOriginBody = GetOriginMarker()->GetBody()->GetQuaternion();
    pgd::Vector3 vOriginBody = GetOriginMarker()->GetBody()->GetPosition();
    pgd::Quaternion qInsertionBody = GetInsertionMarker()->GetBody()->GetQuaternion();
    pgd::Vector3 vInsertionBody = GetInsertionMarker()->GetBody()->GetPosition();
    pgd::Quaternion qCylinder1Body = GetCylinder1Marker()->GetBody()->GetQuaternion();
    pgd::Vector3 vCylinder1Body = GetCylinder1Marker()->GetBody()->GetPosition();
    pgd::Quaternion qCylinder2Body = GetCylinder2Marker()->GetBody()->GetQuaternion();
    pgd::Vector3 vCylinder2Body = GetCylinder2Marker()->GetBody()->GetPosition();
//    m_originBody->GetQuaternion(&qOriginBody);
//    m_originBody->GetPosition(&vOriginBody);
//    m_insertionBody->GetQuaternion(&qInsertionBody);
//    m_insertionBody->GetPosition(&vInsertionBody);
//    m_cylinder1Body->GetQuaternion(&qCylinder1Body);
//    m_cylinder1Body->GetPosition(&vCylinder1Body);
//    m_cylinder2Body->GetQuaternion(&qCylinder2Body);
//    m_cylinder2Body->GetPosition(&vCylinder2Body);

    pgd::Vector3 m_originPosition = GetOriginMarker()->GetPosition();
    pgd::Vector3 m_insertionPosition = GetInsertionMarker()->GetPosition();
    pgd::Vector3 m_cylinder1Position = GetCylinder1Marker()->GetPosition();
    pgd::Vector3 m_cylinder2Position = GetCylinder2Marker()->GetPosition();

    // the cylinder quaternion in this implementation is the quaternion that rotates the the x axis of the marker to the z axis
    pgd::Vector3 v2 = GetCylinder1Marker()->GetAxis(Marker::Axis::X);
    pgd::Vector3 v1(0, 0, 1); // and this is the Z axis we need to rotate
    pgd::Quaternion m_cylinderQuaternion = pgd::FindRotation(v1, v2);

    // calculate some inverses
    pgd::Quaternion qCylinder1BodyInv = ~qCylinder1Body; // we only need qCylinder1Body because the m_CylinderQuaternion is relative to body 1
    pgd::Quaternion qCylinderQuaternionInv = ~m_cylinderQuaternion;

    // get the world coordinates of the origin and insertion
    pgd::Vector3 worldOriginPosition = QVRotate(qOriginBody, m_originPosition) + vOriginBody;
    pgd::Vector3 worldInsertionPosition = QVRotate(qInsertionBody, m_insertionPosition) + vInsertionBody;

    // get the world coordinates of the cylinders
    pgd::Vector3 worldCylinder1Position = QVRotate(qCylinder1Body, m_cylinder1Position) + vCylinder1Body;
    pgd::Vector3 worldCylinder2Position = QVRotate(qCylinder2Body, m_cylinder2Position) + vCylinder2Body;

    // now rotate so the cylinder axes are lined up on the z axis
    pgd::Vector3 cylinderOriginPosition = QVRotate(qCylinderQuaternionInv, QVRotate(qCylinder1BodyInv, worldOriginPosition));
    pgd::Vector3 cylinderInsertionPosition = QVRotate(qCylinderQuaternionInv, QVRotate(qCylinder1BodyInv, worldInsertionPosition));
    pgd::Vector3 cylinderCylinder1Position = QVRotate(qCylinderQuaternionInv, QVRotate(qCylinder1BodyInv, worldCylinder1Position));
    pgd::Vector3 cylinderCylinder2Position = QVRotate(qCylinderQuaternionInv, QVRotate(qCylinder1BodyInv, worldCylinder2Position));

    pgd::Vector3 theOriginForce;
    pgd::Vector3 theInsertionForce;
    pgd::Vector3 theCylinder1Force;
    pgd::Vector3 theCylinder1ForcePosition;
    pgd::Vector3 theCylinder2Force;
    pgd::Vector3 theCylinder2ForcePosition;
    double tension = 1; // normalised initially because tension is applied by muscle

    double length = 0;
    TwoCylinderWrap(cylinderOriginPosition, cylinderInsertionPosition, cylinderCylinder1Position, m_cylinder1Radius,
                    cylinderCylinder2Position, m_cylinder2Radius, tension, m_numWrapSegments, M_PI,
                    theOriginForce, theInsertionForce, theCylinder1Force, theCylinder1ForcePosition,
                    theCylinder2Force, theCylinder2ForcePosition, &length,
                    &m_pathCoordinates, &m_wrapStatus);
    if (m_wrapStatus == -1) {
        std::cerr << "Warning: wrapping impossible in \"" << name() << "\" - attachment inside cylinder\n"; }
    if (Length() >= 0 && simulation() && simulation()->GetTimeIncrement() > 0) setVelocity((length - Length()) / simulation()->GetTimeIncrement());
    else setVelocity(0);
    setLength(length);

    // now rotate back to world reference frame

    theOriginForce = QVRotate(qCylinder1Body, QVRotate(m_cylinderQuaternion, theOriginForce));
    theInsertionForce = QVRotate(qCylinder1Body, QVRotate(m_cylinderQuaternion, theInsertionForce));
    theCylinder1Force = QVRotate(qCylinder1Body, QVRotate(m_cylinderQuaternion, theCylinder1Force));
    theCylinder1ForcePosition = QVRotate(qCylinder1Body, QVRotate(m_cylinderQuaternion, theCylinder1ForcePosition));
    theCylinder2Force = QVRotate(qCylinder1Body, QVRotate(m_cylinderQuaternion, theCylinder2Force));
    theCylinder2ForcePosition = QVRotate(qCylinder1Body, QVRotate(m_cylinderQuaternion, theCylinder2ForcePosition));


    PointForce *theOrigin = (*GetPointForceList())[0].get();
    PointForce *theInsertion = (*GetPointForceList())[1].get();
    PointForce *theCylinder1 = (*GetPointForceList())[2].get();
    PointForce *theCylinder2 = (*GetPointForceList())[3].get();
    theOrigin->vector[0] = theOriginForce.x; theOrigin->vector[1] = theOriginForce.y; theOrigin->vector[2] = theOriginForce.z;
    theOrigin->point[0] = worldOriginPosition.x; theOrigin->point[1] = worldOriginPosition.y; theOrigin->point[2] = worldOriginPosition.z;
    theInsertion->vector[0] = theInsertionForce.x; theInsertion->vector[1] = theInsertionForce.y; theInsertion->vector[2] = theInsertionForce.z;
    theInsertion->point[0] = worldInsertionPosition.x; theInsertion->point[1] = worldInsertionPosition.y; theInsertion->point[2] = worldInsertionPosition.z;
    theCylinder1->vector[0] = theCylinder1Force.x; theCylinder1->vector[1] = theCylinder1Force.y; theCylinder1->vector[2] = theCylinder1Force.z;
    theCylinder1->point[0] = theCylinder1ForcePosition.x; theCylinder1->point[1] = theCylinder1ForcePosition.y; theCylinder1->point[2] = theCylinder1ForcePosition.z;
    theCylinder2->vector[0] = theCylinder2Force.x; theCylinder2->vector[1] = theCylinder2Force.y; theCylinder2->vector[2] = theCylinder2Force.z;
    theCylinder2->point[0] = theCylinder2ForcePosition.x; theCylinder2->point[1] = theCylinder2ForcePosition.y; theCylinder2->point[2] = theCylinder2ForcePosition.z;

    // and handle the path coordinates
    for (size_t i = 0; i < m_pathCoordinates.size(); i++)
    {
        m_pathCoordinates[i] = QVRotate(qCylinder1Body, QVRotate(m_cylinderQuaternion, m_pathCoordinates[i]));
    }

    // check that we don't have any non-finite values for directions which can occur if points co-locate
    for (size_t i = 0; i < GetPointForceList()->size(); i++)
    {
        if ((std::isfinite((*GetPointForceList())[i]->vector[0]) && std::isfinite((*GetPointForceList())[i]->vector[1]) && std::isfinite((*GetPointForceList())[i]->vector[2])) == false)
        {
            (*GetPointForceList())[i]->vector[0] = 0.0;
            (*GetPointForceList())[i]->vector[1] = 0.0;
            (*GetPointForceList())[i]->vector[2] = 0.0;
            std::cerr << "Warning: point force direction in \"" << name() << "\" is invalid so applying standard fixup\n";
        }
    }
}

// function to wrap a line around two parallel cylinders
// the cylinders are assumed to have their axes along the z axis and the wrapping is according
// to the right hand rule
// the coordinate system is right handed too
// wrapOK returns -1 if wrapping cannot occur
void TwoCylinderWrapStrap::TwoCylinderWrap(pgd::Vector3 &origin, pgd::Vector3 &insertion, pgd::Vector3 &cylinderPosition1, double radius1,
                                           pgd::Vector3 &cylinderPosition2, double radius2, double tension, int nPointsPerCylinderArc, double maxAngle,
                                           pgd::Vector3 &originForce, pgd::Vector3 &insertionForce, pgd::Vector3 &cylinderForce1, pgd::Vector3 &cylinderForcePosition1,
                                           pgd::Vector3 &cylinderForce2, pgd::Vector3 &cylinderForcePosition2, double *pathLength,
                                           std::vector<pgd::Vector3> *pathCoordinates, int *wrapOK)
{
    // this is the special case looking down the axis of the cylinder (i.e. xy plane)
    // this is standard tangent to a circle stuff

    // using letters as defined in TwoCylinderDiagram
    // set knowns to match diagram

    pgd::Vector3 O = origin;
    pgd::Vector3 I = insertion;
    pgd::Vector3 C = cylinderPosition1;
    pgd::Vector3 D = cylinderPosition2;
    double r = radius1;
    double s = radius2;

    const double small_angle = 1e-10;
    int number_of_tangents;

    pathCoordinates->clear();

    pgd::Vector3 E1, E2, H1, H2, G1, G2, F1, F2, J1, J2, K1, K2;

    // origin to first cylinder
    FindTangents(C, r, O, E1, E2, &number_of_tangents);
    if (number_of_tangents == 0)
    {
        *wrapOK = -1;
        return;
    }

    // insertion to second cylinder
    FindTangents(D, s, I, H1, H2, &number_of_tangents);
    if (number_of_tangents == 0)
    {
        *wrapOK = -1;
        return;
    }

    // now find line between cylinders
    pgd::Vector3 inner1_p1, inner1_p2, inner2_p1, inner2_p2; // not currently used
    FindCircleCircleTangents(C, r, D, s, F1, G1, F2, G2, inner1_p1, inner1_p2, inner2_p1, inner2_p2, &number_of_tangents);

    // now calculate the planar path length
    double cyl1_start_angle = atan2(E2.y - C.y, E2.x - C.x);
    double cyl1_end_angle = atan2(F2.y - C.y, F2.x - C.x);
    double cyl1_theta = cyl1_end_angle - cyl1_start_angle;
    if (cyl1_theta < 0) cyl1_theta = cyl1_theta + 2 * M_PI;
    double cyl2_start_angle = atan2(G2.y - D.y, G2.x - D.x);
    double cyl2_end_angle = atan2(H1.y - D.y, H1.x - D.x);
    double cyl2_theta = cyl2_end_angle - cyl2_start_angle;
    if (cyl2_theta < 0) cyl2_theta = cyl2_theta + 2 * M_PI;

    // use maxAngle to decide whether wrapping is appropriate
    // values between pi and 2pi are sensible

    double l1, l2, l3, l4, l5;
    double planar_path_length, delta_Z, cyl1_del_theta, cyl2_del_theta, theta;

    if (cyl1_theta < maxAngle && cyl2_theta < maxAngle && cyl1_theta > small_angle && cyl2_theta > small_angle) // the double wrap case
    {
        *wrapOK = 1;

        l1 = vector_distance2d(O, E2);
        l2 = cyl1_theta * r;
        l3 = vector_distance2d(F2, G2);
        l4 = cyl2_theta * s;
        l5 = vector_distance2d(H1, I);

        planar_path_length = l1 + l2 + l3 + l4 + l5;
        delta_Z = I.z - O.z;

        E2.z = O.z + delta_Z * l1 / planar_path_length;
        F2.z = O.z + delta_Z * (l1 + l2) / planar_path_length;
        G2.z = O.z + delta_Z * (l1 + l2 + l3) / planar_path_length;
        H1.z = O.z + delta_Z * (l1 + l2 + l3 + l4) / planar_path_length;

        vector_with_magnitude(O, E2, tension, originForce);
        vector_with_magnitude(I, H1, tension, insertionForce);

        pgd::Vector3 betweenForce;
        vector_with_magnitude(F2, G2, tension, betweenForce);

        cylinderForce1.x = betweenForce.x - originForce.x;
        cylinderForce1.y = betweenForce.y - originForce.y;
        cylinderForce1.z = betweenForce.z - originForce.z;
        cylinderForcePosition1.x = cylinderPosition1.x;
        cylinderForcePosition1.y = cylinderPosition1.y;
        cylinderForcePosition1.z = (E2.z + F2.z) / 2;

        cylinderForce2.x = -betweenForce.x - insertionForce.x;
        cylinderForce2.y = -betweenForce.y - insertionForce.y;
        cylinderForce2.z = -betweenForce.z - insertionForce.z;
        cylinderForcePosition2.x = cylinderPosition2.x;
        cylinderForcePosition2.y = cylinderPosition2.y;
        cylinderForcePosition2.z = (G2.z + H1.z) / 2;

        *pathLength = sqrt(delta_Z * delta_Z + planar_path_length * planar_path_length);

        if (pathCoordinates && nPointsPerCylinderArc > 1)
        {
            pgd::Vector3 p;
            pathCoordinates->push_back(O);
            pathCoordinates->push_back(E2);

            // now fill in the missing bits of the 1st circle
            cyl1_del_theta = cyl1_theta / nPointsPerCylinderArc;
            theta = cyl1_start_angle;
            for (int j = 1; j <= nPointsPerCylinderArc - 1; j++)
            {
                theta = theta + cyl1_del_theta;
                p.x = C.x + r * cos(theta);
                p.y = C.y + r * sin(theta);
                p.z = O.z + delta_Z * (l1 + (double(j) / double(nPointsPerCylinderArc)) * l2) / planar_path_length;
                pathCoordinates->push_back(p);
            }

            pathCoordinates->push_back(F2);
            pathCoordinates->push_back(G2);

            // now fill in the missing bits of the 2nd circle
            cyl2_del_theta = cyl2_theta / nPointsPerCylinderArc;
            theta = cyl2_start_angle;
            for (int j = 1; j <= nPointsPerCylinderArc - 1; j++)
            {
                theta = theta + cyl2_del_theta;
                p.x = D.x + s * cos(theta);
                p.y = D.y + s * sin(theta);
                p.z = O.z + delta_Z * (l1 + l2 + l3 + (double(j) / double(nPointsPerCylinderArc)) * l4) / planar_path_length;
                pathCoordinates->push_back(p);
            }

            pathCoordinates->push_back(H1);
            pathCoordinates->push_back(I);
        }
        return;
    }

    if (cyl1_theta < maxAngle && cyl1_theta > small_angle) // try cyl 1 wrapping
    {
        // insertion to first cylinder
        FindTangents(C, r, I, K1, K2, &number_of_tangents);
        if (number_of_tangents == 0)
        {
            *wrapOK = -1;
            return;
        }

        // calculate new angle
        cyl1_start_angle = atan2(E2.y - C.y, E2.x - C.x);
        cyl1_end_angle = atan2(K1.y - C.y, K1.x - C.x);
        cyl1_theta = cyl1_end_angle - cyl1_start_angle;
        if (cyl1_theta < 0) cyl1_theta = cyl1_theta + 2 * M_PI;

        if (cyl1_theta < maxAngle)
        {
            *wrapOK = 2;

            l1 = vector_distance2d(O, E2);
            l2 = cyl1_theta * r;
            l3 = vector_distance2d(K1, I);

            planar_path_length = l1 + l2 + l3;
            delta_Z = I.z - O.z;

            E2.z = O.z + delta_Z * l1 / planar_path_length;
            K1.z = O.z + delta_Z * (l1 + l2) / planar_path_length;

             vector_with_magnitude(O, E2, tension, originForce);
             vector_with_magnitude(I, K1, tension, insertionForce);

            cylinderForce1.x = -insertionForce.x - originForce.x;
            cylinderForce1.y = -insertionForce.y - originForce.y;
            cylinderForce1.z = -insertionForce.z - originForce.z;
            cylinderForcePosition1.x = cylinderPosition1.x;
            cylinderForcePosition1.y = cylinderPosition1.y;
            cylinderForcePosition1.z = (E2.z + K1.z) / 2;

            cylinderForce2.x = 0;
            cylinderForce2.y = 0;
            cylinderForce2.z = 0;
            cylinderForcePosition2.x = 0;
            cylinderForcePosition2.y = 0;
            cylinderForcePosition2.z = 0;

            *pathLength = sqrt(delta_Z * delta_Z + planar_path_length * planar_path_length);

            if (pathCoordinates && nPointsPerCylinderArc > 1)
            {
                pgd::Vector3 p;
                pathCoordinates->push_back(O);
                pathCoordinates->push_back(E2);

                // now fill in the missing bits of the 1st circle
                cyl1_del_theta = cyl1_theta / nPointsPerCylinderArc;
                theta = cyl1_start_angle;
                for (int j = 1; j <= nPointsPerCylinderArc - 1; j++)
                {
                    theta = theta + cyl1_del_theta;
                    p.x = C.x + r * cos(theta);
                    p.y = C.y + r * sin(theta);
                    p.z = O.z + delta_Z * (l1 + (double(j) / double(nPointsPerCylinderArc)) * l2) / planar_path_length;
                    pathCoordinates->push_back(p);
                }

                pathCoordinates->push_back(K1);
                pathCoordinates->push_back(I);
            }
            return;
        }
    }

    if (cyl2_theta < maxAngle && cyl2_theta > small_angle) // try cyl 2 wrapping
    {
        // insertion to first cylinder
        FindTangents(D, s, O, J1, J2, &number_of_tangents);
        if (number_of_tangents == 0)
        {
            *wrapOK = -1;
            return;
        }

        // calculate new angle
        cyl2_start_angle = atan2(J2.y - D.y, J2.x - D.x);
        cyl2_end_angle = atan2(H1.y - D.y, H1.x - D.x);
        cyl2_theta = cyl2_end_angle - cyl2_start_angle;
        if (cyl2_theta < 0) cyl2_theta = cyl2_theta + 2 * M_PI;

        if (cyl2_theta < maxAngle)
        {
            *wrapOK = 3;

            l1 = vector_distance2d(O, J2);
            l2 = cyl2_theta * s;
            l3 = vector_distance2d(H1, I);

            planar_path_length = l1 + l2 + l3;
            delta_Z = I.z - O.z;

            J2.z = O.z + delta_Z * l1 / planar_path_length;
            H1.z = O.z + delta_Z * (l1 + l2) / planar_path_length;

            vector_with_magnitude(O, J2, tension, originForce);
            vector_with_magnitude(I, H1, tension, insertionForce);

            cylinderForce2.x = -insertionForce.x - originForce.x;
            cylinderForce2.y = -insertionForce.y - originForce.y;
            cylinderForce2.z = -insertionForce.z - originForce.z;
            cylinderForcePosition2.x = cylinderPosition2.x;
            cylinderForcePosition2.y = cylinderPosition2.y;
            cylinderForcePosition2.z = (J2.z + H1.z) / 2;

            cylinderForce1.x = 0;
            cylinderForce1.y = 0;
            cylinderForce1.z = 0;
            cylinderForcePosition1.x = 0;
            cylinderForcePosition1.y = 0;
            cylinderForcePosition1.z = 0;

            *pathLength = sqrt(delta_Z * delta_Z + planar_path_length * planar_path_length);

            if (pathCoordinates && nPointsPerCylinderArc > 1)
            {
                pgd::Vector3 p;
                pathCoordinates->push_back(O);
                pathCoordinates->push_back(J2);

                // now fill in the missing bits of the 1st circle
                cyl2_del_theta = cyl2_theta / nPointsPerCylinderArc;
                theta = cyl2_start_angle;
                for (int j = 1; j <= nPointsPerCylinderArc - 1; j++)
                {
                    theta = theta + cyl2_del_theta;
                    p.x = D.x + s * cos(theta);
                    p.y = D.y + s * sin(theta);
                    p.z = O.z + delta_Z * (l1 + (double(j) / double(nPointsPerCylinderArc)) * l2) / planar_path_length;
                    pathCoordinates->push_back(p);
                }

                pathCoordinates->push_back(H1);
                pathCoordinates->push_back(I);
            }
            return;
        }
    }


    // if we get here then no wrapping is possible

    *wrapOK = 0;

    *pathLength = vector_distance3d(O, I);

    vector_with_magnitude(O, I, tension, originForce);
    insertionForce.x = -originForce.x;
    insertionForce.y = -originForce.y;
    insertionForce.z = -originForce.z;

    cylinderForce1.x = 0;
    cylinderForce1.y = 0;
    cylinderForce1.z = 0;
    cylinderForcePosition1.x = 0;
    cylinderForcePosition1.y = 0;
    cylinderForcePosition1.z = 0;
    cylinderForce2.x = 0;
    cylinderForce2.y = 0;
    cylinderForce2.z = 0;
    cylinderForcePosition2.x = 0;
    cylinderForcePosition2.y = 0;
    cylinderForcePosition2.z = 0;

    if (pathCoordinates && nPointsPerCylinderArc > 1)
    {
        pathCoordinates->push_back(O);
        pathCoordinates->push_back(I);
    }

    return;
}

// Adapted from http://www.vb-helper.com/howto_net_circle_circle_tangents.html
// Find the tangent points for these two circles.
// Return the number of tangents: 4, 2, or 0.
void TwoCylinderWrapStrap::FindCircleCircleTangents(pgd::Vector3 &c1, double radius1, pgd::Vector3 &c2, double radius2,
                                                    pgd::Vector3 &outer1_p1, pgd::Vector3 &outer1_p2, pgd::Vector3 &outer2_p1, pgd::Vector3 &outer2_p2,
                                                    pgd::Vector3 &inner1_p1, pgd::Vector3 &inner1_p2, pgd::Vector3 &inner2_p1, pgd::Vector3 &inner2_p2, int *number_of_tangents)
{

    //  Make sure radius1 <= radius2.
    if (radius1 > radius2)
    {
        // Call this method switching the circles.
        FindCircleCircleTangents(c2, radius2, c1, radius1, outer2_p2, outer2_p1, outer1_p2, outer1_p1, inner2_p2, inner2_p1, inner1_p2, inner1_p1, number_of_tangents);
        return;
    }

    // ***************************
    // * Find the outer tangents *
    // ***************************
    double radius2a = radius2 - radius1;
    FindTangents(c2, radius2a, c1, outer1_p2, outer2_p2, number_of_tangents);
    if (*number_of_tangents == 0)
        return; // There are no tangents.

    // Get the vector perpendicular to the
    // first tangent with length radius1.
    double v1x = -(outer1_p2.y - c1.y);
    double v1y = outer1_p2.x - c1.x;
    double v1_length = sqrt(v1x * v1x + v1y * v1y);
    v1x = v1x * radius1 / v1_length;
    v1y = v1y * radius1 / v1_length;
    // Offset the tangent vector's points.
    outer1_p1.x = c1.x + v1x;
    outer1_p1.y = c1.y + v1y;
    outer1_p2.x = outer1_p2.x + v1x;
    outer1_p2.y = outer1_p2.y + v1y;

    // Get the vector perpendicular to the
    // second tangent with length radius1.
    double v2x = outer2_p2.y - c1.y;
    double v2y = -(outer2_p2.x - c1.x);
    double v2_length = sqrt(v2x * v2x + v2y * v2y);
    v2x = v2x * radius1 / v2_length;
    v2y = v2y * radius1 / v2_length;
    // Offset the tangent vector's points.
    outer2_p1.x = c1.x + v2x;
    outer2_p1.y = c1.y + v2y;
    outer2_p2.x = outer2_p2.x + v2x;
    outer2_p2.y = outer2_p2.y + v2y;

    // If the circles intersect, then there are no inner
    // tangents.
    double dx = c2.x - c1.x;
    double dy = c2.y - c1.y;
    double dist = sqrt(dx * dx + dy * dy);
    if (dist <= radius1 + radius2)
    {
        *number_of_tangents = 2;
        return;
    }

    // ***************************
    // * Find the inner tangents *
    // ***************************
    double radius1a = radius1 + radius2;
    FindTangents(c1, radius1a, c2, inner1_p2, inner2_p2, number_of_tangents);

    // Get the vector perpendicular to the
    // first tangent with length radius2.
    v1x = inner1_p2.y - c2.y;
    v1y = -(inner1_p2.x - c2.x);
    v1_length = sqrt(v1x * v1x + v1y * v1y);
    v1x = v1x * radius2 / v1_length;
    v1y = v1y * radius2 / v1_length;
    // Offset the tangent vector's points.
    inner1_p1.x = c2.x + v1x;
    inner1_p1.y = c2.y + v1y;
    inner1_p2.x = inner1_p2.x + v1x;
    inner1_p2.y = inner1_p2.y + v1y;

    // Get the vector perpendicular to the
    // second tangent with length radius2.
    v2x = -(inner2_p2.y - c2.y);
    v2y = inner2_p2.x - c2.x;
    v2_length = sqrt(v2x * v2x + v2y * v2y);
    v2x = v2x * radius2 / v2_length;
    v2y = v2y * radius2 / v2_length;
    // Offset the tangent vector's points.
    inner2_p1.x = c2.x + v2x;
    inner2_p1.y = c2.y + v2y;
    inner2_p2.x = inner2_p2.x + v2x;
    inner2_p2.y = inner2_p2.y + v2y;

    *number_of_tangents = 4;
    return;
}


// Adapted from http://www.vb-helper.com/howto_net_find_circle_tangents.html
// Find the tangent points for this circle and external
// point.
// Return the number of tangents: 2, or 0.
void TwoCylinderWrapStrap::FindTangents(pgd::Vector3 &center, double radius, pgd::Vector3 &external_point, pgd::Vector3 &pt1, pgd::Vector3 &pt2, int *number_of_tangents)
{
    // Find the distance squared from the
    // external point to the circle's center.
    double dx = center.x - external_point.x;
    double dy = center.y - external_point.y;
    double D_squared = dx * dx + dy * dy;
    if (D_squared < radius * radius)
    {
        *number_of_tangents = 0;
        return;
    }

    // Find the distance from the external point
    // to the tangent points.
    double L = sqrt(D_squared - radius * radius);

    // Find the points of intersection between
    // the original circle and the circle with
    // center external_point and radius dist.

    int number_of_intersections;
    FindCircleCircleIntersections(center.x, center.y, radius, external_point.x, external_point.y, L, pt1, pt2, &number_of_intersections);
    *number_of_tangents = 2;
    return;
}

// Adapted from http://www.vb-helper.com/howto_net_circle_circle_intersection.html
// Find the points where the two circles intersect.
void TwoCylinderWrapStrap::FindCircleCircleIntersections(double cx0, double cy0, double radius0, double cx1, double cy1, double radius1,
                                                         pgd::Vector3 &intersection1, pgd::Vector3 &intersection2, int *number_of_intersections)
{
    // Find the distance between the centers.
    double dx = cx0 - cx1;
    double dy = cy0 - cy1;
    double dist = sqrt(dx * dx + dy * dy);

    // See how many solutions there are.
    if (dist > radius0 + radius1)
    {
        // No solutions, the circles are too far apart.
        *number_of_intersections = 0;
        return;
    }

    if (dist < fabs(radius0 - radius1))
    {
        // No solutions, one circle contains the other.
        *number_of_intersections = 0;
        return;
    }

    if ((dist == 0) && (radius0 == radius1)) // WARNING - shouldn't compare equality for FP numbers
    {
        // No solutions, the circles coincide.
        *number_of_intersections = 0;
        return;
    }

    // Find a and h.
    double a = (radius0 * radius0 - radius1 * radius1 + dist * dist) / (2 * dist);
    double h = sqrt(radius0 * radius0 - a * a);

    // Find P2.
    double cx2 = cx0 + a * (cx1 - cx0) / dist;
    double cy2 = cy0 + a * (cy1 - cy0) / dist;

    // Get the points P3.
    intersection1.x = cx2 + h * (cy1 - cy0) / dist;
    intersection1.y = cy2 - h * (cx1 - cx0) / dist;
    intersection2.x = cx2 - h * (cy1 - cy0) / dist;
    intersection2.y = cy2 + h * (cx1 - cx0) / dist;

    // See if we have 1 or 2 solutions.
    if (dist == radius0 + radius1) // WARNING - shouldn't compare equality for FP numbers
        *number_of_intersections = 1;
    else
        *number_of_intersections = 2;

    return;
}

// calculate the 2D length of a vector
double TwoCylinderWrapStrap::vector_distance2d(pgd::Vector3 &v1, pgd::Vector3 v2)
{
    return sqrt((v2.x - v1.x) * (v2.x - v1.x) + (v2.y - v1.y) * (v2.y - v1.y));
}

// calculate the 3D length of a vector
double TwoCylinderWrapStrap::vector_distance3d(pgd::Vector3 &v1, pgd::Vector3 &v2)
{
    return sqrt((v2.x - v1.x) * (v2.x - v1.x) + (v2.y - v1.y) * (v2.y - v1.y) + (v2.z - v1.z) * (v2.z - v1.z));
}

// return the vector in the direction of v1 to v2 with magnitude specified
void TwoCylinderWrapStrap::vector_with_magnitude(pgd::Vector3 &v1, pgd::Vector3 &v2, double magnitude, pgd::Vector3 &v)
{
    pgd::Vector3 del_v = v2 - v1;
    double mag = vector_distance3d(v1, v2);
    v.x = magnitude * del_v.x / mag;
    v.y = magnitude * del_v.y / mag;
    v.z = magnitude * del_v.z / mag;
    return;
}

double TwoCylinderWrapStrap::Cylinder2Radius() const
{
    return m_cylinder2Radius;
}

double TwoCylinderWrapStrap::Cylinder1Radius() const
{
    return m_cylinder1Radius;
}

const std::vector<pgd::Vector3> *TwoCylinderWrapStrap::GetPathCoordinates()
{
    return &m_pathCoordinates;
}

//int TwoCylinderWrapStrap::SanityCheck(Strap *otherStrap, Simulation::AxisType axis, const std::string &sanityCheckLeft, const std::string &sanityCheckRight)
//{
//    const double epsilon = 1e-10;

//    TwoCylinderWrapStrap *other = dynamic_cast<TwoCylinderWrapStrap *>(otherStrap);
//    if (other == nullptr) return __LINE__;

//    if (fabs(this->m_cylinder1Radius - other->m_cylinder1Radius) > epsilon) return __LINE__;
//    if (fabs(this->m_cylinder2Radius - other->m_cylinder2Radius) > epsilon) return __LINE__;

//    // first check attachment errors
//    switch (axis)
//    {
//    case Simulation::XAxis:
//        if (fabs(this->m_originPosition.x + other->m_originPosition.x) > epsilon) return __LINE__;
//        if (fabs(this->m_originPosition.y - other->m_originPosition.y) > epsilon) return __LINE__;
//        if (fabs(this->m_originPosition.z - other->m_originPosition.z) > epsilon) return __LINE__;
//        if (fabs(this->m_insertionPosition.x + other->m_insertionPosition.x) > epsilon) return __LINE__;
//        if (fabs(this->m_insertionPosition.y - other->m_insertionPosition.y) > epsilon) return __LINE__;
//        if (fabs(this->m_insertionPosition.z - other->m_insertionPosition.z) > epsilon) return __LINE__;
//        if (fabs(this->m_cylinder1Position.x + other->m_cylinder1Position.x) > epsilon) return __LINE__;
//        if (fabs(this->m_cylinder1Position.y - other->m_cylinder1Position.y) > epsilon) return __LINE__;
//        if (fabs(this->m_cylinder1Position.z - other->m_cylinder1Position.z) > epsilon) return __LINE__;
//        if (fabs(this->m_cylinder2Position.x + other->m_cylinder2Position.x) > epsilon) return __LINE__;
//        if (fabs(this->m_cylinder2Position.y - other->m_cylinder2Position.y) > epsilon) return __LINE__;
//        if (fabs(this->m_cylinder2Position.z - other->m_cylinder2Position.z) > epsilon) return __LINE__;
//        break;

//    case Simulation::YAxis:
//        if (fabs(this->m_originPosition.x - other->m_originPosition.x) > epsilon) return __LINE__;
//        if (fabs(this->m_originPosition.y + other->m_originPosition.y) > epsilon) return __LINE__;
//        if (fabs(this->m_originPosition.z - other->m_originPosition.z) > epsilon) return __LINE__;
//        if (fabs(this->m_insertionPosition.x - other->m_insertionPosition.x) > epsilon) return __LINE__;
//        if (fabs(this->m_insertionPosition.y + other->m_insertionPosition.y) > epsilon) return __LINE__;
//        if (fabs(this->m_insertionPosition.z - other->m_insertionPosition.z) > epsilon) return __LINE__;
//        if (fabs(this->m_cylinder1Position.x - other->m_cylinder1Position.x) > epsilon) return __LINE__;
//        if (fabs(this->m_cylinder1Position.y + other->m_cylinder1Position.y) > epsilon) return __LINE__;
//        if (fabs(this->m_cylinder1Position.z - other->m_cylinder1Position.z) > epsilon) return __LINE__;
//        if (fabs(this->m_cylinder2Position.x - other->m_cylinder2Position.x) > epsilon) return __LINE__;
//        if (fabs(this->m_cylinder2Position.y + other->m_cylinder2Position.y) > epsilon) return __LINE__;
//        if (fabs(this->m_cylinder2Position.z - other->m_cylinder2Position.z) > epsilon) return __LINE__;
//        break;

//    case Simulation::ZAxis:
//        if (fabs(this->m_originPosition.x - other->m_originPosition.x) > epsilon) return __LINE__;
//        if (fabs(this->m_originPosition.y - other->m_originPosition.y) > epsilon) return __LINE__;
//        if (fabs(this->m_originPosition.z + other->m_originPosition.z) > epsilon) return __LINE__;
//        if (fabs(this->m_insertionPosition.x - other->m_insertionPosition.x) > epsilon) return __LINE__;
//        if (fabs(this->m_insertionPosition.y - other->m_insertionPosition.y) > epsilon) return __LINE__;
//        if (fabs(this->m_insertionPosition.z + other->m_insertionPosition.z) > epsilon) return __LINE__;
//        if (fabs(this->m_cylinder1Position.x - other->m_cylinder1Position.x) > epsilon) return __LINE__;
//        if (fabs(this->m_cylinder1Position.y - other->m_cylinder1Position.y) > epsilon) return __LINE__;
//        if (fabs(this->m_cylinder1Position.z + other->m_cylinder1Position.z) > epsilon) return __LINE__;
//        if (fabs(this->m_cylinder2Position.x - other->m_cylinder2Position.x) > epsilon) return __LINE__;
//        if (fabs(this->m_cylinder2Position.y - other->m_cylinder2Position.y) > epsilon) return __LINE__;
//        if (fabs(this->m_cylinder2Position.z + other->m_cylinder2Position.z) > epsilon) return __LINE__;
//        break;
//    }

//    // now check for left to right crossover errors
//    if (this->name().find(sanityCheckLeft) != std::string::npos)
//    {
//        if (m_originBody->name().find(sanityCheckRight) != std::string::npos) return __LINE__;
//        if (m_insertionBody->name().find(sanityCheckRight) != std::string::npos) return __LINE__;
//        if (m_cylinder1Body->name().find(sanityCheckRight) != std::string::npos) return __LINE__;
//        if (m_cylinder2Body->name().find(sanityCheckRight) != std::string::npos) return __LINE__;
//    }
//    if (this->name().find(sanityCheckRight) != std::string::npos)
//    {
//        if (m_originBody->name().find(sanityCheckLeft) != std::string::npos) return __LINE__;
//        if (m_insertionBody->name().find(sanityCheckLeft) != std::string::npos) return __LINE__;
//        if (m_cylinder1Body->name().find(sanityCheckLeft) != std::string::npos) return __LINE__;
//        if (m_cylinder2Body->name().find(sanityCheckLeft) != std::string::npos) return __LINE__;
//    }

//    return 0;
//}

std::string *TwoCylinderWrapStrap::createFromAttributes()
{
    if (Strap::createFromAttributes()) return lastErrorPtr();

    std::string buf;

    if (findAttribute("OriginMarkerID"s, &buf) == nullptr) return lastErrorPtr();
    auto originMarker = simulation()->GetMarkerList()->find(buf);
    if (originMarker == simulation()->GetMarkerList()->end())
    {
        setLastError("STRAP ID=\""s + name() +"\" OriginMarker not found"s);
        return lastErrorPtr();
    }
    this->SetOrigin(originMarker->second.get());
    if (findAttribute("InsertionMarkerID"s, &buf) == nullptr) return lastErrorPtr();
    auto insertionMarker = simulation()->GetMarkerList()->find(buf);
    if (insertionMarker == simulation()->GetMarkerList()->end())
    {
        setLastError("STRAP ID=\""s + name() +"\" InsertionMarker not found"s);
        return lastErrorPtr();
    }
    this->SetInsertion(insertionMarker->second.get());
    if (findAttribute("Cylinder1MarkerID"s, &buf) == nullptr) return lastErrorPtr();
    auto cylinder1Marker = simulation()->GetMarkerList()->find(buf);
    if (cylinder1Marker == simulation()->GetMarkerList()->end())
    {
        setLastError("STRAP ID=\""s + name() +"\" Cylinder1Marker not found"s);
        return lastErrorPtr();
    }
    this->SetCylinder1(cylinder1Marker->second.get());
    if (findAttribute("Cylinder2MarkerID"s, &buf) == nullptr) return lastErrorPtr();
    auto cylinder2Marker = simulation()->GetMarkerList()->find(buf);
    if (cylinder2Marker == simulation()->GetMarkerList()->end())
    {
        setLastError("STRAP ID=\""s + name() +"\" Cylinder2Marker not found"s);
        return lastErrorPtr();
    }
    this->SetCylinder2(cylinder2Marker->second.get());
    if (findAttribute("Cylinder1Radius"s, &buf) == nullptr) return lastErrorPtr();
    this->SetCylinder1Radius(GSUtil::Double(buf.c_str()));
    if (findAttribute("Cylinder2Radius"s, &buf) == nullptr) return lastErrorPtr();
    this->SetCylinder2Radius(GSUtil::Double(buf.c_str()));

    setUpstreamObjects({m_originMarker, m_insertionMarker, m_cylinder1Marker, m_cylinder2Marker});
    return nullptr;
}

void TwoCylinderWrapStrap::appendToAttributes()
{
    Strap::appendToAttributes();
    std::string buf;
    setAttribute("Type"s, "TwoCylinderWrap"s);
    setAttribute("OriginMarkerID"s, m_originMarker->name());
    setAttribute("InsertionMarkerID"s, m_insertionMarker->name());
    setAttribute("Cylinder1MarkerID"s, m_cylinder1Marker->name());
    setAttribute("Cylinder1Radius"s, *GSUtil::ToString(m_cylinder1Radius, &buf));
    setAttribute("Cylinder2MarkerID"s, m_cylinder2Marker->name());
    setAttribute("Cylinder2Radius"s, *GSUtil::ToString(m_cylinder2Radius, &buf));
}

