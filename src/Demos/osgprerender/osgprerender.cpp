#include <osg/GLExtensions>
#include <osg/Node>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osg/Stencil>
#include <osg/ColorMask>
#include <osg/Depth>
#include <osg/Billboard>
#include <osg/Material>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgUtil/TransformCallback>
#include <osgUtil/RenderToTextureStage>
#include <osgUtil/SmoothingVisitor>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGLUT/Viewer>

class MyUpdateCallback : public osg::NodeCallback
{
    public:
    
        MyUpdateCallback(osg::Node* subgraph):
            _subgraph(subgraph) {}

        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
            // traverse the subgraph to update any nodes.
            if (_subgraph.valid()) _subgraph->accept(*nv);
        
            // must traverse the Node's subgraph            
            traverse(node,nv);
        }
        
        osg::ref_ptr<osg::Node>     _subgraph;
};

class MyCullCallback : public osg::NodeCallback
{
    public:
    
        MyCullCallback(osg::Node* subgraph,osg::Texture2D* texture):
            _subgraph(subgraph),
            _texture(texture),
            _localState(new osg::StateSet) {}

        MyCullCallback(osg::Node* subgraph,osg::Image* image):
            _subgraph(subgraph),
            _image(image),
            _localState(new osg::StateSet) {}
        
        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {

            osgUtil::CullVisitor* cullVisitor = dynamic_cast<osgUtil::CullVisitor*>(nv);
            if (cullVisitor && (_texture.valid()|| _image.valid()) && _subgraph.valid())
            {            
                doPreRender(*node,*cullVisitor);

                // must traverse the subgraph            
                traverse(node,nv);

            }
            else
            {
                // must traverse the subgraph            
                traverse(node,nv);
            }
        }
        
        void doPreRender(osg::Node& node, osgUtil::CullVisitor& cv);
        
        osg::ref_ptr<osg::Node>      _subgraph;
        osg::ref_ptr<osg::Texture2D> _texture;
        osg::ref_ptr<osg::Image>     _image;
        osg::ref_ptr<osg::StateSet>  _localState;
    
};

void MyCullCallback::doPreRender(osg::Node&, osgUtil::CullVisitor& cv)
{   
    const osg::BoundingSphere& bs = _subgraph->getBound();
    if (!bs.valid())
    {
        osg::notify(osg::WARN) << "bb invalid"<<_subgraph.get()<<std::endl;
        return;
    }
    

    // create the render to texture stage.
    osg::ref_ptr<osgUtil::RenderToTextureStage> rtts = new osgUtil::RenderToTextureStage;

    // set up lighting.
    // currently ignore lights in the scene graph itself..
    // will do later.
    osgUtil::RenderStage* previous_stage = cv.getCurrentRenderBin()->_stage;

    // set up the background color and clear mask.
    rtts->setClearColor(osg::Vec4(0.1f,0.1f,0.3f,0.0f));
    rtts->setClearMask(previous_stage->getClearMask());

    // set up to charge the same RenderStageLighting is the parent previous stage.
    rtts->setRenderStageLighting(previous_stage->getRenderStageLighting());


    // record the render bin, to be restored after creation
    // of the render to text
    osgUtil::RenderBin* previousRenderBin = cv.getCurrentRenderBin();

    // set the current renderbin to be the newly created stage.
    cv.setCurrentRenderBin(rtts.get());


    float znear = 1.0f*bs.radius();
    float zfar  = 3.0f*bs.radius();
        
    // 2:1 aspect ratio as per flag geomtry below.
    float top   = 0.25f*znear;
    float right = 0.5f*znear;

    znear *= 0.9f;
    zfar *= 1.1f;

    // set up projection.
    osg::RefMatrix* projection = new osg::RefMatrix;
    projection->makeFrustum(-right,right,-top,top,znear,zfar);

    cv.pushProjectionMatrix(projection);

    osg::RefMatrix* matrix = new osg::RefMatrix;
    matrix->makeLookAt(bs.center()+osg::Vec3(0.0f,2.0f,0.0f)*bs.radius(),bs.center(),osg::Vec3(0.0f,0.0f,1.0f));

    cv.pushModelViewMatrix(matrix);

    cv.pushStateSet(_localState.get());

    {

        // traverse the subgraph
        _subgraph->accept(cv);

    }

    cv.popStateSet();

    // restore the previous model view matrix.
    cv.popModelViewMatrix();

    // restore the previous model view matrix.
    cv.popProjectionMatrix();

    // restore the previous renderbin.
    cv.setCurrentRenderBin(previousRenderBin);

    if (rtts->_renderGraphList.size()==0 && rtts->_bins.size()==0)
    {
        // getting to this point means that all the subgraph has been
        // culled by small feature culling or is beyond LOD ranges.
        return;
    }



    int height = 256;
    int width  = 512;


    const osg::Viewport& viewport = *cv.getViewport();

    // offset the impostor viewport from the center of the main window
    // viewport as often the edges of the viewport might be obscured by
    // other windows, which can cause image/reading writing problems.
    int center_x = viewport.x()+viewport.width()/2;
    int center_y = viewport.y()+viewport.height()/2;

    osg::Viewport* new_viewport = new osg::Viewport;
    new_viewport->setViewport(center_x-width/2,center_y-height/2,width,height);
    rtts->setViewport(new_viewport);
    
    _localState->setAttribute(new_viewport);

    // and the render to texture stage to the current stages
    // dependancy list.
    cv.getCurrentRenderBin()->_stage->addToDependencyList(rtts.get());

    // if one exist attach texture to the RenderToTextureStage.
    if (_texture.valid()) rtts->setTexture(_texture.get());

    // if one exist attach image to the RenderToTextureStage.
    if (_image.valid()) rtts->setImage(_image.get());

}

// call back which cretes a deformation field to oscilate the model.
class MyGeometryCallback : 
    public osg::Drawable::UpdateCallback, 
    public osg::Drawable::AttributeFunctor
{
    public:
    
        MyGeometryCallback(const osg::Vec3& o,
                           const osg::Vec3& x,const osg::Vec3& y,const osg::Vec3& z,
                           double period,double xphase,double amplitude):
            _firstCall(true),
            _startTime(0.0),
            _time(0.0),
            _period(period),
            _xphase(xphase),
            _amplitude(amplitude),
            _origin(o),
            _xAxis(x),
            _yAxis(y),
            _zAxis(z) {}
    
        virtual void update(osg::NodeVisitor* nv,osg::Drawable* drawable)
        {
            const osg::FrameStamp* fs = nv->getFrameStamp();
            double referenceTime = fs->getReferenceTime();
            if (_firstCall)
            {
                _firstCall = false;
                _startTime = referenceTime;
            }
            
            _time = referenceTime-_startTime;
            
            drawable->accept(*this);
            drawable->dirtyBound();
            
            osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(drawable);
            if (geometry)
            {
                osgUtil::SmoothingVisitor::smooth(*geometry);
            }
            
        }
        
        virtual void apply(osg::Drawable::AttributeType type,unsigned int count,osg::Vec3* begin) 
        {
            if (type == osg::Drawable::VERTICES)
            {
                const float TwoPI=2.0f*osg::PI;
                const float phase = -_time/_period;
                
                osg::Vec3* end = begin+count;
                for (osg::Vec3* itr=begin;itr<end;++itr)
                {
                    osg::Vec3 dv(*itr-_origin);
                    osg::Vec3 local(dv*_xAxis,dv*_yAxis,dv*_zAxis);
                    
                    local.z() = local.x()*_amplitude*
                                sinf(TwoPI*(phase+local.x()*_xphase)); 
                    
                    (*itr) = _origin + 
                             _xAxis*local.x()+
                             _yAxis*local.y()+
                             _zAxis*local.z();
                }
            }
        }

        bool    _firstCall;

        double  _startTime;
        double  _time;
        
        double  _period;
        double  _xphase;
        float   _amplitude;

        osg::Vec3   _origin;
        osg::Vec3   _xAxis;
        osg::Vec3   _yAxis;
        osg::Vec3   _zAxis;
        
};

osg::Node* createPreRenderSubGraph(osg::Node* subgraph)
{
    if (!subgraph) return 0;
 
    // create the quad to visualize.
    osg::Geometry* polyGeom = new osg::Geometry();

    polyGeom->setSupportsDisplayList(false);

    osg::Vec3 origin(0.0f,0.0f,0.0f);
    osg::Vec3 xAxis(1.0f,0.0f,0.0f);
    osg::Vec3 yAxis(0.0f,0.0f,1.0f);
    osg::Vec3 zAxis(0.0f,-1.0f,0.0f);
    float height = 100.0f;
    float width = 200.0f;
    int noSteps = 20;
    
    osg::Vec3Array* vertices = new osg::Vec3Array;
    osg::Vec3 bottom = origin;
    osg::Vec3 top = origin; top.z()+= height;
    osg::Vec3 dv = xAxis*(width/((float)(noSteps-1)));
    
    osg::Vec2Array* texcoords = new osg::Vec2Array;
    osg::Vec2 bottom_texcoord(0.0f,0.0f);
    osg::Vec2 top_texcoord(0.0f,1.0f);
    osg::Vec2 dv_texcoord(1.0f/(float)(noSteps-1),0.0f);

    for(int i=0;i<noSteps;++i)
    {
        vertices->push_back(top);
        vertices->push_back(bottom);
        top+=dv;
        bottom+=dv;

        texcoords->push_back(top_texcoord);
        texcoords->push_back(bottom_texcoord);
        top_texcoord+=dv_texcoord;
        bottom_texcoord+=dv_texcoord;
    }
    

    // pass the created vertex array to the points geometry object.
    polyGeom->setVertexArray(vertices);
    
    polyGeom->setTexCoordArray(0,texcoords);
    

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    polyGeom->setColorArray(colors);
    polyGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

    polyGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP,0,vertices->size()));

    // new we need to add the texture to the Drawable, we do so by creating a 
    // StateSet to contain the Texture StateAttribute.
    osg::StateSet* stateset = new osg::StateSet;

    osg::Texture2D* texture = new osg::Texture2D;
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    stateset->setTextureAttributeAndModes(0, texture,osg::StateAttribute::ON);

    polyGeom->setStateSet(stateset);

    polyGeom->setUpdateCallback(new MyGeometryCallback(origin,xAxis,yAxis,zAxis,1.0,1.0/width,0.2f));

    osg::Geode* geode = new osg::Geode();
    geode->addDrawable(polyGeom);

    osg::Group* parent = new osg::Group;
    
    parent->setUpdateCallback(new MyUpdateCallback(subgraph));
    
    parent->setCullCallback(new MyCullCallback(subgraph,texture));
 
    parent->addChild(geode);
    
    return parent;
}

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getProgramName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
   
    // initialize the viewer.
    osgGLUT::Viewer viewer(arguments);

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

    
    // load the nodes from the commandline arguments.
    osg::Node* loadedModel = osgDB::readNodeFiles(arguments);
    if (!loadedModel)
    {
//        write_usage(osg::notify(osg::NOTICE),argv[0]);
        return 1;
    }
    
    // create a transform to spin the model.
    osg::MatrixTransform* loadedModelTransform = new osg::MatrixTransform;
    loadedModelTransform->addChild(loadedModel);

    osg::NodeCallback* nc = new osgUtil::TransformCallback(loadedModelTransform->getBound().center(),osg::Vec3(0.0f,0.0f,1.0f),osg::inDegrees(45.0f));
    loadedModelTransform->setUpdateCallback(nc);

    osg::Group* rootNode = new osg::Group();
//    rootNode->addChild(loadedModelTransform);
    rootNode->addChild(createPreRenderSubGraph(loadedModelTransform));


    // add model to the viewer.
    viewer.addViewport( rootNode );


    // register trackball, flight and drive.
    viewer.registerCameraManipulator(new osgGA::TrackballManipulator);
    viewer.registerCameraManipulator(new osgGA::FlightManipulator);
    viewer.registerCameraManipulator(new osgGA::DriveManipulator);

    viewer.open();

    viewer.run();

    return 0;
}
