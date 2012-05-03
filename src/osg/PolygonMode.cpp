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
#include <osg/GL>
#include <osg/PolygonMode>
#include <osg/Notify>

using namespace osg;

PolygonMode::PolygonMode():
    _modeFront(FILL),
    _modeBack(FILL)
{
}

PolygonMode::PolygonMode(Face face,Mode mode):
    _modeFront(FILL),
    _modeBack(FILL)
{
    setMode(face,mode);
}


PolygonMode::~PolygonMode()
{
}

void PolygonMode::setMode(Face face,Mode mode)
{
    switch(face)
    {
        case(FRONT):
            _modeFront = mode;
            break;
        case(BACK):
            _modeBack = mode;
            break;
        case(FRONT_AND_BACK):
            _modeFront = mode;
            _modeBack = mode;
            break;
    }
}

PolygonMode::Mode PolygonMode::getMode(Face face) const
{
    switch(face)
    {
        case(FRONT):
            return _modeFront;
        case(BACK):
            return _modeBack;
        case(FRONT_AND_BACK):
            return _modeFront;
    }
    OSG_WARN<<"Warning : invalid Face passed to PolygonMode::getMode(Face face)"<<std::endl;
    return _modeFront;
}

void PolygonMode::apply(State&) const
{
#ifdef OSG_GL1_AVAILABLE
    if (_modeFront==_modeBack)
    {
        glPolygonMode(GL_FRONT_AND_BACK,(GLenum)_modeFront);
    }
    else
    {
        glPolygonMode(GL_FRONT,(GLenum)_modeFront);
        glPolygonMode(GL_BACK,(GLenum)_modeBack);
    }
#else
    OSG_NOTICE<<"Warning: PolygonMode::apply(State&) - not supported."<<std::endl;
#endif
}

