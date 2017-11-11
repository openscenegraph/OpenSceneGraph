
#include <osgAnimation/RigTransformHardware>
#include <osgAnimation/RigTransformSoftware>
#include <osgAnimation/MorphTransformSoftware>
    #include <osgAnimation/MorphTransformHardware>
    #include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

namespace wrap_osgAnimationRigTransform
{
    REGISTER_OBJECT_WRAPPER( osgAnimation_RigTransform,
                             NULL,
                             osgAnimation::RigTransform,
                             "osg::Object osgAnimation::RigTransform" ){}
}

namespace wrap_osgAnimationRigTransformSoftWare
{
    REGISTER_OBJECT_WRAPPER( osgAnimation_RigTransformSoftware,
                             new osgAnimation::RigTransformSoftware,
                             osgAnimation::RigTransformSoftware,
                             "osg::Object osgAnimation::RigTransform  osgAnimation::RigTransformSoftware" ){}
}

namespace wrap_osgAnimationRigTransformHardWare
{
    REGISTER_OBJECT_WRAPPER( osgAnimation_RigTransformHardware,
                             new osgAnimation::RigTransformHardware,
                             osgAnimation::RigTransformHardware,
                             "osg::Object osgAnimation::RigTransform osgAnimation::RigTransformHardware" )
    {
        {
            UPDATE_TO_VERSION_SCOPED(152)
            ADD_OBJECT_SERIALIZER(Shader, osg::Shader, NULL);
            ADD_UINT_SERIALIZER(FirstVertexAttributeTarget, RIGTRANSHW_DEFAULT_FIRST_VERTATTRIB_TARGETTED);
         }
     }
}

namespace wrap_osgAnimationMorphTransform
{
    REGISTER_OBJECT_WRAPPER( osgAnimation_MorphTransform,
                             NULL,
                             osgAnimation::MorphTransform,
                             "osg::Object osgAnimation::MorphTransform" ){}
}

namespace wrap_osgAnimationMorphTransformSoftWare
{
      REGISTER_OBJECT_WRAPPER( osgAnimation_MorphTransformSoftware,
                             new osgAnimation::MorphTransformSoftware,
                             osgAnimation::MorphTransformSoftware,
                             "osg::Object osgAnimation::MorphTransform  osgAnimation::MorphTransformSoftware" ){}
}

namespace wrap_osgAnimationMorphTransformHardware
{
    REGISTER_OBJECT_WRAPPER( osgAnimation_MorphTransformHardware,
                             new osgAnimation::MorphTransformHardware,
                             osgAnimation::MorphTransformHardware,
                             "osg::Object osgAnimation::MorphTransform  osgAnimation::MorphTransformHardware") 
    {
        {
            UPDATE_TO_VERSION_SCOPED(152)
            ADD_OBJECT_SERIALIZER(Shader, osg::Shader, NULL);
            ADD_UINT_SERIALIZER(ReservedTextureUnit, MORPHTRANSHW_DEFAULTMORPHTEXTUREUNIT);
        }
    }
}
