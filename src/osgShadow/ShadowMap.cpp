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

#include <osgShadow/ShadowMap>
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

#define IMPROVE_TEXGEN_PRECISION 1

//////////////////////////////////////////////////////////////////
// fragment shader
//
static const char fragmentShaderSource_noBaseTexture[] =
    "uniform sampler2DShadow osgShadow_shadowTexture; \n"
    "uniform vec2 osgShadow_ambientBias; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "    gl_FragColor = gl_Color * (osgShadow_ambientBias.x + shadow2DProj( osgShadow_shadowTexture, gl_TexCoord[0] ) * osgShadow_ambientBias.y); \n"
    "}\n";

//////////////////////////////////////////////////////////////////
// fragment shader
//
static const char fragmentShaderSource_withBaseTexture[] =
    "uniform sampler2D osgShadow_baseTexture; \n"
    "uniform sampler2DShadow osgShadow_shadowTexture; \n"
    "uniform vec2 osgShadow_ambientBias; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "    vec4 color = gl_Color * texture2D( osgShadow_baseTexture, gl_TexCoord[0].xy ); \n"
    "    gl_FragColor = color * (osgShadow_ambientBias.x + shadow2DProj( osgShadow_shadowTexture, gl_TexCoord[1] ) * osgShadow_ambientBias.y); \n"
    "}\n";

//////////////////////////////////////////////////////////////////
// fragment shader
//
static const char fragmentShaderSource_debugHUD[] =
    "uniform sampler2D osgShadow_shadowTexture; \n"
    " \n"
    "void main(void) \n"
    "{ \n"
    "   vec4 texResult = texture2D(osgShadow_shadowTexture, gl_TexCoord[0].st ); \n"
    "   float value = texResult.r; \n"
    "   gl_FragColor = vec4( value, value, value, 0.8 ); \n"
    "} \n";

ShadowMap::ShadowMap():
    _baseTextureUnit(0),
    _shadowTextureUnit(1),
    _polyOffset(1.0,1.0),
    _ambientBias(0.5f,0.5f),
    _textureSize(1024,1024)
{
}

ShadowMap::ShadowMap(const ShadowMap& copy, const osg::CopyOp& copyop):
ShadowTechnique(copy,copyop),
    _baseTextureUnit(copy._baseTextureUnit),
    _shadowTextureUnit(copy._shadowTextureUnit),
    _polyOffset(copy._polyOffset),
    _ambientBias(copy._ambientBias),
    _textureSize(copy._textureSize)
{
}

void ShadowMap::resizeGLObjectBuffers(unsigned int maxSize)
{
    osg::resizeGLObjectBuffers(_camera, maxSize);
    osg::resizeGLObjectBuffers(_texgen, maxSize);
    osg::resizeGLObjectBuffers(_texture, maxSize);
    osg::resizeGLObjectBuffers(_stateset, maxSize);
    osg::resizeGLObjectBuffers(_program, maxSize);

    osg::resizeGLObjectBuffers(_ls, maxSize);

    for(ShaderList::iterator itr = _shaderList.begin();
        itr != _shaderList.end();
        ++itr)
    {
        osg::resizeGLObjectBuffers(*itr, maxSize);
    }
}

void ShadowMap::releaseGLObjects(osg::State* state) const
{
    osg::releaseGLObjects(_camera, state);
    osg::releaseGLObjects(_texgen, state);
    osg::releaseGLObjects(_texture, state);
    osg::releaseGLObjects(_stateset, state);
    osg::releaseGLObjects(_program, state);

    osg::releaseGLObjects(_ls, state);

    for(ShaderList::const_iterator itr = _shaderList.begin();
        itr != _shaderList.end();
        ++itr)
    {
        osg::releaseGLObjects(*itr, state);
    }
}

void ShadowMap::setTextureUnit(unsigned int unit)
{
    _shadowTextureUnit = unit;
}

void ShadowMap::setPolygonOffset(const osg::Vec2& polyOffset)
{
    _polyOffset = polyOffset;
}

void ShadowMap::setAmbientBias(const osg::Vec2& ambientBias)
{
    _ambientBias = ambientBias;
    if (_ambientBiasUniform.valid()) _ambientBiasUniform->set(_ambientBias);
}

void ShadowMap::setTextureSize(const osg::Vec2s& textureSize)
{
    _textureSize = textureSize;
    dirty();
}

void ShadowMap::setLight(osg::Light* light)
{
    _light = light;
}


void ShadowMap::setLight(osg::LightSource* ls)
{
    _ls = ls;
    _light = _ls->getLight();
}

void ShadowMap::createUniforms()
{
    _uniformList.clear();

    osg::Uniform* baseTextureSampler = new osg::Uniform("osgShadow_baseTexture",(int)_baseTextureUnit);
    _uniformList.push_back(baseTextureSampler);

    osg::Uniform* shadowTextureSampler = new osg::Uniform("osgShadow_shadowTexture",(int)_shadowTextureUnit);
    _uniformList.push_back(shadowTextureSampler);

    _ambientBiasUniform = new osg::Uniform("osgShadow_ambientBias",_ambientBias);
    _uniformList.push_back(_ambientBiasUniform.get());

}

void ShadowMap::createShaders()
{
    // if we are not given shaders, use the default
    if( _shaderList.empty() )
    {
        if (_shadowTextureUnit==0)
        {
            osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource_noBaseTexture);
            _shaderList.push_back(fragment_shader);
        }
        else
        {
            osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource_withBaseTexture);
            _shaderList.push_back(fragment_shader);

        }
    }
}

void ShadowMap::init()
{
    if (!_shadowedScene) return;

    _texture = new osg::Texture2D;
    _texture->setTextureSize(_textureSize.x(), _textureSize.y());
    _texture->setInternalFormat(GL_DEPTH_COMPONENT);
    _texture->setShadowComparison(true);
    _texture->setShadowTextureMode(osg::Texture2D::LUMINANCE);
    _texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    _texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

    // the shadow comparison should fail if object is outside the texture
    _texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
    _texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
    _texture->setBorderColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

    // set up the render to texture camera.
    {
        // create the camera
        _camera = new osg::Camera;

        _camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF_INHERIT_VIEWPOINT);

        _camera->setCullCallback(new CameraCullCallback(this));

        _camera->setClearMask(GL_DEPTH_BUFFER_BIT);
        //_camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        _camera->setClearColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
        _camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);

        // set viewport
        _camera->setViewport(0,0,_textureSize.x(),_textureSize.y());

        // set the camera to render before the main camera.
        _camera->setRenderOrder(osg::Camera::PRE_RENDER);

        // tell the camera to use OpenGL frame buffer object where supported.
        _camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
        //_camera->setRenderTargetImplementation(osg::Camera::SEPERATE_WINDOW);

        // attach the texture and use it as the color buffer.
        _camera->attach(osg::Camera::DEPTH_BUFFER, _texture.get());

        osg::StateSet* stateset = _camera->getOrCreateStateSet();


#if 1
        // cull front faces so that only backfaces contribute to depth map


        osg::ref_ptr<osg::CullFace> cull_face = new osg::CullFace;
        cull_face->setMode(osg::CullFace::FRONT);
        stateset->setAttribute(cull_face.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        stateset->setMode(GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        // negative polygonoffset - move the backface nearer to the eye point so that backfaces
        // shadow themselves
        float factor = -_polyOffset[0];
        float units =  -_polyOffset[1];

        osg::ref_ptr<osg::PolygonOffset> polygon_offset = new osg::PolygonOffset;
        polygon_offset->setFactor(factor);
        polygon_offset->setUnits(units);
        stateset->setAttribute(polygon_offset.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        stateset->setMode(GL_POLYGON_OFFSET_FILL, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
#else
        // disabling cull faces so that only front and backfaces contribute to depth map
        stateset->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

        // negative polygonoffset - move the backface nearer to the eye point
        // so that front faces do not shadow themselves.
        float factor = _polyOffset[0];
        float units =  _polyOffset[1];

        osg::ref_ptr<osg::PolygonOffset> polygon_offset = new osg::PolygonOffset;
        polygon_offset->setFactor(factor);
        polygon_offset->setUnits(units);
        stateset->setAttribute(polygon_offset.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        stateset->setMode(GL_POLYGON_OFFSET_FILL, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
#endif
    }

    {
        _stateset = new osg::StateSet;
        _stateset->setTextureAttributeAndModes(_shadowTextureUnit,_texture.get(),osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        _stateset->setTextureMode(_shadowTextureUnit,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
        _stateset->setTextureMode(_shadowTextureUnit,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
        _stateset->setTextureMode(_shadowTextureUnit,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);
        _stateset->setTextureMode(_shadowTextureUnit,GL_TEXTURE_GEN_Q,osg::StateAttribute::ON);

        _texgen = new osg::TexGen;

        // add Program, when empty of Shaders then we are using fixed functionality
        _program = new osg::Program;
        _stateset->setAttribute(_program.get());

        // create default shaders if needed
        createShaders();

        // add the shader list to the program
        for(ShaderList::const_iterator itr=_shaderList.begin();
            itr!=_shaderList.end();
            ++itr)
        {
            _program->addShader(itr->get());
        }

        // create own uniforms
        createUniforms();

        // add the uniform list to the stateset
        for(UniformList::const_iterator itr=_uniformList.begin();
            itr!=_uniformList.end();
            ++itr)
        {
            _stateset->addUniform(itr->get());
        }

        {
            // fake texture for baseTexture, add a fake texture
            // we support by default at least one texture layer
            // without this fake texture we can not support
            // textured and not textured scene

            // TODO: at the moment the PSSM supports just one texture layer in the GLSL shader, multitexture are
            //       not yet supported !

            osg::Image* image = new osg::Image;
            // allocate the image data, noPixels x 1 x 1 with 4 rgba floats - equivalent to a Vec4!
            int noPixels = 1;
            image->allocateImage(noPixels,1,1,GL_RGBA,GL_FLOAT);
            image->setInternalTextureFormat(GL_RGBA);
            // fill in the image data.
            osg::Vec4* dataPtr = (osg::Vec4*)image->data();
            osg::Vec4 color(1,1,1,1);
            *dataPtr = color;
            // make fake texture
            osg::Texture2D* fakeTex = new osg::Texture2D;
            fakeTex->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_EDGE);
            fakeTex->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_EDGE);
            fakeTex->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
            fakeTex->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
            fakeTex->setImage(image);
            // add fake texture
            _stateset->setTextureAttribute(_baseTextureUnit,fakeTex,osg::StateAttribute::ON);
            _stateset->setTextureMode(_baseTextureUnit,GL_TEXTURE_2D,osg::StateAttribute::ON);
            _stateset->setTextureMode(_baseTextureUnit,GL_TEXTURE_3D,osg::StateAttribute::OFF);
            #if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GLES3_AVAILABLE)
                _stateset->setTextureMode(_baseTextureUnit,GL_TEXTURE_1D,osg::StateAttribute::OFF);
            #endif
        }
    }

    _dirty = false;
}


void ShadowMap::update(osg::NodeVisitor& nv)
{
    _shadowedScene->osg::Group::traverse(nv);
}

void ShadowMap::cull(osgUtil::CullVisitor& cv)
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
    osg::Vec3 lightDir;

    //MR testing giving a specific light
    osgUtil::PositionalStateContainer::AttrMatrixList& aml = orig_rs->getPositionalStateContainer()->getAttrMatrixList();
    for(osgUtil::PositionalStateContainer::AttrMatrixList::iterator itr = aml.begin();
        itr != aml.end();
        ++itr)
    {
        const osg::Light* light = dynamic_cast<const osg::Light*>(itr->first.get());
        if (light)
        {
            if( _light.valid()) {
                if( _light.get() == light )
                    selectLight = light;
                else
                    continue;
            }
            else
                selectLight = light;

            osg::RefMatrix* matrix = itr->second.get();
            if (matrix)
            {
                lightpos = light->getPosition() * (*matrix);
                lightDir = osg::Matrix::transform3x3( light->getDirection(), *matrix );
            }
            else
            {
                lightpos = light->getPosition();
                lightDir = light->getDirection();
            }

        }
    }

    osg::Matrix eyeToWorld;
    eyeToWorld.invert(*cv.getModelViewMatrix());

    lightpos = lightpos * eyeToWorld;
    lightDir = osg::Matrix::transform3x3( lightDir, eyeToWorld );
    lightDir.normalize();

    if (selectLight)
    {

        // set to ambient on light to black so that the ambient bias uniform can take it's affect
        const_cast<osg::Light*>(selectLight)->setAmbient(osg::Vec4(0.0f,0.0f,0.0f,1.0f));

        //std::cout<<"----- VxOSG::ShadowMap selectLight spot cutoff "<<selectLight->getSpotCutoff()<<std::endl;

        float fov = selectLight->getSpotCutoff() * 2;
        if(fov < 180.0f)   // spotlight, then we don't need the bounding box
        {
            osg::Vec3 position(lightpos.x(), lightpos.y(), lightpos.z());
            _camera->setProjectionMatrixAsPerspective(fov, 1.0, 0.1, 1000.0);
            _camera->setViewMatrixAsLookAt(position,position+lightDir,computeOrthogonalVector(lightDir));
        }
        else
        {
            // get the bounds of the model.
            osg::ComputeBoundsVisitor cbbv(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);
            cbbv.setTraversalMask(getShadowedScene()->getCastsShadowTraversalMask());

            _shadowedScene->osg::Group::traverse(cbbv);

            osg::BoundingBox bb = cbbv.getBoundingBox();

            if (lightpos[3]!=0.0)   // point light
            {
                osg::Vec3 position(lightpos.x(), lightpos.y(), lightpos.z());

                float centerDistance = (position-bb.center()).length();

                float znear = centerDistance-bb.radius();
                float zfar  = centerDistance+bb.radius();
                float zNearRatio = 0.001f;
                if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;

                float top   = (bb.radius()/centerDistance)*znear;
                float right = top;

                _camera->setProjectionMatrixAsFrustum(-right,right,-top,top,znear,zfar);
                _camera->setViewMatrixAsLookAt(position,bb.center(),computeOrthogonalVector(bb.center()-position));
            }
            else    // directional light
            {
                // make an orthographic projection
                osg::Vec3 ortho_lightDir(lightpos.x(), lightpos.y(), lightpos.z());
                ortho_lightDir.normalize();

                // set the position far away along the light direction
                osg::Vec3 position = bb.center() + ortho_lightDir * bb.radius() * 2;

                float centerDistance = (position-bb.center()).length();

                float znear = centerDistance-bb.radius();
                float zfar  = centerDistance+bb.radius();
                float zNearRatio = 0.001f;
                if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;

                float top   = bb.radius();
                float right = top;

                _camera->setProjectionMatrixAsOrtho(-right, right, -top, top, znear, zfar);
                _camera->setViewMatrixAsLookAt(position,bb.center(),computeOrthogonalVector(ortho_lightDir));
            }


        }

        cv.setTraversalMask( traversalMask &
            getShadowedScene()->getCastsShadowTraversalMask() );

        // do RTT camera traversal
        _camera->accept(cv);

        _texgen->setMode(osg::TexGen::EYE_LINEAR);

#if IMPROVE_TEXGEN_PRECISION
        // compute the matrix which takes a vertex from local coords into tex coords
        // We actually use two matrices one used to define texgen
        // and second that will be used as modelview when appling to OpenGL
        _texgen->setPlanesFromMatrix( _camera->getProjectionMatrix() *
                                      osg::Matrix::translate(1.0,1.0,1.0) *
                                      osg::Matrix::scale(0.5f,0.5f,0.5f) );

        // Place texgen with modelview which removes big offsets (making it float friendly)
        osg::RefMatrix * refMatrix = new osg::RefMatrix
            ( _camera->getInverseViewMatrix() * *cv.getModelViewMatrix() );

        cv.getRenderStage()->getPositionalStateContainer()->
             addPositionedTextureAttribute( _shadowTextureUnit, refMatrix, _texgen.get() );
#else
        // compute the matrix which takes a vertex from local coords into tex coords
        // will use this later to specify osg::TexGen..
        osg::Matrix MVPT = _camera->getViewMatrix() *
               _camera->getProjectionMatrix() *
               osg::Matrix::translate(1.0,1.0,1.0) *
               osg::Matrix::scale(0.5f,0.5f,0.5f);

        _texgen->setPlanesFromMatrix(MVPT);

        orig_rs->getPositionalStateContainer()->addPositionedTextureAttribute(_shadowTextureUnit, cv.getModelViewMatrix(), _texgen.get());
#endif
    } // if(selectLight)


    // reapply the original traversal mask
    cv.setTraversalMask( traversalMask );
}

void ShadowMap::cleanSceneGraph()
{
}

///////////////////// Debug Methods

////////////////////////////////////////////////////////////////////////////////
// Callback used by debugging hud to display Shadow Map in color buffer
// OSG does not allow to use the same GL Texture Id with different glTexParams.
// Callback simply turns shadow compare mode off via GL while rendering hud and
// restores it afterwards.
////////////////////////////////////////////////////////////////////////////////
class ShadowMap::DrawableDrawWithDepthShadowComparisonOffCallback:
    public osg::Drawable::DrawCallback
{
public:
    //
    DrawableDrawWithDepthShadowComparisonOffCallback
        ( osg::Texture2D * texture, unsigned stage = 0 )
            : _texture( texture ), _stage( stage )
    {
    }

    virtual void drawImplementation
        ( osg::RenderInfo & ri,const osg::Drawable* drawable ) const
    {
        if( _texture.valid() ) {
            // make sure proper texture is currently applied
            ri.getState()->applyTextureAttribute( _stage, _texture.get() );

            // Turn off depth comparison mode
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB,
                             GL_NONE );
        }

        drawable->drawImplementation(ri);

        if( _texture.valid() ) {
            // Turn it back on
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB,
                             GL_COMPARE_R_TO_TEXTURE_ARB );
        }
    }

    osg::ref_ptr< osg::Texture2D > _texture;
    unsigned                       _stage;
};

////////////////////////////////////////////////////////////////////////////////
osg::ref_ptr<osg::Camera> ShadowMap::makeDebugHUD()
{
    // Make sure we attach initialized texture to HUD
    if( !_texture.valid() )    init();

    osg::ref_ptr<osg::Camera> camera = new osg::Camera;

    osg::Vec2 size(1280, 1024);
    // set the projection matrix
    camera->setProjectionMatrix(osg::Matrix::ortho2D(0,size.x(),0,size.y()));

    // set the view matrix
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(osg::Matrix::identity());

    // only clear the depth buffer
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);
    camera->setClearColor(osg::Vec4(0.2f, 0.3f, 0.5f, 0.2f));
    //camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // draw subgraph after main camera view.
    camera->setRenderOrder(osg::Camera::POST_RENDER);

    // we don't want the camera to grab event focus from the viewers main camera(s).
    camera->setAllowEventFocus(false);

    osg::Geode* geode = new osg::Geode;

    osg::Vec3 position(10.0f,size.y()-100.0f,0.0f);
    osg::Vec3 delta(0.0f,-120.0f,0.0f);
    float length = 300.0f;

    // turn the text off to avoid linking with osgText
#if 0
    std::string timesFont("fonts/arial.ttf");

    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );

        text->setFont(timesFont);
        text->setPosition(position);
        text->setText("Shadow Map HUD");

        position += delta;
    }
#endif

    osg::Vec3 widthVec(length, 0.0f, 0.0f);
    osg::Vec3 depthVec(0.0f,length, 0.0f);
    osg::Vec3 centerBase( 10.0f + length/2, size.y()-length/2, 0.0f);
    centerBase += delta;

    osg::Geometry *geometry = osg::createTexturedQuadGeometry
        ( centerBase-widthVec*0.5f-depthVec*0.5f, widthVec, depthVec );

    geode->addDrawable( geometry );

    geometry->setDrawCallback
        ( new DrawableDrawWithDepthShadowComparisonOffCallback( _texture.get() ) );

    osg::StateSet* stateset = geode->getOrCreateStateSet();

    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
    stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    // test with regular texture
    //stateset->setTextureAttributeAndModes(0, new osg::Texture2D(osgDB::readImageFile("Images/lz.rgb")));

    stateset->setTextureAttributeAndModes(0,_texture.get(),osg::StateAttribute::ON);

    //test to check the texture coordinates generated during shadow pass
#if 0
    stateset->setTextureMode(_shadowTextureUnit,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
    stateset->setTextureMode(_shadowTextureUnit,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
    stateset->setTextureMode(_shadowTextureUnit,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);
    stateset->setTextureMode(_shadowTextureUnit,GL_TEXTURE_GEN_Q,osg::StateAttribute::ON);

    // create TexGen node
    osg::ref_ptr<osg::TexGenNode> texGenNode = new osg::TexGenNode;
    texGenNode->setTextureUnit(_shadowTextureUnit);
    texGenNode->setTexGen(_texgen.get());
    camera->addChild(texGenNode.get());
#endif
    //shader for correct display

    osg::ref_ptr<osg::Program> program = new osg::Program;
    stateset->setAttribute(program.get());

    osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource_debugHUD);
    program->addShader(fragment_shader);

    camera->addChild(geode);

    return camera;
}

//////////////////////// End Debug Section

