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



// class to handle events with a pick
class PickHandler : public osgGA::GUIEventHandler {
public: 

    PickHandler(osgProducer::OsgCameraGroup* cg,osgText::Text* updateText):
        _cg(cg),
        _updateText(updateText) {}
        
    ~PickHandler() {}
    
    bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us);

    virtual void pick(const osgGA::GUIEventAdapter& ea);

    void setLabel(const std::string& name)
    {
        if (_updateText.get()) _updateText->setText(name);
    }
    
protected:

    osgProducer::OsgCameraGroup *_cg;
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
    osg::Node *scene=_cg->getSceneData();//main node of the scene.
    if (scene)
    {
        float x=ea.getXnormalized();
        float y=ea.getYnormalized();
        osgUtil::PickVisitor iv;
        const float *matView;
        const float *matProj;
        Producer::Camera *cmm=_cg->getCamera(0);
        matView=cmm->getViewMatrix(); 
        matProj=cmm->getProjectionMatrix();
        osg::Matrix vum;
        vum.set(matView);
        vum.postMult(osg::Matrix(matProj));
        osg::Matrix windowmatrix=osg::Matrix::translate(1.0f,1.0f,1.0f)*
            osg::Matrix::scale(0.5f,0.5f,0.5f);
        vum.postMult(windowmatrix);
        osgUtil::IntersectVisitor::HitList& hlist=iv.getHits(scene, vum, x,y);
        std::string gdlist="";
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
                if (geode) {
                    gdlist=gdlist+" "+geode->getName();
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
