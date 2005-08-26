/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield 
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

#include <osg/Texture2D>
#include <osgUtil/CullVisitor>
#include <osgSim/OverlayNode>

using namespace osgSim;

// use this cull callback to allow the camera to traverse the OverlaySubgraph's children without
// actuall having them assigned as children to the camea itself.  This make the camera a
// decorator without ever directly being assigned to it. 
class OverlayTraverseNodeCallback : public osg::NodeCallback
{
public:

    OverlayTraverseNodeCallback(osg::Node* node):_node(node) {}                                                       

    virtual void operator()(osg::Node*, osg::NodeVisitor* nv)
    {
        _node->accept(*nv);
    }
    
    osg::Node* _node;
};

OverlayNode::OverlayNode():
    _textureUnit(0)
{
    init();
}

OverlayNode::OverlayNode(const OverlayNode& copy, const osg::CopyOp& copyop):
    Group(copy,copyop),
    _overlaySubgraph(copy._overlaySubgraph),
    _textureUnit(copy._textureUnit)
{
    init();
}

void OverlayNode::init()
{
    _camera = new osg::CameraNode;

    _texgenNode = new osg::TexGenNode;
    _texgenNode->setTextureUnit(_textureUnit);

    _texture = new osg::Texture2D;

    _mainSubgraphStateSet = new osg::StateSet;
    _mainSubgraphStateSet->setTextureAttributeAndModes(_textureUnit, _texture.get(), osg::StateAttribute::ON);
}


void OverlayNode::traverse(osg::NodeVisitor& nv)
{
    if (nv.getVisitorType() != osg::NodeVisitor::CULL_VISITOR)
    {
        Group::traverse(nv);
        return;
    }

    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
    if (!cv)
    {
        Group::traverse(nv);
        return;
    }
    
    unsigned int contextID = cv->getState()!=0 ? cv->getState()->getContextID() : 0;

    // if we need to redraw then do cull traversal on camera.
    if (!_textureObjectValidList[contextID])
    {
        _camera->accept(*cv);
        
        _textureObjectValidList[contextID] = 1;
    }
    
    
    // now set up the drawing of the main scene.
    {
        _texgenNode->accept(*cv);
    
        // push the stateset.
        cv->pushStateSet(_mainSubgraphStateSet.get());

        Group::traverse(nv);

        cv->popStateSet();
    }
}

void OverlayNode::setOverlaySubgraph(osg::Node* node)
{
    _overlaySubgraph = node;
    dirtyOverlayTexture();
}

void OverlayNode::dirtyOverlayTexture()
{
    _textureObjectValidList.setAllElementsTo(0);
}

void OverlayNode::setOverlayTextureUnit(unsigned int unit)
{
    if (_textureUnit==unit) return;
    
    _texgenNode->setTextureUnit(unit);
    
    _mainSubgraphStateSet->clear();
    _mainSubgraphStateSet->setTextureAttributeAndModes(_textureUnit, _texture.get(), osg::StateAttribute::ON);
}
