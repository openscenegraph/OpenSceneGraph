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
    is >> osgDB::PROPERTY("Flags") >> data._flags;
    is >> osgDB::PROPERTY("RelativePriority") >> data._relativePriority;
    is >> osgDB::PROPERTY("Transparency") >> data._transparency;
    is >> osgDB::PROPERTY("EffectID1") >> data._effectID1;
    is >> osgDB::PROPERTY("EffectID2") >> data._effectID2;
    is >> osgDB::PROPERTY("Significance") >> data._significance;
    return true;
}

static bool writeData( osgDB::OutputStream& os, const osgSim::ObjectRecordData& data )
{
    os << osgDB::PROPERTY("Flags") << data._flags << std::endl;
    os << osgDB::PROPERTY("RelativePriority") << data._relativePriority << std::endl;
    os << osgDB::PROPERTY("Transparency") << data._transparency << std::endl;
    os << osgDB::PROPERTY("EffectID1") << data._effectID1 << std::endl;
    os << osgDB::PROPERTY("EffectID2") << data._effectID2 << std::endl;
    os << osgDB::PROPERTY("Significance") << data._significance << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgSim_ObjectRecordData,
                         new osgSim::ObjectRecordData,
                         osgSim::ObjectRecordData,
                         "osg::Object osgSim::ObjectRecordData" )
{
    ADD_USER_SERIALIZER( Data );
}
