#include <osg/ShapeDrawable>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( TessellationHints,
                         new osg::TessellationHints,
                         osg::TessellationHints,
                         "osg::Object osg::TessellationHints" )
{
    BEGIN_ENUM_SERIALIZER( TessellationMode, USE_SHAPE_DEFAULTS );
        ADD_ENUM_VALUE( USE_SHAPE_DEFAULTS );
        ADD_ENUM_VALUE( USE_TARGET_NUM_FACES );
    END_ENUM_SERIALIZER();  // _TessellationMode

    ADD_FLOAT_SERIALIZER( DetailRatio, 1.0f );  // _detailRatio
    ADD_UINT_SERIALIZER( TargetNumFaces, 100 );  // _targetNumFaces
    ADD_BOOL_SERIALIZER( CreateFrontFace, true );  // _createFrontFace
    ADD_BOOL_SERIALIZER( CreateBackFace, false );  // _createBackFace
    ADD_BOOL_SERIALIZER( CreateNormals, true );  // _createNormals
    ADD_BOOL_SERIALIZER( CreateTextureCoords, false );  // _createTextureCoords
    ADD_BOOL_SERIALIZER( CreateTop, true );  // _createTop
    ADD_BOOL_SERIALIZER( CreateBody, true );  // _createBody
    ADD_BOOL_SERIALIZER( CreateBottom, true );  // _createBottom
}
