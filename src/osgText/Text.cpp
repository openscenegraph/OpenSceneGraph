/* --------------------------------------------------------------------------
 *
 *    openscenegraph textLib / FTGL wrapper (http://homepages.paradise.net.nz/henryj/code/)
 *
 * --------------------------------------------------------------------------
 *    
 *    prog:    max rheiner;mrn@paus.ch
 *    date:    4/25/2001    (m/d/y)
 *
 * ----------------------------------------------------------------------------
 *
 * --------------------------------------------------------------------------
 */


#include <osgText/Text>

#include <osgDB/FileUtils>

#include "FTFace.h"
#include "FTGLBitmapFont.h"
#include "FTGLPixmapFont.h"
#include "FTGLOutlineFont.h"
#include "FTGLPolygonFont.h"
#include "FTGLTextureFont.h"

//#define BUILD_NO_TEXT

using namespace osg;
using namespace osgText;

///////////////////////////////////////////////////////////////////////////////
// Text
Text::Text()
{
    setDefaults();
}

Text::Text(const Text& text,const osg::CopyOp& copyop):
        Drawable(text,copyop),
        _font(dynamic_cast<Font*>(copyop(text._font.get()))),
        _init(text._init),
        _initAlignment(text._initAlignment),
        _text(text._text),
        _fontType(text._fontType),
        _alignment(text._alignment),
        _drawMode(text._drawMode),
        _boundingBoxType(text._boundingBoxType),
        _axisAlignment(text._axisAlignment),
        _encodedText(text._encodedText),
        _pos(text._pos),
        _alignmentPos(text._alignmentPos),
        _color(text._color)
{
}

Text::Text(Font* font)
{
    setDefaults();

    if(font && font->isOk())
    {
        _init=true;
        _font=font;

        if(dynamic_cast<PolygonFont*>(_font.get()))
            _fontType=POLYGON;
        else if(dynamic_cast<BitmapFont*>(_font.get()))
            _fontType=BITMAP;
        else if(dynamic_cast<PixmapFont*>(_font.get()))
            _fontType=PIXMAP;
        else if(dynamic_cast<TextureFont*>(_font.get()))
            _fontType=TEXTURE;
        else if(dynamic_cast<OutlineFont*>(_font.get()))
            _fontType=OUTLINE;

    }
}


Text::~Text()
{
}

void Text::setFont(Font* font)
{
    if (_font==font) return;
    
    if(font && font->isOk())
    {
    _init=true;
    _font=font;

    if(dynamic_cast<PolygonFont*>(_font.get()))
        _fontType=POLYGON;
    else if(dynamic_cast<BitmapFont*>(_font.get()))
        _fontType=BITMAP;
    else if(dynamic_cast<PixmapFont*>(_font.get()))
        _fontType=PIXMAP;
    else if(dynamic_cast<TextureFont*>(_font.get()))
        _fontType=TEXTURE;
    else if(dynamic_cast<OutlineFont*>(_font.get()))
        _fontType=OUTLINE;

        _initAlignment = false;
        dirtyBound();            

    }
}

void Text::
setDefaults()
{
    _init=false;
    
    _pos.set(0,0,0);
    _alignmentPos.set(0,0,0);
    
    _color.set(1.0f,1.0f,1.0f,1.0f);
    
    _fontType=UNDEF;
    _alignment=LEFT_BOTTOM;
    _drawMode=DEFAULT;

    _boundingBoxType=GLYPH;
    _boundingBoxType=GEOMETRY;

    _axisAlignment = XY_PLANE;

    _initAlignment=false;

     _useDisplayList=false;

    _encodedText = new EncodedText();
}

bool Text::computeBound() const
{
#ifndef BUILD_NO_TEXT
    if(!_init)
    {
        _bbox_computed=false;
        return true;
    }

    // culling
    if(_font->isCreated())
    {    // ready to get the siz
        _bbox.init();

        Vec3 min,max;
        calcBounds(&min,&max);

        _bbox.expandBy(min);
        _bbox.expandBy(max);

        _bbox_computed=true;
    }
    else
    {    // have to wait for the init.
        _bbox.init();

        // to be sure that the obj isn't culled
//        _bbox.expandBy(_pos);

        _bbox.expandBy(_pos + Vec3(-100,-100,-100));
        _bbox.expandBy(_pos + Vec3(100,100,100));

        /*
        _bbox.expandBy(Vec3(-FLT_MAX,-FLT_MAX,-FLT_MAX));
        _bbox.expandBy(Vec3(FLT_MAX,FLT_MAX,FLT_MAX));
        */
        _bbox_computed=true;

    }
#else
    _bbox.init();
    _bbox_computed=true;
#endif

    return true;
}

bool Text::supports(PrimitiveFunctor&) const
{
    return true;
}

void Text::accept(PrimitiveFunctor& functor) const
{
#ifndef BUILD_NO_TEXT
    Vec3 boundingVertices[4];
    boundingVertices[0].set(_bbox._min._v[0],_bbox._max._v[1],_bbox._min._v[2]);
    boundingVertices[1] = _bbox._min;
    boundingVertices[2].set(_bbox._max._v[0],_bbox._min._v[1],_bbox._max._v[2]);
    boundingVertices[3] = _bbox._max;

    functor.setVertexArray(4,boundingVertices);
    functor.drawArrays( GL_QUADS, 0, 4);
#endif
}

void Text::compile(State& state) const
{
#ifndef BUILD_NO_TEXT
    // ahhhh, this is bit doddy, the draw is potentially
    // modifying the text object, this isn't thread safe.
    Text* this_non_const = const_cast<Text*>(this);
    
    if(!_font->isCreated() || !(this_non_const->_font->getFont()->Created(state.getContextID())))
    {
        this_non_const->_font->create(state);
        this_non_const->dirtyBound();
    }

    if(!_initAlignment)
    {
        this_non_const->initAlignment();
    }
#endif
}

void Text::drawImplementation(State& state) const
{
#ifndef BUILD_NO_TEXT

    if(!_init)
        return;
    
    // ahhhh, this is bit doddy, the draw is potentially
    // modifying the text object, this isn't thread safe.
    Text* this_non_const = const_cast<Text*>(this);
    
    if(!_font->isCreated() || !(this_non_const->_font->getFont()->Created(state.getContextID())))
    {
        this_non_const->_font->create(state);
        this_non_const->dirtyBound();
    }

    if(!_initAlignment)
    {
        this_non_const->initAlignment();
    }

    // we must disable all the vertex arrays to prevent any state
    // propagating into text.        
    state.disableAllVertexArrays();
    state.setActiveTextureUnit(0);
    
    // draw boundingBox
    if(_drawMode & BOUNDINGBOX)
        drawBoundingBox();
    // draw alignment
    if(_drawMode & ALIGNMENT)
        drawAlignment();

    // draw boundingBox
    if(_drawMode & TEXT)
    {
        glColor3fv(_color.ptr());

        Vec3    drawPos(_pos+_alignmentPos);
        glPushMatrix();
            switch(_fontType)
            {
                case POLYGON:
                    glTranslatef(drawPos.x(),drawPos.y(),drawPos.z());
                    if(_axisAlignment==XZ_PLANE) glRotatef(90.0f,1.0f,0.0f,0.0f);
                    else if (_axisAlignment==YZ_PLANE) {  glRotatef(90.0f,0.0f,0.0f,1.0f); glRotatef(90.0f,1.0f,0.0f,0.0f);}
                    _font->output(state,getEncodedText());
                    break;
                case OUTLINE:
                    glTranslatef(drawPos.x(),drawPos.y(),drawPos.z());
                    if(_axisAlignment==XZ_PLANE) glRotatef(90.0f,1.0f,0.0f,0.0f);
                    else if (_axisAlignment==YZ_PLANE) {  glRotatef(90.0f,0.0f,0.0f,1.0f); glRotatef(90.0f,1.0f,0.0f,0.0f);}
                    _font->output(state,getEncodedText());
                    break;
                case BITMAP:
                    glRasterPos3f(drawPos.x(),drawPos.y(),drawPos.z());
                    _font->output(state,getEncodedText());
                    break;
                case PIXMAP:
                    glRasterPos3f(drawPos.x(),drawPos.y(),drawPos.z());
                    _font->output(state,getEncodedText());
                    break;
                case TEXTURE:
                    glTranslatef(drawPos.x(),drawPos.y(),drawPos.z());
                    if(_axisAlignment==XZ_PLANE) glRotatef(90.0f,1.0f,0.0f,0.0f);
                    else if (_axisAlignment==YZ_PLANE) {  glRotatef(90.0f,0.0f,0.0f,1.0f); glRotatef(90.0f,1.0f,0.0f,0.0f);}
                    _font->output(state,getEncodedText());
                    break;

            };
        glPopMatrix();
    }

#endif
}

void Text::drawBoundingBox(void) const
{
    if(!_init)
        return;

    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT );
        glDisable(GL_TEXTURE_2D);
        glColor3f(0,1,0);
        glBegin(GL_LINE_LOOP);
            glVertex3f(_bbox.xMin(),_bbox.yMin(),_bbox.zMin());
            glVertex3f(_bbox.xMax(),_bbox.yMin(),_bbox.zMin());
            glVertex3f(_bbox.xMax(),_bbox.yMax(),_bbox.zMin());
            glVertex3f(_bbox.xMin(),_bbox.yMax(),_bbox.zMin());
        glEnd();
    glPopAttrib();
}

void Text::drawAlignment(void) const
{
    if(!_init)
        return;

    double        size=_font->getPointSize()/4;

    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT );
        glDisable(GL_TEXTURE_2D);
        glColor3f(1,0,0);
        glBegin(GL_LINES);
            glVertex3f(_pos.x() - size,_pos.y(),_pos.z());
            glVertex3f(_pos.x() + size,_pos.y(),_pos.z());
            
            glVertex3f(_pos.x(),_pos.y() - size,_pos.z());
            glVertex3f(_pos.x(),_pos.y() + size,_pos.z());
 
        glEnd();
    glPopAttrib();
}
    
void Text::
setPosition(const Vec3& pos)
{ 
    _pos=pos;
    dirtyBound();
}

void Text::
setPosition(const Vec2& pos)
{  
    setPosition(Vec3(pos.x(),pos.y(),0)); 
}

void Text::
calcBounds(Vec3* min,Vec3* max) const
{
    if(!_init)
        return;

    float h=_font->getHeight();
    float w=_font->getWidth(getEncodedText());
    float descender=_font->getDescender();

    min->set(0,descender,0);
    max->set(w,h + descender ,0);
}

bool Text::
initAlignment(void)
{
#ifndef BUILD_NO_TEXT
    if(!_init)
        return false;

    // culling
    if(_font->isCreated())
    {    // ready to get the siz
        _bbox.init();

        Vec3 min,max;
        initAlignment(&min,&max);

        _bbox.expandBy(min);
        _bbox.expandBy(max);

        _bbox_computed=true;

        _initAlignment=true;
    }
    else
    {    // have to wait for the init.
        _bbox.init();

        // to be sure that the obj isn't culled
        _bbox.expandBy(Vec3(-FLT_MAX,-FLT_MAX,-FLT_MAX));
        _bbox.expandBy(Vec3(FLT_MAX,FLT_MAX,FLT_MAX));

        _bbox_computed=true;
    }
#endif
    return true;
}

void Text::
initAlignment(Vec3* min,Vec3* max)
{
#ifndef BUILD_NO_TEXT
    if(!_init)
        return;

    float h=_font->getHeight();
    float w=_font->getWidth(getEncodedText());
    float descender=_font->getDescender();

    min->set(0,descender,0);
    max->set(w,h + descender ,0);

    switch(_boundingBoxType)
    {
        case GLYPH:
            h+=descender;
            switch(_alignment)
            {
                case LEFT_TOP:
                    _alignmentPos.set(0.0,h,0.0);
                    break;
                case LEFT_CENTER:
                    _alignmentPos.set(0.0,h/2.0,0.0);
                    break;
                case LEFT_BOTTOM:
                    _alignmentPos.set(0.0,0.0,0.0);
                    break;

                case CENTER_TOP:
                    _alignmentPos.set(w/2.0,h,0.0);
                    break;
                case CENTER_CENTER:
                    _alignmentPos.set(w/2.0,h/2.0,0.0);
                    break;
                case CENTER_BOTTOM:
                    _alignmentPos.set(w/2.0,0.0,0.0);
                    break;

                case RIGHT_TOP:
                    _alignmentPos.set(w,h,0.0);
                    break;
                case RIGHT_CENTER:
                    _alignmentPos.set(w,h/2.0,0.0);
                    break;
                case RIGHT_BOTTOM:
                    _alignmentPos.set(w,0.0,0.0);
                    break;
            };
            _alignmentPos=-_alignmentPos;

            *min+=_pos+_alignmentPos;
            *max+=_pos+_alignmentPos;
            break;

        case GEOMETRY:
            switch(_alignment)
            {
                case LEFT_TOP:
                    _alignmentPos.set(0.0,h + descender,0.0);
                    break;
                case LEFT_CENTER:
                    _alignmentPos.set(0.0,(max->y()-min->y()) /2.0 + descender,0.0);
                    break;
                case LEFT_BOTTOM:
                    _alignmentPos.set(0.0,descender,0.0);
                    break;

                case CENTER_TOP:
                    _alignmentPos.set(w/2.0,h + descender,0.0);
                    break;
                case CENTER_CENTER:
                    _alignmentPos.set(w/2.0,(max->y()-min->y()) /2.0 + descender,0.0);
                    break;
                case CENTER_BOTTOM:
                    _alignmentPos.set(w/2.0,descender,0.0);
                    break;

                case RIGHT_TOP:
                    _alignmentPos.set(w,h + descender,0.0);
                    break;
                case RIGHT_CENTER:
                    _alignmentPos.set(w,(max->y()-min->y()) /2.0 + descender,0.0);
                    break;
                case RIGHT_BOTTOM:
                    _alignmentPos.set(w,descender,0.0);
                    break;
            };
            _alignmentPos=-_alignmentPos;

            *min+=_pos+_alignmentPos;
            *max+=_pos+_alignmentPos;
            break;
    };

    

    switch(_fontType)
    {
        case BITMAP:
            break;
        case PIXMAP:
            break;

    };
#endif
}


void Text::
setAlignment(int alignment)
{
    _alignment=alignment;
    
    if(!_init || !_font->isCreated())
        return;

    initAlignment();
}

void Text::
setBoundingBox(int mode)
{
    _boundingBoxType=mode;
    
    if(!_init || !_font->isCreated())
        return;

    initAlignment();
}

void Text::
setText(const char* text)
{ 
    _text=text; 
    _initAlignment=false;
    _encodedText->setText((const unsigned char*)text);
}

void Text::
setText(const std::string& text)
{ 
    _text=text; 
    _initAlignment=false;
    _encodedText->setText((const unsigned char*)_text.data(),_text.size());
}

void Text::
setText(const wchar_t* text)
{ 
    _encodedText->setOverrideEncoding(EncodedText::ENCODING_UTF8);
    _text=_encodedText->convertWideString(text); 
    _initAlignment=false;
    _encodedText->setText((const unsigned char*)_text.data(),_text.size());
}

// Text
///////////////////////////////////////////////////////////////////////////////
