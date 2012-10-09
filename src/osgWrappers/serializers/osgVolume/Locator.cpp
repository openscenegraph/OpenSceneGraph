#include <osgVolume/Locator>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkLocatorCallbacks( const osgVolume::Locator& locator )
{
    return locator.getLocatorCallbacks().size()>0;
}

static bool readLocatorCallbacks( osgDB::InputStream& is, osgVolume::Locator& locator )
{
    unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osgVolume::Locator::LocatorCallback* cb =
            dynamic_cast<osgVolume::Locator::LocatorCallback*>( is.readObject() );
        if ( cb ) locator.addCallback( cb );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeLocatorCallbacks( osgDB::OutputStream& os, const osgVolume::Locator& locator )
{
    const osgVolume::Locator::LocatorCallbacks& callbacks = locator.getLocatorCallbacks();
    os.writeSize( callbacks.size() ); os << os.BEGIN_BRACKET << std::endl;
    for ( osgVolume::Locator::LocatorCallbacks::const_iterator itr=callbacks.begin();
          itr!=callbacks.end(); ++itr )
    {
        os << itr->get();
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgVolume_Locator,
                         new osgVolume::Locator,
                         osgVolume::Locator,
                         "osg::Object osgVolume::Locator" )
{
    ADD_MATRIXD_SERIALIZER( Transform, osg::Matrixd() );  // _transform

    ADD_USER_SERIALIZER( LocatorCallbacks );
    {
        UPDATE_TO_VERSION_SCOPED( 90 )
        REMOVE_SERIALIZER( LocatorCallbacks );
    }
}
