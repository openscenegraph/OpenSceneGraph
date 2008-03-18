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

#include <osgShadow/SoftShadowMap>
#include <osgShadow/ShadowedScene>
#include <osg/Notify>
#include <osg/ComputeBoundsVisitor>
#include <osg/PolygonOffset>
#include <osg/CullFace>
#include <osg/io_utils>


#include <osg/Texture3D>
#include <osg/TexGen>
#include <iostream>

using namespace osgShadow;


//////////////////////////////////////////////////////////////////
// fragment shader
//
// Implementation from Chapter 17, Efficient Soft-Edged Shadows Using Pixel Shader Branching, Yury Uralsky.
// GPU Gems 2, Matt Pharr ed. Addison-Wesley.
//
static const char fShaderSource_noBaseTexture[] =
   "#define SAMPLECOUNT 64 \n"
    "#define SAMPLECOUNT_FLOAT 64.0 \n"
    "#define SAMPLECOUNT_D2 32 \n"
    "#define SAMPLECOUNT_D2_FLOAT 32.0 \n"
    "#define INV_SAMPLECOUNT (1.0 / SAMPLECOUNT_FLOAT) \n"

    "uniform sampler2DShadow shadowTexture; \n"
    "uniform sampler3D jitterMapSampler; \n"

    "uniform vec2 ambientBias; \n"
    "uniform float softwidth; \n"
    "uniform float jscale; \n"

    "void main(void) \n"
    "{ \n"
    "  vec4 sceneShadowProj  = gl_TexCoord[1]; \n"
    "  float softFactor = softwidth * sceneShadowProj.w; \n"
    "  vec4 smCoord  = sceneShadowProj; \n"
    "  vec3 jitterCoord = vec3( gl_FragCoord.xy / jscale, 0.0 ); \n"
    "  vec4 shadow = vec4(0.0, 0.0, 0.0, 0.0); \n"
// First "cheap" sample test
    "  const float pass_div = 1.0 / (2.0 * 4.0); \n"
    "  for ( int i = 0; i < 4; ++i ) \n"
    "  { \n"
// Get jitter values in [0,1]; adjust to have values in [-1,1]
    "    vec4 offset = 2.0 * texture3D( jitterMapSampler, jitterCoord ) -1.0; \n"
    "    jitterCoord.z += 1.0 / SAMPLECOUNT_D2_FLOAT; \n"

    "    smCoord.xy = sceneShadowProj.xy  + (offset.xy) * softFactor; \n"
    "    shadow +=  shadow2DProj( shadowTexture, smCoord ) * pass_div; \n"

    "    smCoord.xy = sceneShadowProj.xy  + (offset.zw) * softFactor; \n"
    "    shadow +=  shadow2DProj( shadowTexture, smCoord ) *pass_div; \n"
    "  } \n"
// skip all the expensive shadow sampling if not needed
    "  if ( shadow * (shadow -1.0) != 0.0 ) \n"
    "  { \n"
    "    shadow *= pass_div; \n"
    "    for (int i=0; i<SAMPLECOUNT_D2 - 4; ++i){ \n"
    "      vec4 offset = 2.0 * texture3D( jitterMapSampler, jitterCoord ) - 1.0; \n"
    "      jitterCoord.z += 1.0 / SAMPLECOUNT_D2_FLOAT; \n"

    "      smCoord.xy = sceneShadowProj.xy  + offset.xy * softFactor; \n"
    "      shadow +=  shadow2DProj( shadowTexture, smCoord ) * INV_SAMPLECOUNT; \n"

    "      smCoord.xy = sceneShadowProj.xy  + offset.zw * softFactor; \n"
    "      shadow +=  shadow2DProj( shadowTexture, smCoord ) * INV_SAMPLECOUNT; \n"
    "    } \n"
    "  } \n"
// apply shadow, modulo the ambient bias
    "  gl_FragColor = gl_Color * (ambientBias.x + shadow * ambientBias.y); \n"
    "} \n";



//////////////////////////////////////////////////////////////////
// fragment shader
//
static const char fShaderSource_withBaseTexture[] =
    "#define SAMPLECOUNT 64 \n"
    "#define SAMPLECOUNT_FLOAT 64.0 \n"
    "#define SAMPLECOUNT_D2 32 \n"
    "#define SAMPLECOUNT_D2_FLOAT 32.0 \n"
    "#define INV_SAMPLECOUNT (1.0 / SAMPLECOUNT_FLOAT) \n"

    "uniform sampler2D baseTexture; \n"
    "uniform sampler2DShadow shadowTexture; \n"
    "uniform sampler3D jitterMapSampler; \n"

    "uniform vec2 ambientBias; \n"
    "uniform float softwidth; \n"
    "uniform float jscale; \n"

    "void main(void) \n"
    "{ \n"
    "  vec4 sceneShadowProj  = gl_TexCoord[1]; \n"
    "  float softFactor = softwidth * sceneShadowProj.w; \n"
    "  vec4 smCoord  = sceneShadowProj; \n"
    "  vec3 jitterCoord = vec3( gl_FragCoord.xy / jscale, 0.0 ); \n"
    "  vec4 shadow = vec4(0.0, 0.0, 0.0, 0.0); \n"
// First "cheap" sample test
    "  const float pass_div = 1.0 / (2.0 * 4.0); \n"
    "  for ( int i = 0; i < 4; ++i ) \n"
    "  { \n"
// Get jitter values in [0,1]; adjust to have values in [-1,1]
    "    vec4 offset = 2.0 * texture3D( jitterMapSampler, jitterCoord ) -1.0; \n"
    "    jitterCoord.z += 1.0 / SAMPLECOUNT_D2_FLOAT; \n"

    "    smCoord.xy = sceneShadowProj.xy  + (offset.xy) * softFactor; \n"
    "    shadow +=  shadow2DProj( shadowTexture, smCoord ) * pass_div; \n"

    "    smCoord.xy = sceneShadowProj.xy  + (offset.zw) * softFactor; \n"
    "    shadow +=  shadow2DProj( shadowTexture, smCoord ) *pass_div; \n"
    "  } \n"
// skip all the expensive shadow sampling if not needed
    "  if ( shadow * (shadow -1.0) != 0.0 ) \n"
    "  { \n"
    "    shadow *= pass_div; \n"
    "    for (int i=0; i<SAMPLECOUNT_D2 -4; ++i){ \n"
    "      vec4 offset = 2.0 * texture3D( jitterMapSampler, jitterCoord ) - 1.0; \n"
    "      jitterCoord.z += 1.0 / SAMPLECOUNT_D2_FLOAT; \n"

    "      smCoord.xy = sceneShadowProj.xy  + offset.xy * softFactor; \n"
    "      shadow +=  shadow2DProj( shadowTexture, smCoord ) * INV_SAMPLECOUNT; \n"

    "      smCoord.xy = sceneShadowProj.xy  + offset.zw * softFactor; \n"
    "      shadow +=  shadow2DProj( shadowTexture, smCoord ) * INV_SAMPLECOUNT; \n"
    "    } \n"
    "  } \n"
// apply color and object base texture
    "  vec4 color = gl_Color * texture2D( baseTexture, gl_TexCoord[0].xy ); \n"
// apply shadow, modulo the ambient bias
    "  gl_FragColor = color * (ambientBias.x + shadow * ambientBias.y); \n"
    "} \n";



SoftShadowMap::SoftShadowMap():
    _textureUnit(1),
    _ambientBias(0.5f,0.5f),
    _softnesswidth(0.005f),
    _jitteringscale(32.f),
    _bias(0.0f),
    _textureSize(1024, 1024)
{
}

SoftShadowMap::SoftShadowMap(const SoftShadowMap& copy, const osg::CopyOp& copyop):
    ShadowTechnique(copy,copyop),
    _textureUnit(copy._textureUnit),
    _ambientBias(copy._ambientBias),
    _softnesswidth(copy._softnesswidth),
    _jitteringscale(copy._jitteringscale),
    _bias(copy._bias),
    _textureSize(copy._textureSize)
{
}

void SoftShadowMap::setTextureUnit(unsigned int unit)
{
    _textureUnit = unit;
}

void SoftShadowMap::setAmbientBias(const osg::Vec2& ambientBias)
{
    _ambientBias = ambientBias;
}

void SoftShadowMap::setSoftnessWidth(const float softnesswidth )
{
    _softnesswidth = softnesswidth;
}

void SoftShadowMap::setJitteringScale(const float jitteringscale )
{
    _jitteringscale = jitteringscale;
}

void SoftShadowMap::setTextureSize(const osg::Vec2s& texsize)
{ 
    _textureSize = texsize; 

    if (_texture.valid())
    {
        _texture->setTextureSize(_textureSize[0], _textureSize[1]);
        _camera->setViewport(0,0,_textureSize[0], _textureSize[1]);
    }
}


void SoftShadowMap::init()
{
    if (!_shadowedScene) return;

    _texture = new osg::Texture2D;
    _texture->setTextureSize(_textureSize[0], _textureSize[1]);
    _texture->setInternalFormat(GL_DEPTH_COMPONENT);
    _texture->setSourceType(GL_UNSIGNED_INT);

    // Sets GL_TEXTURE_COMPARE_MODE_ARB to GL_COMPARE_R_TO_TEXTURE_ARB
    _texture->setShadowComparison(true);
    _texture->setShadowCompareFunc(osg::Texture::LEQUAL);

    _texture->setShadowTextureMode(osg::Texture::LUMINANCE);
    _texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
    _texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
    _texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    _texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);



    // set up the render to texture camera.
    {
        // create the camera
        _camera = new osg::Camera;

        _camera->setInheritanceMask(0x0);
        _camera->setCullCallback(new CameraCullCallback(this));

        _camera->setClearMask(GL_DEPTH_BUFFER_BIT);
        //_camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        _camera->setClearColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
        _camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);

        // set viewport
        _camera->setViewport(0,0,_textureSize[0],_textureSize[1]);

        // set the camera to render before the main camera.
        _camera->setRenderOrder(osg::Camera::PRE_RENDER);

        // tell the camera to use OpenGL frame buffer object where supported.
        _camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
        //_camera->setRenderTargetImplementation(osg::Camera::SEPERATE_WINDOW);

        // attach the texture and use it as the color buffer.
        _camera->attach(osg::Camera::DEPTH_BUFFER, _texture.get());

        osg::StateSet* stateset = _camera->getOrCreateStateSet();

        float factor = 0.0f;
        float units = 1.0f;

        osg::ref_ptr<osg::PolygonOffset> polygon_offset = new osg::PolygonOffset;
        polygon_offset->setFactor(factor);
        polygon_offset->setUnits(units);
        stateset->setAttribute(polygon_offset.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        stateset->setMode(GL_POLYGON_OFFSET_FILL, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        osg::ref_ptr<osg::CullFace> cull_face = new osg::CullFace;

        // set culling to BACK facing (cf message from Wojtek Lewandowski in osg Mailing list)
        cull_face->setMode(osg::CullFace::FRONT);
        stateset->setAttribute(cull_face.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        stateset->setMode(GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    }

    {
        _stateset = new osg::StateSet;
        _stateset->setTextureAttributeAndModes(_textureUnit,_texture.get(),osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        _stateset->setTextureMode(_textureUnit,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
        _stateset->setTextureMode(_textureUnit,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
        _stateset->setTextureMode(_textureUnit,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);
        _stateset->setTextureMode(_textureUnit,GL_TEXTURE_GEN_Q,osg::StateAttribute::ON);

        _texgen = new osg::TexGen;


        osg::Program* program = new osg::Program;
        _stateset->setAttribute(program);

        if ( _textureUnit==0)
        {
            osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fShaderSource_noBaseTexture);
            program->addShader(fragment_shader);
        }
        else
        {
            osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fShaderSource_withBaseTexture);
            program->addShader(fragment_shader);

            osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
            _stateset->addUniform(baseTextureSampler);

        }

        osg::Uniform* shadowTextureSampler = new osg::Uniform("shadowTexture",(int)_textureUnit);
        _stateset->addUniform(shadowTextureSampler);

        osg::Uniform* ambientBias = new osg::Uniform("ambientBias",_ambientBias);
        _stateset->addUniform(ambientBias);

        // bhbn
        // Initialisation of jittering texture
        initJittering(_stateset.get());

        osg::Uniform* jitterMapSampler = new osg::Uniform("jitterMapSampler",(int)_textureUnit + 1);
        _stateset->addUniform(jitterMapSampler);

        osg::Uniform* softwidth = new osg::Uniform("softwidth",_softnesswidth);
        _stateset->addUniform(softwidth);

        osg::Uniform* jscale = new osg::Uniform("jscale",_jitteringscale);
        _stateset->addUniform(jscale);

    }

    _dirty = false;
}


void SoftShadowMap::update(osg::NodeVisitor& nv)
{
    _shadowedScene->osg::Group::traverse(nv);
}

void SoftShadowMap::cull(osgUtil::CullVisitor& cv)
{
    // record the traversal mask on entry so we can reapply it later.
    unsigned int traversalMask = cv.getTraversalMask();

    osgUtil::RenderStage* orig_rs = cv.getRenderStage();

    // do traversal of shadow receiving scene which does need to be decorated by the shadow map
    {
        cv.pushStateSet(_stateset.get());

        _shadowedScene->osg::Group::traverse(cv);

        cv.popStateSet();

    }

    // need to compute view frustum for RTT camera.
    // 1) get the light position
    // 2) get the center and extents of the view frustum

    const osg::Light* selectLight = 0;
    osg::Vec4 lightpos;

    osgUtil::PositionalStateContainer::AttrMatrixList& aml = orig_rs->getPositionalStateContainer()->getAttrMatrixList();
    for(osgUtil::PositionalStateContainer::AttrMatrixList::iterator itr = aml.begin();
        itr != aml.end();
        ++itr)
    {
        const osg::Light* light = dynamic_cast<const osg::Light*>(itr->first.get());
        if (light)
        {
            osg::RefMatrix* matrix = itr->second.get();
            if (matrix) lightpos = light->getPosition() * (*matrix);
            else lightpos = light->getPosition();

            selectLight = light;
        }
    }

    osg::Matrix eyeToWorld;
    eyeToWorld.invert(*cv.getModelViewMatrix());

    lightpos = lightpos * eyeToWorld;

    if (selectLight)
    {
        // get the bounds of the model.
        osg::ComputeBoundsVisitor cbbv(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);
        cbbv.setTraversalMask(getShadowedScene()->getCastsShadowTraversalMask());

        _shadowedScene->osg::Group::traverse(cbbv);

        osg::BoundingBox bb = cbbv.getBoundingBox();


        osg::Vec3 position;

        if (lightpos[3]!=0.0)
        {
            position.set(lightpos.x(), lightpos.y(), lightpos.z());
        }
        else
        {
            // make an orthographic projection
            osg::Vec3 lightDir(lightpos.x(), lightpos.y(), lightpos.z());
            lightDir.normalize();

            // set the position far away along the light direction
            position = lightDir * bb.radius()  * 20;
        }

        float centerDistance = (position-bb.center()).length();

        float znear = centerDistance-bb.radius();
        float zfar  = centerDistance+bb.radius();
        float zNearRatio = 0.001f;
        if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;

        float top   = bb.radius();
        float right = top;

        _camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
        _camera->setProjectionMatrixAsOrtho(-right, right, -top, top, znear, zfar);
        _camera->setViewMatrixAsLookAt(position, bb.center(), osg::Vec3(0.0f,1.0f,0.0f));

        // compute the matrix which takes a vertex from local coords into tex coords
        // will use this later to specify osg::TexGen..
        osg::Matrix MVPT = _camera->getViewMatrix() *
                           _camera->getProjectionMatrix() *
                           osg::Matrix::translate(1.0,1.0,1.0) *
                           osg::Matrix::scale(0.5,0.5,0.5+_bias);

        _texgen->setMode(osg::TexGen::EYE_LINEAR);
        _texgen->setPlanesFromMatrix(MVPT);

        cv.setTraversalMask( traversalMask &
                             getShadowedScene()->getCastsShadowTraversalMask() );

        // do RTT camera traversal
        _camera->accept(cv);

        orig_rs->getPositionalStateContainer()->addPositionedTextureAttribute(_textureUnit, cv.getModelViewMatrix(), _texgen.get());
    }


    // reapply the original traversal mask
    cv.setTraversalMask( traversalMask );
}

void SoftShadowMap::cleanSceneGraph()
{
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

    ss->setTextureAttributeAndModes((int)_textureUnit + 1, texture3D, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    ss->setTextureMode((int)_textureUnit + 1,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
    ss->setTextureMode((int)_textureUnit + 1,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
    ss->setTextureMode((int)_textureUnit + 1,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);

}

