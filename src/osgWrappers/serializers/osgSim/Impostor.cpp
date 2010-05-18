#include <osgSim/Impostor>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgSim_Impostor,
                         new osgSim::Impostor,
                         osgSim::Impostor,
                         "osg::Object osg::Node osg::Group osg::LOD osgSim::Impostor" )
{
    ADD_FLOAT_SERIALIZER( ImpostorThreshold, -1.0f );  // _impostorThreshold
}
