/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osg/Group>
#include <osg/Geometry>

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgProducer/Viewer>

float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }


struct DrawCallback : public osg::Drawable::DrawCallback
{

    DrawCallback():
        _firstTime(true) {}

    virtual void drawImplementation(osg::State& state,const osg::Drawable* drawable) const
    {
    
        if (!_firstTime)
        {
            _previousModelViewMatrix = _currentModelViewMatrix;
            _currentModelViewMatrix = state.getModelViewMatrix();
            _inverseModelViewMatrix.invert(_currentModelViewMatrix);
            
            osg::Matrix T(_previousModelViewMatrix*_inverseModelViewMatrix);

            osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(const_cast<osg::Drawable*>(drawable));
            osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
            for(unsigned int i=0;i+1<vertices->size();i+=2)
            {
                (*vertices)[i+1] = (*vertices)[i]*T;
            }
        }
        else
        {
            _currentModelViewMatrix = state.getModelViewMatrix();
        }
                
        _firstTime = false;

        drawable->drawImplementation(state);
    }
    
    mutable bool _firstTime;
    mutable osg::Matrix _currentModelViewMatrix;
    mutable osg::Matrix _inverseModelViewMatrix;
    mutable osg::Matrix _previousModelViewMatrix;
};




osg::Node* createScene(unsigned int noStars)
{
    
    osg::Geometry* geometry = new osg::Geometry;
    
    // set up vertices
    osg::Vec3Array* vertices = new osg::Vec3Array(noStars*2);
    geometry->setVertexArray(vertices);
    
    float min = -1.0f;
    float max = 1.0f;
    unsigned int j = 0;
    unsigned int i = 0;
    for(i=0;i<noStars;++i,j+=2)
    {
        (*vertices)[j].set(random(min,max),random(min,max),random(min,max));
        (*vertices)[j+1] = (*vertices)[j]+osg::Vec3(0.0f,0.0f,0.001f);
    }    

    // set up colours
    osg::Vec4Array* colours = new osg::Vec4Array(1);
    geometry->setColorArray(colours);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    (*colours)[0].set(1.0f,1.0f,1.0f,1.0f);

    // set up the primitive set to draw lines
    geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINES,0,noStars*2));

    // set up the points for the stars.
    osg::DrawElementsUShort* points = new osg::DrawElementsUShort(GL_POINTS,noStars);
    geometry->addPrimitiveSet(points);
    for(i=0;i<noStars;++i)
    {
        (*points)[i] = i*2;
    }

    geometry->setUseDisplayList(false);
    geometry->setDrawCallback(new DrawCallback);
    
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geometry);
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    osg::Group* group = new osg::Group;
    group->addChild(geode);        
    
    return group;
}

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" example demonstrate how to use a draw callback to create a space warp effect .");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = createScene(50000);

    // if no model has been successfully loaded report failure.
    if (!loadedModel) 
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
    }

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
    
    // wait for all cull and draw threads to complete.
    viewer.sync();

    // run a clean up frame to delete all OpenGL objects.
    viewer.cleanup_frame();

    // wait for all the clean up frame to complete.
    viewer.sync();

    return 0;
}

