/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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

#include <osgTerrain/Terrain>
#include <osgTerrain/GeoMipMapRenderer>

using namespace osgTerrain;

Terrain::Terrain()
{
    setNumChildrenRequiringUpdateTraversal(1);
}

Terrain::Terrain(const Terrain& terrain,const osg::CopyOp& copyop):
    Group(terrain,copyop),
    _heightField(terrain._heightField)
{
    setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);
    if (terrain.getRenderer()) setRenderer(dynamic_cast<Renderer*>(terrain.getRenderer()->cloneType()));
}

Terrain::~Terrain()
{
}

void Terrain::traverse(osg::NodeVisitor& nv)
{
    // if app traversal update the frame count.
    if (nv.getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR)
    {
        // if no renderer exists that default to using the GeoMipMapRenderer
        if (!getRenderer()) setRenderer(new GeoMipMapRenderer);
        
        osgUtil::UpdateVisitor* uv = dynamic_cast<osgUtil::UpdateVisitor*>(&nv);
        if (getRenderer() && uv)
        {
            getRenderer()->update(uv);
            return;
        }        
        
    }
    else if (nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR)
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
        if (getRenderer() && cv)
        {
            getRenderer()->cull(cv);
            return;
        }
    }

    // otherwise fallback to the Group::traverse()
    osg::Group::traverse(nv);
}

void Terrain::setHeightField(osg::HeightField* heightField)
{
    _heightField = heightField;
    if (_renderer.valid()) _renderer->initialize();
}

void Terrain::haveModifiedHeightField()
{
    if (_renderer.valid()) _renderer->terrainHasBeenModified();
}

void Terrain::setRenderer(osgTerrain::Renderer* renderer)
{
    // need to figure out how to ensure that only one renderer is
    // used between terrain nodes... issue a warning?
    _renderer = renderer;
    
    if (_renderer.valid())
    {
        _renderer->_terrain = this;
        _renderer->initialize();
    }
}

void Terrain::computeNormalMap()
{
    if (_heightField.valid())
    {
        osg::Image* image = new osg::Image;
        image->allocateImage(_heightField->getNumColumns(),_heightField->getNumRows(),1,GL_RGB,GL_BYTE);

        char* ptr = (char*) image->data();
        for(unsigned int r=0;r<_heightField->getNumRows();++r)
        {
	    for(unsigned int c=0;c<_heightField->getNumColumns();++c)
	    {
	        osg::Vec3 normal = _heightField->getNormal(c,r);
                (*ptr++) = (char)((normal.x()+1.0)*0.5*255);
                (*ptr++) = (char)((normal.y()+1.0)*0.5*255);
                (*ptr++) = (char)((normal.z()+1.0)*0.5*255);
	    }
        }
        
        setNormalMapImage(image);
        
    }
}
