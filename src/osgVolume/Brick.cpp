/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2008 Robert Osfield 
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

#include <osgVolume/Brick>
#include <osgVolume/Volume>


using namespace osg;
using namespace osgVolume;

/////////////////////////////////////////////////////////////////////////////////
//
// Brick
//
Brick::Brick():
    _volume(0),
    _dirty(false),
    _hasBeenTraversal(false)
{
    setThreadSafeRefUnref(true);
}

Brick::Brick(const Brick& brick,const osg::CopyOp& copyop):
    Group(brick,copyop),
    _volume(0),
    _dirty(false),
    _hasBeenTraversal(false),
    _image(brick._image)
{
    if (brick.getVolumeTechnique()) 
    {
        setVolumeTechnique(osg::clone(brick.getVolumeTechnique()));
    }
}

Brick::~Brick()
{
    if (_volume) setVolume(0);
}

void Brick::setVolume(Volume* volume)
{
    if (_volume == volume) return;
    
    if (_volume) _volume->unregisterBrick(this);
    
    _volume = volume;

    if (_volume) _volume->registerBrick(this);
}

void Brick::setBrickID(const BrickID& brickID)
{
    if (_brickID == brickID) return;

    if (_volume) _volume->unregisterBrick(this);

    _brickID = brickID;

    if (_volume) _volume->registerBrick(this);
}


void Brick::traverse(osg::NodeVisitor& nv)
{
    if (!_hasBeenTraversal)
    {
        if (!_volume)
        {
            osg::NodePath& nodePath = nv.getNodePath();
            if (!nodePath.empty())
            {
                for(osg::NodePath::reverse_iterator itr = nodePath.rbegin();
                    itr != nodePath.rend() && !_volume;
                    ++itr)
                {
                    osgVolume::Volume* volume = dynamic_cast<Volume*>(*itr);
                    if (volume) 
                    {
                        osg::notify(osg::INFO)<<"Assigning volume system "<<volume<<std::endl;                        
                        setVolume(volume);
                    }
                }
            }
        }
            
        _hasBeenTraversal = true;
    }

    if (_volumeTechnique.valid())
    {
        _volumeTechnique->traverse(nv);
    }
    else
    {
        osg::Group::traverse(nv);
    }
}

void Brick::init()
{
    if (_volumeTechnique.valid() && getDirty())
    {
        _volumeTechnique->init();
        
        setDirty(false);
    }    
}

void Brick::setVolumeTechnique(VolumeTechnique* volumeTechnique)
{
    if (_volumeTechnique == volumeTechnique) return; 

    int dirtyDelta = _dirty ? -1 : 0;

    if (_volumeTechnique.valid()) 
    {
        _volumeTechnique->_brick = 0;
    }

    _volumeTechnique = volumeTechnique;
    
    if (_volumeTechnique.valid()) 
    {
        _volumeTechnique->_brick = this;
        ++dirtyDelta;        
    }
    
    if (dirtyDelta>0) setDirty(true);
    else if (dirtyDelta<0) setDirty(false);
}

void Brick::setDirty(bool dirty)
{
    if (_dirty==dirty) return;

    _dirty = dirty;

    if (_dirty)
    {
        setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);
    }
    else if (getNumChildrenRequiringUpdateTraversal()>0) 
    {
        setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()-1);
    }
}

osg::BoundingSphere Brick::computeBound() const
{
    osg::BoundingSphere bs;

    osg::notify(osg::NOTICE)<<"TODO Brick::computeBound()"<<std::endl;    
    
    return bs;
}
