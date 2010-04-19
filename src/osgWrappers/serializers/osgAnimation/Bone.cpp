#include <osgAnimation/Bone>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgAnimation_Bone,
                         new osgAnimation::Bone,
                         osgAnimation::Bone,
                         "osg::Object osg::Node osg::Group osg::Transform osg::MatrixTransform osgAnimation::Bone" )
{
    ADD_MATRIX_SERIALIZER( InvBindMatrixInSkeletonSpace, osg::Matrix() );  // _invBindInSkeletonSpace
    ADD_MATRIX_SERIALIZER( MatrixInSkeletonSpace, osg::Matrix() );  // _boneInSkeletonSpace
}
