/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2010 Robert Osfield
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

#include "TextNode.h"
#include <osg/io_utils>

using namespace osgText;

/////////////////////////////////////////////////////////////////////////////////////////
//
// Bevel
//
Bevel::Bevel()
{
    _thickness = 0.1f;
    flatBevel();
}

Bevel::Bevel(const Bevel& bevel, const osg::CopyOp&):
    _thickness(bevel._thickness),
    _vertices(bevel._vertices)
{
}

void Bevel::flatBevel(float width)
{
    _vertices.clear();

    if (width>0.5f) width = 0.5f;

    _vertices.push_back(osg::Vec2(0.0f,0.0f));

    _vertices.push_back(osg::Vec2(width,1.0f));

    if (width<0.5f) _vertices.push_back(osg::Vec2(1-width,1.0f));

    _vertices.push_back(osg::Vec2(1.0f,0.0f));
}

void Bevel::roundedBevel(float width, unsigned int numSteps)
{
    _vertices.clear();

    if (width>0.5f) width = 0.5f;

    unsigned int i = 0;
    for(; i<=numSteps; ++i)
    {
        float angle = float(osg::PI)*0.5f*(float(i)/float(numSteps));
        _vertices.push_back( osg::Vec2((1.0f-cosf(angle))*width, sinf(angle)) );
    }

    // start the second half one into the curve if the width is half way across
    i = width<0.5f ? 0 : 1;
    for(; i<=numSteps; ++i)
    {
        float angle = float(osg::PI)*0.5f*(float(numSteps-i)/float(numSteps));
        _vertices.push_back( osg::Vec2(1.0-(1.0f-cosf(angle))*width, sin(angle)) );
    }
}

void Bevel::roundedBevel2(float width, unsigned int numSteps)
{
    _vertices.clear();

    if (width>0.5f) width = 0.5f;

    float h = 0.1f;
    float r = 1.0f-h;

    _vertices.push_back(osg::Vec2(0.0,0.0));

    unsigned int i = 0;
    for(; i<=numSteps; ++i)
    {
        float angle = float(osg::PI)*0.5f*(float(i)/float(numSteps));
        _vertices.push_back( osg::Vec2((1.0f-cosf(angle))*width, h + sinf(angle)*r) );
    }

    // start the second half one into the curve if the width is half way across
    i = width<0.5f ? 0 : 1;
    for(; i<=numSteps; ++i)
    {
        float angle = float(osg::PI)*0.5f*(float(numSteps-i)/float(numSteps));
        _vertices.push_back( osg::Vec2(1.0-(1.0f-cosf(angle))*width, h + sin(angle)*r) );
    }

    _vertices.push_back(osg::Vec2(1.0,0.0));

}

void Bevel::print(std::ostream& fout)
{
    OSG_NOTICE<<"print bevel"<<std::endl;
    for(Vertices::iterator itr = _vertices.begin();
        itr != _vertices.end();
        ++itr)
    {
        OSG_NOTICE<<"  "<<*itr<<std::endl;
    }
}


/////////////////////////////////////////////////////////////////////////////////////////
//
// Style
//
Style::Style():
    _widthRatio(1.0f),
    _thicknessRatio(0.0f),
    _outlineRatio(0.0f),
    _sampleDensity(1.0f)
{
}

Style::Style(const Style& style, const osg::CopyOp& copyop):
    osg::Object(style,copyop),
    _bevel(dynamic_cast<Bevel*>(copyop(style._bevel.get()))),
    _widthRatio(style._widthRatio),
    _thicknessRatio(style._thicknessRatio),
    _outlineRatio(style._outlineRatio),
    _sampleDensity(style._sampleDensity)
{
}

/// default Layout implementation used if no other is specified on TextNode
osg::ref_ptr<Style>& Style::getDefaultStyle()
{
    static OpenThreads::Mutex s_DefaultStyleMutex;
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_DefaultStyleMutex);

    static osg::ref_ptr<Style> s_defaultStyle = new Style;
    return s_defaultStyle;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Layout
//
Layout::Layout()
{
}

Layout::Layout(const Layout& layout, const osg::CopyOp& copyop):
    osg::Object(layout,copyop)
{
}

osg::ref_ptr<Layout>& Layout::getDefaultLayout()
{
    static OpenThreads::Mutex s_DefaultLayoutMutex;
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_DefaultLayoutMutex);

    static osg::ref_ptr<Layout> s_defaultLayout = new Layout;
    return s_defaultLayout;
}

void Layout::layout(TextNode& text) const
{
    OSG_NOTICE<<"Layout::layout"<<std::endl;

    Font* font = text.getActiveFont();
    Style* style = text.getActiveStyle();
    TextTechnique* technique = text.getTextTechnique();

    if (!text.getTextTechnique())
    {
        OSG_NOTICE<<"Warning: no TextTechnique assigned to Layout"<<std::endl;
        return;
    }

    technique->start();
    technique->finish();
}


/////////////////////////////////////////////////////////////////////////////////////////
//
// TextTechnique
//
TextTechnique::TextTechnique():
    _textNode(0)
{
}


TextTechnique::TextTechnique(const TextTechnique& technique, const osg::CopyOp& copyop):
    osg::Object(technique, copyop),
    _textNode(0)
{
}

osg::ref_ptr<TextTechnique>& TextTechnique::getDefaultTextTechinque()
{
    static OpenThreads::Mutex s_DefaultTextTechniqueMutex;
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_DefaultTextTechniqueMutex);

    static osg::ref_ptr<TextTechnique> s_defaultTextTechnique = new TextTechnique;
    return s_defaultTextTechnique;
}

void TextTechnique::start()
{
    OSG_NOTICE<<"TextTechnique::start()"<<std::endl;
}

void TextTechnique::addCharacter(const osg::Vec3& position, const osg::Vec3& size, Glyph* glyph, Style* style)
{
    OSG_NOTICE<<"TextTechnique::addCharacter("<<position<<", "<<size<<", "<<glyph<<", "<<style<<")"<<std::endl;
}

void TextTechnique::finish()
{
    OSG_NOTICE<<"TextTechnique::finish()"<<std::endl;
}

void TextTechnique::traverse(osg::NodeVisitor& nv)
{
    OSG_NOTICE<<"TextTechnique::traverse()"<<std::endl;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// TextNode
//
TextNode::TextNode()
{
}


TextNode::TextNode(const TextNode& text, const osg::CopyOp& copyop):
    osg::Group(text, copyop)
{
}

TextNode::~TextNode()
{
    setTextTechnique(0);
}

void TextNode::traverse(osg::NodeVisitor& nv)
{
    if (_technique.valid())
    {
        _technique->traverse(nv);
    }
    else
    {
        Group::traverse(nv);
    }
}

void TextNode::setTextTechnique(TextTechnique* technique)
{
    if (_technique==technique) return;

    if (TextTechnique::getDefaultTextTechinque()==technique)
    {
        OSG_NOTICE<<"Warning: Attempt to assign DefaultTextTechnique() prototype to TextNode::setTextTechnique(..), assigning a clone() of it instead."<<std::endl;
        technique = new TextTechnique(*TextTechnique::getDefaultTextTechinque());
    }

    if (_technique.valid()) _technique->setTextNode(0);

    _technique = technique;

    if (_technique.valid()) _technique->setTextNode(this);
}


void TextNode::update()
{
    getActiveLayout()->layout(*this);
}

void TextNode::setText(const std::string& str)
{
    _string.set(str);
}
