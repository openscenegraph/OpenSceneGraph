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
/* osgpick sample
* demonstrate use of osgUtil/PickVisitor for picking in a HUD or
* in a 3d scene,
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

#include <osgUtil/PickVisitor>



static Producer::CameraConfig *BuildConfig(void)
{
    Producer::RenderSurface *rs1 = new Producer::RenderSurface;
    rs1->setScreenNum(0);
    //rs1->useBorder(false);
    rs1->setWindowRect(0,0,640,480);
    Producer::Camera *camera1 = new Producer::Camera;
    camera1->setRenderSurface(rs1);
    camera1->setOffset( 1.0, 0.0 );

    Producer::RenderSurface *rs2 = new Producer::RenderSurface;
    rs2->setScreenNum(0);
    //rs2->useBorder(false);
    rs2->setWindowRect(640,0,640,480);
    Producer::Camera *camera2 = new Producer::Camera;
    camera2->setRenderSurface(rs2);
    camera2->setOffset( -1.0, 0.0 );

    Producer::InputArea *ia = new Producer::InputArea;
    ia->addInputRectangle( rs1, Producer::InputRectangle(0.0,0.5,0.0,1.0));
    ia->addInputRectangle( rs2, Producer::InputRectangle(0.5,1.0,0.0,1.0));
//    ia->addInputRectangle( rs1, Producer::InputRectangle(-1.0,0.0,-1.0,1.0));
//    ia->addInputRectangle( rs2, Producer::InputRectangle(0.0,1.0,-1.0,1.0));


    Producer::CameraConfig *cfg = new Producer::CameraConfig;
    cfg->addCamera("Camera 1",camera1);
    cfg->addCamera("Camera 2", camera2);
    cfg->setInputArea(ia);
    return cfg;
}

// class to handle events with a pick
class PickHandler : public osgGA::GUIEventHandler {
public: 

    PickHandler(osgProducer::Viewer* viewer,osgText::Text* updateText):
        _viewer(viewer),
        _updateText(updateText) {}
        
    ~PickHandler() {}
    
    bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us);

    virtual void pick(const osgGA::GUIEventAdapter& ea);

    void setLabel(const std::string& name)
    {
        if (_updateText.get()) _updateText->setText(name);
    }
    
protected:

    osgProducer::Viewer* _viewer;
    osg::ref_ptr<osgText::Text>  _updateText;
};

bool PickHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
{
    switch(ea.getEventType())
    {
    case(osgGA::GUIEventAdapter::FRAME):
        {
            pick(ea);
        }
        return false;
        
    default:
        return false;
    }
}

void PickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
    // OK here is the interesting bit - How To Pick a Geode
    // including geodes in a HUD under a Projection Matrix
    osg::Node *scene=_viewer->getSceneData();//main node of the scene.
    if (scene)
    {
        float x=ea.getX();
        float y=ea.getY();


        Producer::KeyboardMouse* km = _viewer->getKeyboardMouse();


        std::string gdlist="";

        for(unsigned int i=0;i<_viewer->getNumberOfCameras();++i)
        {
            Producer::Camera* cmm=_viewer->getCamera(i);
            Producer::RenderSurface* rs = cmm->getRenderSurface();

            std::cout << "checking camara "<<i<<std::endl;

            float pixel_x,pixel_y;
            if (km->computePixelCoords(x,y,rs,pixel_x,pixel_y))
            {
                std::cout << "    compute pixel coords "<<pixel_x<<"  "<<pixel_y<<std::endl;

                int pr_wx, pr_wy;
                unsigned int pr_width, pr_height;
                cmm->getProjectionRect( pr_wx, pr_wy, pr_width, pr_height );

                int rs_wx, rs_wy;
                unsigned int rs_width, rs_height;
                rs->getWindowRect( rs_wx, rs_wy, rs_width, rs_height );
                
                pr_wx += rs_wx;
                pr_wy += rs_wy;

                std::cout << "    wx = "<<pr_wx<<"  wy = "<<pr_wy<<" width="<<pr_width<<" height="<<pr_height<<std::endl;



                if (pixel_x<(float)pr_wx) break;
                if (pixel_x>(float)(pr_wx+pr_width)) break;

                if (pixel_y<(float)pr_wy) break;
                if (pixel_y>(float)(pr_wy+pr_height)) break;


                float rx = 2.0f*(pixel_x - (float)pr_wx)/(float)pr_width-1.0f;
                float ry = 2.0f*(pixel_y - (float)pr_wy)/(float)pr_height-1.0f;

                std::cout << "    rx "<<rx<<"  "<<ry<<std::endl;

                osg::Matrix vum(osg::Matrix(cmm->getViewMatrix()) *
                                osg::Matrix(cmm->getProjectionMatrix())/* * 
                                osg::Matrix::translate(1.0f,1.0f,1.0f) *
                                osg::Matrix::scale(0.5f,0.5f,0.5f)*/);

                osgUtil::PickVisitor iv;
                osgUtil::IntersectVisitor::HitList& hlist=iv.getHits(scene, vum, rx,ry);
                if (iv.hits())
                {
                    for(osgUtil::IntersectVisitor::HitList::iterator hitr=hlist.begin();
                    hitr!=hlist.end();
                    ++hitr)
                    {
                        //osg::Vec3 ip = hitr->getLocalIntersectPoint();
                        //osg::Vec3 in = hitr->getLocalIntersectNormal();
                        osg::Geode* geode = hitr->_geode.get();
                        // the geodes are identified by name.
                        if (geode)
                        {
                            if (!geode->getName().empty())
                            {
                                gdlist=gdlist+" "+geode->getName();
                            }
                            else
                            {
                                gdlist=gdlist+" geode";
                            }
                        } 
                    }
                }
            }
        }

        setLabel(gdlist);
    }
}

osg::Node* createHUD(osgText::Text* updateText)
{    // create the hud. derived from osgHud.cpp
    // adds a set of quads, each in a separate Geode - which can be picked individually
    // eg to be used as a menuing/help system!
    // Can pick texts too!
    osg::MatrixTransform* modelview_abs = new osg::MatrixTransform;
    modelview_abs->setReferenceFrame(osg::Transform::RELATIVE_TO_ABSOLUTE);
    modelview_abs->setMatrix(osg::Matrix::identity());
    
    osg::Projection* projection = new osg::Projection;
    projection->setMatrix(osg::Matrix::ortho2D(0,1280,0,1024));
    projection->addChild(modelview_abs);
    
    
    std::string timesFont("fonts/times.ttf");
    
    // turn lighting off for the text and disable depth test to ensure its always ontop.
    osg::Vec3 position(150.0f,800.0f,0.0f);
    osg::Vec3 delta(0.0f,-60.0f,0.0f);
    
    {
        osg::Geode* geode = new osg::Geode();
        osg::StateSet* stateset = geode->getOrCreateStateSet();
        stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
        stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
        geode->setName("simple");
        modelview_abs->addChild(geode);
        
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );
        
        text->setFont(timesFont);
        text->setText("Picking in Head Up Displays is simple !=]");
        text->setPosition(position);
        
        position += delta;
    }    
    
    
    for (int i=0; i<5; i++) {
        osg::Vec3 dy(0.0f,-30.0f,0.0f);
        osg::Vec3 dx(120.0f,0.0f,0.0f);
        osg::Geode* geode = new osg::Geode();
        osg::StateSet* stateset = geode->getOrCreateStateSet();
        const char *opts[]={"One", "Two", "Three", "January", "Feb", "2003"};
        osg::Geometry *quad=new osg::Geometry;
        stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
        stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
        std::string name="subOption";
        name += " ";
        name += std::string(opts[i]);
        geode->setName(name);
        osg::Vec3Array* vertices = new osg::Vec3Array(4); // 1 quad
        osg::Vec4Array* colors = new osg::Vec4Array;
        colors = new osg::Vec4Array;
        colors->push_back(osg::Vec4(0.8-0.1*i,0.1*i,0.2*i, 1.0));
        quad->setColorArray(colors);
        quad->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE);
        (*vertices)[0]=position;
        (*vertices)[1]=position+dx;
        (*vertices)[2]=position+dx+dy;
        (*vertices)[3]=position+dy;
        quad->setVertexArray(vertices);
        quad->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
        geode->addDrawable(quad);
        modelview_abs->addChild(geode);
        
        position += delta;
    }    
    
    
    
    { // this displays what has been selected
        osg::Geode* geode = new osg::Geode();
        osg::StateSet* stateset = geode->getOrCreateStateSet();
        stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
        stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
        geode->setName("whatis");
        geode->addDrawable( updateText );
        modelview_abs->addChild(geode);
        
        updateText->setFont(timesFont);
        updateText->setText("whatis will tell you what is under the mouse");
        updateText->setPosition(position);
        
        position += delta;
    }    
    
    return projection;

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
    //osgProducer::Viewer viewer(arguments);
    osgProducer::Viewer viewer(BuildConfig());

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
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles(arguments);

    osg::ref_ptr<osg::Group> group = dynamic_cast<osg::Group*>(scene.get());
    if (!group)
    {
        group = new osg::Group;
        group->addChild(scene.get());
    }

    osg::ref_ptr<osgText::Text> updateText = new osgText::Text;

    // add the HUD subgraph.    
    group->addChild(createHUD(updateText.get()));

    // add the handler for doing the picking
    viewer.getEventHandlerList().push_front(new PickHandler(&viewer,updateText.get()));

    // set the scene to render
    viewer.setSceneData(group.get());

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
