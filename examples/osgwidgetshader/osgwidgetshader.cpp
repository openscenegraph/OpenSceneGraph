// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgetshader.cpp 28 2008-03-26 15:26:48Z cubicool $

#include <osgDB/FileUtils>
#include <osgWidget/Util>
#include <osgWidget/WindowManager>
#include <osgWidget/Canvas>

const unsigned int MASK_2D = 0xF0000000;

osgWidget::Widget* createWidget(
    const std::string&       name,
    osgWidget::color_type    col,
    osgWidget::Widget::Layer layer
) {
    osgWidget::Widget* widget = new osgWidget::Widget(name, 200.0f, 200.0f);

    widget->setColor(col, col, col, 0.2f);
    widget->setLayer(layer);

    return widget;
}

int main(int argc, char** argv) {
    osgViewer::Viewer viewer;

    osgWidget::WindowManager* wm = new osgWidget::WindowManager(
        &viewer,
        1280.0f,
        1024.0f,
        MASK_2D
    );
    
    osgWidget::Canvas* canvas = new osgWidget::Canvas("canvas");

    canvas->attachMoveCallback();
    canvas->attachScaleCallback();

    canvas->addWidget(
        createWidget("w1", 0.2f, osgWidget::Widget::LAYER_LOW),
        0.0f,
        0.0f
    );
    
    canvas->addWidget(
        createWidget("w2", 0.4f, osgWidget::Widget::LAYER_MIDDLE),
        200.0f,
        0.0f
    );

    canvas->addWidget(
        createWidget("w3", 0.6f, osgWidget::Widget::LAYER_HIGH),
        400.0f,
        0.0f
    );


    wm->addChild(canvas);

    osg::Program* program = new osg::Program();

    program->addShader(osg::Shader::readShaderFile(
        osg::Shader::VERTEX,
        osgDB::findDataFile("osgWidget/osgwidgetshader-vert.glsl")
    ));
    
    program->addShader(osg::Shader::readShaderFile(
        osg::Shader::FRAGMENT,
        osgDB::findDataFile("osgWidget/osgwidgetshader-frag.glsl")
    ));

    canvas->getGeode()->getOrCreateStateSet()->setAttribute(program);

    return osgWidget::createExample(viewer, wm);
}
