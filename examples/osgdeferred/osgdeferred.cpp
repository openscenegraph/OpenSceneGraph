
/* OpenSceneGraph example, osgdeferred.
 *
 *  Original code by Michael Kapelko, published to osg example with permission
 *  OSG Deferred Shading ( https://bitbucket.org/kornerr/osg-deferred-shading )
 *  Shader cleanup, removal of osgFX EffectCompositor and exchange of textures
 *  done by Christian Buchner
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

#include "osgdeferred.h"

#include <osg/AnimationPath>
#include <osg/PolygonMode>
#include <osgDB/ReadFile>
#include <osgShadow/SoftShadowMap>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgUtil/TangentSpaceGenerator>

#ifdef OSG_LIBRARY_STATIC
// in case of a static build...
USE_OSGPLUGIN(osg2)
USE_OSGPLUGIN(png)
USE_OSGPLUGIN(jpeg)
USE_OSGPLUGIN(glsl)
USE_SERIALIZER_WRAPPER_LIBRARY(osg)
USE_GRAPHICSWINDOW()
#endif


osg::TextureRectangle *createFloatTextureRectangle(int textureSize)
{
    osg::ref_ptr<osg::TextureRectangle> tex2D = new osg::TextureRectangle;
    tex2D->setTextureSize(textureSize, textureSize);
    tex2D->setInternalFormat(GL_RGBA16F_ARB);
    tex2D->setSourceFormat(GL_RGBA);
    tex2D->setSourceType(GL_FLOAT);
    return tex2D.release();
}

osg::Camera *createHUDCamera(double left,
                             double right,
                             double bottom,
                             double top)
{
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);
    camera->setRenderOrder(osg::Camera::POST_RENDER);
    camera->setAllowEventFocus(false);
    camera->setProjectionMatrix(osg::Matrix::ortho2D(left, right, bottom, top));
    camera->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    return camera.release();
}

osg::ref_ptr<osg::LightSource> createLight(const osg::Vec3 &pos)
{
    osg::ref_ptr<osg::LightSource> light = new osg::LightSource;
    light->getLight()->setPosition(osg::Vec4(pos.x(), pos.y(), pos.z(), 1));
    light->getLight()->setAmbient(osg::Vec4(0.2, 0.2, 0.2, 1));
    light->getLight()->setDiffuse(osg::Vec4(0.8, 0.8, 0.8, 1));
    return light;
}

class CreateTangentSpace : public osg::NodeVisitor
{
public:
    CreateTangentSpace() : NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN), tsg(new osgUtil::TangentSpaceGenerator) {}
    virtual void apply(osg::Geode& geode)
    {
        for (unsigned int i = 0; i < geode.getNumDrawables(); ++i)
        {
            osg::Geometry *geo = dynamic_cast<osg::Geometry *>(geode.getDrawable(i));
            if (geo != NULL)
            {
                // assume the texture coordinate for normal maps is stored in unit #0
                tsg->generate(geo, 0);
                // pass2.vert expects the tangent array to be stored inside gl_MultiTexCoord1
                geo->setTexCoordArray(1, tsg->getTangentArray());
            }
        }
        traverse(geode);
    }
private:
    osg::ref_ptr<osgUtil::TangentSpaceGenerator> tsg;
};

Pipeline createPipelinePlainOSG(
        osg::ref_ptr<osg::Group> scene,
        osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene,
        const osg::Vec3 lightPos)
{
    Pipeline p;
    p.graph = new osg::Group;
    p.textureSize = 1024;

    // Pass 1 (shadow).
    p.pass1Shadows = createFloatTextureRectangle(p.textureSize);
    osg::ref_ptr<osg::Camera> pass1 =
        createRTTCamera(osg::Camera::COLOR_BUFFER, p.pass1Shadows);
    pass1->addChild(shadowedScene.get());

    // pass2 shades expects tangent vectors to be available as texcoord array for texture #1
    // we use osgUtil::TangentSpaceGenerator to generate these
    CreateTangentSpace cts;
    scene->accept(cts);

    // Pass 2 (positions, normals, colors).
    p.pass2Positions = createFloatTextureRectangle(p.textureSize);
    p.pass2Normals   = createFloatTextureRectangle(p.textureSize);
    p.pass2Colors    = createFloatTextureRectangle(p.textureSize);
    osg::ref_ptr<osg::Camera> pass2 =
        createRTTCamera(osg::Camera::COLOR_BUFFER0, p.pass2Positions);
    pass2->attach(osg::Camera::COLOR_BUFFER1, p.pass2Normals);
    pass2->attach(osg::Camera::COLOR_BUFFER2, p.pass2Colors);
    pass2->addChild(scene.get());
    osg::ref_ptr<osg::StateSet> ss = setShaderProgram(pass2, "shaders/pass2.vert", "shaders/pass2.frag");
    ss->setTextureAttributeAndModes(0, createTexture("Images/whitemetal_diffuse.jpg"));
    ss->setTextureAttributeAndModes(1, createTexture("Images/whitemetal_normal.jpg"));
    ss->addUniform(new osg::Uniform("diffMap", 0));
    ss->addUniform(new osg::Uniform("bumpMap", 1));
    ss->addUniform(new osg::Uniform("useBumpMap", 1));

    // Pass 3 (final).
    p.pass3Final = createFloatTextureRectangle(p.textureSize);
    osg::ref_ptr<osg::Camera> pass3 =
        createRTTCamera(osg::Camera::COLOR_BUFFER, p.pass3Final, true);
    ss = setShaderProgram(pass3, "shaders/pass3.vert", "shaders/pass3.frag");
    ss->setTextureAttributeAndModes(0, p.pass2Positions);
    ss->setTextureAttributeAndModes(1, p.pass2Normals);
    ss->setTextureAttributeAndModes(2, p.pass2Colors);
    ss->setTextureAttributeAndModes(3, p.pass1Shadows);
    ss->addUniform(new osg::Uniform("posMap",    0));
    ss->addUniform(new osg::Uniform("normalMap", 1));
    ss->addUniform(new osg::Uniform("colorMap",  2));
    ss->addUniform(new osg::Uniform("shadowMap", 3));
    // Light position.
    ss->addUniform(new osg::Uniform("lightPos", lightPos));
    // Graph.
    p.graph->addChild(pass1);
    p.graph->addChild(pass2);
    p.graph->addChild(pass3);
    return p;
}

osg::Camera *createRTTCamera(osg::Camera::BufferComponent buffer,
                             osg::Texture *tex,
                             bool isAbsolute)
{
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setClearColor(osg::Vec4());
    camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
    camera->setRenderOrder(osg::Camera::PRE_RENDER);
    if (tex)
    {
        tex->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
        tex->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
        camera->setViewport(0, 0, tex->getTextureWidth(), tex->getTextureHeight());
        camera->attach(buffer, tex);
    }
    if (isAbsolute)
    {
        camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
        camera->setProjectionMatrix(osg::Matrix::ortho2D(0.0, 1.0, 0.0, 1.0));
        camera->setViewMatrix(osg::Matrix::identity());
        camera->addChild(createScreenQuad(1.0f, 1.0f));
    }
    return camera.release();
}

osg::ref_ptr<osg::Group> createSceneRoom()
{
    // Room.
    osg::ref_ptr<osg::MatrixTransform> room = new osg::MatrixTransform;
    osg::ref_ptr<osg::Node> roomModel = osgDB::readRefNodeFile("simpleroom.osgt");
    room->addChild(roomModel);
    room->setMatrix(osg::Matrix::translate(0, 0, 1));
    // Torus.
    osg::ref_ptr<osg::MatrixTransform> torus = new osg::MatrixTransform;
    osg::ref_ptr<osg::Node> torusModel = osgDB::readRefNodeFile("torus.osgt");
    torus->addChild(torusModel);
    setAnimationPath(torus, osg::Vec3(0, 0, 15), 6, 16);
    // Torus2.
    osg::ref_ptr<osg::MatrixTransform> torus2 = new osg::MatrixTransform;
    torus2->addChild(torusModel);
    setAnimationPath(torus2, osg::Vec3(-20, 0, 10), 20, 0);
    // Torus3.
    osg::ref_ptr<osg::MatrixTransform> torus3 = new osg::MatrixTransform;
    torus3->addChild(torusModel);
    setAnimationPath(torus3, osg::Vec3(0, 0, 40), 3, 25);
    // Scene.
    osg::ref_ptr<osg::Group> scene = new osg::Group;
    scene->addChild(room);
    scene->addChild(torus);
    scene->addChild(torus2);
    scene->addChild(torus3);
    return scene;
}

osg::Geode *createScreenQuad(float width,
                             float height,
                             float scale,
                             osg::Vec3 corner)
{
    osg::Geometry* geom = osg::createTexturedQuadGeometry(
        corner,
        osg::Vec3(width, 0, 0),
        osg::Vec3(0, height, 0),
        0,
        0,
        scale,
        scale);
    osg::ref_ptr<osg::Geode> quad = new osg::Geode;
    quad->addDrawable(geom);
    int values = osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED;
    quad->getOrCreateStateSet()->setAttribute(
        new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK,
                             osg::PolygonMode::FILL),
        values);
    quad->getOrCreateStateSet()->setMode(GL_LIGHTING, values);
    return quad.release();
}

osg::Texture2D *createTexture(const std::string &fileName)
{
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage(osgDB::readRefImageFile(fileName));
    texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
    texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
    texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
    texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    texture->setMaxAnisotropy(16.0f);
    return texture.release();
}

osg::ref_ptr<osg::Camera> createTextureDisplayQuad(
    const osg::Vec3 &pos,
    osg::StateAttribute *tex,
    float scale,
    float width,
    float height)
{
    osg::ref_ptr<osg::Camera> hc = createHUDCamera();
    hc->addChild(createScreenQuad(width, height, scale, pos));
    hc->getOrCreateStateSet()->setTextureAttributeAndModes(0, tex);
    return hc;
}

void setAnimationPath(osg::ref_ptr<osg::MatrixTransform> node,
                      const osg::Vec3 &center,
                      float time,
                      float radius)
{
    // Create animation.
    osg::ref_ptr<osg::AnimationPath> path = new osg::AnimationPath;
    path->setLoopMode(osg::AnimationPath::LOOP);
    unsigned int numSamples = 32;
    float delta_yaw = 2.0f * osg::PI / (static_cast<float>(numSamples) - 1.0f);
    float delta_time = time / static_cast<float>(numSamples);
    for (unsigned int i = 0; i < numSamples; ++i)
    {
        float yaw = delta_yaw * static_cast<float>(i);
        osg::Vec3 pos(center.x() + sinf(yaw)*radius,
                      center.y() + cosf(yaw)*radius,
                      center.z());
        osg::Quat rot(-yaw, osg::Z_AXIS);
        path->insert(delta_time * static_cast<float>(i),
                     osg::AnimationPath::ControlPoint(pos, rot));
    }
    // Assign it.
    node->setUpdateCallback(new osg::AnimationPathCallback(path.get()));
}

osg::ref_ptr<osg::StateSet> setShaderProgram(osg::ref_ptr<osg::Camera> pass,
                                             const std::string& vert,
                                             const std::string& frag)
{
    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->addShader(osgDB::readRefShaderFile(vert));
    program->addShader(osgDB::readRefShaderFile(frag));
    osg::ref_ptr<osg::StateSet> ss = pass->getOrCreateStateSet();
    ss->setAttributeAndModes(
        program.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    return ss;
}

int main()
{
    // Useful declaration.
    osg::ref_ptr<osg::StateSet> ss;
    // Scene.
    osg::Vec3 lightPos(0, 0, 80);
    osg::ref_ptr<osg::Group> scene = createSceneRoom();
    osg::ref_ptr<osg::LightSource> light = createLight(lightPos);
    scene->addChild(light.get());
    // Shadowed scene.
    osg::ref_ptr<osgShadow::SoftShadowMap> shadowMap = new osgShadow::SoftShadowMap;
    shadowMap->setJitteringScale(16);
    shadowMap->addShader(osgDB::readRefShaderFile("shaders/pass1Shadow.frag"));
    shadowMap->setLight(light.get());
    osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
    shadowedScene->setShadowTechnique(shadowMap.get());
    shadowedScene->addChild(scene.get());
    Pipeline p = createPipelinePlainOSG(scene, shadowedScene, lightPos);
    // Quads to display 1 pass textures.
    osg::ref_ptr<osg::Camera> qTexN =
        createTextureDisplayQuad(osg::Vec3(0, 0.7, 0),
                                 p.pass2Normals,
                                 p.textureSize);
    osg::ref_ptr<osg::Camera> qTexP =
        createTextureDisplayQuad(osg::Vec3(0, 0.35, 0),
                                 p.pass2Positions,
                                 p.textureSize);
    osg::ref_ptr<osg::Camera> qTexC =
        createTextureDisplayQuad(osg::Vec3(0, 0, 0),
                                 p.pass2Colors,
                                 p.textureSize);
    // Qaud to display 2 pass shadow texture.
    osg::ref_ptr<osg::Camera> qTexS =
        createTextureDisplayQuad(osg::Vec3(0.7, 0.7, 0),
                                 p.pass1Shadows,
                                 p.textureSize);
    // Quad to display 3 pass final (screen) texture.
    osg::ref_ptr<osg::Camera> qTexFinal =
        createTextureDisplayQuad(osg::Vec3(0, 0, 0),
                                 p.pass3Final,
                                 p.textureSize,
                                 1,
                                 1);
    // Must be processed before the first pass takes
    // the result into pass1Shadows texture.
    p.graph->insertChild(0, shadowedScene.get());
    // Quads are displayed in order, so the biggest one (final) must be first,
    // otherwise other quads won't be visible.
    p.graph->addChild(qTexFinal.get());
    p.graph->addChild(qTexN.get());
    p.graph->addChild(qTexP.get());
    p.graph->addChild(qTexC.get());
    p.graph->addChild(qTexS.get());

    // Display everything.
    osgViewer::Viewer viewer;

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);

    // Make screenshots with 'c'.
    viewer.addEventHandler(
        new osgViewer::ScreenCaptureHandler(
            new osgViewer::ScreenCaptureHandler::WriteToFile(
                "screenshot",
                "png",
                osgViewer::ScreenCaptureHandler::WriteToFile::OVERWRITE)));

    viewer.getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
    viewer.setSceneData(p.graph.get());

    return viewer.run();
}

