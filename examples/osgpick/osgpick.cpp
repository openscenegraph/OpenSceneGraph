/* OpenSceneGraph example, osgpick.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

/* osgpick sample - Mouse picking in a 3d scene,
*/

#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/CompositeViewer>

#include <osgGA/TerrainManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>

#include <osg/Material>
#include <osg/Geode>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Projection>
#include <osg/MatrixTransform>
#include <osg/Camera>
#include <osg/io_utils>
#include <osg/ShapeDrawable>

#include <osgText/Text>

#include <sstream>

// class to handle events with a pick
class PickHandler : public osgGA::GUIEventHandler {
public:

    PickHandler(osgText::Text* updateText):
        _updateText(updateText) {}

    ~PickHandler() {}

    bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa);

    virtual void pick(osgViewer::View* view, const osgGA::GUIEventAdapter& ea);

    void setLabel(const std::string& name)
    {
        if (_updateText.get()) _updateText->setText(name);
    }

protected:

    osg::ref_ptr<osgText::Text>  _updateText;
};

bool PickHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::PUSH):
        {
            osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
            if (view) pick(view,ea);
            return false;
        }
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            if (ea.getKey()=='c')
            {
                osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
                osg::ref_ptr<osgGA::GUIEventAdapter> event = new osgGA::GUIEventAdapter(ea);
                event->setX((ea.getXmin()+ea.getXmax())*0.5);
                event->setY((ea.getYmin()+ea.getYmax())*0.5);
                if (view) pick(view,*event);
            }
            return false;
        }
        default:
            return false;
    }
}

void PickHandler::pick(osgViewer::View* view, const osgGA::GUIEventAdapter& ea)
{
    osgUtil::LineSegmentIntersector::Intersections intersections;

    std::string gdlist="";

    if (view->computeIntersections(ea,intersections))
    {
        for(osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
            hitr != intersections.end();
            ++hitr)
        {
            std::ostringstream os;
            if (!hitr->nodePath.empty() && !(hitr->nodePath.back()->getName().empty()))
            {
                // the geodes are identified by name.
                os<<"Object \""<<hitr->nodePath.back()->getName()<<"\""<<std::endl;
            }
            else if (hitr->drawable.valid())
            {
                os<<"Object \""<<hitr->drawable->className()<<"\""<<std::endl;
            }

            os<<"        local coords vertex("<< hitr->getLocalIntersectPoint()<<")"<<"  normal("<<hitr->getLocalIntersectNormal()<<")"<<std::endl;
            os<<"        world coords vertex("<< hitr->getWorldIntersectPoint()<<")"<<"  normal("<<hitr->getWorldIntersectNormal()<<")"<<std::endl;
            const osgUtil::LineSegmentIntersector::Intersection::IndexList& vil = hitr->indexList;
            for(unsigned int i=0;i<vil.size();++i)
            {
                os<<"        vertex indices ["<<i<<"] = "<<vil[i]<<std::endl;
            }

            gdlist += os.str();
        }
    }
    setLabel(gdlist);
}

osg::Node* createHUD(osgText::Text* updateText)
{

    // create the hud. derived from osgHud.cpp
    // adds a set of quads, each in a separate Geode - which can be picked individually
    // eg to be used as a menuing/help system!
    // Can pick texts too!

    osg::Camera* hudCamera = new osg::Camera;
    hudCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    hudCamera->setProjectionMatrixAsOrtho2D(0,1280,0,1024);
    hudCamera->setViewMatrix(osg::Matrix::identity());
    hudCamera->setRenderOrder(osg::Camera::POST_RENDER);
    hudCamera->setClearMask(GL_DEPTH_BUFFER_BIT);

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
        hudCamera->addChild(geode);

        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );

        text->setFont(timesFont);
        text->setText("Picking in Head Up Displays is simple!");
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
        quad->setColorArray(colors, osg::Array::BIND_OVERALL);
        (*vertices)[0]=position;
        (*vertices)[1]=position+dx;
        (*vertices)[2]=position+dx+dy;
        (*vertices)[3]=position+dy;
        quad->setVertexArray(vertices);
        quad->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
        geode->addDrawable(quad);
        hudCamera->addChild(geode);

        position += delta;
    }



    { // this displays what has been selected
        osg::Geode* geode = new osg::Geode();
        osg::StateSet* stateset = geode->getOrCreateStateSet();
        stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
        stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
        geode->setName("The text label");
        geode->addDrawable( updateText );
        hudCamera->addChild(geode);

        updateText->setCharacterSize(20.0f);
        updateText->setFont(timesFont);
        updateText->setColor(osg::Vec4(1.0f,1.0f,0.0f,1.0f));
        updateText->setText("");
        updateText->setPosition(position);
        updateText->setDataVariance(osg::Object::DYNAMIC);

        position += delta;
    }

    return hudCamera;

}

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> scene = osgDB::readRefNodeFiles(arguments);

    if (!scene && arguments.read("--relative-camera-scene"))
    {
        // Create a test scene with a camera that has a relative reference frame.
        osg::Group* group = new osg::Group();

        osg::Geode* sphere = new osg::Geode();
        sphere->setName("Sphere");
        sphere->addDrawable(new osg::ShapeDrawable(new osg::Sphere()));

        osg::Geode* cube = new osg::Geode();
        cube->setName("Cube");
        cube->addDrawable(new osg::ShapeDrawable(new osg::Box()));

        osg::Camera* camera = new osg::Camera();
        camera->setRenderOrder(osg::Camera::POST_RENDER);
        camera->setClearMask(GL_DEPTH_BUFFER_BIT);
        camera->setReferenceFrame(osg::Transform::RELATIVE_RF);
        camera->setViewMatrix(osg::Matrix::translate(-2, 0, 0));

        osg::MatrixTransform* xform = new osg::MatrixTransform(osg::Matrix::translate(1, 1, 1));
        xform->addChild(camera);

        group->addChild(sphere);
        group->addChild(xform);
        camera->addChild(cube);

        scene = group;
    }

    // if not loaded assume no arguments passed in, try use default mode instead.
    if (!scene) scene = osgDB::readRefNodeFile("fountain.osgt");

    osg::ref_ptr<osg::Group> group = dynamic_cast<osg::Group*>(scene.get());
    if (!group)
    {
        group = new osg::Group;
        group->addChild(scene.get());
    }

    osg::ref_ptr<osgText::Text> updateText = new osgText::Text;

    // add the HUD subgraph.
    group->addChild(createHUD(updateText.get()));

    if (arguments.read("--CompositeViewer"))
    {
        osg::ref_ptr<osgViewer::View> view = new osgViewer::View;
        // add the handler for doing the picking
        view->addEventHandler(new PickHandler(updateText.get()));

        // set the scene to render
        view->setSceneData(group.get());

        view->setUpViewAcrossAllScreens();

        osgViewer::CompositeViewer viewer;
        viewer.addView(view.get());

        return viewer.run();

    }
    else
    {
        osgViewer::Viewer viewer;


        // add all the camera manipulators
        {
            osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

            keyswitchManipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator() );
            keyswitchManipulator->addMatrixManipulator( '2', "Flight", new osgGA::FlightManipulator() );
            keyswitchManipulator->addMatrixManipulator( '3', "Drive", new osgGA::DriveManipulator() );

            unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
            keyswitchManipulator->addMatrixManipulator( '4', "Terrain", new osgGA::TerrainManipulator() );

            std::string pathfile;
            char keyForAnimationPath = '5';
            while (arguments.read("-p",pathfile))
            {
                osgGA::AnimationPathManipulator* apm = new osgGA::AnimationPathManipulator(pathfile);
                if (apm || !apm->valid())
                {
                    num = keyswitchManipulator->getNumMatrixManipulators();
                    keyswitchManipulator->addMatrixManipulator( keyForAnimationPath, "Path", apm );
                    ++keyForAnimationPath;
                }
            }

            keyswitchManipulator->selectMatrixManipulator(num);

            viewer.setCameraManipulator( keyswitchManipulator.get() );
        }

        // add the handler for doing the picking
        viewer.addEventHandler(new PickHandler(updateText.get()));

        // set the scene to render
        viewer.setSceneData(group.get());

        return viewer.run();
    }

}
