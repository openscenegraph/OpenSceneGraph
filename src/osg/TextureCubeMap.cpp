#if defined(_MSC_VER)
	#pragma warning( disable : 4786 )
#endif

#include <osg/ref_ptr>
#include <osg/Image>
#include <osg/State>
#include <osg/Texture>
#include <osg/TextureCubeMap>
#include <osg/GLExtensions>

#include <osg/GLU>


using namespace osg;


#ifndef GL_ARB_texture_cube_map
#define GL_ARB_texture_cube_map 1
//#define GL_NORMAL_MAP_ARB                   0x8511    // defined in TexGen
//#define GL_REFLECTION_MAP_ARB               0x8512    //    --- '' ---
#define GL_TEXTURE_CUBE_MAP_ARB             0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP_ARB     0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB  0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB  0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB  0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB  0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB  0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB  0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP_ARB       0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB    0x851C
#endif


#ifndef GL_EXT_texture_cube_map
#define GL_EXT_texture_cube_map 1
//#define GL_NORMAL_MAP_EXT                   0x8511
//#define GL_REFLECTION_MAP_EXT               0x8512
#define GL_TEXTURE_CUBE_MAP_EXT             0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP_EXT     0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT  0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT  0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT  0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT  0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT  0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT  0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP_EXT       0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT    0x851C
#endif


#ifdef GL_ARB_texture_cube_map
#  define CUBE_MAP_POSITIVE_X   GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB
#  define CUBE_MAP_NEGATIVE_X   GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB
#  define CUBE_MAP_POSITIVE_Y   GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB
#  define CUBE_MAP_NEGATIVE_Y   GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB
#  define CUBE_MAP_POSITIVE_Z   GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB
#  define CUBE_MAP_NEGATIVE_Z   GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB
#elif GL_EXT_texture_cube_map
#  define CUBE_MAP_POSITIVE_X   GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT
#  define CUBE_MAP_NEGATIVE_X   GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT
#  define CUBE_MAP_POSITIVE_Y   GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT
#  define CUBE_MAP_NEGATIVE_Y   GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT
#  define CUBE_MAP_POSITIVE_Z   GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT
#  define CUBE_MAP_NEGATIVE_Z   GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT
#endif


# if GL_EXT_texture_cube_map || GL_ARB_texture_cube_map
static GLenum faceTarget[6] =
{
    CUBE_MAP_POSITIVE_X,
    CUBE_MAP_NEGATIVE_X,
    CUBE_MAP_POSITIVE_Y,
    CUBE_MAP_NEGATIVE_Y,
    CUBE_MAP_POSITIVE_Z,
    CUBE_MAP_NEGATIVE_Z
};
#endif


TextureCubeMap::TextureCubeMap()
{
    _target = GL_TEXTURE_CUBE_MAP_ARB;      // default to ARB extension
}


TextureCubeMap::~TextureCubeMap()
{
}


int TextureCubeMap::compare(const StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Paramter macro's below.
    COMPARE_StateAttribute_Types(TextureCubeMap,sa)

    for (int n=0; n<6; n++)
    {
        if (_images[n]!=rhs._images[n]) // smart pointer comparison.
        {
            if (_images[n].valid())
            {
                if (rhs._images[n].valid())
                {
                    if (_images[n]->getFileName()<rhs._images[n]->getFileName()) return -1;
                    else if (_images[n]->getFileName()>rhs._images[n]->getFileName()) return 1;;
                }
                else
                {
                    return 1; // valid lhs._image is greater than null. 
                }
            }
            else if (rhs._images[n].valid()) 
            {
                return -1; // valid rhs._image is greater than null. 
            }
        }
    }

    return 0; // passed all the above comparison macro's, must be equal.
}


void TextureCubeMap::setImage( const Face face, Image* image)
{
    // Quick and dirty implementation committed by ABJ.
    if (face == 0)
    {
        // delete old texture objects.
        for(TextureNameList::iterator itr=_handleList.begin();
                                   itr!=_handleList.end();
                                   ++itr)
        {
            if (*itr != 0)
            {
                // contact global texture object handler to delete texture objects
                // in appropriate context.
                // glDeleteTextures( 1L, (const GLuint *)itr );
                *itr = 0;
            }
        }
    }
    _images[face] = image;
}


bool TextureCubeMap::imagesValid() const
{
    for (int n=0; n<6; n++)
    {
        if (!_images[n].valid() || !_images[n]->data())
            return false;
    }
    return true;
}


void TextureCubeMap::apply(State& state) const
{
    static bool s_ARB_CubeMapSupported = isGLExtensionSupported("GL_ARB_texture_cube_map");
    static bool s_EXT_CubeMapSupported = isGLExtensionSupported("GL_EXT_texture_cube_map");

    if (!s_ARB_CubeMapSupported /*&& !s_EXT_CubeMapSupported*/)
        return;

    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const uint contextID = state.getContextID();

    // get the globj for the current contextID.
    GLuint& handle = getHandle(contextID);

    // For multi-texturing will need something like...
    // glActiveTextureARB((GLenum)(GL_TEXTURE0_ARB+_textureUnit));

    if (handle != 0)
    {
        if (_subloadMode == OFF)
        {
            glBindTexture( _target, handle );
        }
        else  if (imagesValid())
        {
            uint& modifiedTag = getModifiedTag(contextID);

            modifiedTag = 0;
            glBindTexture( _target, handle );
            for (int n=0; n<6; n++)
            {
                if ((_subloadMode == AUTO) ||
                    (_subloadMode == IF_DIRTY && modifiedTag != _images[n]->getModifiedTag()))
                {
                    glTexSubImage2D(faceTarget[n], 0,
                                    _subloadOffsX, _subloadOffsY,
                                    _images[n]->s(), _images[n]->t(),
                                    (GLenum) _images[n]->pixelFormat(), (GLenum) _images[n]->dataType(),
                                    _images[n]->data());
                    // update the modified flag to show that the image has been loaded.
                    modifiedTag += _images[n]->getModifiedTag();
                }
            }
        }
    }
    else if (imagesValid())
    {
        glGenTextures( 1L, (GLuint *)&handle );
        glBindTexture( _target, handle );

        applyImmediateMode(state);

        for (int n=0; n<6; n++)
        {
            applyFaceImmediateMode(
               faceTarget[n], _images[n].get(), state);
        }

        // in theory the following line is redundent, but in practice
        // have found that the first frame drawn doesn't apply the textures
        // unless a second bind is called?!!
        // perhaps it is the first glBind which is not required...
        glBindTexture( _target, handle );

    }
}


void TextureCubeMap::applyImmediateMode(State& state) const
{
    WrapMode ws = _wrap_s, wt = _wrap_t;

    // GL_IBM_texture_mirrored_repeat, fall-back REPEAT
    static bool s_mirroredSupported = isGLExtensionSupported("GL_IBM_texture_mirrored_repeat");
    if (!s_mirroredSupported)
    {
        if (ws == MIRROR)
            ws = REPEAT;
        if (wt == MIRROR)
            wt = REPEAT;
    }

    // GL_EXT_texture_edge_clamp, fall-back CLAMP
    static bool s_edgeClampSupported = isGLExtensionSupported("GL_EXT_texture_edge_clamp");
    if (!s_edgeClampSupported)
    {
        if (ws == CLAMP_TO_EDGE)
            ws = CLAMP;
        if (wt == CLAMP_TO_EDGE)
            wt = CLAMP;
    }

    static bool s_borderClampSupported = isGLExtensionSupported("GL_ARB_texture_border_clamp");
    if(!s_borderClampSupported)
    {
        if(ws == CLAMP_TO_BORDER)
            ws = CLAMP;
        if(wt == CLAMP_TO_BORDER)
            wt = CLAMP;
    }

    glTexParameteri( _target, GL_TEXTURE_WRAP_S, ws );
    glTexParameteri( _target, GL_TEXTURE_WRAP_T, wt );

    glTexParameteri( _target, GL_TEXTURE_MIN_FILTER, _min_filter);

    if (_mag_filter == ANISOTROPIC)
    {
        // check for support for anisotropic filter,
        // note since this is static varible it is intialised
        // only on the first time entering this code block,
        // is then never reevaluated on subsequent calls.
        static bool s_anisotropicSupported =
            isGLExtensionSupported("GL_EXT_texture_filter_anisotropic");

        if (s_anisotropicSupported)
        {
            // note, GL_TEXTURE_MAX_ANISOTROPY_EXT will either be defined
            // by gl.h (or via glext.h) or by include/osg/Texture.
            glTexParameterf(_target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 2.f);
        }
        else
        {
            glTexParameteri(_target, GL_TEXTURE_MAG_FILTER, LINEAR);
        }
    }
    else
    {
        glTexParameteri(_target, GL_TEXTURE_MAG_FILTER, _mag_filter);
    }
}



void TextureCubeMap::applyFaceImmediateMode(GLenum facetarget, Image* image, State& state) const
{
    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const uint contextID = state.getContextID();

    // update the modified tag to show that it is upto date.
    getModifiedTag(contextID) = image->getModifiedTag();


    if (_subloadMode == OFF)
        image->ensureDimensionsArePowerOfTwo();

    glPixelStorei(GL_UNPACK_ALIGNMENT,image->packing());

    static bool s_ARB_Compression = isGLExtensionSupported("GL_ARB_texture_compression");
    static bool s_S3TC_Compression = isGLExtensionSupported("GL_EXT_texture_compression_s3tc");

    // select the internalFormat required for the texture.
    int internalFormat = image->internalFormat();
    switch(_internalFormatMode)
    {
        case(USE_IMAGE_DATA_FORMAT):
            internalFormat = image->internalFormat();
            break;

        case(USE_ARB_COMPRESSION):
            if (s_ARB_Compression)
            {
                switch(image->pixelFormat())
                {
                    case(1): internalFormat = GL_COMPRESSED_ALPHA_ARB; break;
                    case(2): internalFormat = GL_COMPRESSED_LUMINANCE_ALPHA_ARB; break;
                    case(3): internalFormat = GL_COMPRESSED_RGB_ARB; break;
                    case(4): internalFormat = GL_COMPRESSED_RGBA_ARB; break;
                    case(GL_RGB): internalFormat = GL_COMPRESSED_RGB_ARB; break;
                    case(GL_RGBA): internalFormat = GL_COMPRESSED_RGBA_ARB; break;
                    case(GL_ALPHA): internalFormat = GL_COMPRESSED_ALPHA_ARB; break;
                    case(GL_LUMINANCE): internalFormat = GL_COMPRESSED_LUMINANCE_ARB; break;
                    case(GL_LUMINANCE_ALPHA): internalFormat = GL_COMPRESSED_LUMINANCE_ALPHA_ARB; break;
                    case(GL_INTENSITY): internalFormat = GL_COMPRESSED_INTENSITY_ARB; break;
                }
            }
            else internalFormat = image->internalFormat();
            break;

        case(USE_S3TC_DXT1_COMPRESSION):
            if (s_S3TC_Compression)
            {
                switch(image->pixelFormat())
                {
                    case(3):        internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
                    case(4):        internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
                    case(GL_RGB):   internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
                    case(GL_RGBA):  internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
                    default:        internalFormat = image->internalFormat(); break;
                }
            }
            else internalFormat = image->internalFormat();
            break;

        case(USE_S3TC_DXT3_COMPRESSION):
            if (s_S3TC_Compression)
            {
                switch(image->pixelFormat())
                {
                    case(3):
                    case(GL_RGB):   internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
                    case(4):
                    case(GL_RGBA):  internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
                    default:        internalFormat = image->internalFormat(); break;
                }
            }
            else internalFormat = image->internalFormat();
            break;

        case(USE_S3TC_DXT5_COMPRESSION):
            if (s_S3TC_Compression)
            {
                switch(image->pixelFormat())
                {
                    case(3):
                    case(GL_RGB):   internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
                    case(4):
                    case(GL_RGBA):  internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
                    default:        internalFormat = image->internalFormat(); break;
                }
            }
            else internalFormat = image->internalFormat();
            break;

        case(USE_USER_DEFINED_FORMAT):
            internalFormat = _internalFormatValue;
            break;

    }

    if (_subloadMode == OFF)
    {
        if( _min_filter == LINEAR || _min_filter == NEAREST )
        {
            glTexImage2D( facetarget, 0, internalFormat,
                image->s(), image->t(), 0,
                (GLenum)image->pixelFormat(),
                (GLenum)image->dataType(),
                image->data() );

            // just estimate estimate it right now..
            // note, ignores texture compression..
            _textureObjectSize = image->s()*image->t()*4;

        }
        else
        {

            gluBuild2DMipmaps( facetarget, internalFormat,
                image->s(), image->t(),
                (GLenum)image->pixelFormat(), (GLenum)image->dataType(),
                image->data() );

            // just estimate size it right now..
            // crude x2 multiplier to account for minmap storage.
            // note, ignores texture compression..
            _textureObjectSize = image->s()*image->t()*4;

        }

        _textureWidth = image->s();
        _textureHeight = image->t();
    }
    else
    {
        /* target=? ABJ
        static bool s_SGIS_GenMipmap = isGLExtensionSupported("GL_SGIS_generate_mipmap");

        if (s_SGIS_GenMipmap && (_min_filter != LINEAR && _min_filter != NEAREST)) {
            glTexParameteri(_target, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
        }
        */
        // calculate texture dimension
        _textureWidth = 1;
        for (; _textureWidth < (_subloadOffsX + image->s()); _textureWidth <<= 1)
            ;

        _textureHeight = 1;
        for (; _textureHeight < (_subloadOffsY + image->t()); _textureHeight <<= 1)
            ;

        // reserve appropriate texture memory
        glTexImage2D(facetarget, 0, internalFormat,
                     _textureWidth, _textureHeight, 0,
                     (GLenum) image->pixelFormat(), (GLenum) image->dataType(),
                     NULL);

        glTexSubImage2D(facetarget, 0,
                        _subloadOffsX, _subloadOffsY,
                        image->s(), image->t(),
                        (GLenum) image->pixelFormat(), (GLenum) image->dataType(),
                        image->data());
    }
    
}

