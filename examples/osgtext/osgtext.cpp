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
#include <osgDB/WriteFile>

#include <osgProducer/Viewer>

#include <osg/Geode>
#include <osg/Projection>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osg/Sequence>

#include <osgText/Font>
#include <osgText/Text>


osg::Group* createHUDText()
{

    osg::Group* rootNode = new osg::Group;

    osgText::Font* font = osgText::readFontFile("fonts/arial.ttf");

    osg::Geode* geode  = new osg::Geode;
    rootNode->addChild(geode);

    float windowHeight = 1024.0f;
    float windowWidth = 1280.0f;
    float margin = 50.0f;


    osg::Vec4 layoutColor(1.0f,1.0f,0.0f,1.0f);
    float layoutCharacterSize = 20.0f;    
    
    {
        osgText::Text* text = new osgText::Text;
        text->setFont(font);
        text->setColor(layoutColor);
        text->setCharacterSize(layoutCharacterSize);
        text->setPosition(osg::Vec3(margin,windowHeight-margin,0.0f));

        // the default layout is left to right, typically used in languages
        // originating from europe such as English, French, German, Spanish etc..
        text->setLayout(osgText::Text::LEFT_TO_RIGHT);

        text->setText("text->setLayout(osgText::Text::LEFT_TO_RIGHT);");
        geode->addDrawable(text);
    }

    {
        osgText::Text* text = new osgText::Text;
        text->setFont(font);
        text->setColor(layoutColor);
        text->setCharacterSize(layoutCharacterSize);
        text->setPosition(osg::Vec3(windowWidth-margin,windowHeight-margin,0.0f));

        // right to left layouts would be used for hebrew or arabic fonts.
        text->setLayout(osgText::Text::RIGHT_TO_LEFT);

        text->setText("text->setLayout(osgText::Text::RIGHT_TO_LEFT);");
        geode->addDrawable(text);
    }

    {
        osgText::Text* text = new osgText::Text;
        text->setFont(font);
        text->setColor(layoutColor);
        text->setPosition(osg::Vec3(margin,windowHeight-margin,0.0f));
        text->setCharacterSize(layoutCharacterSize);

        // vertical font layout would be used for asian fonts.
        text->setLayout(osgText::Text::VERTICAL);

        text->setText("text->setLayout(osgText::Text::VERTICAL);");
        geode->addDrawable(text);
    }
    
    
    osg::Vec4 fontSizeColor(0.0f,1.0f,1.0f,1.0f);
    float fontSizeCharacterSize = 30;
    
    osg::Vec3 cursor = osg::Vec3(margin*2,windowHeight-margin*2,0.0f);
    
    {
        osgText::Text* text = new osgText::Text;
        text->setFont(font);
        text->setColor(fontSizeColor);
        text->setCharacterSize(fontSizeCharacterSize);
        text->setPosition(cursor);
        
        // use text that uses 10 by 10 texels as a target resolution for fonts.
        text->setFontSize(10,10); // blocky but small texture memory usage
        
        text->setText("text->setFontSize(10,10); // blocky but small texture memory usage");
        geode->addDrawable(text);
    }
    
    cursor.y() -= fontSizeCharacterSize;
    {
        osgText::Text* text = new osgText::Text;
        text->setFont(font);
        text->setColor(fontSizeColor);
        text->setCharacterSize(fontSizeCharacterSize);
        text->setPosition(cursor);
        
        // use text that uses 10 by 10 texels as a target resolution for fonts.
        text->setFontSize(20,20); // smoother but higher texture memory usage (but still quite low).
        
        text->setText("text->setFontSize(20,20); // smoother but higher texture memory usage (but still quite low).");
        geode->addDrawable(text);
    }
    
    cursor.y() -= fontSizeCharacterSize;
    {
        osgText::Text* text = new osgText::Text;
        text->setFont(font);
        text->setColor(fontSizeColor);
        text->setCharacterSize(fontSizeCharacterSize);
        text->setPosition(cursor);
        
        // use text that uses 10 by 10 texels as a target resolution for fonts.
        text->setFontSize(40,40); // even smoother but again higher texture memory usage.
        
        text->setText("text->setFontSize(40,40); // even smoother but again higher texture memory usage.");
        geode->addDrawable(text);
    }


    osg::Vec4 characterSizeColor(1.0f,0.0f,1.0f,1.0f);
    
    cursor.y() -= fontSizeCharacterSize*2.0f;
    
    {
        osgText::Text* text = new osgText::Text;
        text->setFont(font);
        text->setColor(characterSizeColor);
        text->setFontSize(20,20);
        text->setPosition(cursor);
        
        // use text that 20 units high.
        text->setCharacterSize(20); // small
        
        text->setText("text->setCharacterSize(15.0f); // small");
        geode->addDrawable(text);
    }
    
    cursor.y() -= 30.0f;
    {
        osgText::Text* text = new osgText::Text;
        text->setFont(font);
        text->setColor(characterSizeColor);
        text->setFontSize(30,30);
        text->setPosition(cursor);
        
        // use text that 20 units high.
        text->setCharacterSize(30.0f); // medium
        
        text->setText("text->setCharacterSize(30.0f); // medium");
        geode->addDrawable(text);
    }
    
    cursor.y() -= 50.0f;
    {
        osgText::Text* text = new osgText::Text;
        text->setFont(font);
        text->setColor(characterSizeColor);
        text->setFontSize(40,40);
        text->setPosition(cursor);
        
        // use text that uses 10 by 10 texels as a target resolution for fonts.
        text->setCharacterSize(60.0f); // large
        
        text->setText("text->setCharacterSize(60.0f); // large");
        geode->addDrawable(text);
    }


    cursor.x() = 500;
    cursor.y() = margin;

    osg::Sequence* sequence = new osg::Sequence;
   
    sequence->setMode(osg::Sequence::START);
    sequence->setInterval(osg::Sequence::LOOP, 0, -1);
    sequence->setDuration(1.0f, -1);
    
//    osg::Group* sequence = new osg::Group;
//    rootNode->addChild(sequence);
   
    {
        for(unsigned int i=osgText::Text::LEFT_TOP;i<=osgText::Text::BASE_LINE;i++)
        {
            osg::Geode* alignmentGeode = new osg::Geode;
            sequence->addChild(alignmentGeode);
            sequence->setTime(i, 1.0f);

            osgText::Text* text = new osgText::Text;
            text->setFont(font);
            text->setColor(characterSizeColor);
            text->setPosition(cursor);
            text->setDrawMode(osgText::Text::TEXT|osgText::Text::ALIGNMENT|osgText::Text::BOUNDINGBOX);
            
            text->setAlignment((osgText::Text::AlignmentType)i);
            
            text->setText("text->setAlignment();");
            alignmentGeode->addDrawable(text);

            cursor.y() += 40.0f;


        }
        
    }
    rootNode->addChild(sequence);
            
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


    osg::Vec3 screenCenter(300.0f,00.0f,00.0f);
    osgText::Text* text4 = new osgText::Text;
    text4->setFont("fonts/times.ttf");
    text4->setAxisAlignment(osgText::Text::SCREEN);
    text4->setPosition(screenCenter);
    text4->setText("SCREEN");
    geode->addDrawable(text4);

    osg::ShapeDrawable* shape = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(screenCenter),2.0f));
    shape->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::ON);
    geode->addDrawable(shape);


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
    
    osgDB::writeNodeFile(*rootNode,"test.osg");

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
