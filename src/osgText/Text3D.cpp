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

#include <osgText/Text3D>
#include <osg/Vec3d>
#include <osg/io_utils>

namespace osgText
{

Text3D::Text3D():
    _renderMode(PER_GLYPH)
{
}

Text3D::Text3D(const Text3D & text3D, const osg::CopyOp & copyop):
    osgText::TextBase(text3D, copyop),
    _renderMode(text3D._renderMode)
{
    computeGlyphRepresentation();
}

float Text3D::getCharacterDepth() const
{
    if (!_style) return _characterHeight*0.1f;
    else return _characterHeight * _style->getThicknessRatio();
}

void Text3D::setCharacterDepth(float characterDepth)
{
    getOrCreateStyle()->setThicknessRatio(characterDepth / _characterHeight);

    computeGlyphRepresentation();
}

void Text3D::accept(osg::Drawable::ConstAttributeFunctor& af) const
{
    // ** for each line, do ...
    TextRenderInfo::const_iterator itLine, endLine = _textRenderInfo.end();
    for (itLine = _textRenderInfo.begin(); itLine!=endLine; ++itLine)
    {
        // ** for each glyph in the line, do ...
        LineRenderInfo::const_iterator it, end = itLine->end();
        for (it = itLine->begin(); it!=end; ++it)
        {
            // ** apply the vertex array
            af.apply(osg::Drawable::VERTICES, it->_glyphGeometry->getVertexArray()->size(), &(it->_glyphGeometry->getVertexArray()->front()));
        }
    }
}
void Text3D::accept(osg::PrimitiveFunctor& pf) const
{
    // ** for each line, do ...
    TextRenderInfo::const_iterator itLine, endLine = _textRenderInfo.end();
    for (itLine = _textRenderInfo.begin(); itLine!=endLine; ++itLine)
    {
        // ** for each glyph in the line, do ...
        LineRenderInfo::const_iterator it, end = itLine->end();
        for (it = itLine->begin(); it!=end; ++it)
        {
            osg::Vec3Array* vertices = it->_glyphGeometry->getVertexArray();

            if (!vertices || vertices->empty())
              continue; //skip over spaces

            //pf.setVertexArray(it->_glyph->getVertexArray()->size(),&(it->_glyph->getVertexArray()->front()));
            //////////////////////////////////////////////////////////////////////////
            // now apply matrix to the glyphs.
            osg::ref_ptr<osg::Vec3Array> transformedVertices = new osg::Vec3Array;
            osg::Matrix matrix = _autoTransformCache[0]._matrix;//osg::Matrix();
            matrix.preMultTranslate(it->_position);
            transformedVertices->reserve(vertices->size());
            for (osg::Vec3Array::iterator itr=vertices->begin(); itr!=vertices->end(); itr++)
            {
              transformedVertices->push_back((*itr)*matrix);
            }
            //////////////////////////////////////////////////////////////////////////
            pf.setVertexArray(transformedVertices->size(),&(transformedVertices->front()));

            // ** render the front face of the glyph
            osg::Geometry::PrimitiveSetList & pslFront = it->_glyphGeometry->getFrontPrimitiveSetList();
            for(osg::Geometry::PrimitiveSetList::const_iterator itr=pslFront.begin(), end = pslFront.end(); itr!=end; ++itr)
            {
                (*itr)->accept(pf);
            }

            // ** render the wall face of the glyph
            osg::Geometry::PrimitiveSetList & pslWall = it->_glyphGeometry->getWallPrimitiveSetList();
            for(osg::Geometry::PrimitiveSetList::const_iterator itr=pslWall.begin(), end=pslWall.end(); itr!=end; ++itr)
            {
                (*itr)->accept(pf);
            }

            // ** render the back face of the glyph
            osg::Geometry::PrimitiveSetList & pslBack = it->_glyphGeometry->getBackPrimitiveSetList();
            for(osg::Geometry::PrimitiveSetList::const_iterator itr=pslBack.begin(), end=pslBack.end(); itr!=end; ++itr)
            {
                (*itr)->accept(pf);
            }
        }
    }
}

String::iterator Text3D::computeLastCharacterOnLine(osg::Vec2& cursor, String::iterator first,String::iterator last)
{
    if (_font.valid() == false) return last;

    bool kerning = true;
    unsigned int previous_charcode = 0;

    String::iterator lastChar = first;

    float maximumHeight = _maximumHeight;
    float maximumWidth = _maximumWidth;

    float hr = 1.0f;
    float wr = 1.0f;

    for(bool outOfSpace=false;lastChar!=last;++lastChar)
    {
        unsigned int charcode = *lastChar;

        if (charcode=='\n')
        {
            return lastChar;
        }

        Glyph3D* glyph = _font->getGlyph3D(charcode);
        if (glyph)
        {
            const osg::BoundingBox & bb = glyph->getBoundingBox();

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
                    osg::Vec2 delta(_font->getKerning(previous_charcode,charcode,_kerningType));
                    cursor.x() += delta.x() * wr;
                    cursor.y() += delta.y() * hr;
                    break;
                  }
                  case RIGHT_TO_LEFT:
                  {
                    osg::Vec2 delta(_font->getKerning(charcode,previous_charcode,_kerningType));
                    cursor.x() -= delta.x() * wr;
                    cursor.y() -= delta.y() * hr;
                    break;
                  }
                  case VERTICAL:
                    break; // no kerning when vertical.
                }
            }

            osg::Vec2 local = cursor;

            switch(_layout)
            {
              case LEFT_TO_RIGHT:
              {
                if (maximumWidth>0.0f && local.x()+bb.xMax()>maximumWidth) outOfSpace=true;
                if(maximumHeight>0.0f && local.y()<-maximumHeight) outOfSpace=true;
                break;
              }
              case RIGHT_TO_LEFT:
              {
                if (maximumWidth>0.0f && local.x()+bb.xMin()<-maximumWidth) outOfSpace=true;
                if(maximumHeight>0.0f && local.y()<-maximumHeight) outOfSpace=true;
                break;
              }
              case VERTICAL:
                if (maximumHeight>0.0f && local.y()<-maximumHeight) outOfSpace=true;
                break;
            }

            // => word boundary detection & wrapping
            if (outOfSpace) break;

            // move the cursor onto the next character.
            switch(_layout)
            {
              case LEFT_TO_RIGHT: cursor.x() += glyph->getHorizontalAdvance() * wr; break;
              case RIGHT_TO_LEFT: break;
              case VERTICAL:      cursor.y() -= glyph->getVerticalAdvance() * hr; break;
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
                Glyph3D* glyph = _font->getGlyph3D(*prevChar);
                if (glyph)
                {
                    switch(_layout)
                    {
                    case LEFT_TO_RIGHT: cursor.x() -= glyph->getHorizontalAdvance() * wr; break;
                    case RIGHT_TO_LEFT: cursor.x() += glyph->getHorizontalAdvance() * wr; break;
                    case VERTICAL:      cursor.y() += glyph->getVerticalAdvance() * wr; break;
                    }
                }
            }

            lastValidChar = prevChar;
          }
    }

    return lastChar;
}

void Text3D::computeGlyphRepresentation()
{
    if (_font.valid() == false) return;

    _textRenderInfo.clear();
    _lineCount = 0;

    if (_text.empty())
    {
        _textBB.set(0,0,0, 0,0,0);//no size text
        TextBase::computePositions(); //to reset the origin
        return;
    }

    // initialize bounding box, it will be expanded during glyph position calculation
    _textBB.init();


    float hr = 1.0f;
    float wr = 1.0f;

    osg::Vec2 startOfLine_coords(0.0f,0.0f);
    osg::Vec2 cursor(startOfLine_coords);
    osg::Vec2 local(0.0f,0.0f);
    osg::Vec2 startOffset(0.0f,0.0f);

    unsigned int previous_charcode = 0;
    unsigned int linelength = 0;
    bool kerning = true;

    unsigned int lineNumber = 0;

    for(String::iterator itr=_text.begin(); itr!=_text.end(); )
    {
        _textRenderInfo.resize(lineNumber + 1);
        LineRenderInfo & currentLineRenderInfo = _textRenderInfo.back();

        // record the start of the current line
        String::iterator startOfLine_itr = itr;

        // find the end of the current line.
        osg::Vec2 endOfLine_coords(cursor);
        String::iterator endOfLine_itr = computeLastCharacterOnLine(endOfLine_coords, itr,_text.end());

        // ** position the cursor function to the Layout and the alignement
        TextBase::positionCursor(endOfLine_coords, cursor, (unsigned int) (endOfLine_itr - startOfLine_itr));


        if (itr!=endOfLine_itr)
        {
            for(;itr!=endOfLine_itr;++itr)
            {
                unsigned int charcode = *itr;

                Glyph3D* glyph = _font->getGlyph3D(charcode);
                if (glyph)
                {
                    const osg::BoundingBox & bb = glyph->getBoundingBox();

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
                            osg::Vec2 delta(_font->getKerning(previous_charcode,charcode,_kerningType));
                            cursor.x() += delta.x() * wr;
                            cursor.y() += delta.y() * hr;
                            break;
                          }
                          case RIGHT_TO_LEFT:
                          {
                            osg::Vec2 delta(_font->getKerning(charcode,previous_charcode,_kerningType));
                            cursor.x() -= delta.x() * wr;
                            cursor.y() -= delta.y() * hr;
                            break;
                          }
                          case VERTICAL:
                            break; // no kerning when vertical.
                        }
                    }

                    local = cursor;

                    // move the cursor onto the next character.
                    // also expand bounding box
                    switch (_layout)
                    {
                        case LEFT_TO_RIGHT:
                            _textBB.expandBy(osg::Vec3(cursor.x() + bb.xMin()*wr, cursor.y() + bb.yMin()*hr, 0.0f)); //lower left corner
                            _textBB.expandBy(osg::Vec3(cursor.x() + bb.xMax()*wr, cursor.y() + bb.yMax()*hr, 0.0f)); //upper right corner
                            cursor.x() += glyph->getHorizontalAdvance() * wr;
                            break;
                        case VERTICAL:
                            _textBB.expandBy(osg::Vec3(cursor.x(), cursor.y(), 0.0f)); //upper left corner
                            _textBB.expandBy(osg::Vec3(cursor.x() + glyph->getWidth()*wr, cursor.y() - glyph->getHeight()*hr, 0.0f)); //lower right corner
                            cursor.y() -= glyph->getVerticalAdvance() * hr;
                            break;
                        case RIGHT_TO_LEFT:
                            _textBB.expandBy(osg::Vec3(cursor.x()+bb.xMax()*wr, cursor.y() + bb.yMax()*hr, 0.0f)); //upper right corner
                            _textBB.expandBy(osg::Vec3(cursor.x()+bb.xMin()*wr, cursor.y()+bb.yMin()*hr, 0.0f)); //lower left corner

                            break;
                    }

                    osg::Vec3 pos = osg::Vec3(local.x(), local.y(), 0.0f);
                    GlyphGeometry* glyphGeometry = glyph->getGlyphGeometry(_style.get());
                    currentLineRenderInfo.push_back(Text3D::GlyphRenderInfo(glyphGeometry, pos));

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
            // skip over spaces and return.
            while (*itr==' ') ++itr;
            if (*itr=='\n') ++itr;
        }

        // move to new line.
        switch(_layout)
        {
            case LEFT_TO_RIGHT:
            case RIGHT_TO_LEFT:
            {
                startOfLine_coords.y() -= (1.0 + _lineSpacing) * hr;
                ++_lineCount;
                break;
            }
            case VERTICAL:
            {
                startOfLine_coords.x() += _characterHeight * (1.0 + _lineSpacing) * wr;
                // because _lineCount is the max vertical no. of characters....
                _lineCount = (_lineCount >linelength)?_lineCount:linelength;
                break;
            }
            default:
                break;
        }

        cursor = startOfLine_coords;
        previous_charcode = 0;

        ++lineNumber;
    }

    float thickness = _style.valid() ? _style->getThicknessRatio() : 0.1f;
    _textBB.zMin() = -thickness;

    TextBase::computePositions();
}

osg::BoundingBox Text3D::computeBound() const
{
    osg::BoundingBox  bbox;

    if (_textBB.valid())
    {
        for(unsigned int i=0;i<_autoTransformCache.size();++i)
        {
            osg::Matrix& matrix = _autoTransformCache[i]._matrix;
            bbox.expandBy(_textBB.corner(0)*matrix);
            bbox.expandBy(_textBB.corner(1)*matrix);
            bbox.expandBy(_textBB.corner(2)*matrix);
            bbox.expandBy(_textBB.corner(3)*matrix);
            bbox.expandBy(_textBB.corner(4)*matrix);
            bbox.expandBy(_textBB.corner(5)*matrix);
            bbox.expandBy(_textBB.corner(6)*matrix);
            bbox.expandBy(_textBB.corner(7)*matrix);
        }
    }

    return bbox;
}

void Text3D::computePositions(unsigned int contextID) const
{
    if (_font.valid() == false) return;

    switch(_alignment)
    {
    case LEFT_TOP:      _offset.set(_textBB.xMin(),_textBB.yMax(),0.0f); break;
    case LEFT_CENTER:   _offset.set(_textBB.xMin(),(_textBB.yMax()+_textBB.yMin())*0.5f,0.0f); break;
    case LEFT_BOTTOM:   _offset.set(_textBB.xMin(),_textBB.yMin(),0.0f); break;

    case CENTER_TOP:    _offset.set((_textBB.xMax()+_textBB.xMin())*0.5f,_textBB.yMax(),0.0f); break;
    case CENTER_CENTER: _offset.set((_textBB.xMax()+_textBB.xMin())*0.5f,(_textBB.yMax()+_textBB.yMin())*0.5f,0.0f); break;
    case CENTER_BOTTOM: _offset.set((_textBB.xMax()+_textBB.xMin())*0.5f,_textBB.yMin(),0.0f); break;

    case RIGHT_TOP:     _offset.set(_textBB.xMax(),_textBB.yMax(),0.0f); break;
    case RIGHT_CENTER:  _offset.set(_textBB.xMax(),(_textBB.yMax()+_textBB.yMin())*0.5f,0.0f); break;
    case RIGHT_BOTTOM:  _offset.set(_textBB.xMax(),_textBB.yMin(),0.0f); break;

    case LEFT_BASE_LINE:  _offset.set(0.0f,0.0f,0.0f); break;
    case CENTER_BASE_LINE:  _offset.set((_textBB.xMax()+_textBB.xMin())*0.5f,0.0f,0.0f); break;
    case RIGHT_BASE_LINE:  _offset.set(_textBB.xMax(),0.0f,0.0f); break;

    case LEFT_BOTTOM_BASE_LINE:  _offset.set(0.0f,-_characterHeight*(1.0 + _lineSpacing)*(_lineCount-1),0.0f); break;
    case CENTER_BOTTOM_BASE_LINE:  _offset.set((_textBB.xMax()+_textBB.xMin())*0.5f,-_characterHeight*(1.0 + _lineSpacing)*(_lineCount-1),0.0f); break;
    case RIGHT_BOTTOM_BASE_LINE:  _offset.set(_textBB.xMax(),-_characterHeight*(1.0 + _lineSpacing)*(_lineCount-1),0.0f); break;
    }

    AutoTransformCache& atc = _autoTransformCache[contextID];
    osg::Matrix& matrix = atc._matrix;


    osg::Vec3 scaleVec(_characterHeight / getCharacterAspectRatio(), _characterHeight , _characterHeight);

    matrix.makeTranslate(-_offset);
    matrix.postMultScale(scaleVec);
    matrix.postMultRotate(_rotation);
    matrix.postMultTranslate(_position);


    _normal = osg::Matrix::transform3x3(osg::Vec3(0.0f,0.0f,1.0f),matrix);
    _normal.normalize();

    const_cast<Text3D*>(this)->dirtyBound();
}

void Text3D::drawImplementation(osg::RenderInfo& renderInfo) const
{
    osg::State & state = *renderInfo.getState();
    unsigned int contextID = state.getContextID();

    // ** save the previous modelview matrix
    osg::Matrix previous(state.getModelViewMatrix());

    // ** get the modelview for this context
    osg::Matrix modelview(_autoTransformCache[contextID]._matrix);

    // ** mult previous by the modelview for this context
    modelview.postMult(previous);

    // ** apply this new modelview matrix
    state.applyModelViewMatrix(modelview);

    osg::GLBeginEndAdapter& gl = (state.getGLBeginEndAdapter());

    if (_drawMode & BOUNDINGBOX)
    {
        if (_textBB.valid())
        {
            gl.Color4fv(_color.ptr());

            osg::Vec3 c000(osg::Vec3(_textBB.xMin(),_textBB.yMin(),_textBB.zMax()));
            osg::Vec3 c100(osg::Vec3(_textBB.xMax(),_textBB.yMin(),_textBB.zMax()));
            osg::Vec3 c110(osg::Vec3(_textBB.xMax(),_textBB.yMax(),_textBB.zMax()));
            osg::Vec3 c010(osg::Vec3(_textBB.xMin(),_textBB.yMax(),_textBB.zMax()));

            osg::Vec3 c001(osg::Vec3(_textBB.xMin(),_textBB.yMin(),_textBB.zMin()));
            osg::Vec3 c101(osg::Vec3(_textBB.xMax(),_textBB.yMin(),_textBB.zMin()));
            osg::Vec3 c111(osg::Vec3(_textBB.xMax(),_textBB.yMax(),_textBB.zMin()));
            osg::Vec3 c011(osg::Vec3(_textBB.xMin(),_textBB.yMax(),_textBB.zMin()));

            gl.Begin(GL_LINE_LOOP);
                gl.Vertex3fv(c000.ptr());
                gl.Vertex3fv(c100.ptr());
                gl.Vertex3fv(c110.ptr());
                gl.Vertex3fv(c010.ptr());
            gl.End();

            gl.Begin(GL_LINE_LOOP);
                gl.Vertex3fv(c001.ptr());
                gl.Vertex3fv(c011.ptr());
                gl.Vertex3fv(c111.ptr());
                gl.Vertex3fv(c101.ptr());
            gl.End();

            gl.Begin(GL_LINES);
                gl.Vertex3fv(c000.ptr());
                gl.Vertex3fv(c001.ptr());

                gl.Vertex3fv(c100.ptr());
                gl.Vertex3fv(c101.ptr());

                gl.Vertex3fv(c110.ptr());
                gl.Vertex3fv(c111.ptr());

                gl.Vertex3fv(c010.ptr());
                gl.Vertex3fv(c011.ptr());
            gl.End();
        }
    }

    if (_drawMode & ALIGNMENT)
    {
        float cursorsize = _characterHeight*0.5f;

        osg::Vec3 hl(osg::Vec3(_offset.x()-cursorsize,_offset.y(),_offset.z()));
        osg::Vec3 hr(osg::Vec3(_offset.x()+cursorsize,_offset.y(),_offset.z()));
        osg::Vec3 vt(osg::Vec3(_offset.x(),_offset.y()-cursorsize,_offset.z()));
        osg::Vec3 vb(osg::Vec3(_offset.x(),_offset.y()+cursorsize,_offset.z()));

        gl.Color4fv(_color.ptr());

        gl.Begin(GL_LINES);
            gl.Vertex3fv(hl.ptr());
            gl.Vertex3fv(hr.ptr());
            gl.Vertex3fv(vt.ptr());
            gl.Vertex3fv(vb.ptr());
        gl.End();

    }

    if (_drawMode & TEXT)
    {
        state.disableColorPointer();
        state.Color(_color.r(),_color.g(),_color.b(),_color.a());

        renderInfo.getState()->disableAllVertexArrays();

        #if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
            renderInfo.getState()->applyMode(GL_NORMALIZE, true);
        #endif

        switch(_renderMode)
        {
            case PER_FACE:  renderPerFace(*renderInfo.getState());   break;
            case PER_GLYPH:
            default:        renderPerGlyph(*renderInfo.getState());  break;
        }
    }


    // restore the previous modelview matrix
    state.applyModelViewMatrix(previous);
}

void Text3D::renderPerGlyph(osg::State & state) const
{
    osg::Matrix original_modelview = state.getModelViewMatrix();

    const osg::StateSet* frontStateSet = getStateSet();
    const osg::StateSet* wallStateSet = getWallStateSet();
    const osg::StateSet* backStateSet = getBackStateSet();
    bool applyMainColor = false;

    if (wallStateSet==0) wallStateSet = frontStateSet;
    else if (wallStateSet->getAttribute(osg::StateAttribute::MATERIAL)!=0) applyMainColor = true;

    if (backStateSet==0) backStateSet = frontStateSet;
    else if (backStateSet->getAttribute(osg::StateAttribute::MATERIAL)!=0) applyMainColor = true;

    // ** for each line, do ...
    TextRenderInfo::const_iterator itLine, endLine = _textRenderInfo.end();
    for (itLine = _textRenderInfo.begin(); itLine!=endLine; ++itLine)
    {
        // ** for each glyph in the line, do ...
        LineRenderInfo::const_iterator it, end = itLine->end();
        for (it = itLine->begin(); it!=end; ++it)
        {

            osg::Matrix matrix(original_modelview);
            matrix.preMultTranslate(osg::Vec3d(it->_position.x(), it->_position.y(), it->_position.z()));
            state.applyModelViewMatrix(matrix);

            state.lazyDisablingOfVertexAttributes();

            // ** apply the vertex array
            state.setVertexPointer(it->_glyphGeometry->getVertexArray());
            state.setNormalPointer(it->_glyphGeometry->getNormalArray());

            state.applyDisablingOfVertexAttributes();

            if (frontStateSet!=backStateSet)
            {
                state.apply(frontStateSet);
                if (applyMainColor) state.Color(_color.r(),_color.g(),_color.b(),_color.a());
            }

            osg::Geometry::PrimitiveSetList & pslFront = it->_glyphGeometry->getFrontPrimitiveSetList();
            for(osg::Geometry::PrimitiveSetList::const_iterator itr=pslFront.begin(), end = pslFront.end(); itr!=end; ++itr)
            {
                (*itr)->draw(state, false);
            }

            if (wallStateSet!=frontStateSet) state.apply(wallStateSet);

            // ** render the wall face of the glyph
            osg::Geometry::PrimitiveSetList & pslWall = it->_glyphGeometry->getWallPrimitiveSetList();
            for(osg::Geometry::PrimitiveSetList::const_iterator itr=pslWall.begin(), end=pslWall.end(); itr!=end; ++itr)
            {
                (*itr)->draw(state, false);
            }

            if (backStateSet!=wallStateSet)
            {
                state.apply(backStateSet);
                if (applyMainColor) state.Color(_color.r(),_color.g(),_color.b(),_color.a());
            }

            osg::Geometry::PrimitiveSetList & pslBack = it->_glyphGeometry->getBackPrimitiveSetList();
            for(osg::Geometry::PrimitiveSetList::const_iterator itr=pslBack.begin(), end=pslBack.end(); itr!=end; ++itr)
            {
                (*itr)->draw(state, false);
            }
        }
    }
}

void Text3D::renderPerFace(osg::State & state) const
{
    osg::Matrix original_modelview = state.getModelViewMatrix();
#if 0
    // ** render all front faces
    state.Normal(0.0f,0.0f,1.0f);
#endif

    const osg::StateSet* frontStateSet = getStateSet();
    const osg::StateSet* wallStateSet = getWallStateSet();
    const osg::StateSet* backStateSet = getBackStateSet();
    bool applyMainColor = false;

    if (wallStateSet==0) wallStateSet = frontStateSet;
    else if (wallStateSet->getAttribute(osg::StateAttribute::MATERIAL)!=0) applyMainColor = true;

    if (backStateSet==0) backStateSet = frontStateSet;
    else if (backStateSet->getAttribute(osg::StateAttribute::MATERIAL)!=0) applyMainColor = true;


    TextRenderInfo::const_iterator itLine, endLine = _textRenderInfo.end();
    for (itLine = _textRenderInfo.begin(); itLine!=endLine; ++itLine)
    {
        // ** for each glyph in the line, do ...
        LineRenderInfo::const_iterator it, end = itLine->end();
        for (it = itLine->begin(); it!=end; ++it)
        {
            osg::Matrix matrix(original_modelview);
            matrix.preMultTranslate(osg::Vec3d(it->_position.x(), it->_position.y(), it->_position.z()));
            state.applyModelViewMatrix(matrix);

            state.setVertexPointer(it->_glyphGeometry->getVertexArray());
            state.setNormalPointer(it->_glyphGeometry->getNormalArray());

            // ** render the front face of the glyph
            osg::Geometry::PrimitiveSetList & psl = it->_glyphGeometry->getFrontPrimitiveSetList();
            for(osg::Geometry::PrimitiveSetList::const_iterator itr=psl.begin(), end=psl.end(); itr!=end; ++itr)
            {
                (*itr)->draw(state, false);
            }
        }
    }

    if (wallStateSet!=frontStateSet) state.apply(wallStateSet);

    // ** render all wall face of the text
    for (itLine = _textRenderInfo.begin(); itLine!=endLine; ++itLine)
    {
        // ** for each glyph in the line, do ...
        LineRenderInfo::const_iterator it, end = itLine->end();
        for (it = itLine->begin(); it!=end; ++it)
        {
            osg::Matrix matrix(original_modelview);
            matrix.preMultTranslate(osg::Vec3d(it->_position.x(), it->_position.y(), it->_position.z()));
            state.applyModelViewMatrix(matrix);

            state.setVertexPointer(it->_glyphGeometry->getVertexArray());
            state.setNormalPointer(it->_glyphGeometry->getNormalArray());

            const osg::Geometry::PrimitiveSetList & psl = it->_glyphGeometry->getWallPrimitiveSetList();
            for(osg::Geometry::PrimitiveSetList::const_iterator itr=psl.begin(), end=psl.end(); itr!=end; ++itr)
            {
                (*itr)->draw(state, false);
            }
        }
    }
#if 0
    state.disableNormalPointer();

    // ** render all back face of the text
    state.Normal(0.0f,0.0f,-1.0f);
#endif

    if (backStateSet!=wallStateSet)
    {
        state.apply(backStateSet);
        if (applyMainColor) state.Color(_color.r(),_color.g(),_color.b(),_color.a());
    }

    for (itLine = _textRenderInfo.begin(); itLine!=endLine; ++itLine)
    {
        // ** for each glyph in the line, do ...
        LineRenderInfo::const_iterator it, end = itLine->end();
        for (it = itLine->begin(); it!=end; ++it)
        {
            osg::Matrix matrix(original_modelview);
            matrix.preMultTranslate(osg::Vec3d(it->_position.x(), it->_position.y(), it->_position.z()));
            state.applyModelViewMatrix(matrix);

            state.setVertexPointer(it->_glyphGeometry->getVertexArray());
            state.setNormalPointer(it->_glyphGeometry->getNormalArray());

            // ** render the back face of the glyph
            const osg::Geometry::PrimitiveSetList & psl = it->_glyphGeometry->getBackPrimitiveSetList();
            for(osg::Geometry::PrimitiveSetList::const_iterator itr=psl.begin(), end=psl.end(); itr!=end; ++itr)
            {
                (*itr)->draw(state, false);
            }
        }
    }
}



void Text3D::setThreadSafeRefUnref(bool threadSafe)
{
    TextBase::setThreadSafeRefUnref(threadSafe);

    if (_font.valid()) _font->setThreadSafeRefUnref(threadSafe);
}

void Text3D::resizeGLObjectBuffers(unsigned int maxSize)
{
    OSG_INFO<<"Text3D::resizeGLObjectBuffers("<<maxSize<<")"<<std::endl;

    TextBase::resizeGLObjectBuffers(maxSize);

    if (_font.valid()) _font->resizeGLObjectBuffers(maxSize);

    TextBase::computePositions();
}

void Text3D::releaseGLObjects(osg::State* state) const
{
    TextBase::releaseGLObjects(state);

    if (_font.valid()) _font->releaseGLObjects(state);
}

}
