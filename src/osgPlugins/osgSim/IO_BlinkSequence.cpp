#include <osgSim/BlinkSequence>

#include <iostream>
#include <string>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

using namespace osgSim;

bool BlinkSequence_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool BlinkSequence_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy BlinkSequence_Proxy
(
    new BlinkSequence,
    "BlinkSequence",
    "Object BlinkSequence",
    &BlinkSequence_readLocalData,
    &BlinkSequence_writeLocalData,
    osgDB::DotOsgWrapper::READ_AND_WRITE
);

bool BlinkSequence_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    bool iteratorAdvanced = false;
    BlinkSequence &seq = static_cast<BlinkSequence &>(obj);

    if (fr.matchSequence("phaseShift %f"))
    {
        double ps;
        fr[1].getFloat(ps);
        fr += 2;
        seq.setPhaseShift(ps);
        iteratorAdvanced = true;
    }
    if (fr.matchSequence("pulse %f %f %f %f %f"))
    {
        double length;
        float r, g, b, a;
        fr[1].getFloat(length);
        fr[2].getFloat(r);
        fr[3].getFloat(g);
        fr[4].getFloat(b);
        fr[5].getFloat(a);
        fr += 6;

        seq.addPulse(length, osg::Vec4(r, g, b, a));
        iteratorAdvanced = true;
    }

    BlinkSequence::SequenceGroup * sg = static_cast<BlinkSequence::SequenceGroup *>
        (fr.readObjectOfType(osgDB::type_wrapper<BlinkSequence::SequenceGroup>()));

    if (sg) {
        seq.setSequenceGroup(sg);
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool BlinkSequence_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const BlinkSequence &seq = static_cast<const BlinkSequence &>(obj);

    fw.indent()<<"phaseShift "<< seq.getPhaseShift() << std::endl;

    if (seq.getSequenceGroup() != NULL) {
        fw.writeObject(*seq.getSequenceGroup());
    }
    for (int i=0; i<seq.getNumPulses(); i++) {
        double length;
        osg::Vec4 color;
        seq.getPulse(i, length, color);
        fw.indent()<<"pulse " << length << " " << color << std::endl;
    }

    return true;
}

/******************************************************/

bool BlinkSequence_SequenceGroup_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool BlinkSequence_SequenceGroup_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy BlinkSequence_SequenceGroup_Proxy
(
    new BlinkSequence::SequenceGroup,
    "BlinkSequence::SequenceGroup",
    "Object BlinkSequence::SequenceGroup",
    &BlinkSequence_SequenceGroup_readLocalData,
    &BlinkSequence_SequenceGroup_writeLocalData,
    osgDB::DotOsgWrapper::READ_AND_WRITE
);

bool BlinkSequence_SequenceGroup_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    bool iteratorAdvanced = false;
    BlinkSequence::SequenceGroup &sg = static_cast<BlinkSequence::SequenceGroup &>(obj);

    if (fr.matchSequence("baseTime %f"))
    {
        fr[1].getFloat(sg._baseTime);
        fr += 2;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool BlinkSequence_SequenceGroup_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const BlinkSequence::SequenceGroup &sg = static_cast<const BlinkSequence::SequenceGroup &>(obj);

    fw.indent()<<"baseTime "<< sg._baseTime << std::endl;
    return true;
}
