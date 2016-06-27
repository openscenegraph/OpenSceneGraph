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
    controlPoints(0),
    reserved10(0),
    numSubtextures(0)
{
    of_mips[0]=of_mips[1]=of_mips[2]=of_mips[3]=of_mips[4]=of_mips[5]=of_mips[6]=of_mips[7]=0.0f;
}

AttrData::AttrData(const AttrData& attr, const osg::CopyOp& copyop) :
    osg::Object(attr,copyop),
    texels_u(attr.texels_u),
    texels_v(attr.texels_v),
    direction_u(attr.direction_u),
    direction_v(attr.direction_v),
    x_up(attr.x_up),
    y_up(attr.y_up),
    fileFormat(attr.fileFormat),
    minFilterMode(attr.minFilterMode),
    magFilterMode(attr.magFilterMode),
    wrapMode(attr.wrapMode),
    wrapMode_u(attr.wrapMode_u),
    wrapMode_v(attr.wrapMode_v),
    modifyFlag(attr.modifyFlag),
    pivot_x(attr.pivot_x),
    pivot_y(attr.pivot_y),
    texEnvMode(attr.texEnvMode),
    intensityAsAlpha(attr.intensityAsAlpha),
    size_u(attr.size_u),
    size_v(attr.size_v),
    originCode(attr.originCode),
    kernelVersion(attr.kernelVersion),
    intFormat(attr.intFormat),
    extFormat(attr.extFormat),

    useMips(attr.useMips),

    useLodScale(attr.useLodScale),
    lod0(attr.lod0),
    scale0(attr.scale0),
    lod1(attr.lod1),
    scale1(attr.scale1),
    lod2(attr.lod2),
    scale2(attr.scale2),
    lod3(attr.lod3),
    scale3(attr.scale3),
    lod4(attr.lod4),
    scale4(attr.scale4),
    lod5(attr.lod5),
    scale5(attr.scale5),
    lod6(attr.lod6),
    scale6(attr.scale6),
    lod7(attr.lod7),
    scale7(attr.scale7),

    clamp(attr.clamp),
    magFilterAlpha(attr.magFilterAlpha),
    magFilterColor(attr.magFilterColor),
    lambertMeridian(attr.lambertMeridian),
    lambertUpperLat(attr.lambertUpperLat),
    lambertlowerLat(attr.lambertlowerLat),
    useDetail(attr.useDetail),
    txDetail_j(attr.txDetail_j),
    txDetail_k(attr.txDetail_k),
    txDetail_m(attr.txDetail_m),
    txDetail_n(attr.txDetail_n),
    txDetail_s(attr.txDetail_s),
    useTile(attr.useTile),
    txTile_ll_u(attr.txTile_ll_u),
    txTile_ll_v(attr.txTile_ll_v),
    txTile_ur_u(attr.txTile_ur_u),
    txTile_ur_v(attr.txTile_ur_v),
    projection(attr.projection),
    earthModel(attr.earthModel),
    utmZone(attr.utmZone),
    imageOrigin(attr.imageOrigin),
    geoUnits(attr.geoUnits),
    hemisphere(attr.hemisphere),
    comments(attr.comments),
    attrVersion(attr.attrVersion),
    controlPoints(attr.controlPoints),
    reserved10(attr.reserved10),
    numSubtextures(attr.numSubtextures)
{
    for(int i=0; i<8; ++i)
    {
        of_mips[i]=attr.of_mips[i];
    }
}