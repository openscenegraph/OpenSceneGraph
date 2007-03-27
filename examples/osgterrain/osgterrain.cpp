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
#include <osgTerrain/GeometryTechnique>
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

    osg::ref_ptr<osgTerrain::TerrainNode> terrain = new osgTerrain::TerrainNode;
    osg::ref_ptr<osgTerrain::Locator> locator = new osgTerrain::EllipsoidLocator(-osg::PI, -osg::PI*0.5, 2.0*osg::PI, osg::PI, 0.0);


    bool readParameter = false;
    do 
    {
        readParameter = false;
        std::string filename;
        
        if (arguments.read("-e",x,y,w,h))
        {
            // define the extents.
            locator = new osgTerrain::EllipsoidLocator(x,y,w,h,0);
            readParameter = true;
        }

        if (arguments.read("--hf",filename))
        {
            readParameter = true;
            
            osg::notify(osg::NOTICE)<<"--hf "<<filename<<" x="<<x<<" y="<<y<<" w="<<w<<" h="<<h<<std::endl;

            osg::ref_ptr<osg::HeightField> hf = osgDB::readHeightFieldFile(filename);
            if (hf.valid())
            {
                osg::ref_ptr<osgTerrain::HeightFieldLayer> hfl = new osgTerrain::HeightFieldLayer;
                hfl->setHeightField(hf.get());
                
                hfl->setLocator(locator.get());
                
                terrain->setElevationLayer(hfl.get());
                
                osg::notify(osg::NOTICE)<<"created osgTerrain::HeightFieldLayer"<<std::endl;
            }
            else
            {
                osg::notify(osg::NOTICE)<<"failed to create osgTerrain::HeightFieldLayer"<<std::endl;
            }
            
        }
        
        if (arguments.read("-h",filename) || arguments.read("--elevation-image",filename))
        {
            readParameter = true;
            osg::notify(osg::NOTICE)<<"--elevation-image "<<filename<<" x="<<x<<" y="<<y<<" w="<<w<<" h="<<h<<std::endl;

            osg::ref_ptr<osg::Image> image = osgDB::readImageFile(filename);
            if (image.valid())
            {
                osg::ref_ptr<osgTerrain::ImageLayer> imageLayer = new osgTerrain::ImageLayer;
                imageLayer->setImage(image.get());
                imageLayer->setLocator(locator.get());
                
                terrain->setElevationLayer(imageLayer.get());
                
                osg::notify(osg::NOTICE)<<"created Elevation osgTerrain::ImageLayer"<<std::endl;
            }
            else
            {
                osg::notify(osg::NOTICE)<<"failed to create osgTerrain::ImageLayer"<<std::endl;
            }
        }

        if (arguments.read("-c",filename) || arguments.read("--image",filename))
        {
            readParameter = true;
            osg::notify(osg::NOTICE)<<"--image "<<filename<<" x="<<x<<" y="<<y<<" w="<<w<<" h="<<h<<std::endl;

            osg::ref_ptr<osg::Image> image = osgDB::readImageFile(filename);
            if (image.valid())
            {
                osg::ref_ptr<osgTerrain::ImageLayer> imageLayer = new osgTerrain::ImageLayer;
                imageLayer->setImage(image.get());
                imageLayer->setLocator(locator.get());
                
                terrain->setColorLayer(imageLayer.get());
                
                osg::notify(osg::NOTICE)<<"created Color osgTerrain::ImageLayer"<<std::endl;
            }
            else
            {
                osg::notify(osg::NOTICE)<<"failed to create osgTerrain::ImageLayer"<<std::endl;
            }
        }

    } while (readParameter);
    

    osg::ref_ptr<osgTerrain::GeometryTechnique> geometryTechnique = new osgTerrain::GeometryTechnique;
    
    terrain->setTerrainTechnique(geometryTechnique.get());
    
    if (!terrain) return 0;
    
    return 0;

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData(terrain.get());

    return viewer.run();
}
