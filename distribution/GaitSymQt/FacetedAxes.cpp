/*
 *  FacetedAxes.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 13/10/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

// create a 3d set of axes with red x, green y and blue z

#include "FacetedAxes.h"
#include "FacetedBox.h"
#include "FacetedConicSegment.h"

FacetedAxes::FacetedAxes()
{
    // the central cube
    double widthFraction = 0.1;
    FacetedBox cuboid(widthFraction, widthFraction, widthFraction, QColor(220, 220, 220, 255), 1);
    bool useDisplayRotation = false;
    bool useDirectAccess = true;
    AddFacetedObject(&cuboid, useDisplayRotation, useDirectAccess);

    size_t tesselationCylinder = 64;
    size_t tesselationCone = 128;
    double height = 1;
    double lengthCylinder = height * (1 - 2 * widthFraction);
    double heightCone = widthFraction;
    double widthCylinder = height * 0.25 * widthFraction;
    double widthCone = height * widthFraction * 0.5;

    QColor colorCylinder1(255, 0, 0, 255); // red
    FacetedConicSegment cylinder1(lengthCylinder, widthCylinder, widthCylinder, tesselationCylinder, 0, 0, -lengthCylinder / 2, colorCylinder1, 1);
    cylinder1.Rotate(0, 1, 0, 90);
    cylinder1.Move((widthFraction * 0.5 + lengthCylinder * 0.5), 0, 0);
    AddFacetedObject(&cylinder1, useDisplayRotation, useDirectAccess);

    FacetedConicSegment cone1(heightCone, widthCone, 0, tesselationCone, 0, 0, -heightCone / 2, colorCylinder1, 1);
    cone1.Rotate(0, 1, 0, 90);
    cone1.Move((1 - heightCone), 0, 0);
    AddFacetedObject(&cone1, useDisplayRotation, useDirectAccess);

    QColor colorCylinder2(0, 255, 0, 255); // green
    FacetedConicSegment cylinder2(lengthCylinder, widthCylinder, widthCylinder, tesselationCylinder, 0, 0, -lengthCylinder / 2, colorCylinder2, 1);
    cylinder2.Rotate(1, 0, 0, -90);
    cylinder2.Move(0, (widthFraction * 0.5 + lengthCylinder * 0.5), 0);
    AddFacetedObject(&cylinder2, useDisplayRotation, useDirectAccess);

    FacetedConicSegment cone2(heightCone, widthCone, 0, tesselationCone, 0, 0, -heightCone / 2, colorCylinder2, 1);
    cone2.Rotate(1, 0, 0, -90);
    cone2.Move(0, (1 - heightCone), 0);
    AddFacetedObject(&cone2, useDisplayRotation, useDirectAccess);

    QColor colorCylinder3(0, 0, 255, 255); // blue
    FacetedConicSegment cylinder3(lengthCylinder, widthCylinder, widthCylinder, tesselationCylinder, 0, 0, -lengthCylinder / 2, colorCylinder3, 1);
    cylinder3.Rotate(1, 0, 0, 0);
    cylinder3.Move(0, 0, (widthFraction * 0.5 + lengthCylinder * 0.5));
    AddFacetedObject(&cylinder3, useDisplayRotation, useDirectAccess);

    FacetedConicSegment cone3(heightCone, widthCone, 0, tesselationCone, 0, 0, -heightCone / 2, colorCylinder3, 1);
    cone3.Rotate(1, 0, 0, 0);
    cone3.Move(0, 0, (1 - heightCone));
    AddFacetedObject(&cone3, useDisplayRotation, useDirectAccess);
}
