#include <osgSim/ObjectRecordData>
#include <osg/io_utils>

#include <iostream>
#include <string>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

//#include <map>

bool ObjectRecordData_readLocalData( osg::Object &obj, osgDB::Input &fr );
bool ObjectRecordData_writeLocalData( const osg::Object &obj, osgDB::Output &fw );

REGISTER_DOTOSGWRAPPER(ObjectRecordData_Proxy)
(
    new osgSim::ObjectRecordData,
    "ObjectRecordData",
    "Object ObjectRecordData",
    &ObjectRecordData_readLocalData,
    &ObjectRecordData_writeLocalData,
    osgDB::DotOsgWrapper::READ_AND_WRITE
);

bool ObjectRecordData_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    bool iteratorAdvanced = false;
    osgSim::ObjectRecordData &ord = static_cast<osgSim::ObjectRecordData&>(obj);

    if (fr.matchSequence("flags %i"))
    {
        unsigned int flags;
        fr[1].getUInt( flags );
        ord._flags = flags;
        fr += 2;
        iteratorAdvanced = true;
    }
    if (fr.matchSequence("relativePriority %i"))
    {
        int relativePriority;
        fr[1].getInt( relativePriority );
        ord._relativePriority = (short) relativePriority;
        fr += 2;
        iteratorAdvanced = true;
    }
    if (fr.matchSequence("transparency %i"))
    {
        int transparency;
        fr[1].getInt( transparency );
        ord._transparency = (unsigned short) transparency;
        fr += 2;
        iteratorAdvanced = true;
    }
    if (fr.matchSequence("effectID1 %i"))
    {
        int effectID1;
        fr[1].getInt( effectID1 );
        ord._effectID1 = (short) effectID1;
        fr += 2;
        iteratorAdvanced = true;
    }
    if (fr.matchSequence("effectID2 %i"))
    {
        int effectID2;
        fr[1].getInt( effectID2 );
        ord._effectID2 = (short) effectID2;
        fr += 2;
        iteratorAdvanced = true;
    }
    if (fr.matchSequence("significance %i"))
    {
        int significance;
        fr[1].getInt( significance );
        ord._significance = (short) significance;
        fr += 2;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool ObjectRecordData_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgSim::ObjectRecordData &ord = static_cast<const osgSim::ObjectRecordData&>(obj);

    fw.indent() << "flags " << ord._flags << std::endl;
    fw.indent() << "relativePriority " << ord._relativePriority << std::endl;
    fw.indent() << "transparency " << ord._transparency << std::endl;
    fw.indent() << "effectID1 " << ord._effectID1 << std::endl;
    fw.indent() << "effectID2 " << ord._effectID2 << std::endl;
    fw.indent() << "significance " << ord._significance << std::endl;

    return true;
}
