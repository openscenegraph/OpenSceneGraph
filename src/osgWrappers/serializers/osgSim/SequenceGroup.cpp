#include <osgSim/BlinkSequence>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgSim_SequenceGroup,
                         new osgSim::SequenceGroup,
                         osgSim::SequenceGroup,
                         "osg::Object osgSim::SequenceGroup" )
{
    ADD_DOUBLE_SERIALIZER( BaseTime, 0.0 );  // _baseTime
}
