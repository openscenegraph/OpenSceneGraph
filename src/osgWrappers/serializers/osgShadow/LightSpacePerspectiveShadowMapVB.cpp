#include <osgShadow/LightSpacePerspectiveShadowMap>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgShadow_LightSpacePerspectiveShadowMapVB,
                         new osgShadow::LightSpacePerspectiveShadowMapVB,
                         osgShadow::LightSpacePerspectiveShadowMapVB,
                         "osg::Object osgShadow::ShadowTechnique osgShadow::ViewDependentShadowTechnique "
                         "osgShadow::DebugShadowMap osgShadow::StandardShadowMap osgShadow::MinimalShadowMap "
                         "osgShadow::LightSpacePerspectiveShadowMapVB" )
{
}
