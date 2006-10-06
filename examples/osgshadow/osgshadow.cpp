#include <osgProducer/Viewer>

#include <osg/Projection>
#include <osg/Geometry>
#include <osg/Texture>
#include <osg/TexGen>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/PolygonOffset>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/PolygonOffset>
#include <osg/CullFace>
#include <osg/Material>
#include <osg/TexEnvCombine>
#include <osg/TexEnv>

#include <osg/CameraNode>
#include <osg/TexGenNode>

#include <osgDB/ReadFile>

using namespace osg;

class LightTransformCallback: public osg::NodeCallback
{

public:

  LightTransformCallback(float angular_velocity, float height, float radius):
    _angular_velocity(angular_velocity),
    _height(height),
    _radius(radius),
    _previous_traversal_number(-1),
    _previous_time(-1.0f),
    _angle(0)
  {
  }

  void operator()(Node* node, NodeVisitor* nv);

protected:
    
  float                                  _angular_velocity;
  float                                  _height;
  float                                  _radius;
  int                                    _previous_traversal_number;
  double                                 _previous_time;
  float                                  _angle;
};


void 
LightTransformCallback::operator()(Node* node, NodeVisitor* nv)
{
  MatrixTransform* transform = dynamic_cast<MatrixTransform*>(node);
  if (nv && transform)
    {
      const FrameStamp* fs = nv->getFrameStamp();
      if (!fs) return; // not frame stamp, no handle on the time so can't move.
        
      double new_time = fs->getReferenceTime();
      if (nv->getTraversalNumber() != _previous_traversal_number)
        {
          _angle += _angular_velocity * (new_time - _previous_time);

          Matrix matrix = Matrix::rotate(atan(_height / _radius), -X_AXIS) *
            Matrix::rotate(PI_2, Y_AXIS) *
            Matrix::translate(Vec3(_radius, 0, 0)) *
            Matrix::rotate(_angle, Y_AXIS) *
            Matrix::translate(Vec3(0, _height, 0));

          // update the specified transform
          transform->setMatrix(matrix);

          _previous_traversal_number = nv->getTraversalNumber();
        }

      _previous_time = new_time; 
    }

  // must call any nested node callbacks and continue subgraph traversal.
  traverse(node,nv);

}


ref_ptr<MatrixTransform> _create_lights()
{
  ref_ptr<MatrixTransform> transform_0 = new MatrixTransform;

  // create a spot light.
  ref_ptr<Light> light_0 = new Light;
  light_0->setLightNum(0);
  light_0->setPosition(Vec4(0, 0, 0, 1.0f));
  light_0->setAmbient(Vec4(0.0f, 0.0f, 0.0f, 1.0f));
  light_0->setDiffuse(Vec4(1.0f, 0.8f, 0.8f, 1.0f));
  light_0->setSpotCutoff(60.0f);
  light_0->setSpotExponent(2.0f);

  ref_ptr<LightSource> light_source_0 = new LightSource;    
  light_source_0->setLight(light_0.get());
  light_source_0->setLocalStateSetModes(StateAttribute::ON); 
  transform_0->setUpdateCallback(new LightTransformCallback(inDegrees(90.0f), 8, 5));
  transform_0->addChild(light_source_0.get());

  ref_ptr<Geode> geode = new Geode;

  ref_ptr<ShapeDrawable> shape;
  ref_ptr<TessellationHints> hints = new TessellationHints;
  hints->setDetailRatio(0.3f);
  shape = new ShapeDrawable(new Sphere(Vec3(0.0f, 0.0f, 0.0f), 0.15f), hints.get());
  shape->setColor(Vec4(1.0f, 0.5f, 0.5f, 1.0f));
  geode->addDrawable(shape.get());
  shape = new ShapeDrawable(new Cylinder(Vec3(0.0f, 0.0f, -0.4f), 0.05f, 0.8f), hints.get());
  shape->setColor(Vec4(1.0f, 0.5f, 0.5f, 1.0f));
  geode->addDrawable(shape.get());


  geode->getOrCreateStateSet()->setMode(GL_LIGHTING, StateAttribute::OFF);
  transform_0->addChild(geode.get());

  return transform_0;
}

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

  shape = new ShapeDrawable(new Sphere(Vec3(0.0f, 0.0f, 0.0f), radius * 2), hints.get());
  shape->setColor(Vec4(0.8f, 0.8f, 0.8f, 1.0f));
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


class UpdateCameraAndTexGenCallback : public osg::NodeCallback
{
    public:
    
        UpdateCameraAndTexGenCallback(osg::MatrixTransform* light_transform, osg::CameraNode* cameraNode, osg::TexGenNode* texgenNode):
            _light_transform(light_transform),
            _cameraNode(cameraNode),
            _texgenNode(texgenNode)
        {
        }
       
        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
            // first update subgraph to make sure objects are all moved into postion
            traverse(node,nv);
            
            // now compute the camera's view and projection matrix to point at the shadower (the camera's children)
            osg::BoundingSphere bs;
            for(unsigned int i=0; i<_cameraNode->getNumChildren(); ++i)
            {
                bs.expandBy(_cameraNode->getChild(i)->getBound());
            }
            
            if (!bs.valid())
            {
                osg::notify(osg::WARN) << "bb invalid"<<_cameraNode.get()<<std::endl;
                return;
            }
            
            osg::Vec3 position = _light_transform->getMatrix().getTrans();

            float centerDistance = (position-bs.center()).length();

            float znear = centerDistance-bs.radius();
            float zfar  = centerDistance+bs.radius();
            float zNearRatio = 0.001f;
            if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;

#if 0
            // hack to illustrate the precision problems of excessive gap between near far range.
            znear = 0.00001*zfar;
#endif
            float top   = (bs.radius()/centerDistance)*znear;
            float right = top;

            _cameraNode->setReferenceFrame(osg::CameraNode::ABSOLUTE_RF);
            _cameraNode->setProjectionMatrixAsFrustum(-right,right,-top,top,znear,zfar);
            _cameraNode->setViewMatrixAsLookAt(position,bs.center(),osg::Vec3(0.0f,1.0f,0.0f));

            // compute the matrix which takes a vertex from local coords into tex coords
            // will use this later to specify osg::TexGen..
            osg::Matrix MVPT = _cameraNode->getViewMatrix() * 
                               _cameraNode->getProjectionMatrix() *
                               osg::Matrix::translate(1.0,1.0,1.0) *
                               osg::Matrix::scale(0.5f,0.5f,0.5f);
                               
            _texgenNode->getTexGen()->setMode(osg::TexGen::EYE_LINEAR);
            _texgenNode->getTexGen()->setPlanesFromMatrix(MVPT);

        }
        
    protected:
    
        virtual ~UpdateCameraAndTexGenCallback() {}
        
        osg::ref_ptr<osg::MatrixTransform>  _light_transform;
        osg::ref_ptr<osg::CameraNode>       _cameraNode;
        osg::ref_ptr<osg::TexGenNode>       _texgenNode;

};

//////////////////////////////////////////////////////////////////
// fragment shader
//
char fragmentShaderSource_noBaseTexture[] = 
    "uniform sampler2DShadow shadowTexture; \n"
    "uniform vec2 ambientBias; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "    gl_FragColor = gl_Color * (ambientBias.x + shadow2DProj( shadowTexture, gl_TexCoord[0] ) * ambientBias.y); \n"
    "}\n";

//////////////////////////////////////////////////////////////////
// fragment shader
//
char fragmentShaderSource_withBaseTexture[] = 
    "uniform sampler2D baseTexture; \n"
    "uniform sampler2DShadow shadowTexture; \n"
    "uniform vec2 ambientBias; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "    vec4 color = gl_Color * texture2D( baseTexture, gl_TexCoord[0].xy ); \n"
    "    gl_FragColor = color * (ambientBias.x + shadow2DProj( shadowTexture, gl_TexCoord[1] ) * ambientBias.y); \n"
    "}\n";


osg::Group* createShadowedScene(osg::Node* shadowed,osg::MatrixTransform* light_transform, unsigned int unit)
{
    osg::Group* group = new osg::Group;
    
    unsigned int tex_width = 1024;
    unsigned int tex_height = 1024;
    
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setTextureSize(tex_width, tex_height);

    texture->setInternalFormat(GL_DEPTH_COMPONENT);
    texture->setShadowComparison(true);
    texture->setShadowTextureMode(Texture::LUMINANCE);
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    
    // set up the render to texture camera.
    {

        // create the camera
        osg::CameraNode* camera = new osg::CameraNode;

        camera->setClearMask(GL_DEPTH_BUFFER_BIT);
        camera->setClearColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
        camera->setComputeNearFarMode(osg::CameraNode::DO_NOT_COMPUTE_NEAR_FAR);

        // set viewport
        camera->setViewport(0,0,tex_width,tex_height);

        osg::StateSet*  _local_stateset = camera->getOrCreateStateSet();

        _local_stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);


        float factor = 0.0f;
        float units = 1.0f;

        ref_ptr<PolygonOffset> polygon_offset = new PolygonOffset;
        polygon_offset->setFactor(factor);
        polygon_offset->setUnits(units);
        _local_stateset->setAttribute(polygon_offset.get(), StateAttribute::ON | StateAttribute::OVERRIDE);
        _local_stateset->setMode(GL_POLYGON_OFFSET_FILL, StateAttribute::ON | StateAttribute::OVERRIDE);

        ref_ptr<CullFace> cull_face = new CullFace;
        cull_face->setMode(CullFace::FRONT);
        _local_stateset->setAttribute(cull_face.get(), StateAttribute::ON | StateAttribute::OVERRIDE);
        _local_stateset->setMode(GL_CULL_FACE, StateAttribute::ON | StateAttribute::OVERRIDE);


        // set the camera to render before the main camera.
        camera->setRenderOrder(osg::CameraNode::PRE_RENDER);

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(osg::CameraNode::FRAME_BUFFER_OBJECT);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::CameraNode::DEPTH_BUFFER, texture);

        // add subgraph to render
        camera->addChild(shadowed);
        
        group->addChild(camera);
        
        // create the texgen node to project the tex coords onto the subgraph
        osg::TexGenNode* texgenNode = new osg::TexGenNode;
        texgenNode->setTextureUnit(unit);
        group->addChild(texgenNode);

        // set an update callback to keep moving the camera and tex gen in the right direction.
        group->setUpdateCallback(new UpdateCameraAndTexGenCallback(light_transform, camera, texgenNode));
    }
   

    // set the shadowed subgraph so that it uses the texture and tex gen settings.    
    {
        osg::Group* shadowedGroup = new osg::Group;
        shadowedGroup->addChild(shadowed);
        group->addChild(shadowedGroup);
                
        osg::StateSet* stateset = shadowedGroup->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(unit,texture,osg::StateAttribute::ON);
        stateset->setTextureMode(unit,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
        stateset->setTextureMode(unit,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
        stateset->setTextureMode(unit,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);

        stateset->setTextureMode(unit,GL_TEXTURE_GEN_Q,osg::StateAttribute::ON);

        osg::Program* program = new osg::Program;
        stateset->setAttribute(program);

        if (unit==0)
        {
            osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource_noBaseTexture);
            program->addShader(fragment_shader);

            osg::Uniform* shadowTextureSampler = new osg::Uniform("shadowTexture",(int)unit);
            stateset->addUniform(shadowTextureSampler);
        }
        else
        {
            osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource_withBaseTexture);
            program->addShader(fragment_shader);

            osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
            stateset->addUniform(baseTextureSampler);

            osg::Uniform* shadowTextureSampler = new osg::Uniform("shadowTexture",(int)unit);
            stateset->addUniform(shadowTextureSampler);
        }
        
        osg::Uniform* ambientBias = new osg::Uniform("ambientBias",osg::Vec2(0.3f,1.2f));
        stateset->addUniform(ambientBias);

    }
    
    // add the shadower and shadowed.
    group->addChild(light_transform);
    
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
    arguments.getApplicationUsage()->addCommandLineOption("--with-base-texture", "Adde base texture to shadowed model.");
    arguments.getApplicationUsage()->addCommandLineOption("--no-base-texture", "Adde base texture to shadowed model.");

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments. getApplicationUsage());

    bool withBaseTexture = true;
    while(arguments.read("--with-base-texture")) { withBaseTexture = true; }
    while(arguments.read("--no-base-texture")) { withBaseTexture = false; }

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

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

    ref_ptr<Group> shadowed_scene = _create_scene();    
    if (!shadowed_scene.valid()) return 1;

    ref_ptr<MatrixTransform> light_transform = _create_lights();
    if (!light_transform.valid()) return 1;

    ref_ptr<Group> shadowedScene;
    
    
    if (withBaseTexture)
    {
        shadowed_scene->getOrCreateStateSet()->setTextureAttributeAndModes( 0, new osg::Texture2D(osgDB::readImageFile("Images/lz.rgb")), osg::StateAttribute::ON);
        shadowedScene = createShadowedScene(shadowed_scene.get(),light_transform.get(),1);
    }
    else
    {
        shadowedScene = createShadowedScene(shadowed_scene.get(),light_transform.get(),0);
    }
    
    scene->addChild(shadowedScene.get());

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
