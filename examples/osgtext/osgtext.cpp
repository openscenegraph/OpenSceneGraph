/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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
#include <osg/MatrixTransform>

#include <osgText/Text>



osg::Group* createHUDText()
{

    osg::Geode* geode  = new osg::Geode;

    osgText::Text* text1 = new osgText::Text;
    text1->setCharacterSize(12.0f);
    text1->setText("20 GOTO 10");
    geode->addDrawable(text1);

    osgText::Text* text2 = new osgText::Text;
    text2->setPosition(osg::Vec3(0.0f,50.0f,0.0f));
    text2->setCharacterSize(12.0f);
    text2->setText("10 PRINT \"Hello World\"");
    geode->addDrawable(text2);

    osgText::Text* text3 = new osgText::Text;
    text3->setFont(osgText::readFontFile("fonts/arial.ttf"));
    text3->setPosition(osg::Vec3(100.0f,100.0f,0.0f));
    text3->setText("This is a test of text AVAV/.|¬!£$%^&*() - fonts/arial.ttf");
    geode->addDrawable(text3);

    osgText::Text* text4 = new osgText::Text;
    text4->setFont(osgText::readFontFile("fonts/times.ttf"));
    text4->setFontSize(64,64);
    text4->setPosition(osg::Vec3(200.0f,200.0f,0.0f));
    text4->setLayout(osgText::Text::RIGHT_TO_LEFT);
    text4->setDrawMode(osgText::Text::TEXT|osgText::Text::ALIGNMENT|osgText::Text::BOUNDINGBOX);
    text4->setText("This is a test of text AVAV/.|¬!£$%^&*() - fonts/times.ttf");
    geode->addDrawable(text4);

    osgText::Text* text5 = new osgText::Text;
    text5->setFont(osgText::readFontFile("fonts/dirtydoz.ttf"));
    text5->setPosition(osg::Vec3(300.0f,300.0f,0.0f));
    text5->setColor(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
    text5->setLayout(osgText::Text::VERTICAL);
    text5->setDrawMode(osgText::Text::TEXT|osgText::Text::ALIGNMENT|osgText::Text::BOUNDINGBOX);
    text5->setText("This is a test of text AVAV/.|¬!£$%^&*() - fonts/dirtydoz.ttf");
    geode->addDrawable(text5);


//     osgText::Text::TextString string;
//     for(int i=0;i<2048;++i)
//         string.push_back(i);
// 
//     osgText::Text* text6 = new osgText::Text;
//     text6->setFont(osgText::readFontFile("/home/robert/Documents/GuopingSun/msmincho.ttc"));
//     text6->setFontSize(64,64);
//     text6->setText(string);
//     text6->setPosition(osg::Vec3(00.0f,400.0f,0.0f));
//     geode->addDrawable(text6);

    


    osg::Group* rootNode = new osg::Group;
    rootNode->addChild(geode);

    return rootNode;    
}

osg::Group* create3DText()
{

    osg::Geode* geode  = new osg::Geode;

    osgText::Text* text1 = new osgText::Text;
    text1->setFont("fonts/times.ttf");
    text1->setAxisAlignment(osgText::Text::XY_PLANE);
    text1->setText("XY_PLANE");
    geode->addDrawable(text1);

    osgText::Text* text2 = new osgText::Text;
    text2->setFont("fonts/times.ttf");
    text2->setPosition(osg::Vec3(0.0f,0.0f,0.0f));
    text2->setAxisAlignment(osgText::Text::YZ_PLANE);
    text2->setText("YZ_PLANE");
    geode->addDrawable(text2);

    osgText::Text* text3 = new osgText::Text;
    text3->setFont("fonts/times.ttf");
    text3->setPosition(osg::Vec3(00.0f,00.0f,00.0f));
    text3->setAxisAlignment(osgText::Text::XZ_PLANE);
    text3->setText("XZ_PLANE");
    geode->addDrawable(text3);

    osgText::Text* text4 = new osgText::Text;
    text4->setFont("fonts/times.ttf");
    text4->setAxisAlignment(osgText::Text::SCREEN);
    text4->setPosition(osg::Vec3(00.0f,00.0f,00.0f));
    text4->setText("SCREEN");
    geode->addDrawable(text4);

    osg::Group* rootNode = new osg::Group;
    rootNode->addChild(geode);

    return rootNode;    
}

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getProgramName()+" [options] [filename] ...");
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
        arguments.getApplicationUsage()->write(cout);
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(cout);
        return 1;
    }


    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> rootNode = osgDB::readNodeFiles(arguments);

    // prepare scene.
    {

        if (rootNode.valid())
        {
            // optimize the scene graph, remove rendundent nodes and state etc.
            osgUtil::Optimizer optimizer;
            optimizer.optimize(rootNode.get());
        }

        // make sure the root node is group so we can add extra nodes to it.
        osg::Group* group = dynamic_cast<osg::Group*>(rootNode.get());
        if (!group)
        {
            group = new osg::Group;
            
            if (rootNode.valid()) group->addChild(rootNode.get());
            
            rootNode = group;
        }

        {
            // create the hud.
            osg::Projection* projection = new osg::Projection;
            projection->setMatrix(osg::Matrix::ortho2D(0,1280,0,1024));

            osg::MatrixTransform* modelview_abs = new osg::MatrixTransform;
            modelview_abs->setReferenceFrame(osg::Transform::RELATIVE_TO_ABSOLUTE);
            modelview_abs->setMatrix(osg::Matrix::identity());

            modelview_abs->addChild(createHUDText());

            projection->addChild(modelview_abs);

            group->addChild(projection);
        }

        osg::MatrixTransform* scale = new osg::MatrixTransform;
        scale->setMatrix(osg::Matrix::scale(1.0f,1.0f,1.0f));
        scale->addChild(create3DText());
        group->addChild(scale);

    }

    // set the scene to render
    viewer.setSceneData(rootNode.get());

    // create the windows and run the threads.
    viewer.realize(Producer::CameraGroup::ThreadPerCamera);

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
    
    return 0;
}
