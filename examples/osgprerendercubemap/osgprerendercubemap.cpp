#include <osgProducer/Viewer>

#include <osg/Projection>
#include <osg/Geometry>
#include <osg/Texture>
#include <osg/TexGen>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/PolygonOffset>
#include <osg/CullFace>
#include <osg/TextureCubeMap>
#include <osg/TexMat>
#include <osg/MatrixTransform>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/PolygonOffset>
#include <osg/CullFace>
#include <osg/Material>
#include <osg/PositionAttitudeTransform>

#include <osg/CameraNode>
#include <osg/TexGenNode>

using namespace osg;


ref_ptr<Group> _create_scene()
{
  ref_ptr<Group> scene = new Group;
  ref_ptr<Geode> geode_1 = new Geode;
  scene->addChild(geode_1.get());

  ref_ptr<Geode> geode_2 = new Geode;
  ref_ptr<MatrixTransform> transform_2 = new MatrixTransform;
  transform_2->addChild(geode_2.get());
  transform_2->setUpdateCallback(new osg::AnimationPathCallback(Vec3(0, 0, 0), Y_AXIS, inDegrees(45.0f)));
  scene->addChild(transform_2.get());

  ref_ptr<Geode> geode_3 = new Geode;
  ref_ptr<MatrixTransform> transform_3 = new MatrixTransform;
  transform_3->addChild(geode_3.get());
  transform_3->setUpdateCallback(new osg::AnimationPathCallback(Vec3(0, 0, 0), Y_AXIS, inDegrees(-22.5f)));
  scene->addChild(transform_3.get());

  const float radius = 0.8f;
  const float height = 1.0f;
  ref_ptr<TessellationHints> hints = new TessellationHints;
  hints->setDetailRatio(2.0f);
  ref_ptr<ShapeDrawable> shape;

  shape = new ShapeDrawable(new Box(Vec3(0.0f, -2.0f, 0.0f), 10, 0.1f, 10), hints.get());
  shape->setColor(Vec4(0.5f, 0.5f, 0.7f, 1.0f));
  geode_1->addDrawable(shape.get());


  shape = new ShapeDrawable(new Sphere(Vec3(-3.0f, 0.0f, 0.0f), radius), hints.get());
  shape->setColor(Vec4(0.6f, 0.8f, 0.8f, 1.0f));
  geode_2->addDrawable(shape.get());

  shape = new ShapeDrawable(new Box(Vec3(3.0f, 0.0f, 0.0f), 2 * radius), hints.get());
  shape->setColor(Vec4(0.4f, 0.9f, 0.3f, 1.0f));
  geode_2->addDrawable(shape.get());

  shape = new ShapeDrawable(new Cone(Vec3(0.0f, 0.0f, -3.0f), radius, height), hints.get());
  shape->setColor(Vec4(0.2f, 0.5f, 0.7f, 1.0f));
  geode_2->addDrawable(shape.get());

  shape = new ShapeDrawable(new Cylinder(Vec3(0.0f, 0.0f, 3.0f), radius, height), hints.get());
  shape->setColor(Vec4(1.0f, 0.3f, 0.3f, 1.0f));
  geode_2->addDrawable(shape.get());

  shape = new ShapeDrawable(new Box(Vec3(0.0f, 3.0f, 0.0f), 2, 0.1f, 2), hints.get());
  shape->setColor(Vec4(0.8f, 0.8f, 0.4f, 1.0f));
  geode_3->addDrawable(shape.get());

  // material
  ref_ptr<Material> matirial = new Material;
  matirial->setColorMode(Material::DIFFUSE);
  matirial->setAmbient(Material::FRONT_AND_BACK, Vec4(0, 0, 0, 1));
  matirial->setSpecular(Material::FRONT_AND_BACK, Vec4(1, 1, 1, 1));
  matirial->setShininess(Material::FRONT_AND_BACK, 64.0f);
  scene->getOrCreateStateSet()->setAttributeAndModes(matirial.get(), StateAttribute::ON);

  return scene;
}

osg::NodePath createReflector()
{
  osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
  pat->setPosition(osg::Vec3(0.0f,0.0f,0.0f));
  pat->setAttitude(osg::Quat(osg::inDegrees(0.0f),osg::Vec3(0.0f,0.0f,1.0f)));
  
  Geode* geode_1 = new Geode;
  pat->addChild(geode_1);

  const float radius = 0.8f;
  ref_ptr<TessellationHints> hints = new TessellationHints;
  hints->setDetailRatio(2.0f);
  ShapeDrawable* shape = new ShapeDrawable(new Sphere(Vec3(0.0f, 0.0f, 0.0f), radius * 1.5f), hints.get());
  shape->setColor(Vec4(0.8f, 0.8f, 0.8f, 1.0f));
  geode_1->addDrawable(shape);
  
  osg::NodePath nodeList;
  nodeList.push_back(pat);
  nodeList.push_back(geode_1);
  
  return nodeList;
}

class UpdateCameraAndTexGenCallback : public osg::NodeCallback
{
    public:
    
        typedef std::vector< osg::ref_ptr<osg::CameraNode> >  CameraList;

        UpdateCameraAndTexGenCallback(osg::NodePath& reflectorNodePath, CameraList& cameraNodes):
            _reflectorNodePath(reflectorNodePath),
            _cameraNodes(cameraNodes)
        {
        }
       
        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
            // first update subgraph to make sure objects are all moved into postion
            traverse(node,nv);

            // compute the position of the center of the reflector subgraph
            osg::Matrixd worldToLocal = osg::computeWorldToLocal(_reflectorNodePath);
            osg::BoundingSphere bs = _reflectorNodePath.back()->getBound();
            osg::Vec3 position = bs.center();

            typedef std::pair<osg::Vec3, osg::Vec3> ImageData;
            const ImageData id[] =
            {
                ImageData( osg::Vec3( 1,  0,  0), osg::Vec3( 0, -1,  0) ), // +X
                ImageData( osg::Vec3(-1,  0,  0), osg::Vec3( 0, -1,  0) ), // -X
                ImageData( osg::Vec3( 0,  1,  0), osg::Vec3( 0,  0,  1) ), // +Y
                ImageData( osg::Vec3( 0, -1,  0), osg::Vec3( 0,  0, -1) ), // -Y
                ImageData( osg::Vec3( 0,  0,  1), osg::Vec3( 0, -1,  0) ), // +Z
                ImageData( osg::Vec3( 0,  0, -1), osg::Vec3( 0, -1,  0) )  // -Z
            };

            for(unsigned int i=0; 
                i<6 && i<_cameraNodes.size();
                ++i)
            {
                osg::Matrix localOffset;
                localOffset.makeLookAt(position,position+id[i].first,id[i].second);
                
                osg::Matrix viewMatrix = worldToLocal*localOffset;
            
                _cameraNodes[i]->setReferenceFrame(osg::CameraNode::ABSOLUTE_RF);
                _cameraNodes[i]->setProjectionMatrixAsFrustum(-1.0,1.0,-1.0,1.0,1.0,10000.0);
                _cameraNodes[i]->setViewMatrix(viewMatrix);
            }
        }
        
    protected:
    
        virtual ~UpdateCameraAndTexGenCallback() {}
        
        osg::NodePath               _reflectorNodePath;        
        CameraList                  _cameraNodes;
};

class TexMatCullCallback : public osg::NodeCallback
{
    public:
    
        TexMatCullCallback(osg::TexMat* texmat):
            _texmat(texmat)
        {
        }
       
        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
            // first update subgraph to make sure objects are all moved into postion
            traverse(node,nv);
            
            osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
            if (cv)
            {
                osg::Quat quat = cv->getModelViewMatrix().getRotate();
                _texmat->setMatrix(osg::Matrix::rotate(quat.inverse()));
            }
        }
        
    protected:
    
        osg::ref_ptr<TexMat>    _texmat;
};


osg::Group* createShadowedScene(osg::Node* reflectedSubgraph, osg::NodePath reflectorNodePath, unsigned int unit, const osg::Vec4& clearColor, unsigned tex_width, unsigned tex_height, osg::CameraNode::RenderTargetImplementation renderImplementation)
{

    osg::Group* group = new osg::Group;
    
    osg::TextureCubeMap* texture = new osg::TextureCubeMap;
    texture->setTextureSize(tex_width, tex_height);

    texture->setInternalFormat(GL_RGB);
    texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
    texture->setFilter(osg::TextureCubeMap::MIN_FILTER,osg::TextureCubeMap::LINEAR);
    texture->setFilter(osg::TextureCubeMap::MAG_FILTER,osg::TextureCubeMap::LINEAR);
    
    // set up the render to texture cameras.
    UpdateCameraAndTexGenCallback::CameraList cameraNodes;
    for(unsigned int i=0; i<6; ++i)
    {
        // create the camera
        osg::CameraNode* camera = new osg::CameraNode;

        camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        camera->setClearColor(clearColor);

        // set viewport
        camera->setViewport(0,0,tex_width,tex_height);

        // set the camera to render before the main camera.
        camera->setRenderOrder(osg::CameraNode::PRE_RENDER);

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(renderImplementation);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::CameraNode::COLOR_BUFFER, texture, 0, i);

        // add subgraph to render
        camera->addChild(reflectedSubgraph);
        
        group->addChild(camera);
        
        cameraNodes.push_back(camera);
    }
   
    // create the texgen node to project the tex coords onto the subgraph
    osg::TexGenNode* texgenNode = new osg::TexGenNode;
    texgenNode->getTexGen()->setMode(osg::TexGen::REFLECTION_MAP);
    texgenNode->setTextureUnit(unit);
    group->addChild(texgenNode);

    // set the reflected subgraph so that it uses the texture and tex gen settings.    
    {
        osg::Node* reflectorNode = reflectorNodePath.front();
        group->addChild(reflectorNode);
                
        osg::StateSet* stateset = reflectorNode->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(unit,texture,osg::StateAttribute::ON);
        stateset->setTextureMode(unit,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
        stateset->setTextureMode(unit,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
        stateset->setTextureMode(unit,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);
        stateset->setTextureMode(unit,GL_TEXTURE_GEN_Q,osg::StateAttribute::ON);

        osg::TexMat* texmat = new osg::TexMat;
        stateset->setTextureAttributeAndModes(unit,texmat,osg::StateAttribute::ON);
        
        reflectorNode->setCullCallback(new TexMatCullCallback(texmat));
    }
    
    // add the reflector scene to draw just as normal
    group->addChild(reflectedSubgraph);
    
    // set an update callback to keep moving the camera and tex gen in the right direction.
    group->setUpdateCallback(new UpdateCameraAndTexGenCallback(reflectorNodePath, cameraNodes));

    return group;
}


int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    ArgumentParser arguments(&argc, argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName() + " is the example which demonstrates using of GL_ARB_shadow extension implemented in osg::Texture class");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName());
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help", "Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--fbo","Use Frame Buffer Object for render to texture, where supported.");
    arguments.getApplicationUsage()->addCommandLineOption("--fb","Use FrameBuffer for render to texture.");
    arguments.getApplicationUsage()->addCommandLineOption("--pbuffer","Use Pixel Buffer for render to texture, where supported.");
    arguments.getApplicationUsage()->addCommandLineOption("--window","Use a seperate Window for render to texture.");
    arguments.getApplicationUsage()->addCommandLineOption("--width","Set the width of the render to texture");
    arguments.getApplicationUsage()->addCommandLineOption("--height","Set the height of the render to texture");

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments. getApplicationUsage());

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }
    
    unsigned tex_width = 512;
    unsigned tex_height = 512;
    while (arguments.read("--width", tex_width)) {}
    while (arguments.read("--height", tex_height)) {}

    osg::CameraNode::RenderTargetImplementation renderImplementation = osg::CameraNode::FRAME_BUFFER_OBJECT;
    
    while (arguments.read("--fbo")) { renderImplementation = osg::CameraNode::FRAME_BUFFER_OBJECT; }
    while (arguments.read("--pbuffer")) { renderImplementation = osg::CameraNode::PIXEL_BUFFER; }
    while (arguments.read("--fb")) { renderImplementation = osg::CameraNode::FRAME_BUFFER; }
    while (arguments.read("--window")) { renderImplementation = osg::CameraNode::SEPERATE_WINDOW; }
    

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
      arguments.writeErrorMessages(std::cout);
      return 1;
    }

    ref_ptr<MatrixTransform> scene = new MatrixTransform;
    scene->setMatrix(osg::Matrix::rotate(osg::DegreesToRadians(125.0),1.0,0.0,0.0));

    ref_ptr<Group> reflectedSubgraph = _create_scene();    
    if (!reflectedSubgraph.valid()) return 1;

    ref_ptr<Group> reflectedScene = createShadowedScene(reflectedSubgraph.get(), createReflector(), 0, viewer.getClearColor(),
                                                        tex_width, tex_height, renderImplementation);

    scene->addChild(reflectedScene.get());

    viewer.setSceneData(scene.get());

    // create the windows and run the threads.
    viewer.realize();

    while (!viewer.done())
    {
      // wait for all cull and draw threads to complete.
      viewer.sync();

      // update the scene by traversing it with the the update visitor which will
      // call all node update callbacks and animations.
      viewer.update();
         
      // fire off the cull and draw traversals of the scene.
      viewer.frame();
    }
    
    // wait for all cull and draw threads to complete.
    viewer.sync();

    // run a clean up frame to delete all OpenGL objects.
    viewer.cleanup_frame();

    // wait for all the clean up frame to complete.
    viewer.sync();

    return 0;
}
