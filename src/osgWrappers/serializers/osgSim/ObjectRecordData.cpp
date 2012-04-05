#include <osgSim/ObjectRecordData>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkData( const osgSim::ObjectRecordData& data )
{
    return true;
}

static bool readData( osgDB::InputStream& is, osgSim::ObjectRecordData& data )
{
    is >> is.PROPERTY("Flags") >> data._flags;
    is >> is.PROPERTY("RelativePriority") >> data._relativePriority;
    is >> is.PROPERTY("Transparency") >> data._transparency;
    is >> is.PROPERTY("EffectID1") >> data._effectID1;
    is >> is.PROPERTY("EffectID2") >> data._effectID2;
    is >> is.PROPERTY("Significance") >> data._significance;
    return true;
}

static bool writeData( osgDB::OutputStream& os, const osgSim::ObjectRecordData& data )
{
    os << os.PROPERTY("Flags") << data._flags << std::endl;
    os << os.PROPERTY("RelativePriority") << data._relativePriority << std::endl;
    os << os.PROPERTY("Transparency") << data._transparency << std::endl;
    os << os.PROPERTY("EffectID1") << data._effectID1 << std::endl;
    os << os.PROPERTY("EffectID2") << data._effectID2 << std::endl;
    os << os.PROPERTY("Significance") << data._significance << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgSim_ObjectRecordData,
                         new osgSim::ObjectRecordData,
                         osgSim::ObjectRecordData,
                         "osg::Object osgSim::ObjectRecordData" )
{
    ADD_USER_SERIALIZER( Data );
}
