#include <osgUtil/SceneView>
#include <osgUtil/AppVisitor>
#include <osgUtil/DisplayListVisitor>

#include <osg/Notify>
#include <osg/Texture>
#include <osg/AlphaFunc>
#include <osg/TexEnv>
#include <osg/ColorMatrix>

#include <osg/GLU>

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
    
    _viewport = new Viewport;
    
    _initCalled = false;

}


SceneView::~SceneView()
{
}


void SceneView::setDefaults()
{
    _globalState = new osg::StateSet;

    _lightingMode=HEADLIGHT;
    _light = new osg::Light;
    _light->setAmbient(Vec4(0.00f,0.0f,0.00f,1.0f));
    _light->setDiffuse(Vec4(0.8f,0.8f,0.8f,1.0f));
    _light->setSpecular(Vec4(1.0f,1.0f,1.0f,1.0f));

    _state = new State;
    
    _camera = new Camera;

    _rendergraph = new RenderGraph;
    _renderStage = new RenderStage;


    DisplayListVisitor* dlv = new DisplayListVisitor();
    dlv->setState(_state.get());
    _initVisitor = dlv;

    _appVisitor = new AppVisitor;    

    _cullVisitor = new CullVisitor;

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

void SceneView::init()
{
    _initCalled = true;

    if (_sceneData.valid() && _initVisitor.valid())
    {
        _initVisitor->reset();
        _initVisitor->setFrameStamp(_frameStamp.get());
        
        if (_frameStamp.valid())
        {
             _initVisitor->setTraversalNumber(_frameStamp->getFrameNumber());
        }
        
        _sceneData->accept(*_initVisitor.get());
        
    } 
}

void SceneView::app()
{
    if (!_initCalled) init();

    if (_sceneData.valid() && _appVisitor.valid())
    { 
        _appVisitor->reset();

        _appVisitor->setFrameStamp(_frameStamp.get());

        // use the frame number for the traversal number.
        if (_frameStamp.valid())
        {
             _appVisitor->setTraversalNumber(_frameStamp->getFrameNumber());
        }
        
        _sceneData->accept(*_appVisitor.get());
    }
    
    
}

void SceneView::cull()
{

    if (!_sceneData || !_viewport->valid()) return;

    if (!_initCalled) init();

    _camera->adjustAspectRatio(_viewport->aspectRatio());
    
    
    _rendergraph->clean();

    _cullVisitor->reset();

    _cullVisitor->setFrameStamp(_frameStamp.get());

    // use the frame number for the traversal number.
    if (_frameStamp.valid())
    {
         _cullVisitor->setTraversalNumber(_frameStamp->getFrameNumber());
    }

    // comment out reset of rendergraph since clean is more efficient.
    //  _rendergraph->reset();

    // use clean of the rendergraph rather than reset, as it is able to
    // reuse the structure on the rendergraph in the next frame. This
    // achieves a certain amount of frame cohereancy of memory allocation.

    _cullVisitor->setLODBias(_lodBias);
    _cullVisitor->setCamera(*_camera);
    _cullVisitor->setViewport(_viewport.get());
    _cullVisitor->setEarthSky(NULL); // reset earth sky on each frame.

	// SandB
	//now make it compute "clipping directions" needed for detailed culling
	if(_cullVisitor->getDetailedCulling()) 
		_cullVisitor->calcClippingDirections();//only once pre frame

    _renderStage->reset();

    _renderStage->setViewport(_viewport.get());
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


    const osg::EarthSky* earthSky = _cullVisitor->getEarthSky();
    if (earthSky)
    {
        if (earthSky->getRequiresClear())
        {
            _renderStage->setClearColor(earthSky->getClearColor());
            _renderStage->setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            // really should set clear mask here, but what to? Need
            // to consider the stencil and accumulation buffers..
            // will defer to later.  Robert Osfield. October 2001.
        }
        else
        {
            // we have an earth sky implementation to do the work for use
            // so we don't need to clear.
            _renderStage->setClearMask(0);
        }
    }

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
            if(!_cullVisitor->getDetailedCulling())
			{
				float min_near_plane = _far_plane*0.0005f;
				if (_near_plane<min_near_plane) _near_plane=min_near_plane;
			}
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
    if (!_sceneData || !_viewport->valid()) return;

    if (!_state)
    {
        osg::notify(osg::WARN) << "Warning: no valid osgUtil::SceneView::_state"<< std::endl;
        osg::notify(osg::WARN) << "         creating a state automatically."<< std::endl;

        // note the constructor for osg::State will set ContextID to 0.
        _state = new osg::State;
    }
    // we in theory should be able to 
    _state->reset();
    
    _state->setFrameStamp(_frameStamp.get());
    _state->setVisualsSettings(_visualsSettings.get());

    // note, to support multi-pipe systems the deletion of OpenGL display list
    // and texture objects is deferred until the OpenGL context is the correct
    // context for when the object were originally created.  Here we know what
    // context we are in so can flush the appropriate caches.
    osg::Drawable::flushDeletedDisplayLists(_state->getContextID());
    osg::Texture::flushDeletedTextureObjects(_state->getContextID());

    RenderLeaf* previous = NULL;
    
    if (_visualsSettings.valid() && _visualsSettings->getStereo()) 
    {
    
        switch(_visualsSettings->getStereoMode())
        {
        case(osg::VisualsSettings::QUAD_BUFFER):
            {
                osg::ref_ptr<osg::Camera> left_camera = new osg::Camera(*_camera);
                osg::ref_ptr<osg::Camera> right_camera = new osg::Camera(*_camera);
                
                float iod = _visualsSettings->getEyeSeperation();
                float screenDistance = _visualsSettings->getEyeSeperation();

                left_camera->adjustEyeOffsetForStereo(osg::Vec3(-iod*0.5,0.0f,0.0f),screenDistance);
                right_camera->adjustEyeOffsetForStereo(osg::Vec3(iod*0.5,0.0f,0.0f),screenDistance);

                glDrawBuffer(GL_BACK_LEFT);
                _renderStage->setCamera(left_camera.get());
                _renderStage->draw(*_state,previous);


                glDrawBuffer(GL_BACK_RIGHT);
                _renderStage->setCamera(right_camera.get());
                _renderStage->_stageDrawnThisFrame = false;
                _renderStage->draw(*_state,previous);
            }
            break;
        case(osg::VisualsSettings::ANAGLYPHIC):
            {
                osg::ref_ptr<osg::Camera> left_camera = new osg::Camera(*_camera);
                osg::ref_ptr<osg::Camera> right_camera = new osg::Camera(*_camera);

                float iod = _visualsSettings->getEyeSeperation();
                float screenDistance = _visualsSettings->getScreenDistance();

                left_camera->adjustEyeOffsetForStereo(osg::Vec3(-iod*0.5,0.0f,0.0f),screenDistance);
                right_camera->adjustEyeOffsetForStereo(osg::Vec3(iod*0.5,0.0f,0.0f),screenDistance);
                

                osg::ColorMatrix* cm = new osg::ColorMatrix;
                cm->setMatrix(osg::Matrix(0.3f,0.3f,0.3f,0.0f,
                                          0.6f,0.6f,0.6f,0.0f,
                                          0.1f,0.1f,0.1f,0.0f,
                                          0.0f,0.0f,0.0f,1.0f));

                _globalState->setAttribute(cm);                                          

                osg::ColorMask* red = new osg::ColorMask;
                osg::ColorMask* green = new osg::ColorMask;

                red->setMask(true,false,false,true);
                _renderStage->setColorMask(red);
                _renderStage->setCamera(left_camera.get());
                _renderStage->draw(*_state,previous);

                green->setMask(false,true,true,true);
                _renderStage->setColorMask(green);
                _renderStage->_stageDrawnThisFrame = false;
                _renderStage->setCamera(right_camera.get());
                _renderStage->draw(*_state,previous);

            }
            break;
        default:
            {
                osg::notify(osg::NOTICE)<<"Warning: stereo camera mode not implemented yet."<< std::endl;
                _renderStage->draw(*_state,previous);
            }
            break;
        }
    }
    else
    {
        // bog standard draw.
        _renderStage->draw(*_state,previous);
    }
        
    GLenum errorNo = glGetError();
    if (errorNo!=GL_NO_ERROR)
    {
        osg::notify(WARN)<<"Warning: detected OpenGL error '"<<gluErrorString(errorNo)<<"'"<< std::endl;
    }
    
}


/** Calculate, via glUnProject, the object coordinates of a window point.
    Note, current implementation requires that SceneView::draw() has been previously called
    for projectWindowIntoObject to produce valid values.  As per OpenGL
    windows coordinates are calculated relative to the bottom left of the window.*/
bool SceneView::projectWindowIntoObject(const osg::Vec3& window,osg::Vec3& object) const
{
    return _camera->unproject(window,*_viewport,object);
}


/** Calculate, via glUnProject, the object coordinates of a window x,y
    when projected onto the near and far planes.
    Note, current implementation requires that SceneView::draw() has been previously called
    for projectWindowIntoObject to produce valid values.  As per OpenGL
    windows coordinates are calculated relative to the bottom left of the window.*/
bool SceneView::projectWindowXYIntoObject(int x,int y,osg::Vec3& near_point,osg::Vec3& far_point) const
{
    bool result_near = _camera->unproject(Vec3(x,y,0.0f),*_viewport,near_point);
    bool result_far =  _camera->unproject(Vec3(x,y,1.0f),*_viewport,far_point);
    return result_near & result_far;
}


/** Calculate, via glProject, the object coordinates of a window.
    Note, current implementation requires that SceneView::draw() has been previously called
    for projectWindowIntoObject to produce valid values.  As per OpenGL
    windows coordinates are calculated relative to the bottom left of the window.*/
bool SceneView::projectObjectIntoWindow(const osg::Vec3& object,osg::Vec3& window) const
{
    return _camera->project(object,*_viewport,window);
}
