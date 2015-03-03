#include <osg/ClearNode>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#ifndef GL_ACCUM_BUFFER_BIT
    #define GL_ACCUM_BUFFER_BIT 0x00000200
#endif

REGISTER_OBJECT_WRAPPER( ClearNode,
                         new osg::ClearNode,
                         osg::ClearNode,
                         "osg::Object osg::Node osg::Group osg::ClearNode" )
{
    ADD_BOOL_SERIALIZER( RequiresClear, true );  // _requiresClear
    ADD_VEC4_SERIALIZER( ClearColor, osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f) );  // _clearColor
    BEGIN_BITFLAGS_SERIALIZER(ClearMask,GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        ADD_BITFLAG_VALUE(COLOR, GL_COLOR_BUFFER_BIT);
        ADD_BITFLAG_VALUE(DEPTH, GL_DEPTH_BUFFER_BIT);
        ADD_BITFLAG_VALUE(ACCUM, GL_ACCUM_BUFFER_BIT);
        ADD_BITFLAG_VALUE(STENCIL, GL_STENCIL_BUFFER_BIT);
    END_BITFLAGS_SERIALIZER();
}
