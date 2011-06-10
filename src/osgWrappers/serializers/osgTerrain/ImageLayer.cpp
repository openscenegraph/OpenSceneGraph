#include <osgTerrain/Layer>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#include <osgTerrain/TerrainTile>


static bool checkImage( const osgTerrain::ImageLayer& im )
{ return im.getImage() != NULL; }

static bool readImage( osgDB::InputStream& is, osgTerrain::ImageLayer& il )
{      
            
    if(!is.isBinary()) is >> osgDB::BEGIN_BRACKET;    

    bool deferExternalLayerLoading = osgTerrain::TerrainTile::getTileLoadedCallback().valid() ? 
            osgTerrain::TerrainTile::getTileLoadedCallback()->deferExternalLayerLoading() : false;          

    
    osg::ref_ptr<osg::Image> image = is.readImage(!deferExternalLayerLoading);
    if (image.valid())
    {
        if(image->valid()) 
        {
            il.setImage(image.get());          
        } 
    } 
    if(!is.isBinary()) is >> osgDB::END_BRACKET;        

   return true;
}

static bool writeImage( osgDB::OutputStream& os, const osgTerrain::ImageLayer& il )
{ 
    const osg::Image* image = il.getImage();   

    if(!os.isBinary()) os << osgDB::BEGIN_BRACKET << std::endl;                
    os.writeImage(image);
    if(!os.isBinary()) os << osgDB::END_BRACKET << std::endl; 
          
    return true;  
}

REGISTER_OBJECT_WRAPPER( osgTerrain_ImageLayer,
                         new osgTerrain::ImageLayer,
                         osgTerrain::ImageLayer,
                         "osg::Object osgTerrain::Layer osgTerrain::ImageLayer" )
{    
   ADD_USER_SERIALIZER( Image );   
}
