/* OpenSceneGraph example, osgfpdepth.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/
#include <osg/ColorMask>
#include <osg/CullFace>
#include <osg/Depth>
#include <osg/FrameBufferObject>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/GLExtensions>
#include <osg/Node>
#include <osg/Notify>
#include <osg/observer_ptr>
#include <osg/Projection>
#include <osg/Switch>
#include <osg/Texture2D>
#include <osg/ContextData>

#include <osgDB/ReadFile>
#include <osgGA/GUIEventHandler>
#include <osgUtil/Optimizer>

#include <osgText/Text>

#include <osgViewer/Renderer>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <iostream>
#include <sstream>

/* Demonstration of floating point depth buffers. The most basic way to use
 * a floating point depth buffer in OpenGL is to create a frame buffer
 * object, attach a color and floating point depth texture, render,
 * and then copy the color texture to the screen. When doing
 * multisampling we can't use textures directly, so we have to create
 * render buffers with the proper format. Then we let OSG handle the
 * details of resolving the multisampling.
 *
 * When using a floating point depth buffer, it's advantageous to
 * reverse the depth buffer range (and the depth test, of course) so
 * that 0.0 corresponds to the far plane. See
 * e.g. http://www.humus.name/index.php?ID=25 for details.
 */
using namespace osg;
using namespace std;

/* createFBO() and destroyFBO(), and the supporting classes and
 * functions below, are only used to test possible valid frame buffer
 * configurations at startup. They wouldn't be used in a normal OSG
 * program unless we wanted to enumerate all the valid FBO
 * combinations and let the user choose between them.
 */

// Properties of an FBO that we will try to create
struct FboConfig
{
    FboConfig()
        : colorFormat(0), depthFormat(0), redbits(0), depthBits(0),
          depthSamples(0), coverageSamples(0)
    {
    }
    FboConfig(const string& name_, GLenum colorFormat_, GLenum depthFormat_,
              int redbits_, int depthBits_, int depthSamples_ = 0,
              int coverageSamples_ = 0)
        : name(name_), colorFormat(colorFormat_), depthFormat(depthFormat_),
          redbits(redbits_), depthBits(depthBits_), depthSamples(depthSamples_),
          coverageSamples(coverageSamples_)
    {
    }
    string name;
    GLenum colorFormat;
    GLenum depthFormat;
    int redbits;
    int depthBits;
    int depthSamples;
    int coverageSamples;
};

// Properties of a buffer
struct BufferConfig
{
    BufferConfig() {}
    BufferConfig(const string& name_, GLenum format_, int bits_)
        : name(name_), format(format_), bits(bits_)
    {
    }
    string name;
    GLenum format;
    int bits;
};

typedef vector<BufferConfig> BufferConfigList;

vector<FboConfig> validConfigs;
// Ugly global variables for the viewport width and height
// int width, height;

// This is only used when testing possible frame buffer configurations
// to find valid ones.
struct FboData
{
    ref_ptr<Texture2D> tex;             // color texture
    ref_ptr<Texture2D> depthTex;        // depth texture
    ref_ptr<FrameBufferObject> fb;      // render framebuffer
    ref_ptr<FrameBufferObject> resolveFB; // multisample resolve target
};

Texture2D* makeDepthTexture(int width, int height, GLenum internalFormat);

// Assemble lists of the valid buffer configurations, along with the
// possibilities for multisample coverage antialiasing, if any.
void getPossibleConfigs(GraphicsContext* gc, BufferConfigList& colorConfigs,
                        BufferConfigList& depthConfigs,
                        vector<int>& coverageConfigs)
{
    int maxSamples = 0;
    int coverageSampleConfigs = 0;
    unsigned contextID = gc->getState()->getContextID();
    colorConfigs.push_back(BufferConfig("RGBA8", GL_RGBA8, 8));
    depthConfigs.push_back(BufferConfig("D24", GL_DEPTH_COMPONENT24, 24));
    osg::GLExtensions* ext = gc->getState()->get<GLExtensions>();
    if (!ext->isRenderbufferMultisampleSupported())
        return;
    if (ext->isMultisampleSupported)
        glGetIntegerv(GL_MAX_SAMPLES_EXT, &maxSamples);
    // isMultisampleCoverageSupported
    if (isGLExtensionSupported(contextID,
                               "GL_NV_framebuffer_multisample_coverage"))
    {
        glGetIntegerv(GL_MAX_MULTISAMPLE_COVERAGE_MODES_NV,
                      &coverageSampleConfigs);
        coverageConfigs.resize(coverageSampleConfigs * 2 + 4);
        glGetIntegerv(GL_MULTISAMPLE_COVERAGE_MODES_NV, &coverageConfigs[0]);
    }
    if (isGLExtensionSupported(contextID, "GL_ARB_depth_buffer_float"))
        depthConfigs.push_back(BufferConfig("D32F", GL_DEPTH_COMPONENT32F, 32));
    else if (isGLExtensionSupported(contextID, "GL_NV_depth_buffer_float"))
        depthConfigs.push_back(BufferConfig("D32F", GL_DEPTH_COMPONENT32F_NV,
                                            32));
}

bool checkFramebufferStatus(GraphicsContext* gc, bool silent = false)
{
    State& state = *gc->getState();
    osg::GLExtensions* ext = state.get<GLExtensions>();
    switch(ext->glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)) {
        case GL_FRAMEBUFFER_COMPLETE_EXT:
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            if (!silent)
                cout << "Unsupported framebuffer format\n";
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            if (!silent)
                cout << "Framebuffer incomplete, missing attachment\n";
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            if (!silent)
                cout << "Framebuffer incomplete, duplicate attachment\n";
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            if (!silent)
                cout << "Framebuffer incomplete, attached images must have same dimensions\n";
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            if (!silent)
                cout << "Framebuffer incomplete, attached images must have same format\n";
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            if (!silent)
                cout << "Framebuffer incomplete, missing draw buffer\n";
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            if (!silent)
                cout << "Framebuffer incomplete, missing read buffer\n";
            return false;
        default:
            return false;
    }
    return true;
}

// Attempt to create an FBO with a certain configuration. If the FBO
// is created with fewer bits in any of its parameters, the creation
// is deemed to have failed. Even though the result is a valid FBO,
// we're only interested in discrete, valid configurations.
bool createFBO(GraphicsContext* gc, FboConfig &config, FboData &data)
{
    bool result = true;
    bool multisample = config.depthSamples > 0;
    bool csaa = config.coverageSamples > config.depthSamples;
    data.fb = new FrameBufferObject;
    int texWidth = 512, texHeight = 512;
    data.tex = new Texture2D;
    data.tex->setTextureSize(texWidth, texHeight);
    data.tex->setInternalFormat(config.colorFormat);
    data.tex->setSourceFormat(GL_RGBA);
    data.tex->setSourceType(GL_FLOAT);
    data.tex->setFilter(Texture::MIN_FILTER, Texture::LINEAR_MIPMAP_LINEAR);
    data.tex->setFilter(Texture::MAG_FILTER, Texture::LINEAR);
    data.tex->setWrap(Texture::WRAP_S, Texture::CLAMP_TO_EDGE);
    data.tex->setWrap(Texture::WRAP_T, Texture::CLAMP_TO_EDGE);
    RenderBuffer* colorRB = 0;
    RenderBuffer* depthRB = 0;
    if (multisample)
    {
        data.resolveFB = new FrameBufferObject;
        data.resolveFB->setAttachment(Camera::COLOR_BUFFER,
                                      FrameBufferAttachment(data.tex.get()));
        colorRB = new RenderBuffer(texWidth, texHeight, config.colorFormat,
                                   config.coverageSamples, config.depthSamples);
        data.fb->setAttachment(Camera::COLOR_BUFFER,
                               FrameBufferAttachment(colorRB));
        depthRB = new RenderBuffer(texWidth, texHeight, config.depthFormat,
                                   config.coverageSamples, config.depthSamples);
        data.fb->setAttachment(Camera::DEPTH_BUFFER,
                               FrameBufferAttachment(depthRB));
    }
    else
    {
        data.depthTex = makeDepthTexture(texWidth, texHeight,
                                         config.depthFormat);
        data.fb->setAttachment(Camera::COLOR_BUFFER,
                               FrameBufferAttachment(data.tex.get()));
        data.fb->setAttachment(Camera::DEPTH_BUFFER,
                               FrameBufferAttachment(data.depthTex.get()));
    }
    State& state = *gc->getState();
    unsigned int contextID = state.getContextID();
    osg::GLExtensions* ext = gc->getState()->get<GLExtensions>();

    data.fb->apply(state);
    result = checkFramebufferStatus(gc, true);
    if (!result)
    {
        ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
        return false;
    }
    int query;
    if (multisample)
    {
        GLuint colorRBID = colorRB->getObjectID(contextID, ext);
        ext->glBindRenderbuffer(GL_RENDERBUFFER_EXT, colorRBID);
        if (csaa)
        {
            ext->glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT,
                                               GL_RENDERBUFFER_COVERAGE_SAMPLES_NV,
                                               &query);
            if (query < config.coverageSamples)
                result = false;
            else
                config.coverageSamples = query;
            ext->glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT,
                                               GL_RENDERBUFFER_COLOR_SAMPLES_NV,
                                               &query);

            if ( query < config.depthSamples)
               result = false;
            else
                config.depthSamples = query; // report back the actual number

        }
        else
        {
            ext->glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT,
                                               GL_RENDERBUFFER_SAMPLES_EXT,
                                               &query);
            if (query < config.depthSamples)
                result = false;
            else
                config.depthSamples = query;
        }
    }
    glGetIntegerv( GL_RED_BITS, &query);
    if (query != config.redbits)
        result = false;
    glGetIntegerv(GL_DEPTH_BITS, &query);
    if ( query != config.depthBits)
        result = false;
    if (result && multisample && data.resolveFB.valid())
    {
        data.resolveFB->apply(state);
        result = checkFramebufferStatus(gc, true);
        if (result)
        {
            glGetIntegerv( GL_RED_BITS, &query);
            if (query != config.redbits)
                result = false;
        }
    }
    ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    return result;
}

void destroyFBO(GraphicsContext* gc, FboData &data)
{
    data.tex = 0;
    data.depthTex = 0;
    data.fb = 0;
    data.resolveFB = 0;
    State& state = *gc->getState();

    osg::get<GLRenderBufferManager>(state.getContextID())->flushAllDeletedGLObjects();
    osg::get<GLFrameBufferObjectManager>(state.getContextID())->flushAllDeletedGLObjects();
}

void setAttachmentsFromConfig(Camera* camera, const FboConfig& config);
Switch* makeTexturesAndGeometry(int width, int height, Switch* sw = 0);

// Application state accessed from event handlers and main function;
// contains state that can be changed by the user and the OSG classes
// used to display / indicate that state.
//
// camera - Camera with fbo, using either fp depth buffer or fixed
// switch child 0 - texture containing rendering of scene
// switch child 1 - fp depth buffer as texture
// switch child 2 - integer depth buffer as texture
// textNotAvailable- "not available" text if texture isn't valid.

struct AppState : public Referenced
{
    AppState(osgViewer::Viewer* viewer_);
    void setStateFromConfig(const FboConfig& config);
    void advanceConfig(int increment);
    void updateDisplayedTexture();
    void updateNear();
    virtual ~AppState() {}
    ref_ptr<Switch> sw;         // switch between displayed texture
    bool displayScene;
    bool invertRange;
    int currentConfig;
    osgViewer::Viewer* viewer;
    double zNear;
    ref_ptr<Camera> camera;
    // text displayed on the screen showing the user's choices
    ref_ptr<Projection> textProjection;
    ref_ptr<osgText::Text> configText;
    ref_ptr<osgText::Text> zNearText;
    ref_ptr<Geode> textNotAvailable;
    ref_ptr<Geode> textInverted;
};

AppState::AppState(osgViewer::Viewer* viewer_)
    : displayScene(true), invertRange(true), currentConfig(0),
      viewer(viewer_), zNear(0.03125)
{
    sw = new Switch;
    string fontName("fonts/arial.ttf");
    // Text description of current config
    configText = new osgText::Text;
    configText->setDataVariance(Object::DYNAMIC);
    configText->setFont(fontName);
    configText->setPosition(Vec3(50.0f, 50.0f, 0.0f));
    configText->setColor(Vec4(1.0, 1.0, 1.0, 1.0));
    Geode* textGeode = new Geode;
    textGeode->addDrawable(configText.get());
    // Text for the near plane distance
    zNearText = new osgText::Text;
    zNearText->setDataVariance(Object::DYNAMIC);
    zNearText->setFont(fontName);
    zNearText->setPosition(Vec3(1230.0f, 50.0f, 0.0f));
    zNearText->setColor(Vec4(1.0, 1.0, 1.0, 1.0));
    zNearText->setAlignment(osgText::Text::RIGHT_BASE_LINE);
    textGeode->addDrawable(zNearText.get());
    // Projection that lets the text be placed in pixels.
    textProjection = new Projection;
    textProjection->setMatrix(Matrix::ortho2D(0,1280,0,1024));
    textProjection->addChild(textGeode);
    // "texture not available" text displayed when the user tries to
    // display the depth texture while multisampling.
    osgText::Text* noCanDo = new osgText::Text;
    noCanDo->setFont(fontName);
    noCanDo->setPosition(Vec3(512.0f, 384.0f, 0.0f));
    noCanDo->setColor(Vec4(1.0, 0.0, 0.0, 1.0));
    noCanDo->setText("not available");
    textNotAvailable = new Geode;
    textNotAvailable->addDrawable(noCanDo);
    textProjection->addChild(textNotAvailable.get());
    // Is the depth test inverted?
    osgText::Text* inverted = new osgText::Text;
    inverted->setFont(fontName);
    inverted->setPosition(Vec3(512.0f, 50.0f, 0.0f));
    inverted->setColor(Vec4(1.0, 1.0, 1.0, 1.0));
    inverted->setText("inverted depth test");
    textInverted = new Geode;
    textInverted->addDrawable(inverted);
    textInverted->setNodeMask(~0u);
    textProjection->addChild(textInverted.get());
    textProjection->getOrCreateStateSet()->setRenderBinDetails(11, "RenderBin");
}

void AppState::setStateFromConfig(const FboConfig& config)
{
    Camera* cam = viewer->getSlave(0)._camera.get();
    setAttachmentsFromConfig(cam, config);
    osgViewer::Renderer* renderer
        = dynamic_cast<osgViewer::Renderer*>(cam->getRenderer());
    if (renderer)
        renderer->setCameraRequiresSetUp(true);
    if (configText.valid())
    {
        configText->setText(validConfigs[currentConfig].name);
        configText->update();
    }
    updateDisplayedTexture();
}

void AppState::advanceConfig(int increment)
{
    currentConfig = (currentConfig + increment) % validConfigs.size();
    setStateFromConfig(validConfigs[currentConfig]);
}

void AppState::updateDisplayedTexture()
{
    if (displayScene)
        sw->setSingleChildOn(0);
    else if (validConfigs[currentConfig].depthSamples > 0
             || validConfigs[currentConfig].coverageSamples > 0)
        sw->setAllChildrenOff();
    else if (validConfigs[currentConfig].depthFormat != GL_DEPTH_COMPONENT24)
        sw->setSingleChildOn(2);
    else
        sw->setSingleChildOn(3);
    if (displayScene
        || (validConfigs[currentConfig].depthSamples == 0
            && validConfigs[currentConfig].coverageSamples == 0))
        textNotAvailable->setNodeMask(0u);
    else
        textNotAvailable->setNodeMask(~0u);
}

void AppState::updateNear()
{
    // Assume that the viewing frustum is symmetric.
    double fovy, aspectRatio, cNear, cFar;
    viewer->getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio,
                                                          cNear, cFar);
    viewer->getCamera()->setProjectionMatrixAsPerspective(fovy, aspectRatio,
                                                          zNear, cFar);
    stringstream nearStream;
    nearStream << "near: " << zNear;
    zNearText->setText(nearStream.str());
    zNearText->update();
}

class ConfigHandler : public osgGA::GUIEventHandler
{
public:

    ConfigHandler(AppState* appState)
        : _appState(appState)
    {
    }

    virtual bool handle(const osgGA::GUIEventAdapter& ea,
                        osgGA::GUIActionAdapter& aa,
                        Object*, NodeVisitor* /*nv*/)
    {
        if (ea.getHandled()) return false;
        osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
        if (!viewer) return false;
        switch(ea.getEventType())
        {
        case osgGA::GUIEventAdapter::KEYUP:
        {
            if (ea.getKey()=='d')
            {
                _appState->displayScene = !_appState->displayScene;
                _appState->updateDisplayedTexture();
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Right ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Right)
            {
                _appState->advanceConfig(1);
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Left ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Left)
            {
                _appState->advanceConfig(-1);
                return true;
            }
            break;
        }
        default:
            break;
        }
        return false;
    }

    void getUsage(ApplicationUsage& usage) const
    {
        usage.addKeyboardMouseBinding("d", "display depth texture");
        usage.addKeyboardMouseBinding("right arrow",
                                      "next frame buffer configuration");
        usage.addKeyboardMouseBinding("left arrow",
                                      "previous frame buffer configuration");
    }

protected:
    virtual ~ConfigHandler() {}
    ref_ptr<AppState> _appState;
};

class DepthHandler : public osgGA::GUIEventHandler
{
public:

    DepthHandler(AppState *appState, Depth* depth)
        : _appState(appState), _depth(depth)
    {
        depth->setDataVariance(Object::DYNAMIC);
    }

    virtual bool handle(const osgGA::GUIEventAdapter& ea,
                        osgGA::GUIActionAdapter& /*aa*/,
                        Object*, NodeVisitor* /*nv*/)
    {
        if (ea.getHandled()) return false;

        ref_ptr<Depth> depth;
        if (!_depth.lock(depth)) return false;

        switch(ea.getEventType())
        {
        case(osgGA::GUIEventAdapter::KEYUP):
        {
            if (ea.getKey() == 'i')
            {
                _appState->invertRange = !_appState->invertRange;
                if (!_appState->invertRange)
                {
                    _appState->camera->setClearDepth(1.0f);
                    depth->setFunction(Depth::LESS);
                    depth->setRange(0.0f, 1.0f);
                    _appState->textInverted->setNodeMask(0u);
                }
                else
                {
                    _appState->camera->setClearDepth(0.0f);
                    depth->setFunction(Depth::GEQUAL);
                    depth->setRange(1.0f, 0.0f);
                    _appState->textInverted->setNodeMask(~0u);
                }
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Up)
            {
                _appState->zNear *= 2.0;
                _appState->updateNear();
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Down)
            {
                _appState->zNear *= .5;
                _appState->updateNear();
                return true;
            }
            break;
        }
        default:
            break;
        }
        return false;
    }

    void getUsage(ApplicationUsage& usage) const
    {
        usage.addKeyboardMouseBinding("i", "invert depth buffer range");
        usage.addKeyboardMouseBinding("up arrow",
                                      "double near plane distance");
        usage.addKeyboardMouseBinding("down arrow",
                                      "half near plane distance");
    }
protected:
    virtual ~DepthHandler() {}
    ref_ptr<AppState> _appState;
    observer_ptr<Depth> _depth;
};

Geode* createTextureQuad(Texture2D *texture)
{
    Vec3Array *vertices = new Vec3Array;
    vertices->push_back(Vec3(-1.0, -1.0, 0.0));
    vertices->push_back(Vec3(1.0, -1.0, 0.0));
    vertices->push_back(Vec3(1.0, 1.0, 0.0));
    vertices->push_back(Vec3(-1.0, 1.0, 0.0));

    Vec2Array *texcoord = new Vec2Array;
    texcoord->push_back(Vec2(0.0, 0.0));
    texcoord->push_back(Vec2(1.0, 0.0));
    texcoord->push_back(Vec2(1.0, 1.0));
    texcoord->push_back(Vec2(0.0, 1.0));

    Geometry *geom = new Geometry;
    geom->setVertexArray(vertices);
    geom->setTexCoordArray(0, texcoord);
    geom->addPrimitiveSet(new DrawArrays(GL_QUADS, 0, 4));

    Geode *geode = new Geode;
    geode->addDrawable(geom);
    geode->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture, StateAttribute::ON);

    return geode;
}

struct ResizedCallback : public osg::GraphicsContext::ResizedCallback
{
    ResizedCallback(AppState* appState)
        : _appState(appState)
    {
    }
    void resizedImplementation(GraphicsContext* gc, int x, int y, int width,
                               int height);
    ref_ptr<AppState> _appState;
};

void ResizedCallback::resizedImplementation(GraphicsContext* gc, int x, int y,
                                            int width, int height)
{
    gc->resizedImplementation(x, y, width, height);
    makeTexturesAndGeometry(width, height, _appState->sw.get());
    _appState->setStateFromConfig(validConfigs[_appState
                                               ->currentConfig]);
    osgViewer::Viewer* viewer = _appState->viewer;
    Viewport* vp = viewer->getSlave(0)._camera->getViewport();
    if (vp)
    {
        double oldWidth = vp->width(), oldHeight = vp->height();
        double aspectRatioChange
            = (width / oldWidth) / (height / oldHeight);
        vp->setViewport(0, 0, width, height);
        if (aspectRatioChange != 1.0)
        {
            Camera* master = viewer->getCamera();
            switch (master->getProjectionResizePolicy())
            {
            case Camera::HORIZONTAL:
                master->getProjectionMatrix()
                    *= Matrix::scale(1.0/aspectRatioChange,1.0,1.0);
                break;
            case Camera::VERTICAL:
                master->getProjectionMatrix()
                    *= Matrix::scale(1.0, aspectRatioChange,1.0);
                break;
            default:
                break;
            }
        }
    }
}

// Prefer GL_DEPTH_COMPONENT32F, otherwise use
// GL_DEPTH_COMPONENT32F_NV if available
GLenum depthTextureEnum = 0;

// Standard OSG code for initializing osgViewer::Viewer with explicit
// creation of own graphics context. This is also a good time to test
// for valid frame buffer configurations; we have a valid graphics
// context, but multithreading hasn't started, etc.
GraphicsContext* setupGC(osgViewer::Viewer& viewer, ArgumentParser& arguments)
{
    int x = -1, y = -1, width = -1, height = -1;
    while (arguments.read("--window",x,y,width,height)) {}

    GraphicsContext::WindowingSystemInterface* wsi =
        GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
    {
        OSG_NOTIFY(NOTICE)<<"View::setUpViewOnSingleScreen() : Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return 0;
    }

    DisplaySettings* ds = viewer.getDisplaySettings() ? viewer.getDisplaySettings() : DisplaySettings::instance().get();
    GraphicsContext::ScreenIdentifier si;
    si.readDISPLAY();
    si.setUndefinedScreenDetailsToDefaultScreen();

    bool decoration = true;
    if (x < 0)
    {
        unsigned int w, h;
        wsi->getScreenResolution(si, w, h);
        OSG_NOTICE<<"Screen resolution is "<<w<<", "<<h<<std::endl;
        OSG_NOTICE<<"ScreenIdentifier "<<si.displayNum<<", "<<si.screenNum<<std::endl;
        x = 0;
        y = 0;
        width = w;
        height = h;
        decoration = false;
    }

    OSG_NOTICE<<"x = "<<x<<", y = "<<y<<", width = "<<width<<", height = "<<height<<std::endl;

    ref_ptr<GraphicsContext::Traits> traits = new GraphicsContext::Traits(ds);
    traits->hostName = si.hostName;
    traits->displayNum = si.displayNum;
    traits->screenNum = si.screenNum;
    traits->x = x;
    traits->y = y;
    traits->width = width;
    traits->height = height;
    traits->windowDecoration = decoration;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
    traits->readDISPLAY();
    traits->setUndefinedScreenDetailsToDefaultScreen();

    ref_ptr<GraphicsContext> gc = GraphicsContext::createGraphicsContext(traits.get());
    osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(gc.get());
    if (gw)
    {
        OSG_NOTIFY(INFO)<<"View::setUpViewOnSingleScreen - GraphicsWindow has been created successfully."<<std::endl;
        gw->getEventQueue()->getCurrentEventState()
            ->setWindowRectangle(0, 0, width, height);
    }
    else
    {
        OSG_NOTIFY(NOTICE)<<"  GraphicsWindow has not been created successfully."<<std::endl;
    }
    double fovy, aspectRatio, zNear, zFar;
    viewer.getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio,
                                                         zNear, zFar);
    double newAspectRatio = double(traits->width) / double(traits->height);
    double aspectRatioChange = newAspectRatio / aspectRatio;
    if (aspectRatioChange != 1.0)
    {
        viewer.getCamera()->getProjectionMatrix()
            *= Matrix::scale(1.0/aspectRatioChange,1.0,1.0);
    }
    // Context has to be current to test for extensions
    gc->realize();
    if (!gc->makeCurrent())
    {
        OSG_NOTIFY(NOTICE) << "Unable to create GraphicsWindow"<<std::endl;
        gc->releaseContext();
        gc->close(true);
        return 0;
    }

    unsigned int contextID = gc->getState()->getContextID();
    osg::GLExtensions* ext = gc->getState()->get<GLExtensions>();
    if (!ext->isFrameBufferObjectSupported)
    {
        OSG_NOTIFY(NOTICE) << "Frame buffer objects are not supported\n";
        gc->releaseContext();
        gc->close(true);
        return 0;
    }

    if (isGLExtensionSupported(contextID, "GL_ARB_depth_buffer_float"))
        depthTextureEnum = GL_DEPTH_COMPONENT32F;
    else if (isGLExtensionSupported(contextID, "GL_NV_depth_buffer_float"))
        depthTextureEnum = GL_DEPTH_COMPONENT32F_NV;

    BufferConfigList colorConfigs;
    BufferConfigList depthConfigs;
    vector<int> coverageConfigs;
    getPossibleConfigs(gc.get(), colorConfigs, depthConfigs, coverageConfigs);
    int coverageSampleConfigs = (coverageConfigs.size() - 4) / 2;
    cout << "color configs\nname\tbits\n";
    for (BufferConfigList::const_iterator colorItr = colorConfigs.begin(),
             colorEnd = colorConfigs.end();
         colorItr != colorEnd;
         ++colorItr)
    {
        for (BufferConfigList::const_iterator depthItr = depthConfigs.begin(),
             depthEnd = depthConfigs.end();
             depthItr != depthEnd;
             ++depthItr)
        {
            string root = colorItr->name + " " + depthItr->name;
            FboConfig config(root, colorItr->format, depthItr->format,
                             colorItr->bits, depthItr->bits);
            FboData data;
            if (createFBO(gc.get(), config, data))
                validConfigs.push_back(config);
            destroyFBO(gc.get(), data);
            if (coverageConfigs.size() > 0)
            {
                //CSAA provides a list of all supported AA modes for
                //quick enumeration
                for (int kk = 0; kk < coverageSampleConfigs; kk++)
                {
                    stringstream msText;
                    msText << root;
                    config.depthSamples = coverageConfigs[kk*2+1];
                    config.coverageSamples = coverageConfigs[kk*2];

                    if ( config.coverageSamples == config.depthSamples )
                    {
                        // Normal antialiasing
                        msText << " - " << config.depthSamples << " MSAA";
                    }
                    else
                    {
                        // coverage antialiasing
                        msText << " - " << config.coverageSamples << "/"
                               << config.depthSamples << " CSAA";
                    }
                    config.name = msText.str();

                    if (createFBO(gc.get(), config, data)) {
                        validConfigs.push_back( config);
                    }
                    destroyFBO(gc.get(), data);
                }
            }
        }
    }
    if (validConfigs.empty())
    {
        cout << "no valid frame buffer configurations!\n";
        return 0;
    }
    cout << "valid frame buffer configurations:\n";
    for (vector<FboConfig>::iterator itr = validConfigs.begin(),
             end = validConfigs.end();
         itr != end;
         ++itr)
        cout << itr->name << "\n";
    gc->releaseContext();

    return gc.release();
}

ref_ptr<Texture2D> colorTexture;
ref_ptr<Texture2D> depthTexture;
ref_ptr<Texture2D> depthTexture24;

Texture2D* makeDepthTexture(int width, int height, GLenum internalFormat)
{
    Texture2D *depthTex = new Texture2D;
    depthTex->setTextureSize(width, height);
    depthTex->setSourceFormat(GL_DEPTH_COMPONENT);
    depthTex->setSourceType(GL_FLOAT);
    depthTex->setInternalFormat(internalFormat);
    depthTex->setFilter(Texture2D::MIN_FILTER, Texture2D::NEAREST);
    depthTex->setFilter(Texture2D::MAG_FILTER, Texture2D::NEAREST);
    depthTex->setWrap(Texture::WRAP_S, Texture::CLAMP_TO_EDGE);
    depthTex->setWrap(Texture::WRAP_T, Texture::CLAMP_TO_EDGE);
    return depthTex;
}

Camera* makeRttCamera(GraphicsContext* gc, int width, int height)
{
    Camera* rttCamera = new Camera;
    rttCamera->setGraphicsContext(gc);
    rttCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    rttCamera->setClearColor(Vec4(0.0, 0.4, 0.5, 0.0));
    // normally the depth test is inverted, although the user can
    // change that.
    rttCamera->setClearDepth(0.0);
    rttCamera->setViewport(0, 0, width, height);
    rttCamera->setDrawBuffer(GL_FRONT);
    rttCamera->setReadBuffer(GL_FRONT);
    rttCamera->setRenderTargetImplementation(Camera::FRAME_BUFFER_OBJECT);
    rttCamera->setComputeNearFarMode(CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
    return rttCamera;
}

void setAttachmentsFromConfig(Camera* camera, const FboConfig& config)
{
    // XXX Detaching the old buffers may not be necessary.
    if (!camera->getBufferAttachmentMap().empty())
    {
        camera->detach(Camera::COLOR_BUFFER);
        camera->detach(Camera::DEPTH_BUFFER);
    }
    camera->attach(Camera::COLOR_BUFFER, colorTexture.get(), 0, 0, false,
                   config.coverageSamples, config.depthSamples);
    if (config.coverageSamples != 0 || config.depthSamples != 0)
        camera->attach(Camera::DEPTH_BUFFER, config.depthFormat);
    else if (config.depthFormat == GL_DEPTH_COMPONENT24)
        camera->attach(Camera::DEPTH_BUFFER, depthTexture24.get());
    else
        camera->attach(Camera::DEPTH_BUFFER, depthTexture.get());
}

// Create the parts of the local scene graph used to display the final
// results.
Switch* makeTexturesAndGeometry(int width, int height, Switch* sw)
{
    if (!sw)
        sw = new Switch;
    colorTexture = new Texture2D;
    colorTexture->setTextureSize(width, height);
    colorTexture->setInternalFormat(GL_RGBA);
    colorTexture->setFilter(Texture2D::MIN_FILTER, Texture2D::LINEAR);
    colorTexture->setFilter(Texture2D::MAG_FILTER, Texture2D::LINEAR);
    colorTexture->setWrap(Texture::WRAP_S, Texture::CLAMP_TO_EDGE);
    colorTexture->setWrap(Texture::WRAP_T, Texture::CLAMP_TO_EDGE);
    colorTexture->setBorderColor(Vec4(0, 0, 0, 0));

    depthTexture24 = makeDepthTexture(width, height, GL_DEPTH_COMPONENT24);
    if (depthTextureEnum)
        depthTexture = makeDepthTexture(width, height, depthTextureEnum);
    else
        depthTexture = depthTexture24;
    sw->removeChildren(0, sw->getNumChildren());
    sw->addChild(createTextureQuad(colorTexture.get()));
    sw->addChild(createTextureQuad(depthTexture.get()));
    sw->addChild(createTextureQuad(depthTexture24.get()));
    sw->setSingleChildOn(0);
    return sw;
}

int main(int argc, char **argv)
{
    // use an ArgumentParser object to manage the program arguments.
    ArgumentParser arguments(&argc,argv);
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()
                         + " demonstrates using a floating point depth buffer.\nThe user can invert the depth buffer range and choose among available multi-sample configurations.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("--far <number>", "Set far plane value");
    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }
    float zFar = 500.0f;
    while (arguments.read("--far", zFar))
        ;
    // construct the viewer.
    osgViewer::Viewer viewer;
    ref_ptr<AppState> appState = new AppState(&viewer);
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.addEventHandler(new osgViewer::WindowSizeHandler);
    viewer.addEventHandler(new osgViewer::ScreenCaptureHandler);
    // The aspect ratio is set to the correct ratio for the window in
    // setupGC().
    viewer.getCamera()
        ->setProjectionMatrixAsPerspective(40.0, 1.0, appState->zNear, zFar);
    GraphicsContext* gc = setupGC(viewer, arguments);
    if (!gc)
        return 1;
    gc->setResizedCallback(new ResizedCallback(appState.get()));
    const GraphicsContext::Traits* traits = gc->getTraits();
    int width = traits->width;
    int height = traits->height;
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }
    ref_ptr<Node> loadedModel = osgDB::readRefNodeFiles(arguments);
    if (!loadedModel) {
        cerr << "couldn't load " << argv[1] << "\n";
        return 1;
    }
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel.get());
    // creates texture to be rendered
    Switch* sw = makeTexturesAndGeometry(width, height, appState->sw.get());
    ref_ptr<Camera> rttCamera = makeRttCamera(gc, width, height);
    rttCamera->setRenderOrder(Camera::PRE_RENDER);
    viewer.addSlave(rttCamera.get());
    appState->camera = rttCamera;
    // geometry and slave camera to display the result
    Group* displayRoot = new Group;
    displayRoot->addChild(sw);
    displayRoot->addChild(appState->textProjection.get());
    StateSet* displaySS = displayRoot->getOrCreateStateSet();
    displaySS->setMode(GL_LIGHTING, StateAttribute::OFF);
    displaySS->setMode(GL_DEPTH_TEST, StateAttribute::OFF);
    Camera* texCamera = new Camera;
    texCamera->setGraphicsContext(gc);
    texCamera->setClearMask(GL_COLOR_BUFFER_BIT);
    texCamera->setClearColor(Vec4(0.0, 0.0, 0.0, 0.0));
    texCamera->setReferenceFrame(Camera::ABSOLUTE_RF);
    texCamera->setViewport(0, 0, width, height);
    texCamera->setDrawBuffer(GL_BACK);
    texCamera->setReadBuffer(GL_BACK);
    texCamera->addChild(displayRoot);
    texCamera->setAllowEventFocus(false);
    texCamera->setCullingMode(CullSettings::NO_CULLING);
    texCamera->setProjectionResizePolicy(Camera::FIXED);
    viewer.addSlave(texCamera, Matrixd(), Matrixd(), false);
    viewer.addEventHandler(new ConfigHandler(appState.get()));

    // add model to the viewer.
    Group* sceneRoot = new Group;
    StateSet* sceneSS = sceneRoot->getOrCreateStateSet();
    Depth* depth = new Depth(Depth::GEQUAL, 1.0, 0.0);
    sceneSS->setAttributeAndModes(depth,(StateAttribute::ON
                                         | StateAttribute::OVERRIDE));
#if 0
    // Hack to work around Blender osg export bug
    sceneSS->setAttributeAndModes(new CullFace(CullFace::BACK));
#endif
    sceneRoot->addChild(loadedModel.get());
    appState->setStateFromConfig(validConfigs[0]);
    appState->updateNear();
    viewer.addEventHandler(new DepthHandler(appState.get(), depth));
    // add the help handler
    viewer.addEventHandler(new osgViewer
                           ::HelpHandler(arguments.getApplicationUsage()));

    viewer.setSceneData(sceneRoot);

    return viewer.run();
}
