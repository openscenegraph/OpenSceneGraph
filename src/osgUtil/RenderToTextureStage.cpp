/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/
#include <osgUtil/RenderToTextureStage>

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
    
    if (_stageDrawnThisFrame) return;

    //cout << "begining RTTS draw "<<this<< "  "<<_viewport->x()<<","<< _viewport->y()<<","<< _viewport->width()<<","<< _viewport->height()<<std::endl;

    RenderStage::draw(state,previous);

    // now copy the rendered image to attached texture.
    if (_texture.valid())
    {
        //cout << "        reading "<<this<< "  "<<_viewport->x()<<","<< _viewport->y()<<","<< _viewport->width()<<","<< _viewport->height()<<std::endl;

        _texture->copyTexImage2D(state,_viewport->x(),_viewport->y(),_viewport->width(),_viewport->height());
    }
    
    if (_image.valid())
        _image->readPixels(_viewport->x(),_viewport->y(),_viewport->width(),_viewport->height(),GL_RGBA,GL_UNSIGNED_BYTE);

}
