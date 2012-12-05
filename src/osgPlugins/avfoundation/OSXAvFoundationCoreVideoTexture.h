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

#pragma once


#include <osg/Texture>


class OSXAVFoundationCoreVideoTexture : public osg::Texture {

    public:
        
        OSXAVFoundationCoreVideoTexture();
        
        OSXAVFoundationCoreVideoTexture(osg::Image* image);
        
        OSXAVFoundationCoreVideoTexture(const OSXAVFoundationCoreVideoTexture& text,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
        
        META_StateAttribute( , OSXAVFoundationCoreVideoTexture, TEXTURE);

        virtual int compare(const osg::StateAttribute& rhs) const;

        virtual GLenum getTextureTarget() const { return _textureTarget; }

        
        virtual void setImage(unsigned int, osg::Image* image) { setImage(image); }
        
        void setImage(osg::Image* image);
        
        osg::Image* getImage() { return _image.get(); }
        const osg::Image* getImage() const { return _image.get(); }
        
        virtual osg::Image* getImage(unsigned int) { return _image.get(); }

        virtual const osg::Image* getImage(unsigned int) const { return _image.get(); }

        virtual unsigned int getNumImages() const { return 1; }

        virtual int getTextureWidth() const { return _textureWidth; }
        virtual int getTextureHeight() const { return _textureHeight; }
        virtual int getTextureDepth() const { return 1; }
        

        virtual void apply(osg::State& state) const;
        
        virtual void allocateMipmap(osg::State& state) const {}

        inline unsigned int& getModifiedCount(unsigned int contextID) const
        {
            return _modifiedCount[contextID];
        }
        
    protected:
        virtual void computeInternalFormat() const  {}
        virtual ~OSXAVFoundationCoreVideoTexture();
        
        mutable GLenum    _textureTarget;
        mutable int        _textureWidth;
        mutable int        _textureHeight;
        bool            _inited;
        osg::ref_ptr<osg::Image>    _image;
        
        typedef osg::buffered_value<unsigned int> ImageModifiedCount;
        mutable ImageModifiedCount _modifiedCount;

};
