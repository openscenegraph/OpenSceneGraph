#include <osg/ClipNode>

using namespace osg;

ClipNode::ClipNode()
{
    _value = StateAttribute::ON;
    _dstate = osgNew StateSet;
}

ClipNode::ClipNode(const ClipNode& cn, const CopyOp& copyop):Group(cn,copyop)
{
    for(ClipPlaneList::const_iterator itr=cn._planes.begin();
        itr!=cn._planes.end();
        ++itr)
    {
        ClipPlane* plane = dynamic_cast<ClipPlane*>(copyop(itr->get()));
        if (plane) addClipPlane(plane);
    }
}

ClipNode::~ClipNode()
{
}

// Create a 6 clip planes to create a clip box.
void ClipNode::createClipBox(const BoundingBox& bb,unsigned int clipPlaneNumberBase)
{
    _planes.clear();

    _planes.push_back(new ClipPlane(clipPlaneNumberBase  ,1.0,0.0,0.0,-bb.xMin()));
    _planes.push_back(new ClipPlane(clipPlaneNumberBase+1,-1.0,0.0,0.0,bb.xMax()));

    _planes.push_back(new ClipPlane(clipPlaneNumberBase+2,0.0,1.0,0.0,-bb.yMin()));
    _planes.push_back(new ClipPlane(clipPlaneNumberBase+3,0.0,-1.0,0.0,bb.yMax()));

    _planes.push_back(new ClipPlane(clipPlaneNumberBase+4,0.0,0.0,1.0,-bb.zMin()));
    _planes.push_back(new ClipPlane(clipPlaneNumberBase+5,0.0,0.0,-1.0,bb.zMax()));

    setLocalStateSetModes(_value);
}

// Add a ClipPlane to a ClipNode. Return true if plane is added, 
// return false if plane already exists in ClipNode, or clipplane is false.
const bool ClipNode::addClipPlane(ClipPlane* clipplane)
{
    if (!clipplane) return false;

    if (std::find(_planes.begin(),_planes.end(),clipplane)==_planes.end())
    {
        // cliplane doesn't exist in list so add it.
        _planes.push_back(clipplane);
        setLocalStateSetModes(_value);
        return true;
    }
    else
    {
        return false;
    }
}

// Remove ClipPlane from a ClipNode. Return true if plane is removed, 
// return false if plane does not exists in ClipNode.
const bool ClipNode::removeClipPlane(ClipPlane* clipplane)
{
    if (!clipplane) return false;

    ClipPlaneList::iterator itr = std::find(_planes.begin(),_planes.end(),clipplane);
    if (itr!=_planes.end())
    {
        // cliplane exist in list so erase it.
        _planes.erase(itr);
        setLocalStateSetModes(_value);
        return true;
    }
    else
    {
        return false;
    }
}

// Remove ClipPlane, at specified index, from a ClipNode. Return true if plane is removed, 
// return false if plane does not exists in ClipNode.
const bool ClipNode::removeClipPlane(unsigned int pos)
{
    if (pos<_planes.size())
    {
        _planes.erase(_planes.begin()+pos);
        setLocalStateSetModes(_value);
        return true;
    }
    else
    {
        return false;
    }
}

// Set the GLModes on StateSet associated with the ClipPlanes.
void ClipNode::setStateSetModes(StateSet& stateset,const StateAttribute::GLModeValue value) const
{
    for(ClipPlaneList::const_iterator itr=_planes.begin();
        itr!=_planes.end();
        ++itr)
    {
        (*itr)->setStateSetModes(stateset,value);
    }
}

void ClipNode::setLocalStateSetModes(const StateAttribute::GLModeValue value)
{
    if (!_dstate) _dstate = osgNew StateSet;
    _dstate->setAllToInherit();
    setStateSetModes(*_dstate,value);
}

const bool ClipNode::computeBound() const
{
    return Group::computeBound();
}
