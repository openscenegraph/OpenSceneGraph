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

#ifndef OSGUI_COLORPALETTE
#define OSGUI_COLORPALETTE

#include <osg/Object>
#include <osg/Vec4>
#include <osgUI/Export>

namespace osgUI
{

class OSGUI_EXPORT ColorPalette : public osg::Object
{
public:
    ColorPalette();
    ColorPalette(const ColorPalette& cp, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    META_Object(osgUI, ColorPalette);


    typedef std::vector<osg::Vec4f> Colors;

    void setColors(const Colors& colors) { _colors = colors; }
    Colors& getColors() { return _colors; }
    const Colors& getColors() const { return _colors; }

    typedef std::vector<std::string> Names;

    void setNames(const Names& names) { _names = names; }
    Names& getNames() { return _names; }
    const Names& getNames() const { return _names; }


protected:

    virtual ~ColorPalette() {}

    Colors      _colors;
    Names       _names;

};

}

#endif
