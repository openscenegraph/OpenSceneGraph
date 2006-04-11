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
        virtual bool supports(const osg::PrimitiveFunctor&) const { return false; }
        virtual void accept(osg::PrimitiveFunctor&) const {}
        virtual bool supports(const osg::PrimitiveIndexFunctor&) const { return false; }
        virtual void accept(osg::PrimitiveIndexFunctor&) const {}
        
        virtual osg::BoundingBox computeBound() const
        {
            return osg::BoundingBox();
        }

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
        std::cout<<" i= "<<i<<" s="<<s<<" p="<<totalSize<<std::endl;
    }

    std::cout<<"Total size ="<<totalSize<<std::endl;

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


osg::Node* createRainEffect(const osg::BoundingBox& bb, const osg::Vec3& velocity, unsigned int numParticles, bool useShaders)
{
    osg::Geode* geode = new osg::Geode;

    osg::Geometry* quad_geometry = 0;
    osg::Geometry* line_geometry = 0;
    osg::Geometry* point_geometry = 0;
    
#if 0
    quad_geometry = new PrecipitationGeometry;
    quad_geometry->setUseVertexBufferObjects(true);
    quad_geometry->setInitialBound(bb);
    geode->addDrawable(quad_geometry);

    osg::StateSet* quad_stateset = quad_geometry->getOrCreateStateSet();
    {
        osg::Program* program = new osg::Program;
        quad_stateset->setAttribute(program);

        // get shaders from source
        program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("quad_rain.vert")));
        program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("rain.frag")));
    }
#endif

#if 0   
    line_geometry = new PrecipitationGeometry;
    line_geometry->setUseVertexBufferObjects(true);
    line_geometry->setInitialBound(bb);
    geode->addDrawable(line_geometry);

    osg::StateSet* line_stateset = line_geometry->getOrCreateStateSet();
    {
        osg::Program* program = new osg::Program;
        line_stateset->setAttribute(program);

        // get shaders from source
        program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("line_rain.vert")));
        program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("rain.frag")));
    }
#endif


#if 1    
    point_geometry = new PrecipitationGeometry;
    point_geometry->setUseVertexBufferObjects(true);
    point_geometry->setInitialBound(bb);
    geode->addDrawable(point_geometry);

    osg::StateSet* point_stateset = point_geometry->getOrCreateStateSet();
    {
        osg::Program* program = new osg::Program;
        point_stateset->setAttribute(program);

        // get shaders from source
        program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("point_rain.vert")));
        program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("rain.frag")));

        /// Setup the point sprites
        osg::PointSprite *sprite = new osg::PointSprite();
        point_stateset->setTextureAttributeAndModes(0, sprite, osg::StateAttribute::ON);

        point_stateset->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);

    }
#endif


    createGeometry(numParticles, quad_geometry, line_geometry, point_geometry);


    // set up state.
    osg::StateSet* stateset = geode->getOrCreateStateSet();
    {
        // time taken to get from start to the end of cycle
        float period = fabs((bb.zMax()-bb.zMin()) / velocity.z());

        // distance between start point and end of cyclce
        osg::Vec3 position(bb.xMin(), bb.yMin(), bb.zMax());
        osg::Vec3 dv_i( bb.xMax()-bb.xMin(), 0.0f, 0.0f );
        osg::Vec3 dv_j( 0.0f, bb.yMax()-bb.yMin(), 0.0f );
        osg::Vec3 dv_k( velocity * period );

        // set up uniforms
        osg::Uniform* position_Uniform = new osg::Uniform("position",position);
        osg::Uniform* dv_i_Uniform = new osg::Uniform("dv_i",dv_i);
        osg::Uniform* dv_j_Uniform = new osg::Uniform("dv_j",dv_j);
        osg::Uniform* dv_k_Uniform = new osg::Uniform("dv_k",dv_k);
        
        osg::Uniform* inversePeriodUniform = new osg::Uniform("inversePeriod",1.0f/period);
        osg::Uniform* startTime = new osg::Uniform("startTime",0.0f);


        stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
        
        stateset->addUniform(position_Uniform);
        stateset->addUniform(dv_i_Uniform);
        stateset->addUniform(dv_j_Uniform);
        stateset->addUniform(dv_k_Uniform);
        stateset->addUniform(inversePeriodUniform);
        stateset->addUniform(startTime);
        stateset->addUniform(new osg::Uniform("particleColour", osg::Vec4(0.6,0.6,0.6,1.0)));
        
        osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
        stateset->addUniform(baseTextureSampler);
        
//        osg::Texture2D* texture = new osg::Texture2D(osgDB::readImageFile("Images/particle.rgb"));
        osg::Texture2D* texture = new osg::Texture2D(createSpotLightImage(osg::Vec4(1.0f,1.0f,1.0f,1.0f),osg::Vec4(1.0f,1.0f,1.0f,0.0f),32,1.0));
//        texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
        stateset->setTextureAttribute(0, texture);

        // make it render after the normal transparent bin
        stateset->setRenderBinDetails(11,"DepthSortedBin");

        osg::Uniform* previousModelViewUniform = new osg::Uniform("previousModelViewMatrix",osg::Matrix());
        stateset->addUniform(previousModelViewUniform);
        geode->setCullCallback(new CullCallback(previousModelViewUniform));

    }
    




    return geode;
}
/*
osg::Node* createSnowEffect(const osg::BoundingBox& bb, const osg::Vec3& velocity, unsigned int numParticles, bool useShaders)
{
    osg::Geometry* geometry = new osg::Geometry;

    osg::StateSet* stateset = geometry->getOrCreateStateSet();

    // set up geometry.
    {
    
        // per vertex properties
        osg::Vec3Array* vertices = new osg::Vec3Array(numParticles);
        osg::FloatArray* offsets = new osg::FloatArray(numParticles);
        
        for(unsigned int i=0; i< numParticles; ++i)
        {
            (*vertices)[i].set(random(bb.xMin(), bb.xMax()), random(bb.yMin(),bb.yMax()), bb.zMax());
            (*offsets)[i] = random(0.0, 1.0);
        }

        geometry->setVertexArray(vertices);
        geometry->setTexCoordArray(0, offsets);

        // overall attributes
        osg::Vec4Array* colours = new osg::Vec4Array(1);
        (*colours)[0].set(1.0f,1.0f,1.0f,1.0f);
        geometry->setColorArray(colours);
        geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        
        geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, numParticles));
    }

    // set up state.
    {
        // time taken to get from start to the end of cycle
        float period = fabs((bb.zMax()-bb.zMin()) / velocity.z());

        // distance between start point and end of cyclce
        osg::Vec3 delta = velocity * period;

        // set up uniforms
        osg::Uniform* deltaUniform = new osg::Uniform("delta",delta);
        osg::Uniform* inversePeriodUniform = new osg::Uniform("inversePeriod",1.0f/period);
        osg::Uniform* startTime = new osg::Uniform("startTime",0.0f);

        osg::Program* program = new osg::Program;
        stateset->setAttribute(program);

        // get shaders from source
        program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("snow.vert")));
        program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("snow.frag")));

        stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        
        stateset->addUniform(deltaUniform);
        stateset->addUniform(inversePeriodUniform);
        stateset->addUniform(startTime);
    }
    
    geometry->setInitialBound(bb);


    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geometry);

    return geode;
}
*/
osg::Node* createModel(osg::Node* loadedModel, bool useShaders)
{
    osg::Group* group = new osg::Group;

    osg::BoundingBox bb(0.0, 0.0, 0.0, 100.0, 100.0, 100.0);
    osg::Vec3 velocity(0.0,2.0,-5.0);
    unsigned int numParticles = 150000;
    
    if (loadedModel)
    {
        group->addChild(loadedModel);
        
        osg::BoundingSphere bs = loadedModel->getBound();

        bb.set( -100, -100, 0, +100, +100, 10);
        
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

    group->addChild(createRainEffect(bb, velocity, numParticles, useShaders));

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
    optimizer.optimize(loadedModel.get());

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

