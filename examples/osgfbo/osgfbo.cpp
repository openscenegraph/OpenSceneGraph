#include <osg/GL>
#include <osg/Group>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Viewport>
#include <osg/Texture2D>
#include <osg/MatrixTransform>

#include <osgProducer/Viewer>

#include <osgDB/ReadFile>

#include "FramebufferObject.h"
#include "FramebufferAttachment.h"
#include "Renderbuffer.h"
#include "FBOExtensions.h"

// This drawable class provides the means for clearing
// the color and depth buffer and it is used in this
// example to clear the offscreen buffers. Of course
// this is just a trick, but I couldn't think of any
// cleaner way to do that (ClearNode always affects
// the main buffers).
class ClearBuffer: public osg::Drawable
{
public:
    ClearBuffer() {}
    ClearBuffer(const ClearBuffer &copy, const osg::CopyOp &copyop = osg::CopyOp::SHALLOW_COPY): osg::Drawable(copy, copyop) {}
    META_Object(example, ClearBuffer);
    void drawImplementation(osg::State &state) const
    {
        glPushAttrib(GL_COLOR_BUFFER_BIT);
        glClearColor(0.5f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPopAttrib();
    }
};

// This function builds a textured quad
osg::Node *build_quad(osg::Texture2D *tex)
{
    osg::Geometry *geo = new osg::Geometry;
    osg::Vec3Array *vx = new osg::Vec3Array;
    vx->push_back(osg::Vec3(-10, 0, -10));
    vx->push_back(osg::Vec3(10, 0, -10));
    vx->push_back(osg::Vec3(10, 0, 10));
    vx->push_back(osg::Vec3(-10, 0, 10));
    geo->setVertexArray(vx);
    osg::Vec3Array *nx = new osg::Vec3Array;
    nx->push_back(osg::Vec3(0, -1, 0));
    geo->setNormalArray(nx);
    geo->setNormalBinding(osg::Geometry::BIND_OVERALL);
    osg::Vec2Array *tx = new osg::Vec2Array;
    tx->push_back(osg::Vec2(0, 0));
    tx->push_back(osg::Vec2(1, 0));
    tx->push_back(osg::Vec2(1, 1));
    tx->push_back(osg::Vec2(0, 1));
    geo->setTexCoordArray(0, tx);
    geo->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));
    geo->getOrCreateStateSet()->setTextureAttributeAndModes(0, tex);

    osg::Geode *geode = new osg::Geode;
    geode->addDrawable(geo);
    return geode;
}

// This class is provided because osg::Texture classes
// don't apply texture parameters when the image pointer
// is null. When performing render-to-texture, texture
// data must be applied as usual but a null pointer must
// be specified as pixel data.
// This is another trick: it would be better to allow
// null image pointers in osg::Texture classes.
struct MySubload: public osg::Texture2D::SubloadCallback
{
    void load(const osg::Texture2D &texture, osg::State &state) const
    {
        glTexImage2D(GL_TEXTURE_2D, 0, texture.getInternalFormat(), texture.getTextureWidth(), texture.getTextureHeight(), texture.getBorderWidth(), GL_RGBA, GL_UNSIGNED_BYTE, 0);
    }

    void subload(const osg::Texture2D &texture, osg::State &state) const
    {
    }
};

void build_world(osg::Group *root)
{
    int width = 512;
    int height = 512;

    // create and configure the texture that we're going
    // to use as target for render-to-texture
    osg::ref_ptr<osg::Texture2D> tex = new osg::Texture2D;
    tex->setTextureSize(width, height);
    tex->setInternalFormat(GL_RGBA);
    tex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
    tex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    tex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    tex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    tex->setSubloadCallback(new MySubload);

    // create and configure a framebuffer object.
    // We attach the texture to the first color buffer,
    // and we attach a simple offscreen image (Renderbuffer)
    // to the depth buffer in order to allow depth operations
    osg::ref_ptr<osg::FramebufferObject> fbo = new osg::FramebufferObject();
    fbo->setAttachment(GL_COLOR_ATTACHMENT0_EXT, osg::FramebufferAttachment(tex.get()));
    fbo->setAttachment(GL_DEPTH_ATTACHMENT_EXT, osg::FramebufferAttachment(new osg::Renderbuffer(width, height, GL_DEPTH_COMPONENT24)));

    // create a subgraph that will be rendered to texture.
    // We apply the previously created FBO and a Viewport
    // attribute to this subgraph.
    osg::ref_ptr<osg::MatrixTransform> offscreen = new osg::MatrixTransform;
    root->addChild(offscreen.get());
    offscreen->getOrCreateStateSet()->setRenderBinDetails(-1, "RenderBin");
    offscreen->getOrCreateStateSet()->setAttribute(fbo.get());
    offscreen->getOrCreateStateSet()->setAttribute(new osg::Viewport(0, 0, width, height));
    offscreen->setMatrix(osg::Matrix::lookAt(osg::Vec3(0, -20, 0), osg::Vec3(0, 0, 0), osg::Vec3(0, 0, 1)));
    offscreen->setReferenceFrame(osg::Transform::ABSOLUTE_RF);

    // add a ClearBuffer drawable to the offscreen subgraph
    // in order to clear the color and depth buffers
    osg::ref_ptr<osg::Geode> cbuf = new osg::Geode;
    cbuf->addDrawable(new ClearBuffer);
    cbuf->getOrCreateStateSet()->setRenderBinDetails(-2, "RenderBin");
    offscreen->addChild(cbuf.get());

    // add our beloved cow the offscreen subgraph
    offscreen->addChild(osgDB::readNodeFile("cow.osg"));

    // now create a simple quad that will be rendered
    // in the main framebuffers. The quad's texture
    // will be the content of the FBO's color buffer
    root->addChild(build_quad(tex.get()));
}

int main()
{
    osg::ref_ptr<osg::Group> root = new osg::Group;
    build_world(root.get());

    osgProducer::Viewer viewer;
    viewer.setUpViewer();
    viewer.setSceneData(root.get());
    viewer.realize();

    while (!viewer.done())
    {
        viewer.sync();
        viewer.update();
        viewer.frame();
    }

    viewer.sync();
    viewer.cleanup_frame();
    viewer.sync();
    return 0;
}
