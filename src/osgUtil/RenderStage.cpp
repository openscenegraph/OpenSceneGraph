#include <osg/Notify>

#include <osgUtil/RenderStage>

using namespace osg;
using namespace osgUtil;

// register a RenderStage prototype with the RenderBin prototype list.
//RegisterRenderBinProxy<RenderStage> s_registerRenderStageProxy;

RenderStage::RenderStage()
{
    // point RenderBin's _stage to this to ensure that references to
    // stage don't go tempted away to any other stage.
    _stage = this;
    _stageDrawnThisFrame = false;

    _clearMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    _clearColor.set(0.0f,0.0f,0.0f,0.0f);
    _clearAccum.set(0.0f,0.0f,0.0f,0.0f);
    _clearDepth = 1.0;
    _clearStencil = 0;


}

RenderStage::~RenderStage()
{
}

void RenderStage::reset()
{
    _dependencyList.clear();
    _stageDrawnThisFrame = false;
    
    if (_renderStageLighting.valid()) _renderStageLighting->reset();

    RenderBin::reset();
}

void RenderStage::addToDependencyList(RenderStage* rs)
{
    if (rs) _dependencyList.push_back(rs);
}
void RenderStage::draw(osg::State& state,RenderLeaf*& previous)
{
    if (_stageDrawnThisFrame) return;
    
    if (!_viewport)
    {
        notify(FATAL) << "Error: cannot drawm stage due to undefined viewport."<<endl;
        return;
    }
    
    _stageDrawnThisFrame = true;

    for(DependencyList::iterator itr=_dependencyList.begin();
        itr!=_dependencyList.end();
        ++itr)
    {
        (*itr)->draw(state,previous);
    }
    
    // set up the back buffer.

#define USE_SISSOR_TEST
#ifdef USE_SISSOR_TEST
    glScissor( _viewport->x(), _viewport->y(), _viewport->width(), _viewport->height() );
    glEnable( GL_SCISSOR_TEST );
#endif

    _viewport->apply(state);

    // glEnable( GL_DEPTH_TEST );

    // set which color planes to operate on.
    if (_colorMask.valid()) _colorMask->apply(state);
    else glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

    if (_clearMask & GL_COLOR_BUFFER_BIT)
        glClearColor( _clearColor[0], _clearColor[1], _clearColor[2], _clearColor[3]);

    if (_clearMask & GL_DEPTH_BUFFER_BIT)
        glClearDepth( _clearDepth);

    if (_clearMask & GL_STENCIL_BUFFER_BIT)
        glClearStencil( _clearStencil);

    if (_clearMask & GL_ACCUM_BUFFER_BIT)
        glClearAccum( _clearAccum[0], _clearAccum[1], _clearAccum[2], _clearAccum[3]);


    glClear( _clearMask );

#ifdef USE_SISSOR_TEST
    glDisable( GL_SCISSOR_TEST );
#endif

    // pass the camera we're about to set up to state, so that
    // subsequent operatiosn know about it, such as for CLOD etc.
    state.setCamera(_camera.get());

    // set up projection
    const Matrix& projectionMat = _camera->getProjectionMatrix();
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glMultMatrixf(projectionMat.ptr());

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    Light* light = getLight();
    if (getLightingMode()==RenderStageLighting::HEADLIGHT && light)
    {
        light->apply(state);
    }

    // set up camera modelview.
    const Matrix& modelView = _camera->getModelViewMatrix();
    glMultMatrixf(modelView.ptr());
    

    if (getLightingMode()==RenderStageLighting::SKY_LIGHT && light)
    {
        light->apply(state);
    }

    // apply the lights.
    if (_renderStageLighting.valid()) _renderStageLighting->draw(state,previous);

    // draw the children and local.
    RenderBin::draw(state,previous);

    // now reset the state so its back in its default state.
    if (previous)
    {
        RenderGraph::moveToRootRenderGraph(state,previous->_parent);
        state.apply();
        if (previous->_matrix.valid()) glPopMatrix();
        previous = NULL;
    }

}
// Statistics features
bool RenderStage::getStats(Statistics* primStats)
{
    if (_renderStageLighting.valid()) primStats->addLight(_renderStageLighting->_lightList.size());
    return RenderBin::getStats(primStats);
}
