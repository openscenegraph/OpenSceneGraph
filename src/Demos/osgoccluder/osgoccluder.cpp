#include <osg/GL>
#include <osgGLUT/glut>
#include <osgGLUT/Viewer>

#include <osg/Transform>
#include <osg/Billboard>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgUtil/Optimizer>

#include <osg/OccluderNode>
#include <osg/GeoSet>

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




osg::Node* createOccludersAroundModel(osg::Node* model)
{
    osg::Group* scene = osgNew osg::Group;
    scene->setName("rootgroup");


    // add the loaded model into a the scene group.
    scene->addChild(model);
    model->setName("model");


    // create and occluder which will site along side the loadmodel model.
    osg::OccluderNode* occluderNode = osgNew osg::OccluderNode;

    // get the bounding volume of the model.
    const osg::BoundingSphere bs = model->getBound();
    
    // create a bounding box around the sphere.
    osg::BoundingBox bb;
    bb.expandBy(bs);

    // create the convex planer occluder 
    osg::ConvexPlanerOccluder* cpo = osgNew osg::ConvexPlanerOccluder;

    // attach it to the occluder node.
    occluderNode->setOccluder(cpo);
    occluderNode->setName("occluder");
    
    // set the occluder up for the front face of the bounding box.
    osg::ConvexPlanerPolygon& occluder = cpo->getOccluder();
    occluder.add(osg::Vec3(bb.xMin(),bb.yMin(),bb.zMin()));
    occluder.add(osg::Vec3(bb.xMax(),bb.yMin(),bb.zMin()));
    occluder.add(osg::Vec3(bb.xMax(),bb.yMin(),bb.zMax()));
    occluder.add(osg::Vec3(bb.xMin(),bb.yMin(),bb.zMax()));
    
// 
//     // create a hole in the occluder.    
//     osg::Vec3 center((bb.xMin()+bb.xMax())*0.5f,bb.yMin(),(bb.zMin()+bb.zMax())*0.5f);
//     float dx = (bb.xMax()-bb.xMin())*0.25f;
//     float dz = (bb.zMax()-bb.zMin())*0.25f;
//     
// 
//     cpo->getHoleList().push_back();
//     osg::ConvexPlanerPolygon& hole = cpo->getHoleList().back();
//     hole.add(center+osg::Vec3(-dx,0.0,-dz));
//     hole.add(center+osg::Vec3(dx,0.0,-dz));
//     hole.add(center+osg::Vec3(dx,0.0,dz));
//     hole.add(center+osg::Vec3(-dx,0.0,dz));
    

   // create a drawable for occluder.
    osg::GeoSet* geoset = osgNew osg::GeoSet;
    
    osg::Vec3* coords = osgNew osg::Vec3[occluder.getVertexList().size()];
    std::copy(occluder.getVertexList().begin(),occluder.getVertexList().end(),coords);
    geoset->setCoords(coords);
    
    osg::Vec4* color = osgNew osg::Vec4[1];
    color[0].set(1.0f,1.0f,1.0f,0.5f);
    geoset->setColors(color);
    geoset->setColorBinding(osg::GeoSet::BIND_OVERALL);
    
    geoset->setPrimType(osg::GeoSet::QUADS);
    geoset->setNumPrims(1);
    
    osg::Geode* geode = osgNew osg::Geode;
    geode->addDrawable(geoset);
    
    osg::StateSet* stateset = osgNew osg::StateSet;
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
    stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    
    geoset->setStateSet(stateset);
    
    // add the occluder geode as a child of the occluder,
    // as the occluder can't self occlude its subgraph the
    // geode will never be occluder by this occluder.
    occluderNode->addChild(geode);    
    
    // add the occluder node into the scene.
    scene->addChild(occluderNode);

    return scene;
} 


int main( int argc, char **argv )
{

    // initialize the GLUT
    glutInit( &argc, argv );

    if (argc<2)
    {
        write_usage(std::cout,argv[0]);
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
    osg::Node* loadedmodel = osgDB::readNodeFiles(commandLine);
    if (!loadedmodel)
    {
//        write_usage(osg::notify(osg::NOTICE),argv[0]);
        return 1;
    }
    
    // add the occluders to the loaded model.
    osg::Node* rootnode = createOccludersAroundModel(loadedmodel);
    
    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
     
    // add a viewport to the viewer and attach the scene graph.
    viewer.addViewport( rootnode );
    
    // register trackball, flight and drive.
    viewer.registerCameraManipulator(new osgGA::TrackballManipulator);
    viewer.registerCameraManipulator(new osgGA::FlightManipulator);
    viewer.registerCameraManipulator(new osgGA::DriveManipulator);

    // open the viewer window.
    viewer.open();
    
    // fire up the event loop.
    viewer.run();

    return 0;
}
