// -*-c++-*- osgWidget - Copyright Cedric Pinson 2008


#include <osgWidget/Util>
#include <osgWidget/WindowManager>
#include <osgWidget/Frame>
#include <osgWidget/Box>
#include <osgDB/ReadFile>

const unsigned int MASK_2D = 0xF0000000;


osgWidget::Frame* createError(const std::string& theme, const std::string& text, int fontSize = 13)
{

    osg::ref_ptr<osgWidget::Frame> frame = osgWidget::Frame::createSimpleFrameFromTheme(
        "error",
        osgDB::readImageFile(theme),
        300.0f,
        50.0f,
        osgWidget::Frame::FRAME_ALL
        );
    frame->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.0f);

    osgWidget::Label* label = new osgWidget::Label("", "");
    label->setFont("fonts/Vera.ttf");
    label->setFontSize(fontSize);
    label->setFontColor(osgWidget::Color(0,0,0,1));
    label->setLabel(text);
    label->setCanFill(true);

    osgWidget::Box*   hbox   = new osgWidget::Box("HBOX", osgWidget::Box::HORIZONTAL);
    osgWidget::Box*   vbox   = new osgWidget::Box("VBOX", osgWidget::Box::VERTICAL);
    hbox->addWidget(label);
    hbox->attachScaleCallback();
//    hbox->addWidget(vbox);
    hbox->getBackground()->setColor(186/255.0, 186/255.0, 186/255.0,1);
    label->setColor(osgWidget::Color(0,0,0,0));
    frame->setWindow(hbox);
    hbox->resize();
    frame->resizeFrame(hbox->getWidth(), hbox->getHeight());
    return frame.release();
}


const char* LABEL1 =
    "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed\n"
    "do eiusmod tempor incididunt ut labore et dolore magna aliqua.\n"
    "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris\n"
    "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in..."
;

int main(int argc, char** argv) 
{
    std::string theme = "osgWidget/theme-1.png";
    if (argc > 1)
        theme = std::string(argv[1]);

    osgViewer::Viewer viewer;

    osgWidget::WindowManager* wm = new osgWidget::WindowManager(
        &viewer,
        1280.0f,
        1024.0f,
        MASK_2D,
        osgWidget::WindowManager::WM_PICK_DEBUG
    );

    osgWidget::Frame* frame = createError(theme, LABEL1);
    // Add everything to the WindowManager.
    wm->addChild(frame);
    frame->resizeAdd(30, 30);
    return osgWidget::createExample(viewer, wm);
}
