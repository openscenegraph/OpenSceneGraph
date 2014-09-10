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

#ifndef OSGUI_ALIGNMENTSETTINGS
#define OSGUI_ALIGNMENTSETTINGS

#include <osg/Object>
#include <osg/BoundingBox>
#include <osg/Vec4>
#include <osgUI/Export>

namespace osgUI
{

class OSGUI_EXPORT AlignmentSettings : public osg::Object
{
public:

    enum Alignment
    {
        LEFT_TOP,
        LEFT_CENTER,
        LEFT_BOTTOM,

        CENTER_TOP,
        CENTER_CENTER,
        CENTER_BOTTOM,

        RIGHT_TOP,
        RIGHT_CENTER,
        RIGHT_BOTTOM,

        LEFT_BASE_LINE,
        CENTER_BASE_LINE,
        RIGHT_BASE_LINE,

        LEFT_BOTTOM_BASE_LINE,
        CENTER_BOTTOM_BASE_LINE,
        RIGHT_BOTTOM_BASE_LINE
    };

    AlignmentSettings(Alignment alignment=AlignmentSettings::LEFT_BOTTOM);
    AlignmentSettings(const AlignmentSettings& as, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    META_Object(osgUI, AlignmentSettings);

    void setAlignment(Alignment alignment) { _alignment = alignment; }
    Alignment getAlignment() const { return _alignment; }

protected:

    virtual ~AlignmentSettings() {}

    Alignment _alignment;
};

}

#endif
