#include <osgShadow/ShadowedScene>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgShadow_ShadowedScene,
                         new osgShadow::ShadowedScene,
                         osgShadow::ShadowedScene,
                         "osg::Object osg::Node osg::Group osgShadow::ShadowedScene" )
{
    ADD_HEXINT_SERIALIZER( ReceivesShadowTraversalMask, 0xffffffff );  // _receivesShadowTraversalMask
    ADD_HEXINT_SERIALIZER( CastsShadowTraversalMask, 0xffffffff );  // _castsShadowTraversalMask
    ADD_OBJECT_SERIALIZER( ShadowTechnique, osgShadow::ShadowTechnique, NULL );  // _shadowTechnique
}
