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

#include <osgDB/ReadFile>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osg/Math>

#include <limits>
#include <iostream>

// Some choices for the kind of textures we can use ...
#define USE_TEXTURE_RECTANGLE
// #define USE_NON_POWER_OF_TWO_TEXTURE
#define USE_PACKED_DEPTH_STENCIL

template<typename T>
inline T
nextPowerOfTwo(T k)
{
    if (k == T(0))
        return 1;
    k--;
    for (int i = 1; i < std::numeric_limits<T>::digits; i <<= 1)
        k = k | k >> i;
    return k + 1;
}

class DepthPeeling : public osg::Referenced {
public:
    osg::Node*
    createQuad(unsigned layerNumber, unsigned numTiles)
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
      
        geometry->setColorArray(colors);
        geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
      
        geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));
      
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(geometry);
      
        return geode;
    }
   
    class CullCallback : public osg::NodeCallback {
    public:
        CullCallback(unsigned texUnit, unsigned texWidth, unsigned texHeight, unsigned offsetValue) :
            _texUnit(texUnit),
            _texWidth(texWidth),
            _texHeight(texHeight),
            _offsetValue(offsetValue)
        {
        }
      
        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
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

            // Kind of polygon offset: note this way, we can also offset lines and points.
            // Whereas with the polygon offset we could only handle surface primitives.
            m.postMultTranslate(osg::Vec3d(0, 0, -ldexp(double(_offsetValue), -24)));

            osg::TexMat* texMat = new osg::TexMat(m);
            osg::StateSet* stateSet = new osg::StateSet;
            stateSet->setTextureAttribute(_texUnit, texMat);
            cullVisitor->pushStateSet(stateSet);
            traverse(node, nv);
            cullVisitor->popStateSet();
        }

    private:
        unsigned _texUnit;
        unsigned _texWidth;
        unsigned _texHeight;
        unsigned _offsetValue;
    };

    void
    createPeeling()
    {
        int numTiles = ceil(sqrt(double(_numPasses)));

        _root->removeChildren(0, _root->getNumChildren());
        _colorTextures.clear();

        // If not enabled, just use the top level camera
        if (!_depthPeelingEnabled) {
            _root->addChild(_scene.get());
            return;
        }

        _compositeCamera = new osg::Camera;
        _compositeCamera->setDataVariance(osg::Object::DYNAMIC);
        _compositeCamera->setInheritanceMask(osg::Camera::READ_BUFFER | osg::Camera::DRAW_BUFFER);
        _compositeCamera->setRenderOrder(osg::Camera::POST_RENDER);
        _compositeCamera->setComputeNearFarMode(osg::Camera::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
        _compositeCamera->setClearMask(0);

        _compositeCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
        _compositeCamera->setViewMatrix(osg::Matrix());
        _compositeCamera->setProjectionMatrix(osg::Matrix::ortho2D(0, 1, 0, 1));

        _compositeCamera->setCullCallback(new CullCallback(0, _texWidth, _texHeight, 0));

        osg::StateSet* stateSet = _compositeCamera->getOrCreateStateSet();
        stateSet->setBinName("TraversalOrderBin");
        stateSet->setRenderBinMode(osg::StateSet::USE_RENDERBIN_DETAILS);

        _root->addChild(_compositeCamera.get());

        for (unsigned i = 0; i < 2; ++i) {
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
            _depthTextures[i]->setInternalFormat(GL_DEPTH_COMPONENT24);
#endif

            _depthTextures[i]->setShadowComparison(true);
            _depthTextures[i]->setShadowAmbient(0); // The r value if the test fails
            _depthTextures[i]->setShadowCompareFunc(osg::Texture::GREATER);
            _depthTextures[i]->setShadowTextureMode(osg::Texture::INTENSITY);
        }

        // Then, the other ones
        for (unsigned i = 0; i < _numPasses; ++i) {
            osg::Camera* camera = new osg::Camera;
            camera->setDataVariance(osg::Object::DYNAMIC);

            camera->setInheritanceMask(osg::Camera::ALL_VARIABLES);
            camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
            camera->setRenderOrder(osg::Camera::PRE_RENDER, i);
            camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
            camera->setClearColor(osg::Vec4f(0, 0, 0, 0));

            camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);

            osg::ref_ptr<osg::Texture> depthTexture = _depthTextures[i%2];
            osg::ref_ptr<osg::Texture> prevDepthTexture = _depthTextures[(i+1)%2];

#ifdef USE_PACKED_DEPTH_STENCIL
            camera->attach(osg::Camera::PACKED_DEPTH_STENCIL_BUFFER, depthTexture.get());
#else
            camera->attach(osg::Camera::DEPTH_BUFFER, depthTexture.get());
#endif

#ifdef USE_TEXTURE_RECTANGLE
            osg::ref_ptr<osg::TextureRectangle> colorTexture = new osg::TextureRectangle;
#else
            osg::ref_ptr<osg::Texture2D> colorTexture = new osg::Texture2D;
#endif
            _colorTextures.push_back(colorTexture);

            colorTexture->setTextureSize(_texWidth, _texHeight);
            colorTexture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
            colorTexture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
            colorTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
            colorTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);
            colorTexture->setInternalFormat(GL_RGBA);
            camera->attach(osg::Camera::COLOR_BUFFER, colorTexture.get());

            camera->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
            if (0 == i) {
                camera->addChild(_scene.get());
            } else {
                osg::StateSet* stateSet = camera->getOrCreateStateSet();

                stateSet->setAttributeAndModes(new osg::AlphaFunc(osg::AlphaFunc::GREATER, 0.01),
                                               osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

                stateSet->setTextureAttributeAndModes(_texUnit, prevDepthTexture.get());

                // Is the default ...
                //         stateSet->setTextureAttributeAndModes(_texUnit, new osg::TexEnv(osg::TexEnv::MODULATE));
                stateSet->setTextureMode(_texUnit, GL_TEXTURE_GEN_S, osg::StateAttribute::ON);
                stateSet->setTextureMode(_texUnit, GL_TEXTURE_GEN_T, osg::StateAttribute::ON);
                stateSet->setTextureMode(_texUnit, GL_TEXTURE_GEN_R, osg::StateAttribute::ON);
                stateSet->setTextureMode(_texUnit, GL_TEXTURE_GEN_Q, osg::StateAttribute::ON);

                osg::TexGenNode* texGenNode = new osg::TexGenNode;
                texGenNode->setReferenceFrame(osg::TexGenNode::ABSOLUTE_RF);
                texGenNode->setTextureUnit(_texUnit);
                texGenNode->getTexGen()->setMode(osg::TexGen::EYE_LINEAR);
                camera->addChild(texGenNode);
                camera->addCullCallback(new CullCallback(_texUnit, _texWidth, _texHeight, _offsetValue));

                texGenNode->addChild(_scene.get());
            }

            _root->addChild(camera);

            osg::Node* geode = createQuad(i, numTiles);
            osg::StateSet* stateSet = geode->getOrCreateStateSet();
            stateSet->setTextureAttributeAndModes(0, colorTexture.get(), osg::StateAttribute::ON);
            stateSet->setAttribute(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA), osg::StateAttribute::ON);
            stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
            stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
            stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
            _compositeCamera->insertChild(0, geode);
        }
    }

    DepthPeeling(unsigned width, unsigned height) :
        _numPasses(8),
        _texUnit(1),
        _texWidth(width),
        _texHeight(height),
        _showAllLayers(false),
        _depthPeelingEnabled(true),
        _offsetValue(8),
        _root(new osg::Group),
        _scene(new osg::Group)
    {
        createPeeling();
    }

    void setScene(osg::Node* scene)
    {
        _scene->removeChildren(0, _scene->getNumChildren());
        _scene->addChild(scene);
    }

    osg::Node* getRoot()
    {
        return _root.get();
    }

    void resize(int width, int height)
    {
#ifdef USE_TEXTURE_RECTANGLE
        _depthTextures[0]->setTextureSize(width, height);
        _depthTextures[1]->setTextureSize(width, height);
        for (unsigned i = 0; i < _colorTextures.size(); ++i)
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
        for (unsigned i = 0; i < _colorTextures.size(); ++i)
            _colorTextures[i]->setTextureSize(width, height);
        _texWidth = width;
        _texHeight = height;
#endif
        createPeeling();
    }

    void setNumPasses(unsigned numPasses)
    {
        if (numPasses == _numPasses)
            return;
        if (numPasses == unsigned(-1))
            return;
        _numPasses = numPasses;
        createPeeling();
    }
    unsigned getNumPasses() const
    {
        return _numPasses;
    }

    void setTexUnit(unsigned texUnit)
    {
        if (texUnit == _texUnit)
            return;
        _texUnit = texUnit;
        createPeeling();
    }

    void setShowAllLayers(bool showAllLayers)
    {
        if (showAllLayers == _showAllLayers)
            return;
        _showAllLayers = showAllLayers;
        createPeeling();
    }
    bool getShowAllLayers() const
    {
        return _showAllLayers;
    }

    void setDepthPeelingEnabled(bool depthPeelingEnabled)
    {
        if (depthPeelingEnabled == _depthPeelingEnabled)
            return;
        _depthPeelingEnabled = depthPeelingEnabled;
        createPeeling();
    }
    bool getDepthPeelingEnabled() const
    {
        return _depthPeelingEnabled;
    }

    void setOffsetValue(unsigned offsetValue)
    {
        if (offsetValue == _offsetValue)
            return;
        _offsetValue = offsetValue;
        createPeeling();
    }
    unsigned getOffsetValue() const
    {
        return _offsetValue;
    }

    unsigned _numPasses;
    unsigned _texUnit;
    unsigned _texWidth;
    unsigned _texHeight;
    bool _showAllLayers;
    bool _depthPeelingEnabled;
    unsigned _offsetValue;

    // The root node that is handed over to the viewer
    osg::ref_ptr<osg::Group> _root;

    // The scene that is displayed
    osg::ref_ptr<osg::Group> _scene;

    // The final camera that composites the pre rendered textures to the final picture
    osg::ref_ptr<osg::Camera> _compositeCamera;

#ifdef USE_TEXTURE_RECTANGLE
    osg::ref_ptr<osg::TextureRectangle> _depthTextures[2];
    std::vector<osg::ref_ptr<osg::TextureRectangle> > _colorTextures;
#else
    osg::ref_ptr<osg::Texture2D> _depthTextures[2];
    std::vector<osg::ref_ptr<osg::Texture2D> > _colorTextures;
#endif
};

class EventHandler : public osgGA::GUIEventHandler {
public:
    EventHandler(DepthPeeling* depthPeeling) :
        _depthPeeling(depthPeeling)
    { }

    /** Handle events, return true if handled, false otherwise. */
    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&, osg::Object*, osg::NodeVisitor*)
    {
        if (ea.getEventType() == osgGA::GUIEventAdapter::RESIZE) {
            _depthPeeling->resize(ea.getWindowWidth(), ea.getWindowHeight());
            return true;
        }

        if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN) {
            switch (ea.getKey()) {
            case 'd':
                _depthPeeling->setDepthPeelingEnabled(!_depthPeeling->getDepthPeelingEnabled());
                return true;
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

    osg::ref_ptr<DepthPeeling> _depthPeeling;
};

int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc, argv);
    arguments.getApplicationUsage()->addKeyboardMouseBinding("d", "Toggle depth peeling enabled");
    arguments.getApplicationUsage()->addKeyboardMouseBinding("m", "Increase the number of depth peeling layers");
    arguments.getApplicationUsage()->addKeyboardMouseBinding("n", "Decrease the number of depth peeling layers");
    arguments.getApplicationUsage()->addKeyboardMouseBinding("l", "Toggle display of the individual or composed layer textures");
    arguments.getApplicationUsage()->addKeyboardMouseBinding("p", "Increase the layer offset");
    arguments.getApplicationUsage()->addKeyboardMouseBinding("o", "Decrease the layer offset");

    // Have the usual viewer
    osgViewer::Viewer viewer(arguments);

    osg::DisplaySettings* displaySettings = new osg::DisplaySettings;
    viewer.setDisplaySettings(displaySettings);
   
    // Add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);
   
    // add the help handler
    viewer.addEventHandler(new osgViewer::HelpHandler(arguments.getApplicationUsage()));

    // load the data
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    if (!loadedModel)
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }
   
    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();
   
    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    // The initial size sez to 0, 0. We get a resize event for the right size ...
    DepthPeeling* depthPeeling = new DepthPeeling(0, 0);
    depthPeeling->setScene(loadedModel.get());
    viewer.setSceneData(depthPeeling->getRoot());

    // Add the event handler for the depth peeling stuff
    viewer.addEventHandler(new EventHandler(depthPeeling));
   
    return viewer.run();
}
