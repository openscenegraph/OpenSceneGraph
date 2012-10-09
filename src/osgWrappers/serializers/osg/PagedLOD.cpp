#include <osg/PagedLOD>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include <osgDB/Options>

// _databasePath
static bool checkDatabasePath( const osg::PagedLOD& node )
{
    return true;
}

static bool readDatabasePath( osgDB::InputStream& is, osg::PagedLOD& node )
{
    bool hasPath; is >> hasPath;
    if ( !hasPath )
    {
        if ( is.getOptions() && !is.getOptions()->getDatabasePathList().empty() )
        {
            const std::string& optionPath = is.getOptions()->getDatabasePathList().front();
            if ( !optionPath.empty() ) node.setDatabasePath( optionPath );
        }
    }
    else
    {
        std::string path; is.readWrappedString( path );
        node.setDatabasePath( path );
    }
    return true;
}

static bool writeDatabasePath( osgDB::OutputStream& os, const osg::PagedLOD& node )
{
    os << (!node.getDatabasePath().empty());
    if ( !node.getDatabasePath().empty() )
        os.writeWrappedString( node.getDatabasePath() );
    os << std::endl;
    return true;
}

// _perRangeDataList
static bool checkRangeDataList( const osg::PagedLOD& node )
{
    return node.getNumFileNames()>0;
}

static bool readRangeDataList( osgDB::InputStream& is, osg::PagedLOD& node )
{
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        std::string name; is.readWrappedString( name );
        node.setFileName( i, name );
    }
    is >> is.END_BRACKET;

    size = 0; is >> is.PROPERTY("PriorityList") >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        float offset, scale;
        is >> offset >> scale;

        node.setPriorityOffset( i, offset );
        node.setPriorityScale( i, scale );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeRangeDataList( osgDB::OutputStream& os, const osg::PagedLOD& node )
{
    unsigned int size = node.getNumFileNames();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os.writeWrappedString( node.getFileName(i) );
        os << std::endl;
    }
    os << os.END_BRACKET << std::endl;

    size = node.getNumPriorityOffsets();
    os << os.PROPERTY("PriorityList") << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << node.getPriorityOffset(i) << node.getPriorityScale(i) << std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

// _children
static bool checkChildren( const osg::PagedLOD& node )
{
    return node.getNumChildren()>0;
}

static bool readChildren( osgDB::InputStream& is, osg::PagedLOD& node )
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

static bool writeChildren( osgDB::OutputStream& os, const osg::PagedLOD& node )
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

REGISTER_OBJECT_WRAPPER( PagedLOD,
                         new osg::PagedLOD,
                         osg::PagedLOD,
                         "osg::Object osg::Node osg::LOD osg::PagedLOD" )
{
    // Note: osg::Group is not in the list to prevent recording dynamic loaded children

    ADD_USER_SERIALIZER( DatabasePath );  // _databasePath
    ADD_UINT_SERIALIZER( FrameNumberOfLastTraversal, 0 );  // _frameNumberOfLastTraversal, note, not required, removed from soversion 70 onwwards, see below
    ADD_UINT_SERIALIZER( NumChildrenThatCannotBeExpired, 0 );  // _numChildrenThatCannotBeExpired
    ADD_BOOL_SERIALIZER( DisableExternalChildrenPaging, false );  // _disableExternalChildrenPaging
    ADD_USER_SERIALIZER( RangeDataList );  // _perRangeDataList
    ADD_USER_SERIALIZER( Children );  // _children (which are not loaded from external)

    {
        UPDATE_TO_VERSION_SCOPED( 70 )
        REMOVE_SERIALIZER( FrameNumberOfLastTraversal );
    }



}
