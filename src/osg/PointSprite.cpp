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
 
#include <osg/GLExtensions>
#include <osg/GL>
#include <osg/PointSprite>
#include <osg/Point>
#include <osg/State>
#include <osg/buffered_value>
#include <osg/Notify>

using namespace osg;

PointSprite::PointSprite()
    : _coordOriginMode(UPPER_LEFT)
{
}

PointSprite::~PointSprite()
{
}

int PointSprite::compare(const StateAttribute& sa) const
{
    COMPARE_StateAttribute_Types(PointSprite,sa)

    COMPARE_StateAttribute_Parameter(_coordOriginMode)

    return 0; // passed all the above comparison macro's, must be equal.
}


bool PointSprite::checkValidityOfAssociatedModes(osg::State& state) const
{

    bool modeValid = isPointSpriteSupported(state.getContextID());
    state.setModeValidity(GL_POINT_SPRITE_ARB, modeValid);

    return modeValid;
}

void PointSprite::apply(osg::State& state) const
{
    if(!isPointSpriteSupported(state.getContextID())) return;

    glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, 1);

    const Point::Extensions* extensions = Point::getExtensions(state.getContextID(),true);

    if (extensions->isPointSpriteCoordOriginSupported())
        extensions->glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN,_coordOriginMode);
}

struct IntializedSupportedPair
{
    IntializedSupportedPair():
        initialized(false),
        supported(false) {}

    bool initialized;
    bool supported;
};

typedef osg::buffered_object< IntializedSupportedPair > BufferedExtensions;
static BufferedExtensions s_extensions;

bool PointSprite::isPointSpriteSupported(unsigned int contextID)
{
    if (!s_extensions[contextID].initialized)
    {
        s_extensions[contextID].initialized = true;
        s_extensions[contextID].supported = isGLExtensionSupported(contextID, "GL_ARB_point_sprite") || isGLExtensionSupported(contextID, "GL_NV_point_sprite");
    }

    return s_extensions[contextID].supported;
}
