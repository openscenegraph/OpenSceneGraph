#include <osgUtil/RenderStageLighting>

using namespace osg;
using namespace osgUtil;

// register a RenderStageLighting prototype with the RenderBin prototype list.
//RegisterRenderBinProxy<RenderStageLighting> s_registerRenderStageLightingProxy;

RenderStageLighting::RenderStageLighting()
{
}

RenderStageLighting::~RenderStageLighting()
{
}

void RenderStageLighting::reset()
{
    _lightList.clear();
}

void RenderStageLighting::draw(osg::State& state,RenderLeaf*& previous)
{

    if (previous)
    {
        RenderGraph::moveToRootRenderGraph(state,previous->_parent);
        state.apply();
        if (previous->_matrix.valid()) glPopMatrix();
        previous = NULL;
    }

    Matrix* prev_matrix = NULL;

    // apply the light list.
    for(LightList::iterator litr=_lightList.begin();
        litr!=_lightList.end();
        ++litr)
    {
        Matrix* matrix = (*litr).second.get();
        if (matrix != prev_matrix)
        {
            if (prev_matrix) glPopMatrix();

            if (matrix)
            {
                glPushMatrix();
                glMultMatrixf((GLfloat*)matrix->_mat);
            }

            prev_matrix = matrix;
        }

        // apply the light source.
        litr->first->apply(state);
            
    }
    // clean up matrix stack.
    if (prev_matrix) glPopMatrix();

}
