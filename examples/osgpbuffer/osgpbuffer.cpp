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

#include <osgUtil/TransformCallback>
#include <osgUtil/SmoothingVisitor>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgProducer/Viewer>

#include "RenderToTextureStage.h"
#include "pbuffer.h"

PBuffer* g_pPixelBuffer;

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
            _texture(texture)
        {
        }

        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {

            osgUtil::CullVisitor* cullVisitor = dynamic_cast<osgUtil::CullVisitor*>(nv);
            if (cullVisitor && _texture.valid() && _subgraph.valid())
                doPreRender(*node,*cullVisitor);

            // must traverse the subgraph            
            traverse(node,nv);
        }
        
        void doPreRender(osg::Node& node, osgUtil::CullVisitor& cv);
        
        osg::ref_ptr<osg::Node>      _subgraph;
        osg::ref_ptr<osg::Texture2D> _texture;
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
    osg::ref_ptr<MyRenderToTextureStage> rtts = new MyRenderToTextureStage;
    rtts->setPBuffer(g_pPixelBuffer);

    // set up lighting.
    // currently ignore lights in the scene graph itself..
    // will do later.
    osgUtil::RenderStage* previous_stage = cv.getCurrentRenderBin()->getStage();

    // set up the background color and clear mask.
    rtts->setClearColor(osg::Vec4(0.1f,0.9f,0.3f,1.0f));
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

    if (!_localState) _localState = new osg::StateSet;

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

    if (rtts->getRenderGraphList().size()==0 && rtts->getRenderBinList().size()==0)
    {
        // getting to this point means that all the subgraph has been
        // culled by small feature culling or is beyond LOD ranges.
        return;
    }



    int height = 512;
    int width  = 512;


    const osg::Viewport& viewport = *cv.getViewport();

    // offset the impostor viewport from the center of the main window
    // viewport as often the edges of the viewport might be obscured by
    // other windows, which can cause image/reading writing problems.
    int center_x = viewport.x()+viewport.width()/2;
    int center_y = viewport.y()+viewport.height()/2;

    osg::Viewport* new_viewport = new osg::Viewport;
//  new_viewport->setViewport(center_x-width/2,center_y-height/2,width,height);
    new_viewport->setViewport(0,0,width,height);
    rtts->setViewport(new_viewport);

    _localState->setAttribute(new_viewport);    

    // and the render to texture stage to the current stages
    // dependancy list.
    cv.getCurrentRenderBin()->getStage()->addToDependencyList(rtts.get());

    // if one exist attach texture to the RenderToTextureStage.
    if (_texture.valid()) rtts->setTexture(_texture.get());

    // if one exist attach image to the RenderToTextureStage.
//    if (_image.valid()) rtts->setImage(_image.get());

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


// Custom Texture subload callback, just acts the the standard subload modes in osg::Texture right now
// but code be used to define your own style callbacks.
class MyTextureSubloadCallback : public osg::Texture2D::SubloadCallback
{
    public:
    
        MyTextureSubloadCallback():
            _subloadMode(AUTO),
            _textureWidth(0),
            _textureHeight(0),
            _subloadTextureOffsetX(0),
            _subloadTextureOffsetY(0),
            _subloadImageOffsetX(0),
            _subloadImageOffsetY(0),
            _subloadImageWidth(0),
            _subloadImageHeight(0)
        {
        }

        enum SubloadMode {
            OFF,
            AUTO,
            IF_DIRTY
        };

        /** Set the texture subload mode. */
        inline void setSubloadMode(const SubloadMode mode) { _subloadMode = mode; }

        /** Get the texture subload mode. */
        inline const SubloadMode getSubloadMode() const { return _subloadMode; }

        /** Set the texture subload texture offsets. */
        inline void setSubloadTextureOffset(const int x, const int y)
        {
            _subloadTextureOffsetX = x;
            _subloadTextureOffsetY = y;
        }

        /** Get the texture subload texture offsets. */
        inline void getSubloadTextureOffset(int& x, int& y) const
        {
            x = _subloadTextureOffsetX;
            y = _subloadTextureOffsetY;
        }
        
        /** Set the texture subload width. If width or height are zero then
          * the repsective size value is calculated from the source image sizes. */
        inline void setSubloadTextureSize(const int width, const int height)
        {
            _textureWidth = width;
            _textureHeight = height;
        }

        /** Get the texture subload width. */
        inline void getSubloadTextureSize(int& width, int& height) const
        {
            width = _textureWidth;
            height = _textureHeight;
        }


        /** Set the subload image offsets. */
        inline void setSubloadImageOffset(const int x, const int y)
        {
            _subloadImageOffsetX = x;
            _subloadImageOffsetY = y;
        }

        /** Get the subload image offsets. */
        inline void getSubloadImageOffset(int& x, int& y) const
        {
            x = _subloadImageOffsetX;
            y = _subloadImageOffsetY;
        }

        /** Set the image subload width. If width or height are zero then
          * the repsective size value is calculated from the source image sizes. */
        inline void setSubloadImageSize(const int width, const int height)
        {
            _subloadImageWidth = width;
            _subloadImageHeight = height;
        }

        /** Get the image subload width. */
        inline void getSubloadImageSize(int& width, int& height) const
        {
            width = _subloadImageWidth;
            height = _subloadImageHeight;
        }
    
    
    
        virtual void load(const osg::Texture2D& texture,osg::State&) const
        {
            osg::notify(osg::INFO)<<"doing load"<<std::endl;
/*
            static bool s_SGIS_GenMipmap = osg::isGLExtensionSupported("GL_SGIS_generate_mipmap");

            if (s_SGIS_GenMipmap && (texture.getFilter(osg::Texture2D::MIN_FILTER) != osg::Texture2D::LINEAR && texture.getFilter(osg::Texture2D::MIN_FILTER) != osg::Texture2D::NEAREST))
            {
                texture.setNumMipmapLevels(1); // will leave this at one, since the mipmap will be created internally by OpenGL.
                glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
            }
            else
            {
                texture.setNumMipmapLevels(1);
            }


            GLsizei width = (_subloadImageWidth>0)?_subloadImageWidth:texture.getImage()->s();
            GLsizei height = (_subloadImageHeight>0)?_subloadImageHeight:texture.getImage()->t();

        
            bool sizeChanged = false;
            if (_textureWidth==0)
            {
                // need to calculate texture dimension
                sizeChanged = true;
                _textureWidth = 1;
                for (; _textureWidth < (static_cast<GLsizei>(_subloadTextureOffsetX) + width); _textureWidth <<= 1) {}
             }

            if (_textureHeight==0)
            {
                // need to calculate texture dimension
                sizeChanged = true;
                _textureHeight = 1;
                for (; _textureHeight < (static_cast<GLsizei>(_subloadTextureOffsetY) + height); _textureHeight <<= 1) {}
            }
            
            if (sizeChanged)
            {
                texture.setTextureSize(_textureWidth, _textureHeight);
            }
*/
#if 0
            // reserve appropriate texture memory
            glTexImage2D(GL_TEXTURE_2D, 0, texture.getInternalFormat(),
                         _textureWidth, _textureHeight, 0,
                         (GLenum) texture.getImage()->getPixelFormat(), (GLenum) texture.getImage()->getDataType(),
                         NULL);


            glPixelStorei(GL_UNPACK_ROW_LENGTH,texture.getImage()->s());
            

            glTexSubImage2D(GL_TEXTURE_2D, 0,
                            _subloadTextureOffsetX, _subloadTextureOffsetY,
                            width, height,
                            (GLenum) texture.getImage()->getPixelFormat(), (GLenum) texture.getImage()->getDataType(),
                            texture.getImage()->data(_subloadImageOffsetX,_subloadImageOffsetY));

            glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
#else
	        glTexImage2D( GL_TEXTURE_2D, 0, texture.getInternalFormat(), _textureWidth, _textureHeight, 0, GL_RGB, GL_FLOAT, 0 );
#endif
        }
        
        virtual void subload(const osg::Texture2D& texture,osg::State&) const
        {
            osg::notify(osg::INFO)<<"doing subload"<<std::endl;
#if 0        
            glPixelStorei(GL_UNPACK_ROW_LENGTH,texture.getImage()->s());

            glTexSubImage2D(GL_TEXTURE_2D, 0,
                            _subloadTextureOffsetX, _subloadTextureOffsetY,
                            (_subloadImageWidth>0)?_subloadImageWidth:texture.getImage()->s(), (_subloadImageHeight>0)?_subloadImageHeight:texture.getImage()->t(),
                            (GLenum) texture.getImage()->getPixelFormat(), (GLenum) texture.getImage()->getDataType(),
                            texture.getImage()->data(_subloadImageOffsetX,_subloadImageOffsetY));

            glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
#else
#endif
        }


        SubloadMode _subloadMode;
        mutable GLsizei _textureWidth, _textureHeight;
        GLint _subloadTextureOffsetX, _subloadTextureOffsetY;
        GLint _subloadImageOffsetX, _subloadImageOffsetY;
        GLsizei _subloadImageWidth, _subloadImageHeight;
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

    // set up the texture.
//    osg::Image* image = new osg::Image;
//    image->setInternalTextureFormat(GL_RGBA);

    // Dynamic texture filled with data from pbuffer.
    osg::Texture2D* texture = new osg::Texture2D;
    //texture->setSubloadMode(osg::Texture::IF_DIRTY);
    texture->setInternalFormat(GL_RGB);
    texture->setTextureSize(512,512);
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP);
texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP);
    texture->setSubloadCallback(new MyTextureSubloadCallback());
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

class InitializePbufferCallback : public osgProducer::OsgCameraGroup::RealizeCallback
{
    public:
        InitializePbufferCallback() {}
        
        virtual void operator()( osgProducer::OsgCameraGroup&, osgProducer::OsgSceneHandler& sh, const Producer::RenderSurface& )
        { 
            if (!g_pPixelBuffer)
            {
                 g_pPixelBuffer = new PBuffer(512,512);
                g_pPixelBuffer->initialize();
            }            

            // now safe to continue
            sh.init();
        }
        
};

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use pbuffers and render to texture..");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    

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


    // set the scene to render
    viewer.setSceneData(rootNode);

    // use a realize callback to create and initialize the PBuffer to ensure it has a valid graphics context.
    viewer.setRealizeCallback(new InitializePbufferCallback());

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
    
    // wait for all cull and draw threads to complete before exit.
    viewer.sync();

    delete g_pPixelBuffer;

    return 0;
}
