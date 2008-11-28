// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgetframe.cpp 40 2008-04-11 14:05:11Z cubicool $

#include <osgWidget/Util>
#include <osgWidget/WindowManager>
#include <osgWidget/Frame>
#include <osgWidget/Box>
#include <osgDB/ReadFile>

const unsigned int MASK_2D = 0xF0000000;

int main(int argc, char** argv) {
    osgViewer::Viewer viewer;

    osgWidget::WindowManager* wm = new osgWidget::WindowManager(
        &viewer,
        1280.0f,
        1024.0f,
        MASK_2D,
        osgWidget::WindowManager::WM_PICK_DEBUG
    );
    
    osgWidget::Frame* frame = osgWidget::Frame::createSimpleFrame(
        "frame",
        32.0f,
        32.0f,
        300.0f,
        300.0f
    );

    osgWidget::Frame* frame2 = osgWidget::Frame::createSimpleFrameFromTheme(
        "frameTheme",
        osgDB::readImageFile("osgWidget/theme-1.png"),
        300.0f,
        300.0f,
        osgWidget::Frame::FRAME_ALL
        );
    frame2->setPosition(300,100,0);
    frame2->getBackground()->setColor(1.0f, 1.0f, 1.0f, 0.0f);

    osgWidget::Frame* frame22 = osgWidget::Frame::createSimpleFrameFromTheme(
        "frameTheme",
        osgDB::readImageFile("osgWidget/theme-2.png"),
        300.0f,
        300.0f,
        osgWidget::Frame::FRAME_ALL
        );
    frame22->setPosition(300,100,0);
    frame22->getBackground()->setColor(1.0f, 1.0f, 1.0f, 0.0f);


    osgWidget::Frame* frame3 = osgWidget::Frame::createSimpleFrameFromTheme(
        "frameTheme",
        osgDB::readImageFile("osgWidget/theme-2.png"),
        300.0f,
        300.0f,
        osgWidget::Frame::FRAME_ALL
        );
    frame3->setPosition(300,100,0);
    frame3->getBackground()->setColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    osgWidget::Table* table  = new osgWidget::Table("table", 2, 2);
    osgWidget::Box*   bottom = new osgWidget::Box("panel", osgWidget::Box::HORIZONTAL);

    table->addWidget(new osgWidget::Widget("red", 300.0f, 300.0f), 0, 0);
    table->addWidget(new osgWidget::Widget("white", 300.0f, 300.0f), 0, 1);
    table->addWidget(new osgWidget::Widget("yellow", 300.0f, 300.0f), 1, 0);
    table->addWidget(new osgWidget::Widget("purple", 300.0f, 300.0f), 1, 1);
    table->getByRowCol(0, 0)->setColor(1.0f, 0.0f, 0.0f, 1.0f);
    table->getByRowCol(0, 1)->setColor(1.0f, 1.0f, 1.0f, 1.0f);
    table->getByRowCol(1, 0)->setColor(1.0f, 1.0f, 0.0f, 1.0f);
    table->getByRowCol(1, 1)->setColor(1.0f, 0.0f, 1.0f, 1.0f);
    table->getByRowCol(0, 0)->setMinimumSize(100.0f, 100.0f);
    table->getByRowCol(0, 1)->setMinimumSize(100.0f, 100.0f);
    table->getByRowCol(1, 0)->setMinimumSize(100.0f, 100.0f);
    table->getByRowCol(1, 1)->setMinimumSize(100.0f, 100.0f);

    frame->setWindow(table);

    // Give frame some nice textures.
    // TODO: This has to be done after setWindow(); wtf?
    frame->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.0f);

    osgWidget::Widget* l = frame->getBorder(osgWidget::Frame::BORDER_LEFT);
    osgWidget::Widget* r = frame->getBorder(osgWidget::Frame::BORDER_RIGHT);
    osgWidget::Widget* t = frame->getBorder(osgWidget::Frame::BORDER_TOP);
    osgWidget::Widget* b = frame->getBorder(osgWidget::Frame::BORDER_BOTTOM);

    l->setImage("osgWidget/border-left.tga", true);
    r->setImage("osgWidget/border-right.tga", true);
    t->setImage("osgWidget/border-top.tga", true);
    b->setImage("osgWidget/border-bottom.tga", true);

    l->setTexCoordWrapVertical();
    r->setTexCoordWrapVertical();
    t->setTexCoordWrapHorizontal();
    b->setTexCoordWrapHorizontal();

    // Create the bottom, XArt panel.
    osgWidget::Widget* left   = new osgWidget::Widget("left", 512.0f, 256.0f);
    osgWidget::Widget* center = new osgWidget::Widget("center", 256.0f, 256.0f);
    osgWidget::Widget* right  = new osgWidget::Widget("right", 512.0f, 256.0f);

    left->setImage("osgWidget/panel-left.tga", true);
    center->setImage("osgWidget/panel-center.tga", true);
    right->setImage("osgWidget/panel-right.tga", true);

    center->setTexCoordWrapHorizontal();

    bottom->addWidget(left);
    bottom->addWidget(center);
    bottom->addWidget(right);
    bottom->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.0f);
    bottom->setOrigin(0.0f, 1024.0f - 256.0f);

    // Add everything to the WindowManager.
    wm->addChild(frame);
    wm->addChild(frame2);
    wm->addChild(frame22);
    wm->addChild(frame3);
    wm->addChild(bottom);

    return osgWidget::createExample(viewer, wm);
}
