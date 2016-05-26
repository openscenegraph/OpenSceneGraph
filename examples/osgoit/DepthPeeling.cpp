
#include "DepthPeeling.h"

#include <osg/Array>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Vec3>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/TexGen>
#include <osg/TexEnv>
#include <osg/TexMat>
#include <osg/TexGenNode>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osg/Math>

#include <limits>
#include <iostream>

const char *DepthPeeling::PeelingShader =
{
    "#version 120\n"
#ifdef USE_TEXTURE_RECTANGLE
    "#extension GL_ARB_texture_rectangle : enable\n"
    "uniform sampler2DRectShadow depthtex;\n"
#else
    "uniform sampler2DShadow depthtex;\n"
#endif
    "uniform bool depthtest;\n"  // depth test enable flag
    "uniform float invWidth;\n"  // 1.0/width (shadow texture size)
    "uniform float invHeight;\n" // 1.0/height (shadow texture size)
    "uniform float offsetX;\n"   // viewport lower left corner (int)
    "uniform float offsetY;\n"   // viewport lower left corner (int)
    "\n"
    "bool depthpeeling()\n"
    "{\n"
    "  if( depthtest ) {\n"
    "    vec3 r0 = vec3((gl_FragCoord.x-offsetX)*invWidth,\n"
    "                   (gl_FragCoord.y-offsetY)*invHeight,\n"
    "                    gl_FragCoord.z);\n"
#ifdef USE_TEXTURE_RECTANGLE
    "    return shadow2DRect(depthtex, r0).r < 0.5;\n"
#else
    "    return shadow2D(depthtex, r0).r < 0.5;\n"
#endif
    "  }\n"
    "  return false;\n"
    "}\n"
};

class PreDrawFBOCallback : public osg::Camera::DrawCallback
{
public:
  PreDrawFBOCallback( osg::FrameBufferObject* fbo, osg::FrameBufferObject* source_fbo, unsigned int width, unsigned int height, osg::Texture *dt, osg::Texture *ct ) :
 _fbo(fbo), _source_fbo(source_fbo), _depthTexture(dt), _colorTexture(ct), _width(width), _height(height) {}

  virtual void operator () (osg::RenderInfo& renderInfo) const
  {
      // switching only the frame buffer attachments is actually faster than switching the framebuffer
#ifdef USE_PACKED_DEPTH_STENCIL
#ifdef USE_TEXTURE_RECTANGLE
     _fbo->setAttachment(osg::Camera::PACKED_DEPTH_STENCIL_BUFFER, osg::FrameBufferAttachment((osg::TextureRectangle*)(_depthTexture.get())));
#else
     _fbo->setAttachment(osg::Camera::PACKED_DEPTH_STENCIL_BUFFER, osg::FrameBufferAttachment((osg::Texture2D*)(_depthTexture.get())));
#endif
#else
#ifdef USE_TEXTURE_RECTANGLE
     _fbo->setAttachment(osg::Camera::DEPTH_BUFFER, osg::FrameBufferAttachment((osg::TextureRectangle*)(_depthTexture.get())));
#else
     _fbo->setAttachment(osg::Camera::DEPTH_BUFFER, osg::FrameBufferAttachment((osg::Texture2D*)(_depthTexture.get())));
#endif
#endif
#ifdef USE_TEXTURE_RECTANGLE
     _fbo->setAttachment(osg::Camera::COLOR_BUFFER0, osg::FrameBufferAttachment((osg::TextureRectangle*)(_colorTexture.get())));
#else
     _fbo->setAttachment(osg::Camera::COLOR_BUFFER0, osg::FrameBufferAttachment((osg::Texture2D*)(_colorTexture.get())));
#endif

     // check if we need to do some depth buffer copying from a source FBO into the current FBO
     if (_source_fbo.get() != NULL)
     {
         osg::GLExtensions* ext = renderInfo.getState()->get<osg::GLExtensions>();
         bool fbo_supported = ext && ext->isFrameBufferObjectSupported;
         if (fbo_supported && ext->glBlitFramebuffer)
         {
             // blit the depth buffer from the solid geometry fbo into the current transparency fbo
             (_fbo.get())->apply(*renderInfo.getState(), osg::FrameBufferObject::DRAW_FRAMEBUFFER);
             (_source_fbo.get())->apply(*renderInfo.getState(), osg::FrameBufferObject::READ_FRAMEBUFFER);

//             glReadBuffer(GL_COLOR_ATTACHMENT0_EXT); // only needed to blit the color buffer
//             glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT); // only needed to blit the color buffer
             ext->glBlitFramebuffer(
                 0, 0, static_cast<GLint>(_width), static_cast<GLint>(_height),
                 0, 0, static_cast<GLint>(_width), static_cast<GLint>(_height),
#ifdef USE_PACKED_DEPTH_STENCIL
                 GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
#else
                 GL_DEPTH_BUFFER_BIT, GL_NEAREST);
#endif
             (_fbo.get())->apply(*renderInfo.getState(), osg::FrameBufferObject::READ_FRAMEBUFFER);
             (_fbo.get())->apply(*renderInfo.getState(), osg::FrameBufferObject::DRAW_FRAMEBUFFER);
         }
     }
     // switch to this fbo, if it isn't already bound
     (_fbo.get())->apply( *renderInfo.getState() );
  }
protected:
  osg::ref_ptr<osg::FrameBufferObject> _fbo;
  osg::ref_ptr<osg::FrameBufferObject> _source_fbo;
  osg::ref_ptr<osg::Texture> _depthTexture;
  osg::ref_ptr<osg::Texture> _colorTexture;
  unsigned int _width;
  unsigned int _height;
};


class PostDrawFBOCallback : public osg::Camera::DrawCallback
{
public:
  PostDrawFBOCallback(bool restore) : _restore(restore) {}

  virtual void operator () (osg::RenderInfo& renderInfo) const
  {
    // only unbind the fbo if this is the last transparency pass
    if (_restore)
    {
        renderInfo.getState()->get<osg::GLExtensions>()->glBindFramebuffer( GL_FRAMEBUFFER_EXT, 0 );
    }
  }
protected:
  bool _restore;
};


DepthPeeling::CullCallback::CullCallback(unsigned int texUnit, unsigned int offsetValue) :
    _texUnit(texUnit),
    _offsetValue(offsetValue)
{
}

void DepthPeeling::CullCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    osgUtil::CullVisitor* cullVisitor = static_cast<osgUtil::CullVisitor*>(nv);
    osgUtil::RenderStage* renderStage = cullVisitor->getCurrentRenderStage();
    const osg::Viewport* viewport = renderStage->getViewport();

    osg::Matrixd m(*cullVisitor->getProjectionMatrix());
    m.postMultTranslate(osg::Vec3d(1, 1, 1));
    m.postMultScale(osg::Vec3d(0.5, 0.5, 0.5));

    // scale the texture coordinates to the viewport
#ifdef USE_TEXTURE_RECTANGLE
    m.postMultScale(osg::Vec3d(viewport->width(), viewport->height(), 1));
#else
#ifndef USE_NON_POWER_OF_TWO_TEXTURE
    m.postMultScale(osg::Vec3d(viewport->width()/double(_texWidth), viewport->height()/double(_texHeight), 1));
#endif
#endif

    if (_texUnit != 0 && _offsetValue)
    {
        // Kind of polygon offset: note this way, we can also offset lines and points.
        // Whereas with the polygon offset we could only handle surface primitives.
        m.postMultTranslate(osg::Vec3d(0, 0, -ldexp(double(_offsetValue), -24)));
    }

    osg::TexMat* texMat = new osg::TexMat(m);
    osg::StateSet* stateSet = new osg::StateSet;
    stateSet->setTextureAttribute(_texUnit, texMat);

    if (_texUnit != 0)
    {
        //
        // GLSL pipeline support
        //

#ifdef USE_TEXTURE_RECTANGLE
        // osg::Uniform::SAMPLER_2D_RECT_SHADOW not yet available in OSG 3.0.1
        // osg::Uniform* depthUniform = new osg::Uniform(osg::Uniform::SAMPLER_2D_RECT_SHADOW, "depthtex");
        // depthUniform->set((int)_texUnit);
        osg::Uniform* depthUniform = new osg::Uniform("depthtex", (int)_texUnit);
        osg::Uniform *invWidthUniform = new osg::Uniform("invWidth", (float)1.0f);
        osg::Uniform *invHeightUniform = new osg::Uniform("invHeight", (float)1.0f);
#else
        osg::Uniform* depthUniform = new osg::Uniform(osg::Uniform::SAMPLER_2D_SHADOW, "depthtex");
        depthUniform->set((int)_texUnit);
        osg::Uniform *invWidthUniform = new osg::Uniform("invWidth", (float)1.0f / _texWidth);
        osg::Uniform *invHeightUniform = new osg::Uniform("invHeight", (float)1.0f / _texHeight);
#endif
        osg::Uniform *offsetXUniform = new osg::Uniform("offsetX", (float)viewport->x());
        osg::Uniform *offsetYUniform = new osg::Uniform("offsetY", (float)viewport->y());

        // uniforms required for any any GLSL implementation in the rendered geometry
        stateSet->addUniform(depthUniform);
        stateSet->addUniform(invWidthUniform);
        stateSet->addUniform(invHeightUniform);
        stateSet->addUniform(offsetXUniform);
        stateSet->addUniform(offsetYUniform);
    }

    cullVisitor->pushStateSet(stateSet);
    traverse(node, nv);
    cullVisitor->popStateSet();
}


osg::Node* DepthPeeling::createQuad(unsigned int layerNumber, unsigned int numTiles)
{
    float tileSpan = 1;
    float tileOffsetX = 0;
    float tileOffsetY = 0;
    if (_showAllLayers) {
        tileSpan /= numTiles;
        tileOffsetX = tileSpan * (layerNumber%numTiles);
        tileOffsetY = 1 - tileSpan * (1 + layerNumber/numTiles);
    }

    osg::Vec3Array* vertices = new osg::Vec3Array;

    vertices->push_back(osg::Vec3f(tileOffsetX           , tileOffsetY            , 0));
    vertices->push_back(osg::Vec3f(tileOffsetX           , tileOffsetY  + tileSpan, 0));
    vertices->push_back(osg::Vec3f(tileOffsetX + tileSpan, tileOffsetY  + tileSpan, 0));
    vertices->push_back(osg::Vec3f(tileOffsetX + tileSpan, tileOffsetY            , 0));

    osg::Vec3Array* colors = new osg::Vec3Array;
    colors->push_back(osg::Vec3(1, 1, 1));

    osg::Vec2Array* texcoords = new osg::Vec2Array;
    texcoords->push_back(osg::Vec2f(0, 0));
    texcoords->push_back(osg::Vec2f(0, 1));
    texcoords->push_back(osg::Vec2f(1, 1));
    texcoords->push_back(osg::Vec2f(1, 0));

    osg::Geometry* geometry = new osg::Geometry;
    geometry->setVertexArray(vertices);
    geometry->setTexCoordArray(0, texcoords);

    geometry->setColorArray(colors, osg::Array::BIND_OVERALL);

    geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geometry);

    return geode;
}

#include <osg/State>

void DepthPeeling::createPeeling()
{
    int numTiles = ceil(sqrt(double(_numPasses)));

    // cleanup any previous scene data
    _root->removeChildren(0, _root->getNumChildren());

    // create depth textures
    _depthTextures.clear();
    _depthTextures.resize(3);
    for (unsigned int i = 0; i < 3; ++i) {
#ifdef USE_TEXTURE_RECTANGLE
        _depthTextures[i] = new osg::TextureRectangle;
#else
        _depthTextures[i] = new osg::Texture2D;
#endif
        _depthTextures[i]->setTextureSize(_texWidth, _texHeight);

        _depthTextures[i]->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
        _depthTextures[i]->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
        _depthTextures[i]->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
        _depthTextures[i]->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);

#ifdef USE_PACKED_DEPTH_STENCIL
        _depthTextures[i]->setInternalFormat(GL_DEPTH24_STENCIL8_EXT);
        _depthTextures[i]->setSourceFormat(GL_DEPTH_STENCIL_EXT);
        _depthTextures[i]->setSourceType(GL_UNSIGNED_INT_24_8_EXT);
#else
        _depthTextures[i]->setInternalFormat(GL_DEPTH_COMPONENT);
#endif

        _depthTextures[i]->setShadowComparison(true);
        _depthTextures[i]->setShadowAmbient(0.0); // The r value if the test fails
        _depthTextures[i]->setShadowCompareFunc(osg::Texture::GREATER);
        _depthTextures[i]->setShadowTextureMode(osg::Texture::INTENSITY);
    }

    // create the cameras for the individual depth peel layers
    _colorTextures.clear();
    _colorTextures.resize(_numPasses);
    for (unsigned int i = 0; i < _numPasses; ++i) {

        // create textures for the color buffers
#ifdef USE_TEXTURE_RECTANGLE
        osg::ref_ptr<osg::TextureRectangle> colorTexture = new osg::TextureRectangle;
#else
        osg::ref_ptr<osg::Texture2D> colorTexture = new osg::Texture2D;
#endif
        colorTexture->setTextureSize(_texWidth, _texHeight);
        colorTexture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
        colorTexture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
        colorTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
        colorTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);
        colorTexture->setInternalFormat(GL_RGBA);

        _colorTextures[i] = colorTexture;
    }

    // create some uniform and cull callback objects
    osg::Uniform *depthOff = new osg::Uniform("depthtest", (bool)false);
    osg::Uniform *depthOn  = new osg::Uniform("depthtest", (bool)true);
    CullCallback *ccb      = new CullCallback(_texUnit, _offsetValue);

    // create a node for solid model rendering
    osg::Group *pre_solidNode = new osg::Group;
    pre_solidNode->addChild(_solidscene.get());

    // create a node for non depth peeled transparent rendering (topmost layer)
    osg::Group *transparentNodeNoPeel = new osg::Group;
    transparentNodeNoPeel->addChild(_transparentscene.get());
    transparentNodeNoPeel->getOrCreateStateSet()->addUniform(depthOff);
    transparentNodeNoPeel->getOrCreateStateSet()->setRenderBinDetails(99, "RenderBin", osg::StateSet::OVERRIDE_RENDERBIN_DETAILS);

    // create a node for depth peeled transparent rendering (any layers below).
    osg::TexGenNode* transparentNodePeel = new osg::TexGenNode;
    transparentNodePeel->setReferenceFrame(osg::TexGenNode::ABSOLUTE_RF);
    transparentNodePeel->setTextureUnit(_texUnit);
    transparentNodePeel->getTexGen()->setMode(osg::TexGen::EYE_LINEAR);
    transparentNodePeel->addChild(_transparentscene.get());
    transparentNodePeel->getOrCreateStateSet()->addUniform(depthOn);
    transparentNodePeel->getOrCreateStateSet()->setRenderBinDetails(99, "RenderBin", osg::StateSet::OVERRIDE_RENDERBIN_DETAILS);

    // only render fragments that are not completely transparent
    transparentNodePeel->getOrCreateStateSet()->setAttributeAndModes(new osg::AlphaFunc(osg::AlphaFunc::GREATER, 0.01),
                                    osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    // generate texcoords for the depth texture, supporting the fixed function pipeline
    transparentNodePeel->getOrCreateStateSet()->setTextureMode(_texUnit, GL_TEXTURE_GEN_S, osg::StateAttribute::ON);
    transparentNodePeel->getOrCreateStateSet()->setTextureMode(_texUnit, GL_TEXTURE_GEN_T, osg::StateAttribute::ON);
    transparentNodePeel->getOrCreateStateSet()->setTextureMode(_texUnit, GL_TEXTURE_GEN_R, osg::StateAttribute::ON);
    transparentNodePeel->getOrCreateStateSet()->setTextureMode(_texUnit, GL_TEXTURE_GEN_Q, osg::StateAttribute::ON);

    // use two FBOs, one for solid geometry - the other one for the transparency passes
    // depth and color attachments will be switched as needed.
    osg::ref_ptr<osg::FrameBufferObject> fbos[2] = {new osg::FrameBufferObject(), new osg::FrameBufferObject()};

    // create the cameras for the individual depth peel layers
    for (unsigned int i = 0; i < _numPasses; ++i) {

        // get the pointers to the required fbo, color and depth textures for each camera instance
        // we perform ping ponging between two depth textures
        osg::FrameBufferObject *fbo0 = (i >= 1) ? fbos[0].get() : NULL;
        osg::FrameBufferObject *fbo  = (i >= 1) ? fbos[1].get() : fbos[0].get();
        osg::Texture *colorTexture     = _colorTextures[i].get();
        osg::Texture *depthTexture     = (i >= 1) ? _depthTextures[1+(i-1)%2].get() : _depthTextures[i].get();
        osg::Texture *prevDepthTexture = (i >= 2) ? _depthTextures[1+(i-2)%2].get() : NULL;

        // all our peeling layer cameras are post render
        osg::Camera* camera = new osg::Camera;
        camera->setDataVariance(osg::Object::DYNAMIC);
        camera->setInheritanceMask(osg::Camera::ALL_VARIABLES);
        camera->setRenderOrder(osg::Camera::POST_RENDER, i);
        camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        camera->setClearColor(osg::Vec4f(0, 0, 0, 0));
        camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);
        camera->setPreDrawCallback(new PreDrawFBOCallback(fbo, fbo0, _texWidth, _texHeight, depthTexture, colorTexture));
        camera->setPostDrawCallback(new PostDrawFBOCallback(i == _numPasses - 1));
        camera->setDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
        camera->setReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
        camera->setAllowEventFocus(false);

        // the peeled layers are rendered with blending forced off
        // and the depth buffer is directly taken from camera 0 via framebuffer blit
        if (i > 0) {
            camera->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
            camera->setClearMask(GL_COLOR_BUFFER_BIT);
        } else {
            // camera 0 has to clear both the depth and color buffers
            camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        }

        // add the correct geometry for each pass.
        // the peeling passes also need read access to prevDepthTexture and a cull callback
        if (0 == i) {        // solid geometry
            camera->addChild(pre_solidNode);
        } else if (1 == i) { // topmost layer peeling pass
            camera->addChild(transparentNodeNoPeel);
        } else {             // behind layers peeling passes
            camera->addChild(transparentNodePeel);
            // set depth (shadow) texture for depth peeling and add a cull callback
            camera->getOrCreateStateSet()->setTextureAttributeAndModes(_texUnit, prevDepthTexture);
            camera->addCullCallback(ccb);
        }
        _root->addChild(camera);
    }

    // create the composite camera that blends the peeled layers into the final scene
    _compositeCamera = new osg::Camera;
    _compositeCamera->setDataVariance(osg::Object::DYNAMIC);
    _compositeCamera->setInheritanceMask(osg::Camera::READ_BUFFER | osg::Camera::DRAW_BUFFER);
    _compositeCamera->setRenderOrder(osg::Camera::POST_RENDER, _numPasses);
    _compositeCamera->setComputeNearFarMode(osg::Camera::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
    _compositeCamera->setClearMask(0);
    _compositeCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    _compositeCamera->setViewMatrix(osg::Matrix());
    _compositeCamera->setProjectionMatrix(osg::Matrix::ortho2D(0, 1, 0, 1));
    _compositeCamera->setCullCallback(new CullCallback(0, 0));
    osg::StateSet* ss = _compositeCamera->getOrCreateStateSet();
    ss->setRenderBinDetails(100, "TraversalOrderBin", osg::StateSet::OVERRIDE_RENDERBIN_DETAILS);
    _root->addChild(_compositeCamera.get());

    // solid geometry is blended first, transparency layers are blended in back to front order.
    // this order is achieved by rendering using a TraversalOrderBin (see camera stateset).
    for (unsigned int i = _numPasses; i > 0; --i) {
        osg::Node* geode = createQuad(i%_numPasses, numTiles);
        osg::StateSet *stateSet = geode->getOrCreateStateSet();
        stateSet->setTextureAttributeAndModes(0, _colorTextures[i%_numPasses].get(), osg::StateAttribute::ON);
        stateSet->setAttribute(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA), osg::StateAttribute::ON);
        stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
        stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        osg::Depth* depth = new osg::Depth;
        depth->setWriteMask( false );
        stateSet->setAttributeAndModes( depth, osg::StateAttribute::ON );
        stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
        _compositeCamera->addChild(geode);
    }
}

DepthPeeling::DepthPeeling(unsigned int width, unsigned int height) :
    _numPasses(9),
    _texUnit(2),
    _texWidth(width),
    _texHeight(height),
    _showAllLayers(false),
    _offsetValue(8),
    _root(new osg::Group),
    _solidscene(new osg::Group),
    _transparentscene(new osg::Group)
{
    createPeeling();
}

void DepthPeeling::setSolidScene(osg::Node* scene)
{
    _solidscene->removeChildren(0, _solidscene->getNumChildren());
    _solidscene->addChild(scene);
}

void DepthPeeling::setTransparentScene(osg::Node* scene)
{
    _transparentscene->removeChildren(0, _transparentscene->getNumChildren());
    _transparentscene->addChild(scene);
}

osg::Node* DepthPeeling::getRoot()
{
    return _root.get();
}

void DepthPeeling::resize(int width, int height)
{
#ifdef USE_TEXTURE_RECTANGLE
    for (unsigned int i = 0; i < 3; ++i)
        _depthTextures[i]->setTextureSize(width, height);
    for (unsigned int i = 0; i < _colorTextures.size(); ++i)
        _colorTextures[i]->setTextureSize(width, height);
    _texWidth = width;
    _texHeight = height;
#else
#ifndef USE_NON_POWER_OF_TWO_TEXTURE
    width = nextPowerOfTwo(width);
    height = nextPowerOfTwo(height);
#endif
    _depthTextures[0]->setTextureSize(width, height);
    _depthTextures[1]->setTextureSize(width, height);
    for (unsigned int i = 0; i < _colorTextures.size(); ++i)
        _colorTextures[i]->setTextureSize(width, height);
    _texWidth = width;
    _texHeight = height;
#endif
    createPeeling();
}

void DepthPeeling::setNumPasses(unsigned int numPasses)
{
    if (numPasses == _numPasses)
        return;
    if (numPasses == unsigned(-1))
        return;
    _numPasses = numPasses;
    createPeeling();
}
unsigned int DepthPeeling::getNumPasses() const
{
    return _numPasses;
}

void DepthPeeling::setTexUnit(unsigned int texUnit)
{
    if (texUnit == _texUnit)
        return;
    _texUnit = texUnit;
    createPeeling();
}

void DepthPeeling::setShowAllLayers(bool showAllLayers)
{
    if (showAllLayers == _showAllLayers)
        return;
    _showAllLayers = showAllLayers;
    createPeeling();
}
bool DepthPeeling::getShowAllLayers() const
{
    return _showAllLayers;
}

void DepthPeeling::setOffsetValue(unsigned int offsetValue)
{
    if (offsetValue == _offsetValue)
        return;
    _offsetValue = offsetValue;
    createPeeling();
}
unsigned int DepthPeeling::getOffsetValue() const
{
    return _offsetValue;
}


DepthPeeling::EventHandler::EventHandler(DepthPeeling* depthPeeling) :
    _depthPeeling(depthPeeling)
{ }


/** Handle events, return true if handled, false otherwise. */
bool DepthPeeling::EventHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&, osg::Object*, osg::NodeVisitor*)
{
    if (ea.getEventType() == osgGA::GUIEventAdapter::RESIZE) {
        _depthPeeling->resize(ea.getWindowWidth(), ea.getWindowHeight());
        return true;
    }

    if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN) {
        switch (ea.getKey()) {
        case 'm':
            _depthPeeling->setNumPasses(_depthPeeling->getNumPasses() + 1);
            return true;
        case 'n':
            _depthPeeling->setNumPasses(_depthPeeling->getNumPasses() - 1);
            return true;
        case 'p':
            _depthPeeling->setOffsetValue(_depthPeeling->getOffsetValue() + 1);
            return true;
        case 'o':
            _depthPeeling->setOffsetValue(_depthPeeling->getOffsetValue() - 1);
            return true;
        case 'l':
            _depthPeeling->setShowAllLayers(!_depthPeeling->getShowAllLayers());
            return true;
        default:
            return false;
        };
    }

    return false;
}
