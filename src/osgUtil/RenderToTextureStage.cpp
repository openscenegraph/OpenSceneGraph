#include <osgUtil/RenderToTextureStage>

#include <osgDB/ReadFile>

using namespace osg;
using namespace osgUtil;

// register a RenderToTextureStage prototype with the RenderBin prototype list.
//RegisterRenderBinProxy<RenderToTextureStage> s_registerRenderToTextureStageProxy;

RenderToTextureStage::RenderToTextureStage()
{
}

RenderToTextureStage::~RenderToTextureStage()
{
}

void RenderToTextureStage::reset()
{
    RenderStage::reset();
}

void RenderToTextureStage::draw(osg::State& state,RenderLeaf*& previous)
{

    RenderStage::draw(state,previous);

    // now copy the rendered image to attached texture.
    if (_texture.valid()) _texture->copyTexImage2D(state,_view[0],_view[1],_view[2],_view[3]);
    
}
