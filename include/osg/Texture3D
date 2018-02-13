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

#ifndef OSG_TEXTURE3D
#define OSG_TEXTURE3D 1

#include <osg/Texture>

namespace osg {

/** Encapsulates OpenGL 3D texture functionality. Doesn't support cube maps,
  * so ignore \a face parameters.
*/
class OSG_EXPORT Texture3D : public Texture
{

    public :

        Texture3D();

        Texture3D(Image* image);


        template<class T> Texture3D(const osg::ref_ptr<T>& image):
                    _textureWidth(0),
                    _textureHeight(0),
                    _textureDepth(0),
                    _numMipmapLevels(0)
        {
            setImage(image.get());
        }

        /** Copy constructor using CopyOp to manage deep vs shallow copy. */
        Texture3D(const Texture3D& text,const CopyOp& copyop=CopyOp::SHALLOW_COPY);

        META_StateAttribute(osg, Texture3D,TEXTURE);

        /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
        virtual int compare(const StateAttribute& rhs) const;

        virtual GLenum getTextureTarget() const { return GL_TEXTURE_3D; }

        /** Sets the texture image. */
        void setImage(Image* image);

        template<class T> void setImage(const ref_ptr<T>& image) { setImage(image.get()); }

        /** Gets the texture image. */
        Image* getImage() { return _image.get(); }

        /** Gets the const texture image. */
        inline const Image* getImage() const { return _image.get(); }

        /** return true if the texture image data has been modified and the associated GL texture object needs to be updated.*/
        virtual bool isDirty(unsigned int contextID) const { return (_image.valid() && _image->getModifiedCount()!=_modifiedCount[contextID]); }

        inline unsigned int& getModifiedCount(unsigned int contextID) const
        {
            // get the modified count for the current contextID.
            return _modifiedCount[contextID];
        }

        /** Sets the texture image, ignoring face. */
        virtual void setImage(unsigned int, Image* image) { setImage(image); }

        /** Gets the texture image, ignoring face. */
        virtual Image* getImage(unsigned int) { return _image.get(); }

        /** Gets the const texture image, ignoring face. */
        virtual const Image* getImage(unsigned int) const { return _image.get(); }

        /** Gets the number of images that can be assigned to the Texture. */
        virtual unsigned int getNumImages() const { return 1; }


        /** Sets the texture width, height, and depth. If width, height, or
          * depth are zero, calculate the respective value from the source
          * image size. */
        inline void setTextureSize(int width, int height, int depth) const
        {
            _textureWidth = width;
            _textureHeight = height;
            _textureDepth = depth;
        }

        /** Gets the texture subload width. */
        inline void getTextureSize(int& width, int& height, int& depth) const
        {
            width = _textureWidth;
            height = _textureHeight;
            depth = _textureDepth;
        }

        void setTextureWidth(int width) { _textureWidth=width; }
        void setTextureHeight(int height) { _textureHeight=height; }
        void setTextureDepth(int depth) { _textureDepth=depth; }

        virtual int getTextureWidth() const { return _textureWidth; }
        virtual int getTextureHeight() const { return _textureHeight; }
        virtual int getTextureDepth() const { return _textureDepth; }


        class OSG_EXPORT SubloadCallback : public Referenced
        {
            public:
                virtual void load(const Texture3D& texture,State& state) const = 0;
                virtual void subload(const Texture3D& texture,State& state) const = 0;
        };

        void setSubloadCallback(SubloadCallback* cb) { _subloadCallback = cb;; }

        SubloadCallback* getSubloadCallback() { return _subloadCallback.get(); }

        const SubloadCallback* getSubloadCallback() const { return _subloadCallback.get(); }


        /** Helper function. Sets the number of mipmap levels created for this
          * texture. Should only be called within an osg::Texture::apply(), or
          * during a custom OpenGL texture load. */
        void setNumMipmapLevels(unsigned int num) const { _numMipmapLevels=num; }

        /** Gets the number of mipmap levels created. */
        unsigned int getNumMipmapLevels() const { return _numMipmapLevels; }


        /** Copies a two-dimensional texture subimage, as per
          * glCopyTexSubImage3D. Updates a portion of an existing OpenGL
          * texture object from the current OpenGL background framebuffer
          * contents at position \a x, \a y with width \a width and height
          * \a height. Loads framebuffer data into the texture using offsets
          * \a xoffset, \a yoffset, and \a zoffset. \a width and \a height
          * must be powers of two. */
        void copyTexSubImage3D(State& state, int xoffset, int yoffset, int zoffset, int x, int y, int width, int height);


        /** Bind the texture object. If the texture object hasn't already been
          * compiled, create the texture mipmap levels. */
        virtual void apply(State& state) const;

    protected :

        virtual ~Texture3D();

        void computeRequiredTextureDimensions(State& state, const osg::Image& image,GLsizei& width, GLsizei& height,GLsizei& depth, GLsizei& numMipmapLevels) const;

        virtual void computeInternalFormat() const;
        void allocateMipmap(State& state) const;

        void applyTexImage3D(GLenum target, Image* image, State& state, GLsizei& inwidth, GLsizei& inheight, GLsizei& indepth, GLsizei& numMipmapLevels) const;

        /** It's not ideal that _image is mutable, but it's required since
          * Image::ensureDimensionsArePowerOfTwo() can only be called in a
          * valid OpenGL context, and therefore within Texture::apply, which
          * is const. */
        mutable ref_ptr<Image> _image;

        /** Subloaded images can have different texture and image sizes. */
        mutable GLsizei _textureWidth, _textureHeight, _textureDepth;

        /** Number of mip map levels the texture has been created with, */
        mutable GLsizei _numMipmapLevels;

        ref_ptr<SubloadCallback> _subloadCallback;

        typedef buffered_value<unsigned int> ImageModifiedCount;
        mutable ImageModifiedCount _modifiedCount;

};

}

#endif
