#include <osg/GLExtensions>
#include <osg/Node>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/Stencil>
#include <osg/ColorMask>
#include <osg/Depth>
#include <osg/Billboard>
#include <osg/Material>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgUtil/SmoothingVisitor>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgProducer/Viewer>


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

struct MyCameraPostDrawCallback : public osg::CameraNode::DrawCallback
{
    MyCameraPostDrawCallback(osg::Image* image):
        _image(image)
    {
    }

    virtual void operator () (const osg::CameraNode& /*camera*/) const
    {
        if (_image && _image->getPixelFormat()==GL_RGBA && _image->getDataType()==GL_UNSIGNED_BYTE)
        {
            // we'll pick out the center 1/2 of the whole image,
            int column_start = _image->s()/4;
            int column_end = 3*column_start;
            
            int row_start = _image->t()/4;
            int row_end = 3*row_start;
            

            // and then invert these pixels
            for(int r=row_start; r<row_end; ++r)
            {
                unsigned char* data = _image->data(column_start, r);
                for(int c=column_start; c<column_end; ++c)
                {
                    (*data) = 255-(*data); ++data;
                    (*data) = 255-(*data); ++data;
                    (*data) = 255-(*data); ++data;
                    (*data) = 255; ++data;
                }
            }


            // dirty the image (increments the modified count) so that any textures
            // using the image can be informed that they need to update.
            _image->dirty();
        }
        else if (_image && _image->getPixelFormat()==GL_RGBA && _image->getDataType()==GL_FLOAT)
        {
            // we'll pick out the center 1/2 of the whole image,
            int column_start = _image->s()/4;
            int column_end = 3*column_start;
            
            int row_start = _image->t()/4;
            int row_end = 3*row_start;
            
            // and then invert these pixels
            for(int r=row_start; r<row_end; ++r)
            {
                float* data = (float*)_image->data(column_start, r);
                for(int c=column_start; c<column_end; ++c)
                {
                    (*data) = 1.0f-(*data); ++data;
                    (*data) = 1.0f-(*data); ++data;
                    (*data) = 1.0f-(*data); ++data;
                    (*data) = 1.0f; ++data;
                }
            }

            // dirty the image (increments the modified count) so that any textures
            // using the image can be informed that they need to update.
            _image->dirty();
        }
       
    }
    
    osg::Image* _image;
};


osg::Node* createPreRenderSubGraph(osg::Node* subgraph, unsigned tex_width, unsigned tex_height, osg::CameraNode::RenderTargetImplementation renderImplementation, bool useImage, bool useTextureRectangle, bool useHDR)
{
    if (!subgraph) return 0;

    // create a group to contain the flag and the pre rendering camera.
    osg::Group* parent = new osg::Group;

    // texture to render to and to use for rendering of flag.
    osg::Texture* texture = 0;
    if (useTextureRectangle)
    {
        osg::TextureRectangle* textureRect = new osg::TextureRectangle;
        textureRect->setTextureSize(tex_width, tex_height);
        textureRect->setInternalFormat(GL_RGBA);
        textureRect->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
        textureRect->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
        
        texture = textureRect;
    }
    else
    {
        osg::Texture2D* texture2D = new osg::Texture2D;
        texture2D->setTextureSize(tex_width, tex_height);
        texture2D->setInternalFormat(GL_RGBA);
        texture2D->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
        texture2D->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
        
        texture = texture2D;
    } 

    if (useHDR)
    {
        texture->setInternalFormat(GL_RGBA16F_ARB);
        texture->setSourceFormat(GL_RGBA);
        texture->setSourceType(GL_FLOAT);
    }

    // first create the geometry of the flag of which to view.
    { 
        // create the to visualize.
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
        
        // note, when we use TextureRectangle we have to scale the tex coords up to compensate.
        osg::Vec2 bottom_texcoord(0.0f,0.0f);
        osg::Vec2 top_texcoord(0.0f, useTextureRectangle ? tex_height : 1.0f);
        osg::Vec2 dv_texcoord((useTextureRectangle ? tex_width : 1.0f)/(float)(noSteps-1),0.0f);

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

        stateset->setTextureAttributeAndModes(0, texture,osg::StateAttribute::ON);

        polyGeom->setStateSet(stateset);

        polyGeom->setUpdateCallback(new MyGeometryCallback(origin,xAxis,yAxis,zAxis,1.0,1.0/width,0.2f));

        osg::Geode* geode = new osg::Geode();
        geode->addDrawable(polyGeom);
        
        parent->addChild(geode);

    }


    // then create the camera node to do the render to texture
    {    
        osg::CameraNode* camera = new osg::CameraNode;

        // set up the background color and clear mask.
        camera->setClearColor(osg::Vec4(0.1f,0.1f,0.3f,1.0f));
        camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const osg::BoundingSphere& bs = subgraph->getBound();
        if (!bs.valid())
        {
            return subgraph;
        }

        float znear = 1.0f*bs.radius();
        float zfar  = 3.0f*bs.radius();

        // 2:1 aspect ratio as per flag geomtry below.
        float proj_top   = 0.25f*znear;
        float proj_right = 0.5f*znear;

        znear *= 0.9f;
        zfar *= 1.1f;

        // set up projection.
        camera->setProjectionMatrixAsFrustum(-proj_right,proj_right,-proj_top,proj_top,znear,zfar);

        // set view
        camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
        camera->setViewMatrixAsLookAt(bs.center()-osg::Vec3(0.0f,2.0f,0.0f)*bs.radius(),bs.center(),osg::Vec3(0.0f,0.0f,1.0f));

        // set viewport
        camera->setViewport(0,0,tex_width,tex_height);

        // set the camera to render before the main camera.
        camera->setRenderOrder(osg::CameraNode::PRE_RENDER);

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(renderImplementation);

        
        if (useImage)
        {
            osg::Image* image = new osg::Image;
            //image->allocateImage(tex_width, tex_height, 1, GL_RGBA, GL_UNSIGNED_BYTE);
            image->allocateImage(tex_width, tex_height, 1, GL_RGBA, GL_FLOAT);

            // attach the image so its copied on each frame.
            camera->attach(osg::CameraNode::COLOR_BUFFER, image);
            
            camera->setPostDrawCallback(new MyCameraPostDrawCallback(image));
            
            // Rather than attach the texture directly to illustrate the texture's ability to
            // detect an image update and to subload the image onto the texture.  You needn't
            // do this when using an Image for copying to, as a seperate camera->attach(..)
            // would suffice as well, but we'll do it the long way round here just for demonstation
            // purposes (long way round meaning we'll need to copy image to main memory, then
            // copy it back to the graphics card to the texture in one frame).
            // The long way round allows us to mannually modify the copied image via the callback
            // and then let this modified image by reloaded back.
            texture->setImage(0, image);
        }
        else
        {
            // attach the texture and use it as the color buffer.
            camera->attach(osg::CameraNode::COLOR_BUFFER, texture);
        }


        // add subgraph to render
        camera->addChild(subgraph);

        parent->addChild(camera);

    }    

    return parent;
}

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates pre rendering of scene to a texture, and then apply this texture to geometry.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--fbo","Use Frame Buffer Object for render to texture, where supported.");
    arguments.getApplicationUsage()->addCommandLineOption("--fb","Use FrameBuffer for render to texture.");
    arguments.getApplicationUsage()->addCommandLineOption("--pbuffer","Use Pixel Buffer for render to texture, where supported.");
    arguments.getApplicationUsage()->addCommandLineOption("--window","Use a seperate Window for render to texture.");
    arguments.getApplicationUsage()->addCommandLineOption("--width","Set the width of the render to texture.");
    arguments.getApplicationUsage()->addCommandLineOption("--height","Set the height of the render to texture.");
    arguments.getApplicationUsage()->addCommandLineOption("--image","Render to an image, then apply a post draw callback to it, and use this image to update a texture.");
    arguments.getApplicationUsage()->addCommandLineOption("--texture-rectangle","Use osg::TextureRectangle for doing the render to texure to.");
   
    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    unsigned tex_width = 1024;
    unsigned tex_height = 512;
    while (arguments.read("--width", tex_width)) {}
    while (arguments.read("--height", tex_height)) {}

    osg::CameraNode::RenderTargetImplementation renderImplementation = osg::CameraNode::FRAME_BUFFER_OBJECT;
    
    while (arguments.read("--fbo")) { renderImplementation = osg::CameraNode::FRAME_BUFFER_OBJECT; }
    while (arguments.read("--pbuffer")) { renderImplementation = osg::CameraNode::PIXEL_BUFFER; }
    while (arguments.read("--pbuffer-rtt")) { renderImplementation = osg::CameraNode::PIXEL_BUFFER_RTT; }
    while (arguments.read("--fb")) { renderImplementation = osg::CameraNode::FRAME_BUFFER; }
    while (arguments.read("--window")) { renderImplementation = osg::CameraNode::SEPERATE_WINDOW; }

    bool useImage = false;
    while (arguments.read("--image")) { useImage = true; }
    
    bool useTextureRectangle = false;
    while (arguments.read("--texture-rectangle")) { useTextureRectangle = true; }
    
    bool useHDR = false;
    while (arguments.read("--hdr")) { useHDR = true; }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    
    // load the nodes from the commandline arguments.
    osg::Node* loadedModel = osgDB::readNodeFiles(arguments);
    if (!loadedModel)
    {
        return 1;
    }
    
    // create a transform to spin the model.
    osg::MatrixTransform* loadedModelTransform = new osg::MatrixTransform;
    loadedModelTransform->addChild(loadedModel);

    osg::NodeCallback* nc = new osg::AnimationPathCallback(loadedModelTransform->getBound().center(),osg::Vec3(0.0f,0.0f,1.0f),osg::inDegrees(45.0f));
    loadedModelTransform->setUpdateCallback(nc);

    osg::Group* rootNode = new osg::Group();
    rootNode->addChild(createPreRenderSubGraph(loadedModelTransform,tex_width,tex_height, renderImplementation, useImage, useTextureRectangle, useHDR));


    // add model to the viewer.
    viewer.setSceneData( rootNode );


    // create the windows and run the threads.
    viewer.realize();

    while( !viewer.done() )
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
