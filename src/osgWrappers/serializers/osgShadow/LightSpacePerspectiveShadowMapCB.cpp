#include <osgShadow/LightSpacePerspectiveShadowMap>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgShadow_LightSpacePerspectiveShadowMapCB,
                         new osgShadow::LightSpacePerspectiveShadowMapCB,
                         osgShadow::LightSpacePerspectiveShadowMapCB,
                         "osg::Object osgShadow::ShadowTechnique osgShadow::ViewDependentShadowTechnique "
                         "osgShadow::DebugShadowMap osgShadow::StandardShadowMap osgShadow::MinimalShadowMap "
                         "osgShadow::MinimalCullBoundsShadowMap osgShadow::LightSpacePerspectiveShadowMapCB" )
{
}
