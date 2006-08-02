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

#include <osg/Texture2D>
#include <osg/CoordinateSystemNode>
#include <osg/TexEnv>
#include <osg/io_utils>

#include <osgUtil/CullVisitor>
#include <osgSim/OverlayNode>

using namespace osgSim;
using namespace osg;

OverlayNode::OverlayNode():
    _texEnvMode(GL_DECAL),
    _textureUnit(1),
    _textureSizeHint(1024),
    _continuousUpdate(false)
{
    setNumChildrenRequiringUpdateTraversal(1);
    init();
}

OverlayNode::OverlayNode(const OverlayNode& copy, const osg::CopyOp& copyop):
    osg::Group(copy,copyop),
    _overlaySubgraph(copy._overlaySubgraph),
    _texEnvMode(copy._texEnvMode),
    _textureUnit(copy._textureUnit),
    _textureSizeHint(copy._textureSizeHint),
    _continuousUpdate(copy._continuousUpdate)
{
    setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);            
    init();
}

void OverlayNode::init()
{

    unsigned int tex_width = _textureSizeHint;
    unsigned int tex_height = _textureSizeHint;
    
    if (!_texture) 
    { 
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setTextureSize(tex_width, tex_height);
        texture->setInternalFormat(GL_RGBA);
        texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
        texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
        texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
        texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
        texture->setBorderColor(osg::Vec4(1.0f,1.0f,1.0f,0.0f));
        _texture = texture;
    }   

    // set up the render to texture camera.
    if (!_camera)
    {
        // create the camera
        _camera = new osg::CameraNode;
         
        _camera->setClearColor(osg::Vec4(0.0f,0.0f,0.0f,0.0f));

        // set viewport
        _camera->setViewport(0,0,tex_width,tex_height);

        // set the camera to render before the main camera.
        _camera->setRenderOrder(osg::CameraNode::PRE_RENDER);

        // tell the camera to use OpenGL frame buffer object where supported.
        _camera->setRenderTargetImplementation(osg::CameraNode::FRAME_BUFFER_OBJECT);

        // attach the texture and use it as the color buffer.
        _camera->attach(osg::CameraNode::COLOR_BUFFER, _texture.get());
    }

    if (!_texgenNode) _texgenNode = new osg::TexGenNode;

    if (!_mainSubgraphStateSet) _mainSubgraphStateSet = new osg::StateSet;

    setOverlayTextureUnit(_textureUnit);
}


void OverlayNode::traverse(osg::NodeVisitor& nv)
{
    if (nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
    {


        Group::traverse(nv);

        if (_continuousUpdate)
        {

            // now compute the camera's view and projection matrix to point at the shadower (the camera's children)
            osg::BoundingSphere bs;
            for(unsigned int i=0; i<_camera->getNumChildren(); ++i)
            {
                bs.expandBy(_camera->getChild(i)->getBound());
            }

            if (bs.valid())
            {
                // see if we are within a coordinate system node.
                osg::CoordinateSystemNode* csn = 0;
                osg::NodePath& nodePath = nv.getNodePath();
                for(osg::NodePath::reverse_iterator itr = nodePath.rbegin();
                    itr != nodePath.rend() && csn==0;
                    ++itr)
                {
                    csn = dynamic_cast<osg::CoordinateSystemNode*>(*itr);
                }

                _camera->setReferenceFrame(osg::CameraNode::ABSOLUTE_RF);

                if (csn)
                {
                    osg::Vec3d eyePoint(0.0,0.0,0.0); // center of the planet
                    double centerDistance = (eyePoint-osg::Vec3d(bs.center())).length();

                    double znear = centerDistance-bs.radius();
                    double zfar  = centerDistance+bs.radius();
                    double zNearRatio = 0.001f;
                    if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;

                    double top   = (bs.radius()/centerDistance)*znear;
                    double right = top;

                    _camera->setProjectionMatrixAsFrustum(-right,right,-top,top,znear,zfar);
                    _camera->setViewMatrixAsLookAt(eyePoint, bs.center(), osg::Vec3(0.0f,1.0f,0.0f));
                }
                else
                {
                    osg::Vec3d upDirection(0.0,1.0,0.0);
                    osg::Vec3d viewDirection(0.0,0.0,1.0);

                    double viewDistance = 2.0*bs.radius();
                    osg::Vec3d center = bs.center();
                    osg::Vec3d eyePoint = center+viewDirection*viewDistance;

                    double znear = viewDistance-bs.radius();
                    double zfar  = viewDistance+bs.radius();

                    float top   = bs.radius();
                    float right = top;

                    _camera->setProjectionMatrixAsOrtho(-right,right,-top,top,znear,zfar);
                    _camera->setViewMatrixAsLookAt(eyePoint,center,upDirection);

                }


                // compute the matrix which takes a vertex from local coords into tex coords
                // will use this later to specify osg::TexGen..
                osg::Matrix MVP = _camera->getViewMatrix() * 
                                  _camera->getProjectionMatrix();

                osg::Matrix MVPT = MVP *
                                   osg::Matrix::translate(1.0,1.0,1.0) *
                                   osg::Matrix::scale(0.5,0.5,0.5);

                _texgenNode->getTexGen()->setMode(osg::TexGen::EYE_LINEAR);
                _texgenNode->getTexGen()->setPlanesFromMatrix(MVPT);

                _textureFrustum.setToUnitFrustum(false,false);
                _textureFrustum.transformProvidingInverse(MVP);
            }
        }

        return;
    }

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
    if (!_textureObjectValidList[contextID] || _continuousUpdate)
    {
        _camera->accept(*cv);
        _textureObjectValidList[contextID] = 1;
    }
    
    
    // now set up the drawing of the main scene.
#if 0 
    // Disable for time being as _overlaySubgraphBound isn't accurate for
    // detecting whether the overaly texture will sit over affect the rendering
    // of the children of the OverlayNode.  Frustrum intersection may prove more
    // fruitful.

    // note needs to use bound when captured not current bound.
    if (!_overlaySubgraphBound.valid() || cv->isCulled(_overlaySubgraphBound))
    {
        Group::traverse(nv);
    }
    else
#endif
    {

        _texgenNode->accept(*cv);
        
        const osg::Matrix modelView = (cv->getModelViewMatrix());
        osg::Polytope viewTextureFrustum;
        viewTextureFrustum.setAndTransformProvidingInverse(_textureFrustum, osg::Matrix::inverse(modelView));

        cv->getProjectionCullingStack().back().addStateFrustum(_mainSubgraphStateSet.get(), viewTextureFrustum);
        cv->getCurrentCullingSet().addStateFrustum(_mainSubgraphStateSet.get(), _textureFrustum);
        
        // push the stateset.
        // cv->pushStateSet(_mainSubgraphStateSet.get());

        Group::traverse(nv);

        // cv->popStateSet();

        cv->getCurrentCullingSet().getStateFrustumList().pop_back();
        cv->getProjectionCullingStack().back().getStateFrustumList().pop_back();
    }
}

void OverlayNode::setOverlaySubgraph(osg::Node* node)
{
    if (_overlaySubgraph == node) return;

    _overlaySubgraph = node;

    _camera->removeChildren(0, _camera->getNumChildren());
    _camera->addChild(node);

    dirtyOverlayTexture();
}

void OverlayNode::dirtyOverlayTexture()
{
    _textureObjectValidList.setAllElementsTo(0);
}

void OverlayNode::setOverlayClearColor(const osg::Vec4& color)
{
    _camera->setClearColor(color);
}

const osg::Vec4& OverlayNode::getOverlayClearColor() const
{
    return _camera->getClearColor();
}


void OverlayNode::setTexEnvMode(GLenum mode)
{
    _texEnvMode = mode;
    updateMainSubgraphStateSet();
}


void OverlayNode::setOverlayTextureUnit(unsigned int unit)
{
    _textureUnit = unit;

    _texgenNode->setTextureUnit(_textureUnit);
    
    updateMainSubgraphStateSet();
}

void OverlayNode::setOverlayTextureSizeHint(unsigned int size)
{
    if (_textureSizeHint == size) return;

    _textureSizeHint = size;    
    //_texture->dirtyTextureObject();
    _texture->setTextureSize(_textureSizeHint, _textureSizeHint);
    _camera->setViewport(0,0,_textureSizeHint,_textureSizeHint);
}

void OverlayNode::updateMainSubgraphStateSet()
{
    _mainSubgraphStateSet->clear();
    _mainSubgraphStateSet->setTextureAttributeAndModes(_textureUnit, _texture.get(), osg::StateAttribute::ON);
    _mainSubgraphStateSet->setTextureMode(_textureUnit, GL_TEXTURE_GEN_S, osg::StateAttribute::ON);
    _mainSubgraphStateSet->setTextureMode(_textureUnit, GL_TEXTURE_GEN_T, osg::StateAttribute::ON);
    _mainSubgraphStateSet->setTextureMode(_textureUnit, GL_TEXTURE_GEN_R, osg::StateAttribute::ON);
    _mainSubgraphStateSet->setTextureMode(_textureUnit, GL_TEXTURE_GEN_Q, osg::StateAttribute::ON);

    if (_texEnvMode!=GL_NONE) 
    {
        _mainSubgraphStateSet->setTextureAttribute(_textureUnit, new osg::TexEnv((osg::TexEnv::Mode)_texEnvMode));
    }
}
