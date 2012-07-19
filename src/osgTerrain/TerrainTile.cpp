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

#include <osgTerrain/TerrainTile>
#include <osgTerrain/Terrain>
#include <osgTerrain/GeometryTechnique>

#include <osg/ClusterCullingCallback>

#include <osgDB/ReadFile>


using namespace osg;
using namespace osgTerrain;

/////////////////////////////////////////////////////////////////////////////////
//
// TileID
//
TileID::TileID():
    level(-1),
    x(-1),
    y(-1)
{
}

TileID::TileID(int in_level, int in_x, int in_y):
    level(in_level),
    x(in_x),
    y(in_y)
{
}

/////////////////////////////////////////////////////////////////////////////////
//
// TerrainTile
//
void TerrainTile::setTileLoadedCallback(TerrainTile::TileLoadedCallback* lc)
{
    getTileLoadedCallback() = lc;
}

osg::ref_ptr<TerrainTile::TileLoadedCallback>& TerrainTile::getTileLoadedCallback()
{
    static osg::ref_ptr<TileLoadedCallback> s_TileLoadedCallback;
    return s_TileLoadedCallback;
}

TerrainTile::TerrainTile():
    _terrain(0),
    _dirtyMask(NOT_DIRTY),
    _hasBeenTraversal(false),
    _requiresNormals(true),
    _treatBoundariesToValidDataAsDefaultValue(false),
    _blendingPolicy(INHERIT)
{
    setThreadSafeRefUnref(true);
}

TerrainTile::TerrainTile(const TerrainTile& terrain,const osg::CopyOp& copyop):
    Group(terrain,copyop),
    _terrain(0),
    _dirtyMask(NOT_DIRTY),
    _hasBeenTraversal(false),
    _elevationLayer(terrain._elevationLayer),
    _colorLayers(terrain._colorLayers),
    _requiresNormals(terrain._requiresNormals),
    _treatBoundariesToValidDataAsDefaultValue(terrain._treatBoundariesToValidDataAsDefaultValue),
    _blendingPolicy(terrain._blendingPolicy)
{
    if (terrain.getTerrainTechnique())
    {
        setTerrainTechnique(dynamic_cast<TerrainTechnique*>(terrain.getTerrainTechnique()->cloneType()));
    }
}

TerrainTile::~TerrainTile()
{
    if (_terrainTechnique.valid())
    {
        _terrainTechnique->setTerrainTile(0);
    }

    if (_terrain) setTerrain(0);
}

void TerrainTile::setTerrain(Terrain* ts)
{
    if (_terrain == ts) return;

    if (_terrain) _terrain->unregisterTerrainTile(this);

    _terrain = ts;

    if (_terrain) _terrain->registerTerrainTile(this);
}

void TerrainTile::setTileID(const TileID& tileID)
{
    if (_tileID == tileID) return;

    if (_terrain) _terrain->unregisterTerrainTile(this);

    _tileID = tileID;

    if (_terrain) _terrain->registerTerrainTile(this);
}


void TerrainTile::traverse(osg::NodeVisitor& nv)
{
    if (!_hasBeenTraversal)
    {
        if (!_terrain)
        {
            osg::NodePath& nodePath = nv.getNodePath();
            if (!nodePath.empty())
            {
                for(osg::NodePath::reverse_iterator itr = nodePath.rbegin();
                    itr != nodePath.rend() && !_terrain;
                    ++itr)
                {
                    osgTerrain::Terrain* ts = dynamic_cast<Terrain*>(*itr);
                    if (ts)
                    {
                        OSG_INFO<<"Assigning terrain system "<<ts<<std::endl;
                        setTerrain(ts);
                    }
                }
            }
        }

        init(getDirtyMask(), false);

        _hasBeenTraversal = true;
    }

    if (nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR)
    {
        osg::ClusterCullingCallback* ccc = dynamic_cast<osg::ClusterCullingCallback*>(getCullCallback());
        if (ccc)
        {
            if (ccc->cull(&nv,0,static_cast<State *>(0))) return;
        }
    }

    if (_terrainTechnique.valid())
    {
        _terrainTechnique->traverse(nv);
    }
    else
    {
        osg::Group::traverse(nv);
    }
}

void TerrainTile::init(int dirtyMask, bool assumeMultiThreaded)
{
    if (!_terrainTechnique)
    {
        if (_terrain && _terrain->getTerrainTechniquePrototype())
        {
            osg::ref_ptr<osg::Object> object = _terrain->getTerrainTechniquePrototype()->clone(osg::CopyOp::DEEP_COPY_ALL);
            setTerrainTechnique(dynamic_cast<TerrainTechnique*>(object.get()));
        }
        else
        {
            setTerrainTechnique(new GeometryTechnique);
        }
    }

    if (_terrainTechnique.valid())
    {
        _terrainTechnique->init(dirtyMask, assumeMultiThreaded);
    }
}

void TerrainTile::setTerrainTechnique(TerrainTechnique* terrainTechnique)
{
    if (_terrainTechnique == terrainTechnique) return;

    int dirtyDelta = (_dirtyMask==NOT_DIRTY) ? 0 : -1;

    if (_terrainTechnique.valid())
    {
        _terrainTechnique->setTerrainTile(0);
    }

    _terrainTechnique = terrainTechnique;

    if (_terrainTechnique.valid())
    {
        _terrainTechnique->setTerrainTile(this);
        ++dirtyDelta;
    }

    if (dirtyDelta>0) setDirtyMask(ALL_DIRTY);
    else if (dirtyDelta<0) setDirtyMask(NOT_DIRTY);
}

void TerrainTile::setDirtyMask(int dirtyMask)
{
    if (_dirtyMask==dirtyMask) return;

    int dirtyDelta = (_dirtyMask==NOT_DIRTY) ? 0 : -1;

    _dirtyMask = dirtyMask;

    if (_dirtyMask!=NOT_DIRTY) dirtyDelta += 1;

#if 1
    if (dirtyDelta>0)
    {
        // need to signal that we need an update
        if (_terrain) _terrain->updateTerrainTileOnNextFrame(this);
    }
#else

    // setNumChildrenRequeingUpdateTraversal() isn't thread safe so should avoid using it.
    if (dirtyDelta>0)
    {
        setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);
    }
    else if (dirtyDelta<0 && getNumChildrenRequiringUpdateTraversal()>0)
    {
        setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()-1);
    }
#endif
}



void TerrainTile::setElevationLayer(Layer* layer)
{
    _elevationLayer = layer;
}

void TerrainTile::setColorLayer(unsigned int i, Layer* layer)
{
    if (_colorLayers.size() <= i) _colorLayers.resize(i+1);

    _colorLayers[i] = layer;
}

osg::BoundingSphere TerrainTile::computeBound() const
{
    osg::BoundingSphere bs;

    if (_elevationLayer.valid())
    {
        bs.expandBy(_elevationLayer->computeBound(true));
    }
    else
    {
        for(Layers::const_iterator itr = _colorLayers.begin();
            itr != _colorLayers.end();
            ++itr)
        {
            if (itr->valid()) bs.expandBy((*itr)->computeBound(false));
        }
    }

    return bs;
}


/////////////////////////////////////////////////////////////////////////////////
//
// WhiteListTileLoadedCallback
//
WhiteListTileLoadedCallback::WhiteListTileLoadedCallback()
{
    _minumumNumberOfLayers = 0;
    _replaceSwitchLayer = false;
    _allowAll = false;
}

WhiteListTileLoadedCallback::~WhiteListTileLoadedCallback()
{
}

bool WhiteListTileLoadedCallback::layerAcceptable(const std::string& setname) const
{
    if (_allowAll) return true;

    if (setname.empty()) return true;

    return _setWhiteList.count(setname)!=0;
}

bool WhiteListTileLoadedCallback::readImageLayer(osgTerrain::ImageLayer* imageLayer, const osgDB::ReaderWriter::Options* options) const
{
   if (!imageLayer->getImage() &&
        !imageLayer->getFileName().empty())
    {
        if (layerAcceptable(imageLayer->getSetName()))
        {
            osg::ref_ptr<osg::Image> image = osgDB::readImageFile(imageLayer->getFileName(), options);
            imageLayer->setImage(image.get());
        }
    }
    return imageLayer->getImage()!=0;
}

bool WhiteListTileLoadedCallback::deferExternalLayerLoading() const
{
    return true;
}

void WhiteListTileLoadedCallback::loaded(osgTerrain::TerrainTile* tile, const osgDB::ReaderWriter::Options* options) const
{

    // read any external layers
    for(unsigned int i=0; i<tile->getNumColorLayers(); ++i)
    {
        osgTerrain::Layer* layer = tile->getColorLayer(i);
        osgTerrain::ImageLayer* imageLayer = dynamic_cast<osgTerrain::ImageLayer*>(layer);
        if (imageLayer)
        {
            readImageLayer(imageLayer, options);
            continue;
        }

        osgTerrain::SwitchLayer* switchLayer = dynamic_cast<osgTerrain::SwitchLayer*>(layer);
        if (switchLayer)
        {
            for(unsigned int si=0; si<switchLayer->getNumLayers(); ++si)
            {
                osgTerrain::ImageLayer* imageLayer = dynamic_cast<osgTerrain::ImageLayer*>(switchLayer->getLayer(si));
                if (imageLayer)
                {
                    if (readImageLayer(imageLayer, options))
                    {
                        // replace SwitchLayer by
                        if (_replaceSwitchLayer) tile->setColorLayer(i, imageLayer);
                        else if (switchLayer->getActiveLayer()<0) switchLayer->setActiveLayer(si);

                        continue;
                    }
                }
            }
            continue;
        }

        osgTerrain::CompositeLayer* compositeLayer = dynamic_cast<osgTerrain::CompositeLayer*>(layer);
        if (compositeLayer)
        {
            for(unsigned int ci=0; ci<compositeLayer->getNumLayers(); ++ci)
            {
                osgTerrain::ImageLayer* imageLayer = dynamic_cast<osgTerrain::ImageLayer*>(compositeLayer->getLayer(ci));
                if (imageLayer)
                {
                    readImageLayer(imageLayer, options);
                }
            }
            continue;
        }
    }

    // assign colour layers over missing layers
    osgTerrain::Layer* validLayer = 0;
    for(unsigned int i=0; i<tile->getNumColorLayers(); ++i)
    {
        osgTerrain::Layer* layer = tile->getColorLayer(i);
        osgTerrain::ImageLayer* imageLayer = dynamic_cast<osgTerrain::ImageLayer*>(layer);
        if (imageLayer)
        {
            if (imageLayer->getImage()!=0)
            {
                validLayer = imageLayer;
            }
            continue;
        }

        osgTerrain::SwitchLayer* switchLayer = dynamic_cast<osgTerrain::SwitchLayer*>(layer);
        if (switchLayer)
        {
            for(unsigned int si=0; si<switchLayer->getNumLayers(); ++si)
            {
                osgTerrain::ImageLayer* imageLayer = dynamic_cast<osgTerrain::ImageLayer*>(switchLayer->getLayer(si));
                if (imageLayer && imageLayer->getImage()!=0)
                {
                    validLayer = imageLayer;
                }
            }
            continue;
        }

        osgTerrain::CompositeLayer* compositeLayer = dynamic_cast<osgTerrain::CompositeLayer*>(layer);
        if (compositeLayer)
        {
            for(unsigned int ci=0; ci<compositeLayer->getNumLayers(); ++ci)
            {
                osgTerrain::ImageLayer* imageLayer = dynamic_cast<osgTerrain::ImageLayer*>(switchLayer->getLayer(ci));
                if (imageLayer && imageLayer->getImage()!=0)
                {
                    validLayer = imageLayer;
                }
            }
            continue;
        }
    }

    if (validLayer)
    {
        // fill in any missing layers
        for(unsigned int i=0; i<tile->getNumColorLayers(); ++i)
        {
            osgTerrain::Layer* layer = tile->getColorLayer(i);
            osgTerrain::ImageLayer* imageLayer = dynamic_cast<osgTerrain::ImageLayer*>(layer);
            if (imageLayer)
            {
                if (imageLayer->getImage()==0)
                {
                    tile->setColorLayer(i, validLayer);
                    break;
                }
                continue;
            }

            osgTerrain::SwitchLayer* switchLayer = dynamic_cast<osgTerrain::SwitchLayer*>(layer);
            if (switchLayer)
            {
                for(unsigned int si=0; si<switchLayer->getNumLayers(); ++si)
                {
                    osgTerrain::ImageLayer* imageLayer = dynamic_cast<osgTerrain::ImageLayer*>(switchLayer->getLayer(si));
                    if (imageLayer && imageLayer->getImage()==0)
                    {
                        if (_replaceSwitchLayer) tile->setColorLayer(i, imageLayer);
                        else
                        {
                            switchLayer->setLayer(si, validLayer);
                            if (switchLayer->getActiveLayer()<0) switchLayer->setActiveLayer(si);
                        }
                        break;
                    }
                }
                if (switchLayer->getNumLayers()==0)
                {
                    if (_replaceSwitchLayer) tile->setColorLayer(i, validLayer);
                    else
                    {
                        switchLayer->setLayer(0, validLayer);
                        switchLayer->setActiveLayer(0);
                    }
                }
            }

            osgTerrain::CompositeLayer* compositeLayer = dynamic_cast<osgTerrain::CompositeLayer*>(layer);
            if (compositeLayer)
            {
                for(unsigned int ci=0; ci<compositeLayer->getNumLayers(); ++ci)
                {
                    osgTerrain::ImageLayer* imageLayer = dynamic_cast<osgTerrain::ImageLayer*>(switchLayer->getLayer(ci));
                    if (imageLayer && imageLayer->getImage()==0)
                    {
                        tile->setColorLayer(i, validLayer);
                        break;
                    }
                }
                continue;
            }
        }

        if (_minumumNumberOfLayers>tile->getNumColorLayers())
        {
            for(unsigned int i=tile->getNumColorLayers(); i<_minumumNumberOfLayers; ++i)
            {
                tile->setColorLayer(i, validLayer);
            }
        }

    }
}

void TerrainTile::releaseGLObjects(osg::State* state) const
{
    Group::releaseGLObjects(state);

    if (_terrainTechnique.valid())
    {
        _terrainTechnique->releaseGLObjects( state );
    }
}

