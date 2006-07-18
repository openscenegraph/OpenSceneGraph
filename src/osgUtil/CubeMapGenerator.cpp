/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/
#include <osgUtil/CubeMapGenerator>
#include <stdlib.h>

#include <osg/Matrix>

using namespace osgUtil;

CubeMapGenerator::CubeMapGenerator(int texture_size)
:    osg::Referenced(),
    texture_size_(texture_size)
{
    for (int i=0; i<6; ++i)
    {                
        osg::ref_ptr<osg::Image> image = new osg::Image;
        unsigned char* data = new unsigned char [texture_size*texture_size*4];
        image->setImage(texture_size, texture_size, 1, 4, GL_RGBA, GL_UNSIGNED_BYTE, data, osg::Image::USE_NEW_DELETE);
        images_.push_back(image);
    }
}

CubeMapGenerator::CubeMapGenerator(const CubeMapGenerator &copy, const osg::CopyOp &copyop)
:    osg::Referenced(copy),
    texture_size_(copy.texture_size_)
{
    Image_list::const_iterator i;
    for (i=copy.images_.begin(); i!=copy.images_.end(); ++i)
    {
        images_.push_back(static_cast<osg::Image *>(copyop(i->get())));
    }
}

void CubeMapGenerator::generateMap(bool use_osg_system)
{
    osg::Matrix M;
    
    if (use_osg_system) {
        M = osg::Matrix::rotate(osg::PI_2, osg::Vec3(1, 0, 0));
    } else {
        M = osg::Matrix::identity();
    }

    const float dst = 2.0f/(texture_size_-1);
    
    float t = -1;
    for (int i=0; i<texture_size_; ++i) {
        float s = -1;
        for (int j=0; j<texture_size_; ++j) {
            set_pixel(0, j, i, compute_color(osg::Vec3(1, -t, -s)  * M));
            set_pixel(1, j, i, compute_color(osg::Vec3(-1, -t, s)  * M));
            set_pixel(2, j, i, compute_color(osg::Vec3(s, 1, t)    * M));
            set_pixel(3, j, i, compute_color(osg::Vec3(s, -1, -t)  * M));
            set_pixel(4, j, i, compute_color(osg::Vec3(s, -t, 1)   * M));
            set_pixel(5, j, i, compute_color(osg::Vec3(-s, -t, -1) * M));
            s += dst;
        }
        t += dst;
    }
}
