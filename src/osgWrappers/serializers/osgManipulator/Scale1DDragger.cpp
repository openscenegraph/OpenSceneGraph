#include <osgManipulator/Scale1DDragger>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#define HANDLENODE_FUNC( PROP ) \
    static bool check##PROP( const osgManipulator::Scale1DDragger& dragger ) \
    { return dragger.get##PROP()!=NULL; } \
    static bool read##PROP( osgDB::InputStream& is, osgManipulator::Scale1DDragger& dragger ) { \
        osg::Node* node = dynamic_cast<osg::Node*>( is.readObject() ); \
        if ( node ) dragger.set##PROP( *node ); return true; \
    } \
    static bool write##PROP( osgDB::OutputStream& os, const osgManipulator::Scale1DDragger& dragger ) { \
        os << dragger.get##PROP(); return true; \
    }

HANDLENODE_FUNC( LeftHandleNode )
HANDLENODE_FUNC( RightHandleNode )

REGISTER_OBJECT_WRAPPER( osgManipulator_Scale1DDragger,
                         new osgManipulator::Scale1DDragger,
                         osgManipulator::Scale1DDragger,
                         "osg::Object osg::Node osg::Transform osg::MatrixTransform osgManipulator::Dragger "
                         "osgManipulator::Scale1DDragger" )
{
    ADD_DOUBLE_SERIALIZER( MinScale, 0.0 );// _minScale
    ADD_VEC4_SERIALIZER( Color, osg::Vec4() );  // _color
    ADD_VEC4_SERIALIZER( PickColor, osg::Vec4() );  // _pickColor
    ADD_USER_SERIALIZER( LeftHandleNode );  // _leftHandleNode
    ADD_USER_SERIALIZER( RightHandleNode );  // _rightHandleNode
    ADD_DOUBLE_SERIALIZER( LeftHandlePosition, 0.0 );
    ADD_DOUBLE_SERIALIZER( RightHandlePosition, 0.0 );
}
