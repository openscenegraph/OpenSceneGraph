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
    glScissor( _view[0], _view[1], _view[2], _view[3] );
    glEnable( GL_SCISSOR_TEST );
#endif

    glViewport( _view[0], _view[1], _view[2], _view[3] );

    // glEnable( GL_DEPTH_TEST );

    if (_clearMask & GL_COLOR_BUFFER_BIT)
        glClearColor( _clearColor[0], _clearColor[1], _clearColor[2], _clearColor[3]);

    if (_clearMask & GL_DEPTH_BUFFER_BIT)
        glClearDepth( _clearDepth);

    if (_clearMask & GL_STENCIL_BUFFER_BIT)
        glClearStencil( _clearStencil);

    if (_clearMask & GL_ACCUM_BUFFER_BIT)
        glClearAccum( _clearAccum[0], _clearAccum[1], _clearAccum[2], _clearAccum[3]);

    // clear a color bit planes - osg::ColorMask could also be used here.
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

    glClear( _clearMask );

#ifdef USE_SISSOR_TEST
    glDisable( GL_SCISSOR_TEST );
#endif

    // set up projection
    const Matrix& projectionMat = _camera->getProjectionMatrix();
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glMultMatrixf((GLfloat*)projectionMat._mat);

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    Light* light = getLight();
    if (getLightingMode()==RenderStageLighting::HEADLIGHT && light)
    {
        light->apply(state);
    }

    // set up camera modelview.
    const Matrix& modelView = _camera->getModelViewMatrix();
    glMultMatrixf((GLfloat*)modelView._mat);

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
void RenderStage::getPrims(Statistics *primStats)
{
    if (_renderStageLighting.valid()) primStats->nlights+=_renderStageLighting->_lightList.size();
    RenderBin::getPrims(primStats);
}
