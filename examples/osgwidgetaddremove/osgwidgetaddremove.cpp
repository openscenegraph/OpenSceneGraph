// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgetaddremove.cpp 45 2008-04-23 16:46:11Z cubicool $

#include <osgWidget/Util>
#include <osgWidget/WindowManager>
#include <osgWidget/Table>
#include <osgWidget/Box>
#include <osgWidget/Label>

const unsigned int MASK_2D = 0xF0000000;

class ABCWidget: public osgWidget::Label {
public:
    ABCWidget(const std::string& label):
    osgWidget::Label("", label) {
        setFont("fonts/Vera.ttf");
        setFontSize(20);
        setCanFill(true);
        setShadow(0.08f);
        addSize(10.0f, 10.0f);
    }
};

class Button: public osgWidget::Label {
public:
    Button(const std::string& label):
    osgWidget::Label("", label) {
        setFont("fonts/Vera.ttf");
        setFontSize(30);
        setColor(0.8f, 0.2f, 0.2f, 0.8f);
        setCanFill(true);
        setShadow(0.1f);
        setEventMask(osgWidget::EVENT_MASK_MOUSE_CLICK);
        addSize(20.0f, 20.0f);
    }

    // NOTE! I need to make it clearer than Push/Release can happen so fast that
    // the changes you make aren't visible with your refresh rate. Throttling state
    // changes and what-have-you on mousePush/mouseRelease/etc. is going to be
    // annoying...

    virtual bool mousePush(double, double, osgWidget::WindowManager*) {
        addColor(0.2f, 0.2f, 0.2f, 0.0f);
        
        return true;
    }

    virtual bool mouseRelease(double, double, osgWidget::WindowManager*) {
        addColor(-0.2f, -0.2f, -0.2f, 0.0f);
        
        return true;
    }
};

class AddRemove: public osgWidget::Box {
    osg::ref_ptr<osgWidget::Window> _win1;

public:
    AddRemove():
    osgWidget::Box ("buttons", osgWidget::Box::VERTICAL),
    _win1          (new osgWidget::Box("win1", osgWidget::Box::VERTICAL)) {
        addWidget(new Button("Add Widget"));
        addWidget(new Button("Remove Widget"));

        // Take special note here! Not only do the Button objects have their
        // own overridden methods for changing the color, but they have attached
        // callbacks for doing the work with local data.
        getByName("Widget_1")->addCallback(new osgWidget::Callback(
            &AddRemove::handlePressAdd,
            this,
            osgWidget::EVENT_MOUSE_PUSH
        ));

        getByName("Widget_2")->addCallback(new osgWidget::Callback(
            &AddRemove::handlePressRemove,
            this,
            osgWidget::EVENT_MOUSE_PUSH
        ));
    }

    virtual void managed(osgWidget::WindowManager* wm) {
        osgWidget::Box::managed(wm);

        _win1->setOrigin(250.0f, 0.0f);

        wm->addChild(_win1.get());
    }

    bool handlePressAdd(osgWidget::Event& ev) {
        static unsigned int num = 0;

        std::stringstream ss;

        ss << "a random widget " << num;

        _win1->addWidget(new ABCWidget(ss.str()));

        num++;

        return true;
    }

    bool handlePressRemove(osgWidget::Event& ev) {
        // TODO: Temporary hack!
        const osgWidget::Box::Vector& v = _win1->getObjects();
    
        if(!v.size()) return false;

        osgWidget::Widget* w = _win1->getObjects()[v.size() - 1].get();

        _win1->removeWidget(w);

        return true;
    }
};

int main(int argc, char** argv) {
    osgViewer::Viewer viewer;

    osgWidget::WindowManager* wm = new osgWidget::WindowManager(
        &viewer,
        1280.0f,
        1024.0f,
        MASK_2D
    );
    
    osgWidget::Box* buttons = new AddRemove();

    wm->addChild(buttons);

    return createExample(viewer, wm);
}
