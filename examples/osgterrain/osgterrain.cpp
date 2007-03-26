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


class MyLocator : public osgTerrain::Locator
{
public:
    
    
    MyLocator(double x, double y, double width, double height):
        _x(x),
        _y(y),
        _width(width),
        _height(height) {}
        
    
    bool convertLocalToModel(const osg::Vec3d& local, osg::Vec3d& world)
    {
        world.x() = _x + local.x()*_width;
        world.y() = _y + local.y()*_height;
        world.z() = _z + local.z();
        return true;
    }
    
    bool convertModelToWorld(const osg::Vec3d& world, osg::Vec3d& local)
    {
        local.x() = (world.x()- _x) / _width;
        local.y() = (world.y() - _y) / _height;
        local.z() = world.z() - _z;
        return true;
    }
    
    
    double _x;
    double _y;
    double _z;
    double _width;
    double _height;
};


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
    osg::ref_ptr<osgTerrain::Locator> locator = new MyLocator(0.0, 0.0, 1.0, 1.0);


    bool readParameter = false;
    do 
    {
        readParameter = false;
        std::string filename;
        
        if (arguments.read("-l",x,y,w,h))
        {
            locator = new MyLocator(x,y,w,h);
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
        
        if (arguments.read("-e",filename) || arguments.read("--elevation-image",filename))
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
