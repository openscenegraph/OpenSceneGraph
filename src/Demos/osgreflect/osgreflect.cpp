#include <osg/Node>
#include <osg/GeoSet>
#include <osg/Notify>
#include <osg/Transform>
#include <osg/Texture>
#include <osg/Transparency>
#include <osg/Stencil>
#include <osg/ColorMask>
#include <osg/Depth>
#include <osg/ClipPlane>

#include <osgUtil/TrackballManipulator>
#include <osgUtil/FlightManipulator>
#include <osgUtil/DriveManipulator>
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


/*
 * Function to read several files (typically one) as specified on the command
 * line, and return them in an osg::Node
 */
osg::Node* getNodeFromFiles(int argc,char **argv)
{
    osg::Node *rootnode = new osg::Node;

    int i;

    typedef std::vector<osg::Node*> NodeList;
    NodeList nodeList;
    for( i = 1; i < argc; i++ )
    {

        if (argv[i][0]=='-')
        {
            switch(argv[i][1])
            {
                case('l'):
                    ++i;
                    if (i<argc)
                    {
                        osgDB::Registry::instance()->loadLibrary(argv[i]);
                    }
                    break;
                case('e'):
                    ++i;
                    if (i<argc)
                    {
                        std::string libName = osgDB::Registry::instance()->createLibraryNameForExt(argv[i]);
                        osgDB::Registry::instance()->loadLibrary(libName);
                    }
                    break;
            }
        } else
        {
            osg::Node *node = osgDB::readNodeFile( argv[i] );

            if( node != (osg::Node *)0L )
            {
                if (node->getName().empty()) node->setName( argv[i] );
                nodeList.push_back(node);
            }
        }

    }

    if (nodeList.size()==0)
    {
        osg::notify(osg::WARN) << "No data loaded."<<endl;
        exit(0);
    }

    if (nodeList.size()==1)
    {
        rootnode = nodeList.front();
    }
    else                         // size >1
    {
        osg::Group* group = new osg::Group();
        for(NodeList::iterator itr=nodeList.begin();
            itr!=nodeList.end();
            ++itr)
        {
            group->addChild(*itr);
        }

        rootnode = group;
    }

    return rootnode;
}

osg::StateSet* createMirrorTexturedState(const std::string& filename)
{
    osg::StateSet* dstate = new osg::StateSet;
    dstate->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
    
    // set up the texture.
    osg::Image* image = osgDB::readImageFile(filename.c_str());
    if (image)
    {
        osg::Texture* texture = new osg::Texture;
        texture->setImage(image);
        dstate->setAttributeAndModes(texture,osg::StateAttribute::ON);
    }
    
    return dstate;
}


osg::Drawable* createMirrorSurface(float xMin,float xMax,float yMin,float yMax,float z)
{
    
    // set up the drawstate.

    // set up the geoset.
    osg::GeoSet* gset = new osg::GeoSet;

    osg::Vec3* coords = new osg::Vec3 [4];
    coords[0].set(xMin,yMax,z);
    coords[1].set(xMin,yMin,z);
    coords[2].set(xMax,yMin,z);
    coords[3].set(xMax,yMax,z);
    gset->setCoords(coords);

    osg::Vec3* norms = new osg::Vec3 [1];
    norms[0].set(0.0f,0.0f,1.0f);
    gset->setNormals(norms);
    gset->setNormalBinding(osg::GeoSet::BIND_OVERALL);

    osg::Vec2* tcoords = new osg::Vec2 [4];
    tcoords[0].set(0.0f,1.0f);
    tcoords[1].set(0.0f,0.0f);
    tcoords[2].set(1.0f,0.0f);
    tcoords[3].set(1.0f,1.0f);
    gset->setTextureCoords(tcoords);
    gset->setTextureBinding(osg::GeoSet::BIND_PERVERTEX);
    
    osg::Vec4* colours = new osg::Vec4;
    colours->set(1.0f,1.0f,1.0,1.0f);
    gset->setColors(colours);
    gset->setColorBinding(osg::GeoSet::BIND_OVERALL);

    gset->setNumPrims(1);
    gset->setPrimType(osg::GeoSet::QUADS);

    return gset;
}

void write_usage()
{
    osg::notify(osg::NOTICE)<<endl;
    osg::notify(osg::NOTICE)<<"usage:"<<endl;
    osg::notify(osg::NOTICE)<<"    osgreflect [options] infile1 [infile2 ...]"<<endl;
    osg::notify(osg::NOTICE)<<endl;
    osg::notify(osg::NOTICE)<<"options:"<<endl;
    osg::notify(osg::NOTICE)<<"    -l libraryName     - load plugin of name libraryName"<<endl;
    osg::notify(osg::NOTICE)<<"                         i.e. -l osgdb_pfb"<<endl;
    osg::notify(osg::NOTICE)<<"                         Useful for loading reader/writers which can load"<<endl;
    osg::notify(osg::NOTICE)<<"                         other file formats in addition to its extension."<<endl;
    osg::notify(osg::NOTICE)<<"    -e extensionName   - load reader/wrter plugin for file extension"<<endl;
    osg::notify(osg::NOTICE)<<"                         i.e. -e pfb"<<endl;
    osg::notify(osg::NOTICE)<<"                         Useful short hand for specifying full library name as"<<endl;
    osg::notify(osg::NOTICE)<<"                         done with -l above, as it automatically expands to the"<<endl;
    osg::notify(osg::NOTICE)<<"                         full library name appropriate for each platform."<<endl;
    osg::notify(osg::NOTICE)<<endl;
}

int main( int argc, char **argv )
{

    // initialize the GLUT
    glutInit( &argc, argv );

    if (argc<2)
    {
        write_usage();
        return 0;
    }


    // load a model from file, and add it into the root group node.
    osg::Node* loadedModel = getNodeFromFiles( argc, argv);
    
    if (!loadedModel)
    {
        write_usage();
        return 1;
    }
    
    osg::Transform* loadedModelTransform = new osg::Transform;
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
    

    osg::Group* rootNode = new osg::Group;
        
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
        dstate->setMode(GL_CULL_FACE,osg::StateAttribute::OVERRIDE_OFF);
        dstate->setAttributeAndModes(stencil,osg::StateAttribute::ON);
        dstate->setAttributeAndModes(clipplane,osg::StateAttribute::ON);

        osg::Transform* dcs = new osg::Transform;
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
        osg::Transparency* trans = new osg::Transparency;
        trans->setFunction(osg::Transparency::ONE,osg::Transparency::ONE);

        osg::StateSet* statesetBin5 = createMirrorTexturedState("tank.rgb");

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

    // initialize the viewer.
    osgGLUT::Viewer viewer;
    viewer.addViewport( rootNode );

    osg::NodeCallback* nc = new osgUtil::TransformCallback(loadedModelTransform->getBound().center(),osg::Vec3(0.0f,0.0f,1.0f),osg::inDegrees(45.0f));
    loadedModelTransform->setAppCallback(nc);

    // register trackball, flight and drive.
    viewer.registerCameraManipulator(new osgUtil::TrackballManipulator);
    viewer.registerCameraManipulator(new osgUtil::FlightManipulator);
    viewer.registerCameraManipulator(new osgUtil::DriveManipulator);

    viewer.open();

    viewer.run();

    return 0;
}
