/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
//

#include "AttrData.h"

using namespace flt;


AttrData::AttrData() :
    texels_u(0),
    texels_v(0),
    direction_u(0),
    direction_v(0),
    x_up(0),
    y_up(0),
    fileFormat(-1),                //   -1 Not used
    minFilterMode(MIN_FILTER_NONE),
    magFilterMode(MAG_FILTER_POINT),
    wrapMode(WRAP_REPEAT),
    wrapMode_u(WRAP_REPEAT),
    wrapMode_v(WRAP_REPEAT),
    modifyFlag(0),
    pivot_x(0),
    pivot_y(0),
    texEnvMode(TEXENV_MODULATE),
    intensityAsAlpha(0),
    size_u(0),
    size_v(0),
    originCode(0),
    kernelVersion(0),
    intFormat(0),                  //    0 - Default
    extFormat(0),                  //    0 - Default

    useMips(0),

    useLodScale(0),
    lod0(0.0f),
    scale0(1.0f),
    lod1(0.0f),
    scale1(1.0f),
    lod2(0.0f),
    scale2(1.0f),
    lod3(0.0f),
    scale3(1.0f),
    lod4(0.0f),
    scale4(1.0f),
    lod5(0.0f),
    scale5(1.0f),
    lod6(0.0f),
    scale6(1.0f),
    lod7(0.0f),
    scale7(1.0f),

    clamp(0),
    magFilterAlpha(2),             //    2 = None
    magFilterColor(2),             //    2 = None
    lambertMeridian(0),
    lambertUpperLat(0),
    lambertlowerLat(0),
    useDetail(0),
    txDetail_j(0),
    txDetail_k(0),
    txDetail_m(0),
    txDetail_n(0),
    txDetail_s(0),
    useTile(0),
    txTile_ll_u(0),
    txTile_ll_v(0),
    txTile_ur_u(0),
    txTile_ur_v(0),
    projection(PROJECTION_UNDEFINED),
    earthModel(DATUM_WGS84),
    utmZone(0),
    imageOrigin(0),
    geoUnits(0),
    hemisphere(1),
    comments(""),
    attrVersion(0),
    controlPoints(0)
    // TODO:
{
    of_mips[0]=of_mips[1]=of_mips[2]=of_mips[3]=of_mips[4]=of_mips[5]=of_mips[6]=of_mips[7]=0.0f;
}

AttrData::AttrData(const AttrData& attr, const osg::CopyOp& copyop) :
    osg::Object(attr,copyop)
{}

