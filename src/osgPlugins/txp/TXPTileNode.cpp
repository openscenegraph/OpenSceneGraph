#include <osg/PagedLOD>

#include "TileMapper.h"
#include "TXPTileNode.h"
#include "TXPArchive.h"
#include "TXPSeamLOD.h"
#include "TXPPagedLOD.h"
using namespace txp;

class PrintVisitor : public osg::NodeVisitor
{

   public:
   
        PrintVisitor():NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN)
        {
            _indent = 0;
            _step = 4;
        }
        
        inline void moveIn() { _indent += _step; }
        inline void moveOut() { _indent -= _step; }
        inline void writeIndent() 
        {
            for(int i=0;i<_indent;++i) std::cout << " ";
        }
                
        virtual void apply(osg::Node& node)
        {
            moveIn();
            writeIndent(); std::cout << node.className() <<std::endl;
            traverse(node);
            moveOut();
        }

        virtual void apply(osg::Geode& node)         { apply((osg::Node&)node); }
        virtual void apply(osg::Billboard& node)     { apply((osg::Geode&)node); }
        virtual void apply(osg::LightSource& node)   { apply((osg::Group&)node); }
        virtual void apply(osg::ClipNode& node)      { apply((osg::Group&)node); }
        
        virtual void apply(osg::Group& node)         { apply((osg::Node&)node); }
        virtual void apply(osg::Transform& node)     { apply((osg::Group&)node); }
        virtual void apply(osg::Projection& node)    { apply((osg::Group&)node); }
        virtual void apply(osg::Switch& node)        { apply((osg::Group&)node); }
        virtual void apply(osg::LOD& lod)
        {
            moveIn();
            writeIndent(); std::cout << lod.className() <<std::endl;
            if (lod.className()=="TXPPagedLOD")
            {
                TXPPagedLOD *plod = (TXPPagedLOD*)&lod;
                writeIndent(); std::cout << "X="<<plod->_tileX<<" Y="<<plod->_tileY<<" LOD="<<plod->_tileLOD<< std::endl;
            }
            //writeIndent(); std::cout << "center="<<lod.getCenter()<<std::endl;
            for(unsigned int i=0;i<lod.getNumChildren();++i)
            {
                //writeIndent(); std::cout << "child min="<<lod.getMinRange(i)<<" max="<<lod.getMaxRange(i)<<std::endl;
                lod.getChild(i)->accept(*this);
            }
            moveOut();
        }
        virtual void apply(osg::Impostor& node)      { apply((osg::LOD&)node); }

   protected:
    
        int _indent;
        int _step;
};

TXPTileNode::TXPTileNode():
osg::Group(),
_archive(0)
{
}
        
TXPTileNode::TXPTileNode(const TXPTileNode& tn,const osg::CopyOp& copyop):
osg::Group(tn,copyop),
_archive(tn._archive)
{
}

TXPTileNode::~TXPTileNode()
{
}

void TXPTileNode::setArchive(TXPArchive* archive)
{
    _archive = archive;
}

class SeamFinder: public osg::NodeVisitor
{
public:
    SeamFinder(int x, int y, int lod, TXPArchive::TileInfo& info, TXPArchive *archive ):
    NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN),
    _x(x), _y(y), _lod(lod), _info(info), _archive(archive) 
    {};

    virtual void apply(osg::Group& group)
    {
        for (unsigned int i = 0; i < group.getNumChildren(); i++)
        {
            osg::Node* child = group.getChild(i);
            osg::Node* seam = seamReplacement(child);
            if (child != seam)
            {
                group.replaceChild(child,seam);
            }
            else
            {
                child->accept(*this);
            }
        }
    }

protected:
    osg::Node* seamReplacement(osg::Node* node);

    int _x, _y, _lod;
    TXPArchive::TileInfo& _info;
    TXPArchive *_archive;
};

osg::Node* SeamFinder::seamReplacement(osg::Node* node)
{
    osg::Group* group = node->asGroup();
    if (group == 0) return node;

    std::vector<osg::Node*> nonSeamChildren;
    osg::LOD* hiRes = 0;
    osg::LOD* loRes = 0;

    for (unsigned int i = 0; i < group->getNumChildren(); i++)
    {
        osg::LOD* lod = dynamic_cast<osg::LOD*>(group->getChild(i));
        if (lod == 0)
        {
            nonSeamChildren.push_back(group->getChild(i));
            continue;
        }

        bool nonSeamChild = true;

        // seam center is outside the bounding box of the tile
        if (!_info.bbox.contains(lod->getCenter()))
        {
            // seams have center as the neighbour tile
            osg::Vec3 d = _info.center - lod->getCenter();
            if (((fabs(d.x())-_info.size.x()) > 0.0001) && ((fabs(d.y())-_info.size.y()) > 0.0001))
            {
                nonSeamChildren.push_back(lod);
                continue;
            }

            // low res seam has min/max ranges of lod+1 range/lod 0 range
            if ((fabs(_info.minRange-lod->getMinRange(0))<0.001)&&(fabs(_info.lod0Range-lod->getMaxRange(0))<0.001))
            {

                if (loRes==0)
                {
                    loRes = lod;
                    nonSeamChild = false;
                }
            }

            // hi res seam has min/max ranges of 0 range/lod+1 range
            if ((lod->getMinRange(0)==0.0f)&&(fabs(_info.minRange-lod->getMaxRange(0))<0.001))
            {
                if (hiRes==0)
                {
                    hiRes = lod;
                    nonSeamChild = false;
                }
            }
        }
        if (nonSeamChild)
        {
            nonSeamChildren.push_back(lod);
        }
    }

    if (loRes)
    {
        int x = _x;
        int y = _y;
        int lod = _lod;
        osg::Vec3 delta = loRes->getCenter()-_info.center;
        if (fabs(delta.x())>fabs(delta.y()))
        {
            if (delta.x()<0.0) --x;    // west
            else x++;                // east
        }
        else
        {
            if (delta.y()<0.0) --y;    // south
            else ++y;                // north
        }
        TXPSeamLOD* seam = new TXPSeamLOD(
            x,
            y,
            lod,
            loRes->getCenter(),
            0.f,
            _info.minRange,
            _info.maxRange
        );
        seam->setArchive(_archive);
        seam->addChild(loRes->getChild(0));                // low res
        if (hiRes) 
        {
            seam->addChild(hiRes->getChild(0));    // high res
            seam->setHiResPresent(true);
        }
        if (nonSeamChildren.size())
        {
            seam->setNonSeamChildrenIndex(seam->getNumChildren());
            for (unsigned int i = 0; i < nonSeamChildren.size(); i++)
                seam->addChild(nonSeamChildren[i]);
        }
        return seam;
    }

    return node;
}

bool TXPTileNode::loadTile(int x, int y, int lod)
{
    if (_archive == 0) return false;

    TXPArchive::TileInfo info;

    bool validTile = _archive->getTileInfo(x,y,lod,info);
    int numLods = _archive->getNumLODs();

    if (validTile)
    {
        double realMinRange = info.minRange;
        double realMaxRange = info.maxRange;
        double usedMaxRange = osg::maximum(info.maxRange,1e7);
        osg::Vec3 tileCenter;
        osg::Group* tileGroup = _archive->getTileContent(x,y,lod,realMinRange,realMaxRange,usedMaxRange,tileCenter);

        // if group has only one child, then simply use its child.    
#if 1
        while (tileGroup->getNumChildren()==1 && tileGroup->getChild(0)->asGroup())
        {
            tileGroup = tileGroup->getChild(0)->asGroup();
        }
#endif
        
        // Handle seams
        if (lod < (numLods-1))
        {
            SeamFinder sfv(x,y,lod,info,_archive);
            tileGroup->accept(sfv);
        }
        
        if (lod < (numLods-1))
        {
            char pagedLODfile[1024];
            sprintf(pagedLODfile,"%s\\subtiles%d_%dx%d_%d.txp",
                _archive->getDir(),
                lod,
                x,
                y,
                _archive->getId()
            );

            osg::ref_ptr<TXPPagedLOD> pagedLOD = new TXPPagedLOD;
            // not use maximum(info.maxRange,1e7) as just maxRange would result in some corner tiles from being culled out.
            pagedLOD->addChild(tileGroup,info.minRange,osg::maximum(info.maxRange,1e7));
            pagedLOD->setFileName(1,pagedLODfile);
            pagedLOD->setRange(1,0,info.minRange);
            pagedLOD->setCenter(info.center);
            //pagedLOD->setCenter(tileCenter);
            pagedLOD->setRadius(info.radius);
            pagedLOD->setPriorityOffset(0,numLods-lod);
            pagedLOD->setPriorityScale(0,1.0f);
            pagedLOD->setNumChildrenThatCannotBeExpired(1);
            pagedLOD->setTileId(x,y,lod);

            int sizeX, sizeY;
            if (_archive->getLODSize(lod,sizeX,sizeY))
            {
                if ((x-1) > -1) pagedLOD->addNeighbour(x-1,y);
                if ((x+1) < sizeX) pagedLOD->addNeighbour(x+1,y);
                if ((y-1) > -1) pagedLOD->addNeighbour(x,y-1);
                if ((y+1) < sizeY) pagedLOD->addNeighbour(x,y+1);
            }

            TileMapper::instance()->insertPagedLOD(x,y,lod,pagedLOD.get());

            addChild(pagedLOD.get());

            //PrintVisitor pv;
            //accept(pv);
        }
        else
        {
            addChild(tileGroup);
        }

        return true;
    }

    return false;
}


