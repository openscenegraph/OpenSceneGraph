/* OpenSceneGraph example, osgdepthpartion.
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

#include <osgUtil/UpdateVisitor>

#include <osgDB/ReadFile>

#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>

#include <osgGA/TrackballManipulator>

#include <osgViewer/Viewer>

#include "DepthPartitionNode.h"

const double r_earth = 6378.137;
const double r_sun = 695990.0;
const double AU = 149697900.0;

osg::Node* createScene()
{
    // Create the Earth, in blue
    osg::ShapeDrawable *earth_sd = new osg::ShapeDrawable;
    osg::Sphere* earth_sphere = new osg::Sphere;
    earth_sphere->setRadius(r_earth);
    earth_sd->setShape(earth_sphere);
    earth_sd->setColor(osg::Vec4(0, 0, 1.0, 1.0));

    osg::Geode* earth = new osg::Geode;
    earth->setName("earth");
    earth->addDrawable(earth_sd);

    // Create the Sun, in yellow
    osg::ShapeDrawable *sun_sd = new osg::ShapeDrawable;
    osg::Sphere* sun_sphere = new osg::Sphere;
    sun_sphere->setRadius(r_sun);
    sun_sd->setShape(sun_sphere);
    sun_sd->setColor(osg::Vec4(1.0, 0.0, 0.0, 1.0));

    osg::Geode* sun_geode = new osg::Geode;
    sun_geode->setName("sun");
    sun_geode->addDrawable(sun_sd);

    // Move the sun behind the earth
    osg::PositionAttitudeTransform *pat = new osg::PositionAttitudeTransform;
    pat->setPosition(osg::Vec3d(0.0, AU, 0.0));

    osg::Group* scene = new osg::Group;
    scene->addChild(earth);
    scene->addChild(pat);
    pat->addChild(sun_geode);

    return scene;
}

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // construct the viewer.
    osgViewer::Viewer viewer;

    bool needToSetHomePosition = false;

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles(arguments);

    // if one hasn't been loaded create an earth and sun test model.
    if (!scene) 
    {
        scene = createScene(); 
        needToSetHomePosition = true;
    }
    
    // Create a DepthPartitionNode to manage partitioning of the scene
    osg::ref_ptr<DepthPartitionNode> dpn = new DepthPartitionNode;
    dpn->addChild(scene.get());
    dpn->setActive(true); // Control whether the node analyzes the scene
        
    // pass the loaded scene graph to the viewer.
    viewer.setSceneData(dpn.get());

    viewer.setCameraManipulator(new osgGA::TrackballManipulator);

    if (needToSetHomePosition)
    {
        viewer.getCameraManipulator()->setHomePosition(osg::Vec3d(0.0,-5.0*r_earth,0.0),osg::Vec3d(0.0,0.0,0.0),osg::Vec3d(0.0,0.0,1.0));
    }
    
    // depth partion node is only supports single window/single threaded at present.
    viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);

    return viewer.run();
}

