/* OpenSceneGraph example, osgautotransform.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include <osg/Material>
#include <osg/Geode>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Projection>
#include <osg/AutoTransform>
#include <osg/Geometry>

#include <osgDB/WriteFile>

#include <osgText/Text>

#include <iostream>

osg::Node* createLabel(const osg::Vec3& pos, float size, const std::string& label, osgText::Text::AxisAlignment axisAlignment)
{
    osg::Geode* geode = new osg::Geode();
    
    std::string timesFont("fonts/arial.ttf");

    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );

        text->setFont(timesFont);
        text->setPosition(pos);
        text->setCharacterSize(size);
        text->setAxisAlignment(axisAlignment);
        text->setAlignment(osgText::Text::CENTER_CENTER);
        text->setText(label);
        
    }
    
    return geode;    
}


osg::Node* createLabel3(const osg::Vec3& pos, float size, const std::string& label)
{
    osg::Geode* geode = new osg::Geode();
    
    std::string timesFont("fonts/arial.ttf");

    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );

        text->setFont(timesFont);
        text->setPosition(pos);
        text->setFontResolution(40,40);
        text->setCharacterSize(size);
        text->setAlignment(osgText::Text::CENTER_CENTER);
        text->setAutoRotateToScreen(true);
        text->setCharacterSizeMode(osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);
        text->setText(label);
        
    }
    
    return geode;    
}

osg::Node* createAxis(const osg::Vec3& s, const osg::Vec3& e, int numReps, osg::AutoTransform::AutoRotateMode autoRotateMode, osgText::Text::AxisAlignment axisAlignment, const std::string& str)
{
    osg::Group* group = new osg::Group;

    osg::Vec3 dv = e-s;
    dv /= float(numReps-1);

    osg::Vec3 pos = s;

    bool useAuto = true;
    if (useAuto)
    {
        osg::Vec3Array* vertices = new osg::Vec3Array;

        for(int i=0;i<numReps;++i)
        {
            osg::AutoTransform* at = new osg::AutoTransform;
            at->setPosition(pos);
            at->setAutoRotateMode(autoRotateMode);
            at->addChild(createLabel(osg::Vec3(0.0f,0.0f,0.0f),dv.length()*0.2f,str, axisAlignment));
            vertices->push_back(pos);
            pos += dv;


            group->addChild(at);
        }

        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

        osg::Geometry* geom = new osg::Geometry;
        geom->setVertexArray(vertices);
        geom->setColorArray(colors);
        geom->setColorBinding(osg::Geometry::BIND_OVERALL);
        geom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP,0,vertices->size()));

        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(geom);

        group->addChild(geode);
    }
    else
    {
        osg::Vec3Array* vertices = new osg::Vec3Array;

        for(int i=0;i<numReps;++i)
        {
            group->addChild(createLabel3(osg::Vec3(pos),dv.length()*0.5f,str));
            vertices->push_back(pos);
            pos += dv;


        }

        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

        osg::Geometry* geom = new osg::Geometry;
        geom->setVertexArray(vertices);
        geom->setColorArray(colors);
        geom->setColorBinding(osg::Geometry::BIND_OVERALL);
        geom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP,0,vertices->size()));

        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(geom);

        group->addChild(geode);
    }
    
    return group;
}

osg::Node* createAutoScale(const osg::Vec3& position, float characterSize, const std::string& message, float minScale=0.0, float maxScale=FLT_MAX)
{
    std::string timesFont("fonts/arial.ttf");

    osgText::Text* text = new osgText::Text;
    text->setCharacterSize(characterSize);
    text->setText(message);
    text->setFont(timesFont);
    text->setAlignment(osgText::Text::CENTER_CENTER);

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(text);
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    osg::AutoTransform* at = new osg::AutoTransform;
    at->addChild(geode);
    
    at->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
    at->setAutoScaleToScreen(true);
    at->setMinimumScale(minScale);
    at->setMaximumScale(maxScale);
    at->setPosition(position);
    
    return at;
}

osg::Node* createScene()
{
    osg::Group* root = new osg::Group;
    
//    int numReps = 3333;
    int numReps = 10;
    root->addChild(createAxis(osg::Vec3(0.0,0.0,0.0),osg::Vec3(1000.0,0.0,0.0),numReps,osg::AutoTransform::ROTATE_TO_CAMERA,osgText::Text::XY_PLANE, "ROTATE_TO_CAMERA"));
    root->addChild(createAxis(osg::Vec3(0.0,0.0,0.0),osg::Vec3(0.0,1000.0,0.0),numReps,osg::AutoTransform::ROTATE_TO_SCREEN,osgText::Text::XY_PLANE, "ROTATE_TO_SCREEN"));
    root->addChild(createAxis(osg::Vec3(0.0,0.0,0.0),osg::Vec3(0.0,0.0,1000.0),numReps,osg::AutoTransform::NO_ROTATION,osgText::Text::XZ_PLANE, "NO_ROTATION"));    

    root->addChild(createAutoScale(osg::Vec3(500.0,500.0,500.0), 25.0, "AutoScale with no min, max limits"));
    root->addChild(createAutoScale(osg::Vec3(500.0,500.0,300.0), 25.0, "AutoScale with minScale = 1, maxScale = 2.0 ", 1, 2.0));
    root->addChild(createAutoScale(osg::Vec3(500.0,500.0,700.0), 25.0, "AutoScale with minScale = 0.0, maxScale = 5.0 ", 0.0, 5.0));
    return root;
}

int main(int, char**)
{
    // construct the viewer.
    osgViewer::Viewer viewer;

    // set the scene to render
    viewer.setSceneData(createScene());
    
    // run the viewers frame loop
    return viewer.run();
}
