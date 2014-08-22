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

#ifndef OSGUI_STYLE
#define OSGUI_STYLE

#include <osg/Object>
#include <osg/BoundingBox>
#include <osg/Texture2D>
#include <osg/Depth>
#include <osg/ColorMask>
#include <osg/Vec4>

#include <osgUI/AlignmentSettings>
#include <osgUI/FrameSettings>
#include <osgUI/TextSettings>

namespace osgUI
{

class OSGUI_EXPORT Style : public osg::Object
{
public:
    Style();
    Style(const Style& style, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    META_Object(osgUI, Style);

    static osg::ref_ptr<Style>& instance();

    void setBackgroundColor(const osg::Vec4& color) { _backgroundColor = color; }
    const osg::Vec4& getBackgroundColor() const { return _backgroundColor; }

    void setTextColor(const osg::Vec4& color) { _textColor = color; }
    const osg::Vec4& getTextColor() const { return _textColor; }

    void setDisabledTextColor(const osg::Vec4& color) { _disabledTextColor = color; }
    const osg::Vec4& getDisabledTextColor() const { return _disabledTextColor; }

    virtual osg::Node* createDepthSetPanel(const osg::BoundingBox& extents);
    virtual osg::Node* createPanel(const osg::BoundingBox& extents, const osg::Vec4& colour);
    virtual osg::Node* createFrame(const osg::BoundingBox& extents, const FrameSettings* frameSettings, const osg::Vec4& colour);
    virtual osg::Node* createText(const osg::BoundingBox& extents, const AlignmentSettings* as, const TextSettings* textSettings, const std::string& text);
    virtual osg::Node* createIcon(const osg::BoundingBox& extents, const std::string& filename, const osg::Vec4& colour);
    virtual void setupDialogStateSet(osg::StateSet* stateset, int binNum);
    virtual void setupPopupStateSet(osg::StateSet* stateset, int binNum);
    virtual void setupClipStateSet(const osg::BoundingBox& extents, osg::StateSet* stateset);

protected:
    virtual ~Style() {}

    osg::ref_ptr<osg::Depth>     _disabledDepthWrite;
    osg::ref_ptr<osg::Depth>     _enabledDepthWrite;
    osg::ref_ptr<osg::ColorMask> _disableColorWriteMask;
    osg::ref_ptr<osg::Texture2D> _clipTexture;

    osg::Vec4 _backgroundColor;
    osg::Vec4 _textColor;
    osg::Vec4 _disabledTextColor;
};

}

#endif
