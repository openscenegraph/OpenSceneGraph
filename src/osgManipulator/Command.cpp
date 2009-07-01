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
//osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/Command>
#include <osgManipulator/Constraint>

#include <algorithm>

using namespace osgManipulator;

///////////////////////////////////////////////////////////////////////////////
//
// Motion Command base class.
//

MotionCommand::MotionCommand() : _stage(NONE)
{
}

MotionCommand::~MotionCommand()
{
}


///////////////////////////////////////////////////////////////////////////////
//
// Translate in line command.
//

TranslateInLineCommand::TranslateInLineCommand()
{
    _line = new osg::LineSegment;
}

TranslateInLineCommand::TranslateInLineCommand(const osg::LineSegment::vec_type& s, const osg::LineSegment::vec_type& e)
{
    _line = new osg::LineSegment(s,e);
}

TranslateInLineCommand::~TranslateInLineCommand()
{
}

MotionCommand* TranslateInLineCommand::createCommandInverse()
{
    osg::ref_ptr<TranslateInLineCommand> inverse = new TranslateInLineCommand();
    *inverse = *this;
    inverse->setTranslation(-_translation);
    return inverse.release();
}

///////////////////////////////////////////////////////////////////////////////
//
// Translate in plane command.
//

TranslateInPlaneCommand::TranslateInPlaneCommand()
{
}

TranslateInPlaneCommand::TranslateInPlaneCommand(const osg::Plane& plane) : _plane(plane)
{
}

TranslateInPlaneCommand::~TranslateInPlaneCommand()
{
}
MotionCommand* TranslateInPlaneCommand::createCommandInverse()
{
    osg::ref_ptr<TranslateInPlaneCommand> inverse = new TranslateInPlaneCommand();
    *inverse = *this;
    inverse->setTranslation(-_translation);
    return inverse.release();
}

///////////////////////////////////////////////////////////////////////////////
//
// Scale 1D command.
//

Scale1DCommand::Scale1DCommand() : _scale(1.0)
{
}

Scale1DCommand::~Scale1DCommand()
{
}

MotionCommand* Scale1DCommand::createCommandInverse()
{
    osg::ref_ptr<Scale1DCommand> inverse = new Scale1DCommand();
    *inverse = *this;
    if (_scale) inverse->setScale(1.0/_scale);
    return inverse.release();
}

///////////////////////////////////////////////////////////////////////////////
//
// Scale 2D command.
//

Scale2DCommand::Scale2DCommand() : _scale(1.0,1.0)
{
}

Scale2DCommand::~Scale2DCommand()
{
}

MotionCommand* Scale2DCommand::createCommandInverse()
{
    osg::ref_ptr<Scale2DCommand> inverse = new Scale2DCommand();
    *inverse = *this;
    if (_scale[0] && _scale[1])
        inverse->setScale(osg::Vec2(1.0/_scale[0],1.0/_scale[1]));
    return inverse.release();
}

///////////////////////////////////////////////////////////////////////////////
//
// Scale uniform command.
//

ScaleUniformCommand::ScaleUniformCommand() : _scale(1.0)
{
}

ScaleUniformCommand::~ScaleUniformCommand()
{
}

MotionCommand* ScaleUniformCommand::createCommandInverse()
{
    osg::ref_ptr<ScaleUniformCommand> inverse = new ScaleUniformCommand();
    *inverse = *this;
    if (_scale) inverse->setScale(1.0/_scale);

    return inverse.release();
}

///////////////////////////////////////////////////////////////////////////////
//
// Rotate 3D command.
//

Rotate3DCommand::Rotate3DCommand()
{
}

Rotate3DCommand::~Rotate3DCommand()
{
}

MotionCommand* Rotate3DCommand::createCommandInverse()
{
    osg::ref_ptr<Rotate3DCommand> inverse = new Rotate3DCommand();
    *inverse = *this;
    inverse->setRotation(_rotation.inverse());
    return inverse.release();
}
