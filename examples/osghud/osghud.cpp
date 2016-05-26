/* OpenSceneGraph example, osghud.
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

#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>
#include <osgViewer/CompositeViewer>

#include <osgGA/TrackballManipulator>

#include <osg/Material>
#include <osg/Geode>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/PolygonOffset>
#include <osg/MatrixTransform>
#include <osg/Camera>
#include <osg/RenderInfo>

#include <osgDB/WriteFile>

#include <osgText/Text>


osg::Camera* createHUD()
{
    // create a camera to set up the projection and model view matrices, and the subgraph to draw in the HUD
    osg::Camera* camera = new osg::Camera;

    // set the projection matrix
    camera->setProjectionMatrix(osg::Matrix::ortho2D(0,1280,0,1024));

    // set the view matrix
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(osg::Matrix::identity());

    // only clear the depth buffer
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);

    // draw subgraph after main camera view.
    camera->setRenderOrder(osg::Camera::POST_RENDER);

    // we don't want the camera to grab event focus from the viewers main camera(s).
    camera->setAllowEventFocus(false);



    // add to this camera a subgraph to render
    {

        osg::Geode* geode = new osg::Geode();

        std::string timesFont("fonts/arial.ttf");

        // turn lighting off for the text and disable depth test to ensure it's always ontop.
        osg::StateSet* stateset = geode->getOrCreateStateSet();
        stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

        osg::Vec3 position(150.0f,800.0f,0.0f);
        osg::Vec3 delta(0.0f,-120.0f,0.0f);

        {
            osgText::Text* text = new  osgText::Text;
            geode->addDrawable( text );

            text->setFont(timesFont);
            text->setPosition(position);
            text->setText("Head Up Displays are simple :-)");

            position += delta;
        }


        {
            osgText::Text* text = new  osgText::Text;
            geode->addDrawable( text );

            text->setFont(timesFont);
            text->setPosition(position);
            text->setText("All you need to do is create your text in a subgraph.");

            position += delta;
        }


        {
            osgText::Text* text = new  osgText::Text;
            geode->addDrawable( text );

            text->setFont(timesFont);
            text->setPosition(position);
            text->setText("Then place an osg::Camera above the subgraph\n"
                          "to create an orthographic projection.\n");

            position += delta;
        }

        {
            osgText::Text* text = new  osgText::Text;
            geode->addDrawable( text );

            text->setFont(timesFont);
            text->setPosition(position);
            text->setText("Set the Camera's ReferenceFrame to ABSOLUTE_RF to ensure\n"
                          "it remains independent from any external model view matrices.");

            position += delta;
        }

        {
            osgText::Text* text = new  osgText::Text;
            geode->addDrawable( text );

            text->setFont(timesFont);
            text->setPosition(position);
            text->setText("And set the Camera's clear mask to just clear the depth buffer.");

            position += delta;
        }

        {
            osgText::Text* text = new  osgText::Text;
            geode->addDrawable( text );

            text->setFont(timesFont);
            text->setPosition(position);
            text->setText("And finally set the Camera's RenderOrder to POST_RENDER\n"
                          "to make sure it's drawn last.");

            position += delta;
        }


        {
            osg::BoundingBox bb;
            for(unsigned int i=0;i<geode->getNumDrawables();++i)
            {
                bb.expandBy(geode->getDrawable(i)->getBoundingBox());
            }

            osg::Geometry* geom = new osg::Geometry;

            osg::Vec3Array* vertices = new osg::Vec3Array;
            float depth = bb.zMin()-0.1;
            vertices->push_back(osg::Vec3(bb.xMin(),bb.yMax(),depth));
            vertices->push_back(osg::Vec3(bb.xMin(),bb.yMin(),depth));
            vertices->push_back(osg::Vec3(bb.xMax(),bb.yMin(),depth));
            vertices->push_back(osg::Vec3(bb.xMax(),bb.yMax(),depth));
            geom->setVertexArray(vertices);

            osg::Vec3Array* normals = new osg::Vec3Array;
            normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
            geom->setNormalArray(normals, osg::Array::BIND_OVERALL);

            osg::Vec4Array* colors = new osg::Vec4Array;
            colors->push_back(osg::Vec4(1.0f,1.0,0.8f,0.2f));
            geom->setColorArray(colors, osg::Array::BIND_OVERALL);

            geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));

            osg::StateSet* ss = geom->getOrCreateStateSet();
            ss->setMode(GL_BLEND,osg::StateAttribute::ON);
            //ss->setAttribute(new osg::PolygonOffset(1.0f,1.0f),osg::StateAttribute::ON);
            ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

            geode->addDrawable(geom);
        }

        camera->addChild(geode);
    }

    return camera;
}

struct SnapImage : public osg::Camera::DrawCallback
{
    SnapImage(const std::string& filename):
        _filename(filename),
        _snapImage(false)
    {
        _image = new osg::Image;
    }

    virtual void operator () (osg::RenderInfo& renderInfo) const
    {

        if (!_snapImage) return;

        osg::notify(osg::NOTICE)<<"Camera callback"<<std::endl;

        osg::Camera* camera = renderInfo.getCurrentCamera();
        osg::Viewport* viewport = camera ? camera->getViewport() : 0;

        osg::notify(osg::NOTICE)<<"Camera callback "<<camera<<" "<<viewport<<std::endl;

        if (viewport && _image.valid())
        {
            _image->readPixels(int(viewport->x()),int(viewport->y()),int(viewport->width()),int(viewport->height()),
                               GL_RGBA,
                               GL_UNSIGNED_BYTE);
            osgDB::writeImageFile(*_image, _filename);

            osg::notify(osg::NOTICE)<<"Taken screenshot, and written to '"<<_filename<<"'"<<std::endl;
        }

        _snapImage = false;
    }

    std::string                         _filename;
    mutable bool                        _snapImage;
    mutable osg::ref_ptr<osg::Image>    _image;
};



struct SnapeImageHandler : public osgGA::GUIEventHandler
{

    SnapeImageHandler(int key,SnapImage* si):
        _key(key),
        _snapImage(si) {}

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        if (ea.getHandled()) return false;

        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYUP):
            {
                if (ea.getKey() == 'o' )
                {
                    osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
                    osg::Node* node = view ? view->getSceneData() : 0;
                    if (node)
                    {
                        osgDB::writeNodeFile(*node, "hud.osgt");
                        osgDB::writeNodeFile(*node, "hud.osgb");
                    }
                    return true;
                }

                if (ea.getKey() == _key)
                {
                    osg::notify(osg::NOTICE)<<"event handler"<<std::endl;
                    _snapImage->_snapImage = true;
                    return true;
                }

                break;
            }
        default:
            break;
        }

        return false;
    }

    int                     _key;
    osg::ref_ptr<SnapImage> _snapImage;
};


int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);


    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> scene = osgDB::readRefNodeFiles(arguments);

    // if not loaded assume no arguments passed in, try use default model instead.
    if (!scene) scene = osgDB::readRefNodeFile("dumptruck.osgt");


    if (!scene)
    {
        osg::notify(osg::NOTICE)<<"No model loaded"<<std::endl;
        return 1;
    }


    if (arguments.read("--Viewer"))
    {
        // construct the viewer.
        osgViewer::Viewer viewer;

        // create a HUD as slave camera attached to the master view.

        viewer.setUpViewAcrossAllScreens();

        osgViewer::Viewer::Windows windows;
        viewer.getWindows(windows);

        if (windows.empty()) return 1;

        osg::Camera* hudCamera = createHUD();

        // set up cameras to render on the first window available.
        hudCamera->setGraphicsContext(windows[0]);
        hudCamera->setViewport(0,0,windows[0]->getTraits()->width, windows[0]->getTraits()->height);

        viewer.addSlave(hudCamera, false);

        // set the scene to render
        viewer.setSceneData(scene);

        return viewer.run();

    }
    if (arguments.read("--CompositeViewer"))
    {
        // construct the viewer.
        osgViewer::CompositeViewer viewer;

        // create the main 3D view
        osgViewer::View* view = new osgViewer::View;
        viewer.addView(view);

        view->setSceneData(scene);
        view->setUpViewAcrossAllScreens();;
        view->setCameraManipulator(new osgGA::TrackballManipulator);

        // now create the HUD camera's view

        osgViewer::Viewer::Windows windows;
        viewer.getWindows(windows);

        if (windows.empty()) return 1;

        osg::Camera* hudCamera = createHUD();

        // set up cameras to render on the first window available.
        hudCamera->setGraphicsContext(windows[0]);
        hudCamera->setViewport(0,0,windows[0]->getTraits()->width, windows[0]->getTraits()->height);

        osgViewer::View* hudView = new osgViewer::View;
        hudView->setCamera(hudCamera);

        viewer.addView(hudView);

        return viewer.run();

    }
    else
    {
        // construct the viewer.
        osgViewer::Viewer viewer;

        SnapImage* postDrawCallback = new SnapImage("PostDrawCallback.png");
        viewer.getCamera()->setPostDrawCallback(postDrawCallback);
        viewer.addEventHandler(new SnapeImageHandler('p',postDrawCallback));

        SnapImage* finalDrawCallback = new SnapImage("FinalDrawCallback.png");
        viewer.getCamera()->setFinalDrawCallback(finalDrawCallback);
        viewer.addEventHandler(new SnapeImageHandler('f',finalDrawCallback));

        osg::ref_ptr<osg::Group> group = new osg::Group;

        // add the HUD subgraph.
        if (scene.valid()) group->addChild(scene);
        group->addChild(createHUD());

        // set the scene to render
        viewer.setSceneData(group);

        return viewer.run();
    }

}
