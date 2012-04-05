#include <osg/ProxyNode>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>

// _filenameList
static bool checkFileNames( const osg::ProxyNode& node )
{
    return node.getNumFileNames()>0;
}

static bool readFileNames( osgDB::InputStream& is, osg::ProxyNode& node )
{
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        std::string value;
        is.readWrappedString( value );
        node.setFileName( i, value );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeFileNames( osgDB::OutputStream& os, const osg::ProxyNode& node )
{
    os << node.getNumFileNames() << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<node.getNumFileNames(); ++i )
    {
        os.writeWrappedString( node.getFileName(i) );
        os << std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

// _children
static bool checkChildren( const osg::ProxyNode& node )
{
    return node.getNumChildren()>0;
}

static bool readChildren( osgDB::InputStream& is, osg::ProxyNode& node )
{
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osg::Node* child = dynamic_cast<osg::Node*>( is.readObject() );
        if ( child ) node.addChild( child );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeChildren( osgDB::OutputStream& os, const osg::ProxyNode& node )
{
    unsigned int size=node.getNumFileNames(), dynamicLoadedSize=0;
    for ( unsigned int i=0; i<size; ++i )
    {
        if ( !node.getFileName(i).empty() )
            dynamicLoadedSize++;
    }

    unsigned int realSize = size-dynamicLoadedSize; os << realSize;
    if ( realSize>0 )
    {
        os << os.BEGIN_BRACKET << std::endl;
        for ( unsigned int i=0; i<size; ++i )
        {
            if ( !node.getFileName(i).empty() ) continue;
            if ( i<node.getNumChildren() )
                os << node.getChild(i);
        }
        os << os.END_BRACKET;
    }
    os << std::endl;
    return true;
}

// _userDefinedCenter, _radius
static bool checkUserCenter( const osg::ProxyNode& node )
{
    return (node.getCenterMode()==osg::ProxyNode::USER_DEFINED_CENTER)||(node.getCenterMode()==osg::ProxyNode::UNION_OF_BOUNDING_SPHERE_AND_USER_DEFINED);
}

static bool readUserCenter( osgDB::InputStream& is, osg::ProxyNode& node )
{
    osg::Vec3d center; double radius;
    is >> center >> radius;
    node.setCenter( center ); node.setRadius( radius );
    return true;
}

static bool writeUserCenter( osgDB::OutputStream& os, const osg::ProxyNode& node )
{
    os << osg::Vec3d(node.getCenter()) << (double)node.getRadius() << std::endl;
    return true;
}

struct ProxyNodeFinishedObjectReadCallback : public osgDB::FinishedObjectReadCallback
{
    virtual void objectRead(osgDB::InputStream& is, osg::Object& obj)
    {
        osg::ProxyNode& proxyNode = static_cast<osg::ProxyNode&>(obj);

        if (proxyNode.getLoadingExternalReferenceMode() == osg::ProxyNode::LOAD_IMMEDIATELY)
        {
            for(unsigned int i=0; i<proxyNode.getNumFileNames(); i++)
            {
                if(i >= proxyNode.getNumChildren() && !proxyNode.getFileName(i).empty())
                {
                    osgDB::FilePathList& fpl = ((osgDB::ReaderWriter::Options*)is.getOptions())->getDatabasePathList();
                    fpl.push_front( fpl.empty() ? osgDB::getFilePath(proxyNode.getFileName(i)) : fpl.front()+'/'+ osgDB::getFilePath(proxyNode.getFileName(i)));
                    osg::Node* node = osgDB::readNodeFile(proxyNode.getFileName(i), is.getOptions());
                    fpl.pop_front();
                    if(node)
                        proxyNode.insertChild(i, node);
                }
            }
        }
    }
};

REGISTER_OBJECT_WRAPPER( ProxyNode,
                         new osg::ProxyNode,
                         osg::ProxyNode,
                         "osg::Object osg::Node osg::ProxyNode" )
{
    // Note: osg::Group is not in the list to prevent recording dynamic loaded children

    ADD_USER_SERIALIZER( FileNames );  // _filenameList
    ADD_USER_SERIALIZER( Children );  // _children (which are not loaded from external)
    ADD_STRING_SERIALIZER( DatabasePath, "" );  // _databasePath

    BEGIN_ENUM_SERIALIZER( LoadingExternalReferenceMode, LOAD_IMMEDIATELY );
        ADD_ENUM_VALUE( LOAD_IMMEDIATELY );
        ADD_ENUM_VALUE( DEFER_LOADING_TO_DATABASE_PAGER );
        ADD_ENUM_VALUE( NO_AUTOMATIC_LOADING );
    END_ENUM_SERIALIZER();  // _loadingExtReference

    BEGIN_ENUM_SERIALIZER( CenterMode, USE_BOUNDING_SPHERE_CENTER );
        ADD_ENUM_VALUE( USE_BOUNDING_SPHERE_CENTER );
        ADD_ENUM_VALUE( USER_DEFINED_CENTER );
        ADD_ENUM_VALUE( UNION_OF_BOUNDING_SPHERE_AND_USER_DEFINED );
    END_ENUM_SERIALIZER();  // _centerMode

    ADD_USER_SERIALIZER( UserCenter );  // _userDefinedCenter, _radius

    wrapper->addFinishedObjectReadCallback(new ProxyNodeFinishedObjectReadCallback());
}
