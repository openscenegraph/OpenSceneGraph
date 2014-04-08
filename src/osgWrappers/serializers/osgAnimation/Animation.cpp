#include <osgAnimation/Animation>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

// reading channel helpers

static void readChannel( osgDB::InputStream& is, osgAnimation::Channel* ch )
{
    std::string name, targetName;
    is >> is.PROPERTY("Name"); is.readWrappedString(name);
    is >> is.PROPERTY("TargetName"); is.readWrappedString(targetName);
    ch->setName( name );
    ch->setTargetName( targetName );
}
#include<osg/io_utils>
template <typename ContainerType, typename ValueType>
static void readContainer( osgDB::InputStream& is, ContainerType* container )
{
    typedef typename ContainerType::KeyType KeyType;
    bool hasContainer = false;
    is >> is.PROPERTY("KeyFrameContainer") >> hasContainer;
    if ( hasContainer )
    {
        unsigned int size = 0;
        size = is.readSize(); is >> is.BEGIN_BRACKET;
        for ( unsigned int i=0; i<size; ++i )
        {
            double time = 0.0f;
            ValueType value;
            is >> time >> value;
            container->push_back( KeyType(time, value) );
        }
        is >> is.END_BRACKET;
    }
}

template <typename ContainerType, typename ValueType, typename InternalValueType>
static void readContainer2( osgDB::InputStream& is, ContainerType* container )
{
    typedef typename ContainerType::KeyType KeyType;
    bool hasContainer = false;
    is >> is.PROPERTY("KeyFrameContainer") >> hasContainer;
    if ( hasContainer )
    {
        unsigned int size = 0;
        size = is.readSize(); is >> is.BEGIN_BRACKET;
        for ( unsigned int i=0; i<size; ++i )
        {
            double time = 0.0f;
            InternalValueType pos, ptIn, ptOut;
            is >> time >> pos >> ptIn >> ptOut;
            container->push_back( KeyType(time, ValueType(pos, ptIn, ptOut)) );
        }
        is >> is.END_BRACKET;
    }
}

#define READ_CHANNEL_FUNC( NAME, CHANNEL, CONTAINER, VALUE ) \
    if ( type==#NAME ) { \
        CHANNEL* ch = new CHANNEL; \
        readChannel( is, ch ); \
        readContainer<CONTAINER, VALUE>( is, ch->getOrCreateSampler()->getOrCreateKeyframeContainer() ); \
        is >> is.END_BRACKET; \
        if ( ch ) ani.addChannel( ch ); \
        continue; \
    }

#define READ_CHANNEL_FUNC2( NAME, CHANNEL, CONTAINER, VALUE, INVALUE ) \
    if ( type==#NAME ) { \
        CHANNEL* ch = new CHANNEL; \
        readChannel( is, ch ); \
        readContainer2<CONTAINER, VALUE, INVALUE>( is, ch->getOrCreateSampler()->getOrCreateKeyframeContainer() ); \
        is >> is.END_BRACKET; \
        if ( ch ) ani.addChannel( ch ); \
        continue; \
    }

// writing channel helpers

static void writeChannel( osgDB::OutputStream& os, osgAnimation::Channel* ch )
{
    os << os.PROPERTY("Name"); os.writeWrappedString(ch->getName()); os << std::endl;
    os << os.PROPERTY("TargetName");os.writeWrappedString(ch->getTargetName()); os << std::endl;
}

template <typename ContainerType>
static void writeContainer( osgDB::OutputStream& os, ContainerType* container )
{
    os << os.PROPERTY("KeyFrameContainer") << (container!=NULL);
    if ( container!=NULL )
    {
        os.writeSize(container->size()); os << os.BEGIN_BRACKET << std::endl;
        for ( unsigned int i=0; i<container->size(); ++i )
        {
            os << (*container)[i].getTime() << (*container)[i].getValue() << std::endl;
        }
        os << os.END_BRACKET;
    }
    os << std::endl;
}

template <typename ContainerType>
static void writeContainer2( osgDB::OutputStream& os, ContainerType* container )
{
    typedef typename ContainerType::KeyType KeyType;
    os << os.PROPERTY("KeyFrameContainer") << (container!=NULL);
    if ( container!=NULL )
    {
        os.writeSize(container->size()); os << os.BEGIN_BRACKET << std::endl;
        for ( unsigned int i=0; i<container->size(); ++i )
        {
            const KeyType& keyframe = (*container)[i];
            os << keyframe.getTime() << keyframe.getValue().getPosition()
               << keyframe.getValue().getControlPointIn()
               << keyframe.getValue().getControlPointOut() << std::endl;
        }
        os << os.END_BRACKET;
    }
    os << std::endl;
}

#define WRITE_CHANNEL_FUNC( NAME, CHANNEL, CONTAINER ) \
    CHANNEL* ch_##NAME = dynamic_cast<CHANNEL*>(ch); \
    if ( ch_##NAME ) { \
        os << os.PROPERTY("Type") << std::string(#NAME) << os.BEGIN_BRACKET << std::endl; \
        writeChannel( os, ch_##NAME ); \
        writeContainer<CONTAINER>( os, ch_##NAME ->getSamplerTyped()->getKeyframeContainerTyped() ); \
        os << os.END_BRACKET << std::endl; \
        continue; \
    }

#define WRITE_CHANNEL_FUNC2( NAME, CHANNEL, CONTAINER ) \
    CHANNEL* ch_##NAME = dynamic_cast<CHANNEL*>(ch); \
    if ( ch_##NAME ) { \
        os << os.PROPERTY("Type") << #NAME << os.BEGIN_BRACKET << std::endl; \
        writeChannel( os, ch_##NAME ); \
        writeContainer2<CONTAINER>( os, ch_##NAME ->getSamplerTyped()->getKeyframeContainerTyped() ); \
        os << os.END_BRACKET << std::endl; \
        continue; \
    }

// _channels

static bool checkChannels( const osgAnimation::Animation& ani )
{
    return ani.getChannels().size()>0;
}

static bool readChannels( osgDB::InputStream& is, osgAnimation::Animation& ani )
{
    unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        std::string type;
        is >> is.PROPERTY("Type") >> type >> is.BEGIN_BRACKET;

        READ_CHANNEL_FUNC( DoubleStepChannel, osgAnimation::DoubleStepChannel, osgAnimation::DoubleKeyframeContainer, double );
        READ_CHANNEL_FUNC( FloatStepChannel, osgAnimation::FloatStepChannel, osgAnimation::FloatKeyframeContainer, float );
        READ_CHANNEL_FUNC( Vec2StepChannel, osgAnimation::Vec2StepChannel, osgAnimation::Vec2KeyframeContainer, osg::Vec2 );
        READ_CHANNEL_FUNC( Vec3StepChannel, osgAnimation::Vec3StepChannel, osgAnimation::Vec3KeyframeContainer, osg::Vec3 );
        READ_CHANNEL_FUNC( Vec4StepChannel, osgAnimation::Vec4StepChannel, osgAnimation::Vec4KeyframeContainer, osg::Vec4 );
        READ_CHANNEL_FUNC( QuatStepChannel, osgAnimation::QuatStepChannel, osgAnimation::QuatKeyframeContainer, osg::Quat );
        READ_CHANNEL_FUNC( DoubleLinearChannel, osgAnimation::DoubleLinearChannel, osgAnimation::DoubleKeyframeContainer, double );
        READ_CHANNEL_FUNC( FloatLinearChannel, osgAnimation::FloatLinearChannel, osgAnimation::FloatKeyframeContainer, float );
        READ_CHANNEL_FUNC( Vec2LinearChannel, osgAnimation::Vec2LinearChannel, osgAnimation::Vec2KeyframeContainer, osg::Vec2 );
        READ_CHANNEL_FUNC( Vec3LinearChannel, osgAnimation::Vec3LinearChannel, osgAnimation::Vec3KeyframeContainer, osg::Vec3 );
        READ_CHANNEL_FUNC( Vec4LinearChannel, osgAnimation::Vec4LinearChannel, osgAnimation::Vec4KeyframeContainer, osg::Vec4 );
        READ_CHANNEL_FUNC( QuatSphericalLinearChannel, osgAnimation::QuatSphericalLinearChannel,
                                                       osgAnimation::QuatKeyframeContainer, osg::Quat );
        READ_CHANNEL_FUNC( MatrixLinearChannel, osgAnimation::MatrixLinearChannel,
                                                osgAnimation::MatrixKeyframeContainer, osg::Matrix );
        READ_CHANNEL_FUNC2( FloatCubicBezierChannel, osgAnimation::FloatCubicBezierChannel,
                                                     osgAnimation::FloatCubicBezierKeyframeContainer,
                                                     osgAnimation::FloatCubicBezier, float );
        READ_CHANNEL_FUNC2( DoubleCubicBezierChannel, osgAnimation::DoubleCubicBezierChannel,
                                                      osgAnimation::DoubleCubicBezierKeyframeContainer,
                                                      osgAnimation::DoubleCubicBezier, double );
        READ_CHANNEL_FUNC2( Vec2CubicBezierChannel, osgAnimation::Vec2CubicBezierChannel,
                                                    osgAnimation::Vec2CubicBezierKeyframeContainer,
                                                    osgAnimation::Vec2CubicBezier, osg::Vec2 );
        READ_CHANNEL_FUNC2( Vec3CubicBezierChannel, osgAnimation::Vec3CubicBezierChannel,
                                                    osgAnimation::Vec3CubicBezierKeyframeContainer,
                                                    osgAnimation::Vec3CubicBezier, osg::Vec3 );
        READ_CHANNEL_FUNC2( Vec4CubicBezierChannel, osgAnimation::Vec4CubicBezierChannel,
                                                    osgAnimation::Vec4CubicBezierKeyframeContainer,
                                                    osgAnimation::Vec4CubicBezier, osg::Vec4 );
        is.advanceToCurrentEndBracket();
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeChannels( osgDB::OutputStream& os, const osgAnimation::Animation& ani )
{
    const osgAnimation::ChannelList& channels = ani.getChannels();
    os.writeSize(channels.size()); os << os.BEGIN_BRACKET << std::endl;
    for ( osgAnimation::ChannelList::const_iterator itr=channels.begin();
          itr!=channels.end(); ++itr )
    {
        osgAnimation::Channel* ch = itr->get();
        WRITE_CHANNEL_FUNC( DoubleStepChannel, osgAnimation::DoubleStepChannel, osgAnimation::DoubleKeyframeContainer );
        WRITE_CHANNEL_FUNC( FloatStepChannel, osgAnimation::FloatStepChannel, osgAnimation::FloatKeyframeContainer );
        WRITE_CHANNEL_FUNC( Vec2StepChannel, osgAnimation::Vec2StepChannel, osgAnimation::Vec2KeyframeContainer );
        WRITE_CHANNEL_FUNC( Vec3StepChannel, osgAnimation::Vec3StepChannel, osgAnimation::Vec3KeyframeContainer );
        WRITE_CHANNEL_FUNC( Vec4StepChannel, osgAnimation::Vec4StepChannel, osgAnimation::Vec4KeyframeContainer );
        WRITE_CHANNEL_FUNC( QuatStepChannel, osgAnimation::QuatStepChannel, osgAnimation::QuatKeyframeContainer );
        WRITE_CHANNEL_FUNC( DoubleLinearChannel, osgAnimation::DoubleLinearChannel, osgAnimation::DoubleKeyframeContainer );
        WRITE_CHANNEL_FUNC( FloatLinearChannel, osgAnimation::FloatLinearChannel, osgAnimation::FloatKeyframeContainer );
        WRITE_CHANNEL_FUNC( Vec2LinearChannel, osgAnimation::Vec2LinearChannel, osgAnimation::Vec2KeyframeContainer );
        WRITE_CHANNEL_FUNC( Vec3LinearChannel, osgAnimation::Vec3LinearChannel, osgAnimation::Vec3KeyframeContainer );
        WRITE_CHANNEL_FUNC( Vec4LinearChannel, osgAnimation::Vec4LinearChannel, osgAnimation::Vec4KeyframeContainer );
        WRITE_CHANNEL_FUNC( QuatSphericalLinearChannel, osgAnimation::QuatSphericalLinearChannel,
                                                         osgAnimation::QuatKeyframeContainer );
        WRITE_CHANNEL_FUNC( MatrixLinearChannel, osgAnimation::MatrixLinearChannel,
                                                 osgAnimation::MatrixKeyframeContainer );
        WRITE_CHANNEL_FUNC2( FloatCubicBezierChannel, osgAnimation::FloatCubicBezierChannel,
                                                      osgAnimation::FloatCubicBezierKeyframeContainer );
        WRITE_CHANNEL_FUNC2( DoubleCubicBezierChannel, osgAnimation::DoubleCubicBezierChannel,
                                                       osgAnimation::DoubleCubicBezierKeyframeContainer );
        WRITE_CHANNEL_FUNC2( Vec2CubicBezierChannel, osgAnimation::Vec2CubicBezierChannel,
                                                     osgAnimation::Vec2CubicBezierKeyframeContainer );
        WRITE_CHANNEL_FUNC2( Vec3CubicBezierChannel, osgAnimation::Vec3CubicBezierChannel,
                                                     osgAnimation::Vec3CubicBezierKeyframeContainer );
        WRITE_CHANNEL_FUNC2( Vec4CubicBezierChannel, osgAnimation::Vec4CubicBezierChannel,
                                                     osgAnimation::Vec4CubicBezierKeyframeContainer );

        os << os.PROPERTY("Type") << std::string("UnknownChannel") << os.BEGIN_BRACKET << std::endl;
        os << os.END_BRACKET << std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgAnimation_Animation,
                         new osgAnimation::Animation,
                         osgAnimation::Animation,
                         "osg::Object osgAnimation::Animation" )
{
    ADD_DOUBLE_SERIALIZER( Duration, 0.0f );  // _duration
    ADD_FLOAT_SERIALIZER( Weight, 0.0f );  // _weight
    ADD_DOUBLE_SERIALIZER( StartTime, 0.0f );  // _startTime

    BEGIN_ENUM_SERIALIZER( PlayMode, LOOP );
        ADD_ENUM_VALUE( ONCE );
        ADD_ENUM_VALUE( STAY );
        ADD_ENUM_VALUE( LOOP );
        ADD_ENUM_VALUE( PPONG );
    END_ENUM_SERIALIZER();  // _playmode

    ADD_USER_SERIALIZER( Channels );  // _channels
}
