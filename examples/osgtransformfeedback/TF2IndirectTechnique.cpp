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

#include "TF2IndirectTechnique"
#include "GPUScene"
#include <osg/Notify>
#include <osg/ComputeBoundsVisitor>
#include <osg/PolygonOffset>
#include <osg/CullFace>
#include <osg/io_utils>

//using namespace osgShadow;

#include <iostream>
//for debug
#include <osg/LightSource>
#include <osg/PolygonMode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgText/Text>

#define IMPROVE_TEXGEN_PRECISION 1

//////////////////////////////////////////////////////////////////
// fragment shader
//
///store dc buffer offset at pos.w and dc size at extents.w
static const char vertexShaderSource_Cull[] =
        "layout(index=0) vec4 position;\n"
        "layout(index=1) vec4 rotation;\n"
    "layout(index=2) vec4 extents;\n"
    "bool boundingBoxInViewFrustum( in mat4 matrix, in vec3 bbMin, in vec3 bbMax )\n"
    "{\n"
    "    vec4 BoundingBox[8];\n"
    "    BoundingBox[0] = matrix * vec4( bbMax.x, bbMax.y, bbMax.z, 1.0);\n"
    "    BoundingBox[1] = matrix * vec4( bbMin.x, bbMax.y, bbMax.z, 1.0);\n"
    "    BoundingBox[2] = matrix * vec4( bbMax.x, bbMin.y, bbMax.z, 1.0);\n"
    "    BoundingBox[3] = matrix * vec4( bbMin.x, bbMin.y, bbMax.z, 1.0);\n"
    "    BoundingBox[4] = matrix * vec4( bbMax.x, bbMax.y, bbMin.z, 1.0);\n"
    "    BoundingBox[5] = matrix * vec4( bbMin.x, bbMax.y, bbMin.z, 1.0);\n"
    "    BoundingBox[6] = matrix * vec4( bbMax.x, bbMin.y, bbMin.z, 1.0);\n"
    "    BoundingBox[7] = matrix * vec4( bbMin.x, bbMin.y, bbMin.z, 1.0);\n"
    "\n"
    "    int outOfBound[6] = int[6]( 0, 0, 0, 0, 0, 0 );\n"
    "    for (int i=0; i<8; i++)\n"
    "    {\n"
    "        outOfBound[0] += int( BoundingBox[i].x >  BoundingBox[i].w );\n"
    "        outOfBound[1] += int( BoundingBox[i].x < -BoundingBox[i].w );\n"
    "        outOfBound[2] += int( BoundingBox[i].y >  BoundingBox[i].w );\n"
    "        outOfBound[3] += int( BoundingBox[i].y < -BoundingBox[i].w );\n"
    "        outOfBound[4] += int( BoundingBox[i].z >  BoundingBox[i].w );\n"
    "        outOfBound[5] += int( BoundingBox[i].z < -BoundingBox[i].w );\n"
    "    }\n"
    "    return (outOfBound[0] < 8 ) && ( outOfBound[1] < 8 ) && ( outOfBound[2] < 8 ) && ( outOfBound[3] < 8 ) && ( outOfBound[4] < 8 ) && ( outOfBound[5] < 8 );\n"
    "}\n"

    "\n"
    "void main(void) \n"
    "{ \n"
        "gl_ModelViewProjectionMatrix"
    "    gl_FragColor = gl_Color * (osgShadow_ambientBias.x + shadow2DProj( osgShadow_shadowTexture, gl_TexCoord[0] ) * osgShadow_ambientBias.y); \n"

    "}\n";

static const char geometryShaderSource_Cull[] =
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

TF2IndirectTechnique::TF2IndirectTechnique():
    _baseTextureUnit(0),
    _shadowTextureUnit(1),
    _polyOffset(1.0,1.0),
    _ambientBias(0.5f,0.5f),
    _textureSize(1024,1024)
{
}

TF2IndirectTechnique::TF2IndirectTechnique(const TF2IndirectTechnique& copy, const osg::CopyOp& copyop):
GPUDrawTechnique(copy,copyop),
    _baseTextureUnit(copy._baseTextureUnit),
    _shadowTextureUnit(copy._shadowTextureUnit),
    _polyOffset(copy._polyOffset),
    _ambientBias(copy._ambientBias),
    _textureSize(copy._textureSize)
{
}

void TF2IndirectTechnique::resizeGLObjectBuffers(unsigned int maxSize)
{
    osg::resizeGLObjectBuffers(_camera, maxSize);
    osg::resizeGLObjectBuffers(_texgen, maxSize);
    osg::resizeGLObjectBuffers(_texture, maxSize);
    osg::resizeGLObjectBuffers(_stateset, maxSize);
    osg::resizeGLObjectBuffers(_cullProgram, maxSize);

    osg::resizeGLObjectBuffers(_ls, maxSize);

    for(ShaderList::iterator itr = _shaderList.begin();
        itr != _shaderList.end();
        ++itr)
    {
        osg::resizeGLObjectBuffers(*itr, maxSize);
    }
}

void TF2IndirectTechnique::releaseGLObjects(osg::State* state) const
{
    osg::releaseGLObjects(_camera, state);
    osg::releaseGLObjects(_texgen, state);
    osg::releaseGLObjects(_texture, state);
    osg::releaseGLObjects(_stateset, state);
    osg::releaseGLObjects(_cullProgram, state);

    osg::releaseGLObjects(_ls, state);

    for(ShaderList::const_iterator itr = _shaderList.begin();
        itr != _shaderList.end();
        ++itr)
    {
        osg::releaseGLObjects(*itr, state);
    }
}
/*
void TF2IndirectTechnique::setTextureUnit(unsigned int unit)
{
    _shadowTextureUnit = unit;
}

void TF2IndirectTechnique::setPolygonOffset(const osg::Vec2& polyOffset)
{
    _polyOffset = polyOffset;
}

void TF2IndirectTechnique::setAmbientBias(const osg::Vec2& ambientBias)
{
    _ambientBias = ambientBias;
    if (_ambientBiasUniform.valid()) _ambientBiasUniform->set(_ambientBias);
}

void TF2IndirectTechnique::setTextureSize(const osg::Vec2s& textureSize)
{
    _textureSize = textureSize;
    dirty();
}

void TF2IndirectTechnique::setLight(osg::Light* light)
{
    _light = light;
}


void TF2IndirectTechnique::setLight(osg::LightSource* ls)
{
    _ls = ls;
    _light = _ls->getLight();
}
*/
void TF2IndirectTechnique::createUniforms()
{
    _uniformList.clear();

    osg::Uniform* baseTextureSampler = new osg::Uniform("osgShadow_baseTexture",(int)_baseTextureUnit);
    _uniformList.push_back(baseTextureSampler);

    osg::Uniform* shadowTextureSampler = new osg::Uniform("osgShadow_shadowTexture",(int)_shadowTextureUnit);
    _uniformList.push_back(shadowTextureSampler);

    _ambientBiasUniform = new osg::Uniform("osgShadow_ambientBias",_ambientBias);
    _uniformList.push_back(_ambientBiasUniform.get());

}

void TF2IndirectTechnique::createShaders()
{
    // if we are not given shaders, use the default
    if( _shaderList.empty() )
    {

            osg::Shader* fragment_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource_Cull);
            _shaderList.push_back(fragment_shader);

            osg::Shader* geometry_shader = new osg::Shader(osg::Shader::GEOMETRY, geometryShaderSource_Cull);
            _shaderList.push_back(geometry_shader);


    }
}



///Parse ScebnGRaph count geom and add in position
class TF2IndirectTechnique::UpdaterTraversalVisitor : public osg::NodeVisitor{
public:
    UpdaterTraversalVisitor( osg::Geometry *t):osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN){
        _geomcounter=0;
        _target=t;
       targetpos=(osg::Vec4Array * )_target->getVertexAttribArray(0);
    targetextents=(osg::Vec4Array * )_target->getVertexAttribArray(1);
    }

    virtual void apply(osg::MatrixTransform &ge){

        //push_matrix
        if(_matrixstack.empty())_matrixstack.push_back(ge.getMatrix());
        else {
            _matrixstack.push_back(_matrixstack.back()*ge.getMatrix());
        }

      ge.osg::Group::accept(*this);
        //popmatrix
        _matrixstack.pop_back();


    }
    virtual void apply(osg::PositionAttitudeTransform &ge){
    ///TODO
    }

    virtual void apply(osg::Geode &ge){ osg::Vec3 v;
   for(unsigned int i=0;i<ge.getNumChildren();i++){

       if(check=dynamic_cast<osg::Geometry*>(ge.getChild(i))){
        _geomcounter++;

 osg::Matrix m=_matrixstack.back();
 m.translate(check->getBoundingBox().center() );
v=m.getTrans();
      targetpos ->push_back(osg::Vec4(v[0],v[1],v[2],1) );
      v=(check->getBoundingBox()._max-check->getBoundingBox()._min)*0.5 ;
       targetextents->push_back( osg::Vec4(v[0],v[1],v[2],1));
}
        //

       }

            }
    unsigned int _geomcounter;
    osg::Geometry * check,*_target;

    osg::Vec4Array * targetpos;
    osg::Vec4Array * targetextents;
    std::list<osg::Matrix> _matrixstack;
    //osg::Matrix _current;
};

void TF2IndirectTechnique::init()
{
    if (!_GPUScene) return;


  _cullingData=new osg::Geometry();
_indirectGeometry=new osg::Geometry();

    ///arrays (and so on BO) setted as the first traversed geometry(ensure bos shared)
    ///primset as DrawIndirectpate
 ///cardinality set on update traversal
    _cullingData->addPrimitiveSet(new osg::DrawArrays(GL_POINTS));
    _cullingData->setVertexAttribArray(0,new osg::Vec4Array());
    _cullingData->setVertexAttribArray(1,new osg::Vec4Array());

        _stateset = _cullingData->getOrCreateStateSet();

        // add Program, when empty of Shaders then we are using fixed functionality
        _cullProgram = new osg::Program;
        _stateset->setAttribute(_cullProgram.get());

        // create default shaders if needed
        createShaders();

        // add the shader list to the program
        for(ShaderList::const_iterator itr=_shaderList.begin();
            itr!=_shaderList.end();
            ++itr)
        {
            _cullProgram->addShader(itr->get());
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



_updateTraverser=new TF2IndirectTechnique::UpdaterTraversalVisitor(_cullingData);
    _dirty = false;
}


void TF2IndirectTechnique::update(osg::NodeVisitor& nv)
{
   // _GPUScene->osg::Group::traverse(nv);

}

void TF2IndirectTechnique::cull(osgUtil::CullVisitor& cv)
{
    // record the traversal mask on entry so we can reapply it later.
    unsigned int traversalMask = cv.getTraversalMask();

    osgUtil::RenderStage* orig_rs = cv.getRenderStage();

    // do traversal of shadow receiving scene which does need to be decorated by the shadow map
    {
        cv.pushStateSet(_stateset.get());

        _GPUScene->osg::Group::traverse(cv);

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

        //std::cout<<"----- VxOSG::TF2IndirectTechnique selectLight spot cutoff "<<selectLight->getSpotCutoff()<<std::endl;

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
         //   cbbv.setTraversalMask(getGPUScene()->getCastsShadowTraversalMask());

            _GPUScene->osg::Group::traverse(cbbv);

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

        cv.setTraversalMask( traversalMask
            // &getGPUScene()->getCastsShadowTraversalMask()
            );

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

void TF2IndirectTechnique::cleanSceneGraph()
{
}

///////////////////// Debug Methods

////////////////////////////////////////////////////////////////////////////////
// Callback used by debugging hud to display Shadow Map in color buffer
// OSG does not allow to use the same GL Texture Id with different glTexParams.
// Callback simply turns shadow compare mode off via GL while rendering hud and
// restores it afterwards.
////////////////////////////////////////////////////////////////////////////////
class TF2IndirectTechnique::DrawableDrawWithDepthShadowComparisonOffCallback:
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
osg::ref_ptr<osg::Camera> TF2IndirectTechnique::makeDebugHUD()
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

