#include <osgShadow/ShadowVolume>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgShadow_ShadowVolume,
                         new osgShadow::ShadowVolume,
                         osgShadow::ShadowVolume,
                         "osg::Object osgShadow::ShadowTechnique osgShadow::ShadowVolume" )
{
    BEGIN_ENUM_SERIALIZER4( osgShadow::ShadowVolumeGeometry, DrawMode, STENCIL_TWO_SIDED );
        ADD_ENUM_CLASS_VALUE( osgShadow::ShadowVolumeGeometry, GEOMETRY );
        ADD_ENUM_CLASS_VALUE( osgShadow::ShadowVolumeGeometry, STENCIL_TWO_PASS );
        ADD_ENUM_CLASS_VALUE( osgShadow::ShadowVolumeGeometry, STENCIL_TWO_SIDED );
    END_ENUM_SERIALIZER();  // _drawMode

    ADD_BOOL_SERIALIZER( DynamicShadowVolumes, false );  // _dynamicShadowVolumes
}
