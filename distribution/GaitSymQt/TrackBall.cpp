/*
 *  Trackball.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 08/08/2009.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

#include <cmath>

#include "TrackBall.h"

Trackball::Trackball()
{
}


// called with the mouse start position and the trackbal dimensions
// note: clicks outside the trackball radius have a different rotation behaviour
// note: values given in window coordinates with raster origin at top left
void Trackball::StartTrackball(int mouseX, int mouseY, int trackballOriginX, int trackballOriginY,
                               int trackballRadius, const pgd::Vector3 &up, const pgd::Vector3 &out)
{
    mTrackballRadius = trackballRadius;
    mStartMouseX = mouseX;
    mStartMouseY = mouseY;
    mTrackballOriginX = trackballOriginX;
    mTrackballOriginY = trackballOriginY;
    mOut = out;
    mUp = up;
    mOut.Normalize();
    mUp.Normalize();
    mLeft = mUp ^ mOut;
    mLeft.Normalize();

    double dx = mStartMouseX - mTrackballOriginX;
    double dy = mTrackballOriginY - mStartMouseY;
    double r = sqrt(dx * dx + dy * dy);
    if (r > trackballRadius) mOutsideRadius = true;
    else mOutsideRadius = false;
}

// calculated rotation based on current mouse position
void Trackball::RollTrackballToClick(int mouseX, int mouseY, pgd::Quaternion *rotation)
{
    if (mouseX == mStartMouseX && mouseY == mStartMouseY)
    {
        rotation->n = 1;
        rotation->x = rotation->y = rotation->z = 0;
        return;
    }
    pgd::Vector3 v1;
    pgd::Vector3 v2;
    if (mOutsideRadius == false)   // normal behaviour
    {
        v1 = (mStartMouseX - mTrackballOriginX) * mLeft + (mTrackballOriginY - mStartMouseY) * mUp +
             mTrackballRadius * mOut;
        v2 = (mouseX - mTrackballOriginX) * mLeft + (mTrackballOriginY - mouseY) * mUp + mTrackballRadius *
             mOut;

    }
    else     // rotate around axis coming out of screen
    {
        v1 = (mStartMouseX - mTrackballOriginX) * mLeft + (mTrackballOriginY - mStartMouseY) * mUp;
        v2 = (mouseX - mTrackballOriginX) * mLeft + (mTrackballOriginY - mouseY) * mUp;
    }

    // cross product will get us the rotation axis
    pgd::Vector3 axis = v1 ^ v2;

    // Use atan for a better angle.  If you use only cos or sin, you only get
    // half the possible angles, and you can end up with rotations that flip around near
    // the poles.

    // cos angle obtained from dot product formula
    // cos(a) = (s . e) / (||s|| ||e||)
    double cosAng = v1 * v2; // (s . e)
    double ls = v1.Magnitude();
    ls = 1. / ls; // 1 / ||s||
    double le = v2.Magnitude();
    le = 1. / le; // 1 / ||e||
    cosAng = cosAng * ls * le;

    // sin angle obtained from cross product formula
    // sin(a) = ||(s X e)|| / (||s|| ||e||)
    double sinAng = axis.Magnitude(); // ||(s X e)||;
    sinAng = sinAng * ls * le;
    double angle = atan2(sinAng, cosAng); // rotations are in radians.

    *rotation = pgd::MakeQFromAxisAngle(axis.x, axis.y, axis.z, angle);
}

