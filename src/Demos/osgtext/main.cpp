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
#include <osg/Material>
#include <osg/BlendFunc>
#include <osg/MatrixTransform>
#include <osg/PolygonMode>
#include <osg/Depth>
#include <osg/Notify>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgGLUT/Viewer>
#include <osgGLUT/glut>

#include <osgText/Text>
#include <vector>


using namespace osgGLUT;

///////////////////////////////////////////////////////////////////////////////
// globals
#define        TEXT_POLYGON    "Polygon Font - jygq"
#define        TEXT_OUTLINE    "Outline Font - jygq"
#define        TEXT_TEXTURE    "Texture Font - jygq"
#define        TEXT_BITMAP        "Bitmap Font - jygq"
#define        TEXT_PIXMAP        "Pixmap Font - jygq"

#define        TEXT_COL_2D        osg::Vec4(.9,.9,.9,1)
#define        TEXT_COL_3D        osg::Vec4(.99,.3,.2,1)


std::string    timesFont("fonts/times.ttf");
std::string    arialFont("fonts/arial.ttf");

int    gFontSize=18;
int    gFontSize1=24;
std::vector<osgText::Text*>    gTextList;
osgText::Text::AlignmentType    gAlignment=osgText::Text::BASE_LINE;


void set2dScene(osg::Group* rootNode)
{
    osg::Geode* geode = new osg::Geode();
    rootNode->addChild(geode);
    
    osg::Vec3 position(150.0f,10.0f,0.0f);
    osg::Vec3 delta(90.0f,120.0f,0.0f);

    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );
        gTextList.push_back(text);

        text->setFont(timesFont);
        text->setFontSize(gFontSize,gFontSize);
        text->setText("String 1");
        text->setPosition(position);
        text->setDrawMode( osgText::Text::TEXT |
                           osgText::Text::BOUNDINGBOX |
                           osgText::Text::ALIGNMENT );
        text->setAlignment(gAlignment);
        
        position += delta;
    }    


    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );
        gTextList.push_back(text);

        text->setFont(timesFont);
        text->setFontSize(gFontSize,gFontSize);
        text->setText("String 1");
        text->setPosition(position);
        text->setDrawMode( osgText::Text::TEXT |
                           osgText::Text::BOUNDINGBOX |
                           osgText::Text::ALIGNMENT );
        text->setAlignment(gAlignment);
        
        position += delta;
    }    


    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );
        gTextList.push_back(text);

        text->setFont(timesFont);
        text->setFontSize(gFontSize,gFontSize);
        text->setText("String 1");
        text->setPosition(position);
        text->setDrawMode( osgText::Text::TEXT |
                           osgText::Text::BOUNDINGBOX |
                           osgText::Text::ALIGNMENT );
        text->setAlignment(gAlignment);
        
        position += delta;
    }    
}

void setScene(osg::Group* rootNode)
{
    osg::Geode* geode = new osg::Geode();
    rootNode->addChild(geode);
    
    osg::Vec3 position(150.0f,10.0f,0.0f);
    osg::Vec3 delta(90.0f,120.0f,0.0f);

    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );
        gTextList.push_back(text);

        text->setFont(timesFont);
        text->setFontSize(gFontSize,gFontSize);
        text->setText("String 1");
        text->setPosition(position);
        text->setDrawMode( osgText::Text::TEXT |
                           osgText::Text::BOUNDINGBOX |
                           osgText::Text::ALIGNMENT );
        text->setAlignment(gAlignment);
        
        position += delta;
    }    


    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );
        gTextList.push_back(text);

        text->setFont(timesFont);
        text->setFontSize(gFontSize,gFontSize);
        text->setText("String 1");
        text->setPosition(position);
        text->setDrawMode( osgText::Text::TEXT |
                           osgText::Text::BOUNDINGBOX |
                           osgText::Text::ALIGNMENT );
        text->setAlignment(gAlignment);
        
        position += delta;
    }    


    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );
        gTextList.push_back(text);

        text->setFont(timesFont);
        text->setFontSize(gFontSize,gFontSize);
        text->setText("String 1");
        text->setPosition(position);
        text->setDrawMode( osgText::Text::TEXT |
                           osgText::Text::BOUNDINGBOX |
                           osgText::Text::ALIGNMENT );
        text->setAlignment(gAlignment);
        
        position += delta;
    }    

}

class TextViewer: public osgGLUT::Viewer
{
public:

    virtual float update(unsigned int viewport)
    {
        float ret;
        ret=Viewer::update(viewport);
        if(_hudSceneView.valid() && viewport>=_viewportList.size()-1)
        {
            _hudSceneView->update();
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
            _hudSceneView->getCullVisitor()->setCullingMode(osgUtil::CullVisitor::NO_CULLING);
            _hudSceneView->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR);
            
            _hudCam = new osg::Camera;

            // leftBottom 
            _hudSceneView->setCamera(_hudCam.get());
        }

        return ret;
    }


protected:

    osg::ref_ptr<osg::Camera>           _hudCam;
    osg::ref_ptr<osgUtil::SceneView>    _hudSceneView;

    virtual void clear()
    {
        Viewer::clear();
        _hudCam = NULL;
        _hudSceneView = NULL;
    }


    virtual void keyboard(int key, int x, int y, bool keydown)
    {
        if (keydown)
        {
            switch(key)
            {
                case '1':
                    {    // change DrawMode
                        std::vector<osgText::Text*>::iterator itr=gTextList.begin();
                        for(;itr!=gTextList.end();itr++)
                            (*itr)->setDrawMode(osgText::Text::TEXT ^ (*itr)->getDrawMode());
                    }
                    return;
                case '2':
                    {    // change DrawMode
                        std::vector<osgText::Text*>::iterator itr=gTextList.begin();
                        for(;itr!=gTextList.end();itr++)
                            (*itr)->setDrawMode(osgText::Text::BOUNDINGBOX ^ (*itr)->getDrawMode());
                    }
                    return;
                case '3':
                    {    // change DrawMode
                        std::vector<osgText::Text*>::iterator itr=gTextList.begin();
                        for(;itr!=gTextList.end();itr++)
                            (*itr)->setDrawMode(osgText::Text::ALIGNMENT ^ (*itr)->getDrawMode());
                    }
                    return;
                ///////////////////////////////////////////////////////////////////
                case '4':
                    {    // change the textAlignment
                        gAlignment=(osgText::Text::AlignmentType)((int)gAlignment+1);
                        if(gAlignment>osgText::Text::BASE_LINE)
                            gAlignment=osgText::Text::LEFT_TOP;

                        std::vector<osgText::Text*>::iterator itr=gTextList.begin();
                        for(;itr!=gTextList.end();itr++)
                            (*itr)->setAlignment(gAlignment);
                    }
                    return;
	        default:
                    break;
            }
        }
	Viewer::keyboard(key,x,y,keydown);
    }

};

int main( int argc, char **argv )
{
    // initialize the GLUT
    glutInit( &argc, argv );

    // get the fontName
    if(argc > 1)
        timesFont=argv[1];
    if(argc > 2)
        arialFont=argv[2];
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
    osg::MatrixTransform*        textGroup = new osg::MatrixTransform;

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
    textGroup->preMult(osg::Matrix::rotate(osg::inDegrees(90.0f),1.0f,0.0f,0.0f));
    rootNode->addChild(textGroup);

    // setup the 2dNode
    set2dScene(scene2d);

    // initialize the viewer.
    TextViewer    viewer;
    viewer.setWindowTitle(argv[0]);
/*
    viewer.addViewport( rootNode );
    viewer.getViewportSceneView(0)->setBackgroundColor(osg::Vec4(.2,.2,.2,1));
*/
    viewer.addViewport( rootNode,0.0,0.0,0.5,0.5);
    viewer.addViewport( rootNode,0.5,0.0,0.5,0.5);
    viewer.addViewport( rootNode,0.0,0.5,1.0,0.5);
    viewer.addHUD(scene2d);

    // register trackball, flight and drive.
    viewer.registerCameraManipulator(new osgGA::TrackballManipulator);
    viewer.registerCameraManipulator(new osgGA::FlightManipulator);
    viewer.registerCameraManipulator(new osgGA::DriveManipulator);

    viewer.open();
    viewer.run();

    return 0;
}
