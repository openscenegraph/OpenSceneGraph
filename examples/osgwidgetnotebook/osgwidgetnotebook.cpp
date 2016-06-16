// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgetnotebook.cpp 45 2008-04-23 16:46:11Z cubicool $

#include <osg/io_utils>
#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgWidget/Util>
#include <osgWidget/WindowManager>
#include <osgWidget/Box>
#include <osgWidget/Canvas>
#include <osgWidget/Label>
#include <osgWidget/Label>
#include <osgWidget/ViewerEventHandlers>

const unsigned int MASK_2D = 0xF0000000;

class Notebook: public osgWidget::Box {
    osg::ref_ptr<osgWidget::Box>    _tabs;
    osg::ref_ptr<osgWidget::Canvas> _windows;

public:
    // NOTE: This whole thing is just a hack to demonstrate a concept. The real
    // implementation would need to be much cleaner.
    bool callbackTabPressed(osgWidget::Event& ev) {
        osgWidget::Canvas::Vector& objs = _windows->getObjects();

        for(unsigned int i = 0; i < objs.size(); i++) objs[i]->setLayer(
            osgWidget::Widget::LAYER_MIDDLE,
            i * 2
        );

        _windows->getByName(ev.getWidget()->getName())->setLayer(
            osgWidget::Widget::LAYER_MIDDLE,
            objs.size() * 2
        );

        _windows->resize();

        return true;
    }

    Notebook(const std::string& name):
    osgWidget::Box(name, osgWidget::Box::VERTICAL) {
        _tabs    = new osgWidget::Box("tabs", osgWidget::Box::HORIZONTAL);
        _windows = new osgWidget::Canvas("canvas");

        for(unsigned int i = 0; i < 4; i++) {
            std::stringstream ss;

            // Setup everything for our Tab...
            ss << "Tab_" << i;

            osgWidget::Label* label1 = new osgWidget::Label(ss.str());

            label1->setFont("fonts/VeraMono.ttf");
            label1->setFontSize(20);
            label1->setFontColor(1.0f, 1.0f, 1.0f, 1.0f);
            label1->setColor(0.0f, i / 4.0f, 0.3f, 1.0f);
            label1->setLabel(ss.str());
            label1->addSize(20.0f, 20.0f);
            label1->setShadow(0.1f);
            label1->setCanFill(true);

            _tabs->addWidget(label1);

            // Setup everything for the Window corresponding to the Tab
            // in the Canvas down below.
            std::stringstream descr;

            descr
                << "This is some text" << std::endl
                << "for the Tab_" << i << " tab." << std::endl
                << "Press the button up top" << std::endl
                << "And this should go to the next Window!" << std::endl
            ;

            osgWidget::Label* label2 = new osgWidget::Label(ss.str());

            label2->setFont("fonts/Vera.ttf");
            label2->setFontSize(15);
            label2->setFontColor(1.0f, 1.0f, 1.0f, 1.0f);
            label2->setColor(0.0f, i / 4.0f, 0.3f, 1.0f);
            label2->setLabel(descr.str());
            label2->setLayer(osgWidget::Widget::LAYER_MIDDLE, i * 2);
            label2->addSize(50.0f, 50.0f);

            _windows->addWidget(label2, 0.0f, 0.0f);

            label1->setEventMask(osgWidget::EVENT_MOUSE_PUSH);
            label1->addCallback(new osgWidget::Callback(
                &Notebook::callbackTabPressed,
                this,
                osgWidget::EVENT_MOUSE_PUSH
            ));
        }

        osgWidget::Label* label = new osgWidget::Label("label");

        label->setFont("fonts/arial.ttf");
        label->setFontSize(15);
        label->setFontColor(1.0f, 1.0f, 1.0f, 1.0f);
        label->setLabel("Drag the window here...");
        label->addSize(20.0f, 20.0f);
        label->setShadow(0.08f);
        label->setCanFill(true);

        addWidget(label);
        addWidget(_tabs->embed());
        addWidget(_windows->embed());
    }
};

void bound(osg::Node* node) {
    osg::BoundingSphere bs = node->getBound();

    osgWidget::warn() << "center: " << bs.center() << " radius: " << bs.radius() << std::endl;
}

int main(int, char**)
{
    osgViewer::Viewer viewer;

    osgWidget::WindowManager* wm = new osgWidget::WindowManager(
        &viewer,
        1280.0f,
        720.0f,
        MASK_2D //,
        //osgWidget::WindowManager::WM_USE_RENDERBINS
    );

    Notebook* notebook1 = new Notebook("notebook1");
    Notebook* notebook2 = new Notebook("notebook2");

    notebook2->setOrigin(100.0f, 100.0f);

    notebook1->attachMoveCallback();
    notebook2->attachMoveCallback();

    wm->addChild(notebook1);
    wm->addChild(notebook2);

    return osgWidget::createExample(viewer, wm);
}
