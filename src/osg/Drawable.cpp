#include <stdio.h>
#include <math.h>
#include <float.h>

#include <osg/Drawable>
#include <osg/State>
#include <osg/Notify>
#include <osg/Node>

#include <algorithm>

using namespace osg;

Drawable::DeletedDisplayListCache Drawable::s_deletedDisplayListCache;

Drawable::Drawable()
{
    _globjList.resize(DisplaySettings::instance()->getMaxNumberOfGraphicsContexts(),0);

    _bbox_computed = false;

    // Note, if your are defining a subclass from drawable which is
    // dynamically updated then you should set both the following to
    // to false in your constructor.  This will prevent any display
    // lists from being automatically created and safeguard the
    // dynamic updating of data.
    _supportsDisplayList = true;
    _useDisplayList = true;

}

Drawable::Drawable(const Drawable& drawable,const CopyOp& copyop):
    Object(drawable,copyop),
    _parents(), // leave empty as parentList is managed by Geode
    _dstate(copyop(drawable._dstate.get())),
    _supportsDisplayList(drawable._supportsDisplayList),
    _useDisplayList(drawable._useDisplayList),
    _globjList(drawable._globjList),
    _bbox(drawable._bbox),
    _bbox_computed(drawable._bbox_computed),
    _drawCallback(drawable._drawCallback),
    _cullCallback(drawable._cullCallback)
{}

Drawable::~Drawable()
{
    dirtyDisplayList();
}

void Drawable::addParent(osg::Node* node)
{
    _parents.push_back(node);    
}

void Drawable::removeParent(osg::Node* node)
{
    ParentList::iterator pitr = std::find(_parents.begin(),_parents.end(),node);
    if (pitr!=_parents.end()) _parents.erase(pitr);
}

void Drawable::dirtyBound()
{
    if (_bbox_computed)
    {
        _bbox_computed = false;

        // dirty parent bounding sphere's to ensure that all are valid.
        for(ParentList::iterator itr=_parents.begin();
            itr!=_parents.end();
            ++itr)
        {
            (*itr)->dirtyBound();
        }

    }
}

void Drawable::compile(State& state)
{
    if (!_useDisplayList) return;

    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    uint contextID = state.getContextID();

    // fill in array if required.
    while (_globjList.size()<=contextID) _globjList.push_back(0);

    // get the globj for the current contextID.
    uint& globj = _globjList[contextID];

    // call the globj if already set otherwise comple and execute.
    if( globj != 0 )
    {
        glDeleteLists( globj, 1 );
    }

    
    if (_dstate.valid())
    {
        _dstate->compile(state);
    }

    globj = glGenLists( 1 );
    glNewList( globj, GL_COMPILE );

    if (_drawCallback.valid())
        _drawCallback->drawImmediateMode(state,this);
    else 
        drawImmediateMode(state);

    glEndList();

}

void Drawable::setSupportsDisplayList(const bool flag)
{
    // if value unchanged simply return.
    if (_supportsDisplayList==flag) return;
    
    // if previously set to true then need to check about display lists.
    if (_supportsDisplayList)
    {
        if (_useDisplayList)
        {
            // used to support display lists and display lists switched
            // on so now delete them and turn useDisplayList off.
            dirtyDisplayList();
            _useDisplayList = false;
        }
    }
    
    // set with new value.
    _supportsDisplayList=flag;
}

void Drawable::setUseDisplayList(const bool flag)
{
    // if value unchanged simply return.
    if (_useDisplayList==flag) return;

    // if was previously set to true, remove display list.
    if (_useDisplayList)
    {
        dirtyDisplayList();
    }
    
    if (_supportsDisplayList)
    {
    
        // set with new value.
        _useDisplayList = flag;
        
    }
    else // does not support display lists.
    {
        if (flag)
        {
            notify(WARN)<<"Warning: attempt to setUseDisplayList(true) on a drawable with does not support display lists."<<std::endl;
        }
        else 
        {
            // set with new value.
            _useDisplayList = false;
        }
    }
}


void Drawable::dirtyDisplayList()
{
    for(uint i=0;i<_globjList.size();++i)
    {
        if (_globjList[i] != 0)
        {
            Drawable::deleteDisplayList(i,_globjList[i]);
            _globjList[i] = 0;
        }
    }
}

void Drawable::deleteDisplayList(uint contextID,uint globj)
{
    if (globj!=0)
    {
        // insert the globj into the cache for the appropriate context.
        s_deletedDisplayListCache[contextID].insert(globj);
    }
}

/** flush all the cached display list which need to be deleted
  * in the OpenGL context related to contextID.*/
void Drawable::flushDeletedDisplayLists(uint contextID)
{
    DeletedDisplayListCache::iterator citr = s_deletedDisplayListCache.find(contextID);
    if (citr!=s_deletedDisplayListCache.end())
    {
        std::set<uint>& displayListSet = citr->second;
        for(std::set<uint>::iterator gitr=displayListSet.begin();
                                     gitr!=displayListSet.end();
                                     ++gitr)
        {
            glDeleteLists(*gitr,1);
        }

        s_deletedDisplayListCache.erase(citr);
    }
}

void Drawable::setAppCallback(AppCallback* ac)
{
    if (_appCallback==ac) return;
    
    int delta = 0;
    if (_appCallback.valid()) --delta;
    if (ac) ++delta;
    
    if (delta!=0)
    {
        for(ParentList::iterator itr=_parents.begin();
            itr!=_parents.end();
            ++itr)
        {
            (*itr)->setNumChildrenRequiringAppTraversal((*itr)->getNumChildrenRequiringAppTraversal()+delta);
        }
    }
}
