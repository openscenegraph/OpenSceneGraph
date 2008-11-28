// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgetlabel.cpp 66 2008-07-14 21:54:09Z cubicool $

#include <osg/io_utils>
#include <osgWidget/Util>
#include <osgWidget/WindowManager>
#include <osgWidget/Box>
#include <osgWidget/Label>

const unsigned int MASK_2D = 0xF0000000;

const char* LABEL1 =
    "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed\n"
    "do eiusmod tempor incididunt ut labore et dolore magna aliqua.\n"
    "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris\n"
    "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in..."
;

const char* LABEL2 = 
    "...reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla\n"
    "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in \n"
    "culpa qui officia deserunt mollit anim id est laborum. BBBBB"
;

osgWidget::Label* createLabel(const std::string& l, unsigned int size=13) {
    osgWidget::Label* label = new osgWidget::Label("", "");

    label->setFont("fonts/Vera.ttf");
    label->setFontSize(size);
    label->setFontColor(1.0f, 1.0f, 1.0f, 1.0f);
    label->setLabel(l);

    /*
    text->setBackdropType(osgText::Text::DROP_SHADOW_BOTTOM_RIGHT);
    text->setBackdropImplementation(osgText::Text::NO_DEPTH_BUFFER);
    text->setBackdropOffset(0.2f);
    */

    return label;
}

int main(int argc, char** argv) {
    osgViewer::Viewer viewer;

    osgWidget::WindowManager* wm = new osgWidget::WindowManager(
        &viewer,
        1280.0f,
        1024.0f,
        MASK_2D,
        // osgWidget::WindowManager::WM_USE_RENDERBINS |
        osgWidget::WindowManager::WM_PICK_DEBUG
    );
    
    osgWidget::Box*   box    = new osgWidget::Box("HBOX", osgWidget::Box::HORIZONTAL);
    osgWidget::Box*   vbox   = new osgWidget::Box("vbox", osgWidget::Box::VERTICAL);
    osgWidget::Label* label1 = createLabel(LABEL1);
    osgWidget::Label* label2 = createLabel(LABEL2);

    // Setup the labels for horizontal box.
    label1->setPadding(10.0f);
    label2->setPadding(10.0f);

    label1->addSize(21.0f, 22.0f);
    label2->addSize(21.0f, 22.0f);

    label1->setColor(1.0f, 0.5f, 0.0f, 0.0f);
    label2->setColor(1.0f, 0.5f, 0.0f, 0.5f);

    label2->setImage("Images/Brick-Norman-Brown.TGA", true);

    box->addWidget(label1);
    box->addWidget(label2);
    box->attachMoveCallback();
    box->attachScaleCallback();
    box->attachRotateCallback();

    // Setup the labels for the vertical box.
    osgWidget::Label* label3 = createLabel("Label 3", 80);
    osgWidget::Label* label4 = createLabel("Label 4", 60);
    osgWidget::Label* label5 = createLabel("ABCDEFGHIJK", 93);

    label3->setPadding(3.0f);
    label4->setPadding(3.0f);
    label5->setPadding(3.0f);

    label3->setColor(0.0f, 0.0f, 0.5f, 0.5f);
    label4->setColor(0.0f, 0.0f, 0.5f, 0.5f);
    label5->setColor(0.0f, 0.0f, 0.5f, 0.5f);
    
    //label5->setAlignHorizontal(osgWidget::Widget::HA_LEFT);
    //label5->setAlignVertical(osgWidget::Widget::VA_BOTTOM);

    // Test our label copy construction...
    osgWidget::Label* label6 = osg::clone(label5, "label6", osg::CopyOp::DEEP_COPY_ALL);

    label6->setLabel("abcdefghijklmnopqrs");

    vbox->addWidget(label3);
    vbox->addWidget(label4);
    vbox->addWidget(label5);
    vbox->addWidget(label6);
    vbox->attachMoveCallback();
    vbox->attachScaleCallback();

    vbox->resize();

    // vbox->setVisibilityMode(osgWidget::Window::VM_ENTIRE);
    // vbox->setVisibleArea(50, 50, 500, 200);
    // vbox->setAnchorVertical(osgWidget::Window::VA_TOP);
    // vbox->setAnchorHorizontal(osgWidget::Window::HA_RIGHT);

    // Test our label-in-window copy construction...
    osgWidget::Box* clonedBox = osg::clone(box, "HBOX-new", osg::CopyOp::DEEP_COPY_ALL);
    
    clonedBox->getBackground()->setColor(0.0f, 1.0f, 0.0f, 0.5f);

    wm->addChild(box);
    wm->addChild(vbox);
    wm->addChild(clonedBox);

    return osgWidget::createExample(viewer, wm);
}
