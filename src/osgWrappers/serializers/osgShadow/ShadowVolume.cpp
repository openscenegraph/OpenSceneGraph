#include <osgShadow/ShadowTechnique>

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

namespace osgShadow
{

class ShadowVolumeGeometry
{
    public :
        enum DrawMode
        {
            GEOMETRY,
            STENCIL_TWO_PASS,
            STENCIL_TWO_SIDED
        };
};

class ShadowVolume : public osgShadow::ShadowTechnique
{
public:
        ShadowVolume() {}

        ShadowVolume(const ShadowVolume& es, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY) {}

        META_Object(osgShadow, ShadowVolume);

        void setDrawMode(osgShadow::ShadowVolumeGeometry::DrawMode) {}
        osgShadow::ShadowVolumeGeometry::DrawMode getDrawMode() const { return osgShadow::ShadowVolumeGeometry::GEOMETRY; }

        void setDynamicShadowVolumes(bool) {}
        bool getDynamicShadowVolumes() const { return false; }

        virtual void resizeGLObjectBuffers(unsigned int maxSize) {}
        virtual void releaseGLObjects(osg::State* = 0) const {}
};

}

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
