#include <osgUtil/SceneView>

#include <osg/Notify>
#include <osg/Texture>
#include <osg/AlphaFunc>
#include <osg/TexEnv>

#include <GL/glu.h>

using namespace osg;
using namespace osgUtil;

SceneView::SceneView()
{
    _calc_nearfar = true;

    _backgroundColor.set(0.2f, 0.2f, 0.4f, 1.0f);

    _near_plane = 1.0f;
    _far_plane = 1.0f;

    _lodBias = 1.0f;

    _lightingMode=HEADLIGHT;
    
    _prioritizeTextures = false;
    
    _view[0] = 0;
    _view[1] = 0;
    _view[2] = 1024;
    _view[3] = 768;
    
    _frameNumber = 0;
    
}


SceneView::~SceneView()
{
}


void SceneView::setDefaults()
{
    _globalState = new osg::StateSet;

    _lightingMode=HEADLIGHT;
    _light = new osg::Light;
    _light->setAmbient(Vec4(0.05f,0.05f,0.05f,1.0f));
    _light->setDiffuse(Vec4(0.8f,0.8f,0.8f,1.0f));
    _light->setSpecular(Vec4(0.1f,0.1f,0.1f,1.0f));

   

    _camera = new osg::Camera;
    
    _state = new osg::State;
    
    _rendergraph = new osgUtil::RenderGraph;
    _renderStage = new osgUtil::RenderStage;
    _cullVisitor = new osgUtil::CullVisitor;

    _cullVisitor->setRenderGraph(_rendergraph.get());
    _cullVisitor->setRenderStage(_renderStage.get());

    _globalState->setGlobalDefaults();
    
    // enable lighting by default.
    _globalState->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    
    // enable depth testing by default.
    _globalState->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

    // set up an alphafunc by default to speed up blending operations.
    osg::AlphaFunc* alphafunc = new osg::AlphaFunc();
    alphafunc->setFunction(osg::AlphaFunc::GREATER,0.0f);
    _globalState->setAttributeAndModes(alphafunc, osg::StateAttribute::ON);

    // set up an alphafunc by default to speed up blending operations.
    osg::TexEnv* texenv = new osg::TexEnv();
    texenv->setMode(osg::TexEnv::MODULATE);
    _globalState->setAttributeAndModes(texenv, osg::StateAttribute::ON);

    _backgroundColor.set(0.2f, 0.2f, 0.4f, 1.0f);

}


void SceneView::app()
{
    ++_frameNumber;
    
    if (_sceneData.valid() && _appVisitor.valid())
    { 
        _appVisitor->reset();
        _sceneData->accept(*_appVisitor.get());
    }
}

void SceneView::cull()
{
    if (!_sceneData) return;

    _camera->adjustAspectRatio((GLfloat)_view[2]/(GLfloat) _view[3]);
    
    
    _rendergraph->clean();

    _cullVisitor->reset();

    // comment out reset of rendergraph since clean is more efficient.
    //  _rendergraph->reset();

    // use clean of the rendergraph rather than reset, as it is able to
    // reuse the structure on the rendergraph in the next frame. This
    // achieves a certain amount of frame cohereancy of memory allocation.

    _cullVisitor->setFrameNumber(_frameNumber);
    _cullVisitor->setLODBias(_lodBias);
    _cullVisitor->setCamera(*_camera);
    _cullVisitor->setViewport(_view[0],_view[1],_view[2],_view[3]);

    _renderStage->reset();

    _renderStage->setViewport(_view[0],_view[1],_view[2],_view[3]);
    _renderStage->setCamera(_camera.get());
    _renderStage->setClearColor(_backgroundColor);
    _renderStage->setLight(_light.get());


    switch(_lightingMode)
    {
    case(HEADLIGHT):
        _renderStage->setLightingMode(RenderStageLighting::HEADLIGHT);
        break;
    case(SKY_LIGHT):
        _renderStage->setLightingMode(RenderStageLighting::SKY_LIGHT);
        break;
    case(NO_SCENEVIEW_LIGHT):
        _renderStage->setLightingMode(RenderStageLighting::NO_SCENEVIEW_LIGHT);
        break;
    }            

    if (_globalState.valid()) _cullVisitor->pushStateSet(_globalState.get());


    // traverse the scene graph to generate the rendergraph.
    _sceneData->accept(*_cullVisitor);

    if (_globalState.valid()) _cullVisitor->popStateSet();


    // do any state sorting required.
    _renderStage->sort();


    // prune out any empty RenderGraph children.
    // note, this would be not required if the _renderGraph had been
    // reset at the start of each frame (see top of this method) but
    // a clean has been used instead to try to minimize the amount of
    // allocation and deleteing of the RenderGraph nodes.
    _rendergraph->prune();

    if (_calc_nearfar)
    {
        _near_plane = _cullVisitor->getCalculatedNearPlane();
        _far_plane = _cullVisitor->getCalculatedFarPlane();

        if (_near_plane<=_far_plane)
        {
            // shift the far plane slight further away from the eye point.
            // and shift the near plane slightly near the eye point, this
            // will give a little space betwenn the near and far planes
            // and the model, crucial when the naer and far planes are
            // coincedent.
            _far_plane  *= 1.05;
            _near_plane *= 0.95;

            // if required clamp the near plane to prevent negative or near zero
            // near planes.
            float min_near_plane = _far_plane*0.0005f;
            if (_near_plane<min_near_plane) _near_plane=min_near_plane;
        }
        else
        {
            _near_plane = 1.0f;
            _far_plane = 1000.0f;
        }

        _camera->setNearFar(_near_plane,_far_plane);
    }

}


void SceneView::draw()
{
    if (!_sceneData) return;

    if (!_state)
    {
        osg::notify(osg::WARN) << "Warning: no valid osgUtil::SceneView::_state"<<endl;
        osg::notify(osg::WARN) << "         creating a state automatically."<<endl;

        // note the constructor for osg::State will set ContextID to 0.
        _state = new osg::State;
    }
    // we in theory should be able to 
    _state->reset();

    // note, to support multi-pipe systems the deletion of OpenGL display list
    // and texture objects is deferred until the OpenGL context is the correct
    // context for when the object were originally created.  Here we know what
    // context we are in so can flush the appropriate caches.
    osg::Drawable::flushDeletedDisplayLists(_state->getContextID());
    osg::Texture::flushDeletedTextureObjects(_state->getContextID());

    RenderLeaf* previous = NULL;
    _renderStage->draw(*_state,previous);
    
    GLenum errorNo = glGetError();
    if (errorNo!=GL_NO_ERROR)
    {
        osg::notify(WARN)<<"Warning: detected OpenGL error '"<<gluErrorString(errorNo)<<"'"<<endl;
    }
    
}


/** Calculate, via glUnProject, the object coordinates of a window point.
    Note, current implementation requires that SceneView::draw() has been previously called
    for projectWindowIntoObject to produce valid values.  As per OpenGL
    windows coordinates are calculated relative to the bottom left of the window.*/
bool SceneView::projectWindowIntoObject(const osg::Vec3& window,osg::Vec3& object) const
{
    return _camera->unproject(window,_view,object);
}


/** Calculate, via glUnProject, the object coordinates of a window x,y
    when projected onto the near and far planes.
    Note, current implementation requires that SceneView::draw() has been previously called
    for projectWindowIntoObject to produce valid values.  As per OpenGL
    windows coordinates are calculated relative to the bottom left of the window.*/
bool SceneView::projectWindowXYIntoObject(int x,int y,osg::Vec3& near_point,osg::Vec3& far_point) const
{
    bool result_near = _camera->unproject(Vec3(x,y,0.0f),_view,near_point);
    bool result_far =  _camera->unproject(Vec3(x,y,1.0f),_view,far_point);
    return result_near & result_far;
}


/** Calculate, via glProject, the object coordinates of a window.
    Note, current implementation requires that SceneView::draw() has been previously called
    for projectWindowIntoObject to produce valid values.  As per OpenGL
    windows coordinates are calculated relative to the bottom left of the window.*/
bool SceneView::projectObjectIntoWindow(const osg::Vec3& object,osg::Vec3& window) const
{
    return _camera->project(object,_view,window);
}
