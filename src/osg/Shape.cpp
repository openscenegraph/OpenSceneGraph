/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield 
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
#include <osg/Shape>
#include <algorithm>

using namespace osg;

Shape::~Shape()
{
}

HeightField::~HeightField()
{
}


void HeightField::allocate(unsigned int numColumns,unsigned int numRows)
{
    if (_columns!=numColumns || _rows!=numRows)
    {
    	_heights.resize(numColumns*numRows);
    }
    _columns=numColumns;
    _rows=numRows;
}

Vec3 HeightField::getNormal(unsigned int c,unsigned int r) const 
{
    // four point normal generation.

    float dz_dx;
    if (c==0)
    {
    	dz_dx = (getHeight(c+1,r)-getHeight(c,r))/getXInterval();
    }
    else if (c==getNumColumns()-1)
    {
    	dz_dx = (getHeight(c,r)-getHeight(c-1,r))/getXInterval();
    }
    else // assume 0<c<_numColumns-1
    {
    	dz_dx = 0.5f*(getHeight(c+1,r)-getHeight(c-1,r))/getXInterval();
    }
    
    float dz_dy;
    if (r==0)
    {
    	dz_dy = (getHeight(c,r+1)-getHeight(c,r))/getYInterval();
    }
    else if (r==getNumRows()-1)
    {
    	dz_dy = (getHeight(c,r)-getHeight(c,r-1))/getYInterval();
    }
    else // assume 0<r<_numRows-1
    {
    	dz_dy = 0.5f*(getHeight(c,r+1)-getHeight(c,r-1))/getYInterval();
    }
    
    Vec3 normal(-dz_dx,-dz_dy,1.0f);
    normal.normalize();
    
    return normal;
}


