/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield 
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


#include <vector>
#include <osgText/Text>

#include <osg/Math>
#include <osg/GL>
#include <osg/Notify>
#include <osgUtil/CullVisitor>
#include <osgDB/ReadFile>

#include "DefaultFont.h"

using namespace osg;
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
    _kerningType(KERNING_DEFAULT),
    _lineCount(0)
{
    setUseDisplayList(false);
    setSupportsDisplayList(false);
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
    _kerningType(text._kerningType),
    _lineCount(text._lineCount)
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
    case REVERSED_XZ_PLANE:
        setRotation(osg::Quat(osg::inDegrees(180.0f),osg::Vec3(0.0f,1.0f,0.0f))*
                    osg::Quat(osg::inDegrees(90.0f),osg::Vec3(1.0f,0.0f,0.0f))); 
        break;
    case YZ_PLANE:  
        setRotation(osg::Quat(osg::inDegrees(90.0f),osg::Vec3(1.0f,0.0f,0.0f))*
                    osg::Quat(osg::inDegrees(90.0f),osg::Vec3(0.0f,0.0f,1.0f)));
        break;
    case REVERSED_YZ_PLANE:  
        setRotation(osg::Quat(osg::inDegrees(180.0f),osg::Vec3(0.0f,1.0f,0.0f))*
                    osg::Quat(osg::inDegrees(90.0f),osg::Vec3(1.0f,0.0f,0.0f))*
                    osg::Quat(osg::inDegrees(90.0f),osg::Vec3(0.0f,0.0f,1.0f)));
        break;
    case XY_PLANE:
        setRotation(osg::Quat());  // nop - already on XY plane.
        break;
    case REVERSED_XY_PLANE:
        setRotation(osg::Quat(osg::inDegrees(180.0f),osg::Vec3(0.0f,1.0f,0.0f)));
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


osg::BoundingBox Text::computeBound() const
{
    osg::BoundingBox  bbox;

    if (_textBB.valid())
    {
    
        for(unsigned int i=0;i<_autoTransformCache.size();++i)
        {
            if (_autoTransformCache[i]._traversalNumber<0 && (_characterSizeMode!=OBJECT_COORDS || _autoRotateToScreen))
            {
                // _autoTransformCache is not valid so don't take it into accoumt when compute bounding volume.
            }
            else
            {            
                osg::Matrix& matrix = _autoTransformCache[i]._matrix;
                bbox.expandBy(osg::Vec3(_textBB.xMin(),_textBB.yMin(),_textBB.zMin())*matrix);
                bbox.expandBy(osg::Vec3(_textBB.xMax(),_textBB.yMin(),_textBB.zMin())*matrix);
                bbox.expandBy(osg::Vec3(_textBB.xMax(),_textBB.yMax(),_textBB.zMin())*matrix);
                bbox.expandBy(osg::Vec3(_textBB.xMin(),_textBB.yMax(),_textBB.zMin())*matrix);
            }
        }
    }
    
    return bbox;
}

Font* Text::getActiveFont()
{
    return _font.valid() ? _font.get() : DefaultFont::instance();
}

const Font* Text::getActiveFont() const
{
    return _font.valid() ? _font.get() : DefaultFont::instance();
}

String::iterator Text::computeLastCharacterOnLine(osg::Vec2& cursor, String::iterator first,String::iterator last)
{
    Font* activefont = getActiveFont();
    if (!activefont) return last;

    float hr = _characterHeight/(float)activefont->getFontHeight();
    float wr = hr/_characterAspectRatio;

    bool kerning = true;
    unsigned int previous_charcode = 0;

    String::iterator lastChar = first;

    std::set<unsigned int> deliminatorSet;
    deliminatorSet.insert(' ');
    deliminatorSet.insert('\n');
    deliminatorSet.insert(':');
    deliminatorSet.insert('/');
    deliminatorSet.insert(',');
    deliminatorSet.insert(';');
    deliminatorSet.insert(':');
    deliminatorSet.insert('.');

    for(bool outOfSpace=false;lastChar!=last;++lastChar)
    {
        unsigned int charcode = *lastChar;
        
        if (charcode=='\n')
        {
            return lastChar;
        }

        Font::Glyph* glyph = activefont->getGlyph(charcode);
        if (glyph)
        {

           float width = (float)(glyph->s()-2*activefont->getGlyphImageMargin()) * wr;
           //float height = (float)(glyph->t()-2*activefont->getGlyphImageMargin()) * hr;
           #ifdef TREES_CODE_FOR_MAKING_SPACES_EDITABLE
           if (width == 0.0f)  width = glyph->getHorizontalAdvance() * wr;
           //if (height == 0.0f) height = glyph->getVerticalAdvance() * hr;
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
                }            // check to see if we are still within line if not move to next line.
            }

            switch(_layout)
            {
              case LEFT_TO_RIGHT:
              {
                if (_maximumWidth>0.0f && cursor.x()+width>_maximumWidth) outOfSpace=true;
                break;
              }
              case RIGHT_TO_LEFT:
              {
                if (_maximumWidth>0.0f && cursor.x()<-_maximumWidth) outOfSpace=true;
                break;
              }
              case VERTICAL:
                if (_maximumHeight>0.0f && cursor.y()<-_maximumHeight) outOfSpace=true;
                break;
            }
            
            // => word boundary detection & wrapping
            if (outOfSpace) break;

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
    
    // word boundary detection & wrapping
    if (lastChar!=last)
    {
        if (deliminatorSet.count(*lastChar)==0) 
        {
            String::iterator lastValidChar = lastChar;
            while (lastValidChar!=first && deliminatorSet.count(*lastValidChar)==0)
            {
                --lastValidChar;
            }
            if (first!=lastValidChar)
            {
                ++lastValidChar;
                lastChar = lastValidChar;
            }
        }
    }

    return lastChar;
}

void Text::computeGlyphRepresentation()
{
    Font* activefont = getActiveFont();
    if (!activefont) return;
    
    _textureGlyphQuadMap.clear();
    _lineCount = 0;
    
    if (_text.empty()) 
    {
        _textBB.set(0,0,0,0,0,0);//no size text
        computePositions(); //to reset the origin
        return;
    }

    // initialize bounding box, it will be expanded during glyph position calculation
    _textBB.init();

    osg::Vec2 startOfLine_coords(0.0f,0.0f);
    osg::Vec2 cursor(startOfLine_coords);
    osg::Vec2 local(0.0f,0.0f);
    
    unsigned int previous_charcode = 0;
    unsigned int linelength = 0;
    bool horizontal = _layout!=VERTICAL;
    bool kerning = true;
    
    unsigned int lineNumber = 0;

    activefont->setFontResolution(_fontWidth,_fontHeight);
    
    float hr = _characterHeight/(float)activefont->getFontHeight();
    float wr = hr/_characterAspectRatio;

    for(String::iterator itr=_text.begin();
        itr!=_text.end();
        )
    {
        // record the start of the current line
            String::iterator startOfLine_itr = itr;

            // find the end of the current line.
            osg::Vec2 endOfLine_coords(cursor);
            String::iterator endOfLine_itr = computeLastCharacterOnLine(endOfLine_coords, itr,_text.end());

            linelength = endOfLine_itr - startOfLine_itr;

            // Set line position to correct alignment.
            switch(_layout)
            {
            case LEFT_TO_RIGHT:
            {
            switch(_alignment)
            {
              // nothing to be done for these
              //case LEFT_TOP:
              //case LEFT_CENTER:
              //case LEFT_BOTTOM:
              //case LEFT_BASE_LINE:
              //case LEFT_BOTTOM_BASE_LINE:
              //  break;
              case CENTER_TOP:
              case CENTER_CENTER:
              case CENTER_BOTTOM:
              case CENTER_BASE_LINE:
              case CENTER_BOTTOM_BASE_LINE:
                cursor.x() = (cursor.x() - endOfLine_coords.x()) * 0.5f;
                break;
              case RIGHT_TOP:
              case RIGHT_CENTER:
              case RIGHT_BOTTOM:
              case RIGHT_BASE_LINE:
              case RIGHT_BOTTOM_BASE_LINE:
                cursor.x() = cursor.x() - endOfLine_coords.x();
                break;
              default:
                break;
              }
            break;
            }
            case RIGHT_TO_LEFT:
            {
            switch(_alignment)
            {
              case LEFT_TOP:
              case LEFT_CENTER:
              case LEFT_BOTTOM:
              case LEFT_BASE_LINE:
              case LEFT_BOTTOM_BASE_LINE:
                cursor.x() = 2*cursor.x() - endOfLine_coords.x();
                break;
              case CENTER_TOP:
              case CENTER_CENTER:
              case CENTER_BOTTOM:
              case CENTER_BASE_LINE:
              case CENTER_BOTTOM_BASE_LINE:
                cursor.x() = cursor.x() + (cursor.x() - endOfLine_coords.x()) * 0.5f;
                break;
              // nothing to be done for these
              //case RIGHT_TOP:
              //case RIGHT_CENTER:
              //case RIGHT_BOTTOM:
              //case RIGHT_BASE_LINE:
              //case RIGHT_BOTTOM_BASE_LINE:
              //  break;
              default:
                break;
            }
            break;
            }
            case VERTICAL:
            {
            switch(_alignment)
            {
              // TODO: current behaviour top baselines lined up in both cases - need to implement
              //       top of characters aligment - Question is this neccesary?
              // ... otherwise, nothing to be done for these 6 cases
              //case LEFT_TOP:
              //case CENTER_TOP:
              //case RIGHT_TOP:
              //  break;
              //case LEFT_BASE_LINE:
              //case CENTER_BASE_LINE:
              //case RIGHT_BASE_LINE:
              //  break;
              case LEFT_CENTER:
              case CENTER_CENTER:
              case RIGHT_CENTER:
                cursor.y() = cursor.y() + (cursor.y() - endOfLine_coords.y()) * 0.5f;
                break;
              case LEFT_BOTTOM_BASE_LINE:
              case CENTER_BOTTOM_BASE_LINE:
              case RIGHT_BOTTOM_BASE_LINE:
                cursor.y() = cursor.y() - (linelength * _characterHeight);
                break;
              case LEFT_BOTTOM:
              case CENTER_BOTTOM:
              case RIGHT_BOTTOM:
                cursor.y() = 2*cursor.y() - endOfLine_coords.y();
                break;
              default:
                break;
            }
            break;
          }
        }

        if (itr!=endOfLine_itr)
        {

            for(;itr!=endOfLine_itr;++itr)
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
                    glyphquad._lineNumbers.push_back(lineNumber);

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
                    // also expand bounding box
                    switch(_layout)
                    {
                      case LEFT_TO_RIGHT:
                          cursor.x() += glyph->getHorizontalAdvance() * wr;
                          _textBB.expandBy(osg::Vec3(local.x(),local.y(),0.0f)); //lower left corner
                          _textBB.expandBy(osg::Vec3(cursor.x(),local.y()+height,0.0f)); //upper right corner
                          break;
                      case VERTICAL:
                          cursor.y() -= glyph->getVerticalAdvance() *hr;
                          _textBB.expandBy(osg::Vec3(local.x(),local.y()+height,0.0f)); //upper left corner
                          _textBB.expandBy(osg::Vec3(local.x()+width,cursor.y(),0.0f)); //lower right corner
                          break;
                      case RIGHT_TO_LEFT:
                          _textBB.expandBy(osg::Vec3(local.x()+width,local.y(),0.0f)); //lower right corner
                          _textBB.expandBy(osg::Vec3(cursor.x(),local.y()+height,0.0f)); //upper left corner
                          break;
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
            startOfLine_coords.y() -= _characterHeight;
            cursor = startOfLine_coords;
            previous_charcode = 0;
            _lineCount++;
            break;
          }
          case RIGHT_TO_LEFT:
          {
            startOfLine_coords.y() -= _characterHeight;
            cursor = startOfLine_coords;
            previous_charcode = 0;
            _lineCount++;
            break;
          }
          case VERTICAL:
          {
            startOfLine_coords.x() += _characterHeight/_characterAspectRatio;
            cursor = startOfLine_coords;
            previous_charcode = 0;
            // because _lineCount is the max vertical no. of characters....
            _lineCount = (_lineCount >linelength)?_lineCount:linelength;
          }
          break;
        }
        
        ++lineNumber;

    }
   
    setStateSet(const_cast<osg::StateSet*>((*_textureGlyphQuadMap.begin()).first.get()));

    computePositions();
}

void Text::computePositions()
{
    unsigned int size = osg::maximum(osg::DisplaySettings::instance()->getMaxNumberOfGraphicsContexts(),_autoTransformCache.size());
    for(unsigned int i=0;i<size;++i)
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
    case RIGHT_BASE_LINE:  _offset.set(_textBB.xMax(),0.0f,0.0f); break;
    
    case LEFT_BOTTOM_BASE_LINE:  _offset.set(0.0f,-_characterHeight*(_lineCount-1),0.0f); break;
    case CENTER_BOTTOM_BASE_LINE:  _offset.set((_textBB.xMax()+_textBB.xMin())*0.5f,-_characterHeight*(_lineCount-1),0.0f); break;
    case RIGHT_BOTTOM_BASE_LINE:  _offset.set(_textBB.xMax(),-_characterHeight*(_lineCount-1),0.0f); break;
    }
    
    
    
    AutoTransformCache& atc = _autoTransformCache[contextID];
    osg::Matrix& matrix = atc._matrix;

    if (_characterSizeMode!=OBJECT_COORDS || _autoRotateToScreen)
    {

        matrix.makeTranslate(-_offset);

        osg::Matrix rotate_matrix; 
        if (_autoRotateToScreen) 
        {
            osg::Vec3d trans(atc._modelview.getTrans());
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

            float pixelSizeVert=(_characterHeight*sqrtf(scale_10.length2()))/(pixelSizeVector_w*0.701f);
            float pixelSizeHori=(_characterHeight/_characterAspectRatio*sqrtf(scale_00.length2()))/(pixelSizeVector_w*0.701f);

            // avoid nasty math by preventing a divide by zero
            if (pixelSizeVert == 0.0f)
               pixelSizeVert= 1.0f;
            if (pixelSizeHori == 0.0f)
               pixelSizeHori= 1.0f;

            if (_characterSizeMode==SCREEN_COORDS)
            {
                float scale_font_vert=_characterHeight/pixelSizeVert;
                float scale_font_hori=_characterHeight/_characterAspectRatio/pixelSizeHori;

                if (P10<0)
                   scale_font_vert=-scale_font_vert;
                matrix.postMult(osg::Matrix::scale(scale_font_hori, scale_font_vert,1.0f));
            }
            else if (pixelSizeVert>_fontHeight)
            {
                float scale_font = _fontHeight/pixelSizeVert;
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
        const osg::Matrix& projection = state.getProjectionMatrix();

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
            else if (atc._projection!=projection)
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
            atc._projection = projection;
            atc._modelview = modelview;

            computePositions(contextID);
        }
        
    }
    
    
    // Ensure that the glyph coordinates have been transformed for
    // this context id.

    if ( !_textureGlyphQuadMap.empty() )
    {
        const GlyphQuads& glyphquad = (_textureGlyphQuadMap.begin())->second;
        if ( glyphquad._transformedCoords[contextID].empty() )
        {
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

            const GlyphQuads::Coords3& transformedCoords = glyphquad._transformedCoords[contextID];
            if (!transformedCoords.empty()) 
            {
                state.setVertexPointer( 3, GL_FLOAT, 0, &(transformedCoords.front()));
                state.setTexCoordPointer( 0, 2, GL_FLOAT, 0, &(glyphquad._texcoords.front()));

                glDrawArrays(GL_QUADS,0,transformedCoords.size());
            }
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

void Text::accept(osg::PrimitiveFunctor& pf) const
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

/** If State is non-zero, this function releases OpenGL objects for
  * the specified graphics context. Otherwise, releases OpenGL objexts
  * for all graphics contexts. */
void Text::releaseGLObjects(osg::State* state) const
{
    Drawable::releaseGLObjects(state);
    if (_font.valid()) _font->releaseGLObjects(state);
}
