#include <osg/GLExtensions>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Matrix>
#include <osg/Quat>
#include <osg/StateSet>
#include <osg/TextureCubeMap>
#include <osg/TexGen>
#include <osg/TexMat>
#include <osg/TexEnvCombine>
#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>

#include <osgUtil/RenderToTextureStage>
#include <osgUtil/Optimizer>

#include <osgDB/ReadFile>
#include <osgDB/Registry>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgProducer/Viewer>

#include <iostream>
#include <string>
#include <vector>


//#define UPDATE_ONE_IMAGE_PER_FRAME 1


class PrerenderAppCallback : public osg::NodeCallback
{
    public:
    
        PrerenderAppCallback(osg::Node* subgraph):
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


class PrerenderCullCallback : public osg::NodeCallback
{
    public:
    
        PrerenderCullCallback(osg::Node* subgraph, osg::TextureCubeMap* cubemap, osg::TexMat* texmat):
            _subgraph(subgraph),
            _cubemap(cubemap),
            _texmat(texmat)
            {
                _updateCubemapFace = 0;
                _clearColor = osg::Vec4(1,1,1,1);
                _localState[0] = new osg::StateSet;
                _localState[1] = new osg::StateSet;
                _localState[2] = new osg::StateSet;
                _localState[3] = new osg::StateSet;
                _localState[4] = new osg::StateSet;
                _localState[5] = new osg::StateSet;
            }

        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
            osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
            if (cv && _cubemap.valid() && _subgraph.valid())
            {
                const osg::Vec4 clearColArray[] =
                {
                    osg::Vec4(0, 0, 1, 1), // +X
                    osg::Vec4(1, 0.7f, 0, 1), // -X
                    osg::Vec4(0, 1, 1, 1), // +Y
                    osg::Vec4(1, 1, 0, 1), // -Y
                    osg::Vec4(1, 0, 0, 1), // +Z
                    osg::Vec4(0, 1, 0, 1)  // -Z
                };

                osg::Quat q;
                q.set(cv->getModelViewMatrix());
                const osg::Matrix C = osg::Matrix::rotate( q.inverse() );
                _texmat->setMatrix(C);

#if UPDATE_ONE_IMAGE_PER_FRAME
                if ((_updateCubemapFace >= 0) && (_updateCubemapFace <= 5))
                {
                    _clearColor = clearColArray[_updateCubemapFace];
                    doPreRender(*cv, _updateCubemapFace++);
                }
#else
                while (_updateCubemapFace<6)
                {
                    _clearColor = clearColArray[_updateCubemapFace];
                    doPreRender(*cv, _updateCubemapFace++);
                }
#endif
            }

            // must traverse the subgraph            
            traverse(node,nv);
        }
        
        void doPreRender(osgUtil::CullVisitor& cv, const int nFace);
        
        struct ImageData
        {
            ImageData(const osg::Vec3& dir, const osg::Vec3& up) : _dir(dir), _up(up) {}
            osg::Vec3 _dir;
            osg::Vec3 _up;
        };

        osg::ref_ptr<osg::Node> _subgraph;
        osg::ref_ptr<osg::TextureCubeMap> _cubemap;
        osg::ref_ptr<osg::StateSet> _localState[6];
        osg::ref_ptr<osg::TexMat> _texmat;
        osg::Vec4 _clearColor;
        int _updateCubemapFace;
};


void PrerenderCullCallback::doPreRender(osgUtil::CullVisitor& cv, const int nFace)
{
    const ImageData id[] =
    {
        ImageData( osg::Vec3( 1,  0,  0), osg::Vec3( 0, -1,  0) ), // +X
        ImageData( osg::Vec3(-1,  0,  0), osg::Vec3( 0, -1,  0) ), // -X
        ImageData( osg::Vec3( 0,  1,  0), osg::Vec3( 0,  0,  1) ), // +Y
        ImageData( osg::Vec3( 0, -1,  0), osg::Vec3( 0,  0, -1) ), // -Y
        ImageData( osg::Vec3( 0,  0,  1), osg::Vec3( 0, -1,  0) ), // +Z
        ImageData( osg::Vec3( 0,  0, -1), osg::Vec3( 0, -1,  0) )  // -Z
    };

    osg::Image* image = _cubemap->getImage((osg::TextureCubeMap::Face)nFace);
    osg::Vec3 dir = id[nFace]._dir;
    osg::Vec3 up  = id[nFace]._up;

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
    rtts->setClearColor(_clearColor);

    // ABJ: use default (color+depth)
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
        
    znear *= 0.9f;
    zfar *= 1.1f;

    // set up projection.
    const double fovy = 90.0;
    const double aspectRatio = 1.0;
    osg::RefMatrix* projection = new osg::RefMatrix;
    projection->makePerspective(fovy, aspectRatio, znear, zfar);

    cv.pushProjectionMatrix(projection);

    osg::RefMatrix* matrix = new osg::RefMatrix;
    osg::Vec3 eye    = bs.center(); eye.z() = 0.0f;
    osg::Vec3 center = eye + dir;
    matrix->makeLookAt(eye, center, up);

    cv.pushModelViewMatrix(matrix);

    cv.pushStateSet(_localState[nFace].get());

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

    int height = 128;
    int width  = 128;

    const osg::Viewport& viewport = *cv.getViewport();

    // offset the impostor viewport from the center of the main window
    // viewport as often the edges of the viewport might be obscured by
    // other windows, which can cause image/reading writing problems.
    int center_x = viewport.x()+viewport.width()/2;
    int center_y = viewport.y()+viewport.height()/2;

    osg::Viewport* new_viewport = new osg::Viewport;
    new_viewport->setViewport(center_x-width/2,center_y-height/2,width,height);
    rtts->setViewport(new_viewport);
    
    _localState[nFace]->setAttribute(new_viewport);

    // and the render to texture stage to the current stages
    // dependancy list.
    cv.getCurrentRenderBin()->_stage->addToDependencyList(rtts.get());

    // if one exist attach image to the RenderToTextureStage.
//  if (_image.valid()) rtts->setImage(_image.get());
    if (image) rtts->setImage(image);
}


osg::Drawable* makeGeometry()
{
    const float radius = 20;
    return new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0f,0.0f,0.0f),radius));
}


osg::Node* createPreRenderSubGraph(osg::Node* subgraph)
{
    if (!subgraph) return 0;
 
    // create the quad to visualize.
    osg::Drawable* geom = makeGeometry();
    geom->setSupportsDisplayList(false);

    // new we need to add the texture to the Drawable, we do so by creating a 
    // StateSet to contain the Texture StateAttribute.
    osg::StateSet* stateset = new osg::StateSet;

    osg::TextureCubeMap* cubemap = new osg::TextureCubeMap;

    // set up the textures
    osg::Image* imagePosX = new osg::Image;
    osg::Image* imageNegX = new osg::Image;
    osg::Image* imagePosY = new osg::Image;
    osg::Image* imageNegY = new osg::Image;
    osg::Image* imagePosZ = new osg::Image;
    osg::Image* imageNegZ = new osg::Image;

    imagePosX->setInternalTextureFormat(GL_RGB);
    imageNegX->setInternalTextureFormat(GL_RGB);
    imagePosY->setInternalTextureFormat(GL_RGB);
    imageNegY->setInternalTextureFormat(GL_RGB);
    imagePosZ->setInternalTextureFormat(GL_RGB);
    imageNegZ->setInternalTextureFormat(GL_RGB);

    cubemap->setImage(osg::TextureCubeMap::POSITIVE_X, imagePosX);
    cubemap->setImage(osg::TextureCubeMap::NEGATIVE_X, imageNegX);
    cubemap->setImage(osg::TextureCubeMap::POSITIVE_Y, imagePosY);
    cubemap->setImage(osg::TextureCubeMap::NEGATIVE_Y, imageNegY);
    cubemap->setImage(osg::TextureCubeMap::POSITIVE_Z, imagePosZ);
    cubemap->setImage(osg::TextureCubeMap::NEGATIVE_Z, imageNegZ);

    cubemap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    cubemap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    cubemap->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
    cubemap->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    cubemap->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    stateset->setTextureAttributeAndModes(0, cubemap, osg::StateAttribute::ON);

    osg::TexGen *texgen = new osg::TexGen;
    texgen->setMode(osg::TexGen::REFLECTION_MAP);
    stateset->setTextureAttributeAndModes(0, texgen, osg::StateAttribute::ON);

    osg::TexMat* texmat = new osg::TexMat;
    stateset->setTextureAttribute(0, texmat);

    stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

    geom->setStateSet(stateset);

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geom);

    // Geodes can't have cull callback so create extra Group to attach cullcallback.
    osg::Group* parent = new osg::Group;
    
    parent->setUpdateCallback(new PrerenderAppCallback(subgraph));
    
    parent->setCullCallback(new PrerenderCullCallback(subgraph, cubemap, texmat));
 
    parent->addChild(geode);
    
    return parent;
}


struct DrawableCullCallback : public osg::Drawable::CullCallback
{
    DrawableCullCallback(osg::TexMat* texmat) : _texmat(texmat)
    {}

    virtual bool cull(osg::NodeVisitor* nv, osg::Drawable* /*drawable*/, osg::State* /*state*/) const
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        if (cv)
        {
            osg::Quat q;
            q.set(cv->getModelViewMatrix());
            const osg::Matrix C = osg::Matrix::rotate( q.inverse() );
            _texmat->setMatrix(C);
        }
        return false;
    }

    mutable osg::ref_ptr<osg::TexMat> _texmat;
};

osg::Node* createReferenceSphere()
{
    const float radius = 10;
    osg::Drawable* sphere = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0f,0.0f,0.0f),radius));

    osg::StateSet* stateset = new osg::StateSet;
    sphere->setStateSet(stateset);

    osg::TextureCubeMap* cubemap = new osg::TextureCubeMap;
    #define CUBEMAP_FILENAME(face) "Cubemap_axis/" #face ".png"

    osg::Image* imagePosX = osgDB::readImageFile(CUBEMAP_FILENAME(posx));
    osg::Image* imageNegX = osgDB::readImageFile(CUBEMAP_FILENAME(negx));
    osg::Image* imagePosY = osgDB::readImageFile(CUBEMAP_FILENAME(posy));
    osg::Image* imageNegY = osgDB::readImageFile(CUBEMAP_FILENAME(negy));
    osg::Image* imagePosZ = osgDB::readImageFile(CUBEMAP_FILENAME(posz));
    osg::Image* imageNegZ = osgDB::readImageFile(CUBEMAP_FILENAME(negz));

    if (imagePosX && imageNegX && imagePosY && imageNegY && imagePosZ && imageNegZ)
    {
        cubemap->setImage(osg::TextureCubeMap::POSITIVE_X, imagePosX);
        cubemap->setImage(osg::TextureCubeMap::NEGATIVE_X, imageNegX);
        cubemap->setImage(osg::TextureCubeMap::POSITIVE_Y, imagePosY);
        cubemap->setImage(osg::TextureCubeMap::NEGATIVE_Y, imageNegY);
        cubemap->setImage(osg::TextureCubeMap::POSITIVE_Z, imagePosZ);
        cubemap->setImage(osg::TextureCubeMap::NEGATIVE_Z, imageNegZ);

        cubemap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
        cubemap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
        cubemap->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
        cubemap->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
        cubemap->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
        stateset->setTextureAttributeAndModes(0, cubemap, osg::StateAttribute::ON);
    }

    osg::TexGen *texgen = new osg::TexGen;
    texgen->setMode(osg::TexGen::REFLECTION_MAP);
    stateset->setTextureAttributeAndModes(0, texgen, osg::StateAttribute::ON);

    osg::TexMat* texmat = new osg::TexMat;
    stateset->setTextureAttribute(0, texmat);

    stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );


    sphere->setCullCallback(new DrawableCullCallback(texmat));

    osg::Geode* geode = new osg::Geode();
    geode->addDrawable(sphere);

    return geode;
}

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates pre rendering of scene to a texture, and then apply this texture to geometry.");
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
/*    
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }
*/
    
    osg::Group* rootNode = new osg::Group();

#if 1
    osg::Node* sky = osgDB::readNodeFile("skydome.osg");
    if (sky)
        rootNode->addChild(createPreRenderSubGraph(sky));
#endif

#if 1
    osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
    pat->setPosition(osg::Vec3(0,0,50));
    pat->addChild(createReferenceSphere());
    rootNode->addChild(pat);
#endif
    // load the nodes from the commandline arguments.
    osg::Node* loadedModel = osgDB::readNodeFiles(arguments);
    if (loadedModel)
        rootNode->addChild(loadedModel);


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
    
    // wait for all cull and draw threads to complete before exit.
    viewer.sync();

    return 0;
}

