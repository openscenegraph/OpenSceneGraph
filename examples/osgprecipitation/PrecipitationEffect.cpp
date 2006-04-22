/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield 
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

#include "PrecipitationEffect.h"

#include<OpenThreads/ScopedLock>

#include<osg/Texture2D>
#include<osg/PointSprite>
#include<osgDB/FileUtils>
#include<osgUtil/CullVisitor>
#include<osgUtil/GLObjectsVisitor>

#include <osg/Notify>
#include <osg/io_utils>
#include <osg/Timer>

using namespace osgParticle;

#define USE_LOCAL_SHADERS

static float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }

static void fillSpotLightImage(unsigned char* ptr, const osg::Vec4& centerColour, const osg::Vec4& backgroudColour, unsigned int size, float power)
{
    float mid = (float(size)-1.0f)*0.5f;
    float div = 2.0f/float(size);
    for(unsigned int r=0;r<size;++r)
    {
        //unsigned char* ptr = image->data(0,r,0);
        for(unsigned int c=0;c<size;++c)
        {
            float dx = (float(c) - mid)*div;
            float dy = (float(r) - mid)*div;
            float r = powf(1.0f-sqrtf(dx*dx+dy*dy),power);
            if (r<0.0f) r=0.0f;
            osg::Vec4 color = centerColour*r+backgroudColour*(1.0f-r);
            *ptr++ = (unsigned char)((color[0])*255.0f);
            *ptr++ = (unsigned char)((color[1])*255.0f);
            *ptr++ = (unsigned char)((color[2])*255.0f);
            *ptr++ = (unsigned char)((color[3])*255.0f);
        }
    }
}

static osg::Image* createSpotLightImage(const osg::Vec4& centerColour, const osg::Vec4& backgroudColour, unsigned int size, float power)
{

#if 0
    osg::Image* image = new osg::Image;
    unsigned char* ptr = image->data(0,0,0);
    fillSpotLightImage(ptr, centerColour, backgroudColour, size, power);

    return image;
#else
    osg::Image* image = new osg::Image;
    osg::Image::MipmapDataType mipmapData;
    unsigned int s = size;
    unsigned int totalSize = 0;
    unsigned i;
    for(i=0; s>0; s>>=1, ++i)
    {
        if (i>0) mipmapData.push_back(totalSize);
        totalSize += s*s*4;
    }

    unsigned char* ptr = new unsigned char[totalSize];
    image->setImage(size, size, size, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, ptr, osg::Image::USE_NEW_DELETE,1);

    image->setMipmapLevels(mipmapData);

    s = size;
    for(i=0; s>0; s>>=1, ++i)
    {
        fillSpotLightImage(ptr, centerColour, backgroudColour, s, power);
        ptr += s*s*4;
    }

    return image;
#endif    
}


PrecipitationEffect::PrecipitationEffect()
{
    setParameters(new PrecipitationParameters);
    update();
}

PrecipitationEffect::PrecipitationEffect(const PrecipitationEffect& copy, const osg::CopyOp& copyop):
    osg::Group(copy,copyop)
{
    setParameters(const_cast<PrecipitationParameters*>(copy._parameters.get()));
    update();
}

void PrecipitationEffect::compileGLObjects(osg::State& state) const
{
    osg::notify(osg::NOTICE)<<"PrecipitationEffect::compileGLObjects()"<<this<<std::endl;

    if (_quadGeometry.valid()) 
    {
        _quadGeometry->compileGLObjects(state);
        if (_quadGeometry->getStateSet()) _quadGeometry->getStateSet()->compileGLObjects(state);
    }

    if (_lineGeometry.valid()) 
    {
        _lineGeometry->compileGLObjects(state);
        if (_lineGeometry->getStateSet()) _lineGeometry->getStateSet()->compileGLObjects(state);
    }

    if (_pointGeometry.valid()) 
    {
        _pointGeometry->compileGLObjects(state);
        if (_pointGeometry->getStateSet()) _pointGeometry->getStateSet()->compileGLObjects(state);
    }
}


void PrecipitationEffect::traverse(osg::NodeVisitor& nv)
{
    if (nv.getVisitorType() == osg::NodeVisitor::NODE_VISITOR)
    {
        osgUtil::GLObjectsVisitor* globjVisitor = dynamic_cast<osgUtil::GLObjectsVisitor*>(&nv);
        if (globjVisitor)
        {
            if (globjVisitor->getMode() & osgUtil::GLObjectsVisitor::COMPILE_STATE_ATTRIBUTES)
            {
                compileGLObjects(*(globjVisitor->getState()));
            }
        }
    
        Group::traverse(nv);
        return;
    }
    

    if (nv.getVisitorType() != osg::NodeVisitor::CULL_VISITOR)
    {
        Group::traverse(nv);
        return;
    }

    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
    if (!cv)
    {
        Group::traverse(nv);
        return;
    }

    ViewIdenitifier viewIndentifier(cv, nv.getNodePath());
    
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        PrecipitationDrawableSet& precipitationDrawableSet = _viewDrawableMap[viewIndentifier];
        
        if (!precipitationDrawableSet._quadPrecipitationDrawable)
        {
            precipitationDrawableSet._quadPrecipitationDrawable = new PrecipitationDrawable;
            precipitationDrawableSet._quadPrecipitationDrawable->setGeometry(_quadGeometry.get());
            precipitationDrawableSet._quadPrecipitationDrawable->setStateSet(_quadStateSet.get());

            precipitationDrawableSet._linePrecipitationDrawable = new PrecipitationDrawable;
            precipitationDrawableSet._linePrecipitationDrawable->setGeometry(_lineGeometry.get());
            precipitationDrawableSet._linePrecipitationDrawable->setStateSet(_lineStateSet.get());

            precipitationDrawableSet._pointPrecipitationDrawable = new PrecipitationDrawable;
            precipitationDrawableSet._pointPrecipitationDrawable->setGeometry(_pointGeometry.get());
            precipitationDrawableSet._pointPrecipitationDrawable->setStateSet(_pointStateSet.get());
        }
        
        cull(precipitationDrawableSet, cv);
        
        cv->pushStateSet(_precipitationStateSet.get());
        float depth = 0.0f;

        if (!precipitationDrawableSet._quadPrecipitationDrawable->getCurrentCellMatrixMap().empty())
        {
            cv->pushStateSet(precipitationDrawableSet._quadPrecipitationDrawable->getStateSet());
            cv->addDrawableAndDepth(precipitationDrawableSet._quadPrecipitationDrawable.get(),&cv->getModelViewMatrix(),depth);    
            cv->popStateSet();
        }

        if (!precipitationDrawableSet._linePrecipitationDrawable->getCurrentCellMatrixMap().empty())
        {
            cv->pushStateSet(precipitationDrawableSet._linePrecipitationDrawable->getStateSet());
            cv->addDrawableAndDepth(precipitationDrawableSet._linePrecipitationDrawable.get(),&cv->getModelViewMatrix(),depth);    
            cv->popStateSet();
        }

        if (!precipitationDrawableSet._pointPrecipitationDrawable->getCurrentCellMatrixMap().empty())
        {
            cv->pushStateSet(precipitationDrawableSet._pointPrecipitationDrawable->getStateSet());
            cv->addDrawableAndDepth(precipitationDrawableSet._pointPrecipitationDrawable.get(),&cv->getModelViewMatrix(),depth);    
            cv->popStateSet();
        }

        cv->popStateSet();

    }

    Group::traverse(nv);
}

void PrecipitationEffect::setParameters(PrecipitationParameters* parameters)
{
    if (_parameters==parameters) return;

    _parameters = parameters;
}

void PrecipitationEffect::update()
{
    osg::notify(osg::NOTICE)<<"PrecipitationEffect::update()"<<std::endl;

    if (!_parameters) return;
    
    if (_fog.valid())
    {
        if (_parameters->fogExponent<1.0)
        {
            _fog->setMode(osg::Fog::LINEAR);
        }
        else if (_parameters->fogExponent<2.0)
        {
            _fog->setMode(osg::Fog::EXP);
        }
        else
        {
            _fog->setMode(osg::Fog::EXP2);
        }
        
        _fog->setDensity(_parameters->fogDensity);
        _fog->setStart(0.0f);
        _fog->setEnd(_parameters->fogEnd);
        _fog->setColor(_parameters->fogColour);
    }
    
    if (_clearNode.valid())
    {
        _clearNode->setClearColor(_parameters->clearColour);
    }
    
    
    float length_u = _parameters->cellSizeX;
    float length_v = _parameters->cellSizeY;
    float length_w = _parameters->cellSizeZ;
        
    // volume of a single cell
    float cellVolume = length_u*length_v*length_w;

    // time taken to get from start to the end of cycle
    _period = fabsf(_parameters->cellSizeZ / _parameters->particleVelocity.z());

    _du.set(length_u, 0.0f, 0.0f);
    _dv.set(0.0f, length_v, 0.0f);
    _dw.set(0.0f, 0.0f, length_w);

    _inverse_du.set(1.0f/length_u, 0.0f, 0.0f);
    _inverse_dv.set(0.0f, 1.0f/length_v, 0.0f);
    _inverse_dw.set(0.0f, 0.0f, 1.0f/length_w);
    
    osg::notify(osg::NOTICE)<<"Cell size X="<<length_u<<std::endl;
    osg::notify(osg::NOTICE)<<"Cell size Y="<<length_v<<std::endl;
    osg::notify(osg::NOTICE)<<"Cell size Z="<<length_w<<std::endl;


    osg::BoundingBox& bb = _parameters->boundingBox;
    if (bb.valid())
    {
        _origin.set(bb.xMin(), bb.yMin(), bb.zMin());
    }
    else
    {
        _origin.set(0.0f, 0.0f, 0.0f);
    }
    
    setUpGeometries(_parameters->particleDensity * cellVolume);
    
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        for(ViewDrawableMap::iterator itr=_viewDrawableMap.begin();
            itr!=_viewDrawableMap.end();
            ++itr)
        {
            PrecipitationDrawableSet& precipitationDrawableSet = itr->second;
            precipitationDrawableSet._quadPrecipitationDrawable = new PrecipitationDrawable;
            precipitationDrawableSet._quadPrecipitationDrawable->setGeometry(_quadGeometry.get());
            precipitationDrawableSet._quadPrecipitationDrawable->setStateSet(_quadStateSet.get());

            precipitationDrawableSet._linePrecipitationDrawable = new PrecipitationDrawable;
            precipitationDrawableSet._linePrecipitationDrawable->setGeometry(_lineGeometry.get());
            precipitationDrawableSet._linePrecipitationDrawable->setStateSet(_lineStateSet.get());

            precipitationDrawableSet._pointPrecipitationDrawable = new PrecipitationDrawable;
            precipitationDrawableSet._pointPrecipitationDrawable->setGeometry(_pointGeometry.get());
            precipitationDrawableSet._pointPrecipitationDrawable->setStateSet(_pointStateSet.get());

        }
    }    

    // set up state/
    {
        _precipitationStateSet = new osg::StateSet;

        const osg::BoundingBox& bb = _parameters->boundingBox;

        osg::Uniform* inversePeriodUniform = new osg::Uniform("inversePeriod",1.0f/_period);

        _precipitationStateSet->addUniform(inversePeriodUniform); // float

        _precipitationStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        _precipitationStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);

        osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
        _precipitationStateSet->addUniform(baseTextureSampler);

        osg::Texture2D* texture = new osg::Texture2D(createSpotLightImage(osg::Vec4(1.0f,1.0f,1.0f,1.0f),osg::Vec4(1.0f,1.0f,1.0f,0.0f),32,1.0));
        _precipitationStateSet->setTextureAttribute(0, texture);

        _precipitationStateSet->addUniform(new osg::Uniform("particleColour", _parameters->particleColour));
        _precipitationStateSet->addUniform(new osg::Uniform("particleSize", _parameters->particleSize));

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
        quad_geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, numParticles*4));
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
        line_geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, numParticles*2));
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
        point_geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, numParticles));
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
    unsigned int renderBin = 11;
    
    osg::notify(osg::NOTICE)<<"setUpGeometries("<<numParticles<<")"<<std::endl;

    {
        _quadGeometry = new osg::Geometry;
        _quadGeometry->setUseVertexBufferObjects(true);

        _quadStateSet = new osg::StateSet;
        
        osg::Program* program = new osg::Program;
        _quadStateSet->setAttribute(program);
        _quadStateSet->setRenderBinDetails(renderBin,"DepthSortedBin");

#ifdef USE_LOCAL_SHADERS
        char vertexShaderSource[] = 
            "uniform float inversePeriod;\n"
            "uniform vec4 particleColour;\n"
            "uniform float particleSize;\n"
            "\n"
            "uniform float osg_FrameTime;\n"
            "uniform float osg_DeltaFrameTime;\n"
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
            "    v_previous.z = fract( (osg_FrameTime - startTime)*inversePeriod - offset);\n"
            "    \n"
            "    vec4 v_current =  v_previous;\n"
            "    v_current.z += (osg_DeltaFrameTime*inversePeriod);\n"
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


    {
        _lineGeometry = new osg::Geometry;
        _lineGeometry->setUseVertexBufferObjects(true);

        _lineStateSet = new osg::StateSet;

        osg::Program* program = new osg::Program;
        _lineStateSet->setAttribute(program);
        _lineStateSet->setRenderBinDetails(renderBin,"DepthSortedBin");

#ifdef USE_LOCAL_SHADERS
        char vertexShaderSource[] = 
            "uniform float inversePeriod;\n"
            "uniform vec4 particleColour;\n"
            "uniform float particleSize;\n"
            "\n"
            "uniform float osg_FrameTime;\n"
            "uniform float osg_DeltaFrameTime;\n"
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
            "    v_previous.z = fract( (osg_FrameTime - startTime)*inversePeriod - offset);\n"
            "    \n"
            "    vec4 v_current =  v_previous;\n"
            "    v_current.z += (osg_DeltaFrameTime*inversePeriod);\n"
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


    {
        _pointGeometry = new osg::Geometry;
        _pointGeometry->setUseVertexBufferObjects(true);

        _pointStateSet = new osg::StateSet;

        osg::Program* program = new osg::Program;
        _pointStateSet->setAttribute(program);

#ifdef USE_LOCAL_SHADERS
        char vertexShaderSource[] = 
            "uniform float inversePeriod;\n"
            "uniform vec4 particleColour;\n"
            "uniform float particleSize;\n"
            "\n"
            "uniform float osg_FrameTime;\n"
            "\n"
            "varying vec4 colour;\n"
            "\n"
            "void main(void)\n"
            "{\n"
            "    float offset = gl_Vertex.z;\n"
            "    float startTime = gl_MultiTexCoord1.x;\n"
            "\n"
            "    vec4 v_current = gl_Vertex;\n"
            "    v_current.z = fract( (osg_FrameTime - startTime)*inversePeriod - offset);\n"
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

        _pointStateSet->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
        _pointStateSet->setRenderBinDetails(renderBin,"DepthSortedBin");
    }

    createGeometry(numParticles, _quadGeometry.get(), _lineGeometry.get(), _pointGeometry.get());

}

void PrecipitationEffect::cull(PrecipitationDrawableSet& pds, osgUtil::CullVisitor* cv) const
{
    osg::Timer_t startTick = osg::Timer::instance()->tick();

    pds._quadPrecipitationDrawable->newFrame();
    pds._linePrecipitationDrawable->newFrame();
    pds._pointPrecipitationDrawable->newFrame();

    osg::Matrix inverse_modelview;
    inverse_modelview.invert(cv->getModelViewMatrix());
    
    osg::Vec3 eyeLocal = osg::Vec3(0.0f,0.0f,0.0f) * inverse_modelview;
    //osg::notify(osg::NOTICE)<<"  eyeLocal "<<eyeLocal<<std::endl;
    
    float eye_k = (eyeLocal-_origin)*_inverse_dw;
    osg::Vec3 eye_kPlane = eyeLocal-_dw*eye_k-_origin;
    
    // osg::notify(osg::NOTICE)<<"  eye_kPlane "<<eye_kPlane<<std::endl;
    
    float eye_i = eye_kPlane*_inverse_du;
    float eye_j = eye_kPlane*_inverse_dv;
    
    osg::Polytope frustum;
    frustum.setToUnitFrustum(false,false);
    frustum.transformProvidingInverse(cv->getProjectionMatrix());
    frustum.transformProvidingInverse(cv->getModelViewMatrix());

    float i_delta = _parameters->farTransition * _inverse_du.x();
    float j_delta = _parameters->farTransition * _inverse_dv.y();
    float k_delta = 1;//_parameters->nearTransition * _inverse_dw.z();
    
    int i_min = (int)floor(eye_i - i_delta);
    int j_min = (int)floor(eye_j - j_delta);
    int k_min = (int)floor(eye_k - k_delta);

    int i_max = (int)ceil(eye_i + i_delta);
    int j_max = (int)ceil(eye_j + j_delta);
    int k_max = (int)ceil(eye_k + k_delta);
    
    //osg::notify(osg::NOTICE)<<"i_delta="<<i_delta<<" j_delta="<<j_delta<<" k_delta="<<k_delta<<std::endl;

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

    osg::Timer_t endTick = osg::Timer::instance()->tick();
    
    // osg::notify(osg::NOTICE)<<"time for cull "<<osg::Timer::instance()->delta_m(startTick,endTick)<<"ms  numTested="<<numTested<<" numInFrustum"<<numInFrustum<<std::endl;
    // osg::notify(osg::NOTICE)<<"     quads "<<pds._quadPrecipitationDrawable->getCurrentCellMatrixMap().size()<<"   lines "<<pds._linePrecipitationDrawable->getCurrentCellMatrixMap().size()<<"   points "<<pds._pointPrecipitationDrawable->getCurrentCellMatrixMap().size()<<std::endl;
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
    if (distance < _parameters->nearTransition)
    {
        PrecipitationDrawable::MatrixStartTimePair& mstp = pds._quadPrecipitationDrawable->getCurrentCellMatrixMap()[PrecipitationDrawable::Cell(i,k,j)];
        mymodelview = &mstp.first;
        mstp.second = startTime;
    }
    else if (distance <= _parameters->farTransition)
    {
        if (_parameters->useFarLineSegments)
        {
            PrecipitationDrawable::MatrixStartTimePair& mstp = pds._linePrecipitationDrawable->getCurrentCellMatrixMap()[PrecipitationDrawable::Cell(i,k,j)];
            mymodelview = &mstp.first;
            mstp.second = startTime;
        }
        else
        {
            PrecipitationDrawable::MatrixStartTimePair& mstp = pds._pointPrecipitationDrawable->getCurrentCellMatrixMap()[PrecipitationDrawable::Cell(i,k,j)];
            mymodelview = &mstp.first;
            mstp.second = startTime;
        }
    }
    else
    {
        return false;
    }

    *mymodelview = cv->getModelViewMatrix();
    mymodelview->preMult(osg::Matrix::translate(position));
    mymodelview->preMult(osg::Matrix::scale(scale));
    
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   Precipitation Drawable
//
////////////////////////////////////////////////////////////////////////////////////////////////////

PrecipitationEffect::PrecipitationDrawable::PrecipitationDrawable()
{
    setSupportsDisplayList(false);
}

PrecipitationEffect::PrecipitationDrawable::PrecipitationDrawable(const PrecipitationDrawable& copy, const osg::CopyOp& copyop):
    Drawable(copy,copyop)
{
}

void PrecipitationEffect::PrecipitationDrawable::drawImplementation(osg::State& state) const
{
    if (!_geometry) return;

    const osg::Geometry::Extensions* extensions = osg::Geometry::getExtensions(state.getContextID(),true);
    
    // save OpenGL matrices
    glPushMatrix();
    glMatrixMode( GL_TEXTURE );
    glPushMatrix();

    state.setActiveTextureUnit(0);

    for(CellMatrixMap::const_iterator itr = _currentCellMatrixMap.begin();
        itr != _currentCellMatrixMap.end();
        ++itr)
    {

        extensions->glMultiTexCoord1f(GL_TEXTURE0+1, itr->second.second);

        // load cells current modelview matrix
        glMatrixMode( GL_MODELVIEW );
        glLoadMatrix(itr->second.first.ptr());


        CellMatrixMap::const_iterator pitr = _previousCellMatrixMap.find(itr->first);
        if (pitr != _previousCellMatrixMap.end())
        {
            // load previous frame modelview matrix for motion blurr effect
            glMatrixMode( GL_TEXTURE );
            glLoadMatrix(pitr->second.first.ptr());    
        }
        else
        {
            // use current modelview matrix as "previous" frame value, cancelling motion blurr effect
            glMatrixMode( GL_TEXTURE );
            glLoadMatrix(itr->second.first.ptr());    
        }

        _geometry->draw(state);

    }

    // restore OpenGL matrices
    glMatrixMode( GL_TEXTURE );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();

    
}
