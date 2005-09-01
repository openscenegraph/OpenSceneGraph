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

OverlayNode::OverlayNode():
    _textureUnit(1)
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
    unsigned int tex_width = 1024;
    unsigned int tex_height = 1024;
    
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setTextureSize(tex_width, tex_height);
    texture->setInternalFormat(GL_RGBA);
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
    texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
    texture->setBorderColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
   
    _texture = texture;

    // set up the render to texture camera.
    {

        // create the camera
        _camera = new osg::CameraNode;

        _camera->setClearColor(osg::Vec4(1.0f,0.5f,0.5f,1.0f));

        // set viewport
        _camera->setViewport(0,0,tex_width,tex_height);

        // set the camera to render before the main camera.
        _camera->setRenderOrder(osg::CameraNode::PRE_RENDER);

        // tell the camera to use OpenGL frame buffer object where supported.
        _camera->setRenderTargetImplmentation(osg::CameraNode::FRAME_BUFFER_OBJECT);

        // attach the texture and use it as the color buffer.
        _camera->attach(osg::CameraNode::COLOR_BUFFER, _texture.get());
    }

    _texgenNode = new osg::TexGenNode;

    _mainSubgraphStateSet = new osg::StateSet;

    setOverlayTextureUnit(1);
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
        osg::Vec3 _position(0,0,0);

        // now compute the camera's view and projection matrix to point at the shadower (the camera's children)
        osg::BoundingSphere bs;
        for(unsigned int i=0; i<_camera->getNumChildren(); ++i)
        {
            bs.expandBy(_camera->getChild(i)->getBound());
        }

        if (!bs.valid())
        {
            osg::notify(osg::WARN) << "bb invalid"<<_camera.get()<<std::endl;
            return;
        }

        float centerDistance = (_position-bs.center()).length();

        float znear = centerDistance-bs.radius();
        float zfar  = centerDistance+bs.radius();
        float zNearRatio = 0.001f;
        if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;

        float top   = (bs.radius()/centerDistance)*znear;
        float right = top;

        _camera->setReferenceFrame(osg::CameraNode::ABSOLUTE_RF);
        _camera->setProjectionMatrixAsFrustum(-right,right,-top,top,znear,zfar);
        _camera->setViewMatrixAsLookAt(_position,bs.center(),osg::Vec3(0.0f,1.0f,0.0f));

        // compute the matrix which takes a vertex from local coords into tex coords
        // will use this later to specify osg::TexGen..
        osg::Matrix MVPT = _camera->getViewMatrix() * 
                           _camera->getProjectionMatrix() *
                           osg::Matrix::translate(1.0,1.0,1.0) *
                           osg::Matrix::scale(0.5f,0.5f,0.5f);

        _texgenNode->getTexGen()->setMode(osg::TexGen::EYE_LINEAR);
        _texgenNode->getTexGen()->setPlanesFromMatrix(MVPT);


        _camera->accept(*cv);
        
//        _textureObjectValidList[contextID] = 1;
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
    if (_overlaySubgraph == node) return;

    _overlaySubgraph = node;

    _camera->removeChild(0, _camera->getNumChildren());
    _camera->addChild(node);

    dirtyOverlayTexture();
}

void OverlayNode::dirtyOverlayTexture()
{
    _textureObjectValidList.setAllElementsTo(0);
}

void OverlayNode::setOverlayTextureUnit(unsigned int unit)
{
    _textureUnit = unit;

    _texgenNode->setTextureUnit(_textureUnit);
    
    _mainSubgraphStateSet->clear();
    _mainSubgraphStateSet->setTextureAttributeAndModes(_textureUnit, _texture.get(), osg::StateAttribute::ON);
    _mainSubgraphStateSet->setTextureMode(_textureUnit, GL_TEXTURE_GEN_S, osg::StateAttribute::ON);
    _mainSubgraphStateSet->setTextureMode(_textureUnit, GL_TEXTURE_GEN_T, osg::StateAttribute::ON);
    _mainSubgraphStateSet->setTextureMode(_textureUnit, GL_TEXTURE_GEN_R, osg::StateAttribute::ON);
    _mainSubgraphStateSet->setTextureMode(_textureUnit, GL_TEXTURE_GEN_Q, osg::StateAttribute::ON);
}
