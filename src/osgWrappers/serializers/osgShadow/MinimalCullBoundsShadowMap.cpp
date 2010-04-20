#include <osgShadow/MinimalCullBoundsShadowMap>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgShadow_MinimalCullBoundsShadowMap,
                         new osgShadow::MinimalCullBoundsShadowMap,
                         osgShadow::MinimalCullBoundsShadowMap,
                         "osg::Object osgShadow::ShadowTechnique osgShadow::ViewDependentShadowTechnique "
                         "osgShadow::DebugShadowMap osgShadow::StandardShadowMap osgShadow::MinimalShadowMap "
                         "osgShadow::MinimalCullBoundsShadowMap" )
{
}
