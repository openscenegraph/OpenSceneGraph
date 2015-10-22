#include <osgFX/AnisotropicLighting>
#include <osgDB/ReadFile>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkLightingMap( const osgFX::AnisotropicLighting& effect )
{
    if ( !effect.getLightingMap() ) return false;
    return !effect.getLightingMap()->getFileName().empty();
}

static bool readLightingMap( osgDB::InputStream& is, osgFX::AnisotropicLighting& effect )
{
    std::string fileName; is.readWrappedString( fileName );
    osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile(fileName, is.getOptions());
    effect.setLightingMap( image.get() );
    return true;
}

static bool writeLightingMap( osgDB::OutputStream& os, const osgFX::AnisotropicLighting& effect )
{
    os.writeWrappedString( effect.getLightingMap()->getFileName() );
    os << std::endl; return true;
}

REGISTER_OBJECT_WRAPPER( osgFX_AnisotropicLighting,
                         new osgFX::AnisotropicLighting,
                         osgFX::AnisotropicLighting,
                         "osg::Object osg::Node osg::Group osgFX::Effect osgFX::AnisotropicLighting" )
{
    ADD_INT_SERIALIZER( LightNumber, 0 );  // _lightnum
    ADD_USER_SERIALIZER( LightingMap );  // _texture
}
