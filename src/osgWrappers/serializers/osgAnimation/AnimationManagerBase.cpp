#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgAnimation/AnimationManagerBase>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkAnimations( const osgAnimation::AnimationManagerBase& manager )
{
    return manager.getAnimationList().size()>0;
}

static bool readAnimations( osgDB::InputStream& is, osgAnimation::AnimationManagerBase& manager )
{
    unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osgAnimation::Animation* ani = dynamic_cast<osgAnimation::Animation*>( is.readObject() );
        if ( ani ) manager.registerAnimation( ani );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeAnimations( osgDB::OutputStream& os, const osgAnimation::AnimationManagerBase& manager )
{
    const osgAnimation::AnimationList& animations = manager.getAnimationList();
    os.writeSize(animations.size()); os << os.BEGIN_BRACKET << std::endl;
    for ( osgAnimation::AnimationList::const_iterator itr=animations.begin();
          itr!=animations.end(); ++itr )
    {
        os << itr->get();
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgAnimation_AnimationManagerBase,
                         /*new osgAnimation::AnimationManagerBase*/NULL,
                         osgAnimation::AnimationManagerBase,
                         "osg::Object osg::NodeCallback osgAnimation::AnimationManagerBase" )
{
    ADD_USER_SERIALIZER( Animations );  // _animations
    ADD_BOOL_SERIALIZER( AutomaticLink, true );  // _automaticLink
}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
