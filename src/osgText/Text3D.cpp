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
#include <osg/io_utils>
namespace osgText 
{

Text3D::Text3D():
    _font(0),
    _characterDepth(1),
    _renderMode(PER_GLYPH)
{
}

Text3D::Text3D(const Text3D & text3D, const osg::CopyOp & copyop):
    osgText::TextBase(text3D, copyop),
    _font(text3D._font),
    _characterDepth(text3D._characterDepth),
    _renderMode(text3D._renderMode)
{
    computeGlyphRepresentation();
}

//void Text3D::accept(osg::Drawable::ConstAttributeFunctor& af) const
//{
//    TODO
//}

//void Text3D::accept(osg::PrimitiveFunctor& /*pf*/) const
//{
//    TODO 
//}

void Text3D::setFont(osg::ref_ptr<Font3D> font)
{ 
    _font = font;
    
    computeGlyphRepresentation();
}

void Text3D::setFont(const std::string & fontfile)
{ 
    setFont(readRefFont3DFile(fontfile));
}

String::iterator Text3D::computeLastCharacterOnLine(osg::Vec2& cursor, String::iterator first,String::iterator last)
{
    if (_font.valid() == false) return last;

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

    float maximumHeight = _maximumHeight / _font->getScale();
    float maximumWidth = _maximumWidth / _font->getScale();
    
    for(bool outOfSpace=false;lastChar!=last;++lastChar)
    {
        unsigned int charcode = *lastChar;
        
        if (charcode=='\n')
        {
            return lastChar;
        }

        Font3D::Glyph3D* glyph = _font->getGlyph(charcode);
        if (glyph)
        {
            const osg::BoundingBox & bb = glyph->getBoundingBox();
            
            // adjust cursor position w.r.t any kerning.
            if (previous_charcode)
            {
                
                if (_layout == RIGHT_TO_LEFT)
                {
                    cursor.x() -= glyph->getHorizontalAdvance();
                }
                
                if (kerning)
                {
                    switch (_layout)
                    {
                        case LEFT_TO_RIGHT:
                        {
                            cursor += _font->getKerning(previous_charcode, charcode, _kerningType);
                            break;
                        }
                        case RIGHT_TO_LEFT:
                        {
                            cursor -= _font->getKerning(charcode, previous_charcode, _kerningType);
                            break;
                        }
                        case VERTICAL:
                            break; // no kerning when vertical.
                    }
                }
            }
            else
            { 
                switch (_layout)
                {
                    case LEFT_TO_RIGHT:
                    {
                        cursor.x() -= glyph->getHorizontalBearing().x();
                        break;
                    }
                    case RIGHT_TO_LEFT:
                    {
                        cursor.x() -= bb.xMax();
                        break;
                    }
                    case VERTICAL:
                        break;
                }
                
            }
            
            

            switch(_layout)
            {
              case LEFT_TO_RIGHT:
              {
                if (maximumWidth>0.0f && cursor.x()+bb.xMax()>maximumWidth) outOfSpace=true;
                if(maximumHeight>0.0f && cursor.y()<-maximumHeight) outOfSpace=true;
                break;
              }
              case RIGHT_TO_LEFT:
              {
                if (maximumWidth>0.0f && cursor.x()+bb.xMin()<-maximumWidth) outOfSpace=true;
                if(maximumHeight>0.0f && cursor.y()<-maximumHeight) outOfSpace=true;
                break;
              }
              case VERTICAL:
                if (maximumHeight>0.0f && cursor.y()<-maximumHeight) outOfSpace=true;
                break;
            }
            
            // => word boundary detection & wrapping
            if (outOfSpace) break;

            // move the cursor onto the next character.
            switch(_layout)
            {
              case LEFT_TO_RIGHT: cursor.x() += glyph->getHorizontalAdvance(); break;
              case RIGHT_TO_LEFT: break;
              case VERTICAL:      cursor.y() -= glyph->getVerticalAdvance(); break;
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
                
                //Substract off glyphs from the cursor position (to correctly center text)
                Font3D::Glyph3D* glyph = _font->getGlyph(*lastValidChar);
                if (glyph)
                {
                    switch(_layout)
                    {
                    case LEFT_TO_RIGHT: cursor.x() -= glyph->getHorizontalAdvance(); break;
                    case RIGHT_TO_LEFT: cursor.x() += glyph->getHorizontalAdvance(); break;
                    case VERTICAL:      cursor.y() += glyph->getVerticalAdvance(); break;
                    }
                }
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

                Font3D::Glyph3D* glyph = _font->getGlyph(charcode);
                if (glyph)
                {
                    const osg::BoundingBox & bb = glyph->getBoundingBox();
                    
                    // adjust cursor position w.r.t any kerning.
                    if (previous_charcode)
                    {
                        if (_layout == RIGHT_TO_LEFT)
                        {
                            cursor.x() -= glyph->getHorizontalAdvance();
                        }
                        
                        if (kerning)
                        {
                            switch (_layout)
                            {
                                case LEFT_TO_RIGHT:
                                {
                                    cursor += _font->getKerning(previous_charcode, charcode, _kerningType);
                                    break;
                                }
                                case RIGHT_TO_LEFT:
                                {
                                    cursor -= _font->getKerning(charcode, previous_charcode, _kerningType);
                                    break;
                                }
                                case VERTICAL:
                                    break; // no kerning when vertical.
                            }
                        }
                    }
                    else
                    { 
                        switch (_layout)
                        {
                            case LEFT_TO_RIGHT:
                            {
                                cursor.x() -= glyph->getHorizontalBearing().x();
                                break;
                            }
                            case RIGHT_TO_LEFT:
                            {
                                cursor.x() -= bb.xMax();
                                break;
                            }
                            case VERTICAL:
                            {
//                                cursor.y() += glyph->getVerticalBearing().y();
                                break;
                            }
                        }
                        
                    }

                    local = cursor;
                    
                    if (_layout==VERTICAL)
                    {
                        local.x() += -glyph->getVerticalBearing().x();
                        local.y() += -glyph->getVerticalBearing().y();
                    }
                     
                    
                    
                    // move the cursor onto the next character.
                    // also expand bounding box
                    switch (_layout)
                    {
                        case LEFT_TO_RIGHT:
                            _textBB.expandBy(osg::Vec3(cursor.x() + bb.xMin(), cursor.y() + bb.yMin(), 0.0f)); //lower left corner
                            _textBB.expandBy(osg::Vec3(cursor.x() + bb.xMax(), cursor.y() + bb.yMax(), 0.0f)); //upper right corner
                            cursor.x() += glyph->getHorizontalAdvance();
                            break;
                        case VERTICAL:
                            _textBB.expandBy(osg::Vec3(cursor.x(), cursor.y(), 0.0f)); //upper left corner
                            _textBB.expandBy(osg::Vec3(cursor.x() + glyph->getWidth(), cursor.y() - glyph->getHeight(), 0.0f)); //lower right corner
                            cursor.y() -= glyph->getVerticalAdvance();
                            break;
                        case RIGHT_TO_LEFT:
                            _textBB.expandBy(osg::Vec3(cursor.x()+bb.xMax(), cursor.y() + bb.yMax(), 0.0f)); //upper right corner
                            _textBB.expandBy(osg::Vec3(cursor.x()+bb.xMin(), cursor.y()+bb.yMin(), 0.0f)); //lower left corner
                            
                            break;
                    }
                    
                    osg::Vec3 pos = osg::Vec3(local.x(), local.y(), 0.0f);
                    currentLineRenderInfo.push_back(Text3D::GlyphRenderInfo(glyph, pos));

                    
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
            case RIGHT_TO_LEFT:
            {
                startOfLine_coords.y() -= (1.0 + _lineSpacing) / _font->getScale();
                ++_lineCount;
                break;
            }
            case VERTICAL:
            {
                startOfLine_coords.x() += _characterHeight / _font->getScale() / _characterAspectRatio * (1.0 + _lineSpacing);
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
    _textBB.expandBy(0.0f,0.0f,-1);
    
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
            bbox.expandBy(osg::Vec3(_textBB.xMin(),_textBB.yMin(),_textBB.zMin())*matrix);
//          bbox.expandBy(osg::Vec3(_textBB.xMax(),_textBB.yMin(),_textBB.zMin())*matrix);
            bbox.expandBy(osg::Vec3(_textBB.xMax(),_textBB.yMax(),_textBB.zMax())*matrix);
//          bbox.expandBy(osg::Vec3(_textBB.xMin(),_textBB.yMax(),_textBB.zMin())*matrix);
        }
    }
    
    return bbox;
}







void Text3D::computePositions(unsigned int contextID) const
{
    if (_font.valid() == false) return;
    
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

    
    float scale = _font->getScale();
    osg::Vec3 scaleVec(scale * _characterHeight, scale * _characterHeight / _characterAspectRatio, _characterDepth);
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
    osg::ref_ptr<osg::RefMatrix> previous(new osg::RefMatrix(state.getModelViewMatrix()));

    // ** get the modelview for this context
    osg::ref_ptr<osg::RefMatrix> modelview(new osg::RefMatrix(_autoTransformCache[contextID]._matrix));

    // ** mult previous by the modelview for this context
    modelview->postMult(*previous.get());
   
    // ** apply this new modelview matrix
    state.applyModelViewMatrix(modelview.get());
    
    
    if (_drawMode & TEXT)
    {
        renderInfo.getState()->disableAllVertexArrays();
        
        glPushAttrib(GL_TRANSFORM_BIT);
        glEnable(GL_RESCALE_NORMAL);
        
        switch(_renderMode)
        {
            case PER_FACE:  renderPerFace(*renderInfo.getState());   break;
            case PER_GLYPH:
            default:        renderPerGlyph(*renderInfo.getState());  break;
        }
        
        glPopAttrib();
    }

    if (_drawMode & BOUNDINGBOX)
    {
        if (_textBB.valid())
        {
            osg::Vec3 c000(osg::Vec3(_textBB.xMin(),_textBB.yMin(),_textBB.zMax()));
            osg::Vec3 c100(osg::Vec3(_textBB.xMax(),_textBB.yMin(),_textBB.zMax()));
            osg::Vec3 c110(osg::Vec3(_textBB.xMax(),_textBB.yMax(),_textBB.zMax()));
            osg::Vec3 c010(osg::Vec3(_textBB.xMin(),_textBB.yMax(),_textBB.zMax()));
        
            osg::Vec3 c001(osg::Vec3(_textBB.xMin(),_textBB.yMin(),_textBB.zMin()));
            osg::Vec3 c101(osg::Vec3(_textBB.xMax(),_textBB.yMin(),_textBB.zMin()));
            osg::Vec3 c111(osg::Vec3(_textBB.xMax(),_textBB.yMax(),_textBB.zMin()));
            osg::Vec3 c011(osg::Vec3(_textBB.xMin(),_textBB.yMax(),_textBB.zMin()));
        
            glBegin(GL_LINE_LOOP);
                glVertex3fv(c000.ptr());
                glVertex3fv(c100.ptr());
                glVertex3fv(c110.ptr());
                glVertex3fv(c010.ptr());
            glEnd();
            
            glBegin(GL_LINE_LOOP);
                glVertex3fv(c001.ptr());
                glVertex3fv(c011.ptr());
                glVertex3fv(c111.ptr());
                glVertex3fv(c101.ptr());
            glEnd();
            
            glBegin(GL_LINES);
                glVertex3fv(c000.ptr());
                glVertex3fv(c001.ptr());
                
                glVertex3fv(c100.ptr());
                glVertex3fv(c101.ptr());
                
                glVertex3fv(c110.ptr());
                glVertex3fv(c111.ptr());
                
                glVertex3fv(c010.ptr());
                glVertex3fv(c011.ptr());
            glEnd();
        }
    }    

    if (_drawMode & ALIGNMENT)
    {
        float cursorsize = _characterHeight*0.5f;

        osg::Vec3 hl(osg::Vec3(_offset.x()-cursorsize,_offset.y(),_offset.z()));
        osg::Vec3 hr(osg::Vec3(_offset.x()+cursorsize,_offset.y(),_offset.z()));
        osg::Vec3 vt(osg::Vec3(_offset.x(),_offset.y()-cursorsize,_offset.z()));
        osg::Vec3 vb(osg::Vec3(_offset.x(),_offset.y()+cursorsize,_offset.z()));

        glBegin(GL_LINES);
            glVertex3fv(hl.ptr());
            glVertex3fv(hr.ptr());
            glVertex3fv(vt.ptr());
            glVertex3fv(vb.ptr());
        glEnd();
        
    }

    // restore the previous modelview matrix
    state.applyModelViewMatrix(previous.get());
}



void Text3D::renderPerGlyph(osg::State & state) const
{   
    // ** for each line, do ...
    TextRenderInfo::const_iterator itLine, endLine = _textRenderInfo.end();
    for (itLine = _textRenderInfo.begin(); itLine!=endLine; ++itLine)
    {
        // ** for each glyph in the line, do ...
        LineRenderInfo::const_iterator it, end = itLine->end();
        for (it = itLine->begin(); it!=end; ++it)
        {
            
            glPushMatrix();

            glTranslatef(it->_position.x(), it->_position.y(), it->_position.z());
           
            // ** apply the vertex array
            state.setVertexPointer(it->_glyph->getVertexArray());
            
            // ** render the front face of the glyph
            glNormal3f(0.0f,0.0f,1.0f);
            
            osg::Geometry::PrimitiveSetList & pslFront = it->_glyph->getFrontPrimitiveSetList();
            for(osg::Geometry::PrimitiveSetList::const_iterator itr=pslFront.begin(), end = pslFront.end(); itr!=end; ++itr)
            {
                
                (*itr)->draw(state, false);
            }
            
            // ** render the wall face of the glyph
            state.setNormalPointer(it->_glyph->getNormalArray());
            osg::Geometry::PrimitiveSetList & pslWall = it->_glyph->getWallPrimitiveSetList();
            for(osg::Geometry::PrimitiveSetList::const_iterator itr=pslWall.begin(), end=pslWall.end(); itr!=end; ++itr)
            {
                (*itr)->draw(state, false);
            }
            
            // ** render the back face of the glyph
            glNormal3f(0.0f,0.0f,-1.0f);
                        
            osg::Geometry::PrimitiveSetList & pslBack = it->_glyph->getBackPrimitiveSetList();
            for(osg::Geometry::PrimitiveSetList::const_iterator itr=pslBack.begin(), end=pslBack.end(); itr!=end; ++itr)
            {
                (*itr)->draw(state, false);
            }
            
            glPopMatrix();
        }
    }
}

void Text3D::renderPerFace(osg::State & state) const
{
    // ** render all front faces
    glNormal3f(0.0f,0.0f,1.0f);
    
    TextRenderInfo::const_iterator itLine, endLine = _textRenderInfo.end();
    for (itLine = _textRenderInfo.begin(); itLine!=endLine; ++itLine)
    {
        // ** for each glyph in the line, do ...
        LineRenderInfo::const_iterator it, end = itLine->end();
        for (it = itLine->begin(); it!=end; ++it)   
        {
            glPushMatrix();
            glTranslatef(it->_position.x(), it->_position.y(), it->_position.z());
            state.setVertexPointer(it->_glyph->getVertexArray());
            
            // ** render the front face of the glyph
            osg::Geometry::PrimitiveSetList & psl = it->_glyph->getFrontPrimitiveSetList();
            for(osg::Geometry::PrimitiveSetList::const_iterator itr=psl.begin(), end=psl.end(); itr!=end; ++itr)
            {
                (*itr)->draw(state, false);
            }
            glPopMatrix();
        }
    }
    

    // ** render all wall face of the text
    for (itLine = _textRenderInfo.begin(); itLine!=endLine; ++itLine)
    {
        // ** for each glyph in the line, do ...
        LineRenderInfo::const_iterator it, end = itLine->end();
        for (it = itLine->begin(); it!=end; ++it)
        {
            glPushMatrix();
            glTranslatef(it->_position.x(), it->_position.y(), it->_position.z());
            state.setVertexPointer(it->_glyph->getVertexArray());
            state.setNormalPointer(it->_glyph->getNormalArray());
            
            osg::Geometry::PrimitiveSetList & psl = it->_glyph->getWallPrimitiveSetList();
            for(osg::Geometry::PrimitiveSetList::const_iterator itr=psl.begin(), end=psl.end(); itr!=end; ++itr)
            {
                (*itr)->draw(state, false);
            }
            glPopMatrix();
        }
    }

    
    // ** render all back face of the text
    glNormal3f(0.0f,0.0f,-1.0f);
                
    for (itLine = _textRenderInfo.begin(); itLine!=endLine; ++itLine)
    {
        // ** for each glyph in the line, do ...
        LineRenderInfo::const_iterator it, end = itLine->end();
        for (it = itLine->begin(); it!=end; ++it)
        {
            glPushMatrix();
            glTranslatef(it->_position.x(), it->_position.y(), it->_position.z());
            state.setVertexPointer(it->_glyph->getVertexArray());
            
            // ** render the back face of the glyph
            osg::Geometry::PrimitiveSetList & psl = it->_glyph->getBackPrimitiveSetList();
            for(osg::Geometry::PrimitiveSetList::const_iterator itr=psl.begin(), end=psl.end(); itr!=end; ++itr)
            {
                (*itr)->draw(state, false);
            }
            glPopMatrix();
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
    TextBase::resizeGLObjectBuffers(maxSize);
    
    if (_font.valid()) _font->resizeGLObjectBuffers(maxSize);
}

void Text3D::releaseGLObjects(osg::State* state) const
{
    TextBase::releaseGLObjects(state);
    
    if (_font.valid()) _font->releaseGLObjects(state);
}

}
