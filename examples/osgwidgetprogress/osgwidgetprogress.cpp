// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id$

#include <osgDB/ReadFile>
#include <osgWidget/Util>
#include <osgWidget/WindowManager>
#include <osgWidget/Canvas>

const unsigned int MASK_2D = 0xF0000000;

struct UpdateProgressNode: public osg::NodeCallback {
    float start;
    float done;

    UpdateProgressNode():
    start (0.0f),
    done  (5.0f) {
    }

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) {
        const osg::FrameStamp* fs = nv->getFrameStamp();

        float t = fs->getSimulationTime();

        if(start == 0.0f) start = t;

        float width   = ((t - start) / done) * 512.0f;
        float percent = (width / 512.0f) * 100.0f;
    
        if(width < 1.0f || width > 512.0f) return;

        osgWidget::Window* window = dynamic_cast<osgWidget::Window*>(node);

        if(!window) return;

        osgWidget::Widget* w = window->getByName("pMeter");
        osgWidget::Label*  l = dynamic_cast<osgWidget::Label*>(window->getByName("pLabel"));

        if(!w || !l) return;

        w->setWidth(width);
        w->setTexCoordRegion(0.0f, 0.0f, width, 64.0f);

        std::ostringstream ss;

        ss << osg::round(percent) << "% Done" << std::endl;

        l->setLabel(ss.str());
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
    
    osgWidget::Canvas* canvas   = new osgWidget::Canvas("canvas");
    osgWidget::Widget* pOutline = new osgWidget::Widget("pOutline", 512.0f, 64.0f);
    osgWidget::Widget* pMeter   = new osgWidget::Widget("pMeter", 0.0f, 64.0f);
    osgWidget::Label*  pLabel   = new osgWidget::Label("pLabel", "0% Done");

    pOutline->setImage("osgWidget/progress-outline.png", true);
    pOutline->setLayer(osgWidget::Widget::LAYER_MIDDLE, 2);
    
    pMeter->setImage("osgWidget/progress-meter.png");
    pMeter->setColor(0.7f, 0.1f, 0.1f, 0.7f);
    pMeter->setLayer(osgWidget::Widget::LAYER_MIDDLE, 1);

    pLabel->setFont("fonts/VeraMono.ttf");
    pLabel->setFontSize(20);
    pLabel->setFontColor(1.0f, 1.0f, 1.0f, 1.0f);
    pLabel->setSize(512.0f, 64.0f);
    pLabel->setLayer(osgWidget::Widget::LAYER_MIDDLE, 3);

    canvas->setOrigin(300.0f, 300.0f);
    canvas->addWidget(pMeter, 0.0f, 0.0f);
    canvas->addWidget(pOutline, 0.0f, 0.0f);
    canvas->addWidget(pLabel, 0.0f, 0.0f);
    canvas->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.0f);
    canvas->setUpdateCallback(new UpdateProgressNode());

    wm->addChild(canvas);

    return osgWidget::createExample(viewer, wm, osgDB::readNodeFile("cow.osgt"));
}
