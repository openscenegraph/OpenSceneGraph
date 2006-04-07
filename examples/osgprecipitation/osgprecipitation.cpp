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
#include <osg/io_utils>

float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }

class PrecipitationGeometry : public osg::Geometry
{
public:
        virtual bool supports(const osg::PrimitiveFunctor&) const { return false; }
        virtual void accept(osg::PrimitiveFunctor&) const {}
        virtual bool supports(const osg::PrimitiveIndexFunctor&) const { return false; }
        virtual void accept(osg::PrimitiveIndexFunctor&) const {}

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

osg::Node* createRainEffect(const osg::BoundingBox& bb, const osg::Vec3& velocity, unsigned int numParticles, bool useShaders)
{
    osg::Geode* geode = new osg::Geode;

    osg::Geometry* geometry = new PrecipitationGeometry;
    geode->addDrawable(geometry);

    osg::StateSet* stateset = geometry->getOrCreateStateSet();

    // set up geometry.
    {
    
        // per vertex properties
        osg::Vec3Array* vertices = new osg::Vec3Array(numParticles*4);
        osg::Vec3Array* offsets = new osg::Vec3Array(numParticles*4);
        
        osg::Vec3 frameDelta = velocity*(2.0f/60.0f);
        float size = 1.0;
        
        for(unsigned int i=0; i< numParticles; ++i)
        {
            (*vertices)[i*4].set(random(bb.xMin(), bb.xMax()), random(bb.yMin(),bb.yMax()), bb.zMax());
            (*vertices)[i*4+1] = (*vertices)[i*4];
            (*vertices)[i*4+2] = (*vertices)[i*4];
            (*vertices)[i*4+3] = (*vertices)[i*4];
            (*offsets)[i*4].z() = random(0.0, 1.0);
            (*offsets)[i*4+1].z() = (*offsets)[i*4].z();
            (*offsets)[i*4+2].z() = (*offsets)[i*4].z();
            (*offsets)[i*4+3].z() = (*offsets)[i*4].z();
            (*offsets)[i*4].x() = 0.0;
            (*offsets)[i*4].y() = 0.0;
            (*offsets)[i*4+1].x() = 0.0;
            (*offsets)[i*4+1].y() = 1.0;
            (*offsets)[i*4+2].x() = 1.0;
            (*offsets)[i*4+2].y() = 1.0;
            (*offsets)[i*4+3].x() = 1.0;
            (*offsets)[i*4+3].y() = 0.0;
        }

        geometry->setVertexArray(vertices);
        geometry->setTexCoordArray(0, offsets);

        // overall attributes
        osg::Vec4Array* colours = new osg::Vec4Array(1);
        (*colours)[0].set(0.5f,0.5f,0.5f,1.0f);
        geometry->setColorArray(colours);
        geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        
        geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, numParticles*4));
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
        program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("rain.vert")));
        program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("rain.frag")));

        stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
        
        stateset->addUniform(deltaUniform);
        stateset->addUniform(inversePeriodUniform);
        stateset->addUniform(startTime);
        
        osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
        stateset->addUniform(baseTextureSampler);
        
        osg::Texture2D* texture = new osg::Texture2D(osgDB::readImageFile("Images/particle.rgb"));
        stateset->setTextureAttribute(0, texture);

        // make it render after the normal transparent bin
        stateset->setRenderBinDetails(11,"DepthSortedBin");

        osg::Uniform* previousModelViewUniform = new osg::Uniform("previousModelViewMatrix",osg::Matrix());
        stateset->addUniform(previousModelViewUniform);
        geode->setCullCallback(new CullCallback(previousModelViewUniform));

    }
    
    geometry->setUseVertexBufferObjects(true);
    geometry->setInitialBound(bb);

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
    osg::Vec3 velocity(0.0,0.0,-2.0);
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

