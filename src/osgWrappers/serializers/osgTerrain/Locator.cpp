#include <osgTerrain/Locator>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgTerrain_Locator,
                         new osgTerrain::Locator,
                         osgTerrain::Locator,
                         "osg::Object osgTerrain::Locator" )
{
    BEGIN_ENUM_SERIALIZER( CoordinateSystemType, PROJECTED );
        ADD_ENUM_VALUE( GEOCENTRIC );
        ADD_ENUM_VALUE( GEOGRAPHIC );
        ADD_ENUM_VALUE( PROJECTED );
    END_ENUM_SERIALIZER();  // _coordinateSystemType

    ADD_STRING_SERIALIZER( Format, "" );  // _format
    ADD_STRING_SERIALIZER( CoordinateSystem, "" );  // _cs
    ADD_OBJECT_SERIALIZER( EllipsoidModel, osg::EllipsoidModel, NULL );  // _ellipsoidModel
    ADD_MATRIXD_SERIALIZER( Transform, osg::Matrixd() );  // _transform
    ADD_BOOL_SERIALIZER( DefinedInFile, false );  // _definedInFile
    ADD_BOOL_SERIALIZER( TransformScaledByResolution, false );  // _transformScaledByResolution
}
