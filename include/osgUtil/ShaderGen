/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2009 Robert Osfield
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

/**
 * \brief    Shader generator framework.
 * \author   Maciej Krol
 */

#ifndef OSGUTIL_SHADER_STATE_
#define OSGUTIL_SHADER_STATE_ 1

#include <osgUtil/Export>
#include <osg/NodeVisitor>
#include <osg/State>

namespace osgUtil
{

class OSGUTIL_EXPORT ShaderGenVisitor : public osg::NodeVisitor
{
public:
    ShaderGenVisitor();

    /// assign default uber program to specified StateSet - typically the root node of the scene graph or the view's Camera
    void assignUberProgram(osg::StateSet *stateSet);

    void apply(osg::Node& node);

    void remapStateSet(osg::StateSet* stateSet);

protected:
};

}

#endif
