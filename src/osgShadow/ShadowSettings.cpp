/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2011 Robert Osfield
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

#include <osgShadow/ShadowSettings>
#include <float.h>

using namespace osgShadow;

ShadowSettings::ShadowSettings():
    _receivesShadowTraversalMask(0xffffffff),
    _castsShadowTraversalMask(0xffffffff),
    _computeNearFearModeOverride(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR),
    _lightNum(-1),
    _baseShadowTextureUnit(1),
    _useShadowMapTextureOverride(true),
    _textureSize(2048,2048),
    _minimumShadowMapNearFarRatio(0.05),
    _maximumShadowMapDistance(DBL_MAX),
    _shadowMapProjectionHint(PERSPECTIVE_SHADOW_MAP),
    _perspectiveShadowMapCutOffAngle(2.0),
    _numShadowMapsPerLight(1),
    _multipleShadowMapHint(PARALLEL_SPLIT),
    _shaderHint(NO_SHADERS),
//    _shaderHint(PROVIDE_FRAGMENT_SHADER),
    _debugDraw(false)
{
    //_computeNearFearModeOverride = osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES;
    //_computeNearFearModeOverride = osg::CullSettings::COMPUTE_NEAR_USING_PRIMITIVES);
}

ShadowSettings::ShadowSettings(const ShadowSettings& ss, const osg::CopyOp& copyop):
    Object(ss,copyop),
    _receivesShadowTraversalMask(ss._receivesShadowTraversalMask),
    _castsShadowTraversalMask(ss._castsShadowTraversalMask),
    _computeNearFearModeOverride(ss._computeNearFearModeOverride),
    _lightNum(ss._lightNum),
    _baseShadowTextureUnit(ss._baseShadowTextureUnit),
    _useShadowMapTextureOverride(ss._useShadowMapTextureOverride),
    _textureSize(ss._textureSize),
    _minimumShadowMapNearFarRatio(ss._minimumShadowMapNearFarRatio),
    _maximumShadowMapDistance(ss._maximumShadowMapDistance),
    _shadowMapProjectionHint(ss._shadowMapProjectionHint),
    _perspectiveShadowMapCutOffAngle(ss._perspectiveShadowMapCutOffAngle),
    _numShadowMapsPerLight(ss._numShadowMapsPerLight),
    _multipleShadowMapHint(ss._multipleShadowMapHint),
    _shaderHint(ss._shaderHint),
    _debugDraw(ss._debugDraw)
{
}

ShadowSettings::~ShadowSettings()
{
}
