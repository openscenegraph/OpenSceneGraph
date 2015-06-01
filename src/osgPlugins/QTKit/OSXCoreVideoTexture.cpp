/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2008 Robert Osfield
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


#include "OSXCoreVideoTexture.h"



OSXCoreVideoTexture::OSXCoreVideoTexture()
    : osg::Texture()
    , _textureTarget(GL_TEXTURE_RECTANGLE_EXT)
    , _inited(false)
    , _adapter(NULL)
{
}


OSXCoreVideoTexture::OSXCoreVideoTexture(osg::Image* image): 
    osg::Texture(),
    _textureTarget(GL_TEXTURE_RECTANGLE_EXT),
    _inited(false),
    _adapter(NULL)
{
    setImage(image);
}
    

OSXCoreVideoTexture::OSXCoreVideoTexture(const OSXCoreVideoTexture& text,const osg::CopyOp& copyop) : 
    osg::Texture(text, copyop),  
    _textureTarget(text._textureTarget),
    _inited(text._inited),
    _adapter(text._adapter),
    _image(text._image)
{
}


OSXCoreVideoTexture::~OSXCoreVideoTexture() {
}


int OSXCoreVideoTexture::compare(const osg::StateAttribute& sa) const {
    COMPARE_StateAttribute_Types(OSXCoreVideoTexture,sa)

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
        // no image attached to either Texture2D
        // but could these textures already be downloaded?
        // check the _textureObjectBuffer to see if they have been
        // downloaded

        int result = compareTextureObjects(rhs);
        if (result!=0) return result;
    }

    int result = compareTexture(rhs);
    if (result!=0) return result;

    // compare each parameter in turn against the rhs.
#if 1    
    if (_textureWidth != 0 && rhs._textureWidth != 0)
    {
        COMPARE_StateAttribute_Parameter(_textureWidth)
    }
    if (_textureHeight != 0 && rhs._textureHeight != 0)
    {
        COMPARE_StateAttribute_Parameter(_textureHeight)
    }
#endif
    return 0; // passed all the above comparison macro's, must be equal.
}



void OSXCoreVideoTexture::setImage(osg::Image* image)
{
    if (_image == image) return;

    if (_image.valid() && _image->requiresUpdateCall())
    {
        setUpdateCallback(0);
        setDataVariance(osg::Object::STATIC);
    }

    _image = image;
    _modifiedCount.setAllElementsTo(0);

    if (_image.valid() && _image->requiresUpdateCall())
    {
        setUpdateCallback(new osg::Image::UpdateCallback());
        setDataVariance(osg::Object::DYNAMIC);
    }
    _adapter = NULL;
}





void OSXCoreVideoTexture::apply(osg::State& state) const {
    if (!_image.valid())
        return;
    
    if (!_adapter.valid()) {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        if (!_adapter.valid()) {
            OSXQTKitVideo* m = dynamic_cast<OSXQTKitVideo*>(_image.get());
            if ((m) && (m->getCoreVideoAdapter()))
                _adapter = m->getCoreVideoAdapter();
            else 
                _adapter = new OSXCoreVideoAdapter(state, _image.get());
        }
    }
    
    _adapter->getFrame();
    _textureTarget = _adapter->getTextureTarget();
    glBindTexture(_textureTarget, _adapter->getTextureName());
}


