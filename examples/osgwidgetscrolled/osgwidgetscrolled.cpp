// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgetframe.cpp 34 2008-04-07 03:12:41Z cubicool $

#include <osgDB/ReadFile>

#include <osgWidget/Util>
#include <osgWidget/WindowManager>
#include <osgWidget/Frame>
#include <osgWidget/Box>

const unsigned int MASK_2D = 0xF0000000;

// NOTE: THIS IS JUST A TEMPORARY HACK! :) This functionality will all eventually be
// encapsulate into another class in osgWidget proper.
bool scrollWindow(osgWidget::Event& ev) {
    // The first thing we need to do is make sure we have a Frame object...
    osgWidget::Frame* frame = dynamic_cast<osgWidget::Frame*>(ev.getWindow());

    if(!frame) return false;

    // And now we need to make sure our Frame has a valid internal EmbeddedWindow widget.
    osgWidget::Window::EmbeddedWindow* ew =
        dynamic_cast<osgWidget::Window::EmbeddedWindow*>(frame->getEmbeddedWindow())
    ;

    if(!ew) return false;

    // Lets get the visible area so that we can use it to make sure our scrolling action
    // is necessary in the first place.
    const osgWidget::Quad& va = ew->getWindow()->getVisibleArea();

    // The user wants to scroll up; make sure that the visible area's Y origin isn't already
    // at 0.0f, 0.0f.
    if(ev.getWindowManager()->isMouseScrollingUp() && va[1] != 0.0f)
        ew->getWindow()->addVisibleArea(0, -20)
    ;

    else if(va[1] <= (ew->getWindow()->getHeight() - ew->getHeight()))
        ew->getWindow()->addVisibleArea(0, 20)
    ;

    // We need to manually call update to make sure the visible area scissoring is done
    // properly.
    frame->update();

    return true;
}

bool changeTheme(osgWidget::Event& ev) {
    std::string theme;

    if(ev.key == osgGA::GUIEventAdapter::KEY_Right)
        theme = "osgWidget/theme-1.png"
    ;

    else if(ev.key == osgGA::GUIEventAdapter::KEY_Left)
        theme = "osgWidget/theme-2.png"
    ;

    else return false;

    osgWidget::Frame* frame = dynamic_cast<osgWidget::Frame*>(ev.getWindow());

    if(!frame) return false;

    // This is just one way to access all our Widgets; we could just as well have used:
    //
    // for(osgWidget::Frame::Iterator i = frame.begin(); i != frame.end() i++) {}
    //
    // ...and it have worked, too.
    for(unsigned int row = 0; row < 3; row++) {
        for(unsigned int col = 0; col < 3; col++) {
            frame->getByRowCol(row, col)->setImage(theme);
        }
    }

    return true;
}

int main(int, char**)
{
    osgViewer::Viewer viewer;

    osgWidget::WindowManager* wm = new osgWidget::WindowManager(
        &viewer,
        1280.0f,
        1024.0f,
        MASK_2D,
        osgWidget::WindowManager::WM_PICK_DEBUG
        //osgWidget::WindowManager::WM_NO_INVERT_Y
    );

    osgWidget::Frame* frame = osgWidget::Frame::createSimpleFrameFromTheme(
        "frame",
        osgDB::readRefImageFile("osgWidget/theme.png"),
        40.0f,
        40.0f,
	osgWidget::Frame::FRAME_ALL
    );

    frame->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.0f);

    // This is our Transformers box. :)
    osgWidget::Box*    box  = new osgWidget::Box("images", osgWidget::Box::VERTICAL);
    osgWidget::Widget* img1 = new osgWidget::Widget("im1", 512.0f, 512.0f);
    osgWidget::Widget* img2 = new osgWidget::Widget("im2", 512.0f, 512.0f);
    osgWidget::Widget* img3 = new osgWidget::Widget("im3", 512.0f, 512.0f);
    osgWidget::Widget* img4 = new osgWidget::Widget("im4", 512.0f, 512.0f);

    img1->setImage("osgWidget/scrolled1.jpg", true);
    img2->setImage("osgWidget/scrolled2.jpg", true);
    img3->setImage("osgWidget/scrolled3.jpg", true);
    img4->setImage("osgWidget/scrolled4.jpg", true);

    img1->setMinimumSize(10.0f, 10.0f);
    img2->setMinimumSize(10.0f, 10.0f);
    img3->setMinimumSize(10.0f, 10.0f);
    img4->setMinimumSize(10.0f, 10.0f);

    box->addWidget(img1);
    box->addWidget(img2);
    box->addWidget(img3);
    box->addWidget(img4);
    box->setEventMask(osgWidget::EVENT_NONE);

    //frame->getEmbeddedWindow()->setWindow(box);
    frame->setWindow(box);
    frame->getEmbeddedWindow()->setColor(1.0f, 1.0f, 1.0f, 1.0f);
    frame->resize(300.0f, 300.0f);
    frame->addCallback(new osgWidget::Callback(&scrollWindow, osgWidget::EVENT_MOUSE_SCROLL));
    frame->addCallback(new osgWidget::Callback(&changeTheme, osgWidget::EVENT_KEY_DOWN));

    wm->addChild(frame);

    return osgWidget::createExample(viewer, wm);
}
