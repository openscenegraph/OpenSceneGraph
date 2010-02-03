#include <osg/ClearNode>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#ifndef GL_ACCUM_BUFFER_BIT
    #define GL_ACCUM_BUFFER_BIT 0x00000200
#endif

// _clearMask
static bool checkClearMask( const osg::ClearNode& node )
{
    return node.getClearMask()!=(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

static bool readClearMask( osgDB::InputStream& is, osg::ClearNode& node )
{
    GLbitfield mask = GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT;
    if ( is.isBinary() )
    {
        int maskValue; is >> maskValue;
        mask = (GLbitfield)maskValue;
    }
    else
    {
        std::string maskSetString; is >> maskSetString;
        osgDB::StringList maskList; osgDB::split( maskSetString, maskList, '|' );
        for ( unsigned int i=0; i<maskList.size(); ++i )
        {
            const std::string& maskValue = maskList[i];
            if ( maskValue=="COLOR" ) mask |= GL_COLOR_BUFFER_BIT;
            else if ( maskValue=="DEPTH" ) mask |= GL_DEPTH_BUFFER_BIT;
            else if ( maskValue=="ACCUM" ) mask |= GL_ACCUM_BUFFER_BIT;
            else if ( maskValue=="STENCIL" ) mask |= GL_STENCIL_BUFFER_BIT;
        }
    }
    node.setClearMask( mask );
    return true;
}

static bool writeClearMask( osgDB::OutputStream& os, const osg::ClearNode& node )
{
    GLbitfield mask = node.getClearMask();
    if ( os.isBinary() )
        os << (int)mask;
    else
    {
        std::string maskString;
        if ( mask==GL_COLOR_BUFFER_BIT ) maskString += std::string("COLOR|");
        if ( mask==GL_DEPTH_BUFFER_BIT ) maskString += std::string("DEPTH|");
        if ( mask==GL_ACCUM_BUFFER_BIT ) maskString += std::string("ACCUM|");
        if ( mask==GL_STENCIL_BUFFER_BIT ) maskString += std::string("STENCIL|");
        if ( !maskString.size() ) maskString = std::string("NONE|");
        os << maskString.substr(0, maskString.size()-1) << std::endl;
    }
    return true;
}

REGISTER_OBJECT_WRAPPER( ClearNode,
                         new osg::ClearNode,
                         osg::ClearNode,
                         "osg::Object osg::Node osg::Group osg::ClearNode" )
{
    ADD_BOOL_SERIALIZER( RequiresClear, true );  // _requiresClear
    ADD_VEC4_SERIALIZER( ClearColor, osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f) );  // _clearColor
    ADD_USER_SERIALIZER( ClearMask );  // _clearMask
}
