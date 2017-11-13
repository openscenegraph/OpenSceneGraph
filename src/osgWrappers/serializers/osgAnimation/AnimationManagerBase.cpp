#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgAnimation/AnimationManagerBase>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
namespace  osgAnimation_AnimationManagerBaseWrapper
{
static bool checkAnimations( const osgAnimation::AnimationManagerBase& manager )
{
    return manager.getAnimationList().size()>0;
}

static bool readAnimations( osgDB::InputStream& is, osgAnimation::AnimationManagerBase& manager )
{
    unsigned int size = is.readSize();
    is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osg::ref_ptr<osgAnimation::Animation> ani = is.readObjectOfType<osgAnimation::Animation>();
        if ( ani ) manager.registerAnimation( ani.get() );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeAnimations( osgDB::OutputStream& os, const osgAnimation::AnimationManagerBase& manager )
{
    const osgAnimation::AnimationList& animations = manager.getAnimationList();
    os.writeSize(animations.size());
    os << os.BEGIN_BRACKET << std::endl;
    for ( osgAnimation::AnimationList::const_iterator itr=animations.begin();
            itr!=animations.end(); ++itr )
    {
        os << itr->get();
    }
    os << os.END_BRACKET << std::endl;
    return true;
}
struct osgAnimation_AnimationManagerBasegetnumAnimations : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        osgAnimation::AnimationManagerBase* group =  dynamic_cast<osgAnimation::AnimationManagerBase*>(reinterpret_cast<osg::Object*>(objectPtr));
        if (group) outputParameters.push_back(new osg::UIntValueObject("return",group->getNumRegisteredAnimations()));
        return true;
    }
};
struct osgAnimation_AnimationManagerBasegetAnimation : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        osg::Object* indexObject = inputParameters[0].get();

        unsigned int index = 0;
        osg::DoubleValueObject* dvo = dynamic_cast<osg::DoubleValueObject*>(indexObject);
        if (dvo) index = static_cast<unsigned int>(dvo->getValue());
        else
        {
            osg::UIntValueObject* uivo = dynamic_cast<osg::UIntValueObject*>(indexObject);
            if (uivo) index = uivo->getValue();
        }
        osgAnimation::AnimationManagerBase* group = dynamic_cast<osgAnimation::AnimationManagerBase*>(reinterpret_cast<osg::Object*>(objectPtr));
        if (group) outputParameters.push_back(group->getRegisteredAnimation(index));

        return true;
    }
};
REGISTER_OBJECT_WRAPPER( osgAnimation_AnimationManagerBase,
                         /*new osgAnimation::AnimationManagerBase*/NULL,
                         osgAnimation::AnimationManagerBase,
                         "osg::Object osg::NodeCallback osgAnimation::AnimationManagerBase" )
{
    ADD_USER_SERIALIZER( Animations );  // _animations
    ADD_BOOL_SERIALIZER( AutomaticLink, true );  // _automaticLink
    
    {
        UPDATE_TO_VERSION_SCOPED( 152 )

        ADD_METHOD_OBJECT( "getRegisteredAnimation", osgAnimation_AnimationManagerBasegetAnimation );
        ADD_METHOD_OBJECT( "getNumRegisteredAnimations", osgAnimation_AnimationManagerBasegetnumAnimations );
    }
}
}
#undef OBJECT_CAST
#define OBJECT_CAST static_cast
