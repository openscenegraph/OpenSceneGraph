#include <osg/GL>
#include <osg/Group>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Viewport>
#include <osg/Texture2D>
#include <osg/MatrixTransform>

#include <osgProducer/Viewer>

#include <osgDB/ReadFile>

#include <osg/FrameBufferObject>

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

void build_world(osg::Group *root)
{
    int width = 2048;
    int height = 2048;

    // create and configure the texture that we're going
    // to use as target for render-to-texture
    osg::ref_ptr<osg::Texture2D> tex = new osg::Texture2D;
    tex->setTextureSize(width, height);
    tex->setInternalFormat(GL_RGBA);
    tex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
    tex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    tex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    tex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

    // create and configure a framebuffer object.
    // We attach the texture to the first color buffer,
    // and we attach a simple offscreen image (RenderBuffer)
    // to the depth buffer in order to allow depth operations
    osg::ref_ptr<osg::FrameBufferObject> fbo = new osg::FrameBufferObject();
    fbo->setAttachment(GL_COLOR_ATTACHMENT0_EXT, osg::FrameBufferAttachment(tex.get()));
    fbo->setAttachment(GL_DEPTH_ATTACHMENT_EXT, osg::FrameBufferAttachment(new osg::RenderBuffer(width, height, GL_DEPTH_COMPONENT24)));

    osg::ref_ptr<osg::Node> subgraph = osgDB::readNodeFile("cow.osg");
    if (!subgraph) return;
    
    const osg::BoundingSphere& bs = subgraph->getBound();
    if (!bs.valid())
    {
        return;
    }

    osg::ref_ptr<osg::CameraNode> camera = new osg::CameraNode;
    camera->setClearColor(osg::Vec4(0.1f,0.1f,0.3f,1.0f));
    camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera->setViewport(0, 0, width, height);
    
    // set the camera to render before the main camera.
    camera->setRenderOrder(osg::CameraNode::PRE_RENDER);

    float znear = 1.0f*bs.radius();
    float zfar  = 3.0f*bs.radius();

    // 2:1 aspect ratio as per flag geomtry below.
    float proj_top   = 0.5f*znear;
    float proj_right = 0.5f*znear;

    znear *= 0.9f;
    zfar *= 1.1f;

    // set up projection.
    camera->setProjectionMatrixAsFrustum(-proj_right,proj_right,-proj_top,proj_top,znear,zfar);

    // set view
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrixAsLookAt(bs.center()+osg::Vec3(0.0f,2.0f,0.0f)*bs.radius(),bs.center(),osg::Vec3(0.0f,0.0f,1.0f));

    // tell the camera to use OpenGL frame buffer object where supported.
    camera->setRenderTargetImplmentation(osg::CameraNode::FRAME_BUFFER_OBJECT);
    
    // attach the texture and use it as the color buffer.
    camera->attach(osg::CameraNode::COLOR_BUFFER,tex.get());

    // attach the subgraph
    camera->addChild(subgraph.get());


    // attach the camera to the main scene graph.    
    root->addChild(camera.get());
    

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
