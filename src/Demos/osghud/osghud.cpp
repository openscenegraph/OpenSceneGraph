#include <osg/GL>
#include <osgGLUT/glut>
#include <osgGLUT/Viewer>

#include <osg/Transform>
#include <osg/Projection>
#include <osg/Billboard>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>
#include <osg/Material>
#include <osg/Transparency>
#include <osg/Depth>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgUtil/TrackballManipulator>
#include <osgUtil/FlightManipulator>
#include <osgUtil/DriveManipulator>

#include <osgText/Text>


#include <osgUtil/Optimizer>

void write_usage(std::ostream& out,const std::string& name)
{
    out << std::endl;
    out <<"usage:"<< std::endl;
    out <<"    "<<name<<" [options] infile1 [infile2 ...]"<< std::endl;
    out << std::endl;
    out <<"options:"<< std::endl;
    out <<"    -l libraryName      - load plugin of name libraryName"<< std::endl;
    out <<"                          i.e. -l osgdb_pfb"<< std::endl;
    out <<"                          Useful for loading reader/writers which can load"<< std::endl;
    out <<"                          other file formats in addition to its extension."<< std::endl;
    out <<"    -e extensionName    - load reader/wrter plugin for file extension"<< std::endl;
    out <<"                          i.e. -e pfb"<< std::endl;
    out <<"                          Useful short hand for specifying full library name as"<< std::endl;
    out <<"                          done with -l above, as it automatically expands to"<< std::endl;
    out <<"                          the full library name appropriate for each platform."<< std::endl;
    out <<std::endl;
    out <<"    -stereo             - switch on stereo rendering, using the default of,"<< std::endl;
    out <<"                          ANAGLYPHIC or the value set in the OSG_STEREO_MODE "<< std::endl;
    out <<"                          environmental variable. See doc/stereo.html for "<< std::endl;
    out <<"                          further details on setting up accurate stereo "<< std::endl;
    out <<"                          for your system. "<< std::endl;
    out <<"    -stereo ANAGLYPHIC  - switch on anaglyphic(red/cyan) stereo rendering."<< std::endl;
    out <<"    -stereo QUAD_BUFFER - switch on quad buffered stereo rendering."<< std::endl;
    out <<std::endl;
    out <<"    -stencil            - use a visual with stencil buffer enabled, this "<< std::endl;
    out <<"                          also allows the depth complexity statistics mode"<< std::endl;
    out <<"                          to be used (press 'p' three times to cycle to it)."<< std::endl;
    out << std::endl;
}


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

void set2dScene(osg::Group* rootNode)
{
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
    osgText::BitmapFont*    bitmapFont= osgNew  osgText::BitmapFont(ttfPath,
                                                               gFontSize1);
    text= osgNew  osgText::Text(bitmapFont);
    gTextList.push_back(text);
    text->setText(std::string("2d ")+std::string(TEXT_BITMAP));
    text->setPosition(osg::Vec3(xOffset,yOffset,0));
    text->setDrawMode( osgText::Text::TEXT |
                       osgText::Text::BOUNDINGBOX |
                       osgText::Text::ALIGNEMENT );
    text->setAlignment(gAlignment);
    geode = osgNew osg::Geode();
    geode->setName("BitmapFont");
    geode->addDrawable( text );

    textMaterial = osgNew osg::Material();
    textMaterial->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE);
    textMaterial->setDiffuse( osg::Material::FRONT_AND_BACK, TEXT_COL_2D);
    textState = osgNew osg::StateSet();
    textState->setAttribute(textMaterial );
    geode->setStateSet( textState );

    rootNode->addChild(geode);

    xOffset+=90;
    yOffset+=120;

    ///////////////////////////////////////////////////////////////////////////
    // PixmapFont
    osgText::PixmapFont*    pixmapFont= osgNew  osgText::PixmapFont(ttfPath,
                                                               gFontSize1);
    text= osgNew  osgText::Text(pixmapFont);
    gTextList.push_back(text);
    text->setText(std::string("2d ")+std::string(TEXT_PIXMAP));
    text->setPosition(osg::Vec3(xOffset,yOffset,0));
    text->setDrawMode( osgText::Text::TEXT |
                       osgText::Text::BOUNDINGBOX |
                       osgText::Text::ALIGNEMENT );
    text->setAlignment(gAlignment);
    geode = osgNew osg::Geode();
    geode->setName("PixmapFont");
    geode->addDrawable( text );

    textMaterial = osgNew osg::Material();
    textMaterial->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE);
    textMaterial->setDiffuse( osg::Material::FRONT_AND_BACK,TEXT_COL_2D);
    // to get antiaA pixmapFonts we have to draw them with blending
    osg::Transparency    *transp= osgNew  osg::Transparency();
    transp->setFunction(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);




    textState = osgNew osg::StateSet();
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
    osgText::TextureFont*    textureFont= osgNew  osgText::TextureFont(ttfPath1,
                                                                 gFontSize1);
    text= osgNew  osgText::Text(textureFont);
    gTextList.push_back(text);
    text->setText(std::string("2d ")+std::string(TEXT_TEXTURE));
    text->setPosition(osg::Vec3(xOffset,yOffset,0));
    text->setDrawMode( osgText::Text::TEXT |
                       osgText::Text::BOUNDINGBOX |
                       osgText::Text::ALIGNEMENT );
    text->setAlignment(gAlignment);
    geode = osgNew osg::Geode();
    geode->setName("TextureFont");
    geode->addDrawable( text );

    textMaterial = osgNew osg::Material();
    textMaterial->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE);
    textMaterial->setDiffuse( osg::Material::FRONT_AND_BACK, TEXT_COL_2D);
    // to get antiaA pixmapFonts we have to draw them with blending
    transp= osgNew  osg::Transparency();
    transp->setFunction(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    textState = osgNew osg::StateSet();
    textState->setAttribute(textMaterial );
    textState->setAttribute(transp);

    textState->setMode(GL_TEXTURE_2D,osg::StateAttribute::ON);
    textState->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    geode->setStateSet( textState );

    rootNode->addChild(geode);

    xOffset+=90;
    yOffset+=120;

    ///////////////////////////////////////////////////////////////////////////
    // PolygonFont
    osgText::PolygonFont*    polygonFont= osgNew  osgText::PolygonFont(ttfPath,
                                                                 gFontSize1,
                                                                 3);
    text= osgNew  osgText::Text(polygonFont);
    gTextList.push_back(text);
    text->setText(std::string("2d ")+std::string("TEXT_POLYGON"));
    text->setPosition(osg::Vec3(xOffset,yOffset,0));
    text->setDrawMode( osgText::Text::TEXT |
                       osgText::Text::BOUNDINGBOX |
                       osgText::Text::ALIGNEMENT );
    text->setAlignment(gAlignment);
    geode = osgNew osg::Geode();
    geode->setName("PolygonFont");
    geode->addDrawable( text );

    textMaterial = osgNew osg::Material();
    textMaterial->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE);
    textMaterial->setDiffuse( osg::Material::FRONT_AND_BACK, TEXT_COL_2D);
    textState = osgNew osg::StateSet();
    textState->setAttribute(textMaterial );
    geode->setStateSet( textState );

    rootNode->addChild(geode);

    xOffset+=90;
    yOffset+=120;

    ///////////////////////////////////////////////////////////////////////////
    // OutlineFont
    osgText::OutlineFont*    outlineFont= osgNew  osgText::OutlineFont(ttfPath,
                                                                 gFontSize1,
                                                                 3);

    text= osgNew  osgText::Text(outlineFont);
    gTextList.push_back(text);
    text->setText(std::string("2d ")+std::string(TEXT_OUTLINE));
    text->setPosition(osg::Vec3(xOffset,yOffset,0));
    text->setDrawMode( osgText::Text::TEXT |
                       osgText::Text::BOUNDINGBOX |
                       osgText::Text::ALIGNEMENT );
    text->setAlignment(gAlignment);
    geode = osgNew osg::Geode();
    geode->setName("OutlineFont");
    geode->addDrawable( text );

    textMaterial = osgNew osg::Material();
    textMaterial->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE);
    textMaterial->setDiffuse( osg::Material::FRONT_AND_BACK, TEXT_COL_2D);
    textState = osgNew osg::StateSet();
    textState->setAttribute(textMaterial );
    geode->setStateSet( textState );

    rootNode->addChild(geode);
    
    
    // now add a depth attribute to the scene to force it to draw on top.
    osg::Depth* depth = osgNew osg::Depth;
    depth->setRange(0.0,0.0);
    
    osg::StateSet* rootState = osgNew osg::StateSet();
    rootState->setAttribute(depth);
    rootState->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    
    rootNode->setStateSet(rootState);
    
}






int main( int argc, char **argv )
{

    // initialize the GLUT
    glutInit( &argc, argv );

    if (argc<2)
    {
        write_usage(std::cout,argv[0]);
        return 0;
    }
    
    // create the commandline args.
    std::vector<std::string> commandLine;
    for(int i=1;i<argc;++i) commandLine.push_back(argv[i]);
    

    // initialize the viewer.
    osgGLUT::Viewer viewer;
    viewer.setWindowTitle(argv[0]);

    // configure the viewer from the commandline arguments, and eat any
    // parameters that have been matched.
    viewer.readCommandLine(commandLine);
    
    // configure the plugin registry from the commandline arguments, and 
    // eat any parameters that have been matched.
    osgDB::readCommandLine(commandLine);

    // load the nodes from the commandline arguments.
    osg::Node* rootnode = osgDB::readNodeFiles(commandLine);
    if (!rootnode)
    {
//        write_usage(osg::notify(osg::NOTICE),argv[0]);
        return 1;
    }
    
    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
    
    // make sure the root node is group so we can add extra nodes to it.
    osg::Group* group = dynamic_cast<osg::Group*>(rootnode);
    if (!group)
    {
        group = osgNew osg::Group;
        group->addChild(rootnode);
        rootnode = group;
    }
    
    // create the hud.
    osg::Projection* projection = osgNew osg::Projection;
    projection->setMatrix(osg::Matrix::ortho2D(0,1024,0,768));
    
    osg::Transform* modelview_abs = osgNew osg::Transform;
    modelview_abs->setReferenceFrame(osg::Transform::RELATIVE_TO_ABSOLUTE);
    modelview_abs->setMatrix(osg::Matrix::identity());
    
    
    set2dScene(modelview_abs);

    projection->addChild(modelview_abs);

    group->addChild(projection);
    
     
    // add a viewport to the viewer and attach the scene graph.
    viewer.addViewport( rootnode );
    
    // register trackball, flight and drive.
    viewer.registerCameraManipulator(new osgUtil::TrackballManipulator);
    viewer.registerCameraManipulator(new osgUtil::FlightManipulator);
    viewer.registerCameraManipulator(new osgUtil::DriveManipulator);

    // open the viewer window.
    viewer.open();
    
    // fire up the event loop.
    viewer.run();

    return 0;
}
