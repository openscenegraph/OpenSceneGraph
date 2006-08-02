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

#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgProducer/Viewer>

#include <osg/Material>
#include <osg/Geode>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Projection>
#include <osg/AutoTransform>
#include <osg/Geometry>

#include <osgText/Text>

osg::Node* createLabel(const osg::Vec3& pos, float size, const std::string& label, osgText::Text::AxisAlignment axisAlignment)
{
    osg::Geode* geode = new osg::Geode();
    
    std::string timesFont("fonts/arial.ttf");

    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );

        text->setFont(timesFont);
        text->setPosition(pos);
        text->setCharacterSize(size);
        text->setAxisAlignment(axisAlignment);
        text->setAlignment(osgText::Text::CENTER_CENTER);
        text->setText(label);
        
    }
    
    return geode;    
}


osg::Node* createLabel3(const osg::Vec3& pos, float size, const std::string& label)
{
    osg::Geode* geode = new osg::Geode();
    
    std::string timesFont("fonts/arial.ttf");

    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );

        text->setFont(timesFont);
        text->setPosition(pos);
        text->setFontResolution(40,40);
        text->setCharacterSize(size);
        text->setAlignment(osgText::Text::CENTER_CENTER);
        text->setAutoRotateToScreen(true);
        text->setCharacterSizeMode(osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);
        text->setText(label);
        
    }
    
    return geode;    
}

osg::Node* createAxis(const osg::Vec3& s, const osg::Vec3& e, int numReps, osg::AutoTransform::AutoRotateMode autoRotateMode, osgText::Text::AxisAlignment axisAlignment, const std::string& str)
{
    osg::Group* group = new osg::Group;

    osg::Vec3 dv = e-s;
    dv /= float(numReps-1);

    osg::Vec3 pos = s;

    bool useAuto = true;
    if (useAuto)
    {
        osg::Vec3Array* vertices = new osg::Vec3Array;

        for(int i=0;i<numReps;++i)
        {
            osg::AutoTransform* at = new osg::AutoTransform;
            at->setPosition(pos);
            at->setAutoRotateMode(autoRotateMode);
            at->addChild(createLabel(osg::Vec3(0.0f,0.0f,0.0f),dv.length()*0.2f,str, axisAlignment));
            vertices->push_back(pos);
            pos += dv;


            group->addChild(at);
        }

        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

        osg::Geometry* geom = new osg::Geometry;
        geom->setVertexArray(vertices);
        geom->setColorArray(colors);
        geom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP,0,vertices->size()));

        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(geom);

        group->addChild(geode);
    }
    else
    {
        osg::Vec3Array* vertices = new osg::Vec3Array;

        for(int i=0;i<numReps;++i)
        {
            group->addChild(createLabel3(osg::Vec3(pos),dv.length()*0.5f,str));
            vertices->push_back(pos);
            pos += dv;


        }

        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

        osg::Geometry* geom = new osg::Geometry;
        geom->setVertexArray(vertices);
        geom->setColorArray(colors);
        geom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP,0,vertices->size()));

        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(geom);

        group->addChild(geode);
    }
    
    return group;
}

osg::Node* createScene()
{
    osg::Group* root = new osg::Group;
    
//    int numReps = 3333;
    int numReps = 10;
    root->addChild(createAxis(osg::Vec3(0.0,0.0,0.0),osg::Vec3(1000.0,0.0,0.0),numReps,osg::AutoTransform::ROTATE_TO_CAMERA,osgText::Text::XY_PLANE, "ROTATE_TO_CAMERA"));
    root->addChild(createAxis(osg::Vec3(0.0,0.0,0.0),osg::Vec3(0.0,1000.0,0.0),numReps,osg::AutoTransform::ROTATE_TO_SCREEN,osgText::Text::XY_PLANE, "ROTATE_TO_SCREEN"));
    root->addChild(createAxis(osg::Vec3(0.0,0.0,0.0),osg::Vec3(0.0,0.0,1000.0),numReps,osg::AutoTransform::NO_ROTATION,osgText::Text::XZ_PLANE, "NO_ROTATION"));    
    
    return root;
}

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates how to do Head Up Displays.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] [filename] ...");
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

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    

    // set the scene to render
    viewer.setSceneData(createScene());

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


//     osg::Node* obj = new osg::Node;
//     cout << "obj = "<<obj<<endl;
//     cout << "sizeof(obj) = "<<sizeof(*obj)<<endl;
//     cout << "sizeof(osg::Object) = "<<sizeof(osg::Object)<<endl;
//     cout << "sizeof(osg::Referenced) = "<<sizeof(osg::Referenced)<<endl;
//     cout << "vtbl = "<<*((unsigned int*)obj)<<endl;
//     cout << "next = "<<*((unsigned int*)obj+1)<<endl;
// 
//     osg::Node* obj2 = new osg::Node;
//     obj2->ref();
//     obj2->ref();
//     obj2->ref();
//     cout << "obj = "<<obj2<<endl;
//     cout << "sizeof(obj) = "<<sizeof(*obj)<<endl;
//     cout << "sizeof(osg::Object) = "<<sizeof(osg::Object)<<endl;
//     cout << "sizeof(osg::Referenced) = "<<sizeof(osg::Referenced)<<endl;
//     cout << "vtbl = "<<*((unsigned int*)obj2)<<endl;
//     cout << "next = "<<*((unsigned int*)obj2+1)<<endl;
// 
//     osg::Group* obj3 = new osg::Group;
//     cout << "obj = "<<obj3<<endl;
//     cout << "sizeof(obj) = "<<sizeof(*obj3)<<endl;
//     cout << "sizeof(osg::Object) = "<<sizeof(osg::Object)<<endl;
//     cout << "sizeof(osg::Referenced) = "<<sizeof(osg::Referenced)<<endl;
//     cout << "vtbl = "<<*((unsigned int*)obj3)<<endl;
//     cout << "next = "<<*((unsigned int*)obj3+1)<<endl;
}
