// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgetmenu.cpp 66 2008-07-14 21:54:09Z cubicool $

#include <iostream>
#include <osgDB/ReadFile>
#include <osgWidget/Util>
#include <osgWidget/WindowManager>
#include <osgWidget/Box>
#include <osgWidget/Label>

// For now this is just an example, but osgWidget::Menu will later be it's own Window.
// I just wanted to get this out there so that people could see it was possible.

const unsigned int MASK_2D = 0xF0000000;
const unsigned int MASK_3D = 0x0F000000;

struct ColorLabel: public osgWidget::Label {
    ColorLabel(const char* label):
    osgWidget::Label("", "") {
        setFont("fonts/Vera.ttf");
        setFontSize(14);
        setFontColor(1.0f, 1.0f, 1.0f, 1.0f);
        setColor(0.3f, 0.3f, 0.3f, 1.0f);
        addHeight(18.0f);
        setCanFill(true);
        setLabel(label);
        setEventMask(osgWidget::EVENT_MOUSE_PUSH | osgWidget::EVENT_MASK_MOUSE_MOVE);
    }

    bool mousePush(double, double, osgWidget::WindowManager*) {
        return true;
    }

    bool mouseEnter(double, double, osgWidget::WindowManager*) {
        setColor(0.6f, 0.6f, 0.6f, 1.0f);
        
        return true;
    }

    bool mouseLeave(double, double, osgWidget::WindowManager*) {
        setColor(0.3f, 0.3f, 0.3f, 1.0f);
        
        return true;
    }
};

class ColorLabelMenu: public ColorLabel {
    osg::ref_ptr<osgWidget::Window> _window;

public:
    ColorLabelMenu(const char* label):
    ColorLabel(label) {
        _window = new osgWidget::Box(
            std::string("Menu_") + label,
            osgWidget::Box::VERTICAL,
            true
        );

        _window->addWidget(new ColorLabel("Open Some Stuff"));
        _window->addWidget(new ColorLabel("Do It Now"));
        _window->addWidget(new ColorLabel("Hello, How Are U?"));
        _window->addWidget(new ColorLabel("Hmmm..."));
        _window->addWidget(new ColorLabel("Option 5"));

        _window->resize();

        setColor(0.8f, 0.8f, 0.8f, 0.8f);
    }

    void managed(osgWidget::WindowManager* wm) {
        osgWidget::Label::managed(wm);

        wm->addChild(_window.get());

        _window->hide();
    }

    void positioned() {
        osgWidget::Label::positioned();

        _window->setOrigin(getX(), getHeight());
        _window->resize(getWidth());
    }

    bool mousePush(double, double, osgWidget::WindowManager*) {
        if(!_window->isVisible()) _window->show();

        else _window->hide();

        return true;
    }

    bool mouseLeave(double, double, osgWidget::WindowManager*) {
        if(!_window->isVisible()) setColor(0.8f, 0.8f, 0.8f, 0.8f);

        return true;
    }
};

int main(int argc, char** argv) {
    osgViewer::Viewer viewer;

    osgWidget::WindowManager* wm = new osgWidget::WindowManager(
        &viewer,
        1280.0f,
        1024.0f,
        MASK_2D,
        osgWidget::WindowManager::WM_PICK_DEBUG
    );

    osgWidget::Window* menu = new osgWidget::Box("menu", osgWidget::Box::HORIZONTAL);

    menu->addWidget(new ColorLabelMenu("Pick me!"));
    menu->addWidget(new ColorLabelMenu("No, wait, pick me!"));
    menu->addWidget(new ColorLabelMenu("Dont pick them..."));
    menu->addWidget(new ColorLabelMenu("Grarar!?!"));

    wm->addChild(menu);
    
    menu->getBackground()->setColor(1.0f, 1.0f, 1.0f, 0.0f);
    menu->resizePercent(100.0f);

    osg::Node* model = osgDB::readNodeFile("osgcool.osg");

    model->setNodeMask(MASK_3D);

    return osgWidget::createExample(viewer, wm, model);
}
