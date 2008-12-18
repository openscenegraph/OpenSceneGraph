// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgetwindow.cpp 66 2008-07-14 21:54:09Z cubicool $

#include <iostream>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgWidget/WindowManager>
#include <osgWidget/ViewerEventHandlers>
#include <osgWidget/Box>

const unsigned int MASK_2D = 0xF0000000;
const unsigned int MASK_3D = 0x0F000000;

// Here we create (and later demonstrate) the use of a simple function callback.
bool windowClicked(osgWidget::Event& ev) {
    std::cout << "windowClicked: " << ev.getWindow()->getName() << std::endl;

    if(ev.getData()) {
        std::string* s = static_cast<std::string*>(ev.getData());

        std::cout << "This is data attached to the event: " << *s << std::endl;
    }

    return true;
}

bool windowScrolled(osgWidget::Event& ev) {
    osgWidget::warn()
        << "scrolling up? " << ev.getWindowManager()->isMouseScrollingUp()
        << std::endl
    ;

    return true;
}

// Here we dcreate a new class and show how to use a method callback (which differs from
// a function callback in that we are required to also pass the "this" argument).
struct Object {
    bool windowClicked(osgWidget::Event& ev) {
        std::cout << "Object::windowClicked " << ev.getWindow()->getName() << std::endl;

        return true;
    }
};

// This is the more "traditional" method of creating a callback.
struct CallbackObject: public osgWidget::Callback {
    CallbackObject(osgWidget::EventType evType):
    osgWidget::Callback(evType) {
    }

    virtual bool operator()(osgWidget::Event& ev) {
        std::cout << "here" << std::endl;
        
        return false;
    }
};

int main(int argc, char** argv) {
    osgViewer::Viewer viewer;
    
    // Let's get busy! The WindowManager class is actually an osg::Switch,
    // so you can add it to (ideally) an orthographic camera and have it behave as
    // expected. Note that you create a WindowManager with a NodeMask--it is very important
    // that this be unique for picking to work properly. This also makes it possible to have
    // multiple WindowManagers each operating on their own, unique set of Window objects.
    // The final bool argument is a group of flags that introduce optional functionality
    // for the WindowManager. In our case we include the flags USE_PYTHON and USE_LUA,
    // to demonstrate (and test) their usage. Finally, we pass the temporary WM_NO_BETA_WARN
    // argument, which prevents creating the orange warning window. :) It will be shown
    // in other examples...
    osgWidget::WindowManager* wm = new osgWidget::WindowManager(
        &viewer,
        1280.0f,
        1024.0f,
        MASK_2D,
        osgWidget::WindowManager::WM_USE_LUA |
        osgWidget::WindowManager::WM_USE_PYTHON |
        osgWidget::WindowManager::WM_PICK_DEBUG
    );

    // An actual osgWidget::Window is pure virtual, so we've got to use the osgWidget::Box
    // implementation for now. At a later time, support for Tables and other kinds of
    // advanced layout Window types will be added.
    osgWidget::Window* box = new osgWidget::Box("box", osgWidget::Box::HORIZONTAL);

    // Now we actually attach our two types of callbacks to the box instance. The first
    // uses the simple function signature, the second uses a bound method, passing "this"
    // as the second argument to the Callback constructor.
    // Object obj;

    static std::string data = "lol ur face!";

    /*
    box->addCallback(new osgWidget::Callback(&windowClicked, osgWidget::EVENT_MOUSE_PUSH, &data));
    box->addCallback(new osgWidget::Callback(&windowScrolled, osgWidget::EVENT_MOUSE_SCROLL));
    box->addCallback(osgWidget::Callback(
        &Object::windowClicked,
        &obj,
        osgWidget::EVENT_MOUSE_PUSH
    ));
    */

    box->addCallback(new CallbackObject(osgWidget::EVENT_MOUSE_PUSH));

    // Create some of our "testing" Widgets; included are two Widget subclasses I made
    // during testing which I've kept around for testing purposes. You'll notice
    // that you cannot move the box using the NullWidget, and that the NotifyWidget
    // is a bit verbose. :)
    osgWidget::Widget* widget1 = new osgWidget::NotifyWidget("widget1", 300.0f, 100.0f);
    osgWidget::Widget* widget2 = new osgWidget::NullWidget("widget2", 400.0f, 75.0f);
    osgWidget::Widget* widget3 = new osgWidget::Widget("widget3", 100.0f, 100.0f);
    // Set the colors of widget1 and widget3 to green.
    widget1->setColor(0.0f, 1.0f, 0.0f, 1.0f);
    widget1->setCanFill(true);
    widget3->setColor(0.0f, 1.0f, 0.0f, 1.0f);

    widget1->setImage(osgDB::readImageFile("Images/Saturn.TGA"), true);

    // Set the color of widget2, to differentiate it and make it sassy. This is
    // like a poor man's gradient!
    widget2->setColor(0.9f, 0.0f, 0.0f, 0.9f, osgWidget::Widget::LOWER_LEFT);
    widget2->setColor(0.9f, 0.0f, 0.0f, 0.9f, osgWidget::Widget::LOWER_RIGHT);
    widget2->setColor(0.0f, 0.0f, 0.9f, 0.9f, osgWidget::Widget::UPPER_RIGHT);
    widget2->setColor(0.0f, 0.0f, 0.9f, 0.9f, osgWidget::Widget::UPPER_LEFT);

    // Now add our newly created widgets to our box.
    box->addWidget(widget1);
    box->addWidget(widget2);
    box->addWidget(widget3);

    // For maximum efficiency, Windows don't automatically reallocate their geometry
    // and internal positioning every time a widget is added. Thus, we either have to
    // call the WindowManger::resizeAllWindows method or manually call
    // Window::resize when we're ready.
    box->resize();

    // Now, lets clone our existing box and create a new copy of of it, also adding that
    // to the WindowManager. This demonstrates the usages of OSG's ->clone() support,
    // though that is abstracted by our META_UIObject macro.
    osgWidget::Window* boxCopy = osg::clone(box, "newBox", osg::CopyOp::DEEP_COPY_ALL);

    // Move our copy to make it visible.
    boxCopy->setOrigin(0.0f, 125.0f);

    boxCopy->getByName("widget1")->setColor(0.5f, 0.0f, 1.0f, 1.0f);
    boxCopy->getByName("widget3")->setColor(0.5f, 0.0f, 1.0f, 1.0f);

    // Add the successfully created Box (if we get this far) into the WindowManager, so
    // that they can receive events.
    wm->addChild(box);
    wm->addChild(boxCopy);

    // Now, ask our new box to be 100% the width of the WindowManager.
    boxCopy->resizePercent(100.0f, 0.0f);

    // Here we demonstrate the use of osgWidget/io_utils. This is really only useful for
    // debugging at the moment, but later I'll make it more generic for .osg and .ive
    // creation.
    // std::cout << *box << std::endl << *boxCopy << std::endl;

    // Setup our OSG objects for our scene; note the use of the utility function
    // createOrthoCamera, which is just a helper for setting up a proper viewing area.
    // An alternative (and a MUCH easier alternative at that!) is to
    // simply use the createParentOrthoCamera method of the WindowManager class,
    // which will wrap the calls to createOrthoCamera and addChild for us! Check out
    // some of the other examples to see this in action...
    osg::Group*  group  = new osg::Group();
    osg::Camera* camera = osgWidget::createOrthoCamera(1280.0f, 1024.0f);
    osg::Node*   model  = osgDB::readNodeFile("cow.osg");

    // Add our event handler; is this better as a MatrixManipulator? Add a few other
    // helpful ViewerEventHandlers.
    viewer.addEventHandler(new osgWidget::MouseHandler(wm));
    viewer.addEventHandler(new osgWidget::KeyboardHandler(wm));
    viewer.addEventHandler(new osgWidget::ResizeHandler(wm, camera));
    viewer.addEventHandler(new osgWidget::CameraSwitchHandler(wm, camera));
    viewer.addEventHandler(new osgViewer::StatsHandler());
    viewer.addEventHandler(new osgViewer::WindowSizeHandler());
    viewer.addEventHandler(new osgGA::StateSetManipulator(
        viewer.getCamera()->getOrCreateStateSet()
    ));

    // Set our first non-UI node to be something other than the mask we created our
    // WindowManager with to avoid picking.
    // TODO: Do I need to create a mechanism for doing this automatically, or should
    // that be the responsibility of the users of osgWidget?
    model->setNodeMask(MASK_3D);

    // Add the WindowManager instance to the 2D camera. This isn't strictly necessary,
    // and you can get some cool results putting the WindowManager directly into a
    // 3D scene. This is not necessary if you use WindowManager::createParentOrthoCamera.
    camera->addChild(wm);

    // Add our camera and a testing 3D model to the scene.
    group->addChild(camera);
    group->addChild(model);

    // Here we show how to both run simple strings of code AND run entire files. These
    // assume that you're running the osgwidgetwindow example from the build directory,
    // otherwise you'll need to adjust the file path below in the call to runFile().
    wm->getLuaEngine()->eval("window = osgwidget.newWindow()");
    wm->getLuaEngine()->runFile("osgWidget/osgwidgetwindow.lua");

    wm->getPythonEngine()->eval("import osgwidget");
    wm->getPythonEngine()->runFile("osgWidget/osgwidgetwindow.py");

    viewer.setUpViewInWindow(0, 0, 1280, 1024);
    viewer.setSceneData(group);

    /*
    osgViewer::Viewer::Cameras cameras; 
    viewer.getCameras(cameras);
    osg::Camera* c = cameras[0];
    osg::Matrix s = osg::Matrix::scale(1.0f, -1.0f, 1.0f);
    c->setProjectionMatrix(s * c->getProjectionMatrix());
    */

    return viewer.run();
}
