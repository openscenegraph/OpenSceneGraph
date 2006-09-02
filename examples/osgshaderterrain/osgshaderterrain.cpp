#include <osg/AlphaFunc>
#include <osg/Billboard>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/GL2Extensions>
#include <osg/Material>
#include <osg/Math>
#include <osg/MatrixTransform>
#include <osg/PolygonOffset>
#include <osg/Program>
#include <osg/Projection>
#include <osg/Shader>
#include <osg/ShapeDrawable>
#include <osg/StateSet>
#include <osg/Switch>
#include <osg/Texture2D>
#include <osg/Uniform>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

#include <osgUtil/IntersectVisitor>
#include <osgUtil/SmoothingVisitor>

#include <osgText/Text>

#include <osgProducer/Viewer>

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

        Tree(const osg::Vec3& position, const osg::Vec4ub& color, float width, float height, unsigned int type):
            _position(position),
            _color(color),
            _width(width),
            _height(height),
            _type(type) {}

        osg::Vec3       _position;
        osg::Vec4ub     _color;
        float           _width;
        float           _height;
        unsigned int    _type;
    };
    
    typedef std::vector< osg::ref_ptr<Tree> > TreeList;
    
    class Cell : public osg::Referenced
    {
    public:
        typedef std::vector< osg::ref_ptr<Cell> > CellList;

        Cell():_parent(0) {}
        Cell(osg::BoundingBox& bb):_parent(0), _bb(bb) {}
        
        void addCell(Cell* cell) { cell->_parent=this; _cells.push_back(cell); }

        void addTree(Tree* tree) { _trees.push_back(tree); }
        
        void addTrees(const TreeList& trees) { _trees.insert(_trees.end(),trees.begin(),trees.end()); }
        
        void computeBound();
        
        bool contains(const osg::Vec3& position) const { return _bb.contains(position); }
        
        bool divide(unsigned int maxNumTreesPerCell=10);
        
        bool devide(bool xAxis, bool yAxis, bool zAxis);
        
        void bin();


        Cell*               _parent;
        osg::BoundingBox    _bb;
        CellList            _cells;
        TreeList            _trees;
        
    };

    float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }
    int random(int min,int max) { return min + (int)((float)(max-min)*(float)rand()/(float)RAND_MAX); }

    osg::Geode* createTerrain(const osg::Vec3& origin, const osg::Vec3& size);

    void createTreeList(osg::Node* terrain,const osg::Vec3& origin, const osg::Vec3& size,unsigned int numTreesToCreate,TreeList& trees);

    osg::Geometry* createOrthogonalQuadsNoColor( const osg::Vec3& pos, float w, float h );

    osg::Node* createShaderGraph(Cell* cell,osg::StateSet* stateset);

    osg::Node* createScene(unsigned int numTreesToCreates);
    

};

// event handler to capture keyboard events and use them to advance the technique used for rendering
void ForestTechniqueManager::Cell::computeBound()
{
    _bb.init();
    for(CellList::iterator citr=_cells.begin();
        citr!=_cells.end();
        ++citr)
    {
        (*citr)->computeBound();
        _bb.expandBy((*citr)->_bb);
    }

    for(TreeList::iterator titr=_trees.begin();
        titr!=_trees.end();
        ++titr)
    {
        _bb.expandBy((*titr)->_position);
    }
}

bool ForestTechniqueManager::Cell::divide(unsigned int maxNumTreesPerCell)
{

    if (_trees.size()<=maxNumTreesPerCell) return false;

    computeBound();

    float radius = _bb.radius();
    float divide_distance = radius*0.7f;
    if (devide((_bb.xMax()-_bb.xMin())>divide_distance,(_bb.yMax()-_bb.yMin())>divide_distance,(_bb.zMax()-_bb.zMin())>divide_distance))
    {
        // recusively divide the new cells till maxNumTreesPerCell is met.
        for(CellList::iterator citr=_cells.begin();
            citr!=_cells.end();
            ++citr)
        {
            (*citr)->divide(maxNumTreesPerCell);
        }
        return true;
   }
   else
   {
        return false;
   }
}

bool ForestTechniqueManager::Cell::devide(bool xAxis, bool yAxis, bool zAxis)
{
    if (!(xAxis || yAxis || zAxis)) return false;

    if (_cells.empty())
        _cells.push_back(new Cell(_bb));

    if (xAxis)
    {
        unsigned int numCellsToDivide=_cells.size();
        for(unsigned int i=0;i<numCellsToDivide;++i)
        {
            Cell* orig_cell = _cells[i].get();
            Cell* new_cell = new Cell(orig_cell->_bb);

            float xCenter = (orig_cell->_bb.xMin()+orig_cell->_bb.xMax())*0.5f;
            orig_cell->_bb.xMax() = xCenter;
            new_cell->_bb.xMin() = xCenter;

            _cells.push_back(new_cell);
        }
    }

    if (yAxis)
    {
        unsigned int numCellsToDivide=_cells.size();
        for(unsigned int i=0;i<numCellsToDivide;++i)
        {
            Cell* orig_cell = _cells[i].get();
            Cell* new_cell = new Cell(orig_cell->_bb);

            float yCenter = (orig_cell->_bb.yMin()+orig_cell->_bb.yMax())*0.5f;
            orig_cell->_bb.yMax() = yCenter;
            new_cell->_bb.yMin() = yCenter;

            _cells.push_back(new_cell);
        }
    }

    if (zAxis)
    {
        unsigned int numCellsToDivide=_cells.size();
        for(unsigned int i=0;i<numCellsToDivide;++i)
        {
            Cell* orig_cell = _cells[i].get();
            Cell* new_cell = new Cell(orig_cell->_bb);

            float zCenter = (orig_cell->_bb.zMin()+orig_cell->_bb.zMax())*0.5f;
            orig_cell->_bb.zMax() = zCenter;
            new_cell->_bb.zMin() = zCenter;

            _cells.push_back(new_cell);
        }
    }

    bin();

    return true;

}

void ForestTechniqueManager::Cell::bin()
{   
    // put trees in apprpriate cells.
    TreeList treesNotAssigned;
    for(TreeList::iterator titr=_trees.begin();
        titr!=_trees.end();
        ++titr)
    {
        Tree* tree = titr->get();
        bool assigned = false;
        for(CellList::iterator citr=_cells.begin();
            citr!=_cells.end() && !assigned;
            ++citr)
        {
            if ((*citr)->contains(tree->_position))
            {
                (*citr)->addTree(tree);
                assigned = true;
            }
        }
        if (!assigned) treesNotAssigned.push_back(tree);
    }

    // put the unassigned trees back into the original local tree list.
    _trees.swap(treesNotAssigned);


    // prune empty cells.
    CellList cellsNotEmpty;
    for(CellList::iterator citr=_cells.begin();
        citr!=_cells.end();
        ++citr)
    {
        if (!((*citr)->_trees.empty()))
        {
            cellsNotEmpty.push_back(*citr);
        }
    }
    _cells.swap(cellsNotEmpty);


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

osg::Geometry* ForestTechniqueManager::createOrthogonalQuadsNoColor( const osg::Vec3& pos, float w, float h)
{
    // set up the coords
    osg::Vec3Array& v = *(new osg::Vec3Array(8));
    osg::Vec2Array& t = *(new osg::Vec2Array(8));
    
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

    geom->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,8) );

    return geom;
}

class ShaderGeometry : public osg::Drawable
{
    public:
        ShaderGeometry() { setUseDisplayList(false); }

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        ShaderGeometry(const ShaderGeometry& ShaderGeometry,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
            osg::Drawable(ShaderGeometry,copyop) {}

        META_Object(osg,ShaderGeometry)

        typedef std::vector<osg::Vec4> PositionSizeList;
        
        virtual void drawImplementation(osg::State& state) const
        {
            for(PositionSizeList::const_iterator itr = _trees.begin();
                itr != _trees.end();
                ++itr)
            {
                glColor4fv(itr->ptr());
                _geometry->draw(state);
            }
        }

        virtual osg::BoundingBox computeBound() const
        {
            osg::BoundingBox geom_box = _geometry->getBound();
            osg::BoundingBox bb;
            for(PositionSizeList::const_iterator itr = _trees.begin();
                itr != _trees.end();
                ++itr)
            {
                bb.expandBy(geom_box.corner(0)*(*itr)[3] +
                            osg::Vec3( (*itr)[0], (*itr)[1], (*itr)[2] ));
                bb.expandBy(geom_box.corner(7)*(*itr)[3] +
                            osg::Vec3( (*itr)[0], (*itr)[1], (*itr)[2] ));
            }
            return bb;
        }
        
        void setGeometry(osg::Geometry* geometry)
        {
            _geometry = geometry;
        }
        
        void addTree(ForestTechniqueManager::Tree& tree)
        {
            _trees.push_back(osg::Vec4(tree._position.x(), tree._position.y(), tree._position.z(), tree._height));
        }
        
        osg::ref_ptr<osg::Geometry> _geometry;

        PositionSizeList _trees;

    protected:
    
        virtual ~ShaderGeometry() {}
        
};

osg::Geometry* shared_geometry = 0;

osg::Node* ForestTechniqueManager::createShaderGraph(Cell* cell,osg::StateSet* stateset)
{
    if (shared_geometry==0)
    {
        shared_geometry = createOrthogonalQuadsNoColor(osg::Vec3(0.0f,0.0f,0.0f),1.0f,1.0f);
        //shared_geometry->setUseDisplayList(false);
    }


    bool needGroup = !(cell->_cells.empty());
    bool needTrees = !(cell->_trees.empty());
    
    osg::Geode* geode = 0;
    osg::Group* group = 0;
    
    if (needTrees)
    {
        geode = new osg::Geode;
        
        ShaderGeometry* shader_geometry = new ShaderGeometry;
        shader_geometry->setGeometry(shared_geometry);
        
        
        for(TreeList::iterator itr=cell->_trees.begin();
            itr!=cell->_trees.end();
            ++itr)
        {
            Tree& tree = **itr;
            shader_geometry->addTree(tree);

        }

        geode->setStateSet(stateset);
        geode->addDrawable(shader_geometry);
    }
    
    if (needGroup)
    {
        group = new osg::Group;
        for(Cell::CellList::iterator itr=cell->_cells.begin();
            itr!=cell->_cells.end();
            ++itr)
        {
            group->addChild(createShaderGraph(itr->get(),stateset));
        }
        
        if (geode) group->addChild(geode);
        
    }
    if (group) return group;
    else return geode;
}

osg::Node* ForestTechniqueManager::createScene(unsigned int /*numTreesToCreates*/)
{
    osg::Group* scene = new osg::Group;
    
    unsigned int numColumns = 38;
    unsigned int numRows = 39;
    unsigned int r;
    unsigned int c;

    osg::Vec3 origin(0.0f,0.0f,0.0f);
    osg::Vec3 size(1000.0f,1000.0f,250.0f);
    osg::Vec3 scaleDown(1.0f/size.x(),1.0f/size.y(),1.0f/size.z());

    // ---------------------------------------
    // Set up a StateSet to texture the objects
    // ---------------------------------------
    osg::StateSet* stateset = new osg::StateSet();


    osg::Uniform* originUniform = new osg::Uniform("terrainOrigin",origin);
    stateset->addUniform(originUniform);

    osg::Uniform* sizeUniform = new osg::Uniform("terrainSize",size);
    stateset->addUniform(sizeUniform);

    osg::Uniform* scaleDownUniform = new osg::Uniform("terrainScaleDown",scaleDown);
    stateset->addUniform(scaleDownUniform);

    osg::Uniform* terrainTextureSampler = new osg::Uniform("terrainTexture",0);
    stateset->addUniform(terrainTextureSampler);

    osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",1);
    stateset->addUniform(baseTextureSampler);

    osg::Uniform* treeTextureSampler = new osg::Uniform("treeTexture",1);
    stateset->addUniform(treeTextureSampler);


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

    osg::Image* terrainImage = new osg::Image;
    terrainImage->allocateImage(numColumns,numRows,1,GL_LUMINANCE, GL_FLOAT);
    terrainImage->setInternalTextureFormat(GL_LUMINANCE_FLOAT32_ATI);
    for(r=0;r<numRows;++r)
    {
        for(c=0;c<numColumns;++c)
        {
            *((float*)(terrainImage->data(c,r))) = (vertex[r+c*numRows][2]-min_z)*scale_z;
        }
    }
    
    osg::Texture2D* terrainTexture = new osg::Texture2D;
    terrainTexture->setImage(terrainImage);
    terrainTexture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
    terrainTexture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
    terrainTexture->setResizeNonPowerOfTwoHint(false);
    stateset->setTextureAttributeAndModes(0,terrainTexture,osg::StateAttribute::ON);


    osg::Image* image = osgDB::readImageFile("Images/lz.rgb");
    if (image)
    {
        osg::Texture2D* texture = new osg::Texture2D;
        
        texture->setImage(image);
        stateset->setTextureAttributeAndModes(1,texture,osg::StateAttribute::ON);
    }

    {    
        std::cout<<"Creating terrain...";

        osg::Geode* geode = new osg::Geode();
        geode->setStateSet( stateset );


        {
            osg::Program* program = new osg::Program;
            stateset->setAttribute(program);

#if 1
            // use inline shaders
            
            ///////////////////////////////////////////////////////////////////
            // vertex shader using just Vec4 coefficients
            char vertexShaderSource[] = 
               "uniform float osg_FrameTime;\n"
               "uniform sampler2D terrainTexture;\n"
               "uniform vec3 terrainOrigin;\n"
               "uniform vec3 terrainScaleDown;\n"
               "\n"
               "varying vec2 texcoord;\n"
               "\n"
               "void main(void)\n"
               "{\n"
               "    texcoord = gl_Vertex.xy - terrainOrigin.xy;\n"
               "    texcoord.x *= terrainScaleDown.x;\n"
               "    texcoord.y *= terrainScaleDown.y;\n"
               "\n"
               "    vec4 position;\n"
               "    position.x = gl_Vertex.x;\n"
               "    position.y = gl_Vertex.y;\n"
               "    position.z = texture2D(terrainTexture, texcoord).r;\n"
               "    position.w = 1.0;\n"
               " \n"
               "    gl_Position     = gl_ModelViewProjectionMatrix * position;\n"
                "   gl_FrontColor = vec4(1.0,1.0,1.0,1.0);\n"
               "}\n";

            //////////////////////////////////////////////////////////////////
            // fragment shader
            //
            char fragmentShaderSource[] = 
                "uniform sampler2D baseTexture; \n"
                "varying vec2 texcoord;\n"
                "\n"
                "void main(void) \n"
                "{\n"
                "    gl_FragColor = texture2D( baseTexture, texcoord); \n"
                "}\n";

            program->addShader(new osg::Shader(osg::Shader::VERTEX, vertexShaderSource));
            program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource));
            
#else

            // get shaders from source
            program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("shaders/terrain.vert")));
            program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("shaders/terrain.frag")));

#endif

            // get shaders from source
        }


        {
            osg::Geometry* geometry = new osg::Geometry;

            osg::Vec3Array& v = *(new osg::Vec3Array(numColumns*numRows));
            osg::Vec4ubArray& color = *(new osg::Vec4ubArray(1));

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
                    v[vi].set(pos.x(),pos.y(),pos.z());
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
            
            geometry->setInitialBound(osg::BoundingBox(origin, origin+size));

            geode->addDrawable(geometry);

            scene->addChild(geode);
        }
    }
        
    std::cout<<"done."<<std::endl;
    
#if 0
    std::cout<<"Creating tree locations...";std::cout.flush();
    TreeList trees;
    createTreeList(0,origin,size,numTreesToCreates,trees);
    std::cout<<"done."<<std::endl;
    
    std::cout<<"Creating cell subdivision...";
    osg::ref_ptr<Cell> cell = new Cell;
    cell->addTrees(trees);
    cell->divide();
     std::cout<<"done."<<std::endl;
   
    
    osg::Texture2D *tex = new osg::Texture2D;
    tex->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP );
    tex->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP );
    tex->setImage(osgDB::readImageFile("Images/tree0.rgba"));
    tex->setResizeNonPowerOfTwoHint(false);

    osg::StateSet *dstate = new osg::StateSet;
    {    
        dstate->setTextureAttributeAndModes(1, tex, osg::StateAttribute::ON );

        dstate->setAttributeAndModes( new osg::BlendFunc, osg::StateAttribute::ON );

        osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
        alphaFunc->setFunction(osg::AlphaFunc::GEQUAL,0.05f);
        dstate->setAttributeAndModes( alphaFunc, osg::StateAttribute::ON );

        dstate->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

        dstate->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    }
    

    {
        osg::StateSet* stateset = new osg::StateSet;

        stateset->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON );
        stateset->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

        {
            osg::Program* program = new osg::Program;
            stateset->setAttribute(program);

            // get shaders from source
            program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("shaders/forest2.vert")));
            program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("shaders/forest2.frag")));
        }

        std::cout<<"Creating billboard based forest...";
        scene->addChild(createShaderGraph(cell.get(),stateset));

    }
#endif

    return scene;
}

class TestSupportCallback : public osgProducer::OsgCameraGroup::RealizeCallback
{
    public:
        TestSupportCallback():_supported(true),_errorMessage() {}
        
        virtual void operator()( osgProducer::OsgCameraGroup&, osgProducer::OsgSceneHandler& sh, const Producer::RenderSurface& )
        {
            {
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

                unsigned int contextID = sh.getSceneView()->getState()->getContextID();
                osg::GL2Extensions* gl2ext = osg::GL2Extensions::Get(contextID,true);
                if( gl2ext )
                {
                    if( !gl2ext->isGlslSupported() )
                    {
                        _supported = false;
                        _errorMessage = "ERROR: GLSL not supported by OpenGL driver.";
                    }

                    GLint numVertexTexUnits = 0;
                    glGetIntegerv( GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &numVertexTexUnits );
                    if( numVertexTexUnits <= 0 )
                    {
                        _supported = false;
                        _errorMessage = "ERROR: vertex texturing not supported by OpenGL driver.";
                    }
                }
            }
                        
            sh.init();
        }
        
        OpenThreads::Mutex  _mutex;
        bool                _supported;
        std::string         _errorMessage;
        
};

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates the osg::Shape classes.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--trees <number>","Set the number of trees to create");
   
    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    float numTreesToCreates = 10000;
    arguments.read("--trees",numTreesToCreates);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);
    
    osg::ref_ptr<ForestTechniqueManager> ttm = new ForestTechniqueManager;
    
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
    
    osg::Node* node = ttm->createScene((unsigned int)numTreesToCreates);

    // add model to viewer.
    viewer.setSceneData( node );

    // register a test extension callback to be called when app realizes and gets a valid graphics context
    osg::ref_ptr<TestSupportCallback> testSupportCallback = new TestSupportCallback();
    viewer.setRealizeCallback(testSupportCallback.get());

    // create the windows and run the threads.
    viewer.realize();

    // exit if we don't have the extensions this example needs.
    if (!testSupportCallback->_supported)
    {
        osg::notify(osg::WARN)<<testSupportCallback->_errorMessage<<std::endl;

        exit(1);
    }

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
