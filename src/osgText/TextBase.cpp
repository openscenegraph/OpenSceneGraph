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


#include <osgText/TextBase>
#include <osgText/Font>

#include <osg/Math>
#include <osg/GL>
#include <osg/Notify>
#include <osg/PolygonOffset>
#include <osg/TexEnv>
#include <osg/io_utils>

#include <osgUtil/CullVisitor>

#include <osgDB/ReadFile>

using namespace osg;
using namespace osgText;

//#define TREES_CODE_FOR_MAKING_SPACES_EDITABLE

TextBase::TextBase():
    _color(1.0f,1.0f,1.0f,1.0f),
    _fontSize(32,32),
    _characterHeight(32),
    _characterSizeMode(OBJECT_COORDS),
    _maximumWidth(0.0f),
    _maximumHeight(0.0f),
    _lineSpacing(0.0f),
    _alignment(BASE_LINE),
    _axisAlignment(XY_PLANE),
    _autoRotateToScreen(false),
    _layout(LEFT_TO_RIGHT),
    _drawMode(TEXT),
    _textBBMargin(0.0f),
    _textBBColor(0.0, 0.0, 0.0, 0.5),
    _kerningType(KERNING_DEFAULT),
    _lineCount(0),
    _glyphNormalized(false)
{
    setUseDisplayList(false);
    setSupportsDisplayList(false);

    initArraysAndBuffers();
}

TextBase::TextBase(const TextBase& textBase,const osg::CopyOp& copyop):
    osg::Drawable(textBase,copyop),
    _color(textBase._color),
    _font(textBase._font),
    _style(textBase._style),
    _fontSize(textBase._fontSize),
    _characterHeight(textBase._characterHeight),
    _characterSizeMode(textBase._characterSizeMode),
    _maximumWidth(textBase._maximumWidth),
    _maximumHeight(textBase._maximumHeight),
    _lineSpacing(textBase._lineSpacing),
    _text(textBase._text),
    _position(textBase._position),
    _alignment(textBase._alignment),
    _axisAlignment(textBase._axisAlignment),
    _rotation(textBase._rotation),
    _autoRotateToScreen(textBase._autoRotateToScreen),
    _layout(textBase._layout),
    _drawMode(textBase._drawMode),
    _textBBMargin(textBase._textBBMargin),
    _textBBColor(textBase._textBBColor),
    _kerningType(textBase._kerningType),
    _lineCount(textBase._lineCount),
    _glyphNormalized(textBase._glyphNormalized)
{
    initArraysAndBuffers();
}

TextBase::~TextBase()
{
}

osg::StateSet* TextBase::createStateSet()
{
    return 0;
}

void TextBase::initArraysAndBuffers()
{
     _vbo = new osg::VertexBufferObject;
     _ebo = new osg::ElementBufferObject;

    _coords = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
    _normals = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
    _colorCoords = new osg::Vec4Array(osg::Array::BIND_PER_VERTEX);
    _texcoords = new osg::Vec2Array(osg::Array::BIND_PER_VERTEX);

    _coords->setBufferObject(_vbo.get());
    _normals->setBufferObject(_vbo.get());
    _colorCoords->setBufferObject(_vbo.get());
    _texcoords->setBufferObject(_vbo.get());
}

osg::VertexArrayState* TextBase::createVertexArrayStateImplementation(osg::RenderInfo& renderInfo) const
{
    State& state = *renderInfo.getState();

    VertexArrayState* vas = new osg::VertexArrayState(&state);

    // OSG_NOTICE<<"Creating new osg::VertexArrayState "<< vas<<std::endl;

    if (_coords.valid()) vas->assignVertexArrayDispatcher();
    if (_colorCoords.valid()) vas->assignColorArrayDispatcher();
    if (_normals.valid()) vas->assignNormalArrayDispatcher();

    if (_texcoords.valid()) vas->assignTexCoordArrayDispatcher(1);

    if (state.useVertexArrayObject(_useVertexArrayObject))
    {
        OSG_INFO<<"TextBase::createVertexArrayState() Setup VertexArrayState to use VAO "<<vas<<std::endl;

        vas->generateVertexArrayObject();
    }
    else
    {
        OSG_INFO<<"TextBase::createVertexArrayState() Setup VertexArrayState to without using VAO "<<vas<<std::endl;
    }

    return vas;
}

void TextBase::compileGLObjects(osg::RenderInfo& renderInfo) const
{
    Drawable::compileGLObjects(renderInfo);
}

void TextBase::resizeGLObjectBuffers(unsigned int maxSize)
{
    if (_font.valid()) _font->resizeGLObjectBuffers(maxSize);

    if (_coords.valid()) _coords->resizeGLObjectBuffers(maxSize);
    if (_normals.valid()) _normals->resizeGLObjectBuffers(maxSize);
    if (_colorCoords.valid()) _colorCoords->resizeGLObjectBuffers(maxSize);
    if (_texcoords.valid()) _texcoords->resizeGLObjectBuffers(maxSize);

    for(Primitives::const_iterator itr = _decorationPrimitives.begin();
        itr != _decorationPrimitives.end();
        ++itr)
    {
        (*itr)->resizeGLObjectBuffers(maxSize);
    }

    Drawable::resizeGLObjectBuffers(maxSize);
}


void TextBase::releaseGLObjects(osg::State* state) const
{
    if (_font.valid()) _font->releaseGLObjects(state);

    if (_coords.valid()) _coords->releaseGLObjects(state);
    if (_normals.valid()) _normals->releaseGLObjects(state);
    if (_colorCoords.valid()) _colorCoords->releaseGLObjects(state);
    if (_texcoords.valid()) _texcoords->releaseGLObjects(state);

    for(Primitives::const_iterator itr = _decorationPrimitives.begin();
        itr != _decorationPrimitives.end();
        ++itr)
    {
        (*itr)->releaseGLObjects(state);
    }

    Drawable::releaseGLObjects(state);
}

void TextBase::setColor(const osg::Vec4& color)
{
    _color = color;
}

void TextBase::assignStateSet()
{
    setStateSet(createStateSet());
}

void TextBase::setFont(osg::ref_ptr<Font> font)
{
    if (_font==font) return;

    _font = font;

    assignStateSet();

    computeGlyphRepresentation();
}

void TextBase::setFont(const std::string& fontfile)
{
    setFont(readRefFontFile(fontfile));
}

void TextBase::setFontResolution(unsigned int width, unsigned int height)
{
    FontResolution size(width,height);
    if (_fontSize==size) return;

    _fontSize = size;

    assignStateSet();

    computeGlyphRepresentation();
}

void TextBase::setCharacterSize(float height)
{
    if (_characterHeight==height) return;

    _characterHeight = height;
    computeGlyphRepresentation();
}

void TextBase::setCharacterSize(float height, float aspectRatio)
{
    if (getCharacterAspectRatio()!=aspectRatio)
    {
        getOrCreateStyle()->setWidthRatio(aspectRatio);
    }
    setCharacterSize(height);
}

void TextBase::setMaximumWidth(float maximumWidth)
{
    if (_maximumWidth==maximumWidth) return;

    _maximumWidth = maximumWidth;
    computeGlyphRepresentation();
}

void  TextBase::setMaximumHeight(float maximumHeight)
{
    if (_maximumHeight==maximumHeight) return;

    _maximumHeight = maximumHeight;
    computeGlyphRepresentation();
}

void TextBase::setLineSpacing(float lineSpacing)
{
    if (_lineSpacing==lineSpacing) return;

    _lineSpacing = lineSpacing;
    computeGlyphRepresentation();
}


void TextBase::setText(const String& text)
{
    if (_text==text) return;

    _text = text;
    computeGlyphRepresentation();
}

void TextBase::setText(const std::string& text)
{
    setText(String(text));
}

void TextBase::setText(const std::string& text,String::Encoding encoding)
{
    setText(String(text,encoding));
}


void TextBase::setText(const wchar_t* text)
{
    setText(String(text));
}

void TextBase::setPosition(const osg::Vec3& pos)
{
    if (_position==pos) return;

    _position = pos;
    computePositions();
}

void TextBase::setAlignment(AlignmentType alignment)
{
    if (_alignment==alignment) return;

    _alignment = alignment;
    computePositions();
    // computeGlyphRepresentation();
}

void TextBase::setAxisAlignment(AxisAlignment axis)
{
    _axisAlignment = axis;

    switch(axis)
    {
    case XZ_PLANE:
        setAutoRotateToScreen(false);
        setRotation(osg::Quat(osg::inDegrees(90.0f),osg::Vec3(1.0f,0.0f,0.0f)));
        break;
    case REVERSED_XZ_PLANE:
        setAutoRotateToScreen(false);
        setRotation(osg::Quat(osg::inDegrees(180.0f),osg::Vec3(0.0f,1.0f,0.0f))*
                    osg::Quat(osg::inDegrees(90.0f),osg::Vec3(1.0f,0.0f,0.0f)));
        break;
    case YZ_PLANE:
        setAutoRotateToScreen(false);
        setRotation(osg::Quat(osg::inDegrees(90.0f),osg::Vec3(1.0f,0.0f,0.0f))*
                    osg::Quat(osg::inDegrees(90.0f),osg::Vec3(0.0f,0.0f,1.0f)));
        break;
    case REVERSED_YZ_PLANE:
        setAutoRotateToScreen(false);
        setRotation(osg::Quat(osg::inDegrees(180.0f),osg::Vec3(0.0f,1.0f,0.0f))*
                    osg::Quat(osg::inDegrees(90.0f),osg::Vec3(1.0f,0.0f,0.0f))*
                    osg::Quat(osg::inDegrees(90.0f),osg::Vec3(0.0f,0.0f,1.0f)));
        break;
    case XY_PLANE:
        setAutoRotateToScreen(false);
        setRotation(osg::Quat());  // nop - already on XY plane.
        break;
    case REVERSED_XY_PLANE:
        setAutoRotateToScreen(false);
        setRotation(osg::Quat(osg::inDegrees(180.0f),osg::Vec3(0.0f,1.0f,0.0f)));
        break;
    case SCREEN:
        setAutoRotateToScreen(true);
        setRotation(osg::Quat());  // nop - already on XY plane.
        break;
    default: break;
    }
}

void TextBase::setRotation(const osg::Quat& quat)
{
    _rotation = quat;
    computePositions();
}


void TextBase::setAutoRotateToScreen(bool autoRotateToScreen)
{
    if (_autoRotateToScreen==autoRotateToScreen) return;

    _autoRotateToScreen = autoRotateToScreen;
    computePositions();
}


void TextBase::setLayout(Layout layout)
{
    if (_layout==layout) return;

    _layout = layout;
    computeGlyphRepresentation();
}


void TextBase::setDrawMode(unsigned int mode)
{
    if (_drawMode==mode) return;

    _drawMode=mode;
}


void TextBase::setBoundingBoxMargin(float margin)
{
    if (_textBBMargin == margin)
        return;

    _textBBMargin = margin;
    computeGlyphRepresentation();
}


osg::BoundingBox TextBase::computeBoundingBox() const
{
    osg::BoundingBox  bbox;
#if 0
    return bbox;
#endif

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

#if 0
        if (!bbox.valid())
        {
            // Provide a fallback in cases where no bounding box has been been setup so far.
            // Note, assume a scaling of 1.0 for _characterSizeMode!=OBJECT_COORDS as the
            // for screen space coordinates size modes we don't know what scale will be used until
            // the text is actually rendered, but we haven't to assume something otherwise the
            // text label will be culled by view or small feature culling on first frame.
            if (_autoRotateToScreen)
            {
                // use bounding sphere encompassing the maximum size of the text centered on the _position
                double radius = _textBB.radius();
                osg::Vec3 diagonal(radius, radius, radius);
                bbox.set(_position-diagonal, _position+diagonal);
            }
            else
            {
                osg::Matrix matrix;
                matrix.makeTranslate(-_offset);
                matrix.postMultRotate(_rotation);
                matrix.postMultTranslate(_position);
                bbox.expandBy(_textBB.corner(0)*_matrix);
                bbox.expandBy(_textBB.corner(1)*_matrix);
                bbox.expandBy(_textBB.corner(2)*_matrix);
                bbox.expandBy(_textBB.corner(3)*_matrix);
                bbox.expandBy(_textBB.corner(4)*_matrix);
                bbox.expandBy(_textBB.corner(5)*_matrix);
                bbox.expandBy(_textBB.corner(6)*_matrix);
                bbox.expandBy(_textBB.corner(7)*_matrix);
            }
        }
#endif
    }

    return bbox;
}

void TextBase::computePositions()
{
    computePositionsImplementation();

    osg::Matrix matrix;
    computeMatrix(matrix, 0);

    const_cast<TextBase*>(this)->dirtyBound();
}

void TextBase::computePositionsImplementation()
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

        case LEFT_BASE_LINE:  _offset.set(_textBB.xMin(),0.0f,0.0f); break;
        case CENTER_BASE_LINE:  _offset.set((_textBB.xMax()+_textBB.xMin())*0.5f,0.0f,0.0f); break;
        case RIGHT_BASE_LINE:  _offset.set(_textBB.xMax(),0.0f,0.0f); break;

        case LEFT_BOTTOM_BASE_LINE:  _offset.set(_textBB.xMin(),-_characterHeight*(1.0 + _lineSpacing)*(_lineCount-1),0.0f); break;
        case CENTER_BOTTOM_BASE_LINE:  _offset.set((_textBB.xMax()+_textBB.xMin())*0.5f,-_characterHeight*(1.0 + _lineSpacing)*(_lineCount-1),0.0f); break;
        case RIGHT_BOTTOM_BASE_LINE:  _offset.set(_textBB.xMax(),-_characterHeight*(1.0 + _lineSpacing)*(_lineCount-1),0.0f); break;
    }

    _normal = osg::Vec3(0.0f,0.0f,1.0f);
}

bool TextBase::computeMatrix(osg::Matrix& matrix, osg::State* state) const
{
    if (state && (_characterSizeMode!=OBJECT_COORDS || _autoRotateToScreen))
    {
        osg::Matrix modelview = state->getModelViewMatrix();
        osg::Matrix projection = state->getProjectionMatrix();

        matrix.makeTranslate(-_offset);

        osg::Matrix rotate_matrix;
        if (_autoRotateToScreen)
        {
            osg::Matrix temp_matrix(modelview);
            temp_matrix.setTrans(0.0f,0.0f,0.0f);

            rotate_matrix.invert(temp_matrix);
        }

        matrix.postMultRotate(_rotation);

        if (_characterSizeMode!=OBJECT_COORDS)
        {
            osg::Matrix M(rotate_matrix);
            M.postMultTranslate(_position);
            M.postMult(modelview);
            osg::Matrix& P = projection;

            // compute the pixel size vector.

            // pre adjust P00,P20,P23,P33 by multiplying them by the viewport window matrix.
            // here we do it in short hand with the knowledge of how the window matrix is formed
            // note P23,P33 are multiplied by an implicit 1 which would come from the window matrix.
            // Robert Osfield, June 2002.

            int width = 1280;
            int height = 1024;

            const osg::Viewport* viewport = state->getCurrentViewport();
            if (viewport)
            {
                width = static_cast<int>(viewport->width());
                height = static_cast<int>(viewport->height());
            }

            // scaling for horizontal pixels
            float P00 = P(0,0)*width*0.5f;
            float P20_00 = P(2,0)*width*0.5f + P(2,3)*width*0.5f;
            osg::Vec3 scale_00(M(0,0)*P00 + M(0,2)*P20_00,
                               M(1,0)*P00 + M(1,2)*P20_00,
                               M(2,0)*P00 + M(2,2)*P20_00);

            // scaling for vertical pixels
            float P10 = P(1,1)*height*0.5f;
            float P20_10 = P(2,1)*height*0.5f + P(2,3)*height*0.5f;
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

            if (_glyphNormalized)
            {
                osg::Vec3 scaleVec(_characterHeight/getCharacterAspectRatio(), _characterHeight, _characterHeight);
                matrix.postMultScale(scaleVec);
            }

            if (_characterSizeMode==SCREEN_COORDS)
            {
                float scale_font_vert=_characterHeight/pixelSizeVert;
                float scale_font_hori=_characterHeight/getCharacterAspectRatio()/pixelSizeHori;

                if (P10<0)
                   scale_font_vert=-scale_font_vert;
                matrix.postMultScale(osg::Vec3f(scale_font_hori, scale_font_vert, scale_font_hori));
            }
            else if (pixelSizeVert>getFontHeight())
            {
                float scale_font = getFontHeight()/pixelSizeVert;
                matrix.postMultScale(osg::Vec3f(scale_font, scale_font, scale_font));
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
        matrix.makeTranslate(-_offset);
        if (_glyphNormalized)
        {
            osg::Vec3 scaleVec(_characterHeight/getCharacterAspectRatio(), _characterHeight, _characterHeight);
            matrix.postMultScale(scaleVec);
        }
        matrix.postMultRotate(_rotation);
        matrix.postMultTranslate(_position);
        // OSG_NOTICE<<"New Need to rotate "<<matrix<<std::endl;
    }
    else
    {
        matrix.makeTranslate(-_offset);
        if (_glyphNormalized)
        {
            osg::Vec3 scaleVec(_characterHeight/getCharacterAspectRatio(), _characterHeight, _characterHeight);
            matrix.postMultScale(scaleVec);
        }
        matrix.postMultTranslate(_position);
    }

    if (_matrix!=matrix)
    {
        _matrix = matrix;
        const_cast<TextBase*>(this)->dirtyBound();
    }

    return true;
}

void TextBase::positionCursor(const osg::Vec2 & endOfLine_coords, osg::Vec2 & cursor, unsigned int linelength)
{
    switch(_layout)
    {
        case LEFT_TO_RIGHT:
        {
            switch (_alignment)
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
            switch (_alignment)
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
            switch (_alignment)
            {
                // TODO: current behaviour top baselines lined up in both cases - need to implement
                //       top of characters aligment - Question is this necessary?
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
}


void TextBase::setupDecoration()
{
    unsigned int numVerticesRequired = 0;
    if (_drawMode & FILLEDBOUNDINGBOX) numVerticesRequired += 4;
    if (_drawMode & BOUNDINGBOX) numVerticesRequired += 8;
    if (_drawMode & ALIGNMENT) numVerticesRequired += 4;

    _decorationPrimitives.clear();

    if (numVerticesRequired==0) return;

    if (!_coords)
    {
        _coords = new osg::Vec3Array;
        _coords->setBufferObject(_vbo.get());
        _coords->resize(numVerticesRequired);
    }

    if (!_texcoords)
    {
        _texcoords = new osg::Vec2Array;
        _texcoords->setBufferObject(_vbo.get());
        _texcoords->resize(numVerticesRequired);
    }

    osg::Vec2 default_texcoord(-1.0, -1.0);

    if ((_drawMode & FILLEDBOUNDINGBOX)!=0 && _textBB.valid())
    {
        osg::Vec3 c000(_textBB.xMin(),_textBB.yMin(),_textBB.zMin());
        osg::Vec3 c100(_textBB.xMax(),_textBB.yMin(),_textBB.zMin());
        osg::Vec3 c110(_textBB.xMax(),_textBB.yMax(),_textBB.zMin());
        osg::Vec3 c010(_textBB.xMin(),_textBB.yMax(),_textBB.zMin());

        unsigned int base = _coords->size();

        _coords->push_back(c000);
        _coords->push_back(c100);
        _coords->push_back(c110);
        _coords->push_back(c010);

        _texcoords->push_back(default_texcoord);
        _texcoords->push_back(default_texcoord);
        _texcoords->push_back(default_texcoord);
        _texcoords->push_back(default_texcoord);

        osg::ref_ptr<osg::DrawElementsUShort> primitives = new osg::DrawElementsUShort(GL_TRIANGLES);
        primitives->setBufferObject(_ebo.get());
        _decorationPrimitives.push_back(primitives);

        primitives->push_back(base);
        primitives->push_back(base+1);
        primitives->push_back(base+2);
        primitives->push_back(base);
        primitives->push_back(base+2);
        primitives->push_back(base+3);

        _coords->dirty();
        primitives->dirty();
    }

    if ((_drawMode & BOUNDINGBOX)!=0 && _textBB.valid())
    {
        if (_textBB.zMin()==_textBB.zMax())
        {
            osg::Vec3 c000(_textBB.xMin(),_textBB.yMin(),_textBB.zMin());
            osg::Vec3 c100(_textBB.xMax(),_textBB.yMin(),_textBB.zMin());
            osg::Vec3 c110(_textBB.xMax(),_textBB.yMax(),_textBB.zMin());
            osg::Vec3 c010(_textBB.xMin(),_textBB.yMax(),_textBB.zMin());

            unsigned int base = _coords->size();

            _coords->push_back(c000);
            _coords->push_back(c100);
            _coords->push_back(c110);
            _coords->push_back(c010);


            _texcoords->push_back(default_texcoord);
            _texcoords->push_back(default_texcoord);
            _texcoords->push_back(default_texcoord);
            _texcoords->push_back(default_texcoord);

            osg::ref_ptr<osg::DrawElementsUShort> primitives = new osg::DrawElementsUShort(GL_LINE_LOOP);
            primitives->setBufferObject(_ebo.get());
            _decorationPrimitives.push_back(primitives);

            primitives->push_back(base);
            primitives->push_back(base+1);
            primitives->push_back(base+2);
            primitives->push_back(base+3);

            _coords->dirty();
            primitives->dirty();
        }
        else
        {
            osg::Vec3 c000(_textBB.xMin(),_textBB.yMin(),_textBB.zMin());
            osg::Vec3 c100(_textBB.xMax(),_textBB.yMin(),_textBB.zMin());
            osg::Vec3 c110(_textBB.xMax(),_textBB.yMax(),_textBB.zMin());
            osg::Vec3 c010(_textBB.xMin(),_textBB.yMax(),_textBB.zMin());

            osg::Vec3 c001(_textBB.xMin(),_textBB.yMin(),_textBB.zMax());
            osg::Vec3 c101(_textBB.xMax(),_textBB.yMin(),_textBB.zMax());
            osg::Vec3 c111(_textBB.xMax(),_textBB.yMax(),_textBB.zMax());
            osg::Vec3 c011(_textBB.xMin(),_textBB.yMax(),_textBB.zMax());

            unsigned int base = _coords->size();

            _coords->push_back(c000); // +0
            _coords->push_back(c100); // +1
            _coords->push_back(c110); // +2
            _coords->push_back(c010); // +3

            _texcoords->push_back(default_texcoord);
            _texcoords->push_back(default_texcoord);
            _texcoords->push_back(default_texcoord);
            _texcoords->push_back(default_texcoord);

            _coords->push_back(c001); // +4
            _coords->push_back(c101); // +5
            _coords->push_back(c111); // +6
            _coords->push_back(c011); // +7

            _texcoords->push_back(default_texcoord);
            _texcoords->push_back(default_texcoord);
            _texcoords->push_back(default_texcoord);
            _texcoords->push_back(default_texcoord);

            osg::ref_ptr<osg::DrawElementsUShort> primitives = new osg::DrawElementsUShort(GL_LINES);
            primitives->setBufferObject(_ebo.get());
            _decorationPrimitives.push_back(primitives);

            // front loop
            primitives->push_back(base+0);
            primitives->push_back(base+1);

            primitives->push_back(base+1);
            primitives->push_back(base+2);

            primitives->push_back(base+2);
            primitives->push_back(base+3);

            primitives->push_back(base+3);
            primitives->push_back(base+0);

            // back loop
            primitives->push_back(base+4);
            primitives->push_back(base+5);

            primitives->push_back(base+5);
            primitives->push_back(base+6);

            primitives->push_back(base+6);
            primitives->push_back(base+7);

            primitives->push_back(base+7);
            primitives->push_back(base+4);

            // edges from corner 000
            primitives->push_back(base+0);
            primitives->push_back(base+4);

            primitives->push_back(base+1);
            primitives->push_back(base+5);

            primitives->push_back(base+2);
            primitives->push_back(base+6);

            primitives->push_back(base+3);
            primitives->push_back(base+7);

            _coords->dirty();
            primitives->dirty();
        }
    }

    if (_drawMode & ALIGNMENT)
    {
        float cursorsize = _characterHeight*0.5f;

        osg::Vec3 hl(osg::Vec3(_offset.x()-cursorsize,_offset.y(),_offset.z()));
        osg::Vec3 hr(osg::Vec3(_offset.x()+cursorsize,_offset.y(),_offset.z()));
        osg::Vec3 vt(osg::Vec3(_offset.x(),_offset.y()-cursorsize,_offset.z()));
        osg::Vec3 vb(osg::Vec3(_offset.x(),_offset.y()+cursorsize,_offset.z()));

        unsigned int base = _coords->size();

        _coords->push_back(hl);
        _coords->push_back(hr);
        _coords->push_back(vt);
        _coords->push_back(vb);

        _texcoords->push_back(default_texcoord);
        _texcoords->push_back(default_texcoord);
        _texcoords->push_back(default_texcoord);
        _texcoords->push_back(default_texcoord);

        osg::ref_ptr<osg::DrawElementsUShort> primitives = new osg::DrawElementsUShort(GL_LINES);
        primitives->setBufferObject(_ebo.get());
        _decorationPrimitives.push_back(primitives);

        // front loop
        primitives->push_back(base+0);
        primitives->push_back(base+1);

        primitives->push_back(base+2);
        primitives->push_back(base+3);

        _coords->dirty();
        primitives->dirty();
    }
}
