#include <osg/Notify>
#include <osg/BoundingBox>
#include <osg/PagedLOD>
#include <osg/Timer>
#include <osgUtil/CullVisitor>

#include <iostream>
#include <vector>
#include <algorithm>

#include "TileMapper.h"
#include "TXPNode.h"
#include "TXPPagedLOD.h"



using namespace txp;




#define TXPNodeERROR(s) osg::notify(osg::NOTICE) << "txp::TXPNode::" << (s) << " error: "

TXPNode::TXPNode():
osg::Group(),
_originX(0.0),
_originY(0.0)
{
    setNumChildrenRequiringUpdateTraversal(1);
}
            
TXPNode::TXPNode(const TXPNode& txpNode,const osg::CopyOp& copyop):
osg::Group(txpNode,copyop),
_originX(txpNode._originX),
_originY(txpNode._originY)
{
    setNumChildrenRequiringUpdateTraversal(1);
}

TXPNode::~TXPNode()
{
}

TXPArchive* TXPNode::getArchive()
{
    return _archive.get();
}

void TXPNode::traverse(osg::NodeVisitor& nv)
{
    switch(nv.getVisitorType())
    {
    case osg::NodeVisitor::CULL_VISITOR:
    {
                
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
        if (cv)
        {
            //const osg::Timer& timer = *osg::Timer::instance();
            //osg::Timer_t start = timer.tick();
            //std::cout<<"Doing visible tile search"<<std::endl;
        
            osg::ref_ptr<TileMapper> tileMapper = new TileMapper;
            tileMapper->pushViewport(cv->getViewport());
            tileMapper->pushProjectionMatrix(&(cv->getProjectionMatrix()));
            tileMapper->pushModelViewMatrix(&(cv->getModelViewMatrix()));

            // traverse the scene graph to search for valid tiles
            accept(*tileMapper);

            tileMapper->popModelViewMatrix();
            tileMapper->popProjectionMatrix();
            tileMapper->popViewport();

            //std::cout<<"   found " << tileMapper._tileMap.size() << std::endl;
            
            tileMapper->checkValidityOfAllVisibleTiles();
            
            cv->setUserData(tileMapper.get());

            //std::cout<<"Completed visible tile search in "<<timer.delta_m(start,timer.tick())<<std::endl;

        }        
    
        updateEye(nv);
        break;
    }
    case osg::NodeVisitor::UPDATE_VISITOR:
        updateSceneGraph();
        break;
    default:
        break;
    }
    Group::traverse(nv);
}

bool TXPNode::computeBound() const
{
    if (getNumChildren() == 0)
    {
        _bsphere.init();
        _bsphere.expandBy(_extents);
        _bsphere_computed = true;
        return true;
    }
    return Group::computeBound();
}

void TXPNode::setArchiveName(const std::string& archiveName)
{
    _archiveName = archiveName;
}

void TXPNode::setOptions(const std::string& options)
{
    _options = options;
}

const std::string& TXPNode::getOptions() const
{
    return _options;
}

const std::string& TXPNode::getArchiveName() const
{
    return _archiveName;
}

bool TXPNode::loadArchive()
{
    if (_archive.get())
    {
        TXPNodeERROR("loadArchive()") << "archive already open" << std::endl;
        return false;
    }

    _archive = new TXPArchive;
    if (_archive->openFile(_archiveName) == false)
    {
        TXPNodeERROR("loadArchive()") << "failed to load archive: \"" << _archiveName << "\"" << std::endl;
        return false;
    }

    /*
    if (_archive->loadMaterials() == false)
    {
        TXPNodeERROR("loadArchive()") << "failed to load materials from archive: \"" << _archiveName << "\"" << std::endl;
        return false;
    }

    if (_archive->loadModels() == false)
    {
        TXPNodeERROR("loadArchive()") << "failed to load models from archive: \"" << _archiveName << "\"" << std::endl;
        return false;
    }

    if (_archive->loadLightAttributes() == false)
    {
        TXPNodeERROR("loadArchive()") << "failed to load light attributes from archive: \"" << _archiveName << "\"" << std::endl;
        return false;
    }
    */

    _archive->getOrigin(_originX,_originY);
    _archive->getExtents(_extents);

    int32 numLod;
    _archive->GetHeader()->GetNumLods(numLod);

    trpg2iPoint tileSize;
    _archive->GetHeader()->GetLodSize(0,tileSize);

    _pageManager = new TXPPageManager;
    _pageManager->Init(_archive.get());

    return true;
}

void TXPNode::updateEye(osg::NodeVisitor& nv)
{
    trpg2dPoint loc;
    loc.x = nv.getEyePoint().x() - _originX;
    loc.y = nv.getEyePoint().y() - _originY;

    if (_pageManager->SetLocation(loc))
    {
        trpgManagedTile *tile=NULL;

        while((tile = _pageManager->GetNextUnload()))
        {
            int x,y,lod;
            tile->GetTileLoc(x,y,lod);
            if (lod == 0)
            {
                osg::Node* node = (osg::Node*)(tile->GetLocalData());
                _nodesToRemove.push_back(node);

                //osg::notify(osg::NOTICE) << "Tile unload: " << x << " " << y << " " << lod << std::endl;
            }
            _pageManager->AckUnload();
        }

        while ((tile = _pageManager->GetNextLoad()))
        {
            int x,y,lod;
            tile->GetTileLoc(x,y,lod);
            if (lod==0)
            {
                osg::Node* node = addPagedLODTile(x,y,lod);
                tile->SetLocalData(node);
                //osg::notify(osg::NOTICE) << "Tile load: " << x << " " << y << " " << lod << std::endl;
            }
            _pageManager->AckLoad();
            
        }
    }
}

osg::Node* TXPNode::addPagedLODTile(int x, int y, int lod)
{
    char pagedLODfile[1024];
    sprintf(pagedLODfile,"%s\\tile%d_%dx%d_%d.txp",_archive->getDir(),lod,x,y,_archive->getId());


    TXPArchive::TileInfo info;
    _archive->getTileInfo(x,y,lod,info);

    osg::PagedLOD* pagedLOD = new osg::PagedLOD;
    pagedLOD->setFileName(0,pagedLODfile);
    pagedLOD->setPriorityOffset(0,_archive->getNumLODs());
    pagedLOD->setPriorityScale(0,1.0f);
    pagedLOD->setRange(0,0.0,info.maxRange);
    pagedLOD->setCenter(info.center);
    pagedLOD->setRadius(info.radius);
    pagedLOD->setNumChildrenThatCannotBeExpired(1);

    _nodesToAdd.push_back(pagedLOD);
    
    return pagedLOD;
}

void TXPNode::updateSceneGraph()
{
    if (!_nodesToRemove.empty())
    {
        for (unsigned int i = 0; i < _nodesToRemove.size(); i++)
        {
            removeChild(_nodesToRemove[i]);
        }
        _nodesToRemove.clear();
    }

    if (!_nodesToAdd.empty())
    {
        for (unsigned int i = 0; i < _nodesToAdd.size(); i++)
        {
            addChild(_nodesToAdd[i]);
        }
        _nodesToAdd.clear();
        
    }    
}


