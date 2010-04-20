#include <osgShadow/DebugShadowMap>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgShadow_DebugShadowMap,
                         new osgShadow::DebugShadowMap,
                         osgShadow::DebugShadowMap,
                         "osg::Object osgShadow::ShadowTechnique osgShadow::ViewDependentShadowTechnique "
                         "osgShadow::DebugShadowMap" )
{
    ADD_BOOL_SERIALIZER( DebugDraw, false );  // _doDebugDraw
}
