#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TexGen>
#include <osg/Material>

#include <osgUtil/Optimizer>
#include <osgUtil/RenderToTextureStage>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>


#include <osgGLUT/glut>
#include <osgGLUT/Viewer>


// for the grid data..
#include "../osghangglide/terrain_coords.h"


class MyCullCallback : public osg::NodeCallback
{
    public:
    
        MyCullCallback(osg::Node* subgraph,osg::Texture2D* texture,const osg::Vec3& position):
            _subgraph(subgraph),
            _texture(texture),
            _position(position) {}
       
        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {

            osgUtil::CullVisitor* cullVisitor = dynamic_cast<osgUtil::CullVisitor*>(nv);
            if (cullVisitor && (_texture.valid() && _subgraph.valid()))
            {            
                doPreRender(*node,*cullVisitor);

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
        osg::Vec3                    _position;

    
};

void MyCullCallback::doPreRender(osg::Node& node, osgUtil::CullVisitor& cv)
{   

    const osg::BoundingSphere& bs = _subgraph->getBound();
    if (!bs.valid())
    {
        osg::notify(osg::WARN) << "bb invalid"<<_subgraph.get()<<std::endl;
        return;
    }
    

    // create the render to texture stage.
    osg::ref_ptr<osgUtil::RenderToTextureStage> rtts = osgNew osgUtil::RenderToTextureStage;

    // set up lighting.
    // currently ignore lights in the scene graph itself..
    // will do later.
    osgUtil::RenderStage* previous_stage = cv.getCurrentRenderBin()->_stage;

    // set up the background color and clear mask.
    rtts->setClearColor(osg::Vec4(1.0f,1.0f,1.0f,0.0f));
    rtts->setClearMask(previous_stage->getClearMask());

    // set up to charge the same RenderStageLighting is the parent previous stage.
    rtts->setRenderStageLighting(previous_stage->getRenderStageLighting());


    // record the render bin, to be restored after creation
    // of the render to text
    osgUtil::RenderBin* previousRenderBin = cv.getCurrentRenderBin();

    // set the current renderbin to be the newly created stage.
    cv.setCurrentRenderBin(rtts.get());


    float centerDistance = (_position-bs.center()).length();

    float znear = centerDistance+bs.radius();
    float zfar  = centerDistance-bs.radius();
    float zNearRatio = 0.001f;
    if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;
    
        
    // 2:1 aspect ratio as per flag geomtry below.
    float top   = (bs.radius()/centerDistance)*znear;
    float right = top;

    // set up projection.
    osg::Matrix* projection = osgNew osg::Matrix;
    projection->makeFrustum(-right,right,-top,top,znear,zfar);

    cv.pushProjectionMatrix(projection);

    osg::Matrix* matrix = new osg::Matrix;
    matrix->makeLookAt(_position,bs.center(),osg::Vec3(0.0f,1.0f,0.0f));


    osg::Matrix inverseMV;
    inverseMV.invert(cv.getModelViewMatrix());

    // compute the matrix which takes a vertex from local coords into tex coords
    // will use this later to specify osg::TexGen..
    osg::Matrix MVPT = 
                       *matrix * 
                       *projection *
                       osg::Matrix::translate(1.0,1.0,1.0) *
                       osg::Matrix::scale(0.5f,0.5f,0.5f);

    cv.pushModelViewMatrix(matrix);

    osg::ref_ptr<osg::StateSet> shadowState = osgNew osg::StateSet;

    // make the material black for a shadow.
    osg::Material* material = new osg::Material;
    material->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4(0.0f,0.0f,0.0f,1.0f));
    material->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(0.0f,0.0f,0.0f,1.0f));
    material->setEmission(osg::Material::FRONT_AND_BACK,osg::Vec4(0.0f,0.0f,0.0f,1.0f));
    material->setShininess(osg::Material::FRONT_AND_BACK,0.0f);
    shadowState->setAttribute(material,osg::StateAttribute::OVERRIDE);

    cv.pushStateSet(shadowState.get());

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
    int width  = 256;


    const osg::Viewport& viewport = *cv.getViewport();

    // offset the impostor viewport from the center of the main window
    // viewport as often the edges of the viewport might be obscured by
    // other windows, which can cause image/reading writing problems.
    int center_x = viewport.x()+viewport.width()/2;
    int center_y = viewport.y()+viewport.height()/2;

    osg::Viewport* new_viewport = new osg::Viewport;
    new_viewport->setViewport(center_x-width/2,center_y-height/2,width,height);
    rtts->setViewport(new_viewport);
    
    shadowState->setAttribute(new_viewport);

    // and the render to texture stage to the current stages
    // dependancy list.
    cv.getCurrentRenderBin()->_stage->addToDependencyList(rtts.get());

    // if one exist attach texture to the RenderToTextureStage.
    if (_texture.valid()) rtts->setTexture(_texture.get());


    // set up the stateset to decorate the subgraph with the shadow texture
    // with the appropriate tex gen coords.
    osg::StateSet* stateset = new osg::StateSet;

    osg::TexGen* texgen = new osg::TexGen;
    texgen->setMode(osg::TexGen::OBJECT_LINEAR);
    texgen->setPlane(osg::TexGen::S,osg::Vec4(MVPT(0,0),MVPT(1,0),MVPT(2,0),MVPT(3,0)));
    texgen->setPlane(osg::TexGen::T,osg::Vec4(MVPT(0,1),MVPT(1,1),MVPT(2,1),MVPT(3,1)));
    texgen->setPlane(osg::TexGen::R,osg::Vec4(MVPT(0,2),MVPT(1,2),MVPT(2,2),MVPT(3,2)));
    texgen->setPlane(osg::TexGen::Q,osg::Vec4(MVPT(0,3),MVPT(1,3),MVPT(2,3),MVPT(3,3)));

    int _unit = 1;

    stateset->setTextureAttributeAndModes(_unit,_texture.get(),osg::StateAttribute::ON);
    stateset->setTextureAttribute(_unit,texgen);
    stateset->setTextureMode(_unit,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
    stateset->setTextureMode(_unit,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
    stateset->setTextureMode(_unit,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);
    stateset->setTextureMode(_unit,GL_TEXTURE_GEN_Q,osg::StateAttribute::ON);

    cv.pushStateSet(stateset);

        // must traverse the subgraph            
        traverse(&node,&cv);

    cv.popStateSet();    

}






osg::AnimationPath* createAnimationPath(const osg::Vec3& center,float radius,double looptime)
{
    // set up the animation path 
    osg::AnimationPath* animationPath = new osg::AnimationPath;
    animationPath->setLoopMode(osg::AnimationPath::LOOP);
    
    int numSamples = 40;
    float yaw = 0.0f;
    float yaw_delta = 2.0f*osg::PI/((float)numSamples-1.0f);
    float roll = osg::inDegrees(30.0f);
    
    double time=0.0f;
    double time_delta = looptime/(double)numSamples;
    for(int i=0;i<numSamples;++i)
    {
        osg::Vec3 position(center+osg::Vec3(sinf(yaw)*radius,cosf(yaw)*radius,0.0f));
        osg::Quat rotation(osg::Quat(roll,osg::Vec3(0.0,1.0,0.0))*osg::Quat(-(yaw+osg::inDegrees(90.0f)),osg::Vec3(0.0,0.0,1.0)));
        
        animationPath->insert(time,osg::AnimationPath::ControlPoint(position,rotation));

        yaw += yaw_delta;
        time += time_delta;

    }
    return animationPath;    
}

osg::Node* createBase(const osg::Vec3& center,float radius)
{
// 
//     osg::Geode* geode = osgNew osg::Geode;
//     
//     // set up the texture of the base.
//     osg::StateSet* stateset = osgNew osg::StateSet();
//     osg::Image* image = osgDB::readImageFile("Images/lz.rgb");
//     if (image)
//     {
// 	osg::Texture2D* texture = osgNew osg::Texture2D;
// 	texture->setImage(image);
// 	stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
//     }
//     
//     geode->setStateSet( stateset );
// 
// 
//     osg::Grid* grid = new osg::Grid;
//     grid->allocateGrid(38,39);
//     grid->setOrigin(center+osg::Vec3(-radius,-radius,0.0f));
//     grid->setXInterval(radius*2.0f/(float)(38-1));
//     grid->setYInterval(radius*2.0f/(float)(39-1));
//     
//     float minHeight = FLT_MAX;
//     float maxHeight = -FLT_MAX;
// 
// 
//     unsigned int r;
//     for(r=0;r<39;++r)
//     {
// 	for(unsigned int c=0;c<38;++c)
// 	{
//             float h = vertex[r+c*39][2];
//             if (h>maxHeight) maxHeight=h;
//             if (h<minHeight) minHeight=h;
//         }
//     }
//     
//     float hieghtScale = radius*0.5f/(maxHeight-minHeight);
//     float hieghtOffset = -(minHeight+maxHeight)*0.5f;
// 
//     for(r=0;r<39;++r)
//     {
// 	for(unsigned int c=0;c<38;++c)
// 	{
//             float h = vertex[r+c*39][2];
// 	    grid->setHeight(c,r,(h+hieghtOffset)*hieghtScale);
// 	}
//     }
// 
//     
//     geode->addDrawable(new osg::ShapeDrawable(grid));
//      
//     osg::Group* group = osgNew osg::Group;
//     group->addChild(geode);
//      
//     return group;
// 

    int numTilesX = 10;
    int numTilesY = 10;
    
    float width = 2*radius;
    float height = 2*radius;
    
    osg::Vec3 v000(center - osg::Vec3(width*0.5f,height*0.5f,0.0f));
    osg::Vec3 dx(osg::Vec3(width/((float)numTilesX),0.0,0.0f));
    osg::Vec3 dy(osg::Vec3(0.0f,height/((float)numTilesY),0.0f));
    
    // fill in vertices for grid, note numTilesX+1 * numTilesY+1...
    osg::Vec3Array* coords = osgNew osg::Vec3Array;
    int iy;
    for(iy=0;iy<=numTilesY;++iy)
    {
        for(int ix=0;ix<=numTilesX;++ix)
        {
            coords->push_back(v000+dx*(float)ix+dy*(float)iy);
        }
    }
    
    //Just two colours - black and white.
    osg::Vec4Array* colors = osgNew osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f)); // white
    colors->push_back(osg::Vec4(0.0f,1.0f,1.0f,1.0f)); // black
    int numColors=colors->size();
    
    
    int numIndicesPerRow=numTilesX+1;
    osg::UByteArray* coordIndices = osgNew osg::UByteArray; // assumes we are using less than 256 points...
    osg::UByteArray* colorIndices = osgNew osg::UByteArray;
    for(iy=0;iy<numTilesY;++iy)
    {
        for(int ix=0;ix<numTilesX;++ix)
        {
            // four vertices per quad.
            coordIndices->push_back(ix    +(iy+1)*numIndicesPerRow);
            coordIndices->push_back(ix    +iy*numIndicesPerRow);
            coordIndices->push_back((ix+1)+iy*numIndicesPerRow);
            coordIndices->push_back((ix+1)+(iy+1)*numIndicesPerRow);
            
            // one color per quad
            colorIndices->push_back((ix+iy)%numColors);
        }
    }
    

    // set up a single normal
    osg::Vec3Array* normals = osgNew osg::Vec3Array;
    normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
    

    osg::Geometry* geom = osgNew osg::Geometry;
    geom->setVertexArray(coords);
    geom->setVertexIndices(coordIndices);
    
    geom->setColorArray(colors);
    geom->setColorIndices(colorIndices);
    geom->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE);
    
    geom->setNormalArray(normals);
    geom->setNormalBinding(osg::Geometry::BIND_OVERALL);
    
    geom->addPrimitiveSet(osgNew osg::DrawArrays(osg::PrimitiveSet::QUADS,0,coordIndices->size()));
    
    osg::Geode* geode = osgNew osg::Geode;
    geode->addDrawable(geom);
    
    osg::Group* group = osgNew osg::Group;
    group->addChild(geode);
    
    return group;
}

osg::Node* createMovingModel(const osg::Vec3& center, float radius)
{
    float animationLength = 10.0f;

    osg::AnimationPath* animationPath = createAnimationPath(center,radius,animationLength);

    osg::Group* model = new osg::Group;

//     osg::Node* glider = osgDB::readNodeFile("glider.osg");
//     if (glider)
//     {
//         const osg::BoundingSphere& bs = glider->getBound();
// 
//         float size = radius/bs.radius()*0.3f;
//         osg::MatrixTransform* positioned = osgNew osg::MatrixTransform;
//         positioned->setDataVariance(osg::Object::STATIC);
//         positioned->setMatrix(osg::Matrix::translate(-bs.center())*
//                                      osg::Matrix::scale(size,size,size)*
//                                      osg::Matrix::rotate(osg::inDegrees(-90.0f),0.0f,0.0f,1.0f));
//     
//         positioned->addChild(glider);
//     
//         osg::PositionAttitudeTransform* xform = osgNew osg::PositionAttitudeTransform;    
//         xform->setAppCallback(new osg::PositionAttitudeTransform::AnimationPathCallback(animationPath,0.0,1.0));
//         xform->addChild(positioned);
// 
//         model->addChild(xform);
//     }
//  
    osg::Node* cessna = osgDB::readNodeFile("cessna.osg");
    if (cessna)
    {
        const osg::BoundingSphere& bs = cessna->getBound();

        float size = radius/bs.radius()*0.3f;
        osg::MatrixTransform* positioned = osgNew osg::MatrixTransform;
        positioned->setDataVariance(osg::Object::STATIC);
        positioned->setMatrix(osg::Matrix::translate(-bs.center())*
                              osg::Matrix::scale(size,size,size)*
                              osg::Matrix::rotate(osg::inDegrees(180.0f),0.0f,0.0f,1.0f));
    
        positioned->addChild(cessna);
    
        osg::MatrixTransform* xform = osgNew osg::MatrixTransform;
        xform->setAppCallback(new osg::MatrixTransform::AnimationPathCallback(animationPath,0.0f,2.0));
        xform->addChild(positioned);

        model->addChild(xform);
    }
    
    return model;
}

osg::Node* createModel()
{
    osg::Vec3 center(0.0f,0.0f,0.0f);
    float radius = 100.0f;

    osg::Group* root = osgNew osg::Group;

    // the occluder subgraph
    osg::Node* occluder = createMovingModel(center,radius*0.5f);

    // the occludee subgraph
    osg::Node* occludee = createBase(center-osg::Vec3(0.0f,0.0f,radius*0.25),radius);
    
    // now add a state with a texture for the shadow
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

    osg::Vec3 lightPosition(center+osg::Vec3(0.0f,0.0f,radius));

    osg::Geode* lightgeode = new osg::Geode;
    lightgeode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(lightPosition,radius/100.0f)));

    occludee->setCullCallback(new MyCullCallback(occluder,texture,lightPosition));

    root->addChild(lightgeode);
    root->addChild(occluder);
    root->addChild(occludee);


    return root;
}


int main( int argc, char **argv )
{

    // initialize the GLUT
    glutInit( &argc, argv );

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
    osg::Node* model = createModel();
    if (!model)
    {
        return 1;
    }
    
    // tilt the scene so the default eye position is looking down on the model.
    osg::MatrixTransform* rootnode = new osg::MatrixTransform;
//    rootnode->setMatrix(osg::Matrix::rotate(osg::inDegrees(30.0f),1.0f,0.0f,0.0f));
    rootnode->addChild(model);

    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    //optimzer.optimize(rootnode);
     
    // add a viewport to the viewer and attach the scene graph.
    viewer.addViewport( rootnode );
    
    // register trackball, flight and drive.
    viewer.registerCameraManipulator(new osgGA::TrackballManipulator);
    viewer.registerCameraManipulator(new osgGA::FlightManipulator);
    viewer.registerCameraManipulator(new osgGA::DriveManipulator);


    // open the viewer window.
    viewer.open();
    
    // fire up the event loop.
    viewer.run();

    return 0;
}
