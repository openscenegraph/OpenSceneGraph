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
    _glyphNormalized = true;
}

Text3D::Text3D(const Text3D & text3D, const osg::CopyOp & copyop):
    osgText::TextBase(text3D, copyop),
    _renderMode(text3D._renderMode)
{
    _glyphNormalized = text3D._glyphNormalized;
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
    if (!_coords || _coords->empty()) return;

    af.apply(osg::Drawable::VERTICES, _coords->size(), &(_coords->front()));
}

void Text3D::accept(osg::PrimitiveFunctor& pf) const
{
    if (!_coords || _coords->empty()) return;

    // short term fix/workaround for _coords being transformed by a local matrix before rendering, so we need to replicate this was doing tasks like intersection testing.
    osg::ref_ptr<osg::Vec3Array> vertices = _coords;
    if (!_matrix.isIdentity())
    {
        vertices = new osg::Vec3Array;
        vertices->resize(_coords->size());
        for(osg::Vec3Array::iterator sitr = _coords->begin(), ditr = vertices->begin();
            sitr != _coords->end();
            ++sitr, ++ditr)
        {
            *ditr = *sitr * _matrix;
        }
    }

    pf.setVertexArray(vertices->size(), &(vertices->front()));

    for(osg::Geometry::PrimitiveSetList::const_iterator itr = _frontPrimitiveSetList.begin();
        itr != _frontPrimitiveSetList.end();
        ++itr)
    {
        (*itr)->accept(pf);
    }

    for(osg::Geometry::PrimitiveSetList::const_iterator itr = _wallPrimitiveSetList.begin();
        itr != _wallPrimitiveSetList.end();
        ++itr)
    {
        (*itr)->accept(pf);
    }

    for(osg::Geometry::PrimitiveSetList::const_iterator itr = _backPrimitiveSetList.begin();
        itr != _backPrimitiveSetList.end();
        ++itr)
    {
        (*itr)->accept(pf);
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

        Glyph3D* glyph = _font->getGlyph3D(_fontSize, charcode);
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
                    osg::Vec2 delta(_font->getKerning(_fontSize, previous_charcode, charcode, _kerningType));
                    cursor.x() += delta.x() * wr;
                    cursor.y() += delta.y() * hr;
                    break;
                  }
                  case RIGHT_TO_LEFT:
                  {
                    osg::Vec2 delta(_font->getKerning(_fontSize, charcode, previous_charcode, _kerningType));
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
                Glyph3D* glyph = _font->getGlyph3D(_fontSize, *prevChar);
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

void Text3D::copyAndOffsetPrimitiveSets(osg::Geometry::PrimitiveSetList& dest_PrimitiveSetList, osg::Geometry::PrimitiveSetList& src_PrimitiveSetList, unsigned int offset)
{
    for(osg::Geometry::PrimitiveSetList::const_iterator pitr = src_PrimitiveSetList.begin();
        pitr != src_PrimitiveSetList.end();
        ++pitr)
    {
        const osg::PrimitiveSet* src_primset = pitr->get();
        osg::PrimitiveSet* dst_primset = osg::clone(src_primset, osg::CopyOp::DEEP_COPY_ALL);
        dst_primset->offsetIndices(offset);
        dst_primset->setBufferObject(_ebo.get());
        dest_PrimitiveSetList.push_back(dst_primset);
    }
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

        // ** position the cursor function to the Layout and the alignment
        TextBase::positionCursor(endOfLine_coords, cursor, (unsigned int) (endOfLine_itr - startOfLine_itr));


        if (itr!=endOfLine_itr)
        {
            for(;itr!=endOfLine_itr;++itr)
            {
                unsigned int charcode = *itr;

                Glyph3D* glyph = _font->getGlyph3D(_fontSize, charcode);
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
                            osg::Vec2 delta(_font->getKerning(_fontSize, previous_charcode, charcode, _kerningType));
                            cursor.x() += delta.x() * wr;
                            cursor.y() += delta.y() * hr;
                            break;
                          }
                          case RIGHT_TO_LEFT:
                          {
                            osg::Vec2 delta(_font->getKerning(_fontSize, charcode, previous_charcode, _kerningType));
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

        // skip over spaces
        while ((itr!=_text.end()) && (*itr==' ')) ++itr;

        // skip over return
        if ((itr!=_text.end()) && (*itr=='\n')) ++itr;

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

    computePositions();


    if (!_coords)
    {
        OSG_NOTICE<<"void Text3D::computeGlyphRepresentation() _coords = new osg::Vec3Array;"<<std::endl;
        _coords = new osg::Vec3Array;
        _coords->setVertexBufferObject(_vbo.get());
    }
    else
    {
        _coords->clear();
    }

    if (!_normals)
    {
        OSG_NOTICE<<"void Text3D::computeGlyphRepresentation() _normals = new osg::Vec3Array;"<<std::endl;
        _normals = new osg::Vec3Array;
        _normals->setVertexBufferObject(_vbo.get());
    }
    else
    {
        _normals->clear();
    }

    _frontPrimitiveSetList.clear();
    _wallPrimitiveSetList.clear();
    _backPrimitiveSetList.clear();

    TextRenderInfo::const_iterator itLine, endText = _textRenderInfo.end();
    for (itLine = _textRenderInfo.begin(); itLine!=endText; ++itLine)
    {
        // ** for each glyph in the line, do ...
        LineRenderInfo::const_iterator it, endLine = itLine->end();
        for (it = itLine->begin(); it!=endLine; ++it)
        {
            osg::Vec3Array* src_vertices = dynamic_cast<osg::Vec3Array*>(it->_glyphGeometry->getVertexArray());
            osg::Vec3Array* src_normals = dynamic_cast<osg::Vec3Array*>(it->_glyphGeometry->getNormalArray());

            if (!src_vertices) continue;

            unsigned int base = _coords->size();
            osg::Vec3 position = it->_position;

            // copy vertices and place in final position
            _coords->insert(_coords->end(), src_vertices->begin(), src_vertices->end());
            for(unsigned int i=base; i<_coords->size(); ++i)
            {
                (*_coords)[i] += position;
            }
            _coords->dirty();

            // copy normals
            _normals->insert(_normals->end(), src_normals->begin(), src_normals->end());
            _normals->dirty();

            copyAndOffsetPrimitiveSets(_frontPrimitiveSetList, it->_glyphGeometry->getFrontPrimitiveSetList(), base);
            copyAndOffsetPrimitiveSets(_wallPrimitiveSetList, it->_glyphGeometry->getWallPrimitiveSetList(), base);
            copyAndOffsetPrimitiveSets(_backPrimitiveSetList, it->_glyphGeometry->getBackPrimitiveSetList(), base);
        }
    }


    // set up the vertices for any boundinbox or alignment decoration
    setupDecoration();

}

osg::BoundingBox Text3D::computeBoundingBox() const
{
    osg::BoundingBox  bbox;

    if (_textBB.valid())
    {
        bbox.expandBy(_textBB.corner(0)*_matrix);
        bbox.expandBy(_textBB.corner(1)*_matrix);
        bbox.expandBy(_textBB.corner(2)*_matrix);
        bbox.expandBy(_textBB.corner(3)*_matrix);
        bbox.expandBy(_textBB.corner(4)*_matrix);
        bbox.expandBy(_textBB.corner(5)*_matrix);
        bbox.expandBy(_textBB.corner(6)*_matrix);
        bbox.expandBy(_textBB.corner(7)*_matrix);
    }

    return bbox;
}

void Text3D::drawImplementation(osg::RenderInfo& renderInfo) const
{
    osg::State & state = *renderInfo.getState();


    // ** save the previous modelview matrix
    osg::Matrix previous_modelview(state.getModelViewMatrix());

    // set up the new modelview matrix
    osg::Matrix modelview;
    bool needToApplyMatrix = computeMatrix(modelview, &state);

    if (needToApplyMatrix)
    {
        // ** mult previous by the modelview for this context
        modelview.postMult(previous_modelview);

        // ** apply this new modelview matrix
        state.applyModelViewMatrix(modelview);

        // workaround for GL3/GL2
        if (state.getUseModelViewAndProjectionUniforms()) state.applyModelViewAndProjectionUniformsIfRequired();

        // OSG_NOTICE<<"New state.applyModelViewMatrix() "<<modelview<<std::endl;
    }
    else
    {
        // OSG_NOTICE<<"No need to apply matrix "<<std::endl;
    }

    osg::VertexArrayState* vas = state.getCurrentVertexArrayState();
    bool usingVertexBufferObjects = state.useVertexBufferObject(_supportsVertexBufferObjects && _useVertexBufferObjects);
    bool usingVertexArrayObjects = usingVertexBufferObjects && state.useVertexArrayObject(_useVertexArrayObject);
    bool requiresSetArrays = !usingVertexBufferObjects || !usingVertexArrayObjects || vas->getRequiresSetArrays();

    if (requiresSetArrays)
    {
        vas->lazyDisablingOfVertexAttributes();
        vas->setVertexArray(state, _coords.get());
        vas->setNormalArray(state, _normals.get());
        vas->applyDisablingOfVertexAttributes(state);
    }

    if ((_drawMode&(~TEXT))!=0)
    {

        if (!_decorationPrimitives.empty())
        {
        #if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
            osg::State::ApplyModeProxy applyLightingMode(state, GL_LIGHTING, false);
            osg::State::ApplyTextureModeProxy applyTextureMode(state, 0, GL_TEXTURE_2D, false);
        #endif

            for(Primitives::const_iterator itr = _decorationPrimitives.begin();
                itr != _decorationPrimitives.end();
                ++itr)
            {
                (*itr)->draw(state, usingVertexBufferObjects);
            }
        }
    }

    if (_drawMode & TEXT)
    {
        state.Color(_color.r(),_color.g(),_color.b(),_color.a());

        #if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
        osg::State::ApplyModeProxy applyNormalizeMode(state, GL_NORMALIZE, true);
        osg::State::ApplyModeProxy applyLightingMode(state, GL_LIGHTING, true);
        #endif


        const osg::StateSet* frontStateSet = getStateSet();
        const osg::StateSet* wallStateSet = getWallStateSet();
        const osg::StateSet* backStateSet = getBackStateSet();

        if (wallStateSet==0) wallStateSet = frontStateSet;
        if (backStateSet==0) backStateSet = frontStateSet;

        for(osg::Geometry::PrimitiveSetList::const_iterator itr=_frontPrimitiveSetList.begin(), end = _frontPrimitiveSetList.end(); itr!=end; ++itr)
        {
            (*itr)->draw(state, usingVertexBufferObjects);
        }

        if (wallStateSet!=frontStateSet) state.apply(wallStateSet);

        for(osg::Geometry::PrimitiveSetList::const_iterator itr=_wallPrimitiveSetList.begin(), end = _wallPrimitiveSetList.end(); itr!=end; ++itr)
        {
            (*itr)->draw(state, usingVertexBufferObjects);
        }

        if (backStateSet!=wallStateSet) state.apply(backStateSet);

        for(osg::Geometry::PrimitiveSetList::const_iterator itr=_backPrimitiveSetList.begin(), end = _backPrimitiveSetList.end(); itr!=end; ++itr)
        {
            (*itr)->draw(state, usingVertexBufferObjects);
        }
    }

    if (usingVertexBufferObjects && !usingVertexArrayObjects)
    {
        // unbind the VBO's if any are used.
        vas->unbindVertexBufferObject();
        vas->unbindElementBufferObject();
    }

    if (needToApplyMatrix)
    {
        // restore the previous modelview matrix
        state.applyModelViewMatrix(previous_modelview);

        // workaround for GL3/GL2
        if (state.getUseModelViewAndProjectionUniforms()) state.applyModelViewAndProjectionUniformsIfRequired();
    }
}

void Text3D::resizeGLObjectBuffers(unsigned int maxSize)
{
    TextBase::resizeGLObjectBuffers(maxSize);

    for(osg::Geometry::PrimitiveSetList::const_iterator itr=_frontPrimitiveSetList.begin(), end = _frontPrimitiveSetList.end(); itr!=end; ++itr)
    {
        (*itr)->resizeGLObjectBuffers(maxSize);
    }

    for(osg::Geometry::PrimitiveSetList::const_iterator itr=_wallPrimitiveSetList.begin(), end = _wallPrimitiveSetList.end(); itr!=end; ++itr)
    {
        (*itr)->resizeGLObjectBuffers(maxSize);
    }

    for(osg::Geometry::PrimitiveSetList::const_iterator itr=_backPrimitiveSetList.begin(), end = _backPrimitiveSetList.end(); itr!=end; ++itr)
    {
        (*itr)->resizeGLObjectBuffers(maxSize);
    }
}

void Text3D::releaseGLObjects(osg::State* state) const
{
    TextBase::releaseGLObjects(state);

    for(osg::Geometry::PrimitiveSetList::const_iterator itr=_frontPrimitiveSetList.begin(), end = _frontPrimitiveSetList.end(); itr!=end; ++itr)
    {
        (*itr)->releaseGLObjects(state);
    }

    for(osg::Geometry::PrimitiveSetList::const_iterator itr=_wallPrimitiveSetList.begin(), end = _wallPrimitiveSetList.end(); itr!=end; ++itr)
    {
        (*itr)->releaseGLObjects(state);
    }

    for(osg::Geometry::PrimitiveSetList::const_iterator itr=_backPrimitiveSetList.begin(), end = _backPrimitiveSetList.end(); itr!=end; ++itr)
    {
        (*itr)->releaseGLObjects(state);
    }
}

}

