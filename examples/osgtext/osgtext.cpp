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



///////////////////////////////////////////////////////////////////////////////
// globals
#define        TEXT_POLYGON    "Polygon Font - jygq"
#define        TEXT_OUTLINE    "Outline Font - jygq"
#define        TEXT_TEXTURE    "Texture Font - jygq"
#define        TEXT_BITMAP        "Bitmap Font - jygq"
#define        TEXT_PIXMAP        "Pixmap Font - jygq"

#define        TEXT_COL_2D        osg::Vec4(.9,.9,.9,1)
#define        TEXT_COL_3D        osg::Vec4(.99,.3,.2,1)


std::string    ttfPath("fonts/times.ttf");
std::string    ttfPath1("fonts/arial.ttf");

int    gFontSize=18;
int    gFontSize1=24;
std::vector<osg::ref_ptr<osgText::Text > >    gTextList;
osgText::Text::AlignmentType    gAlignment=osgText::Text::LEFT_BOTTOM;

osg::Group* createText()
{
    osg::Group* rootNode = new osg::Group;

    osgText::Text*    text;
    osg::Geode*        geode;
    osg::Material*    textMaterial;
    osg::StateSet*  textState;
    double            xOffset=150;
    double            yOffset=gFontSize+10;

    ///////////////////////////////////////////////////////////////////////////
    // setup the texts

    ///////////////////////////////////////////////////////////////////////////
    // BitmapFont
    osgText::BitmapFont*    bitmapFont= new  osgText::BitmapFont(ttfPath,
                                                               gFontSize1);
    text= new  osgText::Text(bitmapFont);
    gTextList.push_back(text);
    text->setText(std::string("2d ")+std::string(TEXT_BITMAP));
    text->setPosition(osg::Vec3(xOffset,yOffset,0));
    text->setDrawMode( osgText::Text::TEXT |
                       osgText::Text::BOUNDINGBOX |
                       osgText::Text::ALIGNMENT );
    text->setAlignment(gAlignment);
    geode = new osg::Geode();
    geode->setName("BitmapFont");
    geode->addDrawable( text );

    textMaterial = new osg::Material();
    textMaterial->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE);
    textMaterial->setDiffuse( osg::Material::FRONT_AND_BACK, TEXT_COL_2D);
    textState = new osg::StateSet();
    textState->setAttribute(textMaterial );
    geode->setStateSet( textState );

    rootNode->addChild(geode);

    xOffset+=90;
    yOffset+=120;

    ///////////////////////////////////////////////////////////////////////////
    // PixmapFont
    osgText::PixmapFont*    pixmapFont= new  osgText::PixmapFont(ttfPath,
                                                               gFontSize1);
    text= new  osgText::Text(pixmapFont);
    gTextList.push_back(text);
    text->setText(std::string("2d ")+std::string(TEXT_PIXMAP));
    text->setPosition(osg::Vec3(xOffset,yOffset,0));
    text->setDrawMode( osgText::Text::TEXT |
                       osgText::Text::BOUNDINGBOX |
                       osgText::Text::ALIGNMENT );
    text->setAlignment(gAlignment);
    geode = new osg::Geode();
    geode->setName("PixmapFont");
    geode->addDrawable( text );

    textMaterial = new osg::Material();
    textMaterial->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE);
    textMaterial->setDiffuse( osg::Material::FRONT_AND_BACK,TEXT_COL_2D);
    // to get antiaA pixmapFonts we have to draw them with blending
    osg::BlendFunc    *transp= new  osg::BlendFunc();
    transp->setFunction(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);




    textState = new osg::StateSet();
    textState->setAttribute(textMaterial );
    textState->setAttribute(transp);
    textState->setMode(GL_BLEND,osg::StateAttribute::ON);
    textState->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    
    
    geode->setStateSet( textState );

    rootNode->addChild(geode);

    xOffset+=90;
    yOffset+=120;

    ///////////////////////////////////////////////////////////////////////////
    // TextureFont
    osgText::TextureFont*    textureFont= new  osgText::TextureFont(ttfPath1,
                                                                 gFontSize1);
    text= new  osgText::Text(textureFont);
    gTextList.push_back(text);
    text->setText(std::string("2d ")+std::string(TEXT_TEXTURE));
    text->setPosition(osg::Vec3(xOffset,yOffset,0));
    text->setDrawMode( osgText::Text::TEXT |
                       osgText::Text::BOUNDINGBOX |
                       osgText::Text::ALIGNMENT );
    text->setAlignment(gAlignment);
    geode = new osg::Geode();
    geode->setName("TextureFont");
    geode->addDrawable( text );

    textMaterial = new osg::Material();
    textMaterial->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE);
    textMaterial->setDiffuse( osg::Material::FRONT_AND_BACK, TEXT_COL_2D);
    // to get antiaA pixmapFonts we have to draw them with blending
    transp= new  osg::BlendFunc();
    transp->setFunction(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    textState = new osg::StateSet();
    textState->setAttribute(textMaterial );
    textState->setAttribute(transp);

    textState->setTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::ON);
    textState->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    geode->setStateSet( textState );

    rootNode->addChild(geode);

    xOffset+=90;
    yOffset+=120;

    ///////////////////////////////////////////////////////////////////////////
    // PolygonFont
    osgText::PolygonFont*    polygonFont= new  osgText::PolygonFont(ttfPath,
                                                                 gFontSize1,
                                                                 3);
    text= new  osgText::Text(polygonFont);
    gTextList.push_back(text);
    text->setText(std::string("2d ")+std::string("TEXT_POLYGON"));
    text->setPosition(osg::Vec3(xOffset,yOffset,0));
    text->setDrawMode( osgText::Text::TEXT |
                       osgText::Text::BOUNDINGBOX |
                       osgText::Text::ALIGNMENT );
    text->setAlignment(gAlignment);
    geode = new osg::Geode();
    geode->setName("PolygonFont");
    geode->addDrawable( text );

    textMaterial = new osg::Material();
    textMaterial->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE);
    textMaterial->setDiffuse( osg::Material::FRONT_AND_BACK, TEXT_COL_2D);
    textState = new osg::StateSet();
    textState->setAttribute(textMaterial );
    geode->setStateSet( textState );

    rootNode->addChild(geode);

    xOffset+=90;
    yOffset+=120;

    ///////////////////////////////////////////////////////////////////////////
    // OutlineFont
    osgText::OutlineFont*    outlineFont= new  osgText::OutlineFont(ttfPath,
                                                                 gFontSize1,
                                                                 3);

    text= new  osgText::Text(outlineFont);
    gTextList.push_back(text);
    text->setText(std::string("2d ")+std::string(TEXT_OUTLINE));
    text->setPosition(osg::Vec3(xOffset,yOffset,0));
    text->setDrawMode( osgText::Text::TEXT |
                       osgText::Text::BOUNDINGBOX |
                       osgText::Text::ALIGNMENT );
    text->setAlignment(gAlignment);
    geode = new osg::Geode();
    geode->setName("OutlineFont");
    geode->addDrawable( text );

    textMaterial = new osg::Material();
    textMaterial->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE);
    textMaterial->setDiffuse( osg::Material::FRONT_AND_BACK, TEXT_COL_2D);
    textState = new osg::StateSet();
    textState->setAttribute(textMaterial );
    geode->setStateSet( textState );

    rootNode->addChild(geode);
    
    
    // now add a depth attribute to the scene to force it to draw on top.
    osg::Depth* depth = new osg::Depth;
    depth->setRange(0.0,0.0);
    
    osg::StateSet* rootState = new osg::StateSet();
    rootState->setAttribute(depth);
    rootState->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    
    rootNode->setStateSet(rootState);

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

        // create the hud.
        osg::Projection* projection = new osg::Projection;
        projection->setMatrix(osg::Matrix::ortho2D(0,1024,0,768));

        osg::MatrixTransform* modelview_abs = new osg::MatrixTransform;
        modelview_abs->setReferenceFrame(osg::Transform::RELATIVE_TO_ABSOLUTE);
        modelview_abs->setMatrix(osg::Matrix::identity());

        modelview_abs->addChild(createText());

        projection->addChild(modelview_abs);

        group->addChild(projection);

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
