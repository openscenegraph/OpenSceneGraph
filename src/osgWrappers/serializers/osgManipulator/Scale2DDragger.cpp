#include <osgManipulator/Scale2DDragger>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#define HANDLENODE_FUNC( PROP ) \
    static bool check##PROP( const osgManipulator::Scale2DDragger& dragger ) \
    { return dragger.get##PROP()!=NULL; } \
    static bool read##PROP( osgDB::InputStream& is, osgManipulator::Scale2DDragger& dragger ) { \
        osg::Node* node = dynamic_cast<osg::Node*>( is.readObject() ); \
        if ( node ) dragger.set##PROP( *node ); return true; \
    } \
    static bool write##PROP( osgDB::OutputStream& os, const osgManipulator::Scale2DDragger& dragger ) { \
        os << dragger.get##PROP(); return true; \
    }

HANDLENODE_FUNC( TopLeftHandleNode )
HANDLENODE_FUNC( BottomLeftHandleNode )
HANDLENODE_FUNC( TopRightHandleNode )
HANDLENODE_FUNC( BottomRightHandleNode )

REGISTER_OBJECT_WRAPPER( osgManipulator_Scale2DDragger,
                         new osgManipulator::Scale2DDragger,
                         osgManipulator::Scale2DDragger,
                         "osg::Object osg::Node osg::Transform osg::MatrixTransform osgManipulator::Dragger "
                         "osgManipulator::Scale2DDragger" )
{
    ADD_VEC2D_SERIALIZER( MinScale, osg::Vec2d() );// _minScale
    ADD_VEC4_SERIALIZER( Color, osg::Vec4() );  // _color
    ADD_VEC4_SERIALIZER( PickColor, osg::Vec4() );  // _pickColor
    ADD_USER_SERIALIZER( TopLeftHandleNode );  // _topLeftHandleNode
    ADD_USER_SERIALIZER( BottomLeftHandleNode );  // _bottomLeftHandleNode
    ADD_USER_SERIALIZER( TopRightHandleNode );  // _topRightHandleNode
    ADD_USER_SERIALIZER( BottomRightHandleNode );  // _bottomRightHandleNode
    ADD_VEC2D_SERIALIZER( TopLeftHandlePosition, osg::Vec2d() );
    ADD_VEC2D_SERIALIZER( BottomLeftHandlePosition, osg::Vec2d() );
    ADD_VEC2D_SERIALIZER( TopRightHandlePosition, osg::Vec2d() );
    ADD_VEC2D_SERIALIZER( BottomRightHandlePosition, osg::Vec2d() );
}
