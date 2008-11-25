// -*-c++-*- osgWidget - Copyright Cedric Pinson 2008


#include <osgWidget/Util>
#include <osgWidget/WindowManager>
#include <osgWidget/Frame>
#include <osgWidget/Box>
#include <osgWidget/Widget>
#include <osgWidget/Types>
#include <osgDB/ReadFile>

const unsigned int MASK_2D = 0xF0000000;


struct AlphaSetterVisitor : public osg::NodeVisitor
{
    float _alpha;
    AlphaSetterVisitor( float alpha = 1.0):osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) { _alpha = alpha;}

    void apply(osg::MatrixTransform& node)
    {
        osgWidget::Window* win = dynamic_cast<osgWidget::Window*>(&node);

        if (win) {
            osgWidget::warn() << "I am in Window: " << win->getName() << std::endl;

            for (osgWidget::Window::Iterator it = win->begin(); it != win->end(); it++)
            {
                osgWidget::warn() << "   I am operating on Widget: " << it->get()->getName() << std::endl;
                
                osgWidget::Color color = it->get()->getColor();
                color[3] = color[3] *_alpha;
                it->get()->setColor(color);
            }
            {
                osgWidget::Color color = win->getBackground()->getColor();
                color[3] = color[3] *_alpha;
                win->getBackground()->setColor(color);
            }
        }
        traverse(node);
    }
};


osgWidget::Window* createButtonOk(const std::string& theme, const std::string& text, int fontSize = 13)
{
    osg::ref_ptr<osgWidget::Frame> frame = osgWidget::Frame::createSimpleFrameFromTheme(
        "ButtonOK",
        osgDB::readImageFile(theme),
        300.0f, 
        50.0f,
        osgWidget::Frame::FRAME_TEXTURE
        );
    frame->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.0f);

    osgWidget::Label* label = new osgWidget::Label("", "");
    label->setFont("fonts/Vera.ttf");
    label->setFontSize(fontSize);
    label->setFontColor(osgWidget::Color(0,0,0,1));
    label->setColor(osgWidget::Color(0,0,0,0));
    label->setLabel(text);
    label->setCanFill(true);

    osgWidget::Box* box = new osgWidget::Box("HBOX", osgWidget::Box::HORIZONTAL);
    box->addWidget(label);
    box->resize();
    box->getBackground()->setColor(186/255.0, 186/255.0, 186/255.0,1);
    frame->getEmbeddedWindow()->setWindow(box);

    frame->resizeFrame(box->getWidth(), box->getHeight());
    frame->resizeAdd(0, 0);
    return frame.release();
}

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

    osgWidget::Box*   vbox   = new osgWidget::Box("HBOX", osgWidget::Box::HORIZONTAL);
    osgWidget::Box*   box   = new osgWidget::Box("VBOX", osgWidget::Box::VERTICAL);

    std::string theme2 = "osgWidget/theme-4.png";
    osgWidget::Widget* buttonOK = createButtonOk(theme2,"Ok")->embed();
    buttonOK->setColor(osgWidget::Color(0,0,0,0));
    buttonOK->setCanFill(false);
    box->addWidget(buttonOK);
    box->addWidget(label);
    box->attachScaleCallback();
    box->getBackground()->setColor(186/255.0, 186/255.0, 186/255.0,1);
    label->setColor(osgWidget::Color(0,0,0,0));
    frame->setWindow(box);


    box->resize();
    frame->resizeFrame(box->getWidth(), box->getHeight());
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

    AlphaSetterVisitor alpha(.8f);
    frame->accept(alpha);
    return osgWidget::createExample(viewer, wm, osgDB::readNodeFile("cow.osg"));
}
