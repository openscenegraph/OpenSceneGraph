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
#include <osgText/Text>

#include "DefaultFont.h"

using namespace osgText;

Text::Text():
    _fontWidth(32),
    _fontHeight(32),
    _characterHeight(32),
    _characterAspectRatio(1.0f),
    _alignment(BASE_LINE),
    _axisAlignment(XY_PLANE),
    _rotation(),
    _layout(LEFT_TO_RIGHT),
    _color(1.0f,1.0f,1.0f,1.0f),
    _drawMode(TEXT)
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
    _text(text._text),
    _position(text._position),
    _alignment(text._alignment),
    _axisAlignment(text._axisAlignment),
    _rotation(text._rotation),
    _layout(text._layout),
    _color(text._color),
    _drawMode(text._drawMode)
{
}

Text::~Text()
{
}

void Text::setFont(Font* font)
{
    _font = font;
    computeGlyphRepresentation();
}

void Text::setFont(const std::string& fontfile)
{
    setFont(readFontFile(fontfile));
}

void Text::setFontSize(unsigned int width, unsigned int height)
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


void Text::setText(const TextString& text)
{
    _text = text;
    computeGlyphRepresentation();
}

void Text::setText(const std::string& text)
{
    _text.clear();
    _text.insert(_text.end(),text.begin(),text.end());
    computeGlyphRepresentation();
}

void Text::setText(const std::string& text,Encoding encoding)
{
    _text.clear();

    if (text.empty()) return;
    
    
    std::cerr << "Text::setText(const std::string& text,Encoding encoding) not implemented yet."<<std::endl;

    //std::string::const_iterator itr = text.begin();

    if ((encoding == ENCODING_SIGNATURE) || 
        (encoding == ENCODING_UTF16) || 
        (encoding == ENCODING_UTF32))
    {
//        encoding = findEncoding(text,pos);
    }
    
    
}
    

void Text::setText(const wchar_t* text)
{
    _text.clear();
    if (text)
    {
        // find the end of wchar_t string
        const wchar_t* endOfText = text;
        while (*endOfText) ++endOfText;
        
        // pass it to the _text field.
        _text.insert(_text.end(),text,endOfText);
    }
    computeGlyphRepresentation();
}

void Text::setPosition(const osg::Vec3& pos)
{
    _position = pos;
    computePositions();
}

void Text::setAlignment(AlignmentType alignment)
{
    _alignment = alignment;
    computePositions();
}

void Text::setAxisAlignment(AxisAlignment axis)
{
    _axisAlignment = axis;
    computePositions();
}

void Text::setRotation(const osg::Quat& quat)
{
    _rotation = quat;
    computePositions();
}

void Text::setLayout(Layout layout)
{
    _layout = layout;
    computeGlyphRepresentation();
}

void Text::setColor(const osg::Vec4& color)
{
    _color = color;
}

bool Text::computeBound() const
{
    _bbox.init();

    if (_textBB.valid())
    {
        if (_axisAlignment==SCREEN)
        {
            // build a sphere around the text box, centerd at the offset point.
            float maxlength2 = (_textBB.corner(0)-_offset).length2();
            
            float length2 = (_textBB.corner(1)-_offset).length2();
            maxlength2 = osg::maximum(maxlength2,length2);
            
            length2 = (_textBB.corner(2)-_offset).length2();
            maxlength2 = osg::maximum(maxlength2,length2);

            length2 = (_textBB.corner(3)-_offset).length2();
            maxlength2 = osg::maximum(maxlength2,length2);

            float radius = sqrtf(maxlength2);
            osg::Vec3 center(_position);
            
            _bbox.set(center.x()-radius,center.y()-radius,center.z()-radius,
                      center.x()+radius,center.y()+radius,center.z()+radius);
        }
        else
        {
            _bbox.expandBy(osg::Vec3(_textBB.xMin(),_textBB.yMin(),_textBB.zMin())*_matrix);
            _bbox.expandBy(osg::Vec3(_textBB.xMax(),_textBB.yMin(),_textBB.zMin())*_matrix);
            _bbox.expandBy(osg::Vec3(_textBB.xMax(),_textBB.yMax(),_textBB.zMin())*_matrix);
            _bbox.expandBy(osg::Vec3(_textBB.xMin(),_textBB.yMax(),_textBB.zMin())*_matrix);
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

void Text::computeGlyphRepresentation()
{
    Font* activefont = getActiveFont();
    if (!activefont) return;
    
    _textureGlyphQuadMap.clear();
    
    osg::Vec2 startOfLine(0.0f,0.0f);
    osg::Vec2 cursor(startOfLine);
    osg::Vec2 local(0.0f,0.0f);
    
    unsigned int previous_charcode = 0;
    bool horizontal = _layout!=VERTICAL;
    bool kerning = true;

    activefont->setSize(_fontWidth,_fontHeight);
    
    float hr = _characterHeight/(float)activefont->getHeight();
    float wr = hr/_characterAspectRatio;

    for(TextString::iterator itr=_text.begin();
        itr!=_text.end();
        ++itr)
    {
        unsigned int charcode = *itr;
        
        if (charcode=='\n')
        {
            if (horizontal) startOfLine.y() -= _fontHeight;
            else startOfLine.x() += _fontWidth;
            cursor = startOfLine;
            previous_charcode = 0;
            continue;
        }
        
        
        Font::Glyph* glyph = activefont->getGlyph(charcode);
        if (glyph)
        {

            float width = (float)glyph->s() * wr;
            float height = (float)glyph->t() * hr;

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
                    osg::Vec2 delta(activefont->getKerning(previous_charcode,charcode));
                    cursor.x() += delta.x() * wr;
                    cursor.y() += delta.y() * hr;
                    break;
                  }
                  case RIGHT_TO_LEFT:
                  {
                    osg::Vec2 delta(activefont->getKerning(charcode,previous_charcode));
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
        
        for(GlyphQuads::Coords::const_iterator citr = glyphquad._coords.begin();
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

void Text::computePositions()
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
    case BASE_LINE:     _offset.set(0.0f,0.0f,0.0f); break;
    }

    _matrix.makeTranslate(_position-_offset);

    switch(_axisAlignment)
    {
    case XZ_PLANE:  _matrix.preMult(osg::Matrix::rotate(osg::inDegrees(90.0f),1.0f,0.0f,0.0f)); break;
    case YZ_PLANE:  _matrix.preMult(osg::Matrix::rotate(osg::inDegrees(90.0f),1.0f,0.0f,0.0f)*osg::Matrix::rotate(osg::inDegrees(90.0f),0.0f,0.0f,1.0f)); break;
    case XY_PLANE:  break; // nop - already on XY plane.
    case SCREEN:    break; // nop - need to account for rotation in draw as it depends on ModelView _matrix.
    }

    if (_axisAlignment!=SCREEN && !_rotation.zeroRotation())
    {
        osg::Matrix matrix;
        _rotation.get(matrix);
        _matrix.preMult(matrix);
    }

    dirtyBound();    
}


void Text::drawImplementation(osg::State& state) const
{
    

    
    glPushMatrix();

    // draw part.
    glMultMatrixf(_matrix.ptr());


    if (_axisAlignment==SCREEN)
    {
        osg::Matrix mv = state.getModelViewMatrix();
        mv.setTrans(0.0f,0.0f,0.0f);
        osg::Matrix mat3x3;
        mat3x3.invert(mv);
        glMultMatrixf(mat3x3.ptr());

        if (!_rotation.zeroRotation())
        {
            osg::Matrix matrix;
            _rotation.get(matrix);
            glMultMatrixf(matrix.ptr());
        }

    }      
    
    glNormal3f(0.0f,0.0,1.0f);
    glColor4fv(_color.ptr());

    if (_drawMode & TEXT)
    {

        bool first = true;

        for(TextureGlyphQuadMap::const_iterator titr=_textureGlyphQuadMap.begin();
            titr!=_textureGlyphQuadMap.end();
            ++titr)
        {
            // need to set the texture here...

            if (!first)
            {
                state.apply(titr->first.get());
            }

            const GlyphQuads& glyphquad = titr->second;

            state.setVertexPointer( 2, GL_FLOAT, 0, &(glyphquad._coords.front()));
            state.setTexCoordPointer( 0, 2, GL_FLOAT, 0, &(glyphquad._texcoords.front()));

            glDrawArrays(GL_QUADS,0,glyphquad._coords.size());

            first = false;

        }
    }
    
    if (_drawMode & BOUNDINGBOX)
    {
    
        if (_textBB.valid())
        {
            state.applyTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::OFF);
        
            glColor4f(1.0f,1.0f,0.0f,1.0f);
            glBegin(GL_LINE_LOOP);
                glVertex3f(_textBB.xMin(),_textBB.yMin(),_textBB.zMin());
                glVertex3f(_textBB.xMax(),_textBB.yMin(),_textBB.zMin());
                glVertex3f(_textBB.xMax(),_textBB.yMax(),_textBB.zMin());
                glVertex3f(_textBB.xMin(),_textBB.yMax(),_textBB.zMin());
            glEnd();
        }
    }    

    if (_drawMode & ALIGNMENT)
    {
        glColor4f(1.0f,0.0f,1.0f,1.0f);
        glTranslatef(_offset.x(),_offset.y(),_offset.z());
        
        state.applyTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::OFF);
        
        float cursorsize = _characterHeight*0.5f;
        
        glBegin(GL_LINES);
            glVertex3f(-cursorsize,0.0f,0.0f);
            glVertex3f(cursorsize,0.0f,0.0f);
            glVertex3f(0.0f,-cursorsize,0.0f);
            glVertex3f(0.0f,cursorsize,0.0f);
        glEnd();
        
    }    


    glPopMatrix();
}

