#include <osg/Notify>
#include <osg/Geometry>
#include <osg/AnimationPath>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osg;
using namespace osgDB;


// forward declare functions to use later.
bool  AnimationPath_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  AnimationPath_writeLocalData(const osg::Object &obj, osgDB::Output &fw);


// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(AnimationPath)
(
    new osg::AnimationPath,
    "AnimationPath",
    "Object AnimationPath",
    AnimationPath_readLocalData,
    AnimationPath_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);


bool AnimationPath_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osg::AnimationPath *ap = dynamic_cast<osg::AnimationPath*>(&obj);
    if (!ap) return false;
    
    
    bool itAdvanced = false;

    if (fr[0].matchWord("LoopMode"))
    {
        if (fr[1].matchWord("SWING"))
        {
            ap->setLoopMode(AnimationPath::SWING);
            fr += 2;
            itAdvanced = true;            
        }
        else if (fr[1].matchWord("LOOP"))
        {
            ap->setLoopMode(AnimationPath::LOOP);
            fr += 2;
            itAdvanced = true;                        
        } 
        else if (fr[1].matchWord("NO_LOOPING"))
        {
            ap->setLoopMode(AnimationPath::NO_LOOPING);
            fr += 2;
            itAdvanced = true;                        
        } 
    }



    if (fr.matchSequence("ControlPoints {"))
    {
        int entry = fr[0].getNoNestedBrackets();

        fr += 2;
        

        double time;
        Vec3d position,scale;
        Quat rotation;
        
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            if (fr[0].getFloat(time) &&
                fr[1].getFloat(position[0]) && 
                fr[2].getFloat(position[1]) && 
                fr[3].getFloat(position[2]) &&
                fr[4].getFloat(rotation[0]) && 
                fr[5].getFloat(rotation[1]) && 
                fr[6].getFloat(rotation[2]) &&
                fr[7].getFloat(rotation[3]) &&
                fr[8].getFloat(scale[0]) && 
                fr[9].getFloat(scale[1]) && 
                fr[10].getFloat(scale[2]))
            {


                osg::AnimationPath::ControlPoint ctrlPoint(position,rotation,scale);
                ap->insert(time, ctrlPoint);

                fr+=11; 
            } 
            else fr.advanceOverCurrentFieldOrBlock();

        }

        itAdvanced = true;

    }
    
    return itAdvanced;
}


bool AnimationPath_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osg::AnimationPath* ap = dynamic_cast<const osg::AnimationPath*>(&obj);
    if (!ap) return false;

    fw.indent() << "LoopMode ";
    switch(ap->getLoopMode())
    {
        case AnimationPath::SWING:
            fw << "SWING" <<std::endl;
            break;
        case AnimationPath::LOOP:
            fw << "LOOP"<<std::endl;
            break;
        case AnimationPath::NO_LOOPING:
            fw << "NO_LOOPING"<<std::endl;
            break;
    }

    const AnimationPath::TimeControlPointMap& tcpm = ap->getTimeControlPointMap();

    fw.indent() << "ControlPoints {"<< std::endl;
    fw.moveIn();

    int prec = fw.precision();
    fw.precision(15);

    for (AnimationPath::TimeControlPointMap::const_iterator itr=tcpm.begin();
         itr!=tcpm.end(); 
         ++itr)
    {
        fw.indent() << itr->first << " " << itr->second.getPosition() << " " << itr->second.getRotation() << " " <<itr->second.getScale() << std::endl;

    }

    fw.precision(prec);

    fw.moveOut();
    fw.indent() << "}"<< std::endl;

    return true;
}


// forward declare functions to use later.
bool  AnimationPathCallback_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  AnimationPathCallback_writeLocalData(const osg::Object &obj, osgDB::Output &fw);


// register the read and write functions with the osgDB::Registry.
osgDB::RegisterDotOsgWrapperProxy  AnimationPathCallback_Proxy
(
    new osg::AnimationPathCallback,
    "AnimationPathCallback",
    "Object AnimationPathCallback",
    AnimationPathCallback_readLocalData,
    AnimationPathCallback_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool AnimationPathCallback_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osg::AnimationPathCallback *apc = dynamic_cast<osg::AnimationPathCallback*>(&obj);
    if (!apc) return false;

    bool iteratorAdvanced = false;
    
    if (fr.matchSequence("pivotPoint %f %f %f"))
    {
        osg::Vec3 pivot;
        fr[1].getFloat(pivot[0]);
        fr[2].getFloat(pivot[1]);
        fr[3].getFloat(pivot[2]);
        
        apc->setPivotPoint(pivot);
        
        fr += 4;
        iteratorAdvanced = true;
    }
    
    if (fr.matchSequence("timeOffset %f"))
    {
        fr[1].getFloat(apc->_timeOffset);
        fr+=2;
        iteratorAdvanced = true;
    }
    
    else if(fr.matchSequence("timeMultiplier %f"))
    {
        fr[1].getFloat(apc->_timeMultiplier);
        fr+=2;
        iteratorAdvanced = true;
    }

    static osg::ref_ptr<osg::AnimationPath> s_path = new osg::AnimationPath;
    ref_ptr<osg::Object> object = fr.readObjectOfType(*s_path);
    if (object.valid())
    {
        osg::AnimationPath* animpath = dynamic_cast<osg::AnimationPath*>(object.get());
        if (animpath) apc->setAnimationPath(animpath);
        iteratorAdvanced = true;
    }
    
    return iteratorAdvanced;
}


bool AnimationPathCallback_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{


    const osg::AnimationPathCallback* apc = dynamic_cast<const osg::AnimationPathCallback*>(&obj);
    if (!apc) return false;


    fw.indent() <<"pivotPoint " <<apc->getPivotPoint()<<std::endl;
    fw.indent() <<"timeOffset " <<apc->_timeOffset<<std::endl;
    fw.indent() <<"timeMultiplier " <<apc->_timeMultiplier << std::endl;

    if (apc->getAnimationPath())
    {
        fw.writeObject(*(apc->getAnimationPath()));
    }

    return true;
}
