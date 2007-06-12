/* OpenSceneGraph example, osgfxbrowser.
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

#include "Frame.h"

#include <osgText/Text>

namespace osgfxbrowser {

Frame::Frame()
:    osg::Geode(), 
    bgcolor_(0.5f, 0.5f, 0.5f, 1.0f),
    rect_(0, 0, 100, 100),
    caption_("Frame")
{
}

Frame::Frame(const Frame &copy, const osg::CopyOp &copyop)
:    osg::Geode(copy, copyop),
    bgcolor_(copy.bgcolor_),
    rect_(copy.rect_),
    caption_(copy.caption_)
{
}

void Frame::rebuild()
{
    float zPos = -0.1f;

    removeDrawables(0, getNumDrawables());
    addDrawable(build_quad(rect_, bgcolor_));
    addDrawable(build_quad(Rect(rect_.x0 + 4, rect_.y1 - 24, rect_.x1 - 4, rect_.y1 - 4), osg::Vec4(0, 0, 0, bgcolor_.w()), false, zPos));

    osg::ref_ptr<osgText::Text> caption_text = new osgText::Text;
    caption_text->setText(caption_);
    caption_text->setColor(osg::Vec4(1, 1, 1, 1));
    caption_text->setAlignment(osgText::Text::CENTER_CENTER);
    caption_text->setFont("fonts/arial.ttf");
    caption_text->setCharacterSize(16);
    caption_text->setFontResolution(16, 16);
    caption_text->setPosition(osg::Vec3((rect_.x0 + rect_.x1) / 2, rect_.y1 - 15, zPos*2.0f));
    addDrawable(caption_text.get());

    rebuild_client_area(Rect(rect_.x0 + 4, rect_.y0 + 4, rect_.x1 - 4, rect_.y1 - 28));
}

osg::Geometry *Frame::build_quad(const Rect &rect, const osg::Vec4 &color, bool shadow, float z)
{
    const float shadow_space = 8;
    const float shadow_size = 10;

    osg::ref_ptr<osg::Geometry> geo = new osg::Geometry;
    osg::ref_ptr<osg::Vec3Array> vx = new osg::Vec3Array;

    vx->push_back(osg::Vec3(rect.x0, rect.y0, z));
    vx->push_back(osg::Vec3(rect.x1, rect.y0, z));
    vx->push_back(osg::Vec3(rect.x1, rect.y1, z));
    vx->push_back(osg::Vec3(rect.x0, rect.y1, z));

    if (shadow) {
        vx->push_back(osg::Vec3(rect.x0+shadow_space, rect.y0-shadow_size, z));
        vx->push_back(osg::Vec3(rect.x1+shadow_size, rect.y0-shadow_size, z));
        vx->push_back(osg::Vec3(rect.x1, rect.y0, z));
        vx->push_back(osg::Vec3(rect.x0+shadow_space, rect.y0, z));

        vx->push_back(osg::Vec3(rect.x1, rect.y1-shadow_space, z));
        vx->push_back(osg::Vec3(rect.x1, rect.y0, z));
        vx->push_back(osg::Vec3(rect.x1+shadow_size, rect.y0-shadow_size, z));
        vx->push_back(osg::Vec3(rect.x1+shadow_size, rect.y1-shadow_space, z));
    }

    geo->setVertexArray(vx.get());

    osg::ref_ptr<osg::Vec4Array> clr = new osg::Vec4Array;
    clr->push_back(color);
    clr->push_back(color);
    clr->push_back(color);
    clr->push_back(color);

    if (shadow) {

        float alpha = color.w() * 0.5f;
        const osg::Vec3 black(0, 0, 0);

        clr->push_back(osg::Vec4(black, 0));
        clr->push_back(osg::Vec4(black, 0));
        clr->push_back(osg::Vec4(black, alpha));
        clr->push_back(osg::Vec4(black, alpha));

        clr->push_back(osg::Vec4(black, alpha));
        clr->push_back(osg::Vec4(black, alpha));
        clr->push_back(osg::Vec4(black, 0));
        clr->push_back(osg::Vec4(black, 0));
    }

    geo->setColorArray(clr.get());
    geo->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

    geo->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, shadow? 12: 4));

    return geo.release();
}

}
