#include <osg/Notify>
#include <osg/Geometry>
#include <osg/AnimationPath>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osg;
using namespace osgDB;


// forward declare functions to use later.
bool  AnimationPath_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  AnimationPath_writeLocalData(const osg::Object &obj, osgDB::Output &fw);


// register the read and write functions with the osgDB::Registry.
osgDB::RegisterDotOsgWrapperProxy  AnimationPath_Proxy
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
        int entry = fr[1].getNoNestedBrackets();

        fr += 2;
        
        float fTime = -1;

        osg::AnimationPath::ControlPoint CtrlPoint;
        bool bPosParsed = false;
        bool bRotParsed = false;
        bool bScaleParsed = false;

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            bool handled=false;

            if (fr[0].matchWord("Time") &&
                fr[1].isFloat())
            {
                if (bPosParsed || bRotParsed || bScaleParsed)
                {
                    ap->insert(fTime, CtrlPoint);
                    
                    bPosParsed = false;
                    bRotParsed = false;
                    bScaleParsed = false;
                }

                fr[1].getFloat(fTime);
                fr+=2; 
                handled = true; 
            }

            Vec3 vec3; 
            if (fr[0].matchWord("Position") &&
                fr[1].getFloat(vec3[0]) && 
                fr[2].getFloat(vec3[1]) && 
                fr[3].getFloat(vec3[2])) 
            { 
                CtrlPoint._position = vec3; 
                fr+=4; 
                bPosParsed = true;
                handled = true; 
            } 


            Vec4 vec4; 
            if (fr[0].matchWord("Rotation") &&
                fr[1].getFloat(vec4[0]) && 
                fr[2].getFloat(vec4[1]) && 
                fr[3].getFloat(vec4[2]) && 
                fr[4].getFloat(vec4[3])) 
            { 
                CtrlPoint._rotation.makeRotate(vec4[0], vec4[1], vec4[2], vec4[3]);
                fr+=5; 
                bRotParsed = true;
                handled = true; 
            } 


            if (fr[0].matchWord("Scale") &&
                fr[1].getFloat(vec3[0]) && 
                fr[2].getFloat(vec3[1]) && 
                fr[3].getFloat(vec3[2])) 
            { 
                CtrlPoint._scale = vec3; 
                fr+=4; 
                bScaleParsed = true;
                handled = true; 
            } 

            if (!handled) fr.advanceOverCurrentFieldOrBlock();

        }

            // If all values have been parsed in
        if (bPosParsed && bRotParsed && bScaleParsed)
        {
            ap->insert(fTime, CtrlPoint);
        }

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

    bool bPosSet = false;
    bool bRotSet = false;
    bool bScaleSet = false;
    AnimationPath::TimeControlPointMap::const_iterator itr;
    for (itr=tcpm.begin();
         itr!=tcpm.end(); 
         ++itr)
    {
        if (itr->second._position != Vec3(0,0,0))
            bPosSet = true;
        if (itr->second._rotation.asVec4() != Vec4(0,0,0,1))
            bRotSet = true;
        if (itr->second._scale    != Vec3(1,1,1))
            bScaleSet = true;
    }

    for (itr=tcpm.begin();
         itr!=tcpm.end(); 
         ++itr)
    {
        Vec4 Rot;

        fw.indent() << "Time " << itr->first;

        if (bPosSet)
            fw << "  Position " << itr->second._position;

        if (bRotSet)
        {
            itr->second._rotation.getRotate (Rot[0], Rot[1], Rot[2], Rot[3]);
            fw << "  Rotation " << Rot;
        }

        if (bScaleSet)
            fw << "  Scale " << itr->second._scale;

        fw << std::endl;

    }

    fw.moveOut();
    fw.indent() << "}"<< std::endl;

    return true;
}
