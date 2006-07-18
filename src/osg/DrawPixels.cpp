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
#include <osg/DrawPixels>

using namespace osg;

DrawPixels::DrawPixels()
{
    // turn off display lists right now, just incase we want to modify the projection matrix along the way.
    setSupportsDisplayList(false);
    
    _position.set(0.0f,0.0f,0.0f);
    
    _useSubImage = false;
    _offsetX = 0;
    _offsetY = 0; 
    _width = 0;
    _height = 0;
}

DrawPixels::DrawPixels(const DrawPixels& drawimage,const CopyOp& copyop):
    Drawable(drawimage,copyop),
    _position(drawimage._position),
    _image(drawimage._image),
    _useSubImage(drawimage._useSubImage),
    _offsetX(drawimage._offsetX), 
    _offsetY(drawimage._offsetY),
    _width(drawimage._width),
    _height(drawimage._height)
{
}

DrawPixels::~DrawPixels()
{
    // image will delete itself thanks to ref_ptr :-)
}

void DrawPixels::setPosition(const osg::Vec3& position)
{
    _position = position;
    dirtyBound();
}

void DrawPixels::setSubImageDimensions(unsigned int offsetX,unsigned int offsetY,unsigned int width,unsigned int height)
{
    _useSubImage = true;
    _offsetX = offsetX;
    _offsetY = offsetY; 
    _width = width;
    _height = height;
}

void DrawPixels::getSubImageDimensions(unsigned int& offsetX,unsigned int& offsetY,unsigned int& width,unsigned int& height) const
{
    offsetX = _offsetX;
    offsetY = _offsetY; 
    width = _width;
    height = _height;
}


BoundingBox DrawPixels::computeBound() const
{
    // really needs to be dependant of view poistion and projection... will implement simple version right now.
    BoundingBox bbox;
    float diagonal = 0.0f;
    if (_useSubImage)
    {
        diagonal = sqrtf(_width*_width+_height*_height);
    }
    else
    {
        diagonal = sqrtf(_image->s()*_image->s()+_image->t()*_image->t());
    }
    
    bbox.expandBy(_position-osg::Vec3(diagonal,diagonal,diagonal));
    bbox.expandBy(_position+osg::Vec3(diagonal,diagonal,diagonal));
    return bbox;
}

void DrawPixels::drawImplementation(State&) const
{
    glRasterPos3f(_position.x(),_position.y(),_position.z());

    if (_useSubImage)
    {
        const GLvoid* pixels = _image->data(_offsetX,_offsetY);
        glPixelStorei(GL_UNPACK_ROW_LENGTH,_image->s());
        glDrawPixels(_width,_height,
                     (GLenum)_image->getPixelFormat(),
                     (GLenum)_image->getDataType(),
                     pixels);
        glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
    }
    else
    {
        glDrawPixels(_image->s(), _image->t(),
                     (GLenum)_image->getPixelFormat(),
                     (GLenum)_image->getDataType(),
                     _image->data() );
    }
}

