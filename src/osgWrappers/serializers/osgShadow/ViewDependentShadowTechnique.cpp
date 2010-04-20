#include <osgShadow/ViewDependentShadowTechnique>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgShadow_ViewDependentShadowTechnique,
                         new osgShadow::ViewDependentShadowTechnique,
                         osgShadow::ViewDependentShadowTechnique,
                         "osg::Object osgShadow::ShadowTechnique osgShadow::ViewDependentShadowTechnique" )
{
}
