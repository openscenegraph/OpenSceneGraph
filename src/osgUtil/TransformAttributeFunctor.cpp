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

#include <osgUtil/TransformAttributeFunctor>

using namespace osgUtil;

TransformAttributeFunctor::TransformAttributeFunctor(const osg::Matrix& m)
{
    _m = m;
    _im.invert(_m);
}

TransformAttributeFunctor::~TransformAttributeFunctor()
{
}

void TransformAttributeFunctor::apply(osg::Drawable::AttributeType type,unsigned int count,osg::Vec3* begin)
{
    if (type == osg::Drawable::VERTICES)
    {
        osg::Vec3* end = begin+count;
        for (osg::Vec3* itr=begin;itr<end;++itr)
        {
            (*itr) = (*itr)*_m;
        }
    }
    else if (type == osg::Drawable::NORMALS)
    {
        osg::Vec3* end = begin+count;
        for (osg::Vec3* itr=begin;itr<end;++itr)
        {
            // note post mult by inverse for normals.
            (*itr) = osg::Matrix::transform3x3(_im,(*itr));
            (*itr).normalize();
        }
    }
}
