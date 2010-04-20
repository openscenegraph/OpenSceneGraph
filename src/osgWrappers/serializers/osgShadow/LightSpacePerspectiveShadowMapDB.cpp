#include <osgShadow/LightSpacePerspectiveShadowMap>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgShadow_LightSpacePerspectiveShadowMapDB,
                         new osgShadow::LightSpacePerspectiveShadowMapDB,
                         osgShadow::LightSpacePerspectiveShadowMapDB,
                         "osg::Object osgShadow::ShadowTechnique osgShadow::ViewDependentShadowTechnique "
                         "osgShadow::DebugShadowMap osgShadow::StandardShadowMap osgShadow::MinimalShadowMap "
                         "osgShadow::MinimalDrawBoundsShadowMap osgShadow::LightSpacePerspectiveShadowMapDB" )
{
}
