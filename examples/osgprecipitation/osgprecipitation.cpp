/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgUtil/CullVisitor>
#include <osgProducer/Viewer>

#include <osg/Point>
#include <osg/BlendFunc>
#include <osg/Texture2D>
#include <osg/PointSprite>
#include <osg/Program>
#include <osg/Fog>
#include <osg/Point>
#include <osg/PointSprite>
#include <osg/io_utils>

float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }

class PrecipitationGeometry : public osg::Geometry
{
public:

        PrecipitationGeometry()
        {
            setSupportsDisplayList(false);
        }

        virtual bool supports(const osg::PrimitiveFunctor&) const { return false; }
        virtual void accept(osg::PrimitiveFunctor&) const {}
        virtual bool supports(const osg::PrimitiveIndexFunctor&) const { return false; }
        virtual void accept(osg::PrimitiveIndexFunctor&) const {}

        void setInternalGeometry(osg::Geometry* geometry) { _internalGeometry = geometry; }
        
        osg::Geometry* getInternalGeometry() { return _internalGeometry.get(); }
        
        void setPosition(const osg::Vec3& position) { _position = position; }
        const osg::Vec3& getPosition() const { return _position; }

        
        virtual void compileGLObjects(osg::State& state) const
        {
            if (!_internalGeometry) return;

            static bool s_interalGeometryCompiled = false;
            if (!s_interalGeometryCompiled)
            {
                _internalGeometry->compileGLObjects(state);
                s_interalGeometryCompiled = true;
            }
        }

        virtual void drawImplementation(osg::State& state) const
        {
            if (!_internalGeometry) return;
            
            glNormal3fv(_position.ptr());

            _internalGeometry->draw(state);
        }

        virtual osg::BoundingBox computeBound() const
        {
            return osg::BoundingBox();
        }
        
protected:        

        osg::Vec3                   _position;
        osg::ref_ptr<osg::Geometry> _internalGeometry;

};

class CullCallback : public osg::NodeCallback
{
public:

    CullCallback(osg::Uniform* uniform):
        _previousFrame(0),
        _initialized(false),
        _uniform(uniform)
    {
    }

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    { 
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        if (cv)
        {
            if (!_initialized)
            {
                _previousModelViewMatrix = cv->getModelViewMatrix();
                _previousFrame = nv->getFrameStamp()->getFrameNumber();
                _initialized = true;
            }
        
            _uniform->set(_previousModelViewMatrix);
            
            // osg::notify(osg::NOTICE)<<"Updating uniform "<<_previousModelViewMatrix<<std::endl;

            traverse(node, nv);
            
            if (_previousFrame != nv->getFrameStamp()->getFrameNumber())
            {
                _previousModelViewMatrix = cv->getModelViewMatrix();
                _previousFrame = nv->getFrameStamp()->getFrameNumber();
            }
        }
        else
        {
            traverse(node, nv);
        }
    }
    
    int _previousFrame;
    bool _initialized;
    osg::Matrix _previousModelViewMatrix;
    osg::ref_ptr<osg::Uniform> _uniform;    
};

void fillSpotLightImage(unsigned char* ptr, const osg::Vec4& centerColour, const osg::Vec4& backgroudColour, unsigned int size, float power)
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


osg::Image* createSpotLightImage(const osg::Vec4& centerColour, const osg::Vec4& backgroudColour, unsigned int size, float power)
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

/** create quad, line and point geometry data all with consistent particle positions.*/
void createGeometry(unsigned int numParticles, 
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


static osg::ref_ptr<osg::Geometry> quad_geometry = 0;
static osg::ref_ptr<osg::StateSet> quad_stateset = 0;

static osg::ref_ptr<osg::Geometry> line_geometry = 0;
static osg::ref_ptr<osg::StateSet> line_stateset = 0;

static osg::ref_ptr<osg::Geometry> point_geometry = 0;
static osg::ref_ptr<osg::StateSet> point_stateset = 0;

void setUpGeometries(unsigned int numParticles)
{
    {
        quad_geometry = new osg::Geometry;
        quad_geometry->setUseVertexBufferObjects(true);

        quad_stateset = new osg::StateSet;
        osg::Program* program = new osg::Program;
        quad_stateset->setAttribute(program);
        quad_stateset->setRenderBinDetails(13,"DepthSortedBin");

        // get shaders from source
        program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("quad_rain.vert")));
        program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("rain.frag")));
    }


    {
        line_geometry = new osg::Geometry;
        line_geometry->setUseVertexBufferObjects(true);

        line_stateset = new osg::StateSet;

        osg::Program* program = new osg::Program;
        line_stateset->setAttribute(program);
        line_stateset->setRenderBinDetails(12,"DepthSortedBin");

        // get shaders from source
        program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("line_rain.vert")));
        program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("rain.frag")));
    }


    {
        point_geometry = new osg::Geometry;
        point_geometry->setUseVertexBufferObjects(true);

        point_stateset = new osg::StateSet;

        osg::Program* program = new osg::Program;
        point_stateset->setAttribute(program);

        // get shaders from source
        program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("point_rain.vert")));
        program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("rain.frag")));

        /// Setup the point sprites
        osg::PointSprite *sprite = new osg::PointSprite();
        point_stateset->setTextureAttributeAndModes(0, sprite, osg::StateAttribute::ON);

        point_stateset->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
        point_stateset->setRenderBinDetails(11,"DepthSortedBin");

    }

    createGeometry(numParticles, quad_geometry.get(), line_geometry.get(), point_geometry.get());

}


osg::Node* createRainEffect(const osg::BoundingBox& bb, const osg::Vec3& velocity)
{
    osg::LOD* lod = new osg::LOD;
    
    // time taken to get from start to the end of cycle
    float period = fabs((bb.zMax()-bb.zMin()) / velocity.z());

    // distance between start point and end of cyclce
    osg::Vec3 position(bb.xMin(), bb.yMin(), bb.zMax());
    osg::Vec3 dv_i( bb.xMax()-bb.xMin(), 0.0f, 0.0f );
    osg::Vec3 dv_j( 0.0f, bb.yMax()-bb.yMin(), 0.0f );
    osg::Vec3 dv_k( velocity * period );

    float nearDistance = 25.0;
    float farDistance = 100.0;
    
    // high res LOD.
    {
        osg::Geode* highres_geode = new osg::Geode;

        PrecipitationGeometry* geometry = new PrecipitationGeometry;

        highres_geode->addDrawable(geometry);

        geometry->setPosition(position);
        geometry->setInitialBound(bb);
        geometry->setInternalGeometry(quad_geometry.get());
        geometry->setStateSet(quad_stateset.get());
        
        lod->addChild( highres_geode, 0.0f, nearDistance );
    }
    
    // low res LOD
    {
        osg::Geode* lowres_geode = new osg::Geode;

        PrecipitationGeometry* geometry = new PrecipitationGeometry;

        lowres_geode->addDrawable(geometry);

        geometry->setPosition(position);
        geometry->setInitialBound(bb);
        geometry->setInternalGeometry(point_geometry.get());
        geometry->setStateSet(point_stateset.get());
        
        lod->addChild( lowres_geode, nearDistance, farDistance );
    }


    return lod;
}

osg::Node* createCellRainEffect(const osg::BoundingBox& bb, const osg::Vec3& velocity, unsigned int numParticles)
{
    
    unsigned int numX = 100;
    unsigned int numY = 100;
    unsigned int numCells = numX*numY;
    unsigned int numParticlesPerCell = numParticles/numCells;

    setUpGeometries(numParticlesPerCell);
    
    std::cout<<"Effect total number of particles = "<<numParticles<<std::endl;
    std::cout<<"Number of cells = "<<numCells<<std::endl;
    std::cout<<"Number of particles per cell = "<<numParticlesPerCell<<std::endl;
    std::cout<<"Cell width = "<<(bb.xMax()-bb.xMin())/(float)(numX)<<std::endl;
    std::cout<<"Cell length = "<<(bb.yMax()-bb.yMin())/(float)(numY)<<std::endl;
    std::cout<<"Cell height = "<<(bb.zMax()-bb.zMin())<<std::endl;

    osg::Group* group = new osg::Group;
    for(unsigned int i=0; i<numX; ++i)
    {
        for(unsigned int j=0; j<numX; ++j)
        {
            osg::BoundingBox bbLocal(bb.xMin() + ((bb.xMax()-bb.xMin())*(float)i)/(float)(numX),
                                     bb.yMin() + ((bb.yMax()-bb.yMin())*(float)j)/(float)(numY),
                                     bb.zMin(),
                                     bb.xMin() + ((bb.xMax()-bb.xMin())*(float)(i+1))/(float)(numX),
                                     bb.yMin() + ((bb.yMax()-bb.yMin())*(float)(j+1))/(float)(numY),
                                     bb.zMax());

            group->addChild(createRainEffect(bbLocal, velocity));
        }        
    }
    
#if 1    
    osgUtil::Optimizer::SpatializeGroupsVisitor sgv;
    group->accept(sgv);
    sgv.divide();
#endif    

#if 1
    osg::StateSet* stateset = group->getOrCreateStateSet();

    // time taken to get from start to the end of cycle
    float period = fabs((bb.zMax()-bb.zMin()) / velocity.z());

    // distance between start point and end of cyclce
    osg::Vec3 dv_i( (bb.xMax()-bb.xMin())/(float)(numX), 0.0f, 0.0f );
    osg::Vec3 dv_j( 0.0f, (bb.yMax()-bb.yMin())/(float)(numY), 0.0f );
    osg::Vec3 dv_k( velocity * period );

    // set up uniforms
    // osg::Uniform* position_Uniform = new osg::Uniform("position",position);
    osg::Uniform* dv_i_Uniform = new osg::Uniform("dv_i",dv_i);
    osg::Uniform* dv_j_Uniform = new osg::Uniform("dv_j",dv_j);
    osg::Uniform* dv_k_Uniform = new osg::Uniform("dv_k",dv_k);

    osg::Uniform* inversePeriodUniform = new osg::Uniform("inversePeriod",1.0f/period);
    osg::Uniform* startTime = new osg::Uniform("startTime",0.0f);

    //stateset->addUniform(position_Uniform); // vec3
    stateset->addUniform(dv_i_Uniform); // vec3 could be float 
    stateset->addUniform(dv_j_Uniform); // vec3 could be float
    stateset->addUniform(dv_k_Uniform); // vec3
    stateset->addUniform(inversePeriodUniform); // float
    stateset->addUniform(startTime); // float

    stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    stateset->setMode(GL_BLEND, osg::StateAttribute::ON);

    osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
    stateset->addUniform(baseTextureSampler);

    osg::Texture2D* texture = new osg::Texture2D(createSpotLightImage(osg::Vec4(1.0f,1.0f,1.0f,1.0f),osg::Vec4(1.0f,1.0f,1.0f,0.0f),32,1.0));
    stateset->setTextureAttribute(0, texture);

    stateset->addUniform(new osg::Uniform("particleColour", osg::Vec4(0.6,0.6,0.6,1.0)));
    stateset->addUniform(new osg::Uniform("particleSize", 0.02f));
  
    osg::Uniform* previousModelViewUniform = new osg::Uniform("previousModelViewMatrix",osg::Matrix());
    stateset->addUniform(previousModelViewUniform);
    
    group->setCullCallback(new CullCallback(previousModelViewUniform));
#endif

    return group;
}

osg::Node* createModel(osg::Node* loadedModel, bool /*useShaders*/)
{
    osg::Group* group = new osg::Group;

    osg::BoundingBox bb(0.0, 0.0, 0.0, 100.0, 100.0, 100.0);
    osg::Vec3 velocity(0.0,2.0,-8.0);
    unsigned int numParticles = 50000000;
    
    if (loadedModel)
    {
        group->addChild(loadedModel);
        
        osg::BoundingSphere bs = loadedModel->getBound();

        bb.set( -500, -500, 0, +500, +500, 10);
        
        osg::StateSet* stateset = loadedModel->getOrCreateStateSet();
        
        osg::Fog* fog = new osg::Fog;
        fog->setMode(osg::Fog::LINEAR);
        fog->setDensity(0.1f);
        fog->setStart(0.0f);
        fog->setEnd(1000.0f);
        fog->setColor(osg::Vec4(0.5f,0.5f,0.5f,1.0f));
        stateset->setAttributeAndModes(fog, osg::StateAttribute::ON);
        
        osg::LightSource* lightSource = new osg::LightSource;
        group->addChild(lightSource);

        osg::Light* light = lightSource->getLight();
        light->setLightNum(0);
        light->setPosition(osg::Vec4(0.0f,0.0f,1.0f,0.0f)); // directional light from above
        light->setAmbient(osg::Vec4(0.8f,0.8f,0.8f,1.0f));
        light->setDiffuse(osg::Vec4(0.2f,0.2f,0.2f,1.0f));
        light->setSpecular(osg::Vec4(0.2f,0.2f,0.2f,1.0f));

                
    }
    
    group->addChild(createCellRainEffect(bb, velocity, numParticles));

    return group;    
}

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" example provides an interactive viewer for visualising point clouds..");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--shader","Use GLSL shaders.");
    arguments.getApplicationUsage()->addCommandLineOption("--fixed","Use fixed function pipeline.");
    

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    bool shader = true;
    while (arguments.read("--shader")) shader = true;
    while (arguments.read("--fixed")) shader = false;

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
    osg::Timer timer;
    osg::Timer_t start_tick = timer.tick();

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    
    loadedModel = createModel(loadedModel.get(), shader);

    // if no model has been successfully loaded report failure.
    if (!loadedModel) 
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    osg::Timer_t end_tick = timer.tick();

    std::cout << "Time to load = "<<timer.delta_s(start_tick,end_tick)<<std::endl;

    // optimize the scene graph, remove rendundent nodes and state etc.
    osgUtil::Optimizer optimizer;
    // optimizer.optimize(loadedModel.get());

    // set the scene to render
    viewer.setSceneData(loadedModel.get());

    // create the windows and run the threads.
    viewer.realize();

    while( !viewer.done() )
    {
        // wait for all cull and draw threads to complete.
        viewer.sync();

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        viewer.update();
         
        // fire off the cull and draw traversals of the scene.
        viewer.frame();
        
    }
    
    // wait for all cull and draw threads to complete before exit.
    viewer.sync();

    return 0;
}

