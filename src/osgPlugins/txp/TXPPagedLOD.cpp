#include "TileMapper.h"
#include "TXPPagedLOD.h"
using namespace txp;

TXPPagedLOD::TXPPagedLOD():
PagedLOD(),
_tileX(-1),
_tileY(-1),
_tileLOD(-1),
_lastChildTraversed(-1)
{
}

TXPPagedLOD::TXPPagedLOD(const TXPPagedLOD& plod,const osg::CopyOp& copyop):
PagedLOD(plod,copyop),
_tileX(plod._tileX),
_tileY(plod._tileY),
_tileLOD(plod._tileLOD),
_lastChildTraversed(plod._lastChildTraversed)
{
}

TXPPagedLOD::~TXPPagedLOD()
{
    TileMapper::instance()->removePagedLOD(_tileX,_tileY,_tileLOD);
}

void TXPPagedLOD::traverse(osg::NodeVisitor& nv)
{

    double timeStamp = nv.getFrameStamp()?nv.getFrameStamp()->getReferenceTime():0.0;
    bool updateTimeStamp = nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR;

    switch(nv.getTraversalMode())
    {
    case(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN):
            std::for_each(_children.begin(),_children.end(),osg::NodeAcceptOp(nv));
            break;
    case(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN):
        {
            float distance = nv.getDistanceToEyePoint(getCenter(),true);

            _lastChildTraversed = -1;
            bool needToLoadChild = false;
            bool rollBack = false;
            for(unsigned int i=0;i<_rangeList.size();++i)
            {    
                if (_rangeList[i].first<=distance && distance<_rangeList[i].second)
                {
                    if (i<_children.size())
                    {
                        bool acceptThisChild = true;

                        if (i)
                        {
                            for (unsigned int ni = 0; ni < _neighbours.size(); ni++)
                            {
                                Neighbour& n = _neighbours[ni];
                                if (TileMapper::instance()->getPagedLOD(n._x, n._y, _tileLOD)== 0)
                                {
                                    rollBack = true;
                                    acceptThisChild = false;
                                    break;
                                }
                            }
                        }

                        if (acceptThisChild)
                        {
                            if (updateTimeStamp) _perRangeDataList[i]._timeStamp=timeStamp;
                            _children[i]->accept(nv);
                            _lastChildTraversed = (int)i;
                        }
                    }
                    else
                    {
                        needToLoadChild = true;
                    }
                }
            }

            if (rollBack)
            {
                if (updateTimeStamp) _perRangeDataList[0]._timeStamp=timeStamp;
                _children[0]->accept(nv);
                _lastChildTraversed = 0;
                //std::cout << "Rolling back" << std::endl;
            }
            
            if (needToLoadChild)
            {
                unsigned int numChildren = _children.size();
                
                //std::cout<<"PagedLOD::traverse() - falling back "<<std::endl;
                // select the last valid child.
                if (numChildren>0 && ((int)numChildren-1)!=_lastChildTraversed)
                {
                    //std::cout<<"    to child "<<numChildren-1<<std::endl;
                    if (updateTimeStamp) _perRangeDataList[numChildren-1]._timeStamp=timeStamp;
                    _children[numChildren-1]->accept(nv);
					_lastChildTraversed = numChildren-1;
                }
                
                // now request the loading of the next unload child.
                if (nv.getDatabaseRequestHandler() && numChildren<_perRangeDataList.size())
                {
                    // compute priority from where abouts in the required range the distance falls.
                    float priority = (_rangeList[numChildren].second-distance)/(_rangeList[numChildren].second-_rangeList[numChildren].first);
                    
                    // modify the priority according to the child's priority offset and scale.
                    priority = _perRangeDataList[numChildren]._priorityOffset + priority * _perRangeDataList[numChildren]._priorityScale;

                    //std::cout<<"    requesting child "<<_fileNameList[numChildren]<<" priotity = "<<priority<<std::endl;
                    nv.getDatabaseRequestHandler()->requestNodeFile(_perRangeDataList[numChildren]._filename,this,priority,nv.getFrameStamp());
                }
                
                
            }
            
            
           break;
        }
        default:
            break;
    }
}
