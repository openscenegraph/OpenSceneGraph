#include <osg/Node>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osg/BlendFunc>
#include <osg/Stencil>
#include <osg/ColorMask>
#include <osg/Depth>
#include <osg/ClipPlane>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgUtil/TransformCallback>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGLUT/glut>
#include <osgGLUT/Viewer>



//
// A simple demo demonstrating planer reflections using multiple renderings 
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

    geom->addPrimitive(osgNew osg::DrawArrays(osg::Primitive::QUADS,0,4));

    return geom;
}

void write_usage(std::ostream& out,const std::string& name)
{
    out << std::endl;
    out <<"usage:"<< std::endl;
    out <<"    "<<name<<" [options] infile1 [infile2 ...]"<< std::endl;
    out << std::endl;
    out <<"options:"<< std::endl;
    out <<"    -l libraryName      - load plugin of name libraryName"<< std::endl;
    out <<"                          i.e. -l osgdb_pfb"<< std::endl;
    out <<"                          Useful for loading reader/writers which can load"<< std::endl;
    out <<"                          other file formats in addition to its extension."<< std::endl;
    out <<"    -e extensionName    - load reader/wrter plugin for file extension"<< std::endl;
    out <<"                          i.e. -e pfb"<< std::endl;
    out <<"                          Useful short hand for specifying full library name as"<< std::endl;
    out <<"                          done with -l above, as it automatically expands to"<< std::endl;
    out <<"                          the full library name appropriate for each platform."<< std::endl;
    out <<std::endl;
    out <<"    -stereo             - switch on stereo rendering, using the default of,"<< std::endl;
    out <<"                          ANAGLYPHIC or the value set in the OSG_STEREO_MODE "<< std::endl;
    out <<"                          environmental variable. See doc/stereo.html for "<< std::endl;
    out <<"                          further details on setting up accurate stereo "<< std::endl;
    out <<"                          for your system. "<< std::endl;
    out <<"    -stereo ANAGLYPHIC  - switch on anaglyphic(red/cyan) stereo rendering."<< std::endl;
    out <<"    -stereo QUAD_BUFFER - switch on quad buffered stereo rendering."<< std::endl;
    out <<std::endl;
    out <<"    -stencil            - use a visual with stencil buffer enabled, this "<< std::endl;
    out <<"                          also allows the depth complexity statistics mode"<< std::endl;
    out <<"                          to be used (press 'p' three times to cycle to it)."<< std::endl;
    out << std::endl;
}

int main( int argc, char **argv )
{

    // initialize the GLUT
    glutInit( &argc, argv );

    if (argc<2)
    {
        write_usage(osg::notify(osg::NOTICE),argv[0]);
        return 0;
    }

    // create the commandline args.
    std::vector<std::string> commandLine;
    for(int i=1;i<argc;++i) commandLine.push_back(argv[i]);
    

    // initialize the viewer.
    osgGLUT::Viewer viewer;
    viewer.setWindowTitle(argv[0]);
    
    // configure the viewer from the commandline arguments, and eat any
    // parameters that have been matched.
    viewer.readCommandLine(commandLine);
    
    // configure the plugin registry from the commandline arguments, and 
    // eat any parameters that have been matched.
    osgDB::readCommandLine(commandLine);
    
    // load the nodes from the commandline arguments.
    osg::Node* loadedModel = osgDB::readNodeFiles(commandLine);
    

    if (!loadedModel)
    {
//        write_usage(osg::notify(osg::NOTICE),argv[0]);
        return 1;
    }
    
    osg::MatrixTransform* loadedModelTransform = new osg::MatrixTransform;
    loadedModelTransform->addChild(loadedModel);

    // calculate where to place the mirror according to the
    // loaded models bounding sphere.
    const osg::BoundingSphere& bs = loadedModelTransform->getBound();

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
        stencil->setFunction(osg::Stencil::ALWAYS,1,~0);
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
        stencil->setFunction(osg::Stencil::ALWAYS,0,~0);
        stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::REPLACE);

        osg::StateSet* statesetBin2 = new osg::StateSet();        
        statesetBin2->setRenderBinDetails(2,"RenderBin");
        statesetBin2->setAttributeAndModes(stencil,osg::StateAttribute::ON);
        

        osg::Group* groupBin2 = new osg::Group();
        groupBin2->setStateSet(statesetBin2);
        groupBin2->addChild(loadedModelTransform);
        
        rootNode->addChild(groupBin2);
    }
        
    // bin3  - set up the depth to the furthest depth value
    {
    
        // set up the stencil ops so that only operator on this mirrors stencil value.
        osg::Stencil* stencil = new osg::Stencil;
        stencil->setFunction(osg::Stencil::EQUAL,1,~0);
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

        osg::Stencil* stencil = new osg::Stencil;
        stencil->setFunction(osg::Stencil::EQUAL,1,~0);
        stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);

        // this clip plane removes any of the scene which when mirror would
        // poke through the mirror.  However, this clip plane should really
        // flip sides once the eye point goes to the back of the mirror...
        osg::ClipPlane* clipplane = new osg::ClipPlane;
        clipplane->setClipPlane(osg::Vec4(0.0f,0.0f,-1.0f,z));
        clipplane->setClipPlaneNum(0);

        osg::StateSet* dstate = new osg::StateSet;
        dstate->setRenderBinDetails(4,"RenderBin");
        dstate->setMode(GL_CULL_FACE,osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);
        dstate->setAttributeAndModes(stencil,osg::StateAttribute::ON);
        dstate->setAttributeAndModes(clipplane,osg::StateAttribute::ON);

        osg::MatrixTransform* dcs = new osg::MatrixTransform;
        dcs->setStateSet(dstate);
        dcs->preMult(osg::Matrix::translate(0.0f,0.0f,-z)*
                     osg::Matrix::scale(1.0f,1.0f,-1.0f)*
                     osg::Matrix::translate(0.0f,0.0f,z));

        dcs->addChild(loadedModelTransform);

        rootNode->addChild(dcs);
    
    }


    // bin5  - draw the textured mirror and blend it with the reflection.
    {
    
        // set up depth so all writing to depth goes to maximum depth.
        osg::Depth* depth = new osg::Depth;
        depth->setFunction(osg::Depth::ALWAYS);

        osg::Stencil* stencil = new osg::Stencil;
        stencil->setFunction(osg::Stencil::EQUAL,1,~0);
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

    // add model to the viewer.
    viewer.addViewport( rootNode );

    osg::NodeCallback* nc = new osgUtil::TransformCallback(loadedModelTransform->getBound().center(),osg::Vec3(0.0f,0.0f,1.0f),osg::inDegrees(45.0f));
    loadedModelTransform->setAppCallback(nc);

    // register trackball, flight and drive.
    viewer.registerCameraManipulator(new osgGA::TrackballManipulator);
    viewer.registerCameraManipulator(new osgGA::FlightManipulator);
    viewer.registerCameraManipulator(new osgGA::DriveManipulator);

    viewer.open();

    viewer.run();

    return 0;
}
