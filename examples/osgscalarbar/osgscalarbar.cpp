/* OpenSceneGraph example, osgscalarbar.
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

#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/BlendFunc>
#include <osg/ClearNode>
#include <osg/Projection>

#include <osgUtil/CullVisitor>

#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>
#include <osgDB/ReadFile>

#include <osgSim/ScalarsToColors>
#include <osgSim/ColorRange>
#include <osgSim/ScalarBar>

#include <sstream>
#include <iostream>
#include <math.h>

using namespace osgSim;
using osgSim::ScalarBar;

#if defined(_MSC_VER)
// not have to have this pathway for just VS6.0 as its unable to handle the full
// ScalarBar::ScalarPrinter::printScalar scoping.

// Create a custom scalar printer
struct MyScalarPrinter: public ScalarBar::ScalarPrinter
{
    std::string printScalar(float scalar)
    {
        std::cout<<"In MyScalarPrinter::printScalar"<<std::endl;
        if(scalar==0.0f) return ScalarPrinter::printScalar(scalar)+" Bottom";
        else if(scalar==0.5f) return ScalarPrinter::printScalar(scalar)+" Middle";
        else if(scalar==1.0f) return ScalarPrinter::printScalar(scalar)+" Top";
        else return ScalarPrinter::printScalar(scalar);
    }
};
#else
// Create a custom scalar printer
struct MyScalarPrinter: public ScalarBar::ScalarPrinter
{
    std::string printScalar(float scalar)
    {
        std::cout<<"In MyScalarPrinter::printScalar"<<std::endl;
        if(scalar==0.0f) return ScalarBar::ScalarPrinter::printScalar(scalar)+" Bottom";
        else if(scalar==0.5f) return ScalarBar::ScalarPrinter::printScalar(scalar)+" Middle";
        else if(scalar==1.0f) return ScalarBar::ScalarPrinter::printScalar(scalar)+" Top";
        else return ScalarBar::ScalarPrinter::printScalar(scalar);
    }
};
#endif

osg::Node* createScalarBar(bool vertical)
{
#if 1
    //ScalarsToColors* stc = new ScalarsToColors(0.0f,1.0f);
    //ScalarBar* sb = new ScalarBar(2,3,stc,"STC_ScalarBar");

    // Create a custom color set
    std::vector<osg::Vec4> cs;
    cs.push_back(osg::Vec4(1.0f,0.0f,0.0f,1.0f));   // R
    cs.push_back(osg::Vec4(0.0f,1.0f,0.0f,1.0f));   // G
    cs.push_back(osg::Vec4(1.0f,1.0f,0.0f,1.0f));   // G
    cs.push_back(osg::Vec4(0.0f,0.0f,1.0f,1.0f));   // B
    cs.push_back(osg::Vec4(0.0f,1.0f,1.0f,1.0f));   // R


    ColorRange* cr = new ColorRange(0.0f,1.0f,cs);
    ScalarBar* sb = new ScalarBar(20, 11, cr,
                      vertical ? "Vertical" : "Horizontal",
                      vertical ? ScalarBar::VERTICAL : ScalarBar::HORIZONTAL,
                      0.1f, new MyScalarPrinter);
    sb->setScalarPrinter(new MyScalarPrinter);

    if ( !vertical )
    {
        sb->setPosition( osg::Vec3(0.5f,0.5f,0));
    }

    return sb;
#else
    ScalarBar *sb = new ScalarBar;
    ScalarBar::TextProperties tp;
    tp._fontFile = "fonts/times.ttf";

    sb->setTextProperties(tp);

    return sb;
#endif

}

osg::Node * createScalarBar_HUD()
{
    osgSim::ScalarBar * geode = new osgSim::ScalarBar;
    osgSim::ScalarBar::TextProperties tp;
    tp._fontFile = "fonts/times.ttf";
    geode->setTextProperties(tp);
    osg::StateSet * stateset = geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    stateset->setRenderBinDetails(11, "RenderBin");

    osg::MatrixTransform * modelview = new osg::MatrixTransform;
    modelview->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    osg::Matrixd matrix(osg::Matrixd::scale(1000,1000,1000) * osg::Matrixd::translate(120,10,0)); // I've played with these values a lot and it seems to work, but I have no idea why
    modelview->setMatrix(matrix);
    modelview->addChild(geode);

    osg::Projection * projection = new osg::Projection;
    projection->setMatrix(osg::Matrix::ortho2D(0,1280,0,1024)); // or whatever the OSG window res is
    projection->addChild(modelview);

    return projection; //make sure you delete the return sb line
}

int main(int , char **)
{
    // construct the viewer.
    osgViewer::Viewer viewer;

    osg::Group* group = new osg::Group;

    group->addChild(createScalarBar_HUD());

    // rotate the scalar from XY plane to XZ so we see them viewing it with the default camera manipulators that look along the Y axis, with Z up.
    osg::MatrixTransform* transform = new osg::MatrixTransform;
    group->addChild(transform);
    transform->setMatrix(osg::Matrix::rotate(osg::inDegrees(90.0),1.0,0.0,0.0));
    transform->addChild(createScalarBar(true));
    transform->addChild(createScalarBar(false));

    // add model to viewer.
    viewer.setSceneData( group );

    return viewer.run();
}
