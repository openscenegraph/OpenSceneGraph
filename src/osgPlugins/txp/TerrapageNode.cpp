#include "TerrapageNode.h"

#include <osg/Notify>

using namespace osg;

namespace txp
{

TerrapageNode::TerrapageNode():
    _pageManager(0)
{
    setNumChildrenRequiringUpdateTraversal(1);
}
        
TerrapageNode::TerrapageNode(const TerrapageNode& pager,const osg::CopyOp&):
    osg::Group(),
    _databaseDimensions(pager._databaseDimensions),
    _databaseName(pager._databaseName),
    _databaseOptions(pager._databaseOptions),
    _pageManager(0),
    _lastRecordEyePoint(pager._lastRecordEyePoint)
{
    setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);            
}

TerrapageNode::~TerrapageNode()
{
    // will the page manger delete the archive?
    delete _pageManager;
	_pageManager = 0L;
}

void TerrapageNode::traverse(osg::NodeVisitor& nv)
{
    if (_pageManager)
    {
        if (nv.getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR)
        {
            updateSceneGraph();
        }
        else if (nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR)
        {
            updateEyePoint(nv.getEyePoint());
        }
    }
    
    Group::traverse(nv);
}
        
        
bool TerrapageNode::loadDatabase()
{
    // Open the TXP database
    TrPageArchive *txpArchive = new TrPageArchive();
    if (!txpArchive->OpenFile(_databaseName.c_str()))
    {
        osg::notify(osg::WARN)<<"Couldn't load TerraPage archive "<<_databaseName<<std::endl;
        return false;
    }
    
    // Note: Should be checking the return values
    txpArchive->LoadMaterials();

    // Note: Should be checking the return values
    txpArchive->LoadModels();

    // get the exents of the archive
    const trpgHeader *head = txpArchive->GetHeader();
    trpg2dPoint sw,ne;
    head->GetExtents(sw,ne);
    
    _databaseDimensions.set(sw.x,sw.y,0.0f,
                            ne.x,ne.y,0.0f);
    

    // set up the options.
    bool loadAll=false;
    OSGPageManager::ThreadMode threadMode = OSGPageManager::ThreadFree;

    if (!_databaseOptions.empty())
    {
        if (_databaseOptions.find("LoadAll")!=std::string::npos)
        {
            loadAll = true;
            osg::notify(osg::INFO)<<"TerraPage archive : LoadAll selected"<<std::endl;
        }
        
        if (_databaseOptions.find("ThreadNone")!=std::string::npos)
        {
            threadMode = OSGPageManager::ThreadNone;
            osg::notify(osg::INFO)<<"TerraPage archive : ThreadNone selected"<<std::endl;
        }

        if (_databaseOptions.find("ThreadFree")!=std::string::npos)
        {
            threadMode = OSGPageManager::ThreadFree;
            osg::notify(osg::INFO)<<"TerraPage archive : ThreadFree selected"<<std::endl;
        }

        if (_databaseOptions.find("ThreadSync")!=std::string::npos)
        {
            threadMode = OSGPageManager::ThreadSync;
            osg::notify(osg::INFO)<<"TerraPage archive : ThreadSync selected"<<std::endl;
        }
    }

    if (loadAll)
    {
        // Load the whole scenegraph
        osg::Node* node = txpArchive->LoadAllTiles();
        if (!node) {
            osg::notify(osg::WARN)<<"Couldn't load whole TerraPage archive "<<_databaseName<<std::endl;
            return false;
        }
        
        // need to think about ref counting these..
        delete txpArchive;

        // add the loaded database into the scenegraph.
        addChild(node);
        
    } else {

        _pageManager = new OSGPageManager(txpArchive);

        if (threadMode!=OSGPageManager::ThreadNone)
        {
            ThreadID newThread;
            _pageManager->StartThread(threadMode,newThread);
        }

    }

    return true;
}
        

void TerrapageNode::updateSceneGraph()
{
    if (_pageManager)
    {        _bsphere_computed = true;

        if (_pageManager->GetThreadMode() == OSGPageManager::ThreadNone)
        {
            // we're in non-thread mode, load in the given number of tiles (maximum).
	    int numTile = 1;
            _pageManager->UpdateNoThread(this,_lastRecordEyePoint.x(),_lastRecordEyePoint.y(),numTile);
        }
        else
        {
            // we're in ThreadFree mode, merge in whatever may be ready.
            _pageManager->MergeUpdateThread(this);
        }
    }
}
        

void TerrapageNode::updateEyePoint(const osg::Vec3& eyepoint) const
{
    if (_pageManager && (_pageManager->GetThreadMode() != OSGPageManager::ThreadNone))
    {
        _pageManager->UpdatePositionThread(eyepoint.x(),eyepoint.y());
    }
    
    _lastRecordEyePoint = eyepoint;
}
        
bool TerrapageNode::computeBound() const
{
    if (_databaseDimensions.valid())
    {
        _bsphere.init();
        _bsphere.expandBy(_databaseDimensions);
        _bsphere_computed = true;
        return true;
    }
    else
    {
        return Group::computeBound();
    }
}


} // namespace txp
