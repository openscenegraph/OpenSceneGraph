#include <osg/Notify>
#include <osg/BoundingBox>
#include <osg/PagedLOD>
#include <osg/Timer>
#include <osg/MatrixTransform>
#include <osgUtil/CullVisitor>

#include <iostream>
#include <vector>
#include <algorithm>
#include <stdio.h>

#include "TileMapper.h"
#include "TXPNode.h"
#include "TXPPagedLOD.h"



using namespace txp;
using namespace osg;



class RetestCallback : public osg::NodeCallback
{

public:
    RetestCallback()
    {
        timer = osg::Timer::instance(); // get static timer
        prevTime = 0; // should this be instantiated with current time?
    }

    virtual void operator () ( osg::Node * node, osg::NodeVisitor * nv )
    {
        osg::Group *pLOD = (osg::Group *) node; 
        osg::Group *n = NULL;
        if ((pLOD->getNumChildren() > 0) &&
            (n = (osg::Group *) pLOD->getChild(0)) &&
            (n->getNumChildren() == 0))
        {
            osg::Timer_t curTime = timer->tick();
            if ((prevTime + 2.0/timer->getSecondsPerTick() ) < curTime)
            {
                prevTime = curTime;
                pLOD->removeChildren( 0, pLOD->getNumChildren());
            }
        }

        NodeCallback::traverse( node, nv );
    }

protected:
    const osg::Timer* timer;
    osg::Timer_t prevTime;
};



#define TXPNodeERROR(s) osg::notify(osg::NOTICE) << "txp::TXPNode::" << (s) << " error: "

TXPNode::TXPNode():
osg::Group(),
_originX(0.0),
_originY(0.0)
{
    setNumChildrenRequiringUpdateTraversal(1);
    setCullingActive(false);
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

        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
                
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
        if (cv)
        {
//#define PRINT_TILEMAPP_TIMEINFO
#ifdef PRINT_TILEMAPP_TIMEINFO
            const osg::Timer& timer = *osg::Timer::instance();
            osg::Timer_t start = timer.tick();
            std::cout<<"Doing visible tile search"<<std::endl;
#endif // PRINT_TILEMAPP_TIMEINFO
        
            osg::ref_ptr<TileMapper> tileMapper = new TileMapper;
            tileMapper->setLODScale(cv->getLODScale());
            tileMapper->pushReferenceViewPoint(cv->getReferenceViewPoint());
            tileMapper->pushViewport(cv->getViewport());
            tileMapper->pushProjectionMatrix((cv->getProjectionMatrix()));
            tileMapper->pushModelViewMatrix((cv->getModelViewMatrix()), osg::Transform::RELATIVE_RF);

            // traverse the scene graph to search for valid tiles
            accept(*tileMapper);

            tileMapper->popModelViewMatrix();
            tileMapper->popProjectionMatrix();
            tileMapper->popViewport();
            tileMapper->popReferenceViewPoint();

            //std::cout<<"   found " << tileMapper._tileMap.size() << std::endl;
            
            tileMapper->checkValidityOfAllVisibleTiles();
            
            cv->setUserData(tileMapper.get());

#ifdef PRINT_TILEMAPP_TIMEINFO        
            std::cout<<"Completed visible tile search in "<<timer.delta_m(start,timer.tick())<<std::endl;
#endif // PRINT_TILEMAPP_TIMEINFO        

        }        
    
        updateEye(nv);
        break;
    }
    case osg::NodeVisitor::UPDATE_VISITOR:
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

        updateSceneGraph();
        break;
    }
    default:
        break;
    }
    Group::traverse(nv);
}

osg::BoundingSphere TXPNode::computeBound() const
{
#if 1

    // Steve Lunsford 10/28/05 : - Had to avoid using the child group nodes for bounds calculation
    // because apparently the loader doesn't load all the sub-tiles for this node, resulting in a Group::computeBounds
    // call which returns a size smaller than the actual size represented by this node so consequently this node gets culled out.
    // note, submission merged and rearranged by Robert Osfield as an #if #else just in case we need to revert.
    return osg::BoundingSphere( _extents );
#else
    // original code which uses the extents of the children when one is available.
    if (getNumChildren() == 0)
    {
        return osg::BoundingSphere( _extents );
    }
    return Group::computeBound();
#endif
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

bool TXPNode::loadArchive(TXPArchive* archive)
{
    //if (_archive.get())
    //{
    //    TXPNodeERROR("loadArchive()") << "archive already open" << std::endl;
    //    return false;
    //}


   //modified by Brad Anderegg on May-27-08
   //if NULL is passed in we will create a new archive and open the database
   //otherwise we will use the archive provided which should have already been loaded 
   //by ReaderWriterTXP::getArchive(). See line 57-77 of ReaderWriterTXP.cpp.
   if(archive == NULL)
   {
       _archive = new TXPArchive;
       if (_archive->openFile(_archiveName) == false)
       {
           TXPNodeERROR("loadArchive()") << "failed to load archive: \"" << _archiveName << "\"" << std::endl;
           return false;
       }
   }
   else
   {
      _archive = archive;
   } 

    _archive->getOrigin(_originX,_originY);
    _archive->getExtents(_extents);

    int32 numLod;
    _archive->GetHeader()->GetNumLods(numLod);

    trpg2iPoint tileSize;
    _archive->GetHeader()->GetLodSize(0,tileSize);

    _pageManager = new TXPPageManager;

    // We are going to use _pageManager to manage lod 0 only, all other lod
    // are managed by this OSG plugin
    _pageManager->Init(_archive.get(), 1);

    return true;
}

void TXPNode::updateEye(osg::NodeVisitor& nv)
{
    if (!_pageManager)
    {
        osg::notify(osg::NOTICE)<<"TXPNode::updateEye() no pageManager created"<<std::endl;
        return;
    }

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
                osg::Node* node = addPagedLODTile(x,y);
                tile->SetLocalData(node);
                //osg::notify(osg::NOTICE) << "Tile load: " << x << " " << y << " " << lod << std::endl;
            }
            _pageManager->AckLoad();
            
        }
    }
}

osg::Node* TXPNode::addPagedLODTile(int x, int y)
{
    // For TerraPage 2.1 and over this method must only be use with lod = 0.
    // If you look at the code that calls it, it is effectively called only when
    // lod = 0. So all is OK
    int lod = 0;
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
    pagedLOD->setUpdateCallback(new RetestCallback);

    const trpgHeader* header = _archive->GetHeader();
    trpgHeader::trpgTileType tileType;
    header->GetTileOriginType(tileType);
    if(tileType == trpgHeader::TileLocal)
    {
        // add in MatrixTransform node with Matrixd offsets
        // get offsets from tile.bbox
        osg::Vec3d sw(info.bbox._min);
        sw[2] = 0.0;
        osg::Matrix offset;
        offset.setTrans(sw);
        osg::MatrixTransform *tform = new osg::MatrixTransform(offset);
        pagedLOD->setCenter(info.center - sw);
        tform->addChild(pagedLOD);
        _nodesToAdd.push_back(tform);
        return tform;
    }
    else
    {
    _nodesToAdd.push_back(pagedLOD);
    
    return pagedLOD;
}
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


