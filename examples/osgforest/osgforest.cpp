#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/Billboard>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/StateSet>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/Switch>

#include <osgProducer/Viewer>

#include <osgDB/ReadFile>
#include <osgUtil/IntersectVisitor>
#include <osgUtil/SmoothingVisitor>

#include <osg/Math>

// for the grid data..
#include "../osghangglide/terrain_coords.h"

// class to create the forest and manage the movement between various techniques.
class ForestTechniqueManager : public osg::Referenced
{
public:

    ForestTechniqueManager() {}

    class Tree : public osg::Referenced
    {
    public:

        Tree():
            _color(255,255,255,255),
            _width(1.0f),
            _height(1.0f),
            _type(0) {}

        Tree(const osg::Vec3& position, const osg::UByte4& color, float width, float height, unsigned int type):
            _position(position),
            _color(color),
            _width(width),
            _height(height),
            _type(type) {}

        osg::Vec3       _position;
        osg::UByte4     _color;
        float           _width;
        float           _height;
        unsigned int    _type;
    };

    typedef std::vector< osg::ref_ptr<Tree> > TreeList;

    float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }
    int random(int min,int max) { return min + (int)((float)(max-min)*(float)rand()/(float)RAND_MAX); }

    osg::Geode* createTerrain(const osg::Vec3& origin, const osg::Vec3& size);

    void createTreeList(osg::Node* terrain,const osg::Vec3& origin, const osg::Vec3& size,unsigned int numTreesToCreate,TreeList& trees);

    osg::Geometry* createSprite( float w, float h, osg::UByte4 color );

    osg::Geometry* createOrthogonalQuads( const osg::Vec3& pos, float w, float h, osg::UByte4 color );

    osg::Node* createScene();
    
    void advanceToNextTechnique(int delta=1)
    {
        if (_techniqueSwitch.valid())
        {
            _currentTechnique = (_currentTechnique + delta)%_techniqueSwitch->getNumChildren();
            _techniqueSwitch->setSingleChildOn(_currentTechnique);
        }
    }
    
    osg::ref_ptr<osg::Switch>   _techniqueSwitch;
    int                         _currentTechnique;
    

};

// event handler to capture keyboard events and use them to advance the technique used for rendering
class TechniqueEventHandler : public osgGA::GUIEventHandler, public osg::NodeCallback
{
public:

    TechniqueEventHandler(ForestTechniqueManager* ttm=0) { _ForestTechniqueManager = ttm; }
    
    META_Object(osgforestApp,TechniqueEventHandler);

    virtual void accept(osgGA::GUIEventHandlerVisitor& v) { v.visit(*this); }

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);
    
    virtual void getUsage(osg::ApplicationUsage& usage) const;

protected:

    ~TechniqueEventHandler() {}
    
    TechniqueEventHandler(const TechniqueEventHandler&,const osg::CopyOp&) {}
    
    osg::ref_ptr<ForestTechniqueManager> _ForestTechniqueManager;

        
};

bool TechniqueEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            if (ea.getKey()=='n' ||
                ea.getKey()==osgGA::GUIEventAdapter::KEY_Right || 
                ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Right)
            {
                _ForestTechniqueManager->advanceToNextTechnique(1);
                return true;
            }
            else if (ea.getKey()=='p' ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_Left || 
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Left)
            {
                _ForestTechniqueManager->advanceToNextTechnique(-1);
                return true;
            }
            return false;
        }

        default:
            return false;
    }
}

void TechniqueEventHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("n or Left Arrow","Advance to next technique");
    usage.addKeyboardMouseBinding("p or Right Array","Move to previous technique");
}


osg::Geode* ForestTechniqueManager::createTerrain(const osg::Vec3& origin, const osg::Vec3& size)
{
    osg::Geode* geode = new osg::Geode();

    // ---------------------------------------
    // Set up a StateSet to texture the objects
    // ---------------------------------------
    osg::StateSet* stateset = new osg::StateSet();

    osg::Image* image = osgDB::readImageFile("Images/lz.rgb");
    if (image)
    {
	osg::Texture2D* texture = new osg::Texture2D;
	texture->setImage(image);
	stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
    }
    
    geode->setStateSet( stateset );
    
    unsigned int numColumns = 38;
    unsigned int numRows = 39;
    unsigned int r;
    unsigned int c;

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


    bool createGrid = false;
    if (createGrid)
    {

        osg::Grid* grid = new osg::Grid;
        grid->allocateGrid(numColumns,numRows);
        grid->setOrigin(origin);
        grid->setXInterval(size.x()/(float)(numColumns-1));
        grid->setYInterval(size.y()/(float)(numRows-1));

        for(r=0;r<numRows;++r)
        {
	    for(c=0;c<numColumns;++c)
	    {
	        grid->setHeight(c,r,(vertex[r+c*numRows][2]-min_z)*scale_z);
	    }
        }
        
        geode->addDrawable(new osg::ShapeDrawable(grid));
    }
    else
    {
        osg::Geometry* geometry = new osg::Geometry;
        
        osg::Vec3Array& v = *(new osg::Vec3Array(numColumns*numRows));
        osg::Vec2Array& t = *(new osg::Vec2Array(numColumns*numRows));
        osg::UByte4Array& color = *(new osg::UByte4Array(1));
        
        color[0].set(255,255,255,255);

        float rowCoordDelta = size.y()/(float)(numRows-1);
        float columnCoordDelta = size.x()/(float)(numColumns-1);
        
        float rowTexDelta = 1.0f/(float)(numRows-1);
        float columnTexDelta = 1.0f/(float)(numColumns-1);

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
	        t[vi].set(tex.x(),tex.y());
                pos.x()+=columnCoordDelta;
                tex.x()+=columnTexDelta;
                ++vi;
	    }
            pos.y() += rowCoordDelta;
            tex.y() += rowTexDelta;
        }
        
        geometry->setVertexArray(&v);
        geometry->setColorArray(&color);
        geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        geometry->setTexCoordArray(0,&t);
        
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
        
        geode->addDrawable(geometry);
        
        osgUtil::SmoothingVisitor sv;
        sv.smooth(*geometry);
    }
    
    return geode;
}

void ForestTechniqueManager::createTreeList(osg::Node* terrain,const osg::Vec3& origin, const osg::Vec3& size,unsigned int numTreesToCreate,TreeList& trees)
{

    float max_TreeHeight = sqrtf(size.length2()/(float)numTreesToCreate);
    float max_TreeWidth = max_TreeHeight*0.5f;
    
    float min_TreeHeight = max_TreeHeight*0.3f;
    float min_TreeWidth = min_TreeHeight*0.5f;

    trees.reserve(trees.size()+numTreesToCreate);


    for(unsigned int i=0;i<numTreesToCreate;++i)
    {
        Tree* tree = new Tree;
        tree->_position.set(random(origin.x(),origin.x()+size.x()),random(origin.y(),origin.y()+size.y()),origin.z());
        tree->_color.set(random(128,255),random(128,255),random(128,255),255);
        tree->_width = random(min_TreeWidth,max_TreeWidth);
        tree->_height = random(min_TreeHeight,max_TreeHeight);
        tree->_type = 0;
        
        if (terrain)
        {
            osgUtil::IntersectVisitor iv;
            osg::ref_ptr<osg::LineSegment> segDown = new osg::LineSegment;

            segDown->set(tree->_position,tree->_position+osg::Vec3(0.0f,0.0f,size.z()));
            iv.addLineSegment(segDown.get());
            
            terrain->accept(iv);

            if (iv.hits())
            {
                osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segDown.get());
                if (!hitList.empty())
                {
                    osg::Vec3 ip = hitList.front().getWorldIntersectPoint();
                    osg::Vec3 np = hitList.front().getWorldIntersectNormal();
                    tree->_position = ip;
                }
            }
        }
        
        trees.push_back(tree);
    }
}

osg::Geometry* ForestTechniqueManager::createSprite( float w, float h, osg::UByte4 color )
{
    // set up the coords
    osg::Vec3Array& v = *(new osg::Vec3Array(4));
    osg::Vec2Array& t = *(new osg::Vec2Array(4));
    osg::UByte4Array& c = *(new osg::UByte4Array(1));

    v[0].set(-w*0.5f,0.0f,0.0f);
    v[1].set( w*0.5f,0.0f,0.0f);
    v[2].set( w*0.5f,0.0f,h);
    v[3].set(-w*0.5f,0.0f,h);

    c[0] = color;

    t[0].set(0.0f,0.0f);
    t[1].set(1.0f,0.0f);
    t[2].set(1.0f,1.0f);
    t[3].set(0.0f,1.0f);

    osg::Geometry *geom = new osg::Geometry;

    geom->setVertexArray( &v );

    geom->setTexCoordArray( 0, &t );

    geom->setColorArray( &c );
    geom->setColorBinding( osg::Geometry::BIND_OVERALL );

    geom->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4) );

    return geom;
}

osg::Geometry* ForestTechniqueManager::createOrthogonalQuads( const osg::Vec3& pos, float w, float h, osg::UByte4 color )
{
    // set up the coords
    osg::Vec3Array& v = *(new osg::Vec3Array(8));
    osg::Vec2Array& t = *(new osg::Vec2Array(8));
    osg::UByte4Array& c = *(new osg::UByte4Array(1));
    
    float rotation = random(0.0f,osg::PI/2.0f);
    float sw = sinf(rotation)*w*0.5f;
    float cw = cosf(rotation)*w*0.5f;

    v[0].set(pos.x()-sw,pos.y()-cw,pos.z()+0.0f);
    v[1].set(pos.x()+sw,pos.y()+cw,pos.z()+0.0f);
    v[2].set(pos.x()+sw,pos.y()+cw,pos.z()+h);
    v[3].set(pos.x()-sw,pos.y()-cw,pos.z()+h);

    v[4].set(pos.x()-cw,pos.y()+sw,pos.z()+0.0f);
    v[5].set(pos.x()+cw,pos.y()-sw,pos.z()+0.0f);
    v[6].set(pos.x()+cw,pos.y()-sw,pos.z()+h);
    v[7].set(pos.x()-cw,pos.y()+sw,pos.z()+h);

    c[0] = color;

    t[0].set(0.0f,0.0f);
    t[1].set(1.0f,0.0f);
    t[2].set(1.0f,1.0f);
    t[3].set(0.0f,1.0f);

    t[4].set(0.0f,0.0f);
    t[5].set(1.0f,0.0f);
    t[6].set(1.0f,1.0f);
    t[7].set(0.0f,1.0f);

    osg::Geometry *geom = new osg::Geometry;

    geom->setVertexArray( &v );

    geom->setTexCoordArray( 0, &t );

    geom->setColorArray( &c );
    geom->setColorBinding( osg::Geometry::BIND_OVERALL );

    geom->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,8) );

    return geom;
}

osg::Node* ForestTechniqueManager::createScene()
{
    osg::Vec3 origin(0.0f,0.0f,0.0f);
    osg::Vec3 size(1000.0f,1000.0f,200.0f);
    unsigned int numTreesToCreates = 10000;

    osg::ref_ptr<osg::Node> terrain = createTerrain(origin,size);
    
    TreeList trees;
    createTreeList(terrain.get(),origin,size,numTreesToCreates,trees);
    
    osg::Texture2D *tex = new osg::Texture2D;
    tex->setImage(osgDB::readImageFile("Images/tree0.rgba"));

    osg::StateSet *dstate = new osg::StateSet;
    {    
        dstate->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON );
        dstate->setTextureAttribute(0, new osg::TexEnv );

        dstate->setAttributeAndModes( new osg::BlendFunc, osg::StateAttribute::ON );

        osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
        alphaFunc->setFunction(osg::AlphaFunc::GEQUAL,0.05f);
        dstate->setAttributeAndModes( alphaFunc, osg::StateAttribute::ON );

        dstate->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

        dstate->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    }
    

    _techniqueSwitch = new osg::Switch;
    
    {
        osg::Billboard* billboard = new osg::Billboard;
        billboard->setStateSet(dstate);

        for(TreeList::iterator itr=trees.begin();
            itr!=trees.end();
            ++itr)
        {
            Tree& tree = **itr;
            billboard->addDrawable(createSprite(tree._width,tree._height,tree._color),tree._position);    
        }
    
        _techniqueSwitch->addChild(billboard);
    }

    {
        osg::Geode* geode = new osg::Geode;
        geode->setStateSet(dstate);
        
        for(TreeList::iterator itr=trees.begin();
            itr!=trees.end();
            ++itr)
        {
            Tree& tree = **itr;
            geode->addDrawable(createOrthogonalQuads(tree._position,tree._width,tree._height,tree._color));
        }
        
        _techniqueSwitch->addChild(geode);
    }

    {

        osg::Group* transform_group = new osg::Group;
        //group->setStateSet(dstate);
        
        osg::Geometry* geometry = createOrthogonalQuads(osg::Vec3(0.0f,0.0f,0.0f),1.0f,1.0f,osg::UByte4(255,255,255,255));
        
        for(TreeList::iterator itr=trees.begin();
            itr!=trees.end();
            ++itr)
        {
            Tree& tree = **itr;
            osg::MatrixTransform* transform = new osg::MatrixTransform;
            transform->setMatrix(osg::Matrix::scale(tree._width,tree._width,tree._height)*osg::Matrix::translate(tree._position));
    
            osg::Geode* geode = new osg::Geode;
            geode->setStateSet(dstate);
            geode->addDrawable(geometry);
            transform->addChild(geode);
            transform_group->addChild(transform);
        }
        
        _techniqueSwitch->addChild(transform_group);
    }
    
    _currentTechnique = 0;
    _techniqueSwitch->setSingleChildOn(_currentTechnique);
    

    osg::Group* scene = new osg::Group;
    
    scene->addChild(terrain.get());
    scene->addChild(_techniqueSwitch.get());

    return scene;
}

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates the osg::Shape classes.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
   
    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);
    
    osg::ref_ptr<ForestTechniqueManager> ttm = new ForestTechniqueManager;
    
    viewer.getEventHandlerList().push_front(new TechniqueEventHandler(ttm.get()));

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
    
    osg::Node* node = ttm->createScene();

    // add model to viewer.
    viewer.setSceneData( node );

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
