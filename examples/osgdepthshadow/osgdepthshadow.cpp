#include <osgProducer/Viewer>

#include <osg/Projection>
#include <osg/Geometry>
#include <osg/Texture>
#include <osg/TexGen>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/PolygonOffset>
#include <osg/CullFace>
#include <osg/TexEnvCombine>
#include <osg/MatrixTransform>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/PolygonOffset>
#include <osg/CullFace>
#include <osg/Material>

#include <osgUtil/TransformCallback>
#include <osgUtil/RenderToTextureStage>

using namespace osg;

const int depth_texture_height = 512;
const int depth_texture_width  = 512;

ref_ptr<RefMatrix> bias = new RefMatrix(0.5f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 0.5f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 0.5f, 0.0f,
                                        0.5f, 0.5f, 0.5f, 1.0f);

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

class RenderToTextureCallback: public NodeCallback
{
public:
  RenderToTextureCallback(Node* subgraph, 
                          Texture2D* texture, 
                          MatrixTransform* light_transform,
                          TexGen* tex_gen):
    _subgraph(subgraph),
    _texture(texture),
    _local_stateset(new StateSet),
    _viewport(new Viewport),
    _light_projection(new RefMatrix),
    _light_transform(light_transform),
    _tex_gen(tex_gen)
  {
    _local_stateset->setAttribute(_viewport.get());
    _local_stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
   
    ref_ptr<PolygonOffset> polygon_offset = new PolygonOffset;
    polygon_offset->setFactor(1.1f);
    polygon_offset->setUnits(4.0f);
    _local_stateset->setAttribute(polygon_offset.get(), StateAttribute::ON | StateAttribute::OVERRIDE);
    _local_stateset->setMode(GL_POLYGON_OFFSET_FILL, StateAttribute::ON | StateAttribute::OVERRIDE);

    ref_ptr<CullFace> cull_face = new CullFace;
    cull_face->setMode(CullFace::FRONT);
    _local_stateset->setAttribute(cull_face.get(), StateAttribute::ON | StateAttribute::OVERRIDE);
    _local_stateset->setMode(GL_CULL_FACE, StateAttribute::ON | StateAttribute::OVERRIDE);

    _viewport->setViewport(0, 0, depth_texture_width, depth_texture_height);

    float znear = 1.0f * _subgraph->getBound().radius();
    float zfar  = 3.0f * _subgraph->getBound().radius();
    float top   = 0.5f * _subgraph->getBound().radius();
    float right = 0.5f * _subgraph->getBound().radius();
    znear *= 0.8f;
    zfar *= 1.2f;
    _light_projection->makeFrustum(-right, right, -top, top, znear, zfar);
  }

  virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
  {

     osgUtil::CullVisitor* cullVisitor = dynamic_cast<osgUtil::CullVisitor*>(nv);
     if (cullVisitor && _texture.valid() && _subgraph.valid())
      {            
        _request_render_to_depth_texture(*node, *cullVisitor);
      }

    // must traverse the subgraph            
    traverse(node,nv);
  }
        
  void _request_render_to_depth_texture(osg::Node& node, osgUtil::CullVisitor& cv);
        
  ref_ptr<osg::Node>                     _subgraph;
  ref_ptr<osg::Texture2D>                _texture;
  ref_ptr<osg::StateSet>                 _local_stateset;
  ref_ptr<osg::Viewport>                 _viewport;
  ref_ptr<RefMatrix>                     _light_projection;
  ref_ptr<MatrixTransform>               _light_transform;
  ref_ptr<TexGen>                        _tex_gen;
};

void RenderToTextureCallback::_request_render_to_depth_texture(osg::Node&, osgUtil::CullVisitor& cv)
{   
  // create the render to texture stage.
  osg::ref_ptr<osgUtil::RenderToTextureStage> rtts = new osgUtil::RenderToTextureStage;

  // set up lighting.
  // currently ignore lights in the scene graph itself..
  // will do later.
  osgUtil::RenderStage* previous_stage = cv.getCurrentRenderBin()->getStage();

  // set up the background color and clear mask.
  rtts->setClearMask(GL_DEPTH_BUFFER_BIT);
  rtts->setColorMask(new ColorMask(false, false, false, false));

  // set up to charge the same RenderStageLighting is the parent previous stage.
  rtts->setRenderStageLighting(previous_stage->getRenderStageLighting());


  // record the render bin, to be restored after creation
  // of the render to text
  osgUtil::RenderBin* previousRenderBin = cv.getCurrentRenderBin();

  osgUtil::CullVisitor::ComputeNearFarMode saved_compute_near_far_mode = cv.getComputeNearFarMode();
  cv.setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR);

  // set the current renderbin to be the newly created stage.
  cv.setCurrentRenderBin(rtts.get());

  ref_ptr<RefMatrix> light_view = new RefMatrix;
  light_view->makeLookAt(_light_transform->getMatrix().getTrans(), Vec3(0, 0, 0), Z_AXIS);
  Matrix texture_matrix = (*light_view.get()) * (*_light_projection.get()) * (*bias.get());
  _tex_gen->setPlane(TexGen::S, Vec4(texture_matrix(0, 0), 
                                     texture_matrix(1, 0), 
                                     texture_matrix(2, 0), 
                                     texture_matrix(3, 0))); 
  _tex_gen->setPlane(TexGen::T, Vec4(texture_matrix(0, 1), 
                                     texture_matrix(1, 1), 
                                     texture_matrix(2, 1), 
                                     texture_matrix(3, 1))); 
  _tex_gen->setPlane(TexGen::R, Vec4(texture_matrix(0, 2), 
                                     texture_matrix(1, 2), 
                                     texture_matrix(2, 2), 
                                     texture_matrix(3, 2))); 
  _tex_gen->setPlane(TexGen::Q, Vec4(texture_matrix(0, 3), 
                                     texture_matrix(1, 3), 
                                     texture_matrix(2, 3), 
                                     texture_matrix(3, 3))); 

  cv.pushProjectionMatrix(_light_projection.get());
  cv.pushModelViewMatrix(light_view.get());
  cv.pushStateSet(_local_stateset.get());

  // traverse the subgraph
  _subgraph->accept(cv);

  cv.popStateSet();
  cv.popModelViewMatrix();
  cv.popProjectionMatrix();

  cv.setComputeNearFarMode(saved_compute_near_far_mode);

  // restore the previous renderbin.
  cv.setCurrentRenderBin(previousRenderBin);

  if (rtts->getRenderGraphList().size()==0 && rtts->getRenderBinList().size()==0)
    {
      // getting to this point means that all the subgraph has been
      // culled by small feature culling or is beyond LOD ranges.
      return;
    }

  rtts->setViewport(_viewport.get());
    
  // and the render to texture stage to the current stages
  // dependancy list.
  cv.getCurrentRenderBin()->getStage()->addToDependencyList(rtts.get());

  // if one exist attach texture to the RenderToTextureStage.
  rtts->setTexture(_texture.get());
}

ref_ptr<MatrixTransform> _create_lights(ref_ptr<StateSet> root_stateset)
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

  light_source_0->setStateSetModes(*root_stateset.get(), StateAttribute::ON);
  
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
  transform_2->setUpdateCallback(new osgUtil::TransformCallback(Vec3(0, 0, 0), Y_AXIS, inDegrees(45.0f)));
  scene->addChild(transform_2.get());

  ref_ptr<Geode> geode_3 = new Geode;
  ref_ptr<MatrixTransform> transform_3 = new MatrixTransform;
  transform_3->addChild(geode_3.get());
  transform_3->setUpdateCallback(new osgUtil::TransformCallback(Vec3(0, 0, 0), Y_AXIS, inDegrees(-22.5f)));
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

int main(int argc, char** argv)
{
  // use an ArgumentParser object to manage the program arguments.
  ArgumentParser arguments(&argc, argv);

  // set up the usage document, in case we need to print out how to use this program.
  arguments.getApplicationUsage()->setDescription(arguments.getApplicationName() + " is the example which demonstrates using of GL_ARB_shadow extension implemented in osg::Texture class");
  arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName());
  arguments.getApplicationUsage()->addCommandLineOption("-h or --help", "Display this information");
   
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
  scene->addChild(shadowed_scene.get());

  ref_ptr<MatrixTransform> light_transform = _create_lights(scene->getOrCreateStateSet());
  if (!scene.valid()) return 1;
  scene->addChild(light_transform.get());

  ref_ptr<Texture2D> texture = new Texture2D;
  texture->setInternalFormat(GL_DEPTH_COMPONENT);
  texture->setShadowComparison(true);
  texture->setShadowTextureMode(Texture::LUMINANCE);

  ref_ptr<TexGen> tex_gen = new TexGen;
  tex_gen->setMode(TexGen::EYE_LINEAR);
  shadowed_scene->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture.get(), StateAttribute::ON);
  shadowed_scene->getOrCreateStateSet()->setTextureAttributeAndModes(0, tex_gen.get(), StateAttribute::ON);

  scene->setCullCallback(new RenderToTextureCallback(shadowed_scene.get(), texture.get(), light_transform.get(), tex_gen.get()));

  // add model to viewer.
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
    
  // wait for all cull and draw threads to complete before exit.
  viewer.sync();

  return 0;
}
