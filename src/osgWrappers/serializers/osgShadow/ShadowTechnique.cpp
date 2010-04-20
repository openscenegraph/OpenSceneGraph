#include <osgShadow/ShadowTechnique>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgShadow_ShadowTechnique,
                         new osgShadow::ShadowTechnique,
                         osgShadow::ShadowTechnique,
                         "osg::Object osgShadow::ShadowTechnique" )
{
}
