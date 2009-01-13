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

#include <osgVolume/VolumeTile>
#include <osgVolume/Volume>


using namespace osg;
using namespace osgVolume;

/////////////////////////////////////////////////////////////////////////////////
//
// VolumeTile
//
VolumeTile::VolumeTile():
    _volume(0),
    _dirty(false),
    _hasBeenTraversal(false)
{
    setThreadSafeRefUnref(true);
}

VolumeTile::VolumeTile(const VolumeTile& volumeTile,const osg::CopyOp& copyop):
    Group(volumeTile,copyop),
    _volume(0),
    _dirty(false),
    _hasBeenTraversal(false),
    _layers(volumeTile._layers)
{
    if (volumeTile.getVolumeTechnique()) ;
    {
        setVolumeTechnique(osg::clone(volumeTile.getVolumeTechnique()));
    }
}

VolumeTile::~VolumeTile()
{
    if (_volume) setVolume(0);
}
 
void VolumeTile::setLayer(unsigned int i, Layer* layer)
{
    if (_layers.size() <= i) _layers.resize(i+1);
    
    _layers[i] = layer;
}

void VolumeTile::setVolume(Volume* volume)
{
    if (_volume == volume) return;
    
    if (_volume) _volume->unregisterVolumeTile(this);
    
    _volume = volume;

    if (_volume) _volume->registerVolumeTile(this);
}

void VolumeTile::setTileID(const TileID& tileID)
{
    if (_tileID == tileID) return;

    if (_volume) _volume->unregisterVolumeTile(this);

    _tileID = tileID;

    if (_volume) _volume->registerVolumeTile(this);
}


void VolumeTile::traverse(osg::NodeVisitor& nv)
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

void VolumeTile::init()
{
    if (_volumeTechnique.valid() && getDirty())
    {
        _volumeTechnique->init();
        
        setDirty(false);
    }    
}

void VolumeTile::setVolumeTechnique(VolumeTechnique* volumeTechnique)
{
    if (_volumeTechnique == volumeTechnique) return; 

    int dirtyDelta = _dirty ? -1 : 0;

    if (_volumeTechnique.valid()) 
    {
        _volumeTechnique->_volumeTile = 0;
    }

    _volumeTechnique = volumeTechnique;
    
    if (_volumeTechnique.valid()) 
    {
        _volumeTechnique->_volumeTile = this;
        ++dirtyDelta;        
    }
    
    if (dirtyDelta>0) setDirty(true);
    else if (dirtyDelta<0) setDirty(false);
}

void VolumeTile::setDirty(bool dirty)
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

osg::BoundingSphere VolumeTile::computeBound() const
{
    osg::BoundingSphere bs;

    for(Layers::const_iterator itr = _layers.begin();
        itr != _layers.end();
        ++itr)
    {
        if (itr->valid()) bs.expandBy((*itr)->computeBound());
    }
    
    return bs;

    return bs;
}
