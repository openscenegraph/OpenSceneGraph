#include <osg/Node>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/Transform>
#include <osg/Texture>
#include <osg/Transparency>
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

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGLUT/glut>
#include <osgGLUT/Viewer>


class MyCullCallback : public osg::NodeCallback
{
    public:
    
        MyCullCallback(osg::Node* subgraph,osg::Texture* texture):
            _subgraph(subgraph),
            _texture(texture) {}

        MyCullCallback(osg::Node* subgraph,osg::Image* image):
            _subgraph(subgraph),
            _image(image) {}
        
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
        
        osg::ref_ptr<osg::Node>     _subgraph;
        osg::ref_ptr<osg::Texture>  _texture;
        osg::ref_ptr<osg::Image>    _image;

    
};

void MyCullCallback::doPreRender(osg::Node& node, osgUtil::CullVisitor& cv)
{   
    // default to true right now, will dertermine if perspective from the
    // projection matrix...
    bool isPerspectiveProjection = true;

    const osg::Matrix& matrix = cv.getModelViewMatrix();
    const osg::BoundingSphere& bs = _subgraph->getBound();
    osg::Vec3 eye_local = cv.getEyeLocal();

    if (!bs.valid())
    {
        osg::notify(osg::WARN) << "bb invalid"<<_subgraph.get()<<std::endl;
        return;
    }

    osg::Vec3 eye_world(0.0,0.0,0.0);
    osg::Vec3 center_world = bs.center()*matrix;

    // no appropriate sprite has been found therefore need to create
    // one for use.

    // create the render to texture stage.
    osg::ref_ptr<osgUtil::RenderToTextureStage> rtts = osgNew osgUtil::RenderToTextureStage;

    // set up lighting.
    // currently ignore lights in the scene graph itself..
    // will do later.
    osgUtil::RenderStage* previous_stage = cv.getCurrentRenderBin()->_stage;

    // set up the background color and clear mask.
    osg::Vec4 clear_color = previous_stage->getClearColor();
    clear_color[3] = 0.0f; // set the alpha to zero.
    rtts->setClearColor(clear_color);
    rtts->setClearMask(previous_stage->getClearMask());

    // set up to charge the same RenderStageLighting is the parent previous stage.
    rtts->setRenderStageLighting(previous_stage->getRenderStageLighting());


    // record the render bin, to be restored after creation
    // of the render to text
    osgUtil::RenderBin* previousRenderBin = cv.getCurrentRenderBin();

    // set the current renderbin to be the newly created stage.
    cv.setCurrentRenderBin(rtts.get());

    // create quad coords (in local coords)

    osg::Vec3 center_local = bs.center();
    osg::Vec3 camera_up_local = cv.getUpLocal();
    osg::Vec3 lv_local = center_local-eye_local;

    float distance_local = lv_local.length();
    lv_local /= distance_local;
   
    osg::Vec3 sv_local = lv_local^camera_up_local;
    sv_local.normalize();
    
    osg::Vec3 up_local = sv_local^lv_local;
    

    
    float width = bs.radius();
    if (isPerspectiveProjection)
    {
        // expand the width to account for projection onto sprite.
        width *= (distance_local/sqrtf(distance_local*distance_local-bs.radius2()));
    }
    
    // scale up and side vectors to sprite width.
    up_local *= width;
    sv_local *= width;
    
    // create the corners of the sprite.
    osg::Vec3 c00(center_local - sv_local - up_local);
    osg::Vec3 c11(center_local + sv_local + up_local);
    
// adjust camera left,right,up,down to fit (in world coords)

    osg::Vec3 near_local  ( center_local-lv_local*width );
    osg::Vec3 far_local   ( center_local+lv_local*width );
    osg::Vec3 top_local   ( center_local+up_local);
    osg::Vec3 right_local ( center_local+sv_local);
    
    osg::Vec3 near_world = near_local * matrix;
    osg::Vec3 far_world = far_local * matrix;
    osg::Vec3 top_world = top_local * matrix;
    osg::Vec3 right_world = right_local * matrix;
    
    float znear = (near_world-eye_world).length();
    float zfar  = (far_world-eye_world).length();
        
    float top   = (top_world-center_world).length();
    float right = (right_world-center_world).length();

    znear *= 0.9f;
    zfar *= 1.1f;

    // set up projection.
    osg::Matrix* projection = osgNew osg::Matrix;
    if (isPerspectiveProjection)
    {
        // deal with projection issue move the top and right points
        // onto the near plane.
        float ratio = znear/(center_world-eye_world).length();
        top *= ratio;
        right *= ratio;
        projection->makeFrustum(-right,right,-top,top,znear,zfar);
    }
    else
    {
        projection->makeOrtho(-right,right,-top,top,znear,zfar);
    }

    cv.pushProjectionMatrix(projection);

    osg::Vec3 rotate_from = bs.center()-eye_local;
    osg::Vec3 rotate_to   = cv.getLookVectorLocal();

    osg::Matrix* rotate_matrix = osgNew osg::Matrix(
        osg::Matrix::translate(-eye_local)*        
        osg::Matrix::rotate(rotate_from,rotate_to)*
        osg::Matrix::translate(eye_local)*
        cv.getModelViewMatrix());

    // pushing the cull view state will update it so it takes
    // into account the new camera orientation.
    cv.pushModelViewMatrix(rotate_matrix);

    osg::ref_ptr<osg::StateSet> dummyState = osgNew osg::StateSet;

    cv.pushStateSet(dummyState.get());

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


    const osg::Viewport& viewport = *cv.getViewport();
    

    // calc texture size for eye, bs.

    // convert the corners of the sprite (in world coords) into their
    // equivilant window coordinates by using the camera's project method.
    const osg::Matrix& MVPW = cv.getMVPW();
    osg::Vec3 c00_win = c00 * MVPW;
    osg::Vec3 c11_win = c11 * MVPW;

// adjust texture size to be nearest power of 2.

    float s  = c11_win.x()-c00_win.x();
    float t  = c11_win.y()-c00_win.y();

    // may need to reverse sign of width or height if a matrix has
    // been applied which flips the orientation of this subgraph.
    if (s<0.0f) s = -s;
    if (t<0.0f) t = -t;

    // bias value used to assist the rounding up or down of
    // the texture dimensions to the nearest power of two.
    // bias near 0.0 will almost always round down.
    // bias near 1.0 will almost always round up. 
    float bias = 0.7f;

    float sp2 = logf((float)s)/logf(2.0f);
    float rounded_sp2 = floorf(sp2+bias);
    int new_s = (int)(powf(2.0f,rounded_sp2));

    float tp2 = logf((float)t)/logf(2.0f);
    float rounded_tp2 = floorf(tp2+bias);
    int new_t = (int)(powf(2.0f,rounded_tp2));

    // if dimension is bigger than window divide it down.    
    while (new_s>viewport.width()) new_s /= 2;

    // if dimension is bigger than window divide it down.    
    while (new_t>viewport.height()) new_t /= 2;


    // offset the impostor viewport from the center of the main window
    // viewport as often the edges of the viewport might be obscured by
    // other windows, which can cause image/reading writing problems.
    int center_x = viewport.x()+viewport.width()/2;
    int center_y = viewport.y()+viewport.height()/2;

    osg::Viewport* new_viewport = new osg::Viewport;
    new_viewport->setViewport(center_x-new_s/2,center_y-new_t/2,new_s,new_t);
    rtts->setViewport(new_viewport);
    
    dummyState->setAttribute(new_viewport);

    // and the render to texture stage to the current stages
    // dependancy list.
    cv.getCurrentRenderBin()->_stage->addToDependencyList(rtts.get());

    // if one exist attach texture to the RenderToTextureStage.
    if (_texture.valid()) rtts->setTexture(_texture.get());

    // if one exist attach image to the RenderToTextureStage.
    if (_image.valid()) rtts->setImage(_image.get());

}




osg::Node* createPreRenderSubGraph(osg::Node* subgraph)
{
    if (!subgraph) return 0;
 
    const osg::BoundingSphere& bs = subgraph->getBound();
    
    // create the quad to visualize.
    osg::Geometry* polyGeom = new osg::Geometry();


    float radius = bs.radius()*1.5f;    
    osg::Vec3Array* vertices = new osg::Vec3Array;
    vertices->push_back(osg::Vec3(-radius,0.0f,radius));
    vertices->push_back(osg::Vec3(-radius,0.0f,-radius));
    vertices->push_back(osg::Vec3(radius,0.0f,-radius));
    vertices->push_back(osg::Vec3(radius,0.0f,radius));

    // pass the created vertex array to the points geometry object.
    polyGeom->setVertexArray(vertices);

    osg::Vec3Array* normals = new osg::Vec3Array;
    normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
    polyGeom->setNormalArray(normals);
    polyGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    polyGeom->setColorArray(colors);
    polyGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::Vec2 myTexCoords[] =
    {
        osg::Vec2(0,1),
        osg::Vec2(0,0),
        osg::Vec2(1,0),
        osg::Vec2(1,1)
    };

    int numTexCoords = sizeof(myTexCoords)/sizeof(osg::Vec2);
    polyGeom->setTexCoordArray(0,new osg::Vec2Array(numTexCoords,myTexCoords));

    polyGeom->addPrimitive(new osg::DrawArrays(osg::Primitive::QUADS,0,4));

    // new we need to add the texture to the Drawable, we do so by creating a 
    // StateSet to contain the Texture StateAttribute.
    osg::StateSet* stateset = new osg::StateSet;

    // set up the texture.
    osg::Image* image = new osg::Image;
    image->setInternalTextureFormat(GL_RGBA);
    //osg::Image* image = osgDB::readImageFile("lz.rgb");
    osg::Texture* texture = new osg::Texture;
    texture->setSubloadMode(osg::Texture::IF_DIRTY);
    texture->setImage(image);
    stateset->setTextureAttributeAndModes(0, texture,osg::StateAttribute::ON);

    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OVERRIDE_OFF);

    polyGeom->setStateSet(stateset);

    osg::Billboard* billboard = osgNew osg::Billboard();
    
    billboard->setMode(osg::Billboard::POINT_ROT_EYE);
    billboard->addDrawable(polyGeom,bs.center());
    
    osg::Group* parent = new osg::Group;
//   parent->setCullCallback(new MyCullCallback(subgraph,texture));
    parent->setCullCallback(new MyCullCallback(subgraph,image));
    parent->addChild(billboard);
    
    return parent;
}

void write_usage(std::ostream& out,const std::string& name)
{
    out << std::endl;
    out <<"usage:"<< std::endl;
    out <<"    "<<name<<" [options] infile1 [infile2 ...]"<< std::endl;
    out << std::endl;
    out <<"options:"<< std::endl;
    out <<"    -l libraryName      - load plugin of name libraryName"<< std::endl;
    out <<"                          i.e. -l osgdb_pfb"<< std::endl;
    out <<"                          Useful for loading reader/writers which can load"<< std::endl;
    out <<"                          other file formats in addition to its extension."<< std::endl;
    out <<"    -e extensionName    - load reader/wrter plugin for file extension"<< std::endl;
    out <<"                          i.e. -e pfb"<< std::endl;
    out <<"                          Useful short hand for specifying full library name as"<< std::endl;
    out <<"                          done with -l above, as it automatically expands to"<< std::endl;
    out <<"                          the full library name appropriate for each platform."<< std::endl;
    out <<std::endl;
    out <<"    -stereo             - switch on stereo rendering, using the default of,"<< std::endl;
    out <<"                          ANAGLYPHIC or the value set in the OSG_STEREO_MODE "<< std::endl;
    out <<"                          environmental variable. See doc/stereo.html for "<< std::endl;
    out <<"                          further details on setting up accurate stereo "<< std::endl;
    out <<"                          for your system. "<< std::endl;
    out <<"    -stereo ANAGLYPHIC  - switch on anaglyphic(red/cyan) stereo rendering."<< std::endl;
    out <<"    -stereo QUAD_BUFFER - switch on quad buffered stereo rendering."<< std::endl;
    out <<std::endl;
    out <<"    -stencil            - use a visual with stencil buffer enabled, this "<< std::endl;
    out <<"                          also allows the depth complexity statistics mode"<< std::endl;
    out <<"                          to be used (press 'p' three times to cycle to it)."<< std::endl;
    out << std::endl;
}

int main( int argc, char **argv )
{

    // initialize the GLUT
    glutInit( &argc, argv );

    if (argc<2)
    {
        write_usage(osg::notify(osg::NOTICE),argv[0]);
        return 0;
    }

    // create the commandline args.
    std::vector<std::string> commandLine;
    for(int i=1;i<argc;++i) commandLine.push_back(argv[i]);
    

    // initialize the viewer.
    osgGLUT::Viewer viewer;
    viewer.setWindowTitle(argv[0]);
    
    // configure the viewer from the commandline arguments, and eat any
    // parameters that have been matched.
    viewer.readCommandLine(commandLine);
    
    // configure the plugin registry from the commandline arguments, and 
    // eat any parameters that have been matched.
    osgDB::readCommandLine(commandLine);
    
    // load the nodes from the commandline arguments.
    osg::Node* loadedModel = osgDB::readNodeFiles(commandLine);
    

    if (!loadedModel)
    {
//        write_usage(osg::notify(osg::NOTICE),argv[0]);
        return 1;
    }
    
    // create a transform to spin the model.
    osg::Transform* loadedModelTransform = new osg::Transform;
    loadedModelTransform->addChild(loadedModel);

    osg::NodeCallback* nc = new osgUtil::TransformCallback(loadedModelTransform->getBound().center(),osg::Vec3(0.0f,0.0f,1.0f),osg::inDegrees(45.0f));
    loadedModelTransform->setAppCallback(nc);

    osg::Group* rootNode = new osg::Group();
    rootNode->addChild(loadedModelTransform);
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
