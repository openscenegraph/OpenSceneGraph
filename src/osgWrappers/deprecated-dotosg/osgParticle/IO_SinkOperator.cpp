
#include <osgParticle/SinkOperator>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <osg/Vec3>

#include <iostream>

bool  SinkOperator_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  SinkOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(SinkOperator_Proxy)
(
    new osgParticle::SinkOperator,
    "SinkOperator",
    "Object Operator DomainOperator SinkOperator",
    SinkOperator_readLocalData,
    SinkOperator_writeLocalData
);

bool SinkOperator_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::SinkOperator &sp = static_cast<osgParticle::SinkOperator &>(obj);
    bool itAdvanced = false;

    if (fr[0].matchWord("sinkTarget")) {
        const char *ptstr = fr[1].getStr();
        if (ptstr) {
            std::string str(ptstr);
            if (str == "position")
                sp.setSinkTarget(osgParticle::SinkOperator::SINK_POSITION);
            else if (str == "velocity")
                sp.setSinkTarget(osgParticle::SinkOperator::SINK_VELOCITY);
            else if (str == "angular_velocity")
                sp.setSinkTarget(osgParticle::SinkOperator::SINK_ANGULAR_VELOCITY);

            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("sinkStrategy")) {
        const char *ptstr = fr[1].getStr();
        if (ptstr) {
            std::string str(ptstr);
            if (str == "inside")
                sp.setSinkStrategy(osgParticle::SinkOperator::SINK_INSIDE);
            else if (str == "outside")
                sp.setSinkStrategy(osgParticle::SinkOperator::SINK_OUTSIDE);

            fr += 2;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool SinkOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::SinkOperator &sp = static_cast<const osgParticle::SinkOperator &>(obj);

    fw.indent() << "sinkTarget ";
    switch (sp.getSinkTarget())
    {
    case osgParticle::SinkOperator::SINK_POSITION:
        fw << "position" << std::endl; break;
    case osgParticle::SinkOperator::SINK_VELOCITY:
        fw << "velocity" << std::endl; break;
    case osgParticle::SinkOperator::SINK_ANGULAR_VELOCITY:
        fw << "angular_velocity" << std::endl; break;
    default:
        fw << "undefined" << std::endl; break;
    }

    fw.indent() << "sinkStrategy ";
    switch (sp.getSinkStrategy())
    {
    case osgParticle::SinkOperator::SINK_INSIDE:
        fw << "inside" << std::endl; break;
    case osgParticle::SinkOperator::SINK_OUTSIDE:
        fw << "outside" << std::endl; break;
    default:
        fw << "undefined" << std::endl; break;
    }

    return true;
}
