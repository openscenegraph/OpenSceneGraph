#include <osgVolume/Property>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkProperties( const osgVolume::CompositeProperty& prop )
{
    return prop.getNumProperties()>0;
}

static bool readProperties( osgDB::InputStream& is, osgVolume::CompositeProperty& prop )
{
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osgVolume::Property* child = dynamic_cast<osgVolume::Property*>( is.readObject() );
        if ( child ) prop.addProperty( child );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeProperties( osgDB::OutputStream& os, const osgVolume::CompositeProperty& prop )
{
    unsigned int size = prop.getNumProperties();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << prop.getProperty(i);
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgVolume_CompositeProperty,
                         new osgVolume::CompositeProperty,
                         osgVolume::CompositeProperty,
                         "osg::Object osgVolume::Property osgVolume::CompositeProperty" )
{
    ADD_USER_SERIALIZER( Properties );  // _properties
}
