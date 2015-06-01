#include "TileMapper.h"
#include "TXPPagedLOD.h"

#include <algorithm>

using namespace txp;

TXPPagedLOD::TXPPagedLOD():
    osg::PagedLOD()
{
}

TXPPagedLOD::TXPPagedLOD(const TXPPagedLOD& plod,const osg::CopyOp& copyop):
    osg::PagedLOD(plod,copyop),
    _tileIdentifier(plod._tileIdentifier)
{
}

TXPPagedLOD::~TXPPagedLOD()
{
}

void TXPPagedLOD::traverse(osg::NodeVisitor& nv)
{

    //TileMapper* tileMapper = dynamic_cast<TileMapper*>(nv.getUserData());
    //Modified by Brad Anderegg (May-27-08) because the black listing process appears to make tiles switch lods
    //when they clearly shouldn't, in the worst cases a tile will page out that is right in front of you.
    bool forceUseOfFirstChild = /*tileMapper ? (tileMapper->isNodeBlackListed(this)) :*/ false;

    double timeStamp = nv.getFrameStamp()?nv.getFrameStamp()->getReferenceTime():0.0;
    bool updateTimeStamp = nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR;
    unsigned int frameNumber = nv.getFrameStamp()?nv.getFrameStamp()->getFrameNumber():0;

    // set the frame number of the traversal so that external nodes can find out how active this
    // node is.
    if (nv.getFrameStamp() &&
        nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR)
    {
        setFrameNumberOfLastTraversal(nv.getFrameStamp()->getFrameNumber());
    }

    switch(nv.getTraversalMode())
    {
        case(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN):
            std::for_each(_children.begin(),_children.end(),osg::NodeAcceptOp(nv));
            break;
        case(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN):
        {
            float distance = nv.getDistanceToEyePoint(getCenter(),true);

            int lastChildTraversed = -1;
            bool needToLoadChild = false;

            unsigned maxRangeSize = _rangeList.size();
            if (maxRangeSize!=0 && forceUseOfFirstChild)
                maxRangeSize=1;

            for(unsigned int i=0;i<maxRangeSize;++i)
            {
                if (forceUseOfFirstChild ||
                    (_rangeList[i].first<=distance && distance<_rangeList[i].second))
                {
                    if (i<_children.size())
                    {
                        if (updateTimeStamp)
                        {
                            _perRangeDataList[i]._timeStamp=timeStamp;
                            _perRangeDataList[i]._frameNumber=frameNumber;
                        }

                        _children[i]->accept(nv);
                        lastChildTraversed = (int)i;
                    }
                    else
                    {
                        needToLoadChild = true;
                    }
                }
            }

            if (needToLoadChild)
            {
                unsigned int numChildren = _children.size();

                //std::cout<<"PagedLOD::traverse() - falling back "<<std::endl;
                // select the last valid child.
                if (numChildren>0 && ((int)numChildren-1)!=lastChildTraversed)
                {
                    //std::cout<<"    to child "<<numChildren-1<<std::endl;
                    if (updateTimeStamp)
                        _perRangeDataList[numChildren-1]._timeStamp=timeStamp;

                    _children[numChildren-1]->accept(nv);
                }

                // now request the loading of the next unload child.
                if (nv.getDatabaseRequestHandler() && numChildren<_perRangeDataList.size())
                {
                    // compute priority from where abouts in the required range the distance falls.
                    float priority = (_rangeList[numChildren].second-distance)/(_rangeList[numChildren].second-_rangeList[numChildren].first);

                    // modify the priority according to the child's priority offset and scale.
                    priority = _perRangeDataList[numChildren]._priorityOffset + priority * _perRangeDataList[numChildren]._priorityScale;

                    //std::cout<<"    requesting child "<<_fileNameList[numChildren]<<" priotity = "<<priority<<std::endl;
                    nv.getDatabaseRequestHandler()->requestNodeFile(_perRangeDataList[numChildren]._filename,
                                                                    nv.getNodePath(),
                                                                    priority,
                                                                    nv.getFrameStamp(),
                                                                    _perRangeDataList[numChildren]._databaseRequest);
                }


            }


           break;
        }
        default:
            break;
    }
}

osg::BoundingSphere TXPPagedLOD::computeBound() const
{
    // Force calculation of entire bounding sphere; this will include any
    // externally-referenced models which are attached to the tile.
    // If this is not done, then externally referenced models will disappear
    // when the tile they are attached to leaves the view volume.
    osg::BoundingSphere result = osg::Group::computeBound();

    if (_centerMode==USER_DEFINED_CENTER && _radius>=0.0f)
    {
        float tempRadius = osg::maximum( _radius, result.radius() );
        result = osg::BoundingSphere(_userDefinedCenter,tempRadius);
    }
    return result;
}
