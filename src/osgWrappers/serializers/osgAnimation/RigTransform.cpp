
#include <osgAnimation/RigTransformHardware>
#include <osgAnimation/RigTransformSoftware>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

namespace wrap_osgAnimationRigTransform{
  REGISTER_OBJECT_WRAPPER( osgAnimation_RigTransform,
                             NULL,
                             osgAnimation::RigTransform,
                             "osg::Object osgAnimation::RigTransform" ){}
}
namespace wrap_osgAnimationRigTransformSoftWare{
  REGISTER_OBJECT_WRAPPER( osgAnimation_RigTransformSoftware,
                             new osgAnimation::RigTransformSoftware,
                             osgAnimation::RigTransformSoftware,
                             "osg::Object osgAnimation::RigTransform  osgAnimation::RigTransformSoftware" ){}
}
namespace wrap_osgAnimationRigTransformHardWare{
  REGISTER_OBJECT_WRAPPER( osgAnimation_RigTransformHardware,
                             new osgAnimation::RigTransformHardware,
                             osgAnimation::RigTransformHardware,
                             "osg::Object osgAnimation::RigTransform osgAnimation::RigTransformHardware" ){}
}
