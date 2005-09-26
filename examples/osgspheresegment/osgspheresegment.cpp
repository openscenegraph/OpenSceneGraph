#include <osgProducer/Viewer>

#include <osg/Group>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>
#include <osg/Geometry>

#include <osgUtil/SmoothingVisitor>

#include <osgDB/ReadFile>

#include <osgText/Text>

#include <osgSim/SphereSegment>
#include <osgSim/OverlayNode>

#include <osgParticle/ExplosionEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/FireEffect>
#include <osgParticle/ParticleSystemUpdater>

// for the grid data..
#include "../osghangglide/terrain_coords.h"

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

osg::Node* createMovingModel(const osg::Vec3& center, float radius)
{
    float animationLength = 10.0f;

    osg::AnimationPath* animationPath = createAnimationPath(center,radius,animationLength);

    osg::Group* model = new osg::Group;

    osg::Node* glider = osgDB::readNodeFile("glider.osg");
    if (glider)
    {
        const osg::BoundingSphere& bs = glider->getBound();

        float size = radius/bs.radius()*0.3f;
        osg::MatrixTransform* positioned = new osg::MatrixTransform;
        positioned->setDataVariance(osg::Object::STATIC);
        positioned->setMatrix(osg::Matrix::translate(-bs.center())*
                                     osg::Matrix::scale(size,size,size)*
                                     osg::Matrix::rotate(osg::inDegrees(-90.0f),0.0f,0.0f,1.0f));
    
        positioned->addChild(glider);
    
        osg::PositionAttitudeTransform* xform = new osg::PositionAttitudeTransform;    
        xform->getOrCreateStateSet()->setMode(GL_NORMALIZE, osg::StateAttribute::ON);
        xform->setUpdateCallback(new osg::AnimationPathCallback(animationPath,0.0,1.0));
        xform->addChild(positioned);

        model->addChild(xform);
    }
 
    osg::Node* cessna = osgDB::readNodeFile("cessna.osg");
    if (cessna)
    {
        const osg::BoundingSphere& bs = cessna->getBound();

        osgText::Text* text = new osgText::Text;
        float size = radius/bs.radius()*0.3f;

        text->setPosition(bs.center());
        text->setText("Cessna");
        text->setAlignment(osgText::Text::CENTER_CENTER);
        text->setAxisAlignment(osgText::Text::SCREEN);
        text->setCharacterSize(40.0f);
        text->setCharacterSizeMode(osgText::Text::SCREEN_COORDS);
        
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(text);
    
        osg::LOD* lod = new osg::LOD;
        lod->setRangeMode(osg::LOD::PIXEL_SIZE_ON_SCREEN);
        lod->addChild(geode,0.0f,100.0f);
        lod->addChild(cessna,100.0f,10000.0f);


        osg::MatrixTransform* positioned = new osg::MatrixTransform;
        positioned->getOrCreateStateSet()->setMode(GL_NORMALIZE, osg::StateAttribute::ON);
        positioned->setDataVariance(osg::Object::STATIC);
        positioned->setMatrix(osg::Matrix::translate(-bs.center())*
                                     osg::Matrix::scale(size,size,size)*
                                     osg::Matrix::rotate(osg::inDegrees(180.0f),0.0f,0.0f,1.0f));
    
        //positioned->addChild(cessna);
        positioned->addChild(lod);
    
        osg::MatrixTransform* xform = new osg::MatrixTransform;
        xform->setUpdateCallback(new osg::AnimationPathCallback(animationPath,0.0f,2.0));
        xform->addChild(positioned);

        // add particle effects to cessna.
        {
            osg::PositionAttitudeTransform* positionEffects = new osg::PositionAttitudeTransform;
            positionEffects->setPosition(osg::Vec3(0.0f,0.0f,0.0f));
            xform->addChild(positionEffects);

            osgParticle::ExplosionEffect* explosion = new osgParticle::ExplosionEffect;
            osgParticle::SmokeEffect* smoke = new osgParticle::SmokeEffect;
            osgParticle::FireEffect* fire = new osgParticle::FireEffect;

            positionEffects->addChild(explosion);
            positionEffects->addChild(smoke);
            positionEffects->addChild(fire);
        }
        
        model->addChild(xform);
    }
    
    return model;
}

osg::Group* createOverlay(const osg::Vec3& center, float radius)
{
    osg::Group* group = new osg::Group;
    
    // create a grid of lines.
    {
        osg::Geometry* geom = new osg::Geometry;
        
        unsigned int num_rows = 10;

        osg::Vec3 left = center+osg::Vec3(-radius,-radius,0.0f);
        osg::Vec3 right = center+osg::Vec3(radius,-radius,0.0f);
        osg::Vec3 delta_row = osg::Vec3(0.0f,2.0f*radius/float(num_rows-1),0.0f);

        osg::Vec3 top = center+osg::Vec3(-radius,radius,0.0f);
        osg::Vec3 bottom = center+osg::Vec3(-radius,-radius,0.0f);
        osg::Vec3 delta_column = osg::Vec3(2.0f*radius/float(num_rows-1),0.0f,0.0f);

        osg::Vec3Array* vertices = new osg::Vec3Array;
        for(unsigned int i=0; i<num_rows; ++i)
        {
            vertices->push_back(left);
            vertices->push_back(right);
            left += delta_row;
            right += delta_row;

            vertices->push_back(top);
            vertices->push_back(bottom);
            top += delta_column;
            bottom += delta_column;
        }
        
        geom->setVertexArray(vertices);

        osg::Vec4ubArray& color = *(new osg::Vec4ubArray(1));
        color[0].set(0,0,0,255);
        geom->setColorArray(&color);
        geom->setColorBinding(osg::Geometry::BIND_OVERALL);

        geom->addPrimitiveSet(new osg::DrawArrays(GL_LINES,0,vertices->getNumElements())); 

        geom->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(geom);
        group->addChild(geode);        
    }
    
    return group;
}

osg::Vec3 computeTerrainIntersection(osg::Node* subgraph,float x,float y)
{
    osgUtil::IntersectVisitor iv;
    osg::ref_ptr<osg::LineSegment> segDown = new osg::LineSegment;

    const osg::BoundingSphere& bs = subgraph->getBound();
    float zMax = bs.center().z()+bs.radius();
    float zMin = bs.center().z()-bs.radius();
    
    segDown->set(osg::Vec3(x,y,zMin),osg::Vec3(x,y,zMax));
    iv.addLineSegment(segDown.get());

    subgraph->accept(iv);

    if (iv.hits())
    {
        osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segDown.get());
        if (!hitList.empty())
        {
            osg::Vec3 ip = hitList.front().getWorldIntersectPoint();
            return  ip;
        }
    }

    return osg::Vec3(x,y,0.0f);
}


//////////////////////////////////////////////////////////////////////////////
// MAIN SCENE GRAPH BUILDING FUNCTION
//////////////////////////////////////////////////////////////////////////////

void build_world(osg::Group *root)
{

    // create terrain
    osg::ref_ptr<osg::Geode> terrainGeode = 0;
    {
        terrainGeode = new osg::Geode;

        osg::StateSet* stateset = new osg::StateSet();
        osg::Image* image = osgDB::readImageFile("Images/lz.rgb");
        if (image)
        {
	    osg::Texture2D* texture = new osg::Texture2D;
	    texture->setImage(image);
	    stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
        }

        terrainGeode->setStateSet( stateset );

        
        {
            unsigned int numColumns = 38;
            unsigned int numRows = 39;
            unsigned int r, c;
            
            osg::Vec3 origin(0.0f,0.0f,0.0f);
            osg::Vec3 size(1000.0f,1000.0f,250.0f);

            osg::Geometry* geometry = new osg::Geometry;

            osg::Vec3Array& v = *(new osg::Vec3Array(numColumns*numRows));
            osg::Vec2Array& tc = *(new osg::Vec2Array(numColumns*numRows));
            osg::Vec4ubArray& color = *(new osg::Vec4ubArray(1));

            color[0].set(255,255,255,255);

            float rowCoordDelta = size.y()/(float)(numRows-1);
            float columnCoordDelta = size.x()/(float)(numColumns-1);

            float rowTexDelta = 1.0f/(float)(numRows-1);
            float columnTexDelta = 1.0f/(float)(numColumns-1);

            // compute z range of z values of grid data so we can scale it.
            float min_z = FLT_MAX;
            float max_z = -FLT_MAX;
            for(r=0;r<numRows;++r)
            {
	        for(c=0;c<numColumns;++c)
	        {
	            min_z = osg::minimum(min_z,vertex[r+c*numRows][2]);
	            max_z = osg::maximum(max_z,vertex[r+c*numRows][2]);
	        }
            }

            float scale_z = size.z()/(max_z-min_z);

            osg::Vec3 pos = origin;
            osg::Vec2 tex(0.0f,0.0f);
            int vi=0;
            for(r=0;r<numRows;++r)
            {
                pos.x() = origin.x();
                tex.x() = 0.0f;
	        for(c=0;c<numColumns;++c)
	        {
	            v[vi].set(pos.x(),pos.y(),pos.z()+(vertex[r+c*numRows][2]-min_z)*scale_z);
	            tc[vi] = tex;
                    pos.x()+=columnCoordDelta;
                    tex.x()+=columnTexDelta;
                    ++vi;
	        }
                pos.y() += rowCoordDelta;
                tex.y() += rowTexDelta;
            }

            geometry->setVertexArray(&v);
            geometry->setTexCoordArray(0, &tc);
            geometry->setColorArray(&color);
            geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

            for(r=0;r<numRows-1;++r)
            {
                osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(GL_QUAD_STRIP,2*numColumns));
                geometry->addPrimitiveSet(&drawElements);
                int ei=0;
	        for(c=0;c<numColumns;++c)
	        {
	            drawElements[ei++] = (r+1)*numColumns+c;
	            drawElements[ei++] = (r)*numColumns+c;
	        }
            }
            
            osgUtil::SmoothingVisitor smoother;
            smoother.smooth(*geometry);
            
            terrainGeode->addDrawable(geometry);
        }
        
    }    


    // create sphere segment
    osg::ref_ptr<osgSim::SphereSegment> ss = 0;
    {
        ss = new osgSim::SphereSegment(
                        computeTerrainIntersection(terrainGeode.get(),550.0f,780.0f), // center
                        500.0f, // radius
                        osg::DegreesToRadians(135.0f),
                        osg::DegreesToRadians(245.0f),
                        osg::DegreesToRadians(-10.0f),
                        osg::DegreesToRadians(30.0f),
                        60);
        ss->setAllColors(osg::Vec4(1.0f,1.0f,1.0f,0.5f));
        ss->setSideColor(osg::Vec4(0.0f,1.0f,1.0f,0.1f));

        root->addChild(ss.get());
    }

    osgSim::SphereSegment::LineList lines = ss->computeIntersection(osg::Matrixd::identity(), terrainGeode.get());
    if (!lines.empty())
    {
        osg::notify(osg::NOTICE)<<"We've found intersections!!!!"<<std::endl;

        osg::Geode* geode = new osg::Geode;
        root->addChild(geode);

        geode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

        for(osgSim::SphereSegment::LineList::iterator itr=lines.begin();
           itr!=lines.end();
           ++itr)
        {
            osg::Geometry* geom = new osg::Geometry;
            geode->addDrawable(geom);
            
            osg::Vec3Array* vertices = itr->get();
            geom->setVertexArray(vertices);
            geom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, vertices->getNumElements()));
        }
    }
    else
    {
       osg::notify(osg::NOTICE)<<"No intersections found"<<std::endl;
    }

#if 1


    osgSim::OverlayNode* overlayNode = new osgSim::OverlayNode;
    
    overlayNode->getOrCreateStateSet()->setTextureAttribute(1, new osg::TexEnv(osg::TexEnv::DECAL));

    const osg::BoundingSphere& bs = terrainGeode->getBound();
    osg::Group* overlaySubgraph = createOverlay(bs.center(), bs.radius()*0.5f);
    overlaySubgraph->addChild(ss.get());
    overlayNode->setOverlaySubgraph(overlaySubgraph);
    overlayNode->setOverlayTextureSizeHint(2048);
    overlayNode->addChild(terrainGeode.get());

    root->addChild(overlayNode);
    
    
#else
    root->addChild(terrainGeode);
#endif


    // create particle effects
    {    
        osg::Vec3 position = computeTerrainIntersection(terrainGeode.get(),100.0f,100.0f);

        osgParticle::ExplosionEffect* explosion = new osgParticle::ExplosionEffect(position, 10.0f);
        osgParticle::SmokeEffect* smoke = new osgParticle::SmokeEffect(position, 10.0f);
        osgParticle::FireEffect* fire = new osgParticle::FireEffect(position, 10.0f);

        root->addChild(explosion);
        root->addChild(smoke);
        root->addChild(fire);
    }
    
    // create particle effects
    {    
        osg::Vec3 position = computeTerrainIntersection(terrainGeode.get(),200.0f,100.0f);

        osgParticle::ExplosionEffect* explosion = new osgParticle::ExplosionEffect(position, 1.0f);
        osgParticle::SmokeEffect* smoke = new osgParticle::SmokeEffect(position, 1.0f);
        osgParticle::FireEffect* fire = new osgParticle::FireEffect(position, 1.0f);

        root->addChild(explosion);
        root->addChild(smoke);
        root->addChild(fire);
    }
    
    // create the moving models.
    {
        root->addChild(createMovingModel(osg::Vec3(500.0f,500.0f,500.0f),100.0f));
    }
}


//////////////////////////////////////////////////////////////////////////////
// main()
//////////////////////////////////////////////////////////////////////////////


int main(int argc, char **argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use of particle systems.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] image_file_left_eye image_file_right_eye");
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
    
    osg::Group *root = new osg::Group;
    build_world(root);
   
    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData(root);
        
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
