#include <osgTerrain/Layer>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkValidDataOperator( const osgTerrain::Layer& layer )
{
    return layer.getValidDataOperator()!=NULL;
}

static bool readValidDataOperator( osgDB::InputStream& is, osgTerrain::Layer& layer )
{
    unsigned int type; is >> type;
    switch ( type )
    {
    case 1:
        {
            float value; is >> value;
            layer.setValidDataOperator( new osgTerrain::NoDataValue(value) );
        }
        break;
    case 2:
        {
            float min, max; is >> min >> max;
            layer.setValidDataOperator( new osgTerrain::ValidRange(min, max) );
        }
        break;
    default:
        break;
    }
    return true;
}

static bool writeValidDataOperator( osgDB::OutputStream& os, const osgTerrain::Layer& layer )
{
    const osgTerrain::NoDataValue* ndv = dynamic_cast<const osgTerrain::NoDataValue*>( layer.getValidDataOperator() );
    if ( ndv )
    {
        os << (unsigned int)1 << ndv->getValue() << std::endl;
        return true;
    }

    const osgTerrain::ValidRange* vr = dynamic_cast<const osgTerrain::ValidRange*>( layer.getValidDataOperator() );
    if ( vr )
    {
        os << (unsigned int)2 << vr->getMinValue() << vr->getMaxValue() << std::endl;
        return true;
    }

    os << (unsigned int)0 << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgTerrain_Layer,
                         new osgTerrain::Layer,
                         osgTerrain::Layer,
                         "osg::Object osgTerrain::Layer" )
{
    ADD_STRING_SERIALIZER( FileName, "" );  // _filename
    ADD_OBJECT_SERIALIZER( Locator, osgTerrain::Locator, NULL );  // _locator
    ADD_UINT_SERIALIZER( MinLevel, 0 );  // _minLevel
    ADD_UINT_SERIALIZER( MaxLevel, MAXIMUM_NUMBER_OF_LEVELS );  // _maxLevel
    ADD_USER_SERIALIZER( ValidDataOperator );  // _validDataOperator
    ADD_VEC4_SERIALIZER( DefaultValue, osg::Vec4() );  // _defaultValue
    ADD_GLENUM_SERIALIZER( MinFilter, osg::Texture::FilterMode, osg::Texture::LINEAR_MIPMAP_LINEAR );  // _minFilter
    ADD_GLENUM_SERIALIZER( MagFilter, osg::Texture::FilterMode, osg::Texture::LINEAR );  // _magFilter
}
