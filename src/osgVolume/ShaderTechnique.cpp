/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2009 Robert Osfield 
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

#include <osgVolume/ShaderTechnique>
#include <osgVolume/VolumeTile>

#include <osg/Geometry>
#include <osg/io_utils>

using namespace osgVolume;

ShaderTechnique::ShaderTechnique()
{
}

ShaderTechnique::ShaderTechnique(const ShaderTechnique& fft,const osg::CopyOp& copyop):
    VolumeTechnique(fft,copyop)
{
}

ShaderTechnique::~ShaderTechnique()
{
}

void ShaderTechnique::init()
{
    osg::notify(osg::NOTICE)<<"ShaderTechnique::init()"<<std::endl;
    
     if (!_volumeTile) return;
   
    _geode = new osg::Geode;
    
    osgVolume::Locator* masterLocator = _volumeTile->getLocator();
    for(unsigned int i = 0;
        i < _volumeTile->getNumLayers() && !masterLocator;
        ++i)
    {
        if (_volumeTile->getLayer(i))
        {
            masterLocator = _volumeTile->getLayer(i)->getLocator();
            osg::notify(osg::NOTICE)<<"assigning locator = "<<masterLocator<<std::endl;
        }
    }

    osg::Matrix matrix;
    if (masterLocator)
    {
        matrix = masterLocator->getTransform();
    }
    
    osg::notify(osg::NOTICE)<<"Matrix = "<<matrix<<std::endl;
    
    {
        osg::Geometry* geom = new osg::Geometry;

        osg::Vec3Array* coords = new osg::Vec3Array(8);
        (*coords)[0] = osg::Vec3d(0.0,0.0,0.0) * matrix;
        (*coords)[1] = osg::Vec3d(1.0,0.0,0.0) * matrix;
        (*coords)[2] = osg::Vec3d(1.0,1.0,0.0) * matrix;
        (*coords)[3] = osg::Vec3d(0.0,1.0,0.0) * matrix;
        (*coords)[4] = osg::Vec3d(0.0,0.0,1.0) * matrix;
        (*coords)[5] = osg::Vec3d(1.0,0.0,1.0) * matrix;
        (*coords)[6] = osg::Vec3d(1.0,1.0,1.0) * matrix;
        (*coords)[7] = osg::Vec3d(0.0,1.0,1.0) * matrix;
        geom->setVertexArray(coords);

        osg::Vec4Array* colours = new osg::Vec4Array(1);
        (*colours)[0].set(1.0f,1.0f,1.0,1.0f);
        geom->setColorArray(colours);
        geom->setColorBinding(osg::Geometry::BIND_OVERALL);

        osg::DrawElementsUShort* drawElements = new osg::DrawElementsUShort(GL_QUADS);
        // bottom
        drawElements->push_back(0);
        drawElements->push_back(1);
        drawElements->push_back(2);
        drawElements->push_back(3);
        
        // bottom
        drawElements->push_back(3);
        drawElements->push_back(2);
        drawElements->push_back(6);
        drawElements->push_back(7);

        // left
        drawElements->push_back(0);
        drawElements->push_back(3);
        drawElements->push_back(7);
        drawElements->push_back(4);

        // right
        drawElements->push_back(5);
        drawElements->push_back(6);
        drawElements->push_back(2);
        drawElements->push_back(1);

        // front
        drawElements->push_back(1);
        drawElements->push_back(0);
        drawElements->push_back(4);
        drawElements->push_back(5);

        // top
        drawElements->push_back(7);
        drawElements->push_back(6);
        drawElements->push_back(5);
        drawElements->push_back(4);

        geom->addPrimitiveSet(drawElements);

        _geode->addDrawable(geom);

    } 

}

void ShaderTechnique::update(osgUtil::UpdateVisitor* uv)
{
//    osg::notify(osg::NOTICE)<<"ShaderTechnique:update(osgUtil::UpdateVisitor* nv):"<<std::endl;
}

void ShaderTechnique::cull(osgUtil::CullVisitor* cv)
{
    // osg::notify(osg::NOTICE)<<"ShaderTechnique::cull(osgUtil::CullVisitor* nv)"<<std::endl;    
    if (_geode.valid())
    {
        _geode->accept(*cv);
    }
}

void ShaderTechnique::cleanSceneGraph()
{
    osg::notify(osg::NOTICE)<<"ShaderTechnique::cleanSceneGraph()"<<std::endl;
}

void ShaderTechnique::traverse(osg::NodeVisitor& nv)
{
    // osg::notify(osg::NOTICE)<<"ShaderTechnique::traverse(osg::NodeVisitor& nv)"<<std::endl;
    if (!_volumeTile) return;

    // if app traversal update the frame count.
    if (nv.getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR)
    {
        if (_volumeTile->getDirty()) _volumeTile->init();

        osgUtil::UpdateVisitor* uv = dynamic_cast<osgUtil::UpdateVisitor*>(&nv);
        if (uv)
        {
            update(uv);
            return;
        }        
        
    }
    else if (nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR)
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
        if (cv)
        {
            cull(cv);
            return;
        }
    }


    if (_volumeTile->getDirty()) 
    {
        osg::notify(osg::INFO)<<"******* Doing init ***********"<<std::endl;
        _volumeTile->init();
    }
}
    
