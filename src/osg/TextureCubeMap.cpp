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


// include/osg/TextureCubeMap defines GL_TEXTURE_CUBE_MAP to be
// 0x8513 which is the same as GL_TEXTURE_CUBE_MAP_ARB & _EXT.
// assume its the same as what OpenGL 1.3 defines.

#ifndef GL_ARB_texture_cube_map
#define GL_ARB_texture_cube_map 1
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
    _target = GL_TEXTURE_CUBE_MAP;      // default to ARB extension
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

Image* TextureCubeMap::getImage(const Face face)
{
    return _images[face].get();
}

const Image* TextureCubeMap::getImage(const Face face) const
{
    return _images[face].get();
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
    static bool s_CubeMapSupported = isGLExtensionSupported("GL_ARB_texture_cube_map") ||
                                     isGLExtensionSupported("GL_EXT_texture_cube_map");

    if (!s_CubeMapSupported)
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
            if (_texParamtersDirty) applyTexParameters(_target,state);
        }
        else  if (imagesValid())
        {
            uint& modifiedTag = getModifiedTag(contextID);

            modifiedTag = 0;
            glBindTexture( _target, handle );
            if (_texParamtersDirty) applyTexParameters(_target,state);
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

        applyTexParameters(_target,state);

        for (int n=0; n<6; n++)
        {
            applyTexImage( faceTarget[n], _images[n].get(), state);
        }

        // in theory the following line is redundent, but in practice
        // have found that the first frame drawn doesn't apply the textures
        // unless a second bind is called?!!
        // perhaps it is the first glBind which is not required...
        glBindTexture( _target, handle );

    }
}
