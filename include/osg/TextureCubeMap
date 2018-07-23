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

#ifndef OSG_TEXTURECUBEMAP
#define OSG_TEXTURECUBEMAP 1

#include <osg/Texture>


namespace osg {

/** TextureCubeMap state class which encapsulates OpenGL texture cubemap functionality. */
class OSG_EXPORT TextureCubeMap : public Texture
{

    public :

        TextureCubeMap();

        /** Copy constructor using CopyOp to manage deep vs shallow copy. */
        TextureCubeMap(const TextureCubeMap& cm,const CopyOp& copyop=CopyOp::SHALLOW_COPY);

        META_StateAttribute(osg, TextureCubeMap,TEXTURE);

        /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
        virtual int compare(const StateAttribute& rhs) const;

        virtual GLenum getTextureTarget() const { return GL_TEXTURE_CUBE_MAP; }

        enum Face {
            POSITIVE_X=0,
            NEGATIVE_X=1,
            POSITIVE_Y=2,
            NEGATIVE_Y=3,
            POSITIVE_Z=4,
            NEGATIVE_Z=5
        };

        /** Set the texture image for specified face. */
        virtual void setImage(unsigned int face, Image* image);

        template<class T> void setImage(unsigned int face, const ref_ptr<T>& image) { setImage(face, image.get()); }

        /** Get the texture image for specified face. */
        virtual Image* getImage(unsigned int face);

        /** Get the const texture image for specified face. */
        virtual const Image* getImage(unsigned int face) const;

        /** Get the number of images that can be assigned to the Texture. */
        virtual unsigned int getNumImages() const { return 6; }


        /** return true if the texture image data has been modified and the associated GL texture object needs to be updated.*/
        virtual bool isDirty(unsigned int contextID) const
        {
            return  (_images[0].valid() && _images[0]->getModifiedCount()!=_modifiedCount[0][contextID]) ||
                    (_images[1].valid() && _images[1]->getModifiedCount()!=_modifiedCount[1][contextID]) ||
                    (_images[2].valid() && _images[2]->getModifiedCount()!=_modifiedCount[2][contextID]) ||
                    (_images[3].valid() && _images[3]->getModifiedCount()!=_modifiedCount[3][contextID]) ||
                    (_images[4].valid() && _images[4]->getModifiedCount()!=_modifiedCount[4][contextID]) ||
                    (_images[5].valid() && _images[5]->getModifiedCount()!=_modifiedCount[5][contextID]);
        }

        inline unsigned int& getModifiedCount(unsigned int face,unsigned int contextID) const
        {
            // get the modified count for the current contextID.
            return _modifiedCount[face][contextID];
        }

        /** Set the texture width and height. If width or height are zero then
          * the respective size value is calculated from the source image sizes.
        */
        inline void setTextureSize(int width, int height) const
        {
            _textureWidth = width;
            _textureHeight = height;
        }

        void setTextureWidth(int width) { _textureWidth=width; }
        void setTextureHeight(int height) { _textureHeight=height; }

        virtual int getTextureWidth() const { return _textureWidth; }
        virtual int getTextureHeight() const { return _textureHeight; }
        virtual int getTextureDepth() const { return 1; }

        class OSG_EXPORT SubloadCallback : public Referenced
        {
            public:
                virtual void load(const TextureCubeMap& texture,State& state) const = 0;
                virtual void subload(const TextureCubeMap& texture,State& state) const = 0;
        };

        void setSubloadCallback(SubloadCallback* cb) { _subloadCallback = cb;; }

        SubloadCallback* getSubloadCallback() { return _subloadCallback.get(); }

        const SubloadCallback* getSubloadCallback() const { return _subloadCallback.get(); }


        /** Set the number of mip map levels the texture has been created with.
          * Should only be called within an osg::Texuture::apply() and custom OpenGL texture load.
        */
        void setNumMipmapLevels(unsigned int num) const { _numMipmapLevels=num; }

        /** Get the number of mip map levels the texture has been created with. */
        unsigned int getNumMipmapLevels() const { return _numMipmapLevels; }

        /** Copies a two-dimensional texture subimage, as per
          * glCopyTexSubImage2D. Updates a portion of an existing OpenGL
          * texture object from the current OpenGL background framebuffer
          * contents at position \a x, \a y with width \a width and height
          * \a height. Loads framebuffer data into the texture using offsets
          * \a xoffset and \a yoffset. \a width and \a height must be powers
          * of two. */
        void copyTexSubImageCubeMap(State& state, int face, int xoffset, int yoffset, int x, int y, int width, int height );


        /** On first apply (unless already compiled), create the mipmapped
          * texture and bind it. Subsequent apply will simple bind to texture.
        */
        virtual void apply(State& state) const;

    protected :

        virtual ~TextureCubeMap();

        bool imagesValid() const;

        virtual void computeInternalFormat() const;
        void allocateMipmap(State& state) const;

        ref_ptr<Image> _images[6];

        // subloaded images can have different texture and image sizes.
        mutable GLsizei _textureWidth, _textureHeight;

        // number of mip map levels the texture has been created with,
        mutable GLsizei _numMipmapLevels;

        ref_ptr<SubloadCallback> _subloadCallback;

        typedef buffered_value<unsigned int> ImageModifiedCount;
        mutable ImageModifiedCount _modifiedCount[6];
};

}

#endif
