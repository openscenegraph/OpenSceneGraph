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
#include <osg/Projection>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgUtil/TransformCallback>
#include <osgUtil/RenderToTextureStage>
#include <osgUtil/SmoothingVisitor>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgProducer/Viewer>

using namespace osg;

class DistortionNode : public osg::Group
{
public:

    DistortionNode();
    
    DistortionNode(const DistortionNode& rhs,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
        osg::Group(rhs,copyop) {}

    META_Node(osgDistortion, DistortionNode);

    virtual void traverse(osg::NodeVisitor& nv);

protected:

    void createHUDSubgraph();

    void preRender(osgUtil::CullVisitor& nv);   

    osg::ref_ptr<osg::Node>         _hudSubgraph;
    osg::ref_ptr<osg::Texture2D>    _texture;    
    osg::ref_ptr<osg::StateSet>     _localStateSet;
    
};


DistortionNode::DistortionNode()
{
    createHUDSubgraph();
}

void DistortionNode::createHUDSubgraph()
{
    // create the quad to visualize.
    osg::Geometry* polyGeom = new osg::Geometry();

    polyGeom->setSupportsDisplayList(false);

    osg::Vec3 origin(0.0f,0.0f,0.0f);
    osg::Vec3 xAxis(1.0f,0.0f,0.0f);
    osg::Vec3 yAxis(0.0f,1.0f,0.0f);
    osg::Vec3 zAxis(0.0f,0.0f,1.0f);
    float height = 1024.0f;
    float width = 1280.0f;
    int noSteps = 50;
    
    osg::Vec3Array* vertices = new osg::Vec3Array;
    osg::Vec2Array* texcoords = new osg::Vec2Array;
    osg::Vec4Array* colors = new osg::Vec4Array;

    osg::Vec3 bottom = origin;
    osg::Vec3 dx = xAxis*(width/((float)(noSteps-1)));
    osg::Vec3 dy = yAxis*(height/((float)(noSteps-1)));
    
    osg::Vec2 bottom_texcoord(0.0f,0.0f);
    osg::Vec2 dx_texcoord(1.0f/(float)(noSteps-1),0.0f);
    osg::Vec2 dy_texcoord(0.0f,1.0f/(float)(noSteps-1));
    
    osg::Vec3 cursor = bottom;
    osg::Vec2 texcoord = bottom_texcoord;
    int i,j;
    for(i=0;i<noSteps;++i)
    {
        osg::Vec3 cursor = bottom+dy*(float)i;
        osg::Vec2 texcoord = bottom_texcoord+dy_texcoord*(float)i;
        for(j=0;j<noSteps;++j)
        {
            vertices->push_back(cursor);
            texcoords->push_back(osg::Vec2((sin(texcoord.x()*osg::PI-osg::PI*0.5)+1.0f)*0.5f,(sin(texcoord.y()*osg::PI-osg::PI*0.5)+1.0f)*0.5f));
            colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
            
            cursor += dx;
            texcoord += dx_texcoord;
        }
    }

    // pass the created vertex array to the points geometry object.
    polyGeom->setVertexArray(vertices);

    polyGeom->setColorArray(colors);
    polyGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

    polyGeom->setTexCoordArray(0,texcoords);
    

    for(i=0;i<noSteps-1;++i)
    {
        osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(osg::PrimitiveSet::QUAD_STRIP);
        for(j=0;j<noSteps;++j)
        {
            elements->push_back(j+(i+1)*noSteps);
            elements->push_back(j+(i)*noSteps);
        }
        polyGeom->addPrimitiveSet(elements);
    }


    // new we need to add the texture to the Drawable, we do so by creating a 
    // StateSet to contain the Texture StateAttribute.
    osg::StateSet* stateset = new osg::StateSet;

    osg::Texture2D* texture = new osg::Texture2D;
//     texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
//     texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    stateset->setTextureAttributeAndModes(0, texture,osg::StateAttribute::ON);
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    polyGeom->setStateSet(stateset);
    
    _texture = texture;

    osg::Geode* geode = new osg::Geode();
    geode->addDrawable(polyGeom);

    // create the hud.
    osg::MatrixTransform* modelview_abs = new osg::MatrixTransform;
    modelview_abs->setReferenceFrame(osg::Transform::RELATIVE_TO_ABSOLUTE);
    modelview_abs->setMatrix(osg::Matrix::identity());
    modelview_abs->addChild(geode);

    osg::Projection* projection = new osg::Projection;
    projection->setMatrix(osg::Matrix::ortho2D(0,1280,0,1024));
    projection->addChild(modelview_abs);

    _hudSubgraph = projection;
    
    _localStateSet = new osg::StateSet;


}


void DistortionNode::traverse(osg::NodeVisitor& nv)
{
    if (nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR)
    {
        osgUtil::CullVisitor* cullVisitor = dynamic_cast<osgUtil::CullVisitor*>(&nv);
        if (cullVisitor && _texture.valid() && _hudSubgraph.valid())
        {
            preRender(*cullVisitor);
            
            _hudSubgraph->accept(nv);
            
            return;
        }
    }
    
    Group::traverse(nv);
}

void DistortionNode::preRender(osgUtil::CullVisitor& cv)
{   
    // create the render to texture stage.
    osg::ref_ptr<osgUtil::RenderToTextureStage> rtts = new osgUtil::RenderToTextureStage;

    // set up lighting.
    // currently ignore lights in the scene graph itself..
    // will do later.
    osgUtil::RenderStage* previous_stage = cv.getCurrentRenderBin()->_stage;

    // set up the background color and clear mask.
    rtts->setClearColor(osg::Vec4(0.1f,0.1f,0.3f,1.0f));
    rtts->setClearMask(previous_stage->getClearMask());

    // set up to charge the same RenderStageLighting is the parent previous stage.
    rtts->setRenderStageLighting(previous_stage->getRenderStageLighting());


    // record the render bin, to be restored after creation
    // of the render to text
    osgUtil::RenderBin* previousRenderBin = cv.getCurrentRenderBin();

    // set the current renderbin to be the newly created stage.
    cv.setCurrentRenderBin(rtts.get());

    cv.pushStateSet(_localStateSet.get());

    {

        // traverse the subgraph
        Group::traverse(cv);

    }

    cv.popStateSet();

    // restore the previous renderbin.
    cv.setCurrentRenderBin(previousRenderBin);

    if (rtts->_renderGraphList.size()==0 && rtts->_bins.size()==0)
    {
        // getting to this point means that all the subgraph has been
        // culled by small feature culling or is beyond LOD ranges.
        return;
    }

    int height = 1024;
    int width  = 1024;


    const osg::Viewport& viewport = *cv.getViewport();

    // offset the impostor viewport from the center of the main window
    // viewport as often the edges of the viewport might be obscured by
    // other windows, which can cause image/reading writing problems.
    int center_x = viewport.x()+viewport.width()/2;
    int center_y = viewport.y()+viewport.height()/2;

    osg::Viewport* new_viewport = new osg::Viewport;
    new_viewport->setViewport(center_x-width/2,center_y-height/2,width,height);
    rtts->setViewport(new_viewport);
    
    _localStateSet->setAttribute(new_viewport);

    // and the render to texture stage to the current stages
    // dependancy list.
    cv.getCurrentRenderBin()->_stage->addToDependencyList(rtts.get());

    // if one exist attach texture to the RenderToTextureStage.
    if (_texture.valid()) rtts->setTexture(_texture.get());
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
    DistortionNode* distortionNode = new DistortionNode;
    distortionNode->addChild(loadedModel);

    // add model to the viewer.
    viewer.setSceneData( distortionNode );

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
