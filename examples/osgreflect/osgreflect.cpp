/* OpenSceneGraph example, osgreflect.
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

#include <osg/Node>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osg/BlendFunc>
#include <osg/Stencil>
#include <osg/ColorMask>
#include <osg/Depth>
#include <osg/ClipNode>
#include <osg/AnimationPath>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgUtil/Optimizer>
#include <osgViewer/Viewer>

#include <iostream>

//
// A simple demo demonstrating planar reflections using multiple renderings 
// of a subgraph, overriding of state attribures and use of the stencil buffer.
//
// The multipass system implemented here is a variation if Mark Kilgard's
// paper "Improving Shadows and Reflections via the Stencil Buffer" which
// can be found on the developer parts of the NVidia web site.
//
// The variations comes from the fact that the mirrors stencil values
// are done on the first pass, rather than the second as in Mark's paper.
// The second pass is now Mark's first pass - drawing the unreflected scene,
// but also unsets the stencil buffer.  This variation stops the unreflected
// world poking through the mirror to be seen in the final rendering and
// also obscures the world correctly when on the reverse side of the mirror.
// Although there is still some unresolved issue with the clip plane needing
// to be flipped when looking at the reverse side of the mirror.  Niether
// of these issues are mentioned in the Mark's paper, but trip us up when
// we apply them. 


osg::StateSet* createMirrorTexturedState(const std::string& filename)
{
    osg::StateSet* dstate = new osg::StateSet;
    dstate->setMode(GL_CULL_FACE,osg::StateAttribute::OFF|osg::StateAttribute::PROTECTED);
    
    // set up the texture.
    osg::Image* image = osgDB::readImageFile(filename.c_str());
    if (image)
    {
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);
        dstate->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON|osg::StateAttribute::PROTECTED);
    }
    
    return dstate;
}


osg::Drawable* createMirrorSurface(float xMin,float xMax,float yMin,float yMax,float z)
{
    
    // set up the drawstate.

    // set up the Geometry.
    osg::Geometry* geom = new osg::Geometry;

    osg::Vec3Array* coords = new osg::Vec3Array(4);
    (*coords)[0].set(xMin,yMax,z);
    (*coords)[1].set(xMin,yMin,z);
    (*coords)[2].set(xMax,yMin,z);
    (*coords)[3].set(xMax,yMax,z);
    geom->setVertexArray(coords);

    osg::Vec3Array* norms = new osg::Vec3Array(1);
    (*norms)[0].set(0.0f,0.0f,1.0f);
    geom->setNormalArray(norms);
    geom->setNormalBinding(osg::Geometry::BIND_OVERALL);

    osg::Vec2Array* tcoords = new osg::Vec2Array(4);
    (*tcoords)[0].set(0.0f,1.0f);
    (*tcoords)[1].set(0.0f,0.0f);
    (*tcoords)[2].set(1.0f,0.0f);
    (*tcoords)[3].set(1.0f,1.0f);
    geom->setTexCoordArray(0,tcoords);
    
    osg::Vec4Array* colours = new osg::Vec4Array(1);
    (*colours)[0].set(1.0f,1.0f,1.0,1.0f);
    geom->setColorArray(colours);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);

    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));

    return geom;
}

osg::Node* createMirroredScene(osg::Node* model)
{

    // calculate where to place the mirror according to the
    // loaded models bounding sphere.
    const osg::BoundingSphere& bs = model->getBound();

    float width_factor = 1.5;
    float height_factor = 0.3;
    
    float xMin = bs.center().x()-bs.radius()*width_factor;
    float xMax = bs.center().x()+bs.radius()*width_factor;
    float yMin = bs.center().y()-bs.radius()*width_factor;
    float yMax = bs.center().y()+bs.radius()*width_factor;
    
    float z = bs.center().z()-bs.radius()*height_factor;
    
    
    // create a textured, transparent node at the appropriate place.
    osg::Drawable* mirror = createMirrorSurface(xMin,xMax,yMin,yMax,z);
    

    osg::MatrixTransform* rootNode = new osg::MatrixTransform;
    rootNode->setMatrix(osg::Matrix::rotate(osg::inDegrees(45.0f),1.0f,0.0f,0.0f));
        
    // make sure that the global color mask exists.
    osg::ColorMask* rootColorMask = new osg::ColorMask;
    rootColorMask->setMask(true,true,true,true);        
    
    // set up depth to be inherited by the rest of the scene unless
    // overrideen. this is overridden in bin 3.
    osg::Depth* rootDepth = new osg::Depth;
    rootDepth->setFunction(osg::Depth::LESS);
    rootDepth->setRange(0.0,1.0);

    osg::StateSet* rootStateSet = new osg::StateSet();        
    rootStateSet->setAttribute(rootColorMask);
    rootStateSet->setAttribute(rootDepth);

    rootNode->setStateSet(rootStateSet);


    // bin1  - set up the stencil values and depth for mirror.
    {
    
        // set up the stencil ops so that the stencil buffer get set at
        // the mirror plane 
        osg::Stencil* stencil = new osg::Stencil;
        stencil->setFunction(osg::Stencil::ALWAYS,1,~0u);
        stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::REPLACE);
        
        // switch off the writing to the color bit planes.
        osg::ColorMask* colorMask = new osg::ColorMask;
        colorMask->setMask(false,false,false,false);
        
        osg::StateSet* statesetBin1 = new osg::StateSet();        
        statesetBin1->setRenderBinDetails(1,"RenderBin");
        statesetBin1->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
        statesetBin1->setAttributeAndModes(stencil,osg::StateAttribute::ON);
        statesetBin1->setAttribute(colorMask);
        
        // set up the mirror geode.
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(mirror);
        geode->setStateSet(statesetBin1);
        
        rootNode->addChild(geode);
        
    }

    // bin one - draw scene without mirror or reflection, unset 
    // stencil values where scene is infront of mirror and hence
    // occludes the mirror. 
    {        
        osg::Stencil* stencil = new osg::Stencil;
        stencil->setFunction(osg::Stencil::ALWAYS,0,~0u);
        stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::REPLACE);

        osg::StateSet* statesetBin2 = new osg::StateSet();        
        statesetBin2->setRenderBinDetails(2,"RenderBin");
        statesetBin2->setAttributeAndModes(stencil,osg::StateAttribute::ON);
        

        osg::Group* groupBin2 = new osg::Group();
        groupBin2->setStateSet(statesetBin2);
        groupBin2->addChild(model);
        
        rootNode->addChild(groupBin2);
    }
        
    // bin3  - set up the depth to the furthest depth value
    {
    
        // set up the stencil ops so that only operator on this mirrors stencil value.
        osg::Stencil* stencil = new osg::Stencil;
        stencil->setFunction(osg::Stencil::EQUAL,1,~0u);
        stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);
        
        // switch off the writing to the color bit planes.
        osg::ColorMask* colorMask = new osg::ColorMask;
        colorMask->setMask(false,false,false,false);

        // set up depth so all writing to depth goes to maximum depth.
        osg::Depth* depth = new osg::Depth;
        depth->setFunction(osg::Depth::ALWAYS);
        depth->setRange(1.0,1.0);

        osg::StateSet* statesetBin3 = new osg::StateSet();
        statesetBin3->setRenderBinDetails(3,"RenderBin");
        statesetBin3->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
        statesetBin3->setAttributeAndModes(stencil,osg::StateAttribute::ON);
        statesetBin3->setAttribute(colorMask);
        statesetBin3->setAttribute(depth);
        
        // set up the mirror geode.
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(mirror);
        geode->setStateSet(statesetBin3);
        
        rootNode->addChild(geode);
        
    }

    // bin4  - draw the reflection.
    {
    
        // now create the 'reflection' of the loaded model by applying
        // create a Transform which flips the loaded model about the z axis
        // relative to the mirror node, the loadedModel is added to the
        // Transform so now appears twice in the scene, but is shared so there
        // is negligable memory overhead.  Also use an osg::StateSet 
        // attached to the Transform to override the face culling on the subgraph
        // to prevert an 'inside' out view of the reflected model.
        // set up the stencil ops so that only operator on this mirrors stencil value.



        // this clip plane removes any of the scene which when mirror would
        // poke through the mirror.  However, this clip plane should really
        // flip sides once the eye point goes to the back of the mirror...
        osg::ClipPlane* clipplane = new osg::ClipPlane;
        clipplane->setClipPlane(0.0,0.0,-1.0,z);
        clipplane->setClipPlaneNum(0);

        osg::ClipNode* clipNode = new osg::ClipNode;
        clipNode->addClipPlane(clipplane);


        osg::StateSet* dstate = clipNode->getOrCreateStateSet();
        dstate->setRenderBinDetails(4,"RenderBin");
        dstate->setMode(GL_CULL_FACE,osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);

        osg::Stencil* stencil = new osg::Stencil;
        stencil->setFunction(osg::Stencil::EQUAL,1,~0u);
        stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);
        dstate->setAttributeAndModes(stencil,osg::StateAttribute::ON);

        osg::MatrixTransform* reverseMatrix = new osg::MatrixTransform;
        reverseMatrix->setStateSet(dstate);
        reverseMatrix->preMult(osg::Matrix::translate(0.0f,0.0f,-z)*
                     osg::Matrix::scale(1.0f,1.0f,-1.0f)*
                     osg::Matrix::translate(0.0f,0.0f,z));

        reverseMatrix->addChild(model);

        clipNode->addChild(reverseMatrix);

        rootNode->addChild(clipNode);
    
    }


    // bin5  - draw the textured mirror and blend it with the reflection.
    {
    
        // set up depth so all writing to depth goes to maximum depth.
        osg::Depth* depth = new osg::Depth;
        depth->setFunction(osg::Depth::ALWAYS);

        osg::Stencil* stencil = new osg::Stencil;
        stencil->setFunction(osg::Stencil::EQUAL,1,~0u);
        stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::ZERO);

        // set up additive blending.
        osg::BlendFunc* trans = new osg::BlendFunc;
        trans->setFunction(osg::BlendFunc::ONE,osg::BlendFunc::ONE);

        osg::StateSet* statesetBin5 = createMirrorTexturedState("Images/tank.rgb");

        statesetBin5->setRenderBinDetails(5,"RenderBin");
        statesetBin5->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
        statesetBin5->setAttributeAndModes(stencil,osg::StateAttribute::ON);
        statesetBin5->setAttributeAndModes(trans,osg::StateAttribute::ON);
        statesetBin5->setAttribute(depth);
        
        // set up the mirror geode.
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(mirror);
        geode->setStateSet(statesetBin5);
        
        rootNode->addChild(geode);

    }
    
    return rootNode;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// create the viewer
// load a model
// decoate the model so it renders using a multipass stencil buffer technique for planar reflections.
// release the viewer
// run main loop.
//
int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // construct the viewer.
    osgViewer::Viewer viewer;

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    
    // if not loaded assume no arguments passed in, try use default mode instead.
    if (!loadedModel) loadedModel = osgDB::readNodeFile("cessna.osg");

    // if no model has been successfully loaded report failure.
    if (!loadedModel) 
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }


    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel.get());

    // add a transform with a callback to animate the loaded model.
    osg::ref_ptr<osg::MatrixTransform> loadedModelTransform = new osg::MatrixTransform;
    loadedModelTransform->addChild(loadedModel.get());

    osg::ref_ptr<osg::NodeCallback> nc = new osg::AnimationPathCallback(loadedModelTransform->getBound().center(),osg::Vec3(0.0f,0.0f,1.0f),osg::inDegrees(45.0f));
    loadedModelTransform->setUpdateCallback(nc.get());


    // finally decorate the loaded model so that it has the required multipass/bin scene graph to do the reflection effect.
    osg::ref_ptr<osg::Node> rootNode = createMirroredScene(loadedModelTransform.get());

    // set the scene to render
    viewer.setSceneData(rootNode.get());

    // hint to tell viewer to request stencil buffer when setting up windows
    osg::DisplaySettings::instance()->setMinimumNumStencilBits(8);

    osgDB::writeNodeFile(*rootNode, "test.osg");

    return viewer.run();

}
