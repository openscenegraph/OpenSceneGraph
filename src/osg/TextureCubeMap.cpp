#include <osg/GLExtensions>
#include <osg/ref_ptr>
#include <osg/Image>
#include <osg/State>
#include <osg/TextureCubeMap>

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


TextureCubeMap::TextureCubeMap():
            _textureWidth(0),
            _textureHeight(0),
            _numMimpmapLevels(0)
{
}

TextureCubeMap::TextureCubeMap(const TextureCubeMap& text,const CopyOp& copyop):
            TextureBase(text,copyop),
            _textureWidth(text._textureWidth),
            _textureHeight(text._textureHeight),
            _numMimpmapLevels(text._numMimpmapLevels),
            _subloadCallback(text._subloadCallback)
{
    _images[0] = copyop(text._images[0].get());
    _images[1] = copyop(text._images[1].get());
    _images[2] = copyop(text._images[2].get());
    _images[3] = copyop(text._images[3].get());
    _images[4] = copyop(text._images[4].get());
    _images[5] = copyop(text._images[5].get());

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

    int result = compareTextureBase(rhs);
    if (result!=0) return result;

    // compare each paramter in turn against the rhs.
    COMPARE_StateAttribute_Parameter(_textureWidth)
    COMPARE_StateAttribute_Parameter(_textureHeight)
    COMPARE_StateAttribute_Parameter(_subloadCallback)

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

void TextureCubeMap::computeInternalFormat() const
{
    if (imagesValid()) computeInternalFormatWithImage(*_images[0]); 
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
    GLuint& handle = getTextureObject(contextID);

    if (handle != 0)
    {
        glBindTexture( GL_TEXTURE_CUBE_MAP, handle );
        if (_texParametersDirty) applyTexParameters(GL_TEXTURE_CUBE_MAP,state);

        if (_subloadCallback.valid())
        {
            _subloadCallback->subload(*this,state);
        }

    }
    else if (_subloadCallback.valid())
    {
        glGenTextures( 1L, (GLuint *)&handle );
        glBindTexture( GL_TEXTURE_CUBE_MAP, handle );

        applyTexParameters(GL_TEXTURE_CUBE_MAP,state);

        _subloadCallback->load(*this,state);

        // in theory the following line is redundent, but in practice
        // have found that the first frame drawn doesn't apply the textures
        // unless a second bind is called?!!
        // perhaps it is the first glBind which is not required...
        glBindTexture( GL_TEXTURE_CUBE_MAP, handle );

    }
    else if (imagesValid())
    {

        glGenTextures( 1L, (GLuint *)&handle );
        glBindTexture( GL_TEXTURE_CUBE_MAP, handle );

        applyTexParameters(GL_TEXTURE_CUBE_MAP,state);


        for (int n=0; n<6; n++)
        {
            applyTexImage2D( faceTarget[n], _images[n].get(), state, _textureWidth, _textureHeight, _numMimpmapLevels);
        }


        // in theory the following line is redundent, but in practice
        // have found that the first frame drawn doesn't apply the textures
        // unless a second bind is called?!!
        // perhaps it is the first glBind which is not required...
        glBindTexture( GL_TEXTURE_CUBE_MAP, handle );
        
    }
    else
    {
        glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
    }
}
