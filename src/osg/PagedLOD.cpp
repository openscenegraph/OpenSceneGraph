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

#include <osg/PagedLOD>
#include <osg/CullStack>
#include <osg/Notify>

#include <algorithm>

using namespace osg;

PagedLOD::PerRangeData::PerRangeData():
    _priorityOffset(0.0f),
    _priorityScale(1.0f),
    _timeStamp(0.0f) {}

PagedLOD::PerRangeData::PerRangeData(const PerRangeData& prd):
    _filename(prd._filename),
    _priorityOffset(prd._priorityOffset),
    _priorityScale(prd._priorityScale),
    _timeStamp(prd._timeStamp) {}

PagedLOD::PerRangeData& PagedLOD::PerRangeData::operator = (const PerRangeData& prd)
{
    if (this==&prd) return *this;
    _filename = prd._filename;
    _priorityOffset = prd._priorityOffset;
    _priorityScale = prd._priorityScale;
    _timeStamp = prd._timeStamp;
    return *this;
}

PagedLOD::PagedLOD()
{
    _frameNumberOfLastTraversal = 0;
    _centerMode = USER_DEFINED_CENTER;
    _radius = -1;
    _numChildrenThatCannotBeExpired = 0;
}

PagedLOD::PagedLOD(const PagedLOD& plod,const CopyOp& copyop):
    LOD(plod,copyop),
    _frameNumberOfLastTraversal(plod._frameNumberOfLastTraversal),
    _numChildrenThatCannotBeExpired(plod._numChildrenThatCannotBeExpired),
    _perRangeDataList(plod._perRangeDataList)
{
}

void PagedLOD::setDatabasePath(const std::string& path)
{
    _databasePath = path;
    if (!_databasePath.empty())
    {
        char& lastCharacter = _databasePath[_databasePath.size()-1];
        const char unixSlash = '/';
        const char winSlash = '\\';

        if (lastCharacter==winSlash)
        {
            lastCharacter = unixSlash;
        }
        else if (lastCharacter!=unixSlash)
        {
            _databasePath += unixSlash;
        }

/*        
        // make sure the last character is the appropriate slash 
#ifdef WIN32       
        if (lastCharacter==unixSlash)
        {
            lastCharacter = winSlash;
        }
        else if (lastCharacter!=winSlash)
        {
            _databasePath += winSlash;
        }
#else
        if (lastCharacter==winSlash)
        {
            lastCharacter = unixSlash;
        }
        else if (lastCharacter!=unixSlash)
        {
            _databasePath += unixSlash;
        }
#endif
*/
    }
}


void PagedLOD::traverse(NodeVisitor& nv)
{
    // set the frame number of the traversal so that external nodes can find out how active this
    // node is.
    if (nv.getFrameStamp()) setFrameNumberOfLastTraversal(nv.getFrameStamp()->getFrameNumber());

    double timeStamp = nv.getFrameStamp()?nv.getFrameStamp()->getReferenceTime():0.0;
    bool updateTimeStamp = nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR;

    switch(nv.getTraversalMode())
    {
        case(NodeVisitor::TRAVERSE_ALL_CHILDREN):
            std::for_each(_children.begin(),_children.end(),NodeAcceptOp(nv));
            break;
        case(NodeVisitor::TRAVERSE_ACTIVE_CHILDREN):
        {
            float required_range = 0;
            if (_rangeMode==DISTANCE_FROM_EYE_POINT)
            {
                required_range = nv.getDistanceToEyePoint(getCenter(),true);
            }
            else
            {
                osg::CullStack* cullStack = dynamic_cast<osg::CullStack*>(&nv);
                if (cullStack && cullStack->getLODScale()>0.0f)
                {
                    required_range = cullStack->clampedPixelSize(getBound()) / cullStack->getLODScale();
                }
                else
                {
                    // fallback to selecting the highest res tile by
                    // finding out the max range
                    for(unsigned int i=0;i<_rangeList.size();++i)
                    {
                        required_range = osg::maximum(required_range,_rangeList[i].first);
                    }
                }
            }
 
            int lastChildTraversed = -1;
            bool needToLoadChild = false;
            for(unsigned int i=0;i<_rangeList.size();++i)
            {    
                if (_rangeList[i].first<=required_range && required_range<_rangeList[i].second)
                {
                    if (i<_children.size())
                    {
                        if (updateTimeStamp) _perRangeDataList[i]._timeStamp=timeStamp;

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
                
                // select the last valid child.
                if (numChildren>0 && ((int)numChildren-1)!=lastChildTraversed)
                {
                    if (updateTimeStamp) _perRangeDataList[numChildren-1]._timeStamp=timeStamp;
                    _children[numChildren-1]->accept(nv);
                }
                
                // now request the loading of the next unloaded child.
                if (nv.getDatabaseRequestHandler() && numChildren<_perRangeDataList.size())
                {
                    // compute priority from where abouts in the required range the distance falls.
                    float priority = (_rangeList[numChildren].second-required_range)/(_rangeList[numChildren].second-_rangeList[numChildren].first);
                    
                    // invert priority for PIXEL_SIZE_ON_SCREEN mode
                    if(_rangeMode==PIXEL_SIZE_ON_SCREEN)
                    {
                        priority = -priority;
                    }
                    
                    // modify the priority according to the child's priority offset and scale.
                    priority = _perRangeDataList[numChildren]._priorityOffset + priority * _perRangeDataList[numChildren]._priorityScale;
                    
                    if (_databasePath.empty())
                    {
                        nv.getDatabaseRequestHandler()->requestNodeFile(_perRangeDataList[numChildren]._filename,this,priority,nv.getFrameStamp());
                    }
                    else
                    {
                        // prepend the databasePath to the child's filename.
                        nv.getDatabaseRequestHandler()->requestNodeFile(_databasePath+_perRangeDataList[numChildren]._filename,this,priority,nv.getFrameStamp());
                    }
                }
                
                
            }
            
            
           break;
        }
        default:
            break;
    }
}


void PagedLOD::expandPerRangeDataTo(unsigned int pos)
{
    if (pos>=_perRangeDataList.size()) _perRangeDataList.resize(pos+1);
}

bool PagedLOD::addChild( Node *child )
{
    if (LOD::addChild(child))
    {
        expandPerRangeDataTo(_children.size()-1);
        return true;
    }
    return false;
}

bool PagedLOD::addChild(Node *child, float min, float max)
{
    if (LOD::addChild(child,min,max))
    {
        expandPerRangeDataTo(_children.size()-1);
        return true;
    }
    return false;
}


bool PagedLOD::addChild(Node *child, float min, float max,const std::string& filename, float priorityOffset, float priorityScale)
{
    if (LOD::addChild(child,min,max))
    {
        setFileName(_children.size()-1,filename);
        setPriorityOffset(_children.size()-1,priorityOffset);
        setPriorityScale(_children.size()-1,priorityScale);
        return true;
    }
    return false;
}

bool PagedLOD::removeChildren( unsigned int pos,unsigned int numChildrenToRemove)
{
    if (pos<_rangeList.size()) _rangeList.erase(_rangeList.begin()+pos, osg::minimum(_rangeList.begin()+(pos+numChildrenToRemove), _rangeList.end()) );
    if (pos<_perRangeDataList.size()) _perRangeDataList.erase(_perRangeDataList.begin()+pos, osg::minimum(_perRangeDataList.begin()+ (pos+numChildrenToRemove), _perRangeDataList.end()) );
            
    return Group::removeChildren(pos,numChildrenToRemove);
}

bool PagedLOD::removeExpiredChildren(double expiryTime,NodeList& removedChildren)
{
    if (_children.size()>_numChildrenThatCannotBeExpired)
    {
        if (!_perRangeDataList[_children.size()-1]._filename.empty() && _perRangeDataList[_children.size()-1]._timeStamp<expiryTime)
        {
            osg::Node* nodeToRemove = _children[_children.size()-1].get();
            removedChildren.push_back(nodeToRemove);
            return Group::removeChildren(_children.size()-1,1);
        }
    }
    return false;
}
