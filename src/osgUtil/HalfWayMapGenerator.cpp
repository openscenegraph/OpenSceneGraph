#include <osgUtil/HalfWayMapGenerator>

using namespace osgUtil;

HalfWayMapGenerator::HalfWayMapGenerator(const osg::Vec3 &light_direction, int texture_size)
:	CubeMapGenerator(texture_size),
	ldir_(light_direction)
{
	ldir_.normalize();
}

HalfWayMapGenerator::HalfWayMapGenerator(const HalfWayMapGenerator &copy, const osg::CopyOp &copyop)
:	CubeMapGenerator(copy, copyop),
	ldir_(copy.ldir_)
{
}
