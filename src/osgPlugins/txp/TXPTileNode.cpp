#include <osg/PagedLOD>

#include "TileMapper.h"
#include "TXPTileNode.h"
#include "TXPArchive.h"
#include "TXPSeamLOD.h"

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
            writeIndent(); std::cout << "center="<<lod.getCenter()<<std::endl;
            for(unsigned int i=0;i<lod.getNumChildren();++i)
            {
                writeIndent(); std::cout << "child min="<<lod.getMinRange(i)<<" max="<<lod.getMaxRange(i)<<std::endl;
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


osg::Node* TXPTileNode::seamReplacement(osg::Node* child,int x, int y, int level, TXPArchive::TileInfo& info)
{
    osg::Group* group = child->asGroup();
    if (group)
    {
        if (group->getNumChildren()==2)
        {
            osg::LOD* lod1 = dynamic_cast<osg::LOD*>(group->getChild(0));
            osg::LOD* lod2 = dynamic_cast<osg::LOD*>(group->getChild(1));
            if (lod1 && lod1->getNumChildren()==1 &&
                lod2 && lod2->getNumChildren()==1)
            {
            
                osg::Vec3 delta = lod1->getCenter()-info.center;
                if (fabs(delta.x())>fabs(delta.y()))
                {
                    if (delta.x()<0.0)
                    {
                        // west tile
                        --x;
                    }
                    else
                    {
                        // east tile
                        x++;
                    }
                }
                else
                {
                    if (delta.y()<0.0)
                    {
                        // south tile
                        --y;
                    }
                    else
                    {
                        // north tile
                        ++y;
                    }
                }
            
 //                std::cout<<"seamReplacement lod1 min="<<lod1->getMinRange(0)<<" max="<<lod1->getMaxRange(0)<<std::endl;
//                 std::cout<<"                lod2 min="<<lod2->getMinRange(0)<<" max="<<lod2->getMaxRange(0)<<std::endl;
                
                if (lod1->getMaxRange(0)<lod2->getMaxRange(0))
                {
//                     std::cout<<"   lod1 is high res, lod2 is low res."<<std::endl;
                }
                else if (lod1->getMaxRange(0)==lod2->getMaxRange(0))
                {
//                     std::cout<<"   lod1 and lod2 range equal. ****************"<<std::endl;
                    // don't replace with seam LOD node, leave as a standard LOD.
                    return 0;
                }
                else
                {
//                     std::cout<<"   lod1 is low res, lod2 is high res. --------------"<<std::endl;
                    // don't replace with seam LOD node, leave as a standard LOD
                    return 0;
                }
                
            
                TXPSeamLOD* seam = new TXPSeamLOD(x,y,level,lod1->getCenter(),lod1->getMinRange(0),lod1->getMaxRange(0),lod2->getMaxRange(0));
                seam->setArchive(_archive);
                seam->addChild(lod1->getChild(0)); // high res
                seam->addChild(lod2->getChild(0)); // low res
                return seam;
            }
        }
    }
    return child;
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
        osg::Group* tileGroup = _archive->getTileContent(x,y,lod,realMinRange,realMaxRange,usedMaxRange);


        // if group has only one child, then simply use its child.        
        while (tileGroup->getNumChildren()==1 && tileGroup->getChild(0)->asGroup())
        {
            tileGroup = tileGroup->getChild(0)->asGroup();
        }
        

        
        if (tileGroup->getNumChildren()==5)
        {
            // candidate for being a Tile with seams.
//             std::cout<<"------- Seams candidate ------ "<<std::endl;
//             std::cout<<"        info.center = "<<info.center<<std::endl;
//             PrintVisitor pv;
//             tileGroup->accept(pv);

            for(unsigned int i=1;i<tileGroup->getNumChildren();++i)
            {
                osg::Node* child = tileGroup->getChild(i);
                osg::Node* replacement = seamReplacement(child,x,y,lod,info);
                if (child!=replacement)
                {
                    tileGroup->replaceChild(child,replacement);
                }
            }
            
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

            osg::ref_ptr<osg::PagedLOD> pagedLOD = new osg::PagedLOD;
            // not use maximum(info.maxRange,1e7) as just maxRange would result in some corner tiles from being culled out.
            pagedLOD->addChild(tileGroup,info.minRange,osg::maximum(info.maxRange,1e7));
            pagedLOD->setFileName(1,pagedLODfile);
            pagedLOD->setRange(1,0,info.minRange);
            pagedLOD->setCenter(info.center);
            pagedLOD->setRadius(info.radius);
            pagedLOD->setNumChildrenThatCannotBeExpired(1);

            TileMapper::instance()->insertPagedLOD(x,y,lod,pagedLOD.get());

            addChild(pagedLOD.get());
        }
        else
        {
            addChild(tileGroup);
        }

        return true;
    }

    return false;
}


