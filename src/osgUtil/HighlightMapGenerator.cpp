#include <osgUtil/HighlightMapGenerator>

using namespace osgUtil;

HighlightMapGenerator::HighlightMapGenerator(const osg::Vec3 &light_direction,
											 const osg::Vec4 &light_color,
											 float specular_exponent,
											 int texture_size)
:	CubeMapGenerator(texture_size),
	ldir_(light_direction),
	lcol_(light_color),
	sexp_(specular_exponent)
{
	ldir_.normalize();
}

HighlightMapGenerator::HighlightMapGenerator(const HighlightMapGenerator &copy, const osg::CopyOp &copyop)
:	CubeMapGenerator(copy, copyop),
	ldir_(copy.ldir_),
	lcol_(copy.lcol_),
	sexp_(copy.sexp_)
{
}
