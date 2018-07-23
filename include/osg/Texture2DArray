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

#ifndef OSG_TEXTURE2DARRAY
#define OSG_TEXTURE2DARRAY 1

#include <osg/Texture>
#include <list>

namespace osg {

/** Texture2DArray state class which encapsulates OpenGL 2D array texture functionality.
 * Texture arrays were introduced with Shader Model 4.0 hardware.
 *
 * A 2D texture array does contain textures sharing the same properties (e.g. size, bitdepth,...)
 * in a layered structure. See http://www.opengl.org/registry/specs/EXT/texture_array.txt for more info.
 */
class OSG_EXPORT Texture2DArray : public Texture
{

    public :

        Texture2DArray();

        /** Copy constructor using CopyOp to manage deep vs shallow copy. */
        Texture2DArray(const Texture2DArray& cm,const CopyOp& copyop=CopyOp::SHALLOW_COPY);

        META_StateAttribute(osg, Texture2DArray, TEXTURE);

        /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
        virtual int compare(const StateAttribute& rhs) const;

        virtual GLenum getTextureTarget() const { return GL_TEXTURE_2D_ARRAY; }

        /** Texture2DArray is related to non fixed pipeline usage only so isn't appropriate to enable/disable.*/
        virtual bool getModeUsage(StateAttribute::ModeUsage&) const { return false; }

        /** Set the texture image for specified layer. */
        virtual void setImage(unsigned int layer, Image* image);

        template<class T> void setImage(unsigned int layer, const ref_ptr<T>& image) { setImage(layer, image.get()); }

        /** Get the texture image for specified layer. */
        virtual Image* getImage(unsigned int layer);

        /** Get the const texture image for specified layer. */
        virtual const Image* getImage(unsigned int layer) const;

        /** Get the number of images that are assigned to the Texture.
         * The number is equal to the texture depth. To get the maximum possible
         * image/layer count, you have to use the extension subclass, since it provides
         * graphic context dependent information.
         */
        virtual unsigned int getNumImages() const { return _images.size(); }

        /** return true if the texture image data has been modified and the associated GL texture object needs to be updated.*/
        virtual bool isDirty(unsigned int contextID) const
        {
            for(unsigned int i=0; i<_images.size(); ++i)
            {
                if (_images[i].valid() && _images[i]->getModifiedCount()!=_modifiedCount[i][contextID]) return true;
            }
            return false;
        }

        /** Check how often was a certain layer in the given context modified */
        inline unsigned int& getModifiedCount(unsigned int layer, unsigned int contextID) const
        {
            // get the modified count for the current contextID.
            return _modifiedCount[layer][contextID];
        }

        /** Set the texture width and height. If width or height are zero then
          * the respective size value is calculated from the source image sizes.
          * Depth parameter specifies the number of layers to be used.
        */
        void setTextureSize(int width, int height, int depth);

        void setTextureWidth(int width) { _textureWidth=width; }
        void setTextureHeight(int height) { _textureHeight=height; }
        void setTextureDepth(int depth);

        virtual int getTextureWidth() const { return _textureWidth; }
        virtual int getTextureHeight() const { return _textureHeight; }
        virtual int getTextureDepth() const { return _textureDepth; }

        class OSG_EXPORT SubloadCallback : public Referenced
        {
            public:
                virtual void load(const Texture2DArray& texture,State& state) const = 0;
                virtual void subload(const Texture2DArray& texture,State& state) const = 0;
        };


        void setSubloadCallback(SubloadCallback* cb) { _subloadCallback = cb;; }

        SubloadCallback* getSubloadCallback() { return _subloadCallback.get(); }

        const SubloadCallback* getSubloadCallback() const { return _subloadCallback.get(); }



        /** Set the number of mip map levels the texture has been created with.
          * Should only be called within an osg::Texture::apply() and custom OpenGL texture load.
        */
        void setNumMipmapLevels(unsigned int num) const { _numMipmapLevels=num; }

        /** Get the number of mip map levels the texture has been created with. */
        unsigned int getNumMipmapLevels() const { return _numMipmapLevels; }

        /** Copies a two-dimensional texture subimage, as per
          * glCopyTexSubImage3D. Updates a portion of an existing OpenGL
          * texture object from the current OpenGL background framebuffer
          * contents at position \a x, \a y with width \a width and height
          * \a height. Loads framebuffer data into the texture using offsets
          * \a xoffset and \a yoffset. \a zoffset specifies the layer of the texture
          * array to which the result is copied.
          */
        void copyTexSubImage2DArray(State& state, int xoffset, int yoffset, int zoffset, int x, int y, int width, int height );

        /** Bind the texture if already compiled. Otherwise recompile.
        */
        virtual void apply(State& state) const;

    protected :

        virtual ~Texture2DArray();

        bool imagesValid() const;

        virtual void computeInternalFormat() const;
        GLsizei computeTextureDepth() const;
        void allocateMipmap(State& state) const;

        void applyTexImage2DArray_subload(State& state, Image* image, GLsizei layer, GLsizei inwidth, GLsizei inheight, GLsizei indepth, GLint inInternalFormat, GLsizei& numMipmapLevels) const;

        typedef std::vector< ref_ptr<Image> > Images;
        Images _images;

        // subloaded images can have different texture and image sizes.
        mutable GLsizei _textureWidth, _textureHeight, _textureDepth;

        // number of mip map levels the texture has been created with,
        mutable GLsizei _numMipmapLevels;

        ref_ptr<SubloadCallback> _subloadCallback;

        typedef buffered_value<unsigned int> ImageModifiedCount;
        mutable std::vector<ImageModifiedCount> _modifiedCount;
};

}

#endif
