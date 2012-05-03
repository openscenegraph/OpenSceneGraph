#include <osg/Sequence>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( Sequence,
                         new osg::Sequence,
                         osg::Sequence,
                         "osg::Object osg::Node osg::Group osg::Sequence" )
{
    ADD_LIST_SERIALIZER( TimeList, std::vector<double> );  // _frameTime

    BEGIN_ENUM_SERIALIZER( LoopMode, LOOP );
        ADD_ENUM_VALUE( LOOP );
        ADD_ENUM_VALUE( SWING );
    END_ENUM_SERIALIZER();  // _loopMode

    ADD_INT_SERIALIZER( Begin, 0 );  // _begin
    ADD_INT_SERIALIZER( End, -1 );  // _end
    ADD_FLOAT_SERIALIZER( Speed, 0.0f );  // _speed
    ADD_INT_SERIALIZER( NumRepeats, -1 );  // _nreps
    ADD_DOUBLE_SERIALIZER( DefaultTime, 1.0 );  // _defaultTime
    ADD_DOUBLE_SERIALIZER( LastFrameTime, 0.0 );  // _lastFrameTime

    BEGIN_ENUM_SERIALIZER2( Mode, osg::Sequence::SequenceMode, STOP );
        ADD_ENUM_VALUE( START );
        ADD_ENUM_VALUE( STOP );
        ADD_ENUM_VALUE( PAUSE );
        ADD_ENUM_VALUE( RESUME );
    END_ENUM_SERIALIZER();  // _mode

    ADD_BOOL_SERIALIZER( Sync, false );  // _sync
    ADD_BOOL_SERIALIZER( ClearOnStop, false );  // _clearOnStop
}
