/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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

#include <osg/ProxyNode>
#include <osg/CullStack>
#include <osg/Notify>

using namespace osg;

ProxyNode::ProxyNode() : 
	_centerMode(USER_DEFINED_CENTER),
	_radius(-1)
{
} 

ProxyNode::ProxyNode(const ProxyNode& proxynode,const CopyOp& copyop):
    Group(proxynode,copyop),
    _filenameList(proxynode._filenameList),
    _centerMode(proxynode._centerMode),
    _userDefinedCenter(proxynode._userDefinedCenter),
    _radius(proxynode._radius)
{
}

void ProxyNode::setDatabasePath(const std::string& path)
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
    }
}

void ProxyNode::traverse(NodeVisitor& nv)
{
    if (_filenameList.size()>_children.size() && nv.getVisitorType()==NodeVisitor::CULL_VISITOR)
    {
        for(unsigned int i=_children.size()-1; i<_filenameList.size(); ++i)
		{
            nv.getDatabaseRequestHandler()->requestNodeFile(_databasePath+_filenameList[i], this, 1.0f, nv.getFrameStamp());
		}
	}
    else
    {
        Group::traverse(nv);
    }
}

void ProxyNode::expandFileNameListTo(unsigned int pos)
{
    if (pos>=_filenameList.size()) _filenameList.resize(pos+1);
}

bool ProxyNode::addChild( Node *child )
{
    if (Group::addChild(child))
    {
        expandFileNameListTo(_children.size());
        return true;
    }
    return false;
}

bool ProxyNode::addChild(Node *child, const std::string& filename)
{
    if (Group::addChild(child))
    {
        setFileName(_children.size()-1,filename);
        return true;
    }
    return false;
}

bool ProxyNode::removeChild( Node *child )
{
    // find the child's position.
    unsigned int pos=getChildIndex(child);
    if (pos==_children.size()) return false;
    
    if (pos<_filenameList.size()) _filenameList.erase(_filenameList.begin()+pos);
    
    return Group::removeChild(child);    
}

bool ProxyNode::computeBound() const
{
    if (_centerMode==USER_DEFINED_CENTER && _radius>=0.0f)
    {
        _bsphere._center = _userDefinedCenter;
        _bsphere._radius = _radius;
        _bsphere_computed = true;

        return true;
    }
    else
    {
        return Group::computeBound();
    }
}


