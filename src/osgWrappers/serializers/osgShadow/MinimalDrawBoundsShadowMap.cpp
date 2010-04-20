#include <osgShadow/MinimalDrawBoundsShadowMap>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgShadow_MinimalDrawBoundsShadowMap,
                         new osgShadow::MinimalDrawBoundsShadowMap,
                         osgShadow::MinimalDrawBoundsShadowMap,
                         "osg::Object osgShadow::ShadowTechnique osgShadow::ViewDependentShadowTechnique "
                         "osgShadow::DebugShadowMap osgShadow::StandardShadowMap osgShadow::MinimalShadowMap "
                         "osgShadow::MinimalDrawBoundsShadowMap" )
{
}
