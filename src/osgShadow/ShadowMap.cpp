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


//////////////////////////////////////////////////////////////////
// fragment shader
//
static const char fragmentShaderSource_noBaseTexture[] = 
    "uniform sampler2DShadow shadowTexture; \n"
    "uniform vec2 ambientBias; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "    gl_FragColor = gl_Color * (ambientBias.x + shadow2DProj( shadowTexture, gl_TexCoord[0] ) * ambientBias.y); \n"
    "}\n";

//////////////////////////////////////////////////////////////////
// fragment shader
//
static const char fragmentShaderSource_withBaseTexture[] = 
    "uniform sampler2D baseTexture; \n"
    "uniform sampler2DShadow shadowTexture; \n"
    "uniform vec2 ambientBias; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "    vec4 color = gl_Color * texture2D( baseTexture, gl_TexCoord[0].xy ); \n"
    "    gl_FragColor = color * (ambientBias.x + shadow2DProj( shadowTexture, gl_TexCoord[1] ) * ambientBias.y); \n"
    "}\n";

ShadowMap::ShadowMap():
    _textureUnit(1)
{
    osg::notify(osg::NOTICE)<<"Warning: osgShadow::ShadowMap technique is in development."<<std::endl;
}

ShadowMap::ShadowMap(const ShadowMap& copy, const osg::CopyOp& copyop):
    ShadowTechnique(copy,copyop),
    _textureUnit(copy._textureUnit)
{
}

void ShadowMap::setTextureUnit(unsigned int unit)
{
    _textureUnit = unit;
}

void ShadowMap::init()
{
    if (!_shadowedScene) return;

    unsigned int tex_width = 1024;
    unsigned int tex_height = 1024;
    
    _texture = new osg::Texture2D;
    _texture->setTextureSize(tex_width, tex_height);
    _texture->setInternalFormat(GL_DEPTH_COMPONENT);
    _texture->setShadowComparison(true);
    _texture->setShadowTextureMode(osg::Texture2D::LUMINANCE);
    _texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    _texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

    // set up the render to texture camera.
    {
        // create the camera
        _camera = new osg::Camera;

        _camera->setCullCallback(new CameraCullCallback(this));

        _camera->setClearMask(GL_DEPTH_BUFFER_BIT);
        //_camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        _camera->setClearColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
        _camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);

        // set viewport
        _camera->setViewport(0,0,tex_width,tex_height);

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

#if 1
        osg::Program* program = new osg::Program;
        _stateset->setAttribute(program);

        if (_textureUnit==0)
        {
            osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource_noBaseTexture);
            program->addShader(fragment_shader);

            osg::Uniform* shadowTextureSampler = new osg::Uniform("shadowTexture",(int)_textureUnit);
            _stateset->addUniform(shadowTextureSampler);
        }
        else
        {
            osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource_withBaseTexture);
            program->addShader(fragment_shader);

            osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
            _stateset->addUniform(baseTextureSampler);

            osg::Uniform* shadowTextureSampler = new osg::Uniform("shadowTexture",(int)_textureUnit);
            _stateset->addUniform(shadowTextureSampler);
        }
        
        osg::Uniform* ambientBias = new osg::Uniform("ambientBias",osg::Vec2(0.3f,1.2f));
        _stateset->addUniform(ambientBias);

#endif
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

    // do traversal of shadow recieving scene which does need to be decorated by the shadow map
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
        
        if (lightpos[3]!=0.0)
        {
            osg::Vec3 position(lightpos.x(), lightpos.y(), lightpos.z());

            float centerDistance = (position-bb.center()).length();

            float znear = centerDistance-bb.radius();
            float zfar  = centerDistance+bb.radius();
            float zNearRatio = 0.001f;
            if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;

            float top   = (bb.radius()/centerDistance)*znear;
            float right = top;

            _camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
            _camera->setProjectionMatrixAsFrustum(-right,right,-top,top,znear,zfar);
            _camera->setViewMatrixAsLookAt(position,bb.center(),osg::Vec3(0.0f,1.0f,0.0f));
            

            // compute the matrix which takes a vertex from local coords into tex coords
            // will use this later to specify osg::TexGen..
            osg::Matrix MVPT = _camera->getViewMatrix() * 
                               _camera->getProjectionMatrix() *
                               osg::Matrix::translate(1.0,1.0,1.0) *
                               osg::Matrix::scale(0.5f,0.5f,0.5f);
                               
            _texgen->setMode(osg::TexGen::EYE_LINEAR);
            _texgen->setPlanesFromMatrix(MVPT);
        }
        else
        {
            // make an orthographic projection
            osg::Vec3 lightDir(lightpos.x(), lightpos.y(), lightpos.z());
            lightDir.normalize();

            // set the position far away along the light direction
            osg::Vec3 position = lightDir * bb.radius()  * 20;

            float centerDistance = (position-bb.center()).length();

            float znear = centerDistance-bb.radius();
            float zfar  = centerDistance+bb.radius();
            float zNearRatio = 0.001f;
            if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;

            float top   = bb.radius();
            float right = top;

            _camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
            _camera->setProjectionMatrixAsOrtho(-right, right, -top, top, znear, zfar);
            _camera->setViewMatrixAsLookAt(position,bb.center(),osg::Vec3(0.0f,1.0f,0.0f));
            

            // compute the matrix which takes a vertex from local coords into tex coords
            // will use this later to specify osg::TexGen..
            osg::Matrix MVPT = _camera->getViewMatrix() * 
                               _camera->getProjectionMatrix() *
                               osg::Matrix::translate(1.0,1.0,1.0) *
                               osg::Matrix::scale(0.5f,0.5f,0.5f);
                               
            _texgen->setMode(osg::TexGen::EYE_LINEAR);
            _texgen->setPlanesFromMatrix(MVPT);
        }


        cv.setTraversalMask( traversalMask & 
                             getShadowedScene()->getCastsShadowTraversalMask() );

        // do RTT camera traversal
        _camera->accept(cv);

        orig_rs->getPositionalStateContainer()->addPositionedTextureAttribute(_textureUnit, cv.getModelViewMatrix(), _texgen.get());
    }


    // reapply the original traversal mask
    cv.setTraversalMask( traversalMask );
}

void ShadowMap::cleanSceneGraph()
{
}
