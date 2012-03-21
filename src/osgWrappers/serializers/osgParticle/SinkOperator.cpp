#include <osgParticle/SinkOperator>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgParticleSinkOperator,
                         new osgParticle::SinkOperator,
                         osgParticle::SinkOperator,
                         "osg::Object osgParticle::Operator osgParticle::DomainOperator osgParticle::SinkOperator" )
{
    BEGIN_ENUM_SERIALIZER( SinkTarget, SINK_POSITION );
        ADD_ENUM_VALUE( SINK_POSITION );
        ADD_ENUM_VALUE( SINK_VELOCITY );
        ADD_ENUM_VALUE( SINK_ANGULAR_VELOCITY );
    END_ENUM_SERIALIZER();  // _sinkTarget

    BEGIN_ENUM_SERIALIZER( SinkStrategy, SINK_INSIDE );
        ADD_ENUM_VALUE( SINK_INSIDE );
        ADD_ENUM_VALUE( SINK_OUTSIDE );
    END_ENUM_SERIALIZER();  // _sinkStrategy
}
