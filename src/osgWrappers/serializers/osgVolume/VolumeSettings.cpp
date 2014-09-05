#include <osgVolume/Volume>
#include <osgVolume/VolumeSettings>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgVolume_VolumeSettings,
                         new osgVolume::VolumeSettings,
                         osgVolume::VolumeSettings,
                         "osg::Object osgVolume::Property osgVolume::VolumeSettings" )
{
    ADD_STRING_SERIALIZER( Filename, "" );

    BEGIN_ENUM_SERIALIZER( Technique, MultiPass );
        ADD_ENUM_VALUE( FixedFunction );
        ADD_ENUM_VALUE( RayTraced );
        ADD_ENUM_VALUE( MultiPass );
    END_ENUM_SERIALIZER();

    BEGIN_ENUM_SERIALIZER( ShadingModel, Standard );
        ADD_ENUM_VALUE( Standard );
        ADD_ENUM_VALUE( Light );
        ADD_ENUM_VALUE( Isosurface );
        ADD_ENUM_VALUE( MaximumIntensityProjection );
    END_ENUM_SERIALIZER();

    ADD_FLOAT_SERIALIZER( SampleRatio, 1.0f );
    ADD_FLOAT_SERIALIZER( SampleRatioWhenMoving, 1.0f );
    ADD_FLOAT_SERIALIZER( Cutoff, 0.0f );
    ADD_FLOAT_SERIALIZER( Transparency, 1.0f );

    ADD_OBJECT_SERIALIZER_NO_SET( SampleRatioProperty, osgVolume::SampleRatioProperty, NULL );
    ADD_OBJECT_SERIALIZER_NO_SET( SampleRatioWhenMovingProperty, osgVolume::SampleRatioWhenMovingProperty, NULL );
    ADD_OBJECT_SERIALIZER_NO_SET( CutoffProperty, osgVolume::AlphaFuncProperty, NULL );
    ADD_OBJECT_SERIALIZER_NO_SET( TransparencyProperty, osgVolume::TransparencyProperty, NULL );
    ADD_OBJECT_SERIALIZER_NO_SET( IsoSurfaceProperty, osgVolume::IsoSurfaceProperty, NULL ); 


}
