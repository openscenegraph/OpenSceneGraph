/* OpenSceneGraph example, osganimate.
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

#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/Geometry>
#include <osg/Geode>

#include <osgUtil/Optimizer>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgSim/OverlayNode>

#include <osgViewer/Viewer>
#include <iostream>

osg::AnimationPath* createAnimationPath(const osg::Vec3& center,float radius,double looptime)
{
    // set up the animation path 
    osg::AnimationPath* animationPath = new osg::AnimationPath;
    animationPath->setLoopMode(osg::AnimationPath::LOOP);
    
    int numSamples = 40;
    float yaw = 0.0f;
    float yaw_delta = 2.0f*osg::PI/((float)numSamples-1.0f);
    float roll = osg::inDegrees(30.0f);
    
    double time=0.0f;
    double time_delta = looptime/(double)numSamples;
    for(int i=0;i<numSamples;++i)
    {
        osg::Vec3 position(center+osg::Vec3(sinf(yaw)*radius,cosf(yaw)*radius,0.0f));
        osg::Quat rotation(osg::Quat(roll,osg::Vec3(0.0,1.0,0.0))*osg::Quat(-(yaw+osg::inDegrees(90.0f)),osg::Vec3(0.0,0.0,1.0)));
        
        animationPath->insert(time,osg::AnimationPath::ControlPoint(position,rotation));

        yaw += yaw_delta;
        time += time_delta;

    }
    return animationPath;    
}

osg::Node* createBase(const osg::Vec3& center,float radius)
{

    

    int numTilesX = 10;
    int numTilesY = 10;
    
    float width = 2*radius;
    float height = 2*radius;
    
    osg::Vec3 v000(center - osg::Vec3(width*0.5f,height*0.5f,0.0f));
    osg::Vec3 dx(osg::Vec3(width/((float)numTilesX),0.0,0.0f));
    osg::Vec3 dy(osg::Vec3(0.0f,height/((float)numTilesY),0.0f));
    
    // fill in vertices for grid, note numTilesX+1 * numTilesY+1...
    osg::Vec3Array* coords = new osg::Vec3Array;
    int iy;
    for(iy=0;iy<=numTilesY;++iy)
    {
        for(int ix=0;ix<=numTilesX;++ix)
        {
            coords->push_back(v000+dx*(float)ix+dy*(float)iy);
        }
    }
    
    //Just two colours - black and white.
    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f)); // white
    colors->push_back(osg::Vec4(0.0f,0.0f,0.0f,1.0f)); // black
    int numColors=colors->size();
    
    
    int numIndicesPerRow=numTilesX+1;
    osg::UByteArray* coordIndices = new osg::UByteArray; // assumes we are using less than 256 points...
    osg::UByteArray* colorIndices = new osg::UByteArray;
    for(iy=0;iy<numTilesY;++iy)
    {
        for(int ix=0;ix<numTilesX;++ix)
        {
            // four vertices per quad.
            coordIndices->push_back(ix    +(iy+1)*numIndicesPerRow);
            coordIndices->push_back(ix    +iy*numIndicesPerRow);
            coordIndices->push_back((ix+1)+iy*numIndicesPerRow);
            coordIndices->push_back((ix+1)+(iy+1)*numIndicesPerRow);
            
            // one color per quad
            colorIndices->push_back((ix+iy)%numColors);
        }
    }
    

    // set up a single normal
    osg::Vec3Array* normals = new osg::Vec3Array;
    normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
    

    osg::Geometry* geom = new osg::Geometry;
    geom->setVertexArray(coords);
    geom->setVertexIndices(coordIndices);
    
    geom->setColorArray(colors);
    geom->setColorIndices(colorIndices);
    geom->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE);
    
    geom->setNormalArray(normals);
    geom->setNormalBinding(osg::Geometry::BIND_OVERALL);
    
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,coordIndices->size()));
    
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geom);
    
    return geode;
}

osg::Node* createMovingModel(const osg::Vec3& center, float radius)
{
    float animationLength = 10.0f;

    osg::AnimationPath* animationPath = createAnimationPath(center,radius,animationLength);

    osg::Group* model = new osg::Group;

    osg::Node* glider = osgDB::readNodeFile("glider.osg");
    if (glider)
    {
        const osg::BoundingSphere& bs = glider->getBound();

        float size = radius/bs.radius()*0.3f;
        osg::MatrixTransform* positioned = new osg::MatrixTransform;
        positioned->setDataVariance(osg::Object::STATIC);
        positioned->setMatrix(osg::Matrix::translate(-bs.center())*
                                     osg::Matrix::scale(size,size,size)*
                                     osg::Matrix::rotate(osg::inDegrees(-90.0f),0.0f,0.0f,1.0f));
    
        positioned->addChild(glider);
    
        osg::PositionAttitudeTransform* xform = new osg::PositionAttitudeTransform;    
        xform->setUpdateCallback(new osg::AnimationPathCallback(animationPath,0.0,1.0));
        xform->addChild(positioned);

        model->addChild(xform);
    }
 
    osg::Node* cessna = osgDB::readNodeFile("cessna.osg");
    if (cessna)
    {
        const osg::BoundingSphere& bs = cessna->getBound();

        float size = radius/bs.radius()*0.3f;
        osg::MatrixTransform* positioned = new osg::MatrixTransform;
        positioned->setDataVariance(osg::Object::STATIC);
        positioned->setMatrix(osg::Matrix::translate(-bs.center())*
                                     osg::Matrix::scale(size,size,size)*
                                     osg::Matrix::rotate(osg::inDegrees(180.0f),0.0f,0.0f,1.0f));
    
        positioned->addChild(cessna);
    
        osg::MatrixTransform* xform = new osg::MatrixTransform;
        xform->setUpdateCallback(new osg::AnimationPathCallback(animationPath,0.0f,2.0));
        xform->addChild(positioned);

        model->addChild(xform);
    }
    
    return model;
}

osg::Node* createModel(bool overlay, osgSim::OverlayNode::OverlayTechnique technique)
{
    osg::Vec3 center(0.0f,0.0f,0.0f);
    float radius = 100.0f;

    osg::Group* root = new osg::Group;

    float baseHeight = center.z()-radius*0.5;
    osg::Node* baseModel = createBase(osg::Vec3(center.x(), center.y(), baseHeight),radius);
    osg::Node* movingModel = createMovingModel(center,radius*0.8f);

    if (overlay)
    {
        osgSim::OverlayNode* overlayNode = new osgSim::OverlayNode(technique);
        overlayNode->setContinuousUpdate(true);
        overlayNode->setOverlaySubgraph(movingModel);
        overlayNode->setOverlayBaseHeight(baseHeight-0.01);
        overlayNode->addChild(baseModel);
        root->addChild(overlayNode);
    }
    else
    {
    
        root->addChild(baseModel);
    }
    
    root->addChild(movingModel);

    return root;
}


int main( int argc, char **argv )
{
    
    bool overlay = false;
    osg::ArgumentParser arguments(&argc,argv);
    while (arguments.read("--overlay")) overlay = true;
    
    osgSim::OverlayNode::OverlayTechnique technique = osgSim::OverlayNode::OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY;
    while (arguments.read("--object")) { technique = osgSim::OverlayNode::OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY; overlay=true; }
    while (arguments.read("--ortho") || arguments.read("--orthographic")) { technique = osgSim::OverlayNode::VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY; overlay=true; }
    while (arguments.read("--persp") || arguments.read("--perspective")) { technique = osgSim::OverlayNode::VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY; overlay=true; }
    

    // initialize the viewer.
    osgViewer::Viewer viewer;

    // load the nodes from the commandline arguments.
    osg::Node* model = createModel(overlay, technique);
    if (!model)
    {
        return 1;
    }
    
    // tilt the scene so the default eye position is looking down on the model.
    osg::MatrixTransform* rootnode = new osg::MatrixTransform;
    rootnode->setMatrix(osg::Matrix::rotate(osg::inDegrees(30.0f),1.0f,0.0f,0.0f));
    rootnode->addChild(model);

    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
     
    // set the scene to render
    viewer.setSceneData(rootnode);

    viewer.setCameraManipulator(new osgGA::TrackballManipulator());

    // viewer.setUpViewOnSingleScreen(1);

#if 0

    // use of custom simulation time.
    
    viewer.realize();
    
    double simulationTime = 0.0;
    
    while (!viewer.done())
    {
        viewer.frame(simulationTime);
        simulationTime += 0.001;
    }
    
    return 0;
#else

    // normal viewer usage.
    return viewer.run();

#endif
}
