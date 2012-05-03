#include <osgSim/BlinkSequence>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkPulseData( const osgSim::BlinkSequence& bs )
{
    return bs.getNumPulses()>0;
}

static bool readPulseData( osgDB::InputStream& is, osgSim::BlinkSequence& bs )
{
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        double length = 0.0;
        osg::Vec4 color;
        is >> length >> color;
        bs.addPulse( length, color );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writePulseData( osgDB::OutputStream& os, const osgSim::BlinkSequence& bs )
{
    unsigned int size = bs.getNumPulses();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        double length = 0.0;
        osg::Vec4 color;
        bs.getPulse( i, length, color );
        os << length << color << std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgSim_BlinkSequence,
                         new osgSim::BlinkSequence,
                         osgSim::BlinkSequence,
                         "osg::Object osgSim::BlinkSequence" )
{
    ADD_DOUBLE_SERIALIZER( PhaseShift, 0.0 );  // _phaseShift
    ADD_USER_SERIALIZER( PulseData );  // _pulseData
    ADD_OBJECT_SERIALIZER( SequenceGroup, osgSim::SequenceGroup, NULL );  // _sequenceGroup
}
