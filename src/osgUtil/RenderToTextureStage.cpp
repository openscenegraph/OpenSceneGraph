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
    if (_texture.valid())
        _texture->copyTexImage2D(state,_viewport->x(),_viewport->y(),_viewport->width(),_viewport->height());
    
    if (_image.valid())
        _image->readPixels(_viewport->x(),_viewport->y(),_viewport->width(),_viewport->height(),GL_RGBA,GL_UNSIGNED_BYTE);

}
