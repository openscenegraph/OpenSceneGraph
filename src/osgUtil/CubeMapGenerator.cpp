#include <osgUtil/CubeMapGenerator>

#include <memory>
#include <cstdlib>

using namespace osgUtil;

CubeMapGenerator::CubeMapGenerator(int texture_size)
:	osg::Referenced(),
	texture_size_(texture_size)
{
	for (int i=0; i<6; ++i) {				
		osg::ref_ptr<osg::Image> image = osgNew osg::Image;
		std::auto_ptr<unsigned char> data(static_cast<unsigned char *>(std::malloc(texture_size*texture_size*4)));
		image->setImage(texture_size, texture_size, 1, 4, GL_RGBA, GL_UNSIGNED_BYTE, data.get());
		data.release();
		images_.push_back(image);
	}
}

CubeMapGenerator::CubeMapGenerator(const CubeMapGenerator &copy, const osg::CopyOp &copyop)
:	osg::Referenced(copy),
	texture_size_(copy.texture_size_)
{
	Image_list::const_iterator i;
	for (i=copy.images_.begin(); i!=copy.images_.end(); ++i) {
		images_.push_back(static_cast<osg::Image *>(copyop(i->get())));
	}
}

void CubeMapGenerator::generateMap(bool use_osg_system)
{
	const float duv = 2.0f/(texture_size_-1);
	
	float v = 1;
	for (int i=0; i<texture_size_; ++i) {
		float u = 1;
		for (int j=0; j<texture_size_; ++j) {			
			if (use_osg_system) {
				set_pixel(0, j, i, compute_color(osg::Vec3(1, -u, v)));
				set_pixel(1, j, i, compute_color(osg::Vec3(-1, u, v)));
				set_pixel(2, j, i, compute_color(osg::Vec3(-u, v, 1)));
				set_pixel(3, j, i, compute_color(osg::Vec3(-u, -v, -1)));
				set_pixel(4, j, i, compute_color(osg::Vec3(-u, -1, v)));
				set_pixel(5, j, i, compute_color(osg::Vec3(u, 1, v)));
			} else {
				set_pixel(0, j, i, compute_color(osg::Vec3(1, v, -u)));
				set_pixel(1, j, i, compute_color(osg::Vec3(-1, v, u)));
				set_pixel(2, j, i, compute_color(osg::Vec3(-u, 1, v)));
				set_pixel(3, j, i, compute_color(osg::Vec3(-u, -1, -v)));
				set_pixel(4, j, i, compute_color(osg::Vec3(-u, v, -1)));
				set_pixel(5, j, i, compute_color(osg::Vec3(u, v, 1)));
			}
			u -= duv;
		}
		v -= duv;
	}
}
