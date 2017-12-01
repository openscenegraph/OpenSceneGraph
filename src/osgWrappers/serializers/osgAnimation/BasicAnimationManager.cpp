#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgAnimation/BasicAnimationManager>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
namespace osgAnimation_BasicAnimationManagerWrapper{
struct BasicAnimationManagerIsplaying : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        osgAnimation::Animation* child = dynamic_cast<osgAnimation::Animation*>(inputParameters[0].get());
        if (!child) return false;
        osgAnimation::BasicAnimationManager* group = dynamic_cast<osgAnimation::BasicAnimationManager*>(reinterpret_cast<osg::Object*>(objectPtr));
        if (group) outputParameters.push_back(new osg::BoolValueObject("return", group->isPlaying(child)));
        return true;
    }
};
struct BasicAnimationManagerfindAnimation : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        osgAnimation::Animation* child = dynamic_cast<osgAnimation::Animation*>(inputParameters[0].get());
        if (!child) return false;
        osgAnimation::BasicAnimationManager* group = dynamic_cast<osgAnimation::BasicAnimationManager*>(reinterpret_cast<osg::Object*>(objectPtr));
        if (group) outputParameters.push_back(new osg::BoolValueObject("return",group->findAnimation(child)));
        return true;
    }
};
struct BasicAnimationManagerPlayanimation : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        osgAnimation::Animation* child = dynamic_cast<osgAnimation::Animation*>(inputParameters[0].get());
        if (!child) return false;
        osgAnimation::BasicAnimationManager* group = dynamic_cast<osgAnimation::BasicAnimationManager*>(reinterpret_cast<osg::Object*>(objectPtr));
        if (group) group->playAnimation(child);
        return true;
    }
};
struct BasicAnimationManagerStopanimation : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        osgAnimation::Animation* child = dynamic_cast<osgAnimation::Animation*>(inputParameters[0].get());
        if (!child) return false;
        osgAnimation::BasicAnimationManager* group = dynamic_cast<osgAnimation::BasicAnimationManager*>(reinterpret_cast<osg::Object*>(objectPtr));
        if (group) group->stopAnimation(child);
        return true;
    }
};
REGISTER_OBJECT_WRAPPER( osgAnimation_BasicAnimationManager,
                         new osgAnimation::BasicAnimationManager,
                         osgAnimation::BasicAnimationManager,
                         "osg::Object osg::NodeCallback osgAnimation::AnimationManagerBase osgAnimation::BasicAnimationManager" )
{

    ADD_METHOD_OBJECT( "isPlaying", BasicAnimationManagerIsplaying );
    ADD_METHOD_OBJECT( "findAnimation", BasicAnimationManagerfindAnimation );
    ADD_METHOD_OBJECT( "playAnimation", BasicAnimationManagerPlayanimation );
    ADD_METHOD_OBJECT( "stopAnimation", BasicAnimationManagerStopanimation );

}
}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
