#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <osgUtil/InsertImpostorsVisitor>

#include <algorithm>

using namespace osg;
using namespace osgUtil;

InsertImpostorsVisitor::InsertImpostorsVisitor()
{
    setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
    _impostorThresholdRatio = 10.0f;
    _maximumNumNestedImpostors = 3;
    _numNestedImpostors = 0;
}

void InsertImpostorsVisitor::reset()
{
    _groupList.clear();
    _lodList.clear();
    _numNestedImpostors = 0;
}

void InsertImpostorsVisitor::apply(Node& node)
{
    traverse(node);
}

void InsertImpostorsVisitor::apply(Group& node)
{
    _groupList.push_back(&node);
    
    ++_numNestedImpostors;
    if (_numNestedImpostors<_maximumNumNestedImpostors)
    {
        traverse(node);
    }
    --_numNestedImpostors;
}

void InsertImpostorsVisitor::apply(LOD& node)
{
    _lodList.push_back(&node);

    ++_numNestedImpostors;
    if (_numNestedImpostors<_maximumNumNestedImpostors)
    {
        traverse(node);
    }
    --_numNestedImpostors;
}

void InsertImpostorsVisitor::apply(Impostor& node)
{
    ++_numNestedImpostors;
    if (_numNestedImpostors<_maximumNumNestedImpostors)
    {
        traverse(node);
    }
    --_numNestedImpostors;
}

/* insert the required impostors into the scene graph.*/
void InsertImpostorsVisitor::insertImpostors()
{

    bool _insertImpostorsAboveGroups = true;
    bool _replaceLODsByImpostors = true;

    // handle group's
    if (_insertImpostorsAboveGroups)
    {
        std::sort(_groupList.begin(),_groupList.end());

        Group* previousGroup = NULL;
        for(GroupList::iterator itr=_groupList.begin();
            itr!=_groupList.end();
            ++itr)
        {
            Group* group = (*itr);
            if (group!=previousGroup)
            {
                const BoundingSphere& bs = group->getBound();
                if (bs.isValid())
                {

                    // take a copy of the original parent list
                    // before we change it around by adding the group
                    // to an impostor.
                    Node::ParentList parentList = group->getParents();

                    Impostor* impostor = osgNew Impostor;

                    // standard LOD settings
                    impostor->addChild(group);
                    impostor->setRange(0,0.0f);
                    impostor->setRange(1,1e7f);
                    impostor->setCenter(bs.center());
                    
                    // impostor specfic settings.
                    impostor->setImpostorThresholdToBound(_impostorThresholdRatio);

                    // now replace the group by the new impostor in all of the
                    // group's original parent list.
                    for(Node::ParentList::iterator pitr=parentList.begin();
                        pitr!=parentList.end();
                        ++pitr)
                    {
                        (*pitr)->replaceChild(group,impostor);
                    }

                }
            }
        }
    
    }    
    

    // handle LOD's
    if (_replaceLODsByImpostors)
    {
        std::sort(_lodList.begin(),_lodList.end());

        LOD* previousLOD = NULL;
        for(LODList::iterator itr=_lodList.begin();
            itr!=_lodList.end();
            ++itr)
        {
            osg::LOD* lod = (*itr);
            if (lod!=previousLOD)
            {
                const osg::BoundingSphere& bs = lod->getBound();
                if (bs.isValid())
                {

                    // take a copy of the original parent list
                    // before we change it around by adding the lod
                    // to an impostor.
                    Node::ParentList parentList = lod->getParents();

                    osg::Impostor* impostor = osgNew Impostor;

                    // standard LOD settings
                    for(int ci=0;ci<lod->getNumChildren();++ci)
                    {
                        impostor->addChild(lod->getChild(ci));
                    }
                    
                    for(int ri=0;ri<lod->getNumRanges();++ri)
                    {
                        impostor->setRange(ri,lod->getRange(ri));
                    }
                    
                    impostor->setCenter(lod->getCenter());

                    // impostor specfic settings.
                    impostor->setImpostorThresholdToBound(_impostorThresholdRatio);

                    // now replace the lod by the new impostor in all of the
                    // lod's original parent list.
                    for(Node::ParentList::iterator pitr=parentList.begin();
                        pitr!=parentList.end();
                        ++pitr)
                    {
                        (*pitr)->replaceChild(lod,impostor);
                    }

                }
            }
        }
    
    }    
}
