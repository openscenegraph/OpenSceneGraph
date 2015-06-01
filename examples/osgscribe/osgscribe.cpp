/* OpenSceneGraph example, osgscribe.
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
#include <osg/Group>
#include <osg/Notify>
#include <osg/Material>
#include <osg/PolygonOffset>
#include <osg/PolygonMode>
#include <osg/LineStipple>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>

#include <osgUtil/Optimizer>

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // construct the viewer.
    osgViewer::Viewer viewer;

    // load the nodes from the commandline arguments.
    osg::Node* loadedModel = osgDB::readNodeFiles(arguments);

    // if not loaded assume no arguments passed in, try use default mode instead.
    if (!loadedModel) loadedModel = osgDB::readNodeFile("cow.osgt");
    
    if (!loadedModel)
    {
        osg::notify(osg::NOTICE)<<"Please specify a model filename on the command line."<<std::endl;
        return 1;
    }
  
    // to do scribe mode we create a top most group to contain the
    // original model, and then a second group contains the same model
    // but overrides various state attributes, so that the second instance
    // is rendered as wireframe.
    
    osg::Group* rootnode = new osg::Group;

    osg::Group* decorator = new osg::Group;
    
    rootnode->addChild(loadedModel);
    
    
    rootnode->addChild(decorator);
    
    decorator->addChild(loadedModel);  

    // set up the state so that the underlying color is not seen through
    // and that the drawing mode is changed to wireframe, and a polygon offset
    // is added to ensure that we see the wireframe itself, and turn off 
    // so texturing too.
    osg::StateSet* stateset = new osg::StateSet;
    osg::PolygonOffset* polyoffset = new osg::PolygonOffset;
    polyoffset->setFactor(-1.0f);
    polyoffset->setUnits(-1.0f);
    osg::PolygonMode* polymode = new osg::PolygonMode;
    polymode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
    stateset->setAttributeAndModes(polyoffset,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
    stateset->setAttributeAndModes(polymode,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

#if 1
    osg::Material* material = new osg::Material;
    stateset->setAttributeAndModes(material,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);
#else
    // version which sets the color of the wireframe.
    osg::Material* material = new osg::Material;
    material->setColorMode(osg::Material::OFF); // switch glColor usage off
    // turn all lighting off 
    material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.0,0.0f,0.0f,1.0f));
    material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0.0,0.0f,0.0f,1.0f));
    material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0.0,0.0f,0.0f,1.0f));
    // except emission... in which we set the color we desire
    material->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(0.0,1.0f,0.0f,1.0f));
    stateset->setAttributeAndModes(material,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
#endif

    stateset->setTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);
    
//     osg::LineStipple* linestipple = new osg::LineStipple;
//     linestipple->setFactor(1);
//     linestipple->setPattern(0xf0f0);
//     stateset->setAttributeAndModes(linestipple,osg::StateAttribute::OVERRIDE_ON);
    
    decorator->setStateSet(stateset);
  
    
    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
     
    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData( rootnode );
    
    return viewer.run();
}
