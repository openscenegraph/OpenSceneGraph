/* --------------------------------------------------------------------------
 *
 *    osgText demo
 *
 * --------------------------------------------------------------------------
 *    
 *    prog:    max rheiner;mrn@paus.ch
 *    date:    4/26/2001    (m/d/y)
 *
 * --------------------------------------------------------------------------
 */

#include <osg/Node>
#include <osg/StateSet>
#include <osg/GeoSet>
#include <osg/Material>
#include <osg/Transparency>
#include <osg/Transform>
#include <osg/PolygonMode>
#include <osg/Depth>
#include <osg/Notify>

#include <osgUtil/TrackballManipulator>
#include <osgUtil/FlightManipulator>
#include <osgUtil/DriveManipulator>

#include <osgGLUT/Viewer>
#include <GL/glut.h>

#include <osgText/Text>
#include <vector>


///////////////////////////////////////////////////////////////////////////////
// globals
#define        TEXT_POLYGON    "Polygon Font - jygq"
#define        TEXT_OUTLINE    "Outline Font - jygq"
#define        TEXT_TEXTURE    "Texture Font - jygq"
#define        TEXT_BITMAP        "Bitmap Font - jygq"
#define        TEXT_PIXMAP        "Pixmap Font - jygq"

#define        TEXT_COL_2D        osg::Vec4(.9,.9,.9,1)
#define        TEXT_COL_3D        osg::Vec4(.99,.3,.2,1)


#ifdef WIN32
std::string    ttfPath("./fonts/times.ttf");
std::string    ttfPath1("./fonts/verdana.ttf");

#else

std::string    ttfPath("dirtydoz.ttf");
std::string    ttfPath1("fudd.ttf");

#endif

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
    double            xOffset=250;
    double            yOffset=gFontSize+10;

    ///////////////////////////////////////////////////////////////////////////
    // setup the texts

    ///////////////////////////////////////////////////////////////////////////
    // BitmapFont
    osgText::BitmapFont*    bitmapFont=new osgText::BitmapFont(ttfPath,
                                                               gFontSize1);
    text=new osgText::Text(bitmapFont);
    gTextList.push_back(text);
    text->setText(std::string("2d ")+std::string(TEXT_BITMAP));
    text->setPosition(osg::Vec3(xOffset,yOffset,0));
    text->setDrawMode( osgText::Text::TEXT |
                       osgText::Text::BOUNDINGBOX |
                       osgText::Text::ALIGNEMENT );
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
    osgText::PixmapFont*    pixmapFont=new osgText::PixmapFont(ttfPath,
                                                               gFontSize1);
    text=new osgText::Text(pixmapFont);
    gTextList.push_back(text);
    text->setText(std::string("2d ")+std::string(TEXT_PIXMAP));
    text->setPosition(osg::Vec3(xOffset,yOffset,0));
    text->setDrawMode( osgText::Text::TEXT |
                       osgText::Text::BOUNDINGBOX |
                       osgText::Text::ALIGNEMENT );
    text->setAlignment(gAlignment);
    geode = new osg::Geode();
    geode->setName("PixmapFont");
    geode->addDrawable( text );

    textMaterial = new osg::Material();
    textMaterial->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE);
    textMaterial->setDiffuse( osg::Material::FRONT_AND_BACK,TEXT_COL_2D);
    // to get antiaA pixmapFonts we have to draw them with blending
    osg::Transparency    *transp=new osg::Transparency();
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
    osgText::TextureFont*    textureFont=new osgText::TextureFont(ttfPath1,
                                                                 gFontSize1);
    text=new osgText::Text(textureFont);
    gTextList.push_back(text);
    text->setText(std::string("2d ")+std::string(TEXT_TEXTURE));
    text->setPosition(osg::Vec3(xOffset,yOffset,0));
    text->setDrawMode( osgText::Text::TEXT |
                       osgText::Text::BOUNDINGBOX |
                       osgText::Text::ALIGNEMENT );
    text->setAlignment(gAlignment);
    geode = new osg::Geode();
    geode->setName("TextureFont");
    geode->addDrawable( text );

    textMaterial = new osg::Material();
    textMaterial->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE);
    textMaterial->setDiffuse( osg::Material::FRONT_AND_BACK, TEXT_COL_2D);
    // to get antiaA pixmapFonts we have to draw them with blending
    transp=new osg::Transparency();
    transp->setFunction(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    textState = new osg::StateSet();
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
    osgText::PolygonFont*    polygonFont=new osgText::PolygonFont(ttfPath,
                                                                 gFontSize1,
                                                                 3);
    text=new osgText::Text(polygonFont);
    gTextList.push_back(text);
    text->setText(std::string("2d ")+std::string("TEXT_POLYGON"));
    text->setPosition(osg::Vec3(xOffset,yOffset,0));
    text->setDrawMode( osgText::Text::TEXT |
                       osgText::Text::BOUNDINGBOX |
                       osgText::Text::ALIGNEMENT );
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
    osgText::OutlineFont*    outlineFont=new osgText::OutlineFont(ttfPath,
                                                                 gFontSize1,
                                                                 3);

    text=new osgText::Text(outlineFont);
    gTextList.push_back(text);
    text->setText(std::string("2d ")+std::string(TEXT_OUTLINE));
    text->setPosition(osg::Vec3(xOffset,yOffset,0));
    text->setDrawMode( osgText::Text::TEXT |
                       osgText::Text::BOUNDINGBOX |
                       osgText::Text::ALIGNEMENT );
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
    
    rootNode->setStateSet(rootState);
    
}

void setScene(osg::Group* rootNode)
{
    osgText::Text*    text;
    osg::Geode*        geode;
    osg::Material*    textMaterial;
    osg::StateSet*  textState;
    double            xOffset=0;
    double            yOffset=0;

    ///////////////////////////////////////////////////////////////////////////
    // setup the texts
    ///////////////////////////////////////////////////////////////////////////
    // BitmapFont
    osgText::BitmapFont*    bitmapFont=new osgText::BitmapFont(ttfPath,
                                                               gFontSize);
    text=new osgText::Text(bitmapFont);
    gTextList.push_back(text);
    text->setText(std::string(TEXT_BITMAP));
    text->setPosition(osg::Vec3(xOffset,yOffset,0));
    text->setDrawMode( osgText::Text::TEXT |
                       osgText::Text::BOUNDINGBOX |
                       osgText::Text::ALIGNEMENT );
    text->setAlignment(gAlignment);
    geode = new osg::Geode();
    geode->setName("BitmapFont");
    geode->addDrawable( text );

    textMaterial = new osg::Material();
    textMaterial->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE);
    textMaterial->setDiffuse( osg::Material::FRONT_AND_BACK,TEXT_COL_3D);
    textState = new osg::StateSet();
    textState->setAttribute(textMaterial );
    geode->setStateSet( textState );

    rootNode->addChild(geode);

    yOffset+=gFontSize+5;

    ///////////////////////////////////////////////////////////////////////////
    // PixmapFont
    osgText::PixmapFont*    pixmapFont=new osgText::PixmapFont(ttfPath,
                                                               gFontSize);
    text=new osgText::Text(pixmapFont);
    gTextList.push_back(text);
    text->setText(std::string(TEXT_PIXMAP));
    text->setPosition(osg::Vec3(xOffset,yOffset,0));
    text->setDrawMode( osgText::Text::TEXT |
                       osgText::Text::BOUNDINGBOX |
                       osgText::Text::ALIGNEMENT );
    text->setAlignment(gAlignment);
    geode = new osg::Geode();
    geode->setName("PixmapFont");
    geode->addDrawable( text );

    textMaterial = new osg::Material();
    textMaterial->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE);
    textMaterial->setDiffuse( osg::Material::FRONT_AND_BACK,TEXT_COL_3D);
    // to get antiaA pixmapFonts we have to draw them with blending
    osg::Transparency    *transp=new osg::Transparency();
    transp->setFunction(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    textState = new osg::StateSet();
    textState->setAttribute(textMaterial );
    textState->setAttribute(transp);
    textState->setMode(GL_BLEND,osg::StateAttribute::ON);
    textState->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    geode->setStateSet( textState );

    rootNode->addChild(geode);

    yOffset+=gFontSize+5;

    ///////////////////////////////////////////////////////////////////////////
    // TextureFont
    osgText::TextureFont*    textureFont=new osgText::TextureFont(ttfPath,
                                                               gFontSize);
    text=new osgText::Text(textureFont);
    gTextList.push_back(text);
    text->setText(std::string(TEXT_TEXTURE));
    text->setPosition(osg::Vec3(xOffset,yOffset,0));
    text->setDrawMode( osgText::Text::TEXT |
                       osgText::Text::BOUNDINGBOX |
                       osgText::Text::ALIGNEMENT );
    text->setAlignment(gAlignment);
    geode = new osg::Geode();
    geode->setName("TextureFont");
    geode->addDrawable( text );

    textMaterial = new osg::Material();
    textMaterial->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE);
    textMaterial->setDiffuse( osg::Material::FRONT_AND_BACK,TEXT_COL_3D);
    // to get antiaA pixmapFonts we have to draw them with blending
    transp=new osg::Transparency();
    transp->setFunction(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    textState = new osg::StateSet();
    textState->setAttribute(textMaterial );
    textState->setAttribute(transp);

    //    textState->setMode(GL_BLEND,osg::StateAttribute::ON);
    textState->setMode(GL_TEXTURE_2D,osg::StateAttribute::ON);
    textState->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    geode->setStateSet( textState );

    rootNode->addChild(geode);

    yOffset+=gFontSize+5;

    ///////////////////////////////////////////////////////////////////////////
    // PolygonFont
    osgText::PolygonFont*    polygonFont=new osgText::PolygonFont(ttfPath,
                                                                 gFontSize,
                                                                 3);
    text=new osgText::Text(polygonFont);
    gTextList.push_back(text);
    text->setText(std::string(TEXT_POLYGON));
    text->setPosition(osg::Vec3(xOffset,yOffset,0));
    text->setDrawMode( osgText::Text::TEXT |
                       osgText::Text::BOUNDINGBOX |
                       osgText::Text::ALIGNEMENT );
    text->setAlignment(gAlignment);
    geode = new osg::Geode();
    geode->setName("PolygonFont");
    geode->addDrawable( text );

    textMaterial = new osg::Material();
    textMaterial->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE);
    textMaterial->setDiffuse( osg::Material::FRONT_AND_BACK,TEXT_COL_3D);
    textState = new osg::StateSet();
    textState->setAttribute(textMaterial );
    geode->setStateSet( textState );

    rootNode->addChild(geode);

    yOffset+=gFontSize+5;

    ///////////////////////////////////////////////////////////////////////////
    // OutlineFont
    osgText::OutlineFont*    outlineFont=new osgText::OutlineFont(ttfPath,
                                                                 gFontSize,
                                                                 3);

    text=new osgText::Text(outlineFont);
    gTextList.push_back(text);
    text->setText(std::string(TEXT_OUTLINE));
    text->setPosition(osg::Vec3(xOffset,yOffset,0));
    text->setDrawMode( osgText::Text::TEXT |
                       osgText::Text::BOUNDINGBOX |
                       osgText::Text::ALIGNEMENT );
    text->setAlignment(gAlignment);
    geode = new osg::Geode();
    geode->setName("OutlineFont");
    geode->addDrawable( text );

    textMaterial = new osg::Material();
    textMaterial->setColorMode( osg::Material::AMBIENT_AND_DIFFUSE);
    textMaterial->setDiffuse( osg::Material::FRONT_AND_BACK,TEXT_COL_3D);
    textState = new osg::StateSet();
    textState->setAttribute(textMaterial );
    geode->setStateSet( textState );

    rootNode->addChild(geode);

}

class TextViewer: public osgGLUT::Viewer
{
public:

    virtual float app(unsigned int viewport)
    {
        float ret;
        ret=Viewer::app(viewport);
        if(_hudSceneView.valid() && viewport>=_viewportList.size()-1)
        {
            _hudSceneView->app();
        }
        return ret;
    }
    
    virtual float cull(unsigned int viewport)
    {
        float ret;
        ret=Viewer::cull(viewport);
        if(_hudSceneView.valid() && viewport>=_viewportList.size()-1)
            _hudSceneView->cull();
        return ret;
    }

    virtual float draw(unsigned int viewport)
    {
        float ret;
        ret=Viewer::draw(viewport);
        if(_hudSceneView.valid() && viewport>=_viewportList.size()-1)
            _hudSceneView->draw();
        return ret;
    }

    void addHUD(osg::Node* rootnode) 
    {
        _hudSceneView = new osgUtil::SceneView;
        _hudSceneView->setDefaults();
        _hudSceneView->setSceneData(rootnode);

    }
    
    virtual void reshape(GLint w, GLint h)
    {
        Viewer::reshape(w,h);
        
        if(_hudSceneView.valid())
        {
            _hudSceneView->setViewport(0,0,w,h);
            _hudCam->setOrtho2D(0,w,0,h);
        }
    }

    virtual bool open()
    {
        bool ret=Viewer::open();

        // set the clear flag / after the visualReq.Visitor
        if(_hudSceneView.valid())
        {
            _hudSceneView->getRenderStage()->setClearMask(0);
            _hudSceneView->getCullVisitor()->setCullingMode(osgUtil::CullViewState::NO_CULLING);
            _hudSceneView->setCalcNearFar(false);
            
            _hudCam=new osg::Camera;

            // leftBottom 
            _hudSceneView->setCamera(_hudCam.get());
        }

        return ret;
    }


protected:

    osg::ref_ptr<osg::Camera>            _hudCam;
    osg::ref_ptr<osgUtil::SceneView>    _hudSceneView;

    virtual void keyboard(unsigned char key, int x, int y)
    {
        switch(key)
        {
            case '1':
                {    // change DrawMode
                    std::vector<osg::ref_ptr<osgText::Text> >::iterator itr=gTextList.begin();
                    for(;itr!=gTextList.end();itr++)
                        (*itr)->setDrawMode(osgText::Text::TEXT ^ (*itr)->getDrawMode());
                }
                return;
            case '2':
                {    // change DrawMode
                    std::vector<osg::ref_ptr<osgText::Text> >::iterator itr=gTextList.begin();
                    for(;itr!=gTextList.end();itr++)
                        (*itr)->setDrawMode(osgText::Text::BOUNDINGBOX ^ (*itr)->getDrawMode());
                }
                return;
            case '3':
                {    // change DrawMode
                    std::vector<osg::ref_ptr<osgText::Text> >::iterator itr=gTextList.begin();
                    for(;itr!=gTextList.end();itr++)
                        (*itr)->setDrawMode(osgText::Text::ALIGNEMENT ^ (*itr)->getDrawMode());
                }
                return;
            ///////////////////////////////////////////////////////////////////
            case '4':
                {    // change BoundingBoxType to GEOMETRY
                    std::vector<osg::ref_ptr<osgText::Text> >::iterator itr=gTextList.begin();
                    osgText::Text::BoundingBoxType type=(*itr)->getBoundingBox()==osgText::Text::GLYPH ? 
                             osgText::Text::GEOMETRY :
                             osgText::Text::GLYPH;
                    for(;itr!=gTextList.end();itr++)
                        (*itr)->setBoundingBox(type);
                }
                return;
            ///////////////////////////////////////////////////////////////////
            case '5':
                {    // change the textAlignment
                    gAlignment=(osgText::Text::AlignmentType)((int)gAlignment+1);
                    if(gAlignment>osgText::Text::RIGHT_BOTTOM)
                        gAlignment=osgText::Text::LEFT_TOP;

                    std::vector<osg::ref_ptr<osgText::Text> >::iterator itr=gTextList.begin();
                    for(;itr!=gTextList.end();itr++)
                        (*itr)->setAlignment(gAlignment);
                }
                return;
            default:
                Viewer::keyboard(key,x,y);
        };
    }

};

int main( int argc, char **argv )
{
    // initialize the GLUT
    glutInit( &argc, argv );

    // get the fontName
    if(argc > 1)
        ttfPath=argv[1];
    if(argc > 2)
        ttfPath1=argv[2];
    if(argc > 3)
    {
        gFontSize=atoi(argv[3]);
        if(gFontSize<=4)
            gFontSize=8;    
    }
    if(argc > 4)
    {
        gFontSize1=atoi(argv[3]);
        if(gFontSize1<=4)
            gFontSize1=8;    
    }

    osg::Group*            rootNode = new osg::Group;
    osg::Group*            scene2d = new osg::Group;
    osg::Transform*        textGroup = new osg::Transform;

    // set the name for the hole group
    rootNode->setName("sceneGroup");

    // turn off the culling
    // turn off the light
    osg::StateSet*  gstate = new osg::StateSet;
    gstate->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
    gstate->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    gstate->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    rootNode->setStateSet(gstate);
    scene2d->setStateSet(gstate);

    // setup the sceneData
    setScene(textGroup);
    textGroup->preRotate(osg::inDegrees(90),1,0,0);
    rootNode->addChild(textGroup);

    // setup the 2dNode
    set2dScene(scene2d);

    // initialize the viewer.
    TextViewer    viewer;
/*
    viewer.addViewport( rootNode );
    viewer.getViewportSceneView(0)->setBackgroundColor(osg::Vec4(.2,.2,.2,1));
*/
    viewer.addViewport( rootNode,0.0,0.0,0.5,0.5);
    viewer.addViewport( rootNode,0.5,0.0,0.5,0.5);
    viewer.addViewport( rootNode,0.0,0.5,1.0,0.5);
    viewer.addHUD(scene2d);

    // register trackball, flight and drive.
    viewer.registerCameraManipulator(new osgUtil::TrackballManipulator);
    viewer.registerCameraManipulator(new osgUtil::FlightManipulator);
    viewer.registerCameraManipulator(new osgUtil::DriveManipulator);

    viewer.open();
    viewer.run();

    return 0;
}
