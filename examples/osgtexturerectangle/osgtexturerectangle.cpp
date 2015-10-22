/* OpenSceneGraph example, osgtexturerectangle.
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

/*
 * demonstrates usage of osg::TextureRectangle.
 *
 * Actually there isn't much difference to the rest of the osg::Texture*
 * bunch only this:
 * - texture coordinates for texture rectangles must be in image
 *   coordinates instead of normalized coordinates (0-1). So for a 500x250
 *   image the coordinates for the entire image would be
 *   0,250 0,0 500,0 500,250 instead of 0,1 0,0 1,0 1,1
 * - only the following wrap modes are supported (but not enforced)
 *   CLAMP, CLAMP_TO_EDGE, CLAMP_TO_BORDER
 * - a border is not supported
 * - mipmap is not supported
 */

#include <osg/Notify>
#include <osg/TextureRectangle>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/TexMat>

#include <osg/Group>
#include <osg/Projection>
#include <osg/MatrixTransform>
#include <osgText/Text>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>


/**********************************************************************
 *
 * Texture pan animation callback
 *
 **********************************************************************/

class TexturePanCallback : public osg::NodeCallback
{
public:
    TexturePanCallback(osg::TexMat* texmat,
                       double delay = 0.05) :
        _texmat(texmat),
        _phaseS(35.0f),
        _phaseT(18.0f),
        _phaseScale(5.0f),
        _delay(delay),
        _prevTime(0.0)
    {
    }

    virtual void operator()(osg::Node*, osg::NodeVisitor* nv)
    {
        if (!_texmat)
            return;

        if (nv->getFrameStamp()) {
            double currTime = nv->getFrameStamp()->getSimulationTime();
            if (currTime - _prevTime > _delay) {

                float rad = osg::DegreesToRadians(currTime);

                // zoom scale (0.2 - 1.0)
                float scale = sin(rad * _phaseScale) * 0.4f + 0.6f;
                float scaleR = 1.0f - scale;

                // calculate new texture coordinates
                float s, t;
                s = ((sin(rad * _phaseS) + 1) * 0.5f) * (scaleR);
                t = ((sin(rad * _phaseT) + 1) * 0.5f) * (scaleR);


                _texmat->setMatrix(osg::Matrix::translate(s,t,1.0)*osg::Matrix::scale(scale,scale,1.0));

                // record time
                _prevTime = currTime;
            }
        }
    }

private:
    osg::TexMat* _texmat;

    float _phaseS, _phaseT, _phaseScale;

    double _delay;
    double _prevTime;
};


osg::Node* createRectangle(osg::BoundingBox& bb,
                           const std::string& filename)
{
    osg::Vec3 top_left(bb.xMin(),bb.yMax(),bb.zMax());
    osg::Vec3 bottom_left(bb.xMin(),bb.yMax(),bb.zMin());
    osg::Vec3 bottom_right(bb.xMax(),bb.yMax(),bb.zMin());
    osg::Vec3 top_right(bb.xMax(),bb.yMax(),bb.zMax());

    // create geometry
    osg::Geometry* geom = new osg::Geometry;

    osg::Vec3Array* vertices = new osg::Vec3Array(4);
    (*vertices)[0] = top_left;
    (*vertices)[1] = bottom_left;
    (*vertices)[2] = bottom_right;
    (*vertices)[3] = top_right;
    geom->setVertexArray(vertices);

    osg::Vec2Array* texcoords = new osg::Vec2Array(4);
    (*texcoords)[0].set(0.0f, 0.0f);
    (*texcoords)[1].set(1.0f, 0.0f);
    (*texcoords)[2].set(1.0f, 1.0f);
    (*texcoords)[3].set(0.0f, 1.0f);
    geom->setTexCoordArray(0,texcoords);

    osg::Vec3Array* normals = new osg::Vec3Array(1);
    (*normals)[0].set(0.0f,-1.0f,0.0f);
    geom->setNormalArray(normals, osg::Array::BIND_OVERALL);

    osg::Vec4Array* colors = new osg::Vec4Array(1);
    (*colors)[0].set(1.0f,1.0f,1.0f,1.0f);
    geom->setColorArray(colors, osg::Array::BIND_OVERALL);

    geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));

    // disable display list so our modified tex coordinates show up
    geom->setUseDisplayList(false);

    // load image
    osg::ref_ptr<osg::Image> img = osgDB::readRefImageFile(filename);

    // setup texture
    osg::TextureRectangle* texture = new osg::TextureRectangle(img);

    osg::TexMat* texmat = new osg::TexMat;
    texmat->setScaleByTextureRectangleSize(true);

    // setup state
    osg::StateSet* state = geom->getOrCreateStateSet();
    state->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
    state->setTextureAttributeAndModes(0, texmat, osg::StateAttribute::ON);

    // turn off lighting
    state->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    // install 'update' callback
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geom);
    geode->setUpdateCallback(new TexturePanCallback(texmat));

    return geode;
}


osg::Geode* createText(const std::string& str,
                       const osg::Vec3& pos)
{
    static std::string font("fonts/arial.ttf");

    osg::Geode* geode = new osg::Geode;

    osgText::Text* text = new osgText::Text;
    geode->addDrawable(text);

    text->setFont(font);
    text->setPosition(pos);
    text->setText(str);

    return geode;
}


osg::Node* createHUD()
{
    osg::Group* group = new osg::Group;

    // turn off lighting and depth test
    osg::StateSet* state = group->getOrCreateStateSet();
    state->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    state->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

    // add text
    osg::Vec3 pos(120.0f, 800.0f, 0.0f);
    const osg::Vec3 delta(0.0f, -80.0f, 0.0f);

    const char* text[] = {
        "TextureRectangle Mini-HOWTO",
        "- essentially behaves like Texture2D, *except* that:",
        "- tex coords must be non-normalized (0..pixel) instead of (0..1),\nalternatively you can use osg::TexMat to scale normal non dimensional texcoords.",
        "- wrap modes must be CLAMP, CLAMP_TO_EDGE, or CLAMP_TO_BORDER\n  repeating wrap modes are not supported",
        "- filter modes must be NEAREST or LINEAR since\n  mipmaps are not supported",
        "- texture borders are not supported",
        "- defaults should be fine",
        NULL
    };
    const char** t = text;
    while (*t) {
        group->addChild(createText(*t++, pos));
        pos += delta;
    }

    // create HUD
    osg::MatrixTransform* modelview_abs = new osg::MatrixTransform;
    modelview_abs->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    modelview_abs->setMatrix(osg::Matrix::identity());
    modelview_abs->addChild(group);

    osg::Projection* projection = new osg::Projection;
    projection->setMatrix(osg::Matrix::ortho2D(0,1280,0,1024));
    projection->addChild(modelview_abs);

    return projection;
}


osg::Node* createModel(const std::string& filename)
{
    osg::Group* root = new osg::Group;

    if (filename != "X") {
        osg::BoundingBox bb(0.0f,0.0f,0.0f,1.0f,1.0f,1.0f);
        root->addChild(createRectangle(bb, filename)); // XXX
    }

    root->addChild(createHUD());

    return root;
}


int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // construct the viewer.
    osgViewer::Viewer viewer;

    // create a model from the images.
    osg::Node* rootNode = createModel((arguments.argc() > 1 ? arguments[1] : "Images/lz.rgb"));

    // add model to viewer.
    viewer.setSceneData(rootNode);

    return viewer.run();
}
