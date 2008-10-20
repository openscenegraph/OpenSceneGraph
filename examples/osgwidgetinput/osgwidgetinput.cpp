// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgetinput.cpp 50 2008-05-06 05:06:36Z cubicool $

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgWidget/Util>
#include <osgWidget/WindowManager>
#include <osgWidget/Box>
#include <osgWidget/Table>
#include <osgWidget/Frame>
#include <osgWidget/Label>
#include <osgWidget/Input>

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
    
    osgWidget::Box*   box   = new osgWidget::Box("vbox", osgWidget::Box::VERTICAL);
    osgWidget::Input* input = new osgWidget::Input("input", "", 50);

    input->setFont("fonts/AMERSN__.ttf");
    input->setFontSize(30);
    input->setYOffset(input->calculateBestYOffset("y"));
    input->setSize(400.0f, input->getText()->getCharacterHeight());

    box->addWidget(input);
    box->setOrigin(200.0f, 200.0f);

    wm->addChild(box);

    return osgWidget::createExample(viewer, wm);
}
