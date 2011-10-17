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

using namespace osgShadow;

ShadowSettings::ShadowSettings():
    _lightNum(-1),
    _baseShadowTextureUnit(1),
    _useShadowMapTextureOverride(true),
    _textureSize(2048,2048),
    _minimumShadowMapNearFarRatio(0.05),
    _shadowMapProjectionHint(PERSPECTIVE_SHADOW_MAP),
    _perspectiveShadowMapCutOffAngle(2.0),
    _shaderHint(NO_SHADERS),
//    _shaderHint(PROVIDE_FRAGMENT_SHADER),
    _debugDraw(false)
{
}

ShadowSettings::ShadowSettings(const ShadowSettings& ss, const osg::CopyOp& copyop):
    Object(ss,copyop),
    _lightNum(ss._lightNum),
    _baseShadowTextureUnit(ss._baseShadowTextureUnit),
    _useShadowMapTextureOverride(ss._useShadowMapTextureOverride),
    _textureSize(ss._textureSize),
    _minimumShadowMapNearFarRatio(ss._minimumShadowMapNearFarRatio),
    _shadowMapProjectionHint(ss._shadowMapProjectionHint),
    _perspectiveShadowMapCutOffAngle(ss._perspectiveShadowMapCutOffAngle),
    _shaderHint(ss._shaderHint),
    _debugDraw(ss._debugDraw)
{
}

ShadowSettings::~ShadowSettings()
{
}
