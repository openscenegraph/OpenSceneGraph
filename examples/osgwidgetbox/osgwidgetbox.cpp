// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgetbox.cpp 59 2008-05-15 20:55:31Z cubicool $

// NOTE: You'll find this example very similar to osgwidgetwindow. However, here we
// demonstrate a bit of subclassing of Widget so that we can respond to events
// such as mouseEnter and mouseLeave. We also demonstrate the use of padding, though
// fill and alignment should be working too.

#include <osg/io_utils>
#include <osgDB/ReadFile>
#include <osgWidget/Util>
#include <osgWidget/WindowManager>
#include <osgWidget/Box>

const unsigned int MASK_2D = 0xF0000000;
const unsigned int MASK_3D = 0x0F000000;

struct ColorWidget: public osgWidget::Widget {
    ColorWidget():
    osgWidget::Widget("", 256.0f, 256.0f) {
        setEventMask(osgWidget::EVENT_ALL);
    }

    bool mouseEnter(double, double, const osgWidget::WindowManager*) {
        addColor(-osgWidget::Color(0.4f, 0.4f, 0.4f, 0.0f));

        // osgWidget::warn() << "enter: " << getColor() << std::endl;

        return true;
    }

    bool mouseLeave(double, double, const osgWidget::WindowManager*) {
        addColor(osgWidget::Color(0.4f, 0.4f, 0.4f, 0.0f));

        // osgWidget::warn() << "leave: " << getColor() << std::endl;

        return true;
    }

    bool mouseOver(double x, double y, const osgWidget::WindowManager*) {

        osgWidget::Color c = getImageColorAtPointerXY(x, y);

        if(c.a() < 0.001f) {
            // osgWidget::warn() << "Transparent Pixel: " << x << " " << y << std::endl;

            return false;
        }
        return true;
    }

    bool keyUp(int /*key*/, int /*keyMask*/, osgWidget::WindowManager*) {
        // osgWidget::warn() << "..." << key << " - " << keyMask << std::endl;

        return true;
    }
};

osgWidget::Box* createBox(const std::string& name, osgWidget::Box::BoxType bt) {
    osgWidget::Box*    box     = new osgWidget::Box(name, bt, true);
    osgWidget::Widget* widget1 = new osgWidget::Widget(name + "_widget1", 100.0f, 100.0f);
    osgWidget::Widget* widget2 = new osgWidget::Widget(name + "_widget2", 100.0f, 100.0f);
    osgWidget::Widget* widget3 = new ColorWidget();

    widget1->setColor(0.3f, 0.3f, 0.3f, 1.0f);
    widget2->setColor(0.6f, 0.6f, 0.6f, 1.0f);

    widget3->setImage("osgWidget/natascha.png");
    widget3->setTexCoord(0.0f, 0.0f, osgWidget::Widget::LOWER_LEFT);
    widget3->setTexCoord(1.0f, 0.0f, osgWidget::Widget::LOWER_RIGHT);
    widget3->setTexCoord(1.0f, 1.0f, osgWidget::Widget::UPPER_RIGHT);
    widget3->setTexCoord(0.0f, 1.0f, osgWidget::Widget::UPPER_LEFT);

    box->addWidget(widget1);
    box->addWidget(widget2);
    box->addWidget(widget3);

    return box;
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
    );

    wm->setPointerFocusMode(osgWidget::WindowManager::PFM_SLOPPY);

    osgWidget::Window* box1 = createBox("HBOX", osgWidget::Box::HORIZONTAL);
    osgWidget::Window* box2 = createBox("VBOX", osgWidget::Box::VERTICAL);
    osgWidget::Window* box3 = createBox("HBOX2", osgWidget::Box::HORIZONTAL);
    osgWidget::Window* box4 = createBox("VBOX2", osgWidget::Box::VERTICAL);

    box1->getBackground()->setColor(1.0f, 0.0f, 0.0f, 0.8f);
    box1->attachMoveCallback();

    box2->getBackground()->setColor(0.0f, 1.0f, 0.0f, 0.8f);
    box2->attachMoveCallback();

    box3->getBackground()->setColor(0.0f, 0.0f, 1.0f, 0.8f);
    box3->attachMoveCallback();

    wm->addChild(box1);
    wm->addChild(box2);
    wm->addChild(box3);
    wm->addChild(box4);

    box4->hide();

    osg::ref_ptr<osg::Node> model = osgDB::readRefNodeFile("spaceship.osgt");

    model->setNodeMask(MASK_3D);

    return osgWidget::createExample(viewer, wm, model.get());
}
