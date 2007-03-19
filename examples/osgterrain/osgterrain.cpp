#include <osgViewer/Viewer>

#include <osg/Group>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>
#include <osg/CoordinateSystemNode>
#include <osg/ClusterCullingCallback>
#include <osg/ArgumentParser>

#include <osgDB/FileUtils>
#include <osgDB/ReadFile>

#include <osgText/FadeText>

#include <osgTerrain/TerrainNode>
#include <osgTerrain/Layer>

#include <iostream>


int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    // construct the viewer.
    osgViewer::Viewer viewer;

    double x = 0.0;
    double y = 0.0;
    double w = 1.0;
    double h = 1.0;

    osg::ref_ptr<osg::Node> root = new osgTerrain::TerrainNode;

    bool readParameter = false;
    do 
    {
        readParameter = false;
        std::string filename;
        
        if (arguments.read("-x",x)) readParameter = true;
        if (arguments.read("-y",y)) readParameter = true;
        if (arguments.read("-w",w)) readParameter = true;
        if (arguments.read("-h",h)) readParameter = true;

        if (arguments.read("--dem",filename))
        {
            readParameter = true;
            osg::notify(osg::NOTICE)<<"--dem "<<filename<<" x="<<x<<" y="<<y<<" w="<<w<<" h="<<h<<std::endl;
        }
        
        if (arguments.read("--image",filename))
        {
            readParameter = true;
            osg::notify(osg::NOTICE)<<"--image "<<filename<<" x="<<x<<" y="<<y<<" w="<<w<<" h="<<h<<std::endl;
        }

    } while (readParameter);
    

    
    
    if (!root) return 0;

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData(root.get());

    return viewer.run();
}
