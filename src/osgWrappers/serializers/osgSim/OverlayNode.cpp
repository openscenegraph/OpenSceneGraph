#include <osgSim/OverlayNode>
#include <osg/TexEnv>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgSim_OverlayNode,
                         new osgSim::OverlayNode,
                         osgSim::OverlayNode,
                         "osg::Object osg::Node osg::Group osgSim::OverlayNode" )
{
    BEGIN_ENUM_SERIALIZER( OverlayTechnique, OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY );
        ADD_ENUM_VALUE( OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY );
        ADD_ENUM_VALUE( VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY );
        ADD_ENUM_VALUE( VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY );
    END_ENUM_SERIALIZER();  // _overlayTechnique

    ADD_OBJECT_SERIALIZER( OverlaySubgraph, osg::Node, NULL );  // _overlaySubgraph
    ADD_GLENUM_SERIALIZER( TexEnvMode, GLenum, GL_DECAL );  // _texEnvMode
    ADD_UINT_SERIALIZER( OverlayTextureUnit, 1 );  // _textureUnit
    ADD_UINT_SERIALIZER( OverlayTextureSizeHint, 1024 );  // _textureSizeHint
    ADD_VEC4_SERIALIZER( OverlayClearColor, osg::Vec4() );  // _overlayClearColor
    ADD_BOOL_SERIALIZER( ContinuousUpdate, false );  // _continuousUpdate
    ADD_DOUBLE_SERIALIZER( OverlayBaseHeight, -100.0 );  // _overlayBaseHeight
}
