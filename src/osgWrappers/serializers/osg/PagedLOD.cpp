#include <osg/PagedLOD>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkRangeDataList( const osg::PagedLOD& node )
{
    return node.getNumFileNames()>0;
}

static bool readRangeDataList( osgDB::InputStream& is, osg::PagedLOD& node )
{
    unsigned int size = 0; is >> size >> osgDB::BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        std::string name; is.readWrappedString( name );
        float offset, scale;
        double timeStamp;
        int frameNumber;
        is >> offset >> scale >> timeStamp >> frameNumber;
        
        node.setFileName( i, name );
        node.setPriorityOffset( i, offset );
        node.setPriorityScale( i, scale );
        node.setTimeStamp( i, timeStamp );
        node.setFrameNumber( i, frameNumber );
    }
    is >> osgDB::END_BRACKET;
    return true;
}

static bool writeRangeDataList( osgDB::OutputStream& os, const osg::PagedLOD& node )
{
    unsigned int size = node.getNumFileNames();
    os << size << osgDB::BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os.writeWrappedString( node.getFileName(i) );
        os << node.getPriorityOffset(i) << node.getPriorityScale(i)
           << node.getTimeStamp(i) << node.getFrameNumber(i) << std::endl;
    }
    os << osgDB::END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( PagedLOD,
                         new osg::PagedLOD,
                         osg::PagedLOD,
                         "osg::Object osg::Node osg::LOD osg::PagedLOD" )
{
    // Note: osg::Group is not in the list to prevent recording dynamic loaded children
    
    ADD_STRING_SERIALIZER( DatabasePath, "" );  // _databasePath
    ADD_INT_SERIALIZER( FrameNumberOfLastTraversal, 0 );  // _frameNumberOfLastTraversal
    ADD_UINT_SERIALIZER( NumChildrenThatCannotBeExpired, 0 );  // _numChildrenThatCannotBeExpired
    ADD_BOOL_SERIALIZER( DisableExternalChildrenPaging, false );  // _disableExternalChildrenPaging
    ADD_USER_SERIALIZER( RangeDataList );  // _perRangeDataList
}
