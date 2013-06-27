/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2013 Robert Osfield
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
#include <osg/TextureBuffer>
#include <osg/State>

using namespace osg;

TextureBuffer::TextureBuffer():
            _textureWidth(0), _usageHint(GL_STREAM_DRAW)
{
}

TextureBuffer::TextureBuffer(osg::Image* image):
            _textureWidth(0), _usageHint(GL_STREAM_DRAW)
{
    setImage(image);
}

TextureBuffer::TextureBuffer(const TextureBuffer& text,const CopyOp& copyop):
            Texture(text,copyop),
            _textureWidth(text._textureWidth),
            _usageHint(text._usageHint)
{
    setImage(copyop(text._image.get()));
}

TextureBuffer::~TextureBuffer()
{
    setImage(NULL);
}

int TextureBuffer::compare(const StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Parameter macros below.
    COMPARE_StateAttribute_Types(TextureBuffer,sa)

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

    if (!_image && !rhs._image)
    {
        int result = compareTextureObjects(rhs);
        if (result!=0) return result;
    }

    int result = compareTexture(rhs);
    if (result!=0) return result;

    // compare each parameter in turn against the rhs.
    COMPARE_StateAttribute_Parameter(_textureWidth)
    COMPARE_StateAttribute_Parameter(_usageHint)

    return 0;
}

void TextureBuffer::setImage(Image* image)
{
    if (_image == image) return;

    if (_image.valid())
    {
        _image->removeClient(this);
    }

    _image = image;
    _modifiedCount.setAllElementsTo(0);

    if (_image.valid())
    {
        _image->addClient(this);
    }
}

void TextureBuffer::apply(State& state) const
{
#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE)
    const unsigned int contextID = state.getContextID();

    TextureObject* textureObject = getTextureObject(contextID);
    TextureBufferObject* textureBufferObject = _textureBufferObjects[contextID].get();
    

    if (textureObject)
    {
        if (_image.valid() && getModifiedCount(contextID) != _image->getModifiedCount())
        {
            computeInternalFormat();
            textureBufferObject->bindBuffer(GL_TEXTURE_BUFFER_ARB);
            textureBufferObject->bufferSubData(_image.get() );
            textureBufferObject->unbindBuffer(GL_TEXTURE_BUFFER_ARB);
            _modifiedCount[contextID] = _image->getModifiedCount();
        }
        textureObject->bind();        
        
        if( getTextureParameterDirty(contextID) )
        {
            const Extensions* extensions = Texture::getExtensions(contextID,true);
            if (extensions->isBindImageTextureSupported() && _imageAttachment.access!=0)
            {
                 extensions->glBindImageTexture(
                     _imageAttachment.unit, textureObject->id(), _imageAttachment.level,
                     _imageAttachment.layered, _imageAttachment.layer, _imageAttachment.access,
                     _imageAttachment.format!=0 ? _imageAttachment.format : _internalFormat);
            }
            getTextureParameterDirty(state.getContextID()) = false;
        }
    }
    else if (_image.valid() && _image->data())
    {
        textureObject = generateTextureObject(this, contextID,GL_TEXTURE_BUFFER_ARB);
        _textureObjectBuffer[contextID] = textureObject;
        textureObject->bind();
        
        textureBufferObject = new TextureBufferObject(contextID,_usageHint);
        _textureBufferObjects[contextID] = textureBufferObject;
        
        const Extensions* extensions = Texture::getExtensions(contextID,true);
        if (extensions->isBindImageTextureSupported() && _imageAttachment.access!=0)
        {
                extensions->glBindImageTexture(
                    _imageAttachment.unit, textureObject->id(), _imageAttachment.level,
                    _imageAttachment.layered, _imageAttachment.layer, _imageAttachment.access,
                    _imageAttachment.format!=0 ? _imageAttachment.format : _internalFormat);
        }
        getTextureParameterDirty(state.getContextID()) = false;
        
        computeInternalFormat();
        _textureWidth = _image->s();
        textureBufferObject->bindBuffer(GL_TEXTURE_BUFFER_ARB);
        textureBufferObject->bufferData( _image.get() );
        textureObject->setAllocated(true);
        textureBufferObject->unbindBuffer(GL_TEXTURE_BUFFER_ARB);
        
        textureObject->bind();
        textureBufferObject->texBuffer(_internalFormat);
               
        _modifiedCount[contextID] = _image->getModifiedCount();
    }
    else
    {
        glBindTexture(GL_TEXTURE_BUFFER_ARB, 0);
    }

#else
    OSG_NOTICE<<"Warning: TextureBuffer::apply(State& state) not supported."<<std::endl;
#endif
}

void TextureBuffer::bindBufferAs( unsigned int contextID, GLuint target )
{
    TextureBufferObject* textureBufferObject = _textureBufferObjects[contextID].get();    
    textureBufferObject->bindBuffer(target);
    
}

void TextureBuffer::unbindBufferAs( unsigned int contextID, GLuint target )
{
    TextureBufferObject* textureBufferObject = _textureBufferObjects[contextID].get();    
    textureBufferObject->unbindBuffer(target);
}


void TextureBuffer::computeInternalFormat() const
{
    if (_image.valid()) computeInternalFormatWithImage(*_image);
    else computeInternalFormatType();
}

void TextureBuffer::TextureBufferObject::bindBuffer(GLenum target)
{
    if (_id == 0)
        _extensions->glGenBuffers(1, &_id);
    _extensions->glBindBuffer(target, _id);
}

void TextureBuffer::TextureBufferObject::unbindBuffer(GLenum target)
{
    _extensions->glBindBuffer(target, 0);
}

void TextureBuffer::TextureBufferObject::texBuffer(GLenum internalFormat)
{
    _extensions->glTexBuffer(GL_TEXTURE_BUFFER_ARB, internalFormat, _id);
}

void TextureBuffer::TextureBufferObject::bufferData( osg::Image* image )
{
    _extensions->glBufferData(GL_TEXTURE_BUFFER_ARB, image->getTotalDataSize(), image->data(), _usageHint);
}

void TextureBuffer::TextureBufferObject::bufferSubData( osg::Image* image )
{
    _extensions->glBufferSubData(GL_TEXTURE_BUFFER_ARB, 0, image->getTotalDataSize(), image->data());
}
