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
#include <osg/PrimitiveSet>

using namespace osg;

TextureBuffer::TextureBuffer():
    _textureWidth(0),_bo(NULL)
{
}

TextureBuffer::TextureBuffer(osg::Image* image):
    _textureWidth(0),_bo(NULL)
{
    setImage(image);
}

TextureBuffer::TextureBuffer(const TextureBuffer& text,const CopyOp& copyop):
    Texture(text,copyop),
    _textureWidth(text._textureWidth)
{
    setBufferObject(dynamic_cast<BufferObject*>(copyop(text._bo)));
}

TextureBuffer::~TextureBuffer()
{
    _bo=NULL;
}

void TextureBuffer::setBufferObject(BufferObject *bo){
         if (_bo == bo) return;



    if (_bo.valid())
    {  for (unsigned int ibd=0; ibd<_bo->getNumBufferData(); ibd++)

       _bo-> getBufferData(ibd)->removeClient(_bo);
    }

     _bo=bo;

    _modifiedCount.setAllElementsTo(0);

    if (bo)
    {
        for (unsigned int ibd=0; ibd<bo->getNumBufferData(); ibd++)

         bo->getBufferData(ibd)->addClient(bo);
    }

        }
int TextureBuffer::compare(const StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Parameter macros below.
    COMPARE_StateAttribute_Types(TextureBuffer,sa)

    if (getImage()!=rhs.getImage())
    {
        if (getImage() )
        {
            if (rhs.getImage() )
            {
                int result = getImage()->compare(*rhs.getImage());
                if (result!=0) return result;
            }
            else
            {
                return 1; // valid lhs.getImage() is greater than null.
            }
        }
        else if (rhs.getImage())
        {
            return -1; // valid rhs.getImage() is greater than null.
        }
    }

    if (!getImage() && !rhs.getImage())
    {
        int result = compareTextureObjects(rhs);
        if (result!=0) return result;
    }

    int result = compareTexture(rhs);
    if (result!=0) return result;

    // compare each parameter in turn against the rhs.
    COMPARE_StateAttribute_Parameter(_textureWidth)

///compare buffer object profile
    if(_bo.valid()&&rhs._bo.valid())
    {
        if (_bo->getProfile()<rhs._bo->getProfile()) return -1;
        if (rhs._bo->getProfile()<_bo->getProfile()) return 1;
///TODO compare buffer datas

    }

    return 0;
}

void TextureBuffer::setImage(Image* image)
{
    ///set TBO if not created
    if(!_bo.valid() )
    {
        _bo=new  VertexBufferObject();
        _bo->setUsage(GL_STREAM_DRAW_ARB);
        _bo->setTarget(GL_TEXTURE_BUFFER_ARB);

    }

    if (getImage() == image) return;



    if (getImage())
    {
        getImage()->removeClient(    _bo);
    }
    ///delegate
    _bo->setBufferData(0,image);

    _modifiedCount.setAllElementsTo(0);

    if (image)
    {
        image->addClient(   _bo );
    }
}

/** TODO upload image to the bufferobject...but as Texture do that in apply I don't know if it's consistent with the design */
void TextureBuffer::compileGLObjects(State& state) const
{
    unsigned int contextID=state.getContextID();
    TextureBuffer::apply(state);
}
void TextureBuffer::apply(State& state) const
{
#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE)
    const unsigned int contextID = state.getContextID();

    TextureObject* textureObject = getTextureObject(contextID);

    if (textureObject)
    {
        const GLExtensions* extensions = state.get<GLExtensions>();

        unsigned int totalmodified=0;
        for (unsigned int ibd=0; ibd<_bo->getNumBufferData(); ibd++)
            totalmodified+=_bo->getBufferData(ibd)->getModifiedCount();
        if(totalmodified!= getModifiedCount(contextID))
        {
            computeInternalFormat();
            GLBufferObject* glBufferObject = _bo->getOrCreateGLBufferObject(contextID);
            if (glBufferObject)///force buffer upload
            {
                // OSG_NOTICE<<"Compile buffer "<<glBufferObject<<std::endl;
                glBufferObject->compileBuffer();
            }

              extensions->glBindBuffer(GL_TEXTURE_BUFFER_ARB,0);
            _modifiedCount[contextID] = totalmodified;
        }

        textureObject->bind();

        if( getTextureParameterDirty(contextID) )
        {
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
    else if (_bo.valid()&& _bo->getNumBufferData()>0)
    {
unsigned int totalmodified=0;
        for (unsigned int ibd=0; ibd<_bo->getNumBufferData(); ibd++)
            totalmodified+=_bo->getBufferData(ibd)->getModifiedCount();
        const GLExtensions* extensions = state.get<GLExtensions>();

        textureObject = generateAndAssignTextureObject(contextID, GL_TEXTURE_BUFFER_ARB);
        textureObject->bind();

        if (extensions->isBindImageTextureSupported() && _imageAttachment.access!=0)
        {
            extensions->glBindImageTexture(
                _imageAttachment.unit, textureObject->id(), _imageAttachment.level,
                _imageAttachment.layered, _imageAttachment.layer, _imageAttachment.access,
                _imageAttachment.format!=0 ? _imageAttachment.format : _internalFormat);
        }
        getTextureParameterDirty(state.getContextID()) = false;

        computeInternalFormat();


#if 1
        ///try to compute textureWidth if not set by user
        /// (seams dirty and useless textureWidth is not used annywhere)
        ///check for downcast for getTotalDataSize/datasize  ( kind of dirty just toretrieve datasize )
        if(_textureWidth==0)
            for (unsigned int ibd=0; ibd<_bo->getNumBufferData(); ibd++)
            {
                osg::BufferData *bd=_bo->getBufferData(ibd);

                osg::Array* arr;
                osg::Image* im;
                osg::PrimitiveSet*pr;
                if(arr=bd->asArray())     _textureWidth+=  arr->asArray()->getNumElements()  ;
                else if (im=bd->asImage())    _textureWidth+= im->s()  ;
                else if(pr=bd->asPrimitiveSet())   _textureWidth+=pr->getNumPrimitives();
                else
                {
                    OSG_NOTIFY(WARN)<<"Warning: osg::TextureBuffer: you're trying to bind a not handle BufferData Type , assuming 32bit vec4 container";
                    _textureWidth+=bd->getTotalDataSize()/16;
                }
            }
#endif

/// now compile tbo if required
        GLBufferObject* glBufferObject = _bo->getOrCreateGLBufferObject(contextID);
        if (glBufferObject )
        {
            if( glBufferObject->isDirty())
                   glBufferObject->compileBuffer();
            else      glBufferObject->bindBuffer();

            textureObject->setAllocated(true);
            //  extensions->glBindBuffer(GL_TEXTURE_BUFFER_ARB,0);

            textureObject->bind();
            extensions->glTexBuffer(GL_TEXTURE_BUFFER_ARB, _internalFormat, glBufferObject->getGLObjectID());
        }

        ///assuming the same modified count as the first buff data
        _modifiedCount[contextID] =totalmodified;// _bo->getBufferData(0)->getModifiedCount();
    }
    else
    {
        glBindTexture(GL_TEXTURE_BUFFER_ARB, 0);
    }

#else
    OSG_NOTICE<<"Warning: TextureBuffer::apply(State& state) not supported."<<std::endl;
#endif
}


void TextureBuffer::computeInternalFormat() const
{
    if (getImage() ) computeInternalFormatWithImage(*getImage());
    else computeInternalFormatType();
}

