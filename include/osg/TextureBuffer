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

// -*-c++-*-

#ifndef OSG_TEXTUREBUFFEROBJECT
#define OSG_TEXTUREBUFFEROBJECT 1

#include <osg/Texture>
#include <osg/BufferObject>

namespace osg {

/** Encapsulates OpenGL texture buffer functionality in a Texture delegating its content to attached BufferObject
*/
class OSG_EXPORT TextureBuffer : public Texture
{

    public :

        TextureBuffer();

        TextureBuffer(BufferData* image);

        /** Copy constructor using CopyOp to manage deep vs shallow copy. */
        TextureBuffer(const TextureBuffer& text,const CopyOp& copyop=CopyOp::SHALLOW_COPY);

        META_StateAttribute(osg, TextureBuffer, TEXTURE);

        /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
        virtual int compare(const StateAttribute& rhs) const;

        virtual GLenum getTextureTarget() const { return GL_TEXTURE_BUFFER; }

        /** Sets the texture image. */
        void setImage(Image* image);

        /** Gets the texture image. */
        Image* getImage() { return dynamic_cast<Image*>(_bufferData.get() ); }

        /** Gets the const texture image. */
        inline const Image* getImage() const { return dynamic_cast<Image*>(_bufferData.get() ); }

        /** return true if the texture image data has been modified and the associated GL texture object needs to be updated.*/
        virtual bool isDirty(unsigned int contextID) const { return (_bufferData.valid() && _bufferData->getModifiedCount()!=_modifiedCount[contextID]); }

        inline unsigned int & getModifiedCount(unsigned int contextID) const
        {
            // get the modified count for the current contextID.
            return _modifiedCount[contextID];
        }

        /** Sets the texture image, ignoring face. */
        virtual void setImage(unsigned int, Image* image) { setImage(image); }

        /** Gets the texture image, ignoring face. */
        virtual Image* getImage(unsigned int) { return getImage(); }

        /** Gets the const texture image, ignoring face. */
        virtual const Image* getImage(unsigned int) const { return getImage(); }

        /** Gets the number of images that can be assigned to the Texture. */
        virtual unsigned int getNumImages() const { return 1; }


        /** Sets the texture width. If width is zero, calculate the value
          * from the source image width. */
        inline void setTextureWidth(int width) { _textureWidth = width; }

        /** Gets the texture width. */
        virtual int getTextureWidth() const { return _textureWidth; }
        virtual int getTextureHeight() const { return 1; }
        virtual int getTextureDepth() const { return 1; }

        virtual void allocateMipmap(State& /*state*/) const {};

        /** Bind the texture buffer.*/
        virtual void apply(State& state) const;

        /**  Set setBufferData attached */
        void setBufferData(BufferData *bo);

         /**  Set setBufferData attached */
        const BufferData * getBufferData()const {return _bufferData.get();}
    protected :

        virtual ~TextureBuffer();

        virtual void computeInternalFormat() const;

        ref_ptr< BufferData > _bufferData;

        GLsizei _textureWidth;

        typedef buffered_value< unsigned int > BufferDataModifiedCount;
        mutable BufferDataModifiedCount _modifiedCount;


      };

}

#endif
