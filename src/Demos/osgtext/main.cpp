#include <osg/GL>
#include <osgGLUT/glut>
#include <osgGLUT/Viewer>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgUtil/Optimizer>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/Registry>

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


////////////////////////////////////////////////////////////////////////////////////////////////////////
//    
// Examples of how to set up different text layout
//

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
    
    
////////////////////////////////////////////////////////////////////////////////////////////////////////
//    
// Examples of how to set up different font resolution
//

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


////////////////////////////////////////////////////////////////////////////////////////////////////////
//    
// Examples of how to set up different sized text
//

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


////////////////////////////////////////////////////////////////////////////////////////////////////////
//    
// Examples of how to set up different alignments
//

    osg::Vec4 alignmentSizeColor(0.0f,1.0f,0.0f,1.0f);
    float alignmentCharacterSize = 25.0f;
    cursor.x() = 640;
    cursor.y() = margin*4.0f;
    
    typedef std::pair<osgText::Text::AlignmentType,std::string> AlignmentPair;
    typedef std::vector<AlignmentPair> AlignmentList;
    AlignmentList alignmentList;
    alignmentList.push_back(AlignmentPair(osgText::Text::LEFT_TOP,"text->setAlignment(osgText::Text::LEFT_TOP);"));
    alignmentList.push_back(AlignmentPair(osgText::Text::LEFT_CENTER,"text->setAlignment(osgText::Text::LEFT_CENTER);"));
    alignmentList.push_back(AlignmentPair(osgText::Text::LEFT_BOTTOM,"text->setAlignment(osgText::Text::LEFT_BOTTOM);"));
    alignmentList.push_back(AlignmentPair(osgText::Text::CENTER_TOP,"text->setAlignment(osgText::Text::CENTER_TOP);"));
    alignmentList.push_back(AlignmentPair(osgText::Text::CENTER_CENTER,"text->setAlignment(osgText::Text::CENTER_CENTER);"));
    alignmentList.push_back(AlignmentPair(osgText::Text::CENTER_BOTTOM,"text->setAlignment(osgText::Text::CENTER_BOTTOM);"));
    alignmentList.push_back(AlignmentPair(osgText::Text::RIGHT_TOP,"text->setAlignment(osgText::Text::RIGHT_TOP);"));
    alignmentList.push_back(AlignmentPair(osgText::Text::RIGHT_CENTER,"text->setAlignment(osgText::Text::RIGHT_CENTER);"));
    alignmentList.push_back(AlignmentPair(osgText::Text::RIGHT_BOTTOM,"text->setAlignment(osgText::Text::RIGHT_BOTTOM);"));
    alignmentList.push_back(AlignmentPair(osgText::Text::BASE_LINE,"text->setAlignment(osgText::Text::BASE_LINE);//default"));

    osg::Sequence* sequence = new osg::Sequence;
    {
        for(AlignmentList::iterator itr=alignmentList.begin();
            itr!=alignmentList.end();
            ++itr)
        {
            osg::Geode* alignmentGeode = new osg::Geode;
            sequence->addChild(alignmentGeode);
            sequence->setTime(sequence->getNumChildren(), 1.0f);

            osgText::Text* text = new osgText::Text;
            text->setFont(font);
            text->setColor(alignmentSizeColor);
            text->setCharacterSize(alignmentCharacterSize);
            text->setPosition(cursor);
            text->setDrawMode(osgText::Text::TEXT|osgText::Text::ALIGNMENT|osgText::Text::BOUNDINGBOX);
            
            text->setAlignment(itr->first);
            text->setText(itr->second);
            
            alignmentGeode->addDrawable(text);


        }
        
    }

    sequence->setMode(osg::Sequence::START);
    sequence->setInterval(osg::Sequence::LOOP, 0, -1);
    sequence->setDuration(1.0f, -1);
    
    rootNode->addChild(sequence);


////////////////////////////////////////////////////////////////////////////////////////////////////////
//    
// Examples of how to set up different fonts...
//

    cursor.x() = margin*2.0f;
    cursor.y() = margin*2.0f;
    
    osg::Vec4 fontColor(1.0f,0.5f,0.0f,1.0f);
    float fontCharacterSize = 20.0f;
    float spacing = 40.0f;
    
    {
        osgText::Text* text = new osgText::Text;
        text->setColor(fontColor);
        text->setPosition(cursor);
        text->setCharacterSize(fontCharacterSize);
        
        text->setFont(0);
        text->setText("text->setFont(0); // inbuilt font.");
        geode->addDrawable(text);

        cursor.x() = text->getBound().xMax() + spacing ;
    }
    
    {
        osgText::Font* arial = osgText::readFontFile("fonts/arial.ttf");

        osgText::Text* text = new osgText::Text;
        text->setColor(fontColor);
        text->setPosition(cursor);
        text->setCharacterSize(fontCharacterSize);
        
        text->setFont(arial);
        text->setText(arial!=0?
                      "text->setFont(\"fonts/arial.ttf\");":
                      "unable to load \"fonts/arial.ttf\"");
        geode->addDrawable(text);

        cursor.x() = text->getBound().xMax() + spacing ;
    }
    
    {
        osgText::Font* times = osgText::readFontFile("fonts/times.ttf");

        osgText::Text* text = new osgText::Text;
        text->setColor(fontColor);
        text->setPosition(cursor);
        text->setCharacterSize(fontCharacterSize);
        
        geode->addDrawable(text);
        text->setFont(times);
        text->setText(times!=0?
                      "text->setFont(\"fonts/times.ttf\");":
                      "unable to load \"fonts/times.ttf\"");

        cursor.x() = text->getBound().xMax() + spacing ;
    }
    
    cursor.x() = margin*2.0f;
    cursor.y() = margin;

    {
        osgText::Font* dirtydoz = osgText::readFontFile("fonts/dirtydoz.ttf");

        osgText::Text* text = new osgText::Text;
        text->setColor(fontColor);
        text->setPosition(cursor);
        text->setCharacterSize(fontCharacterSize);
        
        text->setFont(dirtydoz);
        text->setText(dirtydoz!=0?
                      "text->setFont(\"fonts/dirtydoz.ttf\");":
                      "unable to load \"fonts/dirtydoz.ttf\"");
        geode->addDrawable(text);

        cursor.x() = text->getBound().xMax() + spacing ;
    }
    
    {
        osgText::Font* fudd = osgText::readFontFile("fonts/fudd.ttf");
    
        osgText::Text* text = new osgText::Text;
        text->setColor(fontColor);
        text->setPosition(cursor);
        text->setCharacterSize(fontCharacterSize);
        
        text->setFont(fudd);
        text->setText(fudd!=0?
                      "text->setFont(\"fonts/fudd.ttf\");":
                      "unable to load \"fonts/fudd.ttf\"");
        geode->addDrawable(text);

        cursor.x() = text->getBound().xMax() + spacing ;
    }
            
    return rootNode;    
}




// create text which sits in 3D space such as would be inserted into a normal model
osg::Group* create3DText(const osg::Vec3& center,float radius)
{

    osg::Geode* geode  = new osg::Geode;

////////////////////////////////////////////////////////////////////////////////////////////////////////
//    
// Examples of how to set up axis/orientation alignments
//

    float characterSize=radius*0.2f;
    
    osg::Vec3 pos(center.x()-radius*.5f,center.y()-radius*.5f,center.z()-radius*.5f);

    osgText::Text* text1 = new osgText::Text;
    text1->setFont("fonts/times.ttf");
    text1->setCharacterSize(characterSize);
    text1->setPosition(pos);
    text1->setAxisAlignment(osgText::Text::XY_PLANE);
    text1->setText("XY_PLANE");
    geode->addDrawable(text1);

    osgText::Text* text2 = new osgText::Text;
    text2->setFont("fonts/times.ttf");
    text2->setCharacterSize(characterSize);
    text2->setPosition(pos);
    text2->setAxisAlignment(osgText::Text::YZ_PLANE);
    text2->setText("YZ_PLANE");
    geode->addDrawable(text2);

    osgText::Text* text3 = new osgText::Text;
    text3->setFont("fonts/times.ttf");
    text3->setCharacterSize(characterSize);
    text3->setPosition(pos);
    text3->setAxisAlignment(osgText::Text::XZ_PLANE);
    text3->setText("XZ_PLANE");
    geode->addDrawable(text3);


    osgText::Text* text4 = new osgText::Text;
    text4->setFont("fonts/times.ttf");
    text4->setCharacterSize(characterSize);
    text4->setPosition(center);
    text4->setAxisAlignment(osgText::Text::SCREEN);
    text4->setText("SCREEN");
    geode->addDrawable(text4);

    osg::ShapeDrawable* shape = new osg::ShapeDrawable(new osg::Sphere(center,characterSize*0.2f));
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
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getProgramName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
   
    // initialize the viewer.
    osgGLUT::Viewer viewer(arguments);

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

    // load the nodes from the commandline arguments.
    osg::ref_ptr<osg::Node> rootNode = osgDB::readNodeFiles(arguments);
        
    // prepare scene.
    {

        osg::Vec3 center(0.0f,0.0f,0.0f);
        float radius = 1.0f;
        
        if (rootNode.valid())
        {
            // optimize the scene graph, remove rendundent nodes and state etc.
            osgUtil::Optimizer optimizer;
            optimizer.optimize(rootNode.get());
            
            const osg::BoundingSphere& bs = rootNode->getBound();
            center = bs.center();
            radius = bs.radius();
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

        group->addChild(create3DText(center,radius));

    }
     
    // add a viewport to the viewer and attach the scene graph.
    viewer.addViewport( rootNode.get() );
    
    // register trackball, flight and drive.
    viewer.registerCameraManipulator(new osgGA::TrackballManipulator);
    viewer.registerCameraManipulator(new osgGA::FlightManipulator);
    viewer.registerCameraManipulator(new osgGA::DriveManipulator);

    // open the viewer window.
    viewer.open();
    
    // fire up the event loop.
    viewer.run();

    return 0;
}
