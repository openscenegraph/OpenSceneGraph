#include <osg/GLExtensions>
#include <osg/Node>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/Texture2D>
#include <osg/Stencil>
#include <osg/ColorMask>
#include <osg/Geode>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>


osg::Geode* createMask()
{
    osg::Vec3Array *vertices = new osg::Vec3Array;
    vertices->push_back(osg::Vec3(-0.5, -0.5, 0.0));
    vertices->push_back(osg::Vec3(0.5, -0.5, 0.0));
    vertices->push_back(osg::Vec3(0.5, 0.5, 0.0));
    vertices->push_back(osg::Vec3(-0.5, 0.5, 0.0));

    osg::Geometry *geom = new osg::Geometry;
    geom->setVertexArray(vertices);
    geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));

    osg::Geode *geode = new osg::Geode;
    geode->addDrawable(geom);

    osg::Stencil* stencil = new osg::Stencil;
    stencil->setFunction(osg::Stencil::ALWAYS, 1, ~0u);
    stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::REPLACE);

    osg::StateSet *ss = geode->getOrCreateStateSet();
    ss->setAttributeAndModes(stencil, 
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    ss->setAttribute(new osg::ColorMask(false, false, false, false), 
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    return geode;
}

osg::Geode* createGeometry()
{
    osg::Vec3Array* vertices = new osg::Vec3Array;
    vertices->push_back(osg::Vec3(-1.0, -1.0, 0.0));
    vertices->push_back(osg::Vec3(1.0, -1.0, 0.0));
    vertices->push_back(osg::Vec3(1.0, 1.0, 0.0));
    vertices->push_back(osg::Vec3(-1.0, 1.0, 0.0));

    osg::Geometry* geom = new osg::Geometry;
    geom->setVertexArray(vertices);
    geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geom);

    osg::Stencil* stencil = new osg::Stencil;
    stencil->setFunction(osg::Stencil::NOTEQUAL, 1, ~0u);
    stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);

    osg::StateSet *ss = geode->getOrCreateStateSet();
    ss->setAttributeAndModes(stencil,
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    return geode;
}

osg::Geode* createTextureQuad(osg::Texture2D *texture)
{
    osg::Vec3Array *vertices = new osg::Vec3Array;
    vertices->push_back(osg::Vec3(-0.8, 0.0, -0.8));
    vertices->push_back(osg::Vec3(0.8, 0.0, -0.8));
    vertices->push_back(osg::Vec3(0.8, 0.0, 0.8));
    vertices->push_back(osg::Vec3(-0.8, 0.0, 0.8));

    osg::Vec2Array *texcoord = new osg::Vec2Array;
    texcoord->push_back(osg::Vec2(0.0, 0.0));
    texcoord->push_back(osg::Vec2(1.0, 0.0));
    texcoord->push_back(osg::Vec2(1.0, 1.0));
    texcoord->push_back(osg::Vec2(0.0, 1.0));

    osg::Geometry *geom = new osg::Geometry;
    geom->setVertexArray(vertices);
    geom->setTexCoordArray(0, texcoord);
    geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));

    osg::Geode *geode = new osg::Geode;
    geode->addDrawable(geom);
    geode->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);

    return geode;
}


int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // construct the viewer.
    osgViewer::Viewer viewer(arguments);

    // add stats
    viewer.addEventHandler( new osgViewer::StatsHandler() );

    osg::Group* rootNode = new osg::Group;
    rootNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    // creates texture to be rendered
    osg::Texture2D *texture = new osg::Texture2D;
    texture->setTextureSize(1024, 1024);
    texture->setInternalFormat(GL_RGBA);
    texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
    texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    texture->setBorderColor(osg::Vec4(0, 0, 0, 0));

    // creates rtt camera
    osg::ref_ptr<osg::Camera> rttCamera = new osg::Camera;
    rttCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    rttCamera->setClearColor(osg::Vec4(0.0, 0.4, 0.5, 0.0));
    rttCamera->setClearStencil(0);
    rttCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    rttCamera->setViewMatrix(osg::Matrixd::identity());
    rttCamera->setViewport(0, 0, 1024, 1024);
    rttCamera->setRenderOrder(osg::Camera::PRE_RENDER);
    rttCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
    
#define USE_PACKED_DEPTH_STENCIL

#ifdef USE_PACKED_DEPTH_STENCIL
    rttCamera->attach(osg::Camera::PACKED_DEPTH_STENCIL_BUFFER, GL_DEPTH_STENCIL_EXT);
#else
    // this doesn't work on NVIDIA/Vista 64bit
    // FBO status = 0x8cd6 (FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT)
    rttCamera->attach(osg::Camera::DEPTH_BUFFER, GL_DEPTH_COMPONENT);
    rttCamera->attach(osg::Camera::STENCIL_BUFFER, GL_STENCIL_INDEX_EXT);
#endif
    rttCamera->attach(osg::Camera::COLOR_BUFFER, texture);
    rttCamera->setCullingMode(osg::Camera::VIEW_FRUSTUM_SIDES_CULLING);

    // creates rtt subtree
    osg::Group* g0 = new osg::Group;
    g0->addChild(createMask());
    g0->addChild(createGeometry());
    rttCamera->addChild(g0);
    rootNode->addChild(rttCamera);

    // creates textured quad with result
    rootNode->addChild(createTextureQuad(texture));
    
    // add model to the viewer.
    viewer.setSceneData( rootNode );

    return viewer.run();
}
