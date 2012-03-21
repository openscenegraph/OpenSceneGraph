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


#include <osgText/Text>

#include <osg/Math>
#include <osg/GL>
#include <osg/Notify>
#include <osg/PolygonOffset>
#include <osg/TexEnv>

#include <osgUtil/CullVisitor>

#include <osgDB/ReadFile>

using namespace osg;
using namespace osgText;

//#define TREES_CODE_FOR_MAKING_SPACES_EDITABLE

Text::Text():
    _enableDepthWrites(true),
    _backdropType(NONE),
    _backdropImplementation(DELAYED_DEPTH_WRITES),
    _backdropHorizontalOffset(0.07f),
    _backdropVerticalOffset(0.07f),
    _backdropColor(0.0f, 0.0f, 0.0f, 1.0f),
    _colorGradientMode(SOLID),
    _colorGradientTopLeft(1.0f, 0.0f, 0.0f, 1.0f),
    _colorGradientBottomLeft(0.0f, 1.0f, 0.0f, 1.0f),
    _colorGradientBottomRight(0.0f, 0.0f, 1.0f, 1.0f),
    _colorGradientTopRight(1.0f, 1.0f, 1.0f, 1.0f)
{}

Text::Text(const Text& text,const osg::CopyOp& copyop):
    osgText::TextBase(text,copyop),
    _enableDepthWrites(text._enableDepthWrites),
    _backdropType(text._backdropType),
    _backdropImplementation(text._backdropImplementation),
    _backdropHorizontalOffset(text._backdropHorizontalOffset),
    _backdropVerticalOffset(text._backdropVerticalOffset),
    _backdropColor(text._backdropColor),
    _colorGradientMode(text._colorGradientMode),
    _colorGradientTopLeft(text._colorGradientTopLeft),
    _colorGradientBottomLeft(text._colorGradientBottomLeft),
    _colorGradientBottomRight(text._colorGradientBottomRight),
    _colorGradientTopRight(text._colorGradientTopRight)
{
    computeGlyphRepresentation();
}

Text::~Text()
{
}

void Text::setFont(osg::ref_ptr<Font> font)
{
    if (_font==font) return;

    osg::StateSet* previousFontStateSet = _font.valid() ? _font->getStateSet() : Font::getDefaultFont()->getStateSet();
    osg::StateSet* newFontStateSet = font.valid() ? font->getStateSet() : Font::getDefaultFont()->getStateSet();

    if (getStateSet() == previousFontStateSet)
    {
        setStateSet( newFontStateSet );
    }

    TextBase::setFont(font);
}


Font* Text::getActiveFont()
{
    return _font.valid() ? _font.get() : Font::getDefaultFont().get();
}

const Font* Text::getActiveFont() const
{
    return _font.valid() ? _font.get() : Font::getDefaultFont().get();
}

String::iterator Text::computeLastCharacterOnLine(osg::Vec2& cursor, String::iterator first,String::iterator last)
{
    Font* activefont = getActiveFont();
    if (!activefont) return last;

    float hr = _characterHeight;
    float wr = hr/getCharacterAspectRatio();

    bool kerning = true;
    unsigned int previous_charcode = 0;

    String::iterator lastChar = first;

    for(bool outOfSpace=false;lastChar!=last;++lastChar)
    {
        unsigned int charcode = *lastChar;

        if (charcode=='\n')
        {
            return lastChar;
        }

        Glyph* glyph = activefont->getGlyph(_fontSize, charcode);
        if (glyph)
        {

           float width = (float)(glyph->getWidth()) * wr;

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
                if(_maximumHeight>0.0f && cursor.y()<-_maximumHeight) outOfSpace=true;
                break;
              }
              case RIGHT_TO_LEFT:
              {
                if (_maximumWidth>0.0f && cursor.x()<-_maximumWidth) outOfSpace=true;
                if(_maximumHeight>0.0f && cursor.y()<-_maximumHeight) outOfSpace=true;
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
        String::iterator lastValidChar = lastChar;
          String::iterator prevChar;
        while (lastValidChar != first){
            prevChar = lastValidChar - 1;

            // last char is after a hyphen
                if(*lastValidChar == '-')
                return lastValidChar + 1;

            // last char is start of whitespace
            if((*lastValidChar == ' ' || *lastValidChar == '\n') && (*prevChar != ' ' && *prevChar != '\n'))
                return lastValidChar;

            // Subtract off glyphs from the cursor position (to correctly center text)
                if(*prevChar != '-')
            {
                Glyph* glyph = activefont->getGlyph(_fontSize, *prevChar);
                if (glyph)
                {
                    switch(_layout)
                    {
                    case LEFT_TO_RIGHT: cursor.x() -= glyph->getHorizontalAdvance() * wr; break;
                    case VERTICAL:      cursor.y() += glyph->getVerticalAdvance() * hr; break;
                    case RIGHT_TO_LEFT: break; // nop.
                    }
                }
            }

            lastValidChar = prevChar;
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
        TextBase::computePositions(); //to reset the origin
        return;
    }

    //OpenThreads::ScopedLock<Font::FontMutex> lock(*(activefont->getSerializeFontCallsMutex()));

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

    float hr = _characterHeight;
    float wr = hr/getCharacterAspectRatio();

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
              //       top of characters alignment - Question is this necessary?
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

                Glyph* glyph = activefont->getGlyph(_fontSize, charcode);
                if (glyph)
                {
                    float width = (float)(glyph->getWidth()) * wr;
                    float height = (float)(glyph->getHeight()) * hr;

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

                    GlyphQuads& glyphquad = _textureGlyphQuadMap[glyph->getTexture()];

                    glyphquad._glyphs.push_back(glyph);
                    glyphquad._lineNumbers.push_back(lineNumber);

                    // Adjust coordinates and texture coordinates to avoid
                    // clipping the edges of antialiased characters.
                    osg::Vec2 mintc = glyph->getMinTexCoord();
                    osg::Vec2 maxtc = glyph->getMaxTexCoord();
                    osg::Vec2 vDiff = maxtc - mintc;

                    float fHorizTCMargin = 1.0f / glyph->getTexture()->getTextureWidth();
                    float fVertTCMargin = 1.0f / glyph->getTexture()->getTextureHeight();
                    float fHorizQuadMargin = vDiff.x() == 0.0f ? 0.0f : width * fHorizTCMargin / vDiff.x();
                    float fVertQuadMargin = vDiff.y() == 0.0f ? 0.0f : height * fVertTCMargin / vDiff.y();

                    mintc.x() -= fHorizTCMargin;
                    mintc.y() -= fVertTCMargin;
                    maxtc.x() += fHorizTCMargin;
                    maxtc.y() += fVertTCMargin;

                    // set up the coords of the quad
                    osg::Vec2 upLeft = local+osg::Vec2(0.0f-fHorizQuadMargin,height+fVertQuadMargin);
                    osg::Vec2 lowLeft = local+osg::Vec2(0.0f-fHorizQuadMargin,0.0f-fVertQuadMargin);
                    osg::Vec2 lowRight = local+osg::Vec2(width+fHorizQuadMargin,0.0f-fVertQuadMargin);
                    osg::Vec2 upRight = local+osg::Vec2(width+fHorizQuadMargin,height+fVertQuadMargin);
                    glyphquad._coords.push_back(upLeft);
                    glyphquad._coords.push_back(lowLeft);
                    glyphquad._coords.push_back(lowRight);
                    glyphquad._coords.push_back(upRight);

                    // set up the tex coords of the quad
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
                          _textBB.expandBy(osg::Vec3(lowLeft.x(), lowLeft.y(), 0.0f)); //lower left corner
                          _textBB.expandBy(osg::Vec3(upRight.x(), upRight.y(), 0.0f)); //upper right corner
                          break;
                      case VERTICAL:
                          cursor.y() -= glyph->getVerticalAdvance() * hr;
                          _textBB.expandBy(osg::Vec3(upLeft.x(),upLeft.y(),0.0f)); //upper left corner
                          _textBB.expandBy(osg::Vec3(lowRight.x(),lowRight.y(),0.0f)); //lower right corner
                          break;
                      case RIGHT_TO_LEFT:
                          _textBB.expandBy(osg::Vec3(lowRight.x(),lowRight.y(),0.0f)); //lower right corner
                          _textBB.expandBy(osg::Vec3(upLeft.x(),upLeft.y(),0.0f)); //upper left corner
                          break;
                    }
                    previous_charcode = charcode;

                }
            }

            // skip over spaces and return.
            while (itr != _text.end() && *itr==' ') ++itr;
            if (itr != _text.end() && *itr=='\n') ++itr;
        }
        else
        {
            ++itr;
        }


        // move to new line.
        switch(_layout)
        {
          case LEFT_TO_RIGHT:
          {
            startOfLine_coords.y() -= _characterHeight * (1.0 + _lineSpacing);
            cursor = startOfLine_coords;
            previous_charcode = 0;
            _lineCount++;
            break;
          }
          case RIGHT_TO_LEFT:
          {
            startOfLine_coords.y() -= _characterHeight * (1.0 + _lineSpacing);
            cursor = startOfLine_coords;
            previous_charcode = 0;
            _lineCount++;
            break;
          }
          case VERTICAL:
          {
            startOfLine_coords.x() += _characterHeight/getCharacterAspectRatio() * (1.0 + _lineSpacing);
            cursor = startOfLine_coords;
            previous_charcode = 0;
            // because _lineCount is the max vertical no. of characters....
            _lineCount = (_lineCount >linelength)?_lineCount:linelength;
          }
          break;
        }

        ++lineNumber;

    }

    TextBase::computePositions();
    computeBackdropBoundingBox();
    computeBoundingBoxMargin();
    computeColorGradients();
}

// Returns false if there are no glyphs and the width/height values are invalid.
// Also sets avg_width and avg_height to 0.0f if the value is invalid.
// This method is used several times in a loop for the same object which will produce the same values.
// Further optimization may try saving these values instead of recomputing them.
bool Text::computeAverageGlyphWidthAndHeight(float& avg_width, float& avg_height) const
{
    float width = 0.0f;
    float height = 0.0f;
    float running_width = 0.0f;
    float running_height = 0.0f;
    avg_width = 0.0f;
    avg_height = 0.0f;
    int counter = 0;
    unsigned int i;
    bool is_valid_size = true;
    // This section is going to try to compute the average width and height
    // for a character among the text. The reason I shift by an
    // average amount per-character instead of shifting each character
    // by its per-instance amount is because it may look strange to see
    // the individual backdrop text letters not space themselves the same
    // way the foreground text does. Using one value gives uniformity.
    // Note: This loop is repeated for each context. I think it may produce
    // the same values regardless of context. This code be optimized by moving
    // this loop outside the loop.
    for(TextureGlyphQuadMap::const_iterator const_titr=_textureGlyphQuadMap.begin();
        const_titr!=_textureGlyphQuadMap.end();
        ++const_titr)
    {
        const GlyphQuads& glyphquad = const_titr->second;
        const GlyphQuads::Coords2& coords2 = glyphquad._coords;
        for(i = 0; i < coords2.size(); i+=4)
        {
            width = coords2[i+2].x() - coords2[i].x();
            height = coords2[i].y() - coords2[i+1].y();

            running_width += width;
            running_height += height;
            counter++;
        }
    }
    if(0 == counter)
    {
        is_valid_size = false;
    }
    else
    {
        avg_width = running_width/counter;
        avg_height = running_height/counter;
    }
    return is_valid_size;
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

    case LEFT_BOTTOM_BASE_LINE:  _offset.set(0.0f,-_characterHeight*(1.0 + _lineSpacing)*(_lineCount-1),0.0f); break;
    case CENTER_BOTTOM_BASE_LINE:  _offset.set((_textBB.xMax()+_textBB.xMin())*0.5f,-_characterHeight*(1.0 + _lineSpacing)*(_lineCount-1),0.0f); break;
    case RIGHT_BOTTOM_BASE_LINE:  _offset.set(_textBB.xMax(),-_characterHeight*(1.0 + _lineSpacing)*(_lineCount-1),0.0f); break;
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

        matrix.postMultRotate(_rotation);

        if (_characterSizeMode!=OBJECT_COORDS)
        {

            osg::Matrix M(rotate_matrix);
            M.postMultTranslate(_position);
            M.postMult(atc._modelview);
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
            float pixelSizeHori=(_characterHeight/getCharacterAspectRatio()*sqrtf(scale_00.length2()))/(pixelSizeVector_w*0.701f);

            // avoid nasty math by preventing a divide by zero
            if (pixelSizeVert == 0.0f)
               pixelSizeVert= 1.0f;
            if (pixelSizeHori == 0.0f)
               pixelSizeHori= 1.0f;

            if (_characterSizeMode==SCREEN_COORDS)
            {
                float scale_font_vert=_characterHeight/pixelSizeVert;
                float scale_font_hori=_characterHeight/getCharacterAspectRatio()/pixelSizeHori;

                if (P10<0)
                   scale_font_vert=-scale_font_vert;
                matrix.postMultScale(osg::Vec3f(scale_font_hori, scale_font_vert,1.0f));
            }
            else if (pixelSizeVert>getFontHeight())
            {
                float scale_font = getFontHeight()/pixelSizeVert;
                matrix.postMultScale(osg::Vec3f(scale_font, scale_font,1.0f));
            }

        }

        if (_autoRotateToScreen)
        {
            matrix.postMult(rotate_matrix);
        }

        matrix.postMultTranslate(_position);
    }
    else if (!_rotation.zeroRotation())
    {
        matrix.makeRotate(_rotation);
        matrix.preMultTranslate(-_offset);
        matrix.postMultTranslate(_position);
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

    computeBackdropPositions(contextID);

    _normal = osg::Matrix::transform3x3(osg::Vec3(0.0f,0.0f,1.0f),matrix);
    _normal.normalize();

    const_cast<Text*>(this)->dirtyBound();
}

// Presumes the atc matrix is already up-to-date
void Text::computeBackdropPositions(unsigned int contextID) const
{
    if(_backdropType == NONE)
    {
        return;
    }

    float avg_width = 0.0f;
    float avg_height = 0.0f;
    unsigned int i;
    bool is_valid_size;

    AutoTransformCache& atc = _autoTransformCache[contextID];
    osg::Matrix& matrix = atc._matrix;

    // FIXME: OPTIMIZE: This function produces the same value regardless of contextID.
    // Since we tend to loop over contextID, we should cache this value some how
    // instead of recomputing it each time.
    is_valid_size = computeAverageGlyphWidthAndHeight(avg_width, avg_height);

    if (!is_valid_size) return;

    // now apply matrix to the glyphs.
    for(TextureGlyphQuadMap::iterator titr=_textureGlyphQuadMap.begin();
        titr!=_textureGlyphQuadMap.end();
        ++titr)
    {
        GlyphQuads& glyphquad = titr->second;
        GlyphQuads::Coords2& coords2 = glyphquad._coords;

        unsigned int backdrop_index;
        unsigned int max_backdrop_index;
        if(_backdropType == OUTLINE)
        {
            // For outline, we want to draw the in every direction
            backdrop_index = 0;
            max_backdrop_index = 8;
        }
        else
        {
            // Yes, this may seem a little strange,
            // but since the code is using references,
            // I would have to duplicate the following code twice
            // for each part of the if/else because I can't
            // declare a reference without setting it immediately
            // and it wouldn't survive the scope.
            // So it happens that the _backdropType value matches
            // the index in the array I want to store the coordinates
            // in. So I'll just setup the for-loop so it only does
            // the one direction I'm interested in.
            backdrop_index = _backdropType;
            max_backdrop_index = _backdropType+1;
        }
        for( ; backdrop_index < max_backdrop_index; backdrop_index++)
        {
            GlyphQuads::Coords3& transformedCoords = glyphquad._transformedBackdropCoords[backdrop_index][contextID];
            unsigned int numCoords = coords2.size();
            if (numCoords!=transformedCoords.size())
            {
                transformedCoords.resize(numCoords);
            }

            for(i=0;i<numCoords;++i)
            {
                float horizontal_shift_direction;
                float vertical_shift_direction;
                switch(backdrop_index)
                {
                    case DROP_SHADOW_BOTTOM_RIGHT:
                        {
                            horizontal_shift_direction = 1.0f;
                            vertical_shift_direction = -1.0f;
                            break;
                        }
                    case DROP_SHADOW_CENTER_RIGHT:
                        {
                            horizontal_shift_direction = 1.0f;
                            vertical_shift_direction = 0.0f;
                            break;
                        }
                    case DROP_SHADOW_TOP_RIGHT:
                        {
                            horizontal_shift_direction = 1.0f;
                            vertical_shift_direction = 1.0f;
                            break;
                        }
                    case DROP_SHADOW_BOTTOM_CENTER:
                        {
                            horizontal_shift_direction = 0.0f;
                            vertical_shift_direction = -1.0f;
                            break;
                        }
                    case DROP_SHADOW_TOP_CENTER:
                        {
                            horizontal_shift_direction = 0.0f;
                            vertical_shift_direction = 1.0f;
                            break;
                        }
                    case DROP_SHADOW_BOTTOM_LEFT:
                        {
                            horizontal_shift_direction = -1.0f;
                            vertical_shift_direction = -1.0f;
                            break;
                        }
                    case DROP_SHADOW_CENTER_LEFT:
                        {
                            horizontal_shift_direction = -1.0f;
                            vertical_shift_direction = 0.0f;
                            break;
                        }
                    case DROP_SHADOW_TOP_LEFT:
                        {
                            horizontal_shift_direction = -1.0f;
                            vertical_shift_direction = 1.0f;
                            break;
                        }
                    default: // error
                        {
                            horizontal_shift_direction = 1.0f;
                            vertical_shift_direction = -1.0f;
                        }
                }
                transformedCoords[i] = osg::Vec3(horizontal_shift_direction * _backdropHorizontalOffset * avg_width+coords2[i].x(),vertical_shift_direction * _backdropVerticalOffset * avg_height+coords2[i].y(),0.0f)*matrix;
            }
        }
    }
}

// This method adjusts the bounding box to account for the expanded area caused by the backdrop.
// This assumes that the bounding box has already been computed for the text without the backdrop.
void Text::computeBackdropBoundingBox() const
{
    if(_backdropType == NONE)
    {
        return;
    }

    float avg_width = 0.0f;
    float avg_height = 0.0f;
    bool is_valid_size;

    // FIXME: OPTIMIZE: It is possible that this value has already been computed before
    // from previous calls to this function. This might be worth optimizing.
    is_valid_size = computeAverageGlyphWidthAndHeight(avg_width, avg_height);

    // Finally, we have one more issue to deal with.
    // Now that the text takes more space, we need
    // to adjust the size of the bounding box.
    if((!_textBB.valid() || !is_valid_size))
    {
        return;
    }

    // Finally, we have one more issue to deal with.
    // Now that the text takes more space, we need
    // to adjust the size of the bounding box.
    switch(_backdropType)
    {
        case DROP_SHADOW_BOTTOM_RIGHT:
            {
                _textBB.set(
                    _textBB.xMin(),
                    _textBB.yMin() - avg_height * _backdropVerticalOffset,
                    _textBB.zMin(),
                    _textBB.xMax() + avg_width * _backdropHorizontalOffset,
                    _textBB.yMax(),
                    _textBB.zMax()
                );
                break;
            }
        case DROP_SHADOW_CENTER_RIGHT:
            {
                _textBB.set(
                    _textBB.xMin(),
                    _textBB.yMin(),
                    _textBB.zMin(),
                    _textBB.xMax() + avg_width * _backdropHorizontalOffset,
                    _textBB.yMax(),
                    _textBB.zMax()
                );
                break;
            }
        case DROP_SHADOW_TOP_RIGHT:
            {
                _textBB.set(
                    _textBB.xMin(),
                    _textBB.yMin(),
                    _textBB.zMin(),
                    _textBB.xMax() + avg_width * _backdropHorizontalOffset,
                    _textBB.yMax() + avg_height * _backdropVerticalOffset,
                    _textBB.zMax()
                );
                break;
            }
        case DROP_SHADOW_BOTTOM_CENTER:
            {
                _textBB.set(
                    _textBB.xMin(),
                    _textBB.yMin() - avg_height * _backdropVerticalOffset,
                    _textBB.zMin(),
                    _textBB.xMax(),
                    _textBB.yMax(),
                    _textBB.zMax()
                );
                break;
            }
        case DROP_SHADOW_TOP_CENTER:
            {
                _textBB.set(
                    _textBB.xMin(),
                    _textBB.yMin(),
                    _textBB.zMin(),
                    _textBB.xMax(),
                    _textBB.yMax() + avg_height * _backdropVerticalOffset,
                    _textBB.zMax()
                );
                break;
            }
        case DROP_SHADOW_BOTTOM_LEFT:
            {
                _textBB.set(
                    _textBB.xMin() - avg_width * _backdropHorizontalOffset,
                    _textBB.yMin() - avg_height * _backdropVerticalOffset,
                    _textBB.zMin(),
                    _textBB.xMax(),
                    _textBB.yMax(),
                    _textBB.zMax()
                );
                break;
            }
        case DROP_SHADOW_CENTER_LEFT:
            {
                _textBB.set(
                    _textBB.xMin() - avg_width * _backdropHorizontalOffset,
                    _textBB.yMin(),
                    _textBB.zMin(),
                    _textBB.xMax(),
                    _textBB.yMax(),
                    _textBB.zMax()
                );            break;
            }
        case DROP_SHADOW_TOP_LEFT:
            {
                _textBB.set(
                    _textBB.xMin() - avg_width * _backdropHorizontalOffset,
                    _textBB.yMin(),
                    _textBB.zMin(),
                    _textBB.xMax(),
                    _textBB.yMax() + avg_height * _backdropVerticalOffset,
                    _textBB.zMax()
                );
                break;
            }
        case OUTLINE:
            {
                _textBB.set(
                    _textBB.xMin() - avg_width * _backdropHorizontalOffset,
                    _textBB.yMin() - avg_height * _backdropVerticalOffset,
                    _textBB.zMin(),
                    _textBB.xMax() + avg_width * _backdropHorizontalOffset,
                    _textBB.yMax() + avg_height * _backdropVerticalOffset,
                    _textBB.zMax()
                );
                break;
            }
        default: // error
            {
                break;
            }
    }
}

// This method expands the bounding box to a settable margin when a bounding box drawing mode is active.
void Text::computeBoundingBoxMargin() const
{
    if(_drawMode & (BOUNDINGBOX | FILLEDBOUNDINGBOX)){
        _textBB.set(
            _textBB.xMin() - _textBBMargin,
            _textBB.yMin() - _textBBMargin,
            _textBB.zMin(),
            _textBB.xMax() + _textBBMargin,
            _textBB.yMax() + _textBBMargin,
            _textBB.zMax()
        );
    }
}

void Text::computeColorGradients() const
{
    switch(_colorGradientMode)
    {
        case SOLID:
            return;
            break;
        case PER_CHARACTER:
            computeColorGradientsPerCharacter();
            break;
        case OVERALL:
            computeColorGradientsOverall();
            break;
        default:
            break;
    }
}

void Text::computeColorGradientsOverall() const
{

    float min_x = FLT_MAX;
    float min_y = FLT_MAX;
    float max_x = FLT_MIN;
    float max_y = FLT_MIN;

    unsigned int i;

    for(TextureGlyphQuadMap::const_iterator const_titr=_textureGlyphQuadMap.begin();
        const_titr!=_textureGlyphQuadMap.end();
        ++const_titr)
    {
        const GlyphQuads& glyphquad = const_titr->second;
        const GlyphQuads::Coords2& coords2 = glyphquad._coords;

        for(i=0;i<coords2.size();++i)
        {
            // Min and Max are needed for color gradients
            if(coords2[i].x() > max_x)
            {
                max_x = coords2[i].x();
            }
            if(coords2[i].x() < min_x)
            {
                min_x = coords2[i].x();
            }
            if(coords2[i].y() > max_y)
            {
                max_y = coords2[i].y();
            }
            if(coords2[i].y() < min_y)
            {
                min_y = coords2[i].y();
            }

        }
    }

    for(TextureGlyphQuadMap::iterator titr=_textureGlyphQuadMap.begin();
        titr!=_textureGlyphQuadMap.end();
        ++titr)
    {
        GlyphQuads& glyphquad = titr->second;
        GlyphQuads::Coords2& coords2 = glyphquad._coords;
        GlyphQuads::ColorCoords& colorCoords = glyphquad._colorCoords;

        unsigned int numCoords = coords2.size();
        if (numCoords!=colorCoords.size())
        {
            colorCoords.resize(numCoords);
        }

        for(i=0;i<numCoords;++i)
        {
            float red = bilinearInterpolate(
                min_x,
                max_x,
                min_y,
                max_y,
                coords2[i].x(),
                coords2[i].y(),
                _colorGradientBottomLeft[0],
                _colorGradientTopLeft[0],
                _colorGradientBottomRight[0],
                _colorGradientTopRight[0]
            );

            float green = bilinearInterpolate(
                min_x,
                max_x,
                min_y,
                max_y,
                coords2[i].x(),
                coords2[i].y(),
                _colorGradientBottomLeft[1],
                _colorGradientTopLeft[1],
                _colorGradientBottomRight[1],
                _colorGradientTopRight[1]
            );

            float blue = bilinearInterpolate(
                min_x,
                max_x,
                min_y,
                max_y,
                coords2[i].x(),
                coords2[i].y(),
                _colorGradientBottomLeft[2],
                _colorGradientTopLeft[2],
                _colorGradientBottomRight[2],
                _colorGradientTopRight[2]
            );
            // Alpha does not convert to HSV
            float alpha = bilinearInterpolate(
                min_x,
                max_x,
                min_y,
                max_y,
                coords2[i].x(),
                coords2[i].y(),
                _colorGradientBottomLeft[3],
                _colorGradientTopLeft[3],
                _colorGradientBottomRight[3],
                _colorGradientTopRight[3]
            );

            colorCoords[i] = osg::Vec4(red,green,blue,alpha);
        }
    }
}

void Text::computeColorGradientsPerCharacter() const
{
    for(TextureGlyphQuadMap::iterator titr=_textureGlyphQuadMap.begin();
        titr!=_textureGlyphQuadMap.end();
        ++titr)
    {
        GlyphQuads& glyphquad = titr->second;
        GlyphQuads::Coords2& coords2 = glyphquad._coords;
        GlyphQuads::ColorCoords& colorCoords = glyphquad._colorCoords;

        unsigned int numCoords = coords2.size();
        if (numCoords!=colorCoords.size())
        {
            colorCoords.resize(numCoords);
        }

        for(unsigned int i=0;i<numCoords;++i)
        {
            switch(i%4)
            {
                case 0: // top-left
                    {
                        colorCoords[i] = _colorGradientTopLeft;
                        break;
                    }
                case 1: // bottom-left
                    {
                        colorCoords[i] = _colorGradientBottomLeft;
                        break;
                    }
                case 2: // bottom-right
                    {
                        colorCoords[i] = _colorGradientBottomRight;
                        break;
                    }
                case 3: // top-right
                    {
                        colorCoords[i] = _colorGradientTopRight;
                        break;
                    }
                default: // error
                    {
                        colorCoords[i] = osg::Vec4(0.0f,0.0f,0.0f,1.0f);
                    }
            }
        }
    }
}

void Text::drawImplementation(osg::RenderInfo& renderInfo) const
{
    drawImplementation(*renderInfo.getState(), osg::Vec4(1.0f,1.0f,1.0f,1.0f));
}

void Text::drawImplementation(osg::State& state, const osg::Vec4& colorMultiplier) const
{
    unsigned int contextID = state.getContextID();

    state.applyMode(GL_BLEND,true);
#if defined(OSG_GL_FIXED_FUNCTION_AVAILABLE)
    state.applyTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::ON);
    state.applyTextureAttribute(0,getActiveFont()->getTexEnv());
#endif
    if (_characterSizeMode!=OBJECT_COORDS || _autoRotateToScreen)
    {
        unsigned int frameNumber = state.getFrameStamp()?state.getFrameStamp()->getFrameNumber():0;
        AutoTransformCache& atc = _autoTransformCache[contextID];
        const osg::Matrix& modelview = state.getModelViewMatrix();
        const osg::Matrix& projection = state.getProjectionMatrix();

        osg::Vec3 newTransformedPosition = _position*modelview;

        int width = atc._width;
        int height = atc._height;

        const osg::Viewport* viewport = state.getCurrentViewport();
        if (viewport)
        {
            width = static_cast<int>(viewport->width());
            height = static_cast<int>(viewport->height());
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

    osg::GLBeginEndAdapter& gl = (state.getGLBeginEndAdapter());

    state.Normal(_normal.x(), _normal.y(), _normal.z());

    if (_drawMode & FILLEDBOUNDINGBOX)
    {
        if (_textBB.valid())
        {
        #if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
            state.applyTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::OFF);

            const osg::Matrix& matrix = _autoTransformCache[contextID]._matrix;

            osg::Vec3 c00(osg::Vec3(_textBB.xMin(),_textBB.yMin(),_textBB.zMin())*matrix);
            osg::Vec3 c10(osg::Vec3(_textBB.xMax(),_textBB.yMin(),_textBB.zMin())*matrix);
            osg::Vec3 c11(osg::Vec3(_textBB.xMax(),_textBB.yMax(),_textBB.zMin())*matrix);
            osg::Vec3 c01(osg::Vec3(_textBB.xMin(),_textBB.yMax(),_textBB.zMin())*matrix);

            switch(_backdropImplementation)
            {
                case NO_DEPTH_BUFFER:
                    // Do nothing.  The bounding box will be rendered before the text and that's all that matters.
                    break;
                case DEPTH_RANGE:
                    glPushAttrib(GL_DEPTH_BUFFER_BIT);
                    //unsigned int backdrop_index = 0;
                    //unsigned int max_backdrop_index = 8;
                    //const double offset = double(max_backdrop_index - backdrop_index) * 0.003;
                    glDepthRange(0.001, 1.001);
                    break;
                /*case STENCIL_BUFFER:
                    break;*/
                default:
                    glPushAttrib(GL_POLYGON_OFFSET_FILL);
                    glEnable(GL_POLYGON_OFFSET_FILL);
                    glPolygonOffset(0.1f * osg::PolygonOffset::getFactorMultiplier(), 10.0f * osg::PolygonOffset::getUnitsMultiplier() );
            }

            gl.Color4f(colorMultiplier.r()*_textBBColor.r(),colorMultiplier.g()*_textBBColor.g(),colorMultiplier.b()*_textBBColor.b(),colorMultiplier.a()*_textBBColor.a());
            gl.Begin(GL_QUADS);
                gl.Vertex3fv(c00.ptr());
                gl.Vertex3fv(c10.ptr());
                gl.Vertex3fv(c11.ptr());
                gl.Vertex3fv(c01.ptr());
            gl.End();

            switch(_backdropImplementation)
            {
                case NO_DEPTH_BUFFER:
                    // Do nothing.
                    break;
                case DEPTH_RANGE:
                    glDepthRange(0.0, 1.0);
                    glPopAttrib();
                    break;
                /*case STENCIL_BUFFER:
                    break;*/
                default:
                    glDisable(GL_POLYGON_OFFSET_FILL);
                    glPopAttrib();
            }
        #else
            OSG_NOTICE<<"Warning: Text::drawImplementation() fillMode FILLEDBOUNDINGBOX not supported"<<std::endl;
        #endif
        }
    }

#if defined(OSG_GL_FIXED_FUNCTION_AVAILABLE)
    state.applyTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::ON);
    state.applyTextureAttribute(0,getActiveFont()->getTexEnv());
#endif

    if (_drawMode & TEXT)
    {

        state.disableAllVertexArrays();

        // Okay, since ATI's cards/drivers are not working correctly,
        // we need alternative solutions to glPolygonOffset.
        // So this is a pick your poison approach. Each alternative
        // backend has trade-offs associated with it, but with luck,
        // the user may find that works for them.
        if(_backdropType != NONE && _backdropImplementation != DELAYED_DEPTH_WRITES)
        {
            switch(_backdropImplementation)
            {
                case POLYGON_OFFSET:
                    renderWithPolygonOffset(state,colorMultiplier);
                    break;
                case NO_DEPTH_BUFFER:
                    renderWithNoDepthBuffer(state,colorMultiplier);
                    break;
                case DEPTH_RANGE:
                    renderWithDepthRange(state,colorMultiplier);
                    break;
                case STENCIL_BUFFER:
                    renderWithStencilBuffer(state,colorMultiplier);
                    break;
                default:
                    renderWithPolygonOffset(state,colorMultiplier);
            }
        }
        else
        {
            renderWithDelayedDepthWrites(state,colorMultiplier);
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


            gl.Color4f(colorMultiplier.r()*_textBBColor.r(),colorMultiplier.g()*_textBBColor.g(),colorMultiplier.b()*_textBBColor.b(),colorMultiplier.a()*_textBBColor.a());
            gl.Begin(GL_LINE_LOOP);
                gl.Vertex3fv(c00.ptr());
                gl.Vertex3fv(c10.ptr());
                gl.Vertex3fv(c11.ptr());
                gl.Vertex3fv(c01.ptr());
            gl.End();
        }
    }

    if (_drawMode & ALIGNMENT)
    {
        gl.Color4fv(colorMultiplier.ptr());

        float cursorsize = _characterHeight*0.5f;

        const osg::Matrix& matrix = _autoTransformCache[contextID]._matrix;

        osg::Vec3 hl(osg::Vec3(_offset.x()-cursorsize,_offset.y(),_offset.z())*matrix);
        osg::Vec3 hr(osg::Vec3(_offset.x()+cursorsize,_offset.y(),_offset.z())*matrix);
        osg::Vec3 vt(osg::Vec3(_offset.x(),_offset.y()-cursorsize,_offset.z())*matrix);
        osg::Vec3 vb(osg::Vec3(_offset.x(),_offset.y()+cursorsize,_offset.z())*matrix);

        state.applyTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::OFF);

        gl.Begin(GL_LINES);
            gl.Vertex3fv(hl.ptr());
            gl.Vertex3fv(hr.ptr());
            gl.Vertex3fv(vt.ptr());
            gl.Vertex3fv(vb.ptr());
        gl.End();

    }
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


void Text::setThreadSafeRefUnref(bool threadSafe)
{
    TextBase::setThreadSafeRefUnref(threadSafe);

    getActiveFont()->setThreadSafeRefUnref(threadSafe);
}

void Text::resizeGLObjectBuffers(unsigned int maxSize)
{
    TextBase::resizeGLObjectBuffers(maxSize);

    getActiveFont()->resizeGLObjectBuffers(maxSize);
}


void Text::releaseGLObjects(osg::State* state) const
{
    TextBase::releaseGLObjects(state);
    getActiveFont()->releaseGLObjects(state);
}


void Text::setBackdropType(BackdropType type)
{
    if (_backdropType==type) return;

    _backdropType = type;
    computeGlyphRepresentation();
}

void Text::setBackdropImplementation(BackdropImplementation implementation)
{
    if (_backdropImplementation==implementation) return;

    _backdropImplementation = implementation;
    computeGlyphRepresentation();
}


void Text::setBackdropOffset(float offset)
{
    _backdropHorizontalOffset = offset;
    _backdropVerticalOffset = offset;
    computeGlyphRepresentation();
}

void Text::setBackdropOffset(float horizontal, float vertical)
{
    _backdropHorizontalOffset = horizontal;
    _backdropVerticalOffset = vertical;
    computeGlyphRepresentation();
}

void Text::setBackdropColor(const osg::Vec4& color)
{
    _backdropColor = color;
}

void Text::setColorGradientMode(ColorGradientMode mode)
{
    if (_colorGradientMode==mode) return;

    _colorGradientMode = mode;
    computeGlyphRepresentation();
}

void Text::setColorGradientCorners(const osg::Vec4& topLeft, const osg::Vec4& bottomLeft, const osg::Vec4& bottomRight, const osg::Vec4& topRight)
{
    _colorGradientTopLeft = topLeft;
    _colorGradientBottomLeft = bottomLeft;
    _colorGradientBottomRight = bottomRight;
    _colorGradientTopRight = topRight;
    computeGlyphRepresentation();
}

// Formula for f(x,y) from Wikipedia "Bilinear interpolation", 2006-06-18
float Text::bilinearInterpolate(float x1, float x2, float y1, float y2, float x, float y, float q11, float q12, float q21, float q22) const
{
    return (
        ((q11 / ((x2-x1)*(y2-y1))) * (x2-x)*(y2-y))
        + ((q21 / ((x2-x1)*(y2-y1))) * (x-x1)*(y2-y))
        + ((q12 / ((x2-x1)*(y2-y1))) * (x2-x)*(y-y1))
        + ((q22 / ((x2-x1)*(y2-y1))) * (x-x1)*(y-y1))
    );
}

void Text::drawForegroundText(osg::State& state, const GlyphQuads& glyphquad, const osg::Vec4& colorMultiplier) const
{
    unsigned int contextID = state.getContextID();

    const GlyphQuads::Coords3& transformedCoords = glyphquad._transformedCoords[contextID];
    if (!transformedCoords.empty())
    {
        state.setVertexPointer( 3, GL_FLOAT, 0, &(transformedCoords.front()));
        state.setTexCoordPointer( 0, 2, GL_FLOAT, 0, &(glyphquad._texcoords.front()));

        if(_colorGradientMode == SOLID)
        {
            state.disableColorPointer();
            state.Color(colorMultiplier.r()*_color.r(),colorMultiplier.g()*_color.g(),colorMultiplier.b()*_color.b(),colorMultiplier.a()*_color.a());
        }
        else
        {
            state.setColorPointer( 4, GL_FLOAT, 0, &(glyphquad._colorCoords.front()));
        }

        state.drawQuads(0,transformedCoords.size());

    }
}

void Text::renderOnlyForegroundText(osg::State& state, const osg::Vec4& colorMultiplier) const
{
    for(TextureGlyphQuadMap::iterator titr=_textureGlyphQuadMap.begin();
        titr!=_textureGlyphQuadMap.end();
        ++titr)
    {
        // need to set the texture here...
        state.applyTextureAttribute(0,titr->first.get());

        const GlyphQuads& glyphquad = titr->second;

        drawForegroundText(state, glyphquad, colorMultiplier);
    }
}

void Text::renderWithDelayedDepthWrites(osg::State& state, const osg::Vec4& colorMultiplier) const
{
    // If depth testing is disabled, then just render text as normal
    if( !state.getLastAppliedMode(GL_DEPTH_TEST) ) {
        drawTextWithBackdrop(state,colorMultiplier);
        return;
    }

    //glPushAttrib( _enableDepthWrites ? (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) : GL_DEPTH_BUFFER_BIT);
    // Render to color buffer without writing to depth buffer.
    glDepthMask(GL_FALSE);
    drawTextWithBackdrop(state,colorMultiplier);

    // Render to depth buffer if depth writes requested.
    if( _enableDepthWrites )
    {
        glDepthMask(GL_TRUE);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        drawTextWithBackdrop(state,colorMultiplier);
    }

    state.haveAppliedAttribute(osg::StateAttribute::DEPTH);
    state.haveAppliedAttribute(osg::StateAttribute::COLORMASK);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    //glPopAttrib();
}

void Text::drawTextWithBackdrop(osg::State& state, const osg::Vec4& colorMultiplier) const
{
    unsigned int contextID = state.getContextID();

    for(TextureGlyphQuadMap::iterator titr=_textureGlyphQuadMap.begin();
        titr!=_textureGlyphQuadMap.end();
        ++titr)
    {
        // need to set the texture here...
        state.applyTextureAttribute(0,titr->first.get());

        const GlyphQuads& glyphquad = titr->second;

        if(_backdropType != NONE)
        {
            unsigned int backdrop_index;
            unsigned int max_backdrop_index;
            if(_backdropType == OUTLINE)
            {
                backdrop_index = 0;
                max_backdrop_index = 8;
            }
            else
            {
                backdrop_index = _backdropType;
                max_backdrop_index = _backdropType+1;
            }

            state.setTexCoordPointer( 0, 2, GL_FLOAT, 0, &(glyphquad._texcoords.front()));
            state.disableColorPointer();
            state.Color(_backdropColor.r(),_backdropColor.g(),_backdropColor.b(),_backdropColor.a());

            for( ; backdrop_index < max_backdrop_index; backdrop_index++)
            {
                const GlyphQuads::Coords3& transformedBackdropCoords = glyphquad._transformedBackdropCoords[backdrop_index][contextID];
                if (!transformedBackdropCoords.empty())
                {
                    state.setVertexPointer( 3, GL_FLOAT, 0, &(transformedBackdropCoords.front()));
                    state.drawQuads(0,transformedBackdropCoords.size());
                }
            }
        }

        drawForegroundText(state, glyphquad, colorMultiplier);
    }
}


void Text::renderWithPolygonOffset(osg::State& state, const osg::Vec4& colorMultiplier) const
{
#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
    unsigned int contextID = state.getContextID();

    if (!osg::PolygonOffset::areFactorAndUnitsMultipliersSet())
    {
        osg::PolygonOffset::setFactorAndUnitsMultipliersUsingBestGuessForDriver();
    }

    // Do I really need to do this for glPolygonOffset?
    glPushAttrib(GL_POLYGON_OFFSET_FILL);
    glEnable(GL_POLYGON_OFFSET_FILL);

    for(TextureGlyphQuadMap::iterator titr=_textureGlyphQuadMap.begin();
        titr!=_textureGlyphQuadMap.end();
        ++titr)
    {
        // need to set the texture here...
        state.applyTextureAttribute(0,titr->first.get());

        const GlyphQuads& glyphquad = titr->second;

        unsigned int backdrop_index;
        unsigned int max_backdrop_index;
        if(_backdropType == OUTLINE)
        {
            backdrop_index = 0;
            max_backdrop_index = 8;
        }
        else
        {
            backdrop_index = _backdropType;
            max_backdrop_index = _backdropType+1;
        }

        state.setTexCoordPointer( 0, 2, GL_FLOAT, 0, &(glyphquad._texcoords.front()));
        state.disableColorPointer();
        state.Color(_backdropColor.r(),_backdropColor.g(),_backdropColor.b(),_backdropColor.a());

        for( ; backdrop_index < max_backdrop_index; backdrop_index++)
        {
            const GlyphQuads::Coords3& transformedBackdropCoords = glyphquad._transformedBackdropCoords[backdrop_index][contextID];
            if (!transformedBackdropCoords.empty())
            {
                state.setVertexPointer( 3, GL_FLOAT, 0, &(transformedBackdropCoords.front()));
                glPolygonOffset(0.1f * osg::PolygonOffset::getFactorMultiplier(),
                                osg::PolygonOffset::getUnitsMultiplier() * (max_backdrop_index-backdrop_index) );
                state.drawQuads(0,transformedBackdropCoords.size());
            }
        }

        // Reset the polygon offset so the foreground text is on top
        glPolygonOffset(0.0f,0.0f);

        drawForegroundText(state, glyphquad, colorMultiplier);
    }

    glPopAttrib();
#else
    OSG_NOTICE<<"Warning: Text::renderWithPolygonOffset(..) not implemented."<<std::endl;
#endif
}


void Text::renderWithNoDepthBuffer(osg::State& state, const osg::Vec4& colorMultiplier) const
{
#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
    unsigned int contextID = state.getContextID();

    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    for(TextureGlyphQuadMap::iterator titr=_textureGlyphQuadMap.begin();
        titr!=_textureGlyphQuadMap.end();
        ++titr)
    {
        // need to set the texture here...
        state.applyTextureAttribute(0,titr->first.get());

        const GlyphQuads& glyphquad = titr->second;

        unsigned int backdrop_index;
        unsigned int max_backdrop_index;
        if(_backdropType == OUTLINE)
        {
            backdrop_index = 0;
            max_backdrop_index = 8;
        }
        else
        {
            backdrop_index = _backdropType;
            max_backdrop_index = _backdropType+1;
        }

        state.setTexCoordPointer( 0, 2, GL_FLOAT, 0, &(glyphquad._texcoords.front()));
        state.disableColorPointer();
        state.Color(_backdropColor.r(),_backdropColor.g(),_backdropColor.b(),_backdropColor.a());

        for( ; backdrop_index < max_backdrop_index; backdrop_index++)
        {
            const GlyphQuads::Coords3& transformedBackdropCoords = glyphquad._transformedBackdropCoords[backdrop_index][contextID];
            if (!transformedBackdropCoords.empty())
            {
                state.setVertexPointer( 3, GL_FLOAT, 0, &(transformedBackdropCoords.front()));
                state.drawQuads(0,transformedBackdropCoords.size());
            }
        }

        drawForegroundText(state, glyphquad, colorMultiplier);
    }

    glPopAttrib();
#else
    OSG_NOTICE<<"Warning: Text::renderWithNoDepthBuffer(..) not implemented."<<std::endl;
#endif
}

// This idea comes from Paul Martz's OpenGL FAQ: 13.050
void Text::renderWithDepthRange(osg::State& state, const osg::Vec4& colorMultiplier) const
{
#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
    unsigned int contextID = state.getContextID();

    // Hmmm, the man page says GL_VIEWPORT_BIT for Depth range (near and far)
    // but experimentally, GL_DEPTH_BUFFER_BIT for glDepthRange.
//    glPushAttrib(GL_VIEWPORT_BIT);
    glPushAttrib(GL_DEPTH_BUFFER_BIT);

    for(TextureGlyphQuadMap::iterator titr=_textureGlyphQuadMap.begin();
        titr!=_textureGlyphQuadMap.end();
        ++titr)
    {
        // need to set the texture here...
        state.applyTextureAttribute(0,titr->first.get());

        const GlyphQuads& glyphquad = titr->second;

        unsigned int backdrop_index;
        unsigned int max_backdrop_index;
        if(_backdropType == OUTLINE)
        {
            backdrop_index = 0;
            max_backdrop_index = 8;
        }
        else
        {
            backdrop_index = _backdropType;
            max_backdrop_index = _backdropType+1;
        }

        state.setTexCoordPointer( 0, 2, GL_FLOAT, 0, &(glyphquad._texcoords.front()));
        state.disableColorPointer();
        state.Color(_backdropColor.r(),_backdropColor.g(),_backdropColor.b(),_backdropColor.a());

        for( ; backdrop_index < max_backdrop_index; backdrop_index++)
        {
            const GlyphQuads::Coords3& transformedBackdropCoords = glyphquad._transformedBackdropCoords[backdrop_index][contextID];
            if (!transformedBackdropCoords.empty())
            {
                state.setVertexPointer( 3, GL_FLOAT, 0, &(transformedBackdropCoords.front()));
                double offset = double(max_backdrop_index-backdrop_index)*0.0001;
                glDepthRange( offset, 1.0+offset);

                state.drawQuads(0,transformedBackdropCoords.size());
            }
        }

        glDepthRange(0.0, 1.0);

        drawForegroundText(state, glyphquad, colorMultiplier);
    }

    glPopAttrib();
#else
    OSG_NOTICE<<"Warning: Text::renderWithDepthRange(..) not implemented."<<std::endl;
#endif
}

void Text::renderWithStencilBuffer(osg::State& state, const osg::Vec4& colorMultiplier) const
{
#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
    /* Here are the steps:
     * 1) Disable drawing color
     * 2) Enable the stencil buffer
     * 3) Draw all the text to the stencil buffer
     * 4) Disable the stencil buffer
     * 5) Enable color
     * 6) Disable the depth buffer
     * 7) Draw all the text again.
     * 7b) Make sure the foreground text is drawn last if priority levels
     * are the same OR
     * 7c) If priority levels are different, then make sure the foreground
     * text has the higher priority.
     */
    unsigned int contextID = state.getContextID();
    TextureGlyphQuadMap::iterator titr; // Moved up here for VC6

    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_STENCIL_TEST);

    // It seems I can get away without calling this here
    //glClear(GL_STENCIL_BUFFER_BIT);

    // enable stencil buffer
    glEnable(GL_STENCIL_TEST);

    // write a one to the stencil buffer everywhere we are about to draw
    glStencilFunc(GL_ALWAYS, 1, 1);

    // write only to the stencil buffer if we pass the depth test
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // Disable writing to the color buffer so we only write to the stencil
    // buffer and the depth buffer
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    // make sure the depth buffer is enabled
//    glEnable(GL_DEPTH_TEST);
//    glDepthMask(GL_TRUE);
//    glDepthFunc(GL_LESS);

    // Arrrgh! Why does the code only seem to work correctly if I call this?
    glDepthMask(GL_FALSE);


    // Draw all the text to the stencil buffer to mark out the region
    // that we can write too.

    for(titr=_textureGlyphQuadMap.begin();
        titr!=_textureGlyphQuadMap.end();
        ++titr)
    {
        // need to set the texture here...
        state.applyTextureAttribute(0,titr->first.get());

        const GlyphQuads& glyphquad = titr->second;

        unsigned int backdrop_index;
        unsigned int max_backdrop_index;
        if(_backdropType == OUTLINE)
        {
            backdrop_index = 0;
            max_backdrop_index = 8;
        }
        else
        {
            backdrop_index = _backdropType;
            max_backdrop_index = _backdropType+1;
        }

        state.setTexCoordPointer( 0, 2, GL_FLOAT, 0, &(glyphquad._texcoords.front()));
        state.disableColorPointer();

        for( ; backdrop_index < max_backdrop_index; backdrop_index++)
        {
            const GlyphQuads::Coords3& transformedBackdropCoords = glyphquad._transformedBackdropCoords[backdrop_index][contextID];
            if (!transformedBackdropCoords.empty())
            {
                state.setVertexPointer( 3, GL_FLOAT, 0, &(transformedBackdropCoords.front()));
                state.drawQuads(0,transformedBackdropCoords.size());
            }
        }

        // Draw the foreground text
        const GlyphQuads::Coords3& transformedCoords = glyphquad._transformedCoords[contextID];
        if (!transformedCoords.empty())
        {
            state.setVertexPointer( 3, GL_FLOAT, 0, &(transformedCoords.front()));
            state.setTexCoordPointer( 0, 2, GL_FLOAT, 0, &(glyphquad._texcoords.front()));
            state.drawQuads(0,transformedCoords.size());
        }
    }


    // disable the depth buffer
//    glDisable(GL_DEPTH_TEST);
//    glDepthMask(GL_FALSE);
//    glDepthMask(GL_TRUE);
//    glDepthFunc(GL_ALWAYS);

    // Set the stencil function to pass when the stencil is 1
    // Bug: This call seems to have no effect. Try changing to NOTEQUAL
    // and see the exact same results.
    glStencilFunc(GL_EQUAL, 1, 1);

    // disable writing to the stencil buffer
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(GL_FALSE);

    // Re-enable writing to the color buffer so we can see the results
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


    // Draw all the text again

    for(titr=_textureGlyphQuadMap.begin();
        titr!=_textureGlyphQuadMap.end();
        ++titr)
    {
        // need to set the texture here...
        state.applyTextureAttribute(0,titr->first.get());

        const GlyphQuads& glyphquad = titr->second;

        unsigned int backdrop_index;
        unsigned int max_backdrop_index;
        if(_backdropType == OUTLINE)
        {
            backdrop_index = 0;
            max_backdrop_index = 8;
        }
        else
        {
            backdrop_index = _backdropType;
            max_backdrop_index = _backdropType+1;
        }

        state.setTexCoordPointer( 0, 2, GL_FLOAT, 0, &(glyphquad._texcoords.front()));
        state.disableColorPointer();
        state.Color(_backdropColor.r(),_backdropColor.g(),_backdropColor.b(),_backdropColor.a());

        for( ; backdrop_index < max_backdrop_index; backdrop_index++)
        {
            const GlyphQuads::Coords3& transformedBackdropCoords = glyphquad._transformedBackdropCoords[backdrop_index][contextID];
            if (!transformedBackdropCoords.empty())
            {
                state.setVertexPointer( 3, GL_FLOAT, 0, &(transformedBackdropCoords.front()));
                state.drawQuads(0,transformedBackdropCoords.size());
            }
        }

        drawForegroundText(state, glyphquad, colorMultiplier);
    }

    glPopAttrib();
#else
    OSG_NOTICE<<"Warning: Text::renderWithStencilBuffer(..) not implemented."<<std::endl;
#endif
}
