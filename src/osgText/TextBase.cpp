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
    _lineCount(0)
{
    setStateSet(Font::getDefaultFont()->getStateSet());
    setUseDisplayList(false);
    setSupportsDisplayList(false);
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
    _lineCount(textBase._lineCount)
{
}

TextBase::~TextBase()
{
}

void TextBase::setColor(const osg::Vec4& color)
{
    _color = color;
}


void TextBase::setFont(osg::ref_ptr<Font> font)
{
    if (_font==font) return;

    _font = font;

    computeGlyphRepresentation();
}

void TextBase::setFont(const std::string& fontfile)
{
    setFont(readRefFontFile(fontfile));
}

void TextBase::setFontResolution(unsigned int width, unsigned int height)
{
    _fontSize = FontResolution(width,height);
    computeGlyphRepresentation();
}

void TextBase::setCharacterSize(float height)
{
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
    _maximumWidth = maximumWidth;
    computeGlyphRepresentation();
}

void  TextBase::setMaximumHeight(float maximumHeight)
{
    _maximumHeight = maximumHeight;
    computeGlyphRepresentation();
}

void TextBase::setLineSpacing(float lineSpacing)
{
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
    computeGlyphRepresentation();
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


osg::BoundingBox TextBase::computeBound() const
{
    osg::BoundingBox  bbox;

    if (_textBB.valid())
    {
        for(unsigned int i=0;i<_autoTransformCache.size();++i)
        {
            if (_autoTransformCache[i]._traversalNumber>=0)
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
    }

    return bbox;
}

void TextBase::computePositions()
{
    unsigned int size = osg::maximum(osg::DisplaySettings::instance()->getMaxNumberOfGraphicsContexts(),_autoTransformCache.size());

    // FIXME: OPTIMIZE: This would be one of the ideal locations to
    // call computeAverageGlyphWidthAndHeight(). It is out of the contextID loop
    // so the value would be computed fewer times. But the code will need changes
    // to get the value down to the locations it is needed. (Either pass through parameters
    // or member variables, but we would need a system to know if the values are stale.)


    for(unsigned int i=0;i<size;++i)
    {
        computePositions(i);
    }
}

void TextBase::setThreadSafeRefUnref(bool threadSafe)
{
    Drawable::setThreadSafeRefUnref(threadSafe);
}

void TextBase::resizeGLObjectBuffers(unsigned int maxSize)
{
    Drawable::resizeGLObjectBuffers(maxSize);

    _autoTransformCache.resize(maxSize);
}


void TextBase::releaseGLObjects(osg::State* state) const
{
    Drawable::releaseGLObjects(state);
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
}

