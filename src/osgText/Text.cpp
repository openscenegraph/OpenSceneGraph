/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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
#include <osg/Math>
#include <osgUtil/CullVisitor>
#include <osgDB/ReadFile>
#include <osgText/Text>

#include "DefaultFont.h"

using namespace osgText;

//#define TREES_CODE_FOR_MAKING_SPACES_EDITABLE


Text::Text():
    _fontWidth(32),
    _fontHeight(32),
    _characterHeight(32),
    _characterAspectRatio(1.0f),
    _characterSizeMode(OBJECT_COORDS),
    _maximumWidth(0.0f),
    _maximumHeight(0.0f),
    _alignment(BASE_LINE),
    _autoRotateToScreen(false),
    _layout(LEFT_TO_RIGHT),
    _color(1.0f,1.0f,1.0f,1.0f),
    _drawMode(TEXT),
    _kerningType(KERNING_DEFAULT)
{
    setUseDisplayList(false);
}

Text::Text(const Text& text,const osg::CopyOp& copyop):
    osg::Drawable(text,copyop),
    _font(text._font),
    _fontWidth(text._fontWidth),
    _fontHeight(text._fontHeight),
    _characterHeight(text._characterHeight),
    _characterAspectRatio(text._characterAspectRatio),
    _characterSizeMode(text._characterSizeMode),
    _maximumWidth(text._maximumWidth),
    _maximumHeight(text._maximumHeight),
    _text(text._text),
    _position(text._position),
    _alignment(text._alignment),
    _rotation(text._rotation),
    _autoRotateToScreen(text._autoRotateToScreen),
    _layout(text._layout),
    _color(text._color),
    _drawMode(text._drawMode),
    _kerningType(text._kerningType)
{
    computeGlyphRepresentation();
}

Text::~Text()
{
}

void Text::setFont(Font* font)
{
    if (_font==font) return;
    
    _font = font;
    
    computeGlyphRepresentation();
}

void Text::setFont(const std::string& fontfile)
{
    setFont(readFontFile(fontfile));
}

void Text::setFontResolution(unsigned int width, unsigned int height)
{
    _fontWidth = width;
    _fontHeight = height;
    computeGlyphRepresentation();
}

void Text::setCharacterSize(float height,float aspectRatio)
{
    _characterHeight = height;
    _characterAspectRatio = aspectRatio;
    computeGlyphRepresentation();
}

void Text::setMaximumWidth(float maximumWidth)
{
    _maximumWidth = maximumWidth;
    computeGlyphRepresentation();
}

void  Text::setMaximumHeight(float maximumHeight)
{
    _maximumHeight = maximumHeight;
    computeGlyphRepresentation();
}
    

void Text::setText(const String& text)
{
    if (_text==text) return;
    
    _text = text;
    computeGlyphRepresentation();
}

void Text::setText(const std::string& text)
{
    setText(String(text));
}

void Text::setText(const std::string& text,String::Encoding encoding)
{
    setText(String(text,encoding));
}
    

void Text::setText(const wchar_t* text)
{
    setText(String(text));
}

void Text::setPosition(const osg::Vec3& pos)
{
    if (_position==pos) return;

    _position = pos;
    computePositions();
}

void Text::setAlignment(AlignmentType alignment)
{
    if (_alignment==alignment) return;
    
    _alignment = alignment;
    computePositions();
}

void Text::setAxisAlignment(AxisAlignment axis)
{
    switch(axis)
    {
    case XZ_PLANE:
        setRotation(osg::Quat(osg::inDegrees(90.0f),osg::Vec3(1.0f,0.0f,0.0f))); 
        break;
    case YZ_PLANE:  
        setRotation(osg::Quat(osg::inDegrees(90.0f),osg::Vec3(1.0f,0.0f,0.0f))*
                    osg::Quat(osg::inDegrees(90.0f),osg::Vec3(0.0f,0.0f,1.0f)));
        break;
    case XY_PLANE:
        setRotation(osg::Quat());  // nop - already on XY plane.
        break;
    case SCREEN:
        setAutoRotateToScreen(true);
        break;
    }
}

void Text::setRotation(const osg::Quat& quat)
{
    _rotation = quat;
    computePositions();
}


void Text::setAutoRotateToScreen(bool autoRotateToScreen)
{
    if (_autoRotateToScreen==autoRotateToScreen) return;
    
    _autoRotateToScreen = autoRotateToScreen;
}


void Text::setLayout(Layout layout)
{
    if (_layout==layout) return;

    _layout = layout;
    computeGlyphRepresentation();
}

void Text::setColor(const osg::Vec4& color)
{
    _color = color;
}

void Text::setDrawMode(unsigned int mode) 
{ 
    if (_drawMode==mode) return;

    _drawMode=mode;
}


bool Text::computeBound() const
{
    _bbox.init();

    if (_textBB.valid())
    {
    
        for(unsigned int i=0;i<_autoTransformCache.size();++i)
        {
            osg::Matrix& matrix = _autoTransformCache[i]._matrix;
            _bbox.expandBy(osg::Vec3(_textBB.xMin(),_textBB.yMin(),_textBB.zMin())*matrix);
            _bbox.expandBy(osg::Vec3(_textBB.xMax(),_textBB.yMin(),_textBB.zMin())*matrix);
            _bbox.expandBy(osg::Vec3(_textBB.xMax(),_textBB.yMax(),_textBB.zMin())*matrix);
            _bbox.expandBy(osg::Vec3(_textBB.xMin(),_textBB.yMax(),_textBB.zMin())*matrix);
        }
    }
    
    _bbox_computed = true;
    return true;
}

Font* Text::getActiveFont()
{
    return _font.valid() ? _font.get() : DefaultFont::instance();
}

const Font* Text::getActiveFont() const
{
    return _font.valid() ? _font.get() : DefaultFont::instance();
}

String::iterator Text::computeLastCharacterOnLine(osg::Vec2 cursor, String::iterator first,String::iterator last)
{
    Font* activefont = getActiveFont();
    if (!activefont) return last;

    float hr = _characterHeight/(float)activefont->getHeight();
    float wr = hr/_characterAspectRatio;

    bool horizontal = _layout!=VERTICAL;
    bool kerning = true;
    unsigned int previous_charcode = 0;

    for(String::iterator itr=first;itr!=last;++itr)
    {
        unsigned int charcode = *itr;
        
        if (charcode=='\n')
        {
            return itr;
        }

        Font::Glyph* glyph = activefont->getGlyph(charcode);
        if (glyph)
        {

            float width = (float)(glyph->s()-2*activefont->getGlyphImageMargin()) * wr;
            #ifdef TREES_CODE_FOR_MAKING_SPACES_EDITABLE
	        if (width == 0.0f)  width = glyph->getHorizontalAdvance() * wr;
            #endif

            if (_layout==RIGHT_TO_LEFT)
            {
                cursor.x() -= glyph->getHorizontalAdvance() * wr;
            }

            // adjust cursor position w.r.t any kerning.
            if (kerning && previous_charcode)
            {
                switch(_layout)
                {
                  case LEFT_TO_RIGHT:
                  {
                    osg::Vec2 delta(activefont->getKerning(previous_charcode,charcode,_kerningType));
                    cursor.x() += delta.x() * wr;
                    cursor.y() += delta.y() * hr;
                    break;
                  }
                  case RIGHT_TO_LEFT:
                  {
                    osg::Vec2 delta(activefont->getKerning(charcode,previous_charcode,_kerningType));
                    cursor.x() -= delta.x() * wr;
                    cursor.y() -= delta.y() * hr;
                    break;
                  }
                  case VERTICAL:
                    break; // no kerning when vertical.
                }
            }        

            osg::Vec2 bearing(horizontal?glyph->getHorizontalBearing():glyph->getVerticalBearing());
            cursor.x() += bearing.x() * wr;
            cursor.y() += bearing.y() * hr;

            // check to see if we are still within line if not move to next line.
            switch(_layout)
            {
              case LEFT_TO_RIGHT:
              {
                if (_maximumWidth>0.0f && cursor.x()+width>_maximumWidth) return itr;
                break;
              }
              case RIGHT_TO_LEFT:
              {
                if (_maximumWidth>0.0f && cursor.x()<-_maximumWidth) return itr;
                break;
              }
              case VERTICAL:
                if (_maximumHeight>0.0f && cursor.y()<-_maximumHeight) return itr;
                break;
            }
            

            // move the cursor onto the next character.
            switch(_layout)
            {
              case LEFT_TO_RIGHT: cursor.x() += glyph->getHorizontalAdvance() * wr; break;
              case VERTICAL:      cursor.y() -= glyph->getVerticalAdvance() *hr; break;
              case RIGHT_TO_LEFT: break; // nop.
            }
        }

    }
    return last;
}


#if 1
void Text::computeGlyphRepresentation()
{
    Font* activefont = getActiveFont();
    if (!activefont) return;
    
    _textureGlyphQuadMap.clear();
    
    if (_text.empty()) 
	{
	    _textBB.set(0,0,0,0,0,0);//no size text
	    computePositions(); //to reset the origin
		return;
	}
    
    osg::Vec2 startOfLine(0.0f,0.0f);
    osg::Vec2 cursor(startOfLine);
    osg::Vec2 local(0.0f,0.0f);
    
    unsigned int previous_charcode = 0;
    bool horizontal = _layout!=VERTICAL;
    bool kerning = true;

    activefont->setSize(_fontWidth,_fontHeight);
    
    float hr = _characterHeight/(float)activefont->getHeight();
    float wr = hr/_characterAspectRatio;

    std::set<unsigned int> deliminatorSet;
    deliminatorSet.insert(' ');
    deliminatorSet.insert('\n');
    deliminatorSet.insert(':');
    deliminatorSet.insert('/');
    deliminatorSet.insert(',');
    deliminatorSet.insert(';');
    deliminatorSet.insert(':');
    deliminatorSet.insert('.');

    for(String::iterator itr=_text.begin();
        itr!=_text.end();
        )
    {

        // find the end of the current line.
        String::iterator endOfLine = computeLastCharacterOnLine(cursor, itr,_text.end());


        if (itr!=endOfLine)
        {
            if (endOfLine!=_text.end())
            {
                if (deliminatorSet.count(*endOfLine)==0) 
                {
                    String::iterator lastValidChar = endOfLine;
                    while (lastValidChar!=itr && deliminatorSet.count(*lastValidChar)==0)
                    {
                        --lastValidChar;
                    }
                    if (itr!=lastValidChar)
                    {
                        ++lastValidChar;
                        endOfLine = lastValidChar;
                    }
                }
            }

            for(;itr!=endOfLine;++itr)
            {

                unsigned int charcode = *itr;

                Font::Glyph* glyph = activefont->getGlyph(charcode);
                if (glyph)
                {

                    float width = (float)(glyph->s()-2*activefont->getGlyphImageMargin()) * wr;
                    float height = (float)(glyph->t()-2*activefont->getGlyphImageMargin()) * hr;
                    #ifdef TREES_CODE_FOR_MAKING_SPACES_EDITABLE
	                if (width == 0.0f)  width = glyph->getHorizontalAdvance() * wr;
	                if (height == 0.0f) height = glyph->getVerticalAdvance() * hr;
                    #endif
                    if (_layout==RIGHT_TO_LEFT)
                    {
                        cursor.x() -= glyph->getHorizontalAdvance() * wr;
                    }

                    // adjust cursor position w.r.t any kerning.
                    if (kerning && previous_charcode)
                    {
                        switch(_layout)
                        {
                          case LEFT_TO_RIGHT:
                          {
                            osg::Vec2 delta(activefont->getKerning(previous_charcode,charcode,_kerningType));
                            cursor.x() += delta.x() * wr;
                            cursor.y() += delta.y() * hr;
                            break;
                          }
                          case RIGHT_TO_LEFT:
                          {
                            osg::Vec2 delta(activefont->getKerning(charcode,previous_charcode,_kerningType));
                            cursor.x() -= delta.x() * wr;
                            cursor.y() -= delta.y() * hr;
                            break;
                          }
                          case VERTICAL:
                            break; // no kerning when vertical.
                        }
                    }

                    local = cursor;


                    osg::Vec2 bearing(horizontal?glyph->getHorizontalBearing():glyph->getVerticalBearing());
                    local.x() += bearing.x() * wr;
                    local.y() += bearing.y() * hr;


                    GlyphQuads& glyphquad = _textureGlyphQuadMap[glyph->getTexture()->getStateSet()];

                    glyphquad._glyphs.push_back(glyph);

                    // set up the coords of the quad
                    glyphquad._coords.push_back(local+osg::Vec2(0.0f,height));
                    glyphquad._coords.push_back(local+osg::Vec2(0.0f,0.0f));
                    glyphquad._coords.push_back(local+osg::Vec2(width,0.0f));
                    glyphquad._coords.push_back(local+osg::Vec2(width,height));

                    // set up the tex coords of the quad
                    const osg::Vec2& mintc = glyph->getMinTexCoord();
                    const osg::Vec2& maxtc = glyph->getMaxTexCoord();

                    glyphquad._texcoords.push_back(osg::Vec2(mintc.x(),maxtc.y()));
                    glyphquad._texcoords.push_back(osg::Vec2(mintc.x(),mintc.y()));
                    glyphquad._texcoords.push_back(osg::Vec2(maxtc.x(),mintc.y()));
                    glyphquad._texcoords.push_back(osg::Vec2(maxtc.x(),maxtc.y()));

                    // move the cursor onto the next character.
                    switch(_layout)
                    {
                      case LEFT_TO_RIGHT: cursor.x() += glyph->getHorizontalAdvance() * wr; break;
                      case VERTICAL:      cursor.y() -= glyph->getVerticalAdvance() *hr; break;
                      case RIGHT_TO_LEFT: break; // nop.
                    }

                    previous_charcode = charcode;

                }
            }
        }
        else
        {
            ++itr;
        }
                                
        if (itr!=_text.end())
        {
            // skip over return.
            if (*itr=='\n') ++itr;
        }
                
        // move to new line.
        switch(_layout)
        {
          case LEFT_TO_RIGHT:
          {
            startOfLine.y() -= _characterHeight;
            cursor = startOfLine;
            previous_charcode = 0;
            break;
          }
          case RIGHT_TO_LEFT:
          {
            startOfLine.y() -= _characterHeight;
            cursor = startOfLine;
            previous_charcode = 0;
            break;
          }
          case VERTICAL:
          {
            startOfLine.x() += _characterHeight/_characterAspectRatio;
            cursor = startOfLine;
            previous_charcode = 0;
          }
          break;
        }

    }

    _textBB.init();

    for(TextureGlyphQuadMap::const_iterator titr=_textureGlyphQuadMap.begin();
        titr!=_textureGlyphQuadMap.end();
        ++titr)
    {
        const GlyphQuads& glyphquad = titr->second;
        
        for(GlyphQuads::Coords2::const_iterator citr = glyphquad._coords.begin();
            citr != glyphquad._coords.end();
            ++citr)
        {
            _textBB.expandBy(osg::Vec3(citr->x(),citr->y(),0.0f));
        }
    }

    if (!_textureGlyphQuadMap.empty()) 
    {
        setStateSet(const_cast<osg::StateSet*>((*_textureGlyphQuadMap.begin()).first.get()));
    }

    computePositions();
}

#else

        void Text::computeGlyphRepresentation()
        {
            Font* activefont = getActiveFont();
            if (!activefont) return;

            _textureGlyphQuadMap.clear();

            if (_text.empty()) 
	        {
	            _textBB.set(0,0,0,0,0,0);//no size text
	            computePositions(); //to reset the origin
		        return;
	        }

            osg::Vec2 startOfLine(0.0f,0.0f);
            osg::Vec2 cursor(startOfLine);
            osg::Vec2 local(0.0f,0.0f);

            unsigned int previous_charcode = 0;
            bool horizontal = _layout!=VERTICAL;
            bool kerning = true;

            activefont->setSize(_fontWidth,_fontHeight);

            float hr = _characterHeight/(float)activefont->getHeight();
            float wr = hr/_characterAspectRatio;

            for(String::iterator itr=_text.begin();
                itr!=_text.end();
                ++itr)
            {
                unsigned int charcode = *itr;

                if (charcode=='\n')
                {
                    if (horizontal) startOfLine.y() -= _characterHeight;
                    else startOfLine.x() += _characterHeight;
                    cursor = startOfLine;
                    previous_charcode = 0;
                    continue;
                }


                Font::Glyph* glyph = activefont->getGlyph(charcode);
                if (glyph)
                {

                    float width = (float)(glyph->s()-2*activefont->getGlyphImageMargin()) * wr;
                    float height = (float)(glyph->t()-2*activefont->getGlyphImageMargin()) * hr;
                    #ifdef TREES_CODE_FOR_MAKING_SPACES_EDITABLE
	                if (width == 0.0f)  width = glyph->getHorizontalAdvance() * wr;
	                if (height == 0.0f) height = glyph->getVerticalAdvance() * hr;
                    #endif
                    if (_layout==RIGHT_TO_LEFT)
                    {
                        cursor.x() -= glyph->getHorizontalAdvance() * wr;
                    }

                    // adjust cursor position w.r.t any kerning.
                    if (kerning && previous_charcode)
                    {
                        switch(_layout)
                        {
                          case LEFT_TO_RIGHT:
                          {
                            osg::Vec2 delta(activefont->getKerning(previous_charcode,charcode,_kerningType));
                            cursor.x() += delta.x() * wr;
                            cursor.y() += delta.y() * hr;
                            break;
                          }
                          case RIGHT_TO_LEFT:
                          {
                            osg::Vec2 delta(activefont->getKerning(charcode,previous_charcode,_kerningType));
                            cursor.x() -= delta.x() * wr;
                            cursor.y() -= delta.y() * hr;
                            break;
                          }
                          case VERTICAL:
                            break; // no kerning when vertical.
                        }
                    }

                    local = cursor;


                    osg::Vec2 bearing(horizontal?glyph->getHorizontalBearing():glyph->getVerticalBearing());
                    local.x() += bearing.x() * wr;
                    local.y() += bearing.y() * hr;

                    // check to see if we are still within line if not move to next line.
                    switch(_layout)
                    {
                      case LEFT_TO_RIGHT:
                      {
                        if (_maximumWidth>0.0f)
                        {
                            if (local.x()+width>_maximumWidth)
                            {
                                startOfLine.y() -= _characterHeight;
                                cursor = startOfLine;
                                previous_charcode = 0;

                                local = cursor;
                                local.x() += bearing.x() * wr;
                                local.y() += bearing.y() * hr;
                            }
                        }
                        break;
                      }
                      case RIGHT_TO_LEFT:
                      {
                        if (_maximumWidth>0.0f)
                        {
                            if (local.x()<-_maximumWidth)
                            {
                                startOfLine.y() -= _characterHeight;
                                cursor = startOfLine;
                                previous_charcode = 0;

                                local = cursor;
                                local.x() += bearing.x() * wr;
                                local.y() += bearing.y() * hr;
                            }
        ;                }
                        break;
                      }
                      case VERTICAL:
                        if (_maximumHeight>0.0f)
                        {
                            if (local.y()<-_maximumHeight)
                            {
                                startOfLine.x() += _characterHeight/_characterAspectRatio;
                                cursor = startOfLine;
                                previous_charcode = 0;

                                local = cursor;
                                local.x() += bearing.x() * wr;
                                local.y() += bearing.y() * hr;
                            }
                        }
                        break;
                    }

                    GlyphQuads& glyphquad = _textureGlyphQuadMap[glyph->getTexture()->getStateSet()];

                    glyphquad._glyphs.push_back(glyph);

                    // set up the coords of the quad
                    glyphquad._coords.push_back(local+osg::Vec2(0.0f,height));
                    glyphquad._coords.push_back(local+osg::Vec2(0.0f,0.0f));
                    glyphquad._coords.push_back(local+osg::Vec2(width,0.0f));
                    glyphquad._coords.push_back(local+osg::Vec2(width,height));

                    // set up the tex coords of the quad
                    const osg::Vec2& mintc = glyph->getMinTexCoord();
                    const osg::Vec2& maxtc = glyph->getMaxTexCoord();

                    glyphquad._texcoords.push_back(osg::Vec2(mintc.x(),maxtc.y()));
                    glyphquad._texcoords.push_back(osg::Vec2(mintc.x(),mintc.y()));
                    glyphquad._texcoords.push_back(osg::Vec2(maxtc.x(),mintc.y()));
                    glyphquad._texcoords.push_back(osg::Vec2(maxtc.x(),maxtc.y()));

                    // move the cursor onto the next character.
                    switch(_layout)
                    {
                      case LEFT_TO_RIGHT: cursor.x() += glyph->getHorizontalAdvance() * wr; break;
                      case VERTICAL:      cursor.y() -= glyph->getVerticalAdvance() *hr; break;
                      case RIGHT_TO_LEFT: break; // nop.
                    }            
                }

                previous_charcode = charcode;
            }

            _textBB.init();

            for(TextureGlyphQuadMap::const_iterator titr=_textureGlyphQuadMap.begin();
                titr!=_textureGlyphQuadMap.end();
                ++titr)
            {
                const GlyphQuads& glyphquad = titr->second;

                for(GlyphQuads::Coords2::const_iterator citr = glyphquad._coords.begin();
                    citr != glyphquad._coords.end();
                    ++citr)
                {
                    _textBB.expandBy(osg::Vec3(citr->x(),citr->y(),0.0f));
                }
            }

            if (!_textureGlyphQuadMap.empty()) 
            {
                setStateSet(const_cast<osg::StateSet*>((*_textureGlyphQuadMap.begin()).first.get()));
            }

            computePositions();
        }

#endif
void Text::computePositions()
{
    for(unsigned int i=0;i<_autoTransformCache.size();++i)
    {
        computePositions(i);
    }
}

void Text::computePositions(unsigned int contextID) const
{

    switch(_alignment)
    {
    case LEFT_TOP:      _offset.set(_textBB.xMin(),_textBB.yMax(),_textBB.zMin()); break;
    case LEFT_CENTER:   _offset.set(_textBB.xMin(),(_textBB.yMax()+_textBB.yMin())*0.5f,_textBB.zMin()); break;
    case LEFT_BOTTOM:   _offset.set(_textBB.xMin(),_textBB.yMin(),_textBB.zMin()); break;

    case CENTER_TOP:    _offset.set((_textBB.xMax()+_textBB.xMin())*0.5f,_textBB.yMax(),_textBB.zMin()); break;
    case CENTER_CENTER: _offset.set((_textBB.xMax()+_textBB.xMin())*0.5f,(_textBB.yMax()+_textBB.yMin())*0.5f,_textBB.zMin()); break;
    case CENTER_BOTTOM: _offset.set((_textBB.xMax()+_textBB.xMin())*0.5f,_textBB.yMin(),_textBB.zMin()); break;

    case RIGHT_TOP:     _offset.set(_textBB.xMax(),_textBB.yMax(),_textBB.zMin()); break;
    case RIGHT_CENTER:  _offset.set(_textBB.xMax(),(_textBB.yMax()+_textBB.yMin())*0.5f,_textBB.zMin()); break;
    case RIGHT_BOTTOM:  _offset.set(_textBB.xMax(),_textBB.yMin(),_textBB.zMin()); break;
    case LEFT_BASE_LINE:  _offset.set(0.0f,0.0f,0.0f); break;
    case CENTER_BASE_LINE:  _offset.set((_textBB.xMax()+_textBB.xMin())*0.5f,0.0f,0.0f); break;
    case RIGHT_BASE_LINE:  _offset.set((_textBB.xMax()+_textBB.xMin()),0.0f,0.0f); break;
    }
    
    
    
    AutoTransformCache& atc = _autoTransformCache[contextID];
    osg::Matrix& matrix = atc._matrix;

    if (_characterSizeMode!=OBJECT_COORDS || _autoRotateToScreen)
    {

        matrix.makeTranslate(-_offset);

        osg::Matrix rotate_matrix; 
        if (_autoRotateToScreen) 
        {
            osg::Vec3 trans(atc._modelview.getTrans());
            atc._modelview.setTrans(0.0f,0.0f,0.0f);

            rotate_matrix.invert(atc._modelview);

            atc._modelview.setTrans(trans);
        }

        if (_characterSizeMode!=OBJECT_COORDS)
        {

            osg::Matrix M(rotate_matrix*osg::Matrix::translate(_position)*atc._modelview);
            osg::Matrix& P = atc._projection;
            
            // compute the pixel size vector.
                        
            // pre adjust P00,P20,P23,P33 by multiplying them by the viewport window matrix.
            // here we do it in short hand with the knowledge of how the window matrix is formed
            // note P23,P33 are multiplied by an implicit 1 which would come from the window matrix.
            // Robert Osfield, June 2002.

            // scaling for horizontal pixels
            float P00 = P(0,0)*atc._width*0.5f;
            float P20_00 = P(2,0)*atc._width*0.5f + P(2,3)*atc._width*0.5f;
            osg::Vec3 scale_00(M(0,0)*P00 + M(0,2)*P20_00,
                               M(1,0)*P00 + M(1,2)*P20_00,
                               M(2,0)*P00 + M(2,2)*P20_00);

            // scaling for vertical pixels
            float P10 = P(1,1)*atc._height*0.5f;
            float P20_10 = P(2,1)*atc._height*0.5f + P(2,3)*atc._height*0.5f;
            osg::Vec3 scale_10(M(0,1)*P10 + M(0,2)*P20_10,
                               M(1,1)*P10 + M(1,2)*P20_10,
                               M(2,1)*P10 + M(2,2)*P20_10);

            float P23 = P(2,3);
            float P33 = P(3,3);

            float pixelSizeVector_w = M(3,2)*P23 + M(3,3)*P33;

            float pixelSize = (_characterHeight*sqrtf(scale_00.length2()+scale_10.length2()))/(pixelSizeVector_w*0.701f);

            if (_characterSizeMode==SCREEN_COORDS)
            {
                float scale_font = _characterHeight/pixelSize;
                matrix.postMult(osg::Matrix::scale(scale_font, scale_font,1.0f));
            }
            else if (pixelSize>_fontHeight)
            {
                float scale_font = _fontHeight/pixelSize;
                matrix.postMult(osg::Matrix::scale(scale_font, scale_font,1.0f));
            }

        }
        
       
        if (_autoRotateToScreen) 
        {
            matrix.postMult(rotate_matrix);

        }

        if (!_rotation.zeroRotation() )
        {
            matrix.postMult(osg::Matrix::rotate(_rotation));
        }

        matrix.postMult(osg::Matrix::translate(_position));
    }
    else if (!_rotation.zeroRotation())
    {
        matrix.makeTranslate(-_offset);
        matrix.postMult(osg::Matrix::rotate(_rotation));
        matrix.postMult(osg::Matrix::translate(_position));
    }
    else
    {
        matrix.makeTranslate(_position-_offset);
    }

    

    // now apply matrix to the glyphs.
    for(TextureGlyphQuadMap::iterator titr=_textureGlyphQuadMap.begin();
        titr!=_textureGlyphQuadMap.end();
        ++titr)
    {
        GlyphQuads& glyphquad = titr->second;
        GlyphQuads::Coords2& coords2 = glyphquad._coords;
        GlyphQuads::Coords3& transformedCoords = glyphquad._transformedCoords[contextID];
        
        unsigned int numCoords = coords2.size();
        if (numCoords!=transformedCoords.size())
        {
            transformedCoords.resize(numCoords);
        }
        
        for(unsigned int i=0;i<numCoords;++i)
        {
            transformedCoords[i] = osg::Vec3(coords2[i].x(),coords2[i].y(),0.0f)*matrix;
        }
    }

    _normal = osg::Matrix::transform3x3(osg::Vec3(0.0f,0.0f,1.0f),matrix);
    _normal.normalize();

    const_cast<Text*>(this)->dirtyBound();    
}

void Text::drawImplementation(osg::State& state) const
{
    unsigned int contextID = state.getContextID();

    if (_characterSizeMode!=OBJECT_COORDS || _autoRotateToScreen)
    {
        int frameNumber = state.getFrameStamp()?state.getFrameStamp()->getFrameNumber():0;
        AutoTransformCache& atc = _autoTransformCache[contextID];
        const osg::Matrix& modelview = state.getModelViewMatrix();

        osg::Vec3 newTransformedPosition = _position*modelview;
        
        int width = atc._width;
        int height = atc._height;

        const osg::Viewport* viewport = state.getCurrentViewport();
        if (viewport)
        {
            width = viewport->width();
            height = viewport->height();
        }

        bool doUpdate = atc._traversalNumber==-1;
        if (atc._traversalNumber>=0)
        {
            if (atc._modelview!=modelview)
            {
                doUpdate = true;
            }
            else if (width!=atc._width || height!=atc._height)
            {
                doUpdate = true;
            } 
        }
        
        atc._traversalNumber = frameNumber;
        atc._width = width;
        atc._height = height;
        
        if (doUpdate)
        {    
            atc._transformedPosition = newTransformedPosition;
            atc._projection = state.getProjectionMatrix();
            atc._modelview = modelview;

            computePositions(contextID);
        }
        
    }


    glNormal3fv(_normal.ptr());
    glColor4fv(_color.ptr());

    if (_drawMode & TEXT)
    {

        state.disableAllVertexArrays();

        for(TextureGlyphQuadMap::const_iterator titr=_textureGlyphQuadMap.begin();
            titr!=_textureGlyphQuadMap.end();
            ++titr)
        {
            // need to set the texture here...
            state.apply(titr->first.get());

            const GlyphQuads& glyphquad = titr->second;

            state.setVertexPointer( 3, GL_FLOAT, 0, &(glyphquad._transformedCoords[contextID].front()));
            state.setTexCoordPointer( 0, 2, GL_FLOAT, 0, &(glyphquad._texcoords.front()));

            glDrawArrays(GL_QUADS,0,glyphquad._coords.size());

        }
    }

    if (_drawMode & BOUNDINGBOX)
    {
    
        if (_textBB.valid())
        {
            state.applyTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::OFF);
            
            const osg::Matrix& matrix = _autoTransformCache[contextID]._matrix;

            osg::Vec3 c00(osg::Vec3(_textBB.xMin(),_textBB.yMin(),_textBB.zMin())*matrix);
            osg::Vec3 c10(osg::Vec3(_textBB.xMax(),_textBB.yMin(),_textBB.zMin())*matrix);
            osg::Vec3 c11(osg::Vec3(_textBB.xMax(),_textBB.yMax(),_textBB.zMin())*matrix);
            osg::Vec3 c01(osg::Vec3(_textBB.xMin(),_textBB.yMax(),_textBB.zMin())*matrix);

        
            glColor4f(1.0f,1.0f,0.0f,1.0f);
            glBegin(GL_LINE_LOOP);
                glVertex3fv(c00.ptr());
                glVertex3fv(c10.ptr());
                glVertex3fv(c11.ptr());
                glVertex3fv(c01.ptr());
            glEnd();
        }
    }    

    if (_drawMode & ALIGNMENT)
    {
        glColor4f(1.0f,0.0f,1.0f,1.0f);

        float cursorsize = _characterHeight*0.5f;

        const osg::Matrix& matrix = _autoTransformCache[contextID]._matrix;

        osg::Vec3 hl(osg::Vec3(_offset.x()-cursorsize,_offset.y(),_offset.z())*matrix);
        osg::Vec3 hr(osg::Vec3(_offset.x()+cursorsize,_offset.y(),_offset.z())*matrix);
        osg::Vec3 vt(osg::Vec3(_offset.x(),_offset.y()-cursorsize,_offset.z())*matrix);
        osg::Vec3 vb(osg::Vec3(_offset.x(),_offset.y()+cursorsize,_offset.z())*matrix);

        state.applyTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::OFF);
        
        glBegin(GL_LINES);
            glVertex3fv(hl.ptr());
            glVertex3fv(hr.ptr());
            glVertex3fv(vt.ptr());
            glVertex3fv(vb.ptr());
        glEnd();
        
    }    


//    glPopMatrix();
}

void Text::accept(osg::Drawable::ConstAttributeFunctor& af) const
{
    for(TextureGlyphQuadMap::const_iterator titr=_textureGlyphQuadMap.begin();
        titr!=_textureGlyphQuadMap.end();
        ++titr)
    {
        const GlyphQuads& glyphquad = titr->second;
        af.apply(osg::Drawable::VERTICES,glyphquad._transformedCoords[0].size(),&(glyphquad._transformedCoords[0].front()));
        af.apply(osg::Drawable::TEXTURE_COORDS_0,glyphquad._texcoords.size(),&(glyphquad._texcoords.front()));
    }
}

void Text::accept(osg::Drawable::PrimitiveFunctor& pf) const
{
    for(TextureGlyphQuadMap::const_iterator titr=_textureGlyphQuadMap.begin();
        titr!=_textureGlyphQuadMap.end();
        ++titr)
    {
        const GlyphQuads& glyphquad = titr->second;

        pf.setVertexArray(glyphquad._transformedCoords[0].size(),&(glyphquad._transformedCoords[0].front()));
        pf.drawArrays(GL_QUADS,0,glyphquad._transformedCoords[0].size());
            
    }
    
}
