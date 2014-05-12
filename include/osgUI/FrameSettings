/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2014 Robert Osfield
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

#ifndef OSGUI_FRAMESETTINGS
#define OSGUI_FRAMESETTINGS

#include <osg/Object>
#include <osg/BoundingBox>
#include <osg/Vec4>
#include <osgUI/Export>

namespace osgUI
{
class OSGUI_EXPORT FrameSettings : public osg::Object
{
public:
    FrameSettings();
    FrameSettings(const FrameSettings& frameSettings, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    META_Object(osgUI, FrameSettings);

    enum Shape
    {
        NO_FRAME,
        BOX,
        PANEL
    };

    void setShape(Shape shape) { _shape = shape; }
    Shape getShape() const { return _shape; }

    enum Shadow
    {
        PLAIN,
        SUNKEN,
        RAISED
    };

    void setShadow(Shadow shadow) { _shadow = shadow; }
    Shadow getShadow() const { return _shadow; }

    void setLineWidth(float width) { _lineWidth = width; }
    float getLineWidth() const { return _lineWidth; }

protected:

    virtual ~FrameSettings() {}

    Shape       _shape;
    Shadow      _shadow;
    float       _lineWidth;

};

}

#endif
