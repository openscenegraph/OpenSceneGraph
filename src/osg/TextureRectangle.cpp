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

#include <osg/GLExtensions>
#include <osg/TextureRectangle>
#include <osg/State>
#include <osg/GLU>
#include <osg/Notify>

using namespace osg;

TextureRectangle::TextureRectangle():
    _textureWidth(0),
    _textureHeight(0)
{
    setWrap(WRAP_S, CLAMP);
    setWrap(WRAP_T, CLAMP);

    setFilter(MIN_FILTER, LINEAR);
    setFilter(MAG_FILTER, LINEAR);
}

TextureRectangle::TextureRectangle(const TextureRectangle& text,const CopyOp& copyop):
    Texture(text,copyop),
    _image(copyop(text._image.get())),
    _textureWidth(text._textureWidth),
    _textureHeight(text._textureHeight),
    _subloadCallback(text._subloadCallback)
{
}

TextureRectangle::~TextureRectangle()
{
}

int TextureRectangle::compare(const StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Paramter macro's below.
    COMPARE_StateAttribute_Types(TextureRectangle,sa)

    if (_image!=rhs._image) // smart pointer comparison.
    {
        if (_image.valid())
        {
            if (rhs._image.valid())
            {
                int result = _image->compare(*rhs._image);
                if (result!=0) return result;
            }
            else
            {
                return 1; // valid lhs._image is greater than null. 
            }
        }
        else if (rhs._image.valid()) 
        {
            return -1; // valid rhs._image is greater than null. 
        }
    }

    int result = compareTexture(rhs);
    if (result!=0) return result;

    // compare each paramter in turn against the rhs.
    COMPARE_StateAttribute_Parameter(_textureWidth)
    COMPARE_StateAttribute_Parameter(_textureHeight)
    COMPARE_StateAttribute_Parameter(_subloadCallback)

    return 0; // passed all the above comparison macro's, must be equal.
}

void TextureRectangle::setImage(Image* image)
{
    // delete old texture objects.
    dirtyTextureObject();

    _image = image;
}


void TextureRectangle::apply(State& state) const
{
    static bool s_rectangleSupported = isGLExtensionSupported("GL_EXT_texture_rectangle") || isGLExtensionSupported("GL_NV_texture_rectangle");

    if (!s_rectangleSupported)
    {
        notify(WARN)<<"Warning: TextureRectangle::apply(..) failed, texture rectangle is not support by your OpenGL drivers."<<std::endl;
        return;
    }

    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();

    // get the texture object for the current contextID.
    TextureObject* textureObject = getTextureObject(contextID);

    if (textureObject != 0)
    {
        textureObject->bind();
        if (getTextureParameterDirty(state.getContextID()))
            applyTexParameters(GL_TEXTURE_RECTANGLE_NV, state);

        if (_subloadCallback.valid())
            _subloadCallback->subload(*this, state);
    }
    else if (_subloadCallback.valid())
    {
        // we don't have a applyTexImage1D_subload yet so can't reuse.. so just generate a new texture object.        
        _textureObjectBuffer[contextID] = textureObject = getTextureObjectManager()->generateTextureObject(contextID,GL_TEXTURE_RECTANGLE_NV);

        textureObject->bind();

        applyTexParameters(GL_TEXTURE_RECTANGLE_NV, state);

        _subloadCallback->load(*this, state);

        textureObject->setAllocated(1,_internalFormat,_textureWidth,_textureHeight,1,0);

        // in theory the following line is redundant, but in practice
        // have found that the first frame drawn doesn't apply the textures
        // unless a second bind is called?!!
        // perhaps it is the first glBind which is not required...
        //glBindTexture(GL_TEXTURE_RECTANGLE_NV, handle);
    }
    else if (_image.valid() && _image->data())
    {
        // we don't have a applyTexImage1D_subload yet so can't reuse.. so just generate a new texture object.        
        _textureObjectBuffer[contextID] = textureObject = getTextureObjectManager()->generateTextureObject(contextID,GL_TEXTURE_RECTANGLE_NV);

        textureObject->bind();

        applyTexParameters(GL_TEXTURE_RECTANGLE_NV, state);
     
        applyTexImage(GL_TEXTURE_RECTANGLE_NV, _image.get(), state, _textureWidth, _textureHeight);

        textureObject->setAllocated(1,_internalFormat,_textureWidth,_textureHeight,1,0);

        // in theory the following line is redundant, but in practice
        // have found that the first frame drawn doesn't apply the textures
        // unless a second bind is called?!!
        // perhaps it is the first glBind which is not required...
        //glBindTexture(GL_TEXTURE_RECTANGLE_NV, handle);
    }
    else
    {
        glBindTexture(GL_TEXTURE_RECTANGLE_NV, 0);
    }
}

void TextureRectangle::applyTexParameters(GLenum target, State& state) const
{
    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();

    glTexParameteri( target, GL_TEXTURE_WRAP_S, _wrap_s );
    glTexParameteri( target, GL_TEXTURE_WRAP_T, _wrap_t );

    glTexParameteri( target, GL_TEXTURE_MIN_FILTER, _min_filter);
    glTexParameteri( target, GL_TEXTURE_MAG_FILTER, _mag_filter);

    getTextureParameterDirty(contextID) = false;
}

void TextureRectangle::applyTexImage(GLenum target, Image* image, State& state, GLsizei& inwidth, GLsizei& inheight) const
{
    // if we don't have a valid image we can't create a texture!
    if (!image || !image->data())
        return;

    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();

    // update the modified tag to show that it is upto date.
    getModifiedTag(contextID) = image->getModifiedTag();

    // compute the internal texture format, sets _internalFormat.
    computeInternalFormat();

    glPixelStorei(GL_UNPACK_ALIGNMENT, image->getPacking());

    // UH: ignoring compressed for now.
    glTexImage2D(target, 0, _internalFormat,
                 image->s(), image->t(), 0,
                 (GLenum)image->getPixelFormat(),
                 (GLenum)image->getDataType(),
                 image->data());
    
    inwidth = image->s();
    inheight = image->t();
}

void TextureRectangle::computeInternalFormat() const
{
    if (_image.valid())
        computeInternalFormatWithImage(*_image); 
}
