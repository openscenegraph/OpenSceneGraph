#include "TerrapageNode.h"
#include <osg/Notify>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
using namespace osg;

namespace txp
{

osg::ref_ptr<TrPageArchive> TerrapageNode::_archive = NULL;

TerrapageNode::TerrapageNode():
    _pageManager(0)
{
    setNumChildrenRequiringUpdateTraversal(1);
	_dbLoaded = false;
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
	_dbLoaded = pager._dbLoaded;
}

TerrapageNode::~TerrapageNode()
{
    // will the page manger delete the archive?
    delete _pageManager;
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
	std::string name = osgDB::getSimpleFileName(_databaseName);

	// Here we load subtiles for a tile
	if (strncmp(name.c_str(),"subtiles",8)==0)
	{
		std::string path = osgDB::getFilePath(_databaseName);
		_databaseName = path+"\\archive.txp";

		int lod;
		int x;
		int y;
		sscanf(name.c_str(),"subtiles%d_%dx%d",&lod,&x,&y);

		float64 range;
		TerrapageNode::_archive->GetHeader()->GetLodRange(lod+1,range);

		trpg2dPoint tileSize;
		TerrapageNode::_archive->GetHeader()->GetTileSize(lod+1,tileSize);

		trpg2dPoint sw;
		trpg2dPoint ne;
		TerrapageNode::_archive->GetHeader()->GetExtents(sw,ne);

		for (int ix = 0; ix < 2; ix++)
			for (int iy = 0; iy < 2; iy++)
			{
				int tileX = x*2+ix;
				int tileY = y*2+iy;
				int tileLOD = lod+1;

				int parentID;
				addChild(TerrapageNode::_archive->LoadTile(tileX,tileY,tileLOD,parentID));
			}


		//std::cout << "subtiles paged in: " << x <<  " " << y << " " << lod << std::endl;
		return true;
	}

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

    // Note: Should be checking the return values
    txpArchive->LoadLightAttributes();

	if (TerrapageNode::_archive == NULL)
	{
		TerrapageNode::_archive = new TrPageArchive();
		if (!TerrapageNode::_archive->OpenFile(_databaseName.c_str()))
		{
			osg::notify(osg::WARN)<<"Couldn't load interanal TerraPage archive "<<_databaseName<<std::endl;
			TerrapageNode::_archive = NULL;
			return false;
		}

		TerrapageNode::_archive->LoadMaterials();
		TerrapageNode::_archive->LoadModels();
		TerrapageNode::_archive->LoadLightAttributes();
	}

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

	_dbLoaded = true;

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
