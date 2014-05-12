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

#ifndef OSGUI_TEXTSETTINGS
#define OSGUI_TEXTSETTINGS

#include <osg/Object>
#include <osg/BoundingBox>
#include <osg/Vec4>
#include <osgUI/Export>

namespace osgUI
{

class OSGUI_EXPORT TextSettings : public osg::Object
{
public:
    TextSettings();
    TextSettings(const TextSettings& textSettings, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    META_Object(osgUI, TextSettings);

    void setFont(const std::string& font) { _font = font; }
    const std::string& getFont() const { return _font; }

    void setCharacterSize(float characterSize) { _characterSize = characterSize; }
    float getCharacterSize() const { return _characterSize; }

protected:

    virtual ~TextSettings() {}

    std::string                 _font;
    float                       _characterSize;
};


}

#endif
