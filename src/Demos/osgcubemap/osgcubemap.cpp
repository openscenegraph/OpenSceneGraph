#include <osg/Group>
#include <osg/StateSet>
#include <osg/TextureCubeMap>
#include <osg/TexGen>
#include <osg/TexEnvCombine>

#include <osgUtil/ReflectionMapGenerator>
#include <osgUtil/HighlightMapGenerator>
#include <osgUtil/HalfWayMapGenerator>
#include <osgUtil/Optimizer>

#include <osgDB/ReadFile>
#include <osgDB/Registry>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgGLUT/Viewer>
#include <osgGLUT/glut>

#include <iostream>
#include <string>
#include <vector>

void write_usage(std::ostream& out, const std::string& name)
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
    out <<"    -f                  - start with a full screen, borderless window." << std::endl;
    out << std::endl;
}

void create_specular_highlights(osg::Node *node)
{
    osg::StateSet *ss = node->getOrCreateStateSet();
    
    // create and setup the texture object
    osg::TextureCubeMap *tcm = osgNew osg::TextureCubeMap;
    tcm->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP);
    tcm->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP);
    tcm->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP);
    tcm->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
    tcm->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);    

    // generate the six highlight map images (light direction = [1, 1, -1])
    osgUtil::HighlightMapGenerator *mapgen = osgNew osgUtil::HighlightMapGenerator(
        osg::Vec3(1, 1, -1),            // light direction
        osg::Vec4(1, 0.9f, 0.8f, 1),    // light color
        8);                             // specular exponent
    
    mapgen->generateMap();

    // assign the six images to the texture object
    tcm->setImage(osg::TextureCubeMap::POSITIVE_X, mapgen->getImage(osg::TextureCubeMap::POSITIVE_X));
    tcm->setImage(osg::TextureCubeMap::NEGATIVE_X, mapgen->getImage(osg::TextureCubeMap::NEGATIVE_X));
    tcm->setImage(osg::TextureCubeMap::POSITIVE_Y, mapgen->getImage(osg::TextureCubeMap::POSITIVE_Y));
    tcm->setImage(osg::TextureCubeMap::NEGATIVE_Y, mapgen->getImage(osg::TextureCubeMap::NEGATIVE_Y));
    tcm->setImage(osg::TextureCubeMap::POSITIVE_Z, mapgen->getImage(osg::TextureCubeMap::POSITIVE_Z));
    tcm->setImage(osg::TextureCubeMap::NEGATIVE_Z, mapgen->getImage(osg::TextureCubeMap::NEGATIVE_Z));

    // enable texturing, replacing any textures in the subgraphs
    ss->setTextureAttributeAndModes(0, tcm, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
    ss->setTextureMode(0, GL_TEXTURE_CUBE_MAP, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

    // texture coordinate generation
    osg::TexGen *tg = osgNew osg::TexGen;
    tg->setMode(osg::TexGen::REFLECTION_MAP);
    ss->setTextureAttributeAndModes(0, tg, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

    // use TexEnvCombine to add the highlights to the original lighting
    osg::TexEnvCombine *te = osgNew osg::TexEnvCombine;    
    te->setCombine_RGB(osg::TexEnvCombine::ADD);
    te->setOperand0_RGB(osg::TexEnvCombine::TEXTURE0);
    te->setOperand1_RGB(osg::TexEnvCombine::PRIMARY_COLOR);
    ss->setTextureAttributeAndModes(0, te, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
}

int main(int argc, char *argv[])
{
    // initialize the GLUT
    glutInit( &argc, argv );

    if (argc<2) {
        write_usage(std::cout, argv[0]);
        return 0;
    }
    
    // create the commandline args.
    std::vector<std::string> commandLine;
    for(int i=1; i<argc; ++i) commandLine.push_back(argv[i]);    

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
    osg::Node* rootnode = osgDB::readNodeFiles(commandLine);
    if (!rootnode) {
        return 1;
    }

    // create specular highlights
    create_specular_highlights(rootnode);
    
    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
     
    // add a viewport to the viewer and attach the scene graph.
    viewer.addViewport(rootnode);
    
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
