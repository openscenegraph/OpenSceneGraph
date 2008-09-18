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

#include <stdlib.h>

#include <osgShadow/ShadowMap>
#include <osgShadow/SoftShadowMap>
#include <osgShadow/ShadowedScene>
#include <osg/Notify>
#include <osg/ComputeBoundsVisitor>
#include <osg/PolygonOffset>
#include <osg/CullFace>
#include <osg/io_utils>

using namespace osgShadow;

#include <iostream>
//for debug
#include <osg/LightSource>
#include <osg/PolygonMode>
#include <osg/Geometry>
#include <osgDB/ReadFile>
#include <osgText/Text>
#include <osg/Texture3D>
#include <osg/TexGen>

#define IMPROVE_TEXGEN_PRECISION 1

//////////////////////////////////////////////////////////////////
// fragment shader
//
// Implementation from Chapter 17, Efficient Soft-Edged Shadows Using Pixel Shader Branching, Yury Uralsky.
// GPU Gems 2, Matt Pharr ed. Addison-Wesley.
//
static const char fragmentSoftShaderSource_noBaseTexture[] =
   "#define SAMPLECOUNT 64 \n"
    "#define SAMPLECOUNT_FLOAT 64.0 \n"
    "#define SAMPLECOUNT_D2 32 \n"
    "#define SAMPLECOUNT_D2_FLOAT 32.0 \n"
    "#define INV_SAMPLECOUNT (1.0 / SAMPLECOUNT_FLOAT) \n"

    "uniform sampler2DShadow osgShadow_shadowTexture; \n"
    "uniform sampler3D osgShadow_jitterTexture; \n"

    "uniform vec2 osgShadow_ambientBias; \n"
    "uniform float osgShadow_softnessWidth; \n"
    "uniform float osgShadow_jitteringScale; \n"

    "void main(void) \n"
    "{ \n"
    "  vec4 sceneShadowProj  = gl_TexCoord[1]; \n"
    "  float softFactor = osgShadow_softnessWidth * sceneShadowProj.w; \n"
    "  vec4 smCoord  = sceneShadowProj; \n"
    "  vec3 jitterCoord = vec3( gl_FragCoord.xy / osgShadow_jitteringScale, 0.0 ); \n"
    "  vec4 shadow = vec4(0.0, 0.0, 0.0, 0.0); \n"
// First "cheap" sample test
    "  const float pass_div = 1.0 / (2.0 * 4.0); \n"
    "  for ( int i = 0; i < 4; ++i ) \n"
    "  { \n"
// Get jitter values in [0,1]; adjust to have values in [-1,1]
    "    vec4 offset = 2.0 * texture3D( osgShadow_jitterTexture, jitterCoord ) -1.0; \n"
    "    jitterCoord.z += 1.0 / SAMPLECOUNT_D2_FLOAT; \n"

    "    smCoord.xy = sceneShadowProj.xy  + (offset.xy) * softFactor; \n"
    "    shadow +=  shadow2DProj( osgShadow_shadowTexture, smCoord ) * pass_div; \n"

    "    smCoord.xy = sceneShadowProj.xy  + (offset.zw) * softFactor; \n"
    "    shadow +=  shadow2DProj( osgShadow_shadowTexture, smCoord ) *pass_div; \n"
    "  } \n"
// skip all the expensive shadow sampling if not needed
    "  if ( shadow * (shadow -1.0) != 0.0 ) \n"
    "  { \n"
    "    shadow *= pass_div; \n"
    "    for (int i=0; i<SAMPLECOUNT_D2 - 4; ++i){ \n"
    "      vec4 offset = 2.0 * texture3D( osgShadow_jitterTexture, jitterCoord ) - 1.0; \n"
    "      jitterCoord.z += 1.0 / SAMPLECOUNT_D2_FLOAT; \n"

    "      smCoord.xy = sceneShadowProj.xy  + offset.xy * softFactor; \n"
    "      shadow +=  shadow2DProj( osgShadow_shadowTexture, smCoord ) * INV_SAMPLECOUNT; \n"

    "      smCoord.xy = sceneShadowProj.xy  + offset.zw * softFactor; \n"
    "      shadow +=  shadow2DProj( osgShadow_shadowTexture, smCoord ) * INV_SAMPLECOUNT; \n"
    "    } \n"
    "  } \n"
// apply shadow, modulo the ambient bias
    "  gl_FragColor = gl_Color * (osgShadow_ambientBias.x + shadow * osgShadow_ambientBias.y); \n"
    "} \n";



//////////////////////////////////////////////////////////////////
// fragment shader
//
static const char fragmentSoftShaderSource_withBaseTexture[] =
    "#define SAMPLECOUNT 64 \n"
    "#define SAMPLECOUNT_FLOAT 64.0 \n"
    "#define SAMPLECOUNT_D2 32 \n"
    "#define SAMPLECOUNT_D2_FLOAT 32.0 \n"
    "#define INV_SAMPLECOUNT (1.0 / SAMPLECOUNT_FLOAT) \n"

    "uniform sampler2D osgShadow_baseTexture; \n"
    "uniform sampler2DShadow osgShadow_shadowTexture; \n"
    "uniform sampler3D osgShadow_jitterTexture; \n"

    "uniform vec2 osgShadow_ambientBias; \n"
    "uniform float osgShadow_softnessWidth; \n"
    "uniform float osgShadow_jitteringScale; \n"

    "void main(void) \n"
    "{ \n"
    "  vec4 sceneShadowProj  = gl_TexCoord[1]; \n"
    "  float softFactor = osgShadow_softnessWidth * sceneShadowProj.w; \n"
    "  vec4 smCoord  = sceneShadowProj; \n"
    "  vec3 jitterCoord = vec3( gl_FragCoord.xy / osgShadow_jitteringScale, 0.0 ); \n"
    "  vec4 shadow = vec4(0.0, 0.0, 0.0, 0.0); \n"
// First "cheap" sample test
    "  const float pass_div = 1.0 / (2.0 * 4.0); \n"
    "  for ( int i = 0; i < 4; ++i ) \n"
    "  { \n"
// Get jitter values in [0,1]; adjust to have values in [-1,1]
    "    vec4 offset = 2.0 * texture3D( osgShadow_jitterTexture, jitterCoord ) -1.0; \n"
    "    jitterCoord.z += 1.0 / SAMPLECOUNT_D2_FLOAT; \n"

    "    smCoord.xy = sceneShadowProj.xy  + (offset.xy) * softFactor; \n"
    "    shadow +=  shadow2DProj( osgShadow_shadowTexture, smCoord ) * pass_div; \n"

    "    smCoord.xy = sceneShadowProj.xy  + (offset.zw) * softFactor; \n"
    "    shadow +=  shadow2DProj( osgShadow_shadowTexture, smCoord ) *pass_div; \n"
    "  } \n"
// skip all the expensive shadow sampling if not needed
    "  if ( shadow * (shadow -1.0) != 0.0 ) \n"
    "  { \n"
    "    shadow *= pass_div; \n"
    "    for (int i=0; i<SAMPLECOUNT_D2 -4; ++i){ \n"
    "      vec4 offset = 2.0 * texture3D( osgShadow_jitterTexture, jitterCoord ) - 1.0; \n"
    "      jitterCoord.z += 1.0 / SAMPLECOUNT_D2_FLOAT; \n"

    "      smCoord.xy = sceneShadowProj.xy  + offset.xy * softFactor; \n"
    "      shadow +=  shadow2DProj( osgShadow_shadowTexture, smCoord ) * INV_SAMPLECOUNT; \n"

    "      smCoord.xy = sceneShadowProj.xy  + offset.zw * softFactor; \n"
    "      shadow +=  shadow2DProj( osgShadow_shadowTexture, smCoord ) * INV_SAMPLECOUNT; \n"
    "    } \n"
    "  } \n"
// apply color and object base texture
    "  vec4 color = gl_Color * texture2D( osgShadow_baseTexture, gl_TexCoord[0].xy ); \n"
// apply shadow, modulo the ambient bias
    "  gl_FragColor = color * (osgShadow_ambientBias.x + shadow * osgShadow_ambientBias.y); \n"
    "} \n";





SoftShadowMap::SoftShadowMap():
    _softnessWidth(0.005f),
    _jitteringScale(32.f),
    _jitterTextureUnit(_shadowTextureUnit+1)
{
}

SoftShadowMap::SoftShadowMap(const SoftShadowMap& copy, const osg::CopyOp& copyop):
ShadowMap(copy,copyop),
    _softnessWidth(copy._softnessWidth),
    _jitteringScale(copy._jitteringScale),
    _jitterTextureUnit(copy._shadowTextureUnit)
{
}

void SoftShadowMap::setJitteringScale(const float jitteringScale)
{
    _jitteringScale = jitteringScale;
    if (_jitteringScaleUniform.valid()) _jitteringScaleUniform->set(_jitteringScale);
}

void SoftShadowMap::setSoftnessWidth(const float softnessWidth)
{
    _softnessWidth = softnessWidth;
    if (_softnessWidthUniform.valid()) _softnessWidthUniform->set(_softnessWidth);
}

void SoftShadowMap::setJitterTextureUnit(unsigned int jitterTextureUnit)
{
    _jitterTextureUnit=jitterTextureUnit;
}

void SoftShadowMap::createUniforms()
{
    _uniformList.clear();

    osg::Uniform* baseTextureSampler = new osg::Uniform("osgShadow_baseTexture",(int)_baseTextureUnit);
    _uniformList.push_back(baseTextureSampler);

    osg::Uniform* shadowTextureSampler = new osg::Uniform("osgShadow_shadowTexture",(int)_shadowTextureUnit);
    _uniformList.push_back(shadowTextureSampler);

    _ambientBiasUniform = new osg::Uniform("osgShadow_ambientBias",_ambientBias);
    _uniformList.push_back(_ambientBiasUniform.get());

    _softnessWidthUniform = new osg::Uniform("osgShadow_softnessWidth",_softnessWidth);
    _uniformList.push_back(_softnessWidthUniform.get());

    _jitteringScaleUniform = new osg::Uniform("osgShadow_jitteringScale",_jitteringScale);
    _uniformList.push_back(_jitteringScaleUniform.get());

    _jitterTextureUnit=_shadowTextureUnit+1;
    initJittering(_stateset.get());

    osg::Uniform* jitterTextureSampler = new osg::Uniform("osgShadow_jitterTexture",(int)_jitterTextureUnit);
    _uniformList.push_back(jitterTextureSampler);


}

void SoftShadowMap::createShaders()
{
    // if we are not given shaders, use the default
    if( _shaderList.empty() )
    {
        if (_shadowTextureUnit==0)
        {
            osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentSoftShaderSource_noBaseTexture);
            _shaderList.push_back(fragment_shader);
        }
        else
        {
            osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentSoftShaderSource_withBaseTexture);
            _shaderList.push_back(fragment_shader);

        }
    }
}


// Implementation from Chapter 17, Efficient Soft-Edged Shadows Using Pixel Shader Branching, Yury Uralsky.
// GPU Gems 2, Matt Pharr ed. Addison-Wesley.
//
// Creates a 3D texture containing jittering data used in the shader to take samples of the shadow map.
void SoftShadowMap::initJittering(osg::StateSet *ss)
{
    // create a 3D texture with hw mipmapping
    osg::Texture3D* texture3D = new osg::Texture3D;
    texture3D->setFilter(osg::Texture3D::MIN_FILTER,osg::Texture3D::NEAREST);
    texture3D->setFilter(osg::Texture3D::MAG_FILTER,osg::Texture3D::NEAREST);
    texture3D->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::REPEAT);
    texture3D->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::REPEAT);
    texture3D->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::REPEAT);
    texture3D->setUseHardwareMipMapGeneration(true);

    const unsigned int size = 16;
    const unsigned int gridW =  8;
    const unsigned int gridH =  8;
    unsigned int R = (gridW * gridH / 2);
    texture3D->setTextureSize(size, size, R);

    // then create the 3d image to fill with jittering data
    osg::Image* image3D = new osg::Image;
    unsigned char *data3D = new unsigned char[size * size * R * 4];

    for ( unsigned int s = 0; s < size; ++s )
    {
        for ( unsigned int t = 0; t < size; ++t )
        {
            float v[4], d[4];

            for ( unsigned int r = 0; r < R; ++r )
            {
                const int x = r % ( gridW / 2 );
                const int y = ( gridH - 1 ) - ( r / (gridW / 2) );

                // Generate points on a  regular gridW x gridH rectangular
                // grid.   We  multiply  x   by  2  because,  we  treat  2
                // consecutive x  each loop iteration.  Add 0.5f  to be in
                // the center of the pixel. x, y belongs to [ 0.0, 1.0 ].
                v[0] = float( x * 2     + 0.5f ) / gridW;
                v[1] = float( y         + 0.5f ) / gridH;
                v[2] = float( x * 2 + 1 + 0.5f ) / gridW;
                v[3] = v[1];

                // Jitter positions. ( 0.5f / w ) == ( 1.0f / 2*w )
                v[0] += ((float)rand() * 2.f / RAND_MAX - 1.f) * ( 0.5f / gridW );
                v[1] += ((float)rand() * 2.f / RAND_MAX - 1.f) * ( 0.5f / gridH );
                v[2] += ((float)rand() * 2.f / RAND_MAX - 1.f) * ( 0.5f / gridW );
                v[3] += ((float)rand() * 2.f / RAND_MAX - 1.f) * ( 0.5f / gridH );

                // Warp to disk; values in [-1,1]
                d[0] = sqrtf( v[1] ) * cosf( 2.f * 3.1415926f * v[0] );
                d[1] = sqrtf( v[1] ) * sinf( 2.f * 3.1415926f * v[0] );
                d[2] = sqrtf( v[3] ) * cosf( 2.f * 3.1415926f * v[2] );
                d[3] = sqrtf( v[3] ) * sinf( 2.f * 3.1415926f * v[2] );

                // store d into unsigned values [0,255]
                const unsigned int tmp = ( (r * size * size) + (t * size) + s ) * 4;
                data3D[ tmp + 0 ] = (unsigned char)( ( 1.f + d[0] ) * 127  );
                data3D[ tmp + 1 ] = (unsigned char)( ( 1.f + d[1] ) * 127  );
                data3D[ tmp + 2 ] = (unsigned char)( ( 1.f + d[2] ) * 127  );
                data3D[ tmp + 3 ] = (unsigned char)( ( 1.f + d[3] ) * 127  );

            }
        }
    }

    // the GPU Gem implementation uses a NV specific internal texture format (GL_SIGNED_RGBA_NV)
    // In order to make it more generic, we use GL_RGBA4 which should be cross platform.
    image3D->setImage(size, size, R, GL_RGBA4, GL_RGBA, GL_UNSIGNED_BYTE, data3D, osg::Image::USE_NEW_DELETE);
    texture3D->setImage(image3D);

    ss->setTextureAttributeAndModes(_jitterTextureUnit, texture3D, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    ss->setTextureMode(_jitterTextureUnit,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
    ss->setTextureMode(_jitterTextureUnit,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
    ss->setTextureMode(_jitterTextureUnit,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);

}

