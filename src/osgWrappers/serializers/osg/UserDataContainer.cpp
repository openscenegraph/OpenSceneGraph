#include <osg/UserDataContainer>

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkUDC_UserData( const osg::DefaultUserDataContainer& udc )
{
    return dynamic_cast<const osg::Object*>(udc.getUserData())!=0;
}

static bool readUDC_UserData( osgDB::InputStream& is, osg::DefaultUserDataContainer& udc )
{
    is >> is.BEGIN_BRACKET;
    osg::ref_ptr<osg::Object> object = is.readObject();
    if(object) udc.setUserData(object);
    is >> is.END_BRACKET;
    return true;
}

static bool writeUDC_UserData( osgDB::OutputStream& os, const osg::DefaultUserDataContainer& udc )
{
    os << os.BEGIN_BRACKET << std::endl;
    os.writeObject(dynamic_cast<const osg::Object*>(udc.getUserData()));
    os << os.END_BRACKET << std::endl;
    return true;
}

// _descriptions
static bool checkUDC_Descriptions( const osg::DefaultUserDataContainer& udc )
{
    return udc.getNumDescriptions()>0;
}

static bool readUDC_Descriptions( osgDB::InputStream& is, osg::DefaultUserDataContainer& udc )
{
    unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        std::string value;
        is.readWrappedString( value );
        udc.addDescription( value );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeUDC_Descriptions( osgDB::OutputStream& os, const osg::DefaultUserDataContainer& udc )
{
    const osg::UserDataContainer::DescriptionList& slist = udc.getDescriptions();
    os.writeSize(slist.size()); os << os.BEGIN_BRACKET << std::endl;
    for ( osg::UserDataContainer::DescriptionList::const_iterator itr=slist.begin();
          itr!=slist.end(); ++itr )
    {
        os.writeWrappedString( *itr );
        os << std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}


static bool checkUDC_UserObjects( const osg::DefaultUserDataContainer& udc )
{
    return udc.getNumUserObjects()>0;
}

static bool readUDC_UserObjects( osgDB::InputStream& is, osg::DefaultUserDataContainer& udc )
{
    unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
    for( unsigned int i=0; i<size; ++i )
    {
        osg::ref_ptr<osg::Object> read_object = is.readObject();
        if (read_object) udc.addUserObject( read_object );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeUDC_UserObjects( osgDB::OutputStream& os, const osg::DefaultUserDataContainer& udc )
{
    unsigned int numObjects = udc.getNumUserObjects();
    os.writeSize(numObjects); os << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<numObjects; ++i )
    {
        os << udc.getUserObject(i);
    }
    os << os.END_BRACKET << std::endl;
    return true;
}


namespace UserDataContainerNamespace
{
    REGISTER_OBJECT_WRAPPER( UserDataContainer,
                            0,
                            osg::UserDataContainer,
                            "osg::Object osg::UserDataContainer" )
    {
    }
}

namespace DefaultUserDataContainerNamespace
{
    REGISTER_OBJECT_WRAPPER( DefaultUserDataContainer,
                            new osg::DefaultUserDataContainer,
                            osg::DefaultUserDataContainer,
                            "osg::Object osg::UserDataContainer osg::DefaultUserDataContainer" )
    {
        ADD_USER_SERIALIZER( UDC_UserData );  // _userData
        ADD_USER_SERIALIZER( UDC_Descriptions );  // _descriptions
        ADD_USER_SERIALIZER( UDC_UserObjects );  // _userData
    }
}
