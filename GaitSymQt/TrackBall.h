/*
 *  Trackball.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 08/08/2009.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

#ifndef TRACKBALL_H
#define TRACKBALL_H

#include "PGDMath.h"

class Trackball
{
public:
    Trackball();

    // called with the mouse start position and the trackbal dimensions
    // note: clicks outside the trackball radius have a different rotation behaviour
    // note: values given in window coordinates with raster origin at top left
    void StartTrackball(int mouseX, int mouseY, int trackballOriginX, int trackballOriginY,
                        int trackballRadius, const pgd::Vector3 &up, const pgd::Vector3 &out);

    // calculated rotation based on current mouse position
    void RollTrackballToClick(int mouseX, int mouseY, pgd::Quaternion *rotation);

    int GetTrackballRadius()
    {
        return mTrackballRadius;
    }
    bool GetOutsideRadius()
    {
        return mOutsideRadius;
    }

private:

    int mTrackballRadius;
    int mStartMouseX;
    int mStartMouseY;
    int mTrackballOriginX;
    int mTrackballOriginY;
    bool mOutsideRadius;

    pgd::Vector3 mLeft;
    pgd::Vector3 mUp;
    pgd::Vector3 mOut;
};

#endif // TRACKBALL_H
