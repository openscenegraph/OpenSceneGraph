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


#include<OpenThreads/ScopedLock>

#include<osg/Texture2D>
#include<osg/PointSprite>
#include<osgDB/FileUtils>
#include<osgUtil/CullVisitor>
#include<osgUtil/GLObjectsVisitor>

#include <osgParticle/PrecipitationEffect>

#include <osg/Notify>
#include <osg/io_utils>
#include <osg/Timer>
#include <osg/ImageUtils>

using namespace osgParticle;

#define USE_LOCAL_SHADERS

static float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }

PrecipitationEffect::PrecipitationEffect():
    _previousFrameTime(FLT_MAX)
{
    setNumChildrenRequiringUpdateTraversal(1);

    setUpGeometries(1024);

    rain(0.5);
}


PrecipitationEffect::PrecipitationEffect(const PrecipitationEffect& copy, const osg::CopyOp& copyop):
    osg::Node(copy,copyop),
    _previousFrameTime(FLT_MAX)
{
    setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);

    _wind = copy._wind;
    _particleSpeed = copy._particleSpeed;
    _particleSize = copy._particleSize;
    _particleColor = copy._particleColor;
    _maximumParticleDensity = copy._maximumParticleDensity;
    _cellSize = copy._cellSize;
    _nearTransition = copy._nearTransition;
    _farTransition = copy._farTransition;

    _fog = copy._fog.valid() ? dynamic_cast<osg::Fog*>(copy._fog->clone(copyop)) : 0;


    _useFarLineSegments = copy._useFarLineSegments;

    _dirty = true;

    update();
}

void PrecipitationEffect::rain(float intensity)
{
    _wind.set(0.0f,0.0f,0.0f);
    _particleSpeed = -2.0f + -5.0f*intensity;
    _particleSize = 0.01 + 0.02*intensity;
    _particleColor = osg::Vec4(0.6, 0.6, 0.6, 1.0) -  osg::Vec4(0.1, 0.1, 0.1, 1.0)* intensity;
    _maximumParticleDensity = intensity * 8.5f;
    _cellSize.set(5.0f / (0.25f+intensity), 5.0f / (0.25f+intensity), 5.0f);
    _nearTransition = 25.f;
    _farTransition = 100.0f - 60.0f*sqrtf(intensity);

    if (!_fog) _fog = new osg::Fog;

    _fog->setMode(osg::Fog::EXP);
    _fog->setDensity(0.005f*intensity);
    _fog->setColor(osg::Vec4(0.5, 0.5, 0.5, 1.0));

    _useFarLineSegments = false;

    _dirty = true;

    update();
}

void PrecipitationEffect::snow(float intensity)
{
    _wind.set(0.0f,0.0f,0.0f);
    _particleSpeed = -0.75f - 0.25f*intensity;
    _particleSize = 0.02f + 0.03f*intensity;
    _particleColor = osg::Vec4(0.85f, 0.85f, 0.85f, 1.0f) -  osg::Vec4(0.1f, 0.1f, 0.1f, 1.0f)* intensity;
    _maximumParticleDensity = intensity * 8.2f;
    _cellSize.set(5.0f / (0.25f+intensity), 5.0f / (0.25f+intensity), 5.0f);
    _nearTransition = 25.f;
    _farTransition = 100.0f - 60.0f*sqrtf(intensity);

    if (!_fog) _fog = new osg::Fog;

    _fog->setMode(osg::Fog::EXP);
    _fog->setDensity(0.01f*intensity);
    _fog->setColor(osg::Vec4(0.6, 0.6, 0.6, 1.0));

    _useFarLineSegments = false;

    _dirty = true;

    update();
}

void PrecipitationEffect::compileGLObjects(osg::RenderInfo& renderInfo) const
{
    if (_quadGeometry.valid())
    {
        _quadGeometry->compileGLObjects(renderInfo);
        if (_quadGeometry->getStateSet()) _quadGeometry->getStateSet()->compileGLObjects(*renderInfo.getState());
    }

    if (_lineGeometry.valid())
    {
        _lineGeometry->compileGLObjects(renderInfo);
        if (_lineGeometry->getStateSet()) _lineGeometry->getStateSet()->compileGLObjects(*renderInfo.getState());
    }

    if (_pointGeometry.valid())
    {
        _pointGeometry->compileGLObjects(renderInfo);
        if (_pointGeometry->getStateSet()) _pointGeometry->getStateSet()->compileGLObjects(*renderInfo.getState());
    }
}


void PrecipitationEffect::traverse(osg::NodeVisitor& nv)
{
    if (nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
    {
        if (_dirty) update();

        if (nv.getFrameStamp())
        {
            double currentTime = nv.getFrameStamp()->getSimulationTime();
            if (_previousFrameTime==FLT_MAX) _previousFrameTime = currentTime;

            double delta = currentTime - _previousFrameTime;
            _origin += _wind * delta;
            _previousFrameTime = currentTime;
        }

        return;
    }

    if (nv.getVisitorType() == osg::NodeVisitor::NODE_VISITOR)
    {
        if (_dirty) update();

        osgUtil::GLObjectsVisitor* globjVisitor = dynamic_cast<osgUtil::GLObjectsVisitor*>(&nv);
        if (globjVisitor)
        {
            if (globjVisitor->getMode() & osgUtil::GLObjectsVisitor::COMPILE_STATE_ATTRIBUTES)
            {
                compileGLObjects(globjVisitor->getRenderInfo());
            }
        }

        return;
    }


    if (nv.getVisitorType() != osg::NodeVisitor::CULL_VISITOR)
    {
        return;
    }

    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
    if (!cv)
    {
        return;
    }

    ViewIdentifier viewIndentifier(cv, nv.getNodePath());

    {
        PrecipitationDrawableSet* precipitationDrawableSet = 0;

        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
            precipitationDrawableSet = &(_viewDrawableMap[viewIndentifier]);

            if (!precipitationDrawableSet->_quadPrecipitationDrawable)
            {
                precipitationDrawableSet->_quadPrecipitationDrawable = new PrecipitationDrawable;
                precipitationDrawableSet->_quadPrecipitationDrawable->setRequiresPreviousMatrix(true);
                precipitationDrawableSet->_quadPrecipitationDrawable->setGeometry(_quadGeometry.get());
                precipitationDrawableSet->_quadPrecipitationDrawable->setStateSet(_quadStateSet.get());
                precipitationDrawableSet->_quadPrecipitationDrawable->setDrawType(GL_QUADS);

                precipitationDrawableSet->_linePrecipitationDrawable = new PrecipitationDrawable;
                precipitationDrawableSet->_linePrecipitationDrawable->setRequiresPreviousMatrix(true);
                precipitationDrawableSet->_linePrecipitationDrawable->setGeometry(_lineGeometry.get());
                precipitationDrawableSet->_linePrecipitationDrawable->setStateSet(_lineStateSet.get());
                precipitationDrawableSet->_linePrecipitationDrawable->setDrawType(GL_LINES);

                precipitationDrawableSet->_pointPrecipitationDrawable = new PrecipitationDrawable;
                precipitationDrawableSet->_pointPrecipitationDrawable->setRequiresPreviousMatrix(false);
                precipitationDrawableSet->_pointPrecipitationDrawable->setGeometry(_pointGeometry.get());
                precipitationDrawableSet->_pointPrecipitationDrawable->setStateSet(_pointStateSet.get());
                precipitationDrawableSet->_pointPrecipitationDrawable->setDrawType(GL_POINTS);
            }
        }

        cull(*precipitationDrawableSet, cv);

        cv->pushStateSet(_stateset.get());
        float depth = 0.0f;

        if (!precipitationDrawableSet->_quadPrecipitationDrawable->getCurrentCellMatrixMap().empty())
        {
            cv->pushStateSet(precipitationDrawableSet->_quadPrecipitationDrawable->getStateSet());
            cv->addDrawableAndDepth(precipitationDrawableSet->_quadPrecipitationDrawable.get(),cv->getModelViewMatrix(),depth);
            cv->popStateSet();
        }

        if (!precipitationDrawableSet->_linePrecipitationDrawable->getCurrentCellMatrixMap().empty())
        {
            cv->pushStateSet(precipitationDrawableSet->_linePrecipitationDrawable->getStateSet());
            cv->addDrawableAndDepth(precipitationDrawableSet->_linePrecipitationDrawable.get(),cv->getModelViewMatrix(),depth);
            cv->popStateSet();
        }

        if (!precipitationDrawableSet->_pointPrecipitationDrawable->getCurrentCellMatrixMap().empty())
        {
            cv->pushStateSet(precipitationDrawableSet->_pointPrecipitationDrawable->getStateSet());
            cv->addDrawableAndDepth(precipitationDrawableSet->_pointPrecipitationDrawable.get(),cv->getModelViewMatrix(),depth);
            cv->popStateSet();
        }

        cv->popStateSet();

    }
}

void PrecipitationEffect::update()
{
    _dirty = false;

    OSG_INFO<<"PrecipitationEffect::update()"<<std::endl;

    float length_u = _cellSize.x();
    float length_v = _cellSize.y();
    float length_w = _cellSize.z();

    // time taken to get from start to the end of cycle
    _period = fabsf(_cellSize.z() / _particleSpeed);

    _du.set(length_u, 0.0f, 0.0f);
    _dv.set(0.0f, length_v, 0.0f);
    _dw.set(0.0f, 0.0f, length_w);

    _inverse_du.set(1.0f/length_u, 0.0f, 0.0f);
    _inverse_dv.set(0.0f, 1.0f/length_v, 0.0f);
    _inverse_dw.set(0.0f, 0.0f, 1.0f/length_w);

    OSG_INFO<<"Cell size X="<<length_u<<std::endl;
    OSG_INFO<<"Cell size Y="<<length_v<<std::endl;
    OSG_INFO<<"Cell size Z="<<length_w<<std::endl;


    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        _viewDrawableMap.clear();
    }

    // set up state/
    {
        if (!_stateset)
        {
            _stateset = new osg::StateSet;
            _stateset->addUniform(new osg::Uniform("baseTexture",0));

            _stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
            _stateset->setMode(GL_BLEND, osg::StateAttribute::ON);

            osg::Texture2D* texture = new osg::Texture2D(createSpotLightImage(osg::Vec4(1.0f,1.0f,1.0f,1.0f),osg::Vec4(1.0f,1.0f,1.0f,0.0f),32,1.0));
            _stateset->setTextureAttribute(0, texture);
        }

        if (!_inversePeriodUniform)
        {
            _inversePeriodUniform = new osg::Uniform("inversePeriod",1.0f/_period);
            _stateset->addUniform(_inversePeriodUniform.get());
        }
        else _inversePeriodUniform->set(1.0f/_period);

        if (!_particleColorUniform)
        {
            _particleColorUniform = new osg::Uniform("particleColour", _particleColor);
            _stateset->addUniform(_particleColorUniform.get());
        }
        else _particleColorUniform->set(_particleColor);

        if (!_particleSizeUniform)
        {
            _particleSizeUniform = new osg::Uniform("particleSize", _particleSize);
            _stateset->addUniform(_particleSizeUniform.get());
        }
        else _particleSizeUniform->set(_particleSize);

    }

}

void PrecipitationEffect::createGeometry(unsigned int numParticles,
                    osg::Geometry* quad_geometry,
                    osg::Geometry* line_geometry,
                    osg::Geometry* point_geometry)
{
    // particle corner offsets
    osg::Vec2 offset00(0.0f,0.0f);
    osg::Vec2 offset10(1.0f,0.0f);
    osg::Vec2 offset01(0.0f,1.0f);
    osg::Vec2 offset11(1.0f,1.0f);

    osg::Vec2 offset0(0.5f,0.0f);
    osg::Vec2 offset1(0.5f,1.0f);

    osg::Vec2 offset(0.5f,0.5f);


    // configure quad_geometry;
    osg::Vec3Array* quad_vertices = 0;
    osg::Vec2Array* quad_offsets = 0;
    if (quad_geometry)
    {
        quad_geometry->setName("quad");

        quad_vertices = new osg::Vec3Array(numParticles*4);
        quad_offsets = new osg::Vec2Array(numParticles*4);

        quad_geometry->setVertexArray(quad_vertices);
        quad_geometry->setTexCoordArray(0, quad_offsets);
    }

    // configure line_geometry;
    osg::Vec3Array* line_vertices = 0;
    osg::Vec2Array* line_offsets = 0;
    if (line_geometry)
    {
        line_geometry->setName("line");

        line_vertices = new osg::Vec3Array(numParticles*2);
        line_offsets = new osg::Vec2Array(numParticles*2);

        line_geometry->setVertexArray(line_vertices);
        line_geometry->setTexCoordArray(0, line_offsets);
    }

    // configure point_geometry;
    osg::Vec3Array* point_vertices = 0;
    osg::Vec2Array* point_offsets = 0;
    if (point_geometry)
    {
        point_geometry->setName("point");

        point_vertices = new osg::Vec3Array(numParticles);
        point_offsets = new osg::Vec2Array(numParticles);

        point_geometry->setVertexArray(point_vertices);
        point_geometry->setTexCoordArray(0, point_offsets);
    }

    // set up vertex attribute data.
    for(unsigned int i=0; i< numParticles; ++i)
    {
        osg::Vec3 pos( random(0.0f, 1.0f), random(0.0f, 1.0f), random(0.0f, 1.0f));

        // quad particles
        if (quad_vertices)
        {
            (*quad_vertices)[i*4] = pos;
            (*quad_vertices)[i*4+1] = pos;
            (*quad_vertices)[i*4+2] =  pos;
            (*quad_vertices)[i*4+3] =  pos;
            (*quad_offsets)[i*4] = offset00;
            (*quad_offsets)[i*4+1] = offset01;
            (*quad_offsets)[i*4+2] = offset11;
            (*quad_offsets)[i*4+3] = offset10;
        }

        // line particles
        if (line_vertices)
        {
            (*line_vertices)[i*2] = pos;
            (*line_vertices)[i*2+1] = pos;
            (*line_offsets)[i*2] = offset0;
            (*line_offsets)[i*2+1] = offset1;
        }

        // point particles
        if (point_vertices)
        {
            (*point_vertices)[i] = pos;
            (*point_offsets)[i] = offset;
        }
    }
}

void PrecipitationEffect::setUpGeometries(unsigned int numParticles)
{
    unsigned int quadRenderBin = 13;
    unsigned int lineRenderBin = 12;
    unsigned int pointRenderBin = 11;


    OSG_INFO<<"PrecipitationEffect::setUpGeometries("<<numParticles<<")"<<std::endl;

    bool needGeometryRebuild = false;

    if (!_quadGeometry || _quadGeometry->getVertexArray()->getNumElements() != 4*numParticles)
    {
        _quadGeometry = new osg::Geometry;
        _quadGeometry->setUseVertexBufferObjects(true);
        needGeometryRebuild = true;
    }

    if (!_lineGeometry || _lineGeometry->getVertexArray()->getNumElements() != 2*numParticles)
    {
        _lineGeometry = new osg::Geometry;
        _lineGeometry->setUseVertexBufferObjects(true);
        needGeometryRebuild = true;
    }

    if (!_pointGeometry || _pointGeometry->getVertexArray()->getNumElements() != numParticles)
    {
        _pointGeometry = new osg::Geometry;
        _pointGeometry->setUseVertexBufferObjects(true);
        needGeometryRebuild = true;
    }

    if (needGeometryRebuild)
    {
        createGeometry(numParticles, _quadGeometry.get(), _lineGeometry.get(), _pointGeometry.get());
    }


    if (!_quadStateSet)
    {
        _quadStateSet = new osg::StateSet;

        osg::Program* program = new osg::Program;
        _quadStateSet->setAttribute(program);
        _quadStateSet->setRenderBinDetails(quadRenderBin,"DepthSortedBin");

#ifdef USE_LOCAL_SHADERS
        char vertexShaderSource[] =
            "uniform float inversePeriod;\n"
            "uniform vec4 particleColour;\n"
            "uniform float particleSize;\n"
            "\n"
            "uniform float osg_SimulationTime;\n"
            "uniform float osg_DeltaSimulationTime;\n"
            "\n"
            "varying vec4 colour;\n"
            "varying vec2 texCoord;\n"
            "\n"
            "void main(void)\n"
            "{\n"
            "    float offset = gl_Vertex.z;\n"
            "    float startTime = gl_MultiTexCoord1.x;\n"
            "    texCoord = gl_MultiTexCoord0.xy;\n"
            "\n"
            "    vec4 v_previous = gl_Vertex;\n"
            "    v_previous.z = fract( (osg_SimulationTime - startTime)*inversePeriod - offset);\n"
            "    \n"
            "    vec4 v_current =  v_previous;\n"
            "    v_current.z += (osg_DeltaSimulationTime*inversePeriod);\n"
            "    \n"
            "\n"
            "    colour = particleColour;\n"
            "    \n"
            "    vec4 v1 = gl_ModelViewMatrix * v_current;\n"
            "    vec4 v2 = gl_TextureMatrix[0] * v_previous;\n"
            "    \n"
            "    vec3 dv = v2.xyz - v1.xyz;\n"
            "    \n"
            "    vec2 dv_normalized = normalize(dv.xy);\n"
            "    dv.xy += dv_normalized * particleSize;\n"
            "    vec2 dp = vec2( -dv_normalized.y, dv_normalized.x ) * particleSize;\n"
            "    \n"
            "    float area = length(dv.xy);\n"
            "    colour.a = 0.05+(particleSize)/area;\n"
            "    \n"
            "\n"
            "    v1.xyz += dv*texCoord.y;\n"
            "    v1.xy += dp*texCoord.x;\n"
            "    \n"
            "    gl_Position = gl_ProjectionMatrix * v1;\n"
            "    gl_ClipVertex = v1;\n"
            "}\n";

        char fragmentShaderSource[] =
            "uniform sampler2D baseTexture;\n"
            "varying vec2 texCoord;\n"
            "varying vec4 colour;\n"
            "\n"
            "void main (void)\n"
            "{\n"
            "    gl_FragColor = colour * texture2D( baseTexture, texCoord);\n"
            "}\n";

        program->addShader(new osg::Shader(osg::Shader::VERTEX, vertexShaderSource));
        program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource));
#else
        // get shaders from source
        program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("quad_rain.vert")));
        program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("rain.frag")));
#endif
    }


    if (!_lineStateSet)
    {
        _lineStateSet = new osg::StateSet;

        osg::Program* program = new osg::Program;
        _lineStateSet->setAttribute(program);
        _lineStateSet->setRenderBinDetails(lineRenderBin,"DepthSortedBin");

#ifdef USE_LOCAL_SHADERS
        char vertexShaderSource[] =
            "uniform float inversePeriod;\n"
            "uniform vec4 particleColour;\n"
            "uniform float particleSize;\n"
            "\n"
            "uniform float osg_SimulationTime;\n"
            "uniform float osg_DeltaSimulationTime;\n"
            "uniform mat4 previousModelViewMatrix;\n"
            "\n"
            "varying vec4 colour;\n"
            "varying vec2 texCoord;\n"
            "\n"
            "void main(void)\n"
            "{\n"
            "    float offset = gl_Vertex.z;\n"
            "    float startTime = gl_MultiTexCoord1.x;\n"
            "    texCoord = gl_MultiTexCoord0.xy;\n"
            "\n"
            "    vec4 v_previous = gl_Vertex;\n"
            "    v_previous.z = fract( (osg_SimulationTime - startTime)*inversePeriod - offset);\n"
            "    \n"
            "    vec4 v_current =  v_previous;\n"
            "    v_current.z += (osg_DeltaSimulationTime*inversePeriod);\n"
            "    \n"
            "    colour = particleColour;\n"
            "    \n"
            "    vec4 v1 = gl_ModelViewMatrix * v_current;\n"
            "    vec4 v2 = gl_TextureMatrix[0] * v_previous;\n"
            "    \n"
            "    vec3 dv = v2.xyz - v1.xyz;\n"
            "    \n"
            "    vec2 dv_normalized = normalize(dv.xy);\n"
            "    dv.xy += dv_normalized * particleSize;\n"
            "    \n"
            "    float area = length(dv.xy);\n"
            "    colour.a = (particleSize)/area;\n"
            "    \n"
            "    v1.xyz += dv*texCoord.y;\n"
            "    \n"
            "    gl_Position = gl_ProjectionMatrix * v1;\n"
            "    gl_ClipVertex = v1;\n"
            "}\n";

        char fragmentShaderSource[] =
            "uniform sampler2D baseTexture;\n"
            "varying vec2 texCoord;\n"
            "varying vec4 colour;\n"
            "\n"
            "void main (void)\n"
            "{\n"
            "    gl_FragColor = colour * texture2D( baseTexture, texCoord);\n"
            "}\n";

        program->addShader(new osg::Shader(osg::Shader::VERTEX, vertexShaderSource));
        program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource));
#else
        // get shaders from source
        program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("line_rain.vert")));
        program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("rain.frag")));
#endif
    }


    if (!_pointStateSet)
    {
        _pointStateSet = new osg::StateSet;

        osg::Program* program = new osg::Program;
        _pointStateSet->setAttribute(program);

#ifdef USE_LOCAL_SHADERS
        char vertexShaderSource[] =
            "uniform float inversePeriod;\n"
            "uniform vec4 particleColour;\n"
            "uniform float particleSize;\n"
            "\n"
            "uniform float osg_SimulationTime;\n"
            "\n"
            "varying vec4 colour;\n"
            "\n"
            "void main(void)\n"
            "{\n"
            "    float offset = gl_Vertex.z;\n"
            "    float startTime = gl_MultiTexCoord1.x;\n"
            "\n"
            "    vec4 v_current = gl_Vertex;\n"
            "    v_current.z = fract( (osg_SimulationTime - startTime)*inversePeriod - offset);\n"
            "   \n"
            "    colour = particleColour;\n"
            "\n"
            "    gl_Position = gl_ModelViewProjectionMatrix * v_current;\n"
            "\n"
            "    float pointSize = abs(1280.0*particleSize / gl_Position.w);\n"
            "\n"
            "    //gl_PointSize = max(ceil(pointSize),2);\n"
            "    gl_PointSize = ceil(pointSize);\n"
            "    \n"
            "    colour.a = 0.05+(pointSize*pointSize)/(gl_PointSize*gl_PointSize);\n"
            "    gl_ClipVertex = gl_ModelViewMatrix * v_current;\n"
            "}\n";

        char fragmentShaderSource[] =
            "uniform sampler2D baseTexture;\n"
            "varying vec4 colour;\n"
            "\n"
            "void main (void)\n"
            "{\n"
            "    gl_FragColor = colour * texture2D( baseTexture, gl_TexCoord[0].xy);\n"
            "}\n";

        program->addShader(new osg::Shader(osg::Shader::VERTEX, vertexShaderSource));
        program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource));
#else
        // get shaders from source
        program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("point_rain.vert")));
        program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("point_rain.frag")));
#endif

        /// Setup the point sprites
        osg::PointSprite *sprite = new osg::PointSprite();
        _pointStateSet->setTextureAttributeAndModes(0, sprite, osg::StateAttribute::ON);

        #if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE)
            _pointStateSet->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
        #else
            OSG_NOTICE<<"Warning: ParticleEffect::setUpGeometries(..) not fully implemented."<<std::endl;
        #endif

        _pointStateSet->setRenderBinDetails(pointRenderBin,"DepthSortedBin");
    }


}

void PrecipitationEffect::cull(PrecipitationDrawableSet& pds, osgUtil::CullVisitor* cv) const
{
#ifdef DO_TIMING
    osg::Timer_t startTick = osg::Timer::instance()->tick();
#endif

    float cellVolume = _cellSize.x() * _cellSize.y() * _cellSize.z();
    int numberOfParticles = (int)(_maximumParticleDensity * cellVolume);

    if (numberOfParticles==0) return;

    pds._quadPrecipitationDrawable->setNumberOfVertices(numberOfParticles*4);
    pds._linePrecipitationDrawable->setNumberOfVertices(numberOfParticles*2);
    pds._pointPrecipitationDrawable->setNumberOfVertices(numberOfParticles);

    pds._quadPrecipitationDrawable->newFrame();
    pds._linePrecipitationDrawable->newFrame();
    pds._pointPrecipitationDrawable->newFrame();

    osg::Matrix inverse_modelview;
    inverse_modelview.invert(*(cv->getModelViewMatrix()));

    osg::Vec3 eyeLocal = osg::Vec3(0.0f,0.0f,0.0f) * inverse_modelview;
    //OSG_NOTICE<<"  eyeLocal "<<eyeLocal<<std::endl;

    float eye_k = (eyeLocal-_origin)*_inverse_dw;
    osg::Vec3 eye_kPlane = eyeLocal-_dw*eye_k-_origin;

    // OSG_NOTICE<<"  eye_kPlane "<<eye_kPlane<<std::endl;

    float eye_i = eye_kPlane*_inverse_du;
    float eye_j = eye_kPlane*_inverse_dv;

    osg::Polytope frustum;
    frustum.setToUnitFrustum(false,false);
    frustum.transformProvidingInverse(*(cv->getProjectionMatrix()));
    frustum.transformProvidingInverse(*(cv->getModelViewMatrix()));

    float i_delta = _farTransition * _inverse_du.x();
    float j_delta = _farTransition * _inverse_dv.y();
    float k_delta = 1;//_nearTransition * _inverse_dw.z();

    int i_min = (int)floor(eye_i - i_delta);
    int j_min = (int)floor(eye_j - j_delta);
    int k_min = (int)floor(eye_k - k_delta);

    int i_max = (int)ceil(eye_i + i_delta);
    int j_max = (int)ceil(eye_j + j_delta);
    int k_max = (int)ceil(eye_k + k_delta);

    //OSG_NOTICE<<"i_delta="<<i_delta<<" j_delta="<<j_delta<<" k_delta="<<k_delta<<std::endl;

    unsigned int numTested=0;
    unsigned int numInFrustum=0;

    float iCyle = 0.43;
    float jCyle = 0.64;

    for(int i = i_min; i<=i_max; ++i)
    {
        for(int j = j_min; j<=j_max; ++j)
        {
            for(int k = k_min; k<=k_max; ++k)
            {
                float startTime = (float)(i)*iCyle + (float)(j)*jCyle;
                startTime = (startTime-floor(startTime))*_period;

                if (build(eyeLocal, i,j,k, startTime, pds, frustum, cv)) ++numInFrustum;
                ++numTested;
            }
        }
    }


#ifdef DO_TIMING
    osg::Timer_t endTick = osg::Timer::instance()->tick();

    OSG_NOTICE<<"time for cull "<<osg::Timer::instance()->delta_m(startTick,endTick)<<"ms  numTested="<<numTested<<" numInFrustum"<<numInFrustum<<std::endl;
    OSG_NOTICE<<"     quads "<<pds._quadPrecipitationDrawable->getCurrentCellMatrixMap().size()<<"   lines "<<pds._linePrecipitationDrawable->getCurrentCellMatrixMap().size()<<"   points "<<pds._pointPrecipitationDrawable->getCurrentCellMatrixMap().size()<<std::endl;
#endif
}

bool PrecipitationEffect::build(const osg::Vec3 eyeLocal, int i, int j, int k, float startTime, PrecipitationDrawableSet& pds, osg::Polytope& frustum, osgUtil::CullVisitor* cv) const
{
    osg::Vec3 position = _origin + osg::Vec3(float(i)*_du.x(), float(j)*_dv.y(), float(k+1)*_dw.z());
    osg::Vec3 scale(_du.x(), _dv.y(), -_dw.z());

    osg::BoundingBox bb(position.x(), position.y(), position.z()+scale.z(),
                        position.x()+scale.x(), position.y()+scale.y(), position.z());

    if (!frustum.contains(bb)) return false;

    osg::Vec3 center = position + scale*0.5f;
    float distance = (center-eyeLocal).length();

    osg::Matrix* mymodelview = 0;
    if (distance < _nearTransition)
    {
        PrecipitationDrawable::DepthMatrixStartTime& mstp = pds._quadPrecipitationDrawable->getCurrentCellMatrixMap()[PrecipitationDrawable::Cell(i,k,j)];
        mstp.depth = distance;
        mstp.startTime = startTime;
        mymodelview = &mstp.modelview;
    }
    else if (distance <= _farTransition)
    {
        if (_useFarLineSegments)
        {
            PrecipitationDrawable::DepthMatrixStartTime& mstp = pds._linePrecipitationDrawable->getCurrentCellMatrixMap()[PrecipitationDrawable::Cell(i,k,j)];
            mstp.depth = distance;
            mstp.startTime = startTime;
            mymodelview = &mstp.modelview;
        }
        else
        {
            PrecipitationDrawable::DepthMatrixStartTime& mstp = pds._pointPrecipitationDrawable->getCurrentCellMatrixMap()[PrecipitationDrawable::Cell(i,k,j)];
            mstp.depth = distance;
            mstp.startTime = startTime;
            mymodelview = &mstp.modelview;
        }
    }
    else
    {
        return false;
    }

    *mymodelview = *(cv->getModelViewMatrix());
    mymodelview->preMultTranslate(position);
    mymodelview->preMultScale(scale);

    cv->updateCalculatedNearFar(*(cv->getModelViewMatrix()),bb);

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   Precipitation Drawable
//
////////////////////////////////////////////////////////////////////////////////////////////////////

PrecipitationEffect::PrecipitationDrawable::PrecipitationDrawable():
    _requiresPreviousMatrix(true),
    _drawType(GL_QUADS),
    _numberOfVertices(0)
{
    setSupportsDisplayList(false);
}

PrecipitationEffect::PrecipitationDrawable::PrecipitationDrawable(const PrecipitationDrawable& copy, const osg::CopyOp& copyop):
    osg::Drawable(copy,copyop),
    _requiresPreviousMatrix(copy._requiresPreviousMatrix),
    _geometry(copy._geometry),
    _drawType(copy._drawType),
    _numberOfVertices(copy._numberOfVertices)
{
}



void PrecipitationEffect::PrecipitationDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{
#if defined(OSG_GL_MATRICES_AVAILABLE)

if (!_geometry) return;


    const osg::Geometry::Extensions* extensions = osg::Geometry::getExtensions(renderInfo.getContextID(),true);

    // save OpenGL matrices
    glPushMatrix();

    if (_requiresPreviousMatrix)
    {
        renderInfo.getState()->setActiveTextureUnit(0);
        glMatrixMode( GL_TEXTURE );
        glPushMatrix();
    }

    typedef std::vector<const CellMatrixMap::value_type*> DepthMatrixStartTimeVector;
    DepthMatrixStartTimeVector orderedEntries;
    orderedEntries.reserve(_currentCellMatrixMap.size());

    for(CellMatrixMap::const_iterator citr = _currentCellMatrixMap.begin();
        citr != _currentCellMatrixMap.end();
        ++citr)
    {
        orderedEntries.push_back(&(*citr));
    }

    std::sort(orderedEntries.begin(),orderedEntries.end(),LessFunctor());

    for(DepthMatrixStartTimeVector::reverse_iterator itr = orderedEntries.rbegin();
        itr != orderedEntries.rend();
        ++itr)
    {
        extensions->glMultiTexCoord1f(GL_TEXTURE0+1, (*itr)->second.startTime);

        // load cells current modelview matrix
        if (_requiresPreviousMatrix)
        {
            glMatrixMode( GL_MODELVIEW );
            glLoadMatrix((*itr)->second.modelview.ptr());

            CellMatrixMap::const_iterator pitr = _previousCellMatrixMap.find((*itr)->first);
            if (pitr != _previousCellMatrixMap.end())
            {
                // load previous frame modelview matrix for motion blurr effect
                glMatrixMode( GL_TEXTURE );
                glLoadMatrix(pitr->second.modelview.ptr());
            }
            else
            {
                // use current modelview matrix as "previous" frame value, cancelling motion blurr effect
                glMatrixMode( GL_TEXTURE );
                glLoadMatrix((*itr)->second.modelview.ptr());
            }
        }
        else
        {
            glLoadMatrix((*itr)->second.modelview.ptr());
        }

        _geometry->draw(renderInfo);

        unsigned int numVertices = osg::minimum(_geometry->getVertexArray()->getNumElements(), _numberOfVertices);
        glDrawArrays(_drawType, 0, numVertices);
    }

    // restore OpenGL matrices
    if (_requiresPreviousMatrix)
    {
        glPopMatrix();
        glMatrixMode( GL_MODELVIEW );
    }

    glPopMatrix();
#else
    OSG_NOTICE<<"Warning: ParticleEffect::drawImplementation(..) not fully implemented."<<std::endl;
#endif
}
