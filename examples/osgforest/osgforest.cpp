/* OpenSceneGraph example, osgforest.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osg/AlphaFunc>
#include <osg/Billboard>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/Math>
#include <osg/MatrixTransform>
#include <osg/PolygonOffset>
#include <osg/Projection>
#include <osg/ShapeDrawable>
#include <osg/StateSet>
#include <osg/Switch>
#include <osg/Texture2D>
#include <osg/TextureBuffer>
#include <osg/Image>
#include <osg/TexEnv>
#include <osg/VertexProgram>
#include <osg/FragmentProgram>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>

#include <osgUtil/LineSegmentIntersector>
#include <osgUtil/IntersectionVisitor>
#include <osgUtil/SmoothingVisitor>

#include <osgText/Text>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/StateSetManipulator>

#include <iostream>
#include <sstream>

// for the grid data..
#include "../osghangglide/terrain_coords.h"

// class to create the forest and manage the movement between various techniques.
class ForestTechniqueManager : public osg::Referenced
{
public:

    ForestTechniqueManager() {}

    enum Features
    {
        HUD_TEXT = 1,
        BILLBOARD_GRAPH = 2,
        X_GRAPH = 4,
        TRANSFORM_GRAPH = 8,
        SHADER_GRAPH = 16,
        GEOMETRY_SHADER_GRAPH = 32,
        TEXTURE_BUFFER_GRAPH = 64,
        ALL_FEATURES = (TEXTURE_BUFFER_GRAPH<<1)-1
    };

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

        bool divide(bool xAxis, bool yAxis, bool zAxis);

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

    osg::Geometry* createSprite( float w, float h, osg::Vec4ub color );

    osg::Geometry* createOrthogonalQuads( const osg::Vec3& pos, float w, float h, osg::Vec4ub color );
    osg::Geometry* createOrthogonalQuadsNoColor( const osg::Vec3& pos, float w, float h );

    osg::Node* createBillboardGraph(Cell* cell,osg::StateSet* stateset);

    osg::Node* createXGraph(Cell* cell,osg::StateSet* stateset);

    osg::Node* createTransformGraph(Cell* cell,osg::StateSet* stateset);

    osg::Node* createShaderGraph(Cell* cell,osg::StateSet* stateset);

    osg::Node* createGeometryShaderGraph(Cell* cell, osg::StateSet* stateset);

    osg::Node* createTextureBufferGraph(Cell* cell, osg::Geometry* templateGeometry);

    void CollectTreePositions(Cell* cell, std::vector< osg::Vec3 >& positions);

    osg::Node* createHUDWithText(const std::string& text);

    osg::Node* createScene(unsigned int numTreesToCreates, unsigned int maxNumTreesPerCell, unsigned int mask=ALL_FEATURES);

    void advanceToNextTechnique(int delta=1)
    {
        if (_techniqueSwitch.valid())
        {
            _currentTechnique += delta;
            if (_currentTechnique<0)
                _currentTechnique = _techniqueSwitch->getNumChildren()-1;
            if (_currentTechnique>=(int)_techniqueSwitch->getNumChildren())
                _currentTechnique = 0;
            _techniqueSwitch->setSingleChildOn(_currentTechnique);
        }
    }

    osg::ref_ptr<osg::Switch>   _techniqueSwitch;
    int                         _currentTechnique;


};

// event handler to capture keyboard events and use them to advance the technique used for rendering
class TechniqueEventHandler : public osgGA::GUIEventHandler
{
public:

    TechniqueEventHandler(ForestTechniqueManager* ttm=0) { _ForestTechniqueManager = ttm; }

    META_Object(osgforestApp,TechniqueEventHandler);

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&, osg::Object*, osg::NodeVisitor*);

    virtual void getUsage(osg::ApplicationUsage& usage) const;

protected:

    ~TechniqueEventHandler() {}

    TechniqueEventHandler(const TechniqueEventHandler&,const osg::CopyOp&) {}

    osg::ref_ptr<ForestTechniqueManager> _ForestTechniqueManager;


};

bool TechniqueEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&, osg::Object*, osg::NodeVisitor*)
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
    if (divide((_bb.xMax()-_bb.xMin())>divide_distance,(_bb.yMax()-_bb.yMin())>divide_distance,(_bb.zMax()-_bb.zMin())>divide_distance))
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

bool ForestTechniqueManager::Cell::divide(bool xAxis, bool yAxis, bool zAxis)
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
    // put trees in appropriate cells.
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

osg::Geode* ForestTechniqueManager::createTerrain(const osg::Vec3& origin, const osg::Vec3& size)
{
    osg::Geode* geode = new osg::Geode();

    // ---------------------------------------
    // Set up a StateSet to texture the objects
    // ---------------------------------------
    osg::StateSet* stateset = new osg::StateSet();

    osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile("Images/lz.rgb");
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

        osg::HeightField* grid = new osg::HeightField;
        grid->allocate(numColumns,numRows);
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
        geometry->setColorArray(&color, osg::Array::BIND_OVERALL);
        geometry->setTexCoordArray(0,&t);
        geometry->setUseDisplayList(false);
        geometry->setUseVertexBufferObjects(true);

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
            osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector =
                new osgUtil::LineSegmentIntersector(tree->_position,tree->_position+osg::Vec3(0.0f,0.0f,size.z()));

            osgUtil::IntersectionVisitor iv(intersector.get());

            terrain->accept(iv);

            if (intersector->containsIntersections())
            {
                osgUtil::LineSegmentIntersector::Intersections& intersections = intersector->getIntersections();
                for(osgUtil::LineSegmentIntersector::Intersections::iterator itr = intersections.begin();
                    itr != intersections.end();
                    ++itr)
                {
                    const osgUtil::LineSegmentIntersector::Intersection& intersection = *itr;
                    tree->_position = intersection.getWorldIntersectPoint();
                }
            }
        }

        trees.push_back(tree);
    }
}

osg::Geometry* ForestTechniqueManager::createSprite( float w, float h, osg::Vec4ub color )
{
    // set up the coords
    osg::Vec3Array& v = *(new osg::Vec3Array(4));
    osg::Vec2Array& t = *(new osg::Vec2Array(4));
    osg::Vec4ubArray& c = *(new osg::Vec4ubArray(1));

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

    geom->setColorArray( &c, osg::Array::BIND_OVERALL );

    geom->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4) );

    return geom;
}

osg::Geometry* ForestTechniqueManager::createOrthogonalQuads( const osg::Vec3& pos, float w, float h, osg::Vec4ub color )
{
    // set up the coords
    osg::Vec3Array& v = *(new osg::Vec3Array(8));
    osg::Vec2Array& t = *(new osg::Vec2Array(8));
    osg::Vec4ubArray& c = *(new osg::Vec4ubArray(1));

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

    geom->setColorArray( &c, osg::Array::BIND_OVERALL );

    geom->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,8) );

    return geom;
}

osg::Node* ForestTechniqueManager::createBillboardGraph(Cell* cell,osg::StateSet* stateset)
{
    bool needGroup = !(cell->_cells.empty());
    bool needBillboard = !(cell->_trees.empty());

    osg::Billboard* billboard = 0;
    osg::Group* group = 0;

    if (needBillboard)
    {
        billboard = new osg::Billboard;
        billboard->setStateSet(stateset);
        for(TreeList::iterator itr=cell->_trees.begin();
            itr!=cell->_trees.end();
            ++itr)
        {
            Tree& tree = **itr;
            billboard->addDrawable(createSprite(tree._width,tree._height,tree._color),tree._position);
        }
    }

    if (needGroup)
    {
        group = new osg::Group;
        for(Cell::CellList::iterator itr=cell->_cells.begin();
            itr!=cell->_cells.end();
            ++itr)
        {
            group->addChild(createBillboardGraph(itr->get(),stateset));
        }

        if (billboard) group->addChild(billboard);

    }
    if (group) return group;
    else return billboard;
}

osg::Node* ForestTechniqueManager::createXGraph(Cell* cell,osg::StateSet* stateset)
{
    bool needGroup = !(cell->_cells.empty());
    bool needTrees = !(cell->_trees.empty());

    osg::Geode* geode = 0;
    osg::Group* group = 0;

    if (needTrees)
    {
        geode = new osg::Geode;
        geode->setStateSet(stateset);

        for(TreeList::iterator itr=cell->_trees.begin();
            itr!=cell->_trees.end();
            ++itr)
        {
            Tree& tree = **itr;
            geode->addDrawable(createOrthogonalQuads(tree._position,tree._width,tree._height,tree._color));
        }
    }

    if (needGroup)
    {
        group = new osg::Group;
        for(Cell::CellList::iterator itr=cell->_cells.begin();
            itr!=cell->_cells.end();
            ++itr)
        {
            group->addChild(createXGraph(itr->get(),stateset));
        }

        if (geode) group->addChild(geode);

    }
    if (group) return group;
    else return geode;
}

osg::Node* ForestTechniqueManager::createTransformGraph(Cell* cell,osg::StateSet* stateset)
{
    bool needGroup = !(cell->_cells.empty());
    bool needTrees = !(cell->_trees.empty());

    osg::Group* transform_group = 0;
    osg::Group* group = 0;

    if (needTrees)
    {
        transform_group = new osg::Group;

        osg::Geometry* geometry = createOrthogonalQuads(osg::Vec3(0.0f,0.0f,0.0f),1.0f,1.0f,osg::Vec4ub(255,255,255,255));

        for(TreeList::iterator itr=cell->_trees.begin();
            itr!=cell->_trees.end();
            ++itr)
        {
            Tree& tree = **itr;
            osg::MatrixTransform* transform = new osg::MatrixTransform;
            transform->setMatrix(osg::Matrix::scale(tree._width,tree._width,tree._height)*osg::Matrix::translate(tree._position));

            osg::Geode* geode = new osg::Geode;
            geode->setStateSet(stateset);
            geode->addDrawable(geometry);
            transform->addChild(geode);
            transform_group->addChild(transform);
        }
    }

    if (needGroup)
    {
        group = new osg::Group;
        for(Cell::CellList::iterator itr=cell->_cells.begin();
            itr!=cell->_cells.end();
            ++itr)
        {
            group->addChild(createTransformGraph(itr->get(),stateset));
        }

        if (transform_group) group->addChild(transform_group);

    }
    if (group) return group;
    else return transform_group;
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
        ShaderGeometry(const ShaderGeometry& sg,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
            osg::Drawable(sg,copyop) {}

        META_Object(osg,ShaderGeometry)

        typedef std::vector<osg::Vec4> PositionSizeList;

        virtual void resizeGLObjectBuffers(unsigned int maxSize)
        {
            osg::Drawable::resizeGLObjectBuffers(maxSize);
            if (_geometry) _geometry->resizeGLObjectBuffers(maxSize);
        }

        virtual void releaseGLObjects(osg::State* state) const
        {
            osg::Drawable::releaseGLObjects(state);
            if (_geometry) _geometry->releaseGLObjects(state);
        }

        virtual void drawImplementation(osg::RenderInfo& renderInfo) const
        {
            for(PositionSizeList::const_iterator itr = _trees.begin();
                itr != _trees.end();
                ++itr)
            {
                renderInfo.getState()->Color((*itr)[0],(*itr)[1],(*itr)[2],(*itr)[3]);
                _geometry->draw(renderInfo);
            }
        }

        virtual osg::BoundingBox computeBoundingBox() const
        {
            osg::BoundingBox geom_box = _geometry->getBoundingBox();
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

osg::Program* createGeometryShader()
{
    static const char* vertSource = {
    "#version 120\n"
    "#extension GL_EXT_geometry_shader4 : enable\n"
    "varying vec2 texcoord;\n"
    "void main(void)\n"
    "{\n"
    "    gl_Position = gl_Vertex;\n"
    "    texcoord = gl_MultiTexCoord0.st;\n"
    "}\n"
    };

    static const char* geomSource = {
    "#version 120\n"
    "#extension GL_EXT_geometry_shader4 : enable\n"
    "varying vec2 texcoord;\n"
    "varying float intensity; \n"
    "varying float red_intensity; \n"
    "void main(void)\n"
    "{\n"
    "    vec4 v = gl_PositionIn[0];\n"
    "    vec4 info = gl_PositionIn[1];\n"
    "    intensity = info.y;\n"
    "    red_intensity = info.z;\n"
    "\n"
    "    float h = info.x;\n"
    "    float w = h*0.35;\n"
    "    vec4 e;\n"
    "    e = v + vec4(-w,0.0,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; texcoord = vec2(0.0,0.0); EmitVertex();\n"
    "    e = v + vec4(w,0.0,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e;  texcoord = vec2(1.0,0.0); EmitVertex();\n"
    "    e = v + vec4(-w,0.0,h,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e;  texcoord = vec2(0.0,1.0); EmitVertex();\n"
    "    e = v + vec4(w,0.0,h,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e;  texcoord = vec2(1.0,1.0); EmitVertex();\n"
    "    EndPrimitive();\n"
    "    e = v + vec4(0.0,-w,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; texcoord = vec2(0.0,0.0); EmitVertex();\n"
    "    e = v + vec4(0.0,w,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e;  texcoord = vec2(1.0,0.0); EmitVertex();\n"
    "    e = v + vec4(0.0,-w,h,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e;  texcoord = vec2(0.0,1.0); EmitVertex();\n"
    "    e = v + vec4(0.0,w,h,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e;  texcoord = vec2(1.0,1.0); EmitVertex();\n"
    "    EndPrimitive();\n"
    "}\n"
    };


    static const char* fragSource = {
        "uniform sampler2D baseTexture; \n"
        "varying vec2 texcoord; \n"
        "varying float intensity; \n"
        "varying float red_intensity; \n"
        "\n"
        "void main(void) \n"
        "{ \n"
        "   vec4 finalColor = texture2D( baseTexture, texcoord); \n"
        "   vec4 color = finalColor * intensity;\n"
        "   color.w = finalColor.w;\n"
        "   color.x *= red_intensity;\n"
        "   gl_FragColor = color;\n"
        "}\n"
    };


    osg::Program* pgm = new osg::Program;
    pgm->setName( "osgshader2 demo" );

    pgm->addShader( new osg::Shader( osg::Shader::VERTEX,   vertSource ) );
    pgm->addShader( new osg::Shader( osg::Shader::FRAGMENT, fragSource ) );

    pgm->addShader( new osg::Shader( osg::Shader::GEOMETRY, geomSource ) );
    pgm->setParameter( GL_GEOMETRY_VERTICES_OUT_EXT, 8 );
    pgm->setParameter( GL_GEOMETRY_INPUT_TYPE_EXT, GL_LINES );
    pgm->setParameter( GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);

    return pgm;
}

void ForestTechniqueManager::CollectTreePositions(Cell* cell, std::vector< osg::Vec3 >& positions)
{
    bool needGroup = !(cell->_cells.empty());
    bool needTrees = !(cell->_trees.empty());

    if (needTrees)
    {
        for(TreeList::iterator itr=cell->_trees.begin();
            itr!=cell->_trees.end();
            ++itr)
        {
            Tree& tree = **itr;
            positions.push_back(tree._position);
        }
    }

    if (needGroup)
    {
        for(Cell::CellList::iterator itr=cell->_cells.begin();
            itr!=cell->_cells.end();
            ++itr)
        {
            CollectTreePositions(itr->get(),positions);
        }

    }
}

osg::Node* ForestTechniqueManager::createGeometryShaderGraph(Cell* cell, osg::StateSet* dstate)
{
    bool needGroup = !(cell->_cells.empty());
    bool needTrees = !(cell->_trees.empty());

    osg::Geode* geode = 0;
    osg::Group* group = 0;

    if (needTrees)
    {
        geode = new osg::Geode;
        geode->setStateSet(dstate);

        osg::Geometry* geometry = new osg::Geometry;
        geode->addDrawable(geometry);

        osg::Vec3Array* v = new osg::Vec3Array;

        for(TreeList::iterator itr=cell->_trees.begin();
            itr!=cell->_trees.end();
            ++itr)
        {
            Tree& tree = **itr;
            v->push_back(tree._position);
            v->push_back(osg::Vec3(/*tree._height*/30.0,(double)random(0.75f,1.15f),(double)random(1.0f,1.250f)));
        }
        geometry->setVertexArray( v );
        geometry->addPrimitiveSet( new osg::DrawArrays( GL_LINES, 0, v->size() ) );

        osg::StateSet* sset = geode->getOrCreateStateSet();
        sset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
        sset->setAttribute( createGeometryShader() );

        osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
        sset->addUniform(baseTextureSampler);

    }

    if (needGroup)
    {
        group = new osg::Group;
        for(Cell::CellList::iterator itr=cell->_cells.begin();
            itr!=cell->_cells.end();
            ++itr)
        {
            group->addChild(createGeometryShaderGraph(itr->get(),dstate));
        }

        if (geode) group->addChild(geode);

    }
    if (group) return group;
    else return geode;
}

osg::Node* ForestTechniqueManager::createTextureBufferGraph(Cell* cell, osg::Geometry* templateGeometry)
{
    bool needGroup = !(cell->_cells.empty());
    bool needTrees = !(cell->_trees.empty());

    osg::Geode* geode = 0;
    osg::Group* group = 0;

    if (needTrees)
    {
        osg::Geometry* geometry = (osg::Geometry*)templateGeometry->clone( osg::CopyOp::DEEP_COPY_PRIMITIVES );
        osg::DrawArrays* primSet = dynamic_cast<osg::DrawArrays*>( geometry->getPrimitiveSet(0) );
        primSet->setNumInstances( cell->_trees.size() );
        geode = new osg::Geode;
        geode->addDrawable(geometry);

        osg::ref_ptr<osg::Image> treeParamsImage = new osg::Image;
        treeParamsImage->allocateImage( 3*cell->_trees.size(), 1, 1, GL_RGBA, GL_FLOAT );

        unsigned int i=0;
        for(TreeList::iterator itr=cell->_trees.begin();
            itr!=cell->_trees.end();
            ++itr,++i)
        {
            osg::Vec4f* ptr = (osg::Vec4f*)treeParamsImage->data(3*i);
            Tree& tree = **itr;
            ptr[0] = osg::Vec4f(tree._position.x(),tree._position.y(),tree._position.z(),1.0);
            ptr[1] = osg::Vec4f((float)tree._color.r()/255.0f,(float)tree._color.g()/255.0f, (float)tree._color.b()/255.0f, 1.0);
            ptr[2] = osg::Vec4f(tree._width, tree._height, 1.0, 1.0);
        }
        osg::ref_ptr<osg::TextureBuffer> tbo = new osg::TextureBuffer;
        tbo->setImage( treeParamsImage.get() );
        tbo->setInternalFormat(GL_RGBA32F_ARB);
        geometry->getOrCreateStateSet()->setTextureAttribute(1, tbo.get());
        geometry->setInitialBound( cell->_bb );
    }

    if (needGroup)
    {
        group = new osg::Group;
        for(Cell::CellList::iterator itr=cell->_cells.begin();
            itr!=cell->_cells.end();
            ++itr)
        {
            group->addChild(createTextureBufferGraph(itr->get(),templateGeometry));
        }

        if (geode) group->addChild(geode);

    }
    if (group) return group;
    else return geode;
}


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

osg::Node* ForestTechniqueManager::createHUDWithText(const std::string& str)
{
    osg::Geode* geode = new osg::Geode();

    std::string timesFont("fonts/arial.ttf");

    // turn lighting off for the text and disable depth test to ensure its always ontop.
    osg::StateSet* stateset = geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    // or disable depth test, and make sure that the hud is drawn after everything
    // else so that it always appears ontop.
    stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    stateset->setRenderBinDetails(11,"RenderBin");

    osg::Vec3 position(150.0f,800.0f,0.0f);
    osg::Vec3 delta(0.0f,-120.0f,0.0f);

    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );

        text->setFont(timesFont);
        text->setPosition(position);
        text->setText(str);

        position += delta;
    }


    // create the hud.
    osg::MatrixTransform* modelview_abs = new osg::MatrixTransform;
    modelview_abs->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    modelview_abs->setMatrix(osg::Matrix::identity());
    modelview_abs->addChild(geode);

    osg::Projection* projection = new osg::Projection;
    projection->setMatrix(osg::Matrix::ortho2D(0,1280,0,1024));
    projection->addChild(modelview_abs);

    return projection;
}

osg::Node* ForestTechniqueManager::createScene(unsigned int numTreesToCreates, unsigned int maxNumTreesPerCell, unsigned int mask)
{
    osg::Vec3 origin(0.0f,0.0f,0.0f);
    osg::Vec3 size(1000.0f,1000.0f,200.0f);

    std::cout<<"Creating terrain...";
    osg::ref_ptr<osg::Node> terrain = createTerrain(origin,size);
    std::cout<<"done."<<std::endl;

    std::cout<<"Creating tree locations...";std::cout.flush();
    TreeList trees;
    createTreeList(terrain.get(),origin,size,numTreesToCreates,trees);
    std::cout<<"done."<<std::endl;

    std::cout<<"Creating cell subdivision...";
    osg::ref_ptr<Cell> cell = new Cell;
    cell->addTrees(trees);
    cell->divide(maxNumTreesPerCell);
    std::cout<<"done."<<std::endl;


    osg::Texture2D *tex = new osg::Texture2D;
    tex->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP );
    tex->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP );
    tex->setImage(osgDB::readRefImageFile("Images/tree0.rgba"));

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

    if ((mask & BILLBOARD_GRAPH)!=0)
    {
        std::cout<<"Creating osg::Billboard based forest...";
        osg::Group* group = new osg::Group;
        group->addChild(createBillboardGraph(cell.get(),dstate));
        if ((mask & HUD_TEXT)!=0) group->addChild(createHUDWithText("Using osg::Billboard's to create a forest\n\nPress left cursor key to select geometry instancing with Texture Buffer Object\nPress right cursor key to select double quad based forest"));
        _techniqueSwitch->addChild(group);
        std::cout<<"done."<<std::endl;
    }

    if ((mask & X_GRAPH)!=0)
    {
        std::cout<<"Creating double quad based forest...";
        osg::Group* group = new osg::Group;
        group->addChild(createXGraph(cell.get(),dstate));
        if ((mask & HUD_TEXT)!=0) group->addChild(createHUDWithText("Using double quads to create a forest\n\nPress left cursor key to select osg::Billboard based forest\nPress right cursor key to select osg::MatrixTransform based forest\n"));
        _techniqueSwitch->addChild(group);
        std::cout<<"done."<<std::endl;
    }

    if ((mask & TRANSFORM_GRAPH)!=0)
    {
        std::cout<<"Creating osg::MatrixTransform based forest...";
        osg::Group* group = new osg::Group;
        group->addChild(createTransformGraph(cell.get(),dstate));
        if ((mask & HUD_TEXT)!=0) group->addChild(createHUDWithText("Using osg::MatrixTransform's to create a forest\n\nPress left cursor key to select double quad based forest\nPress right cursor key to select osg::Vertex/FragmentProgram based forest"));
        _techniqueSwitch->addChild(group);
        std::cout<<"done."<<std::endl;
    }

    if ((mask & SHADER_GRAPH)!=0)
    {
        std::cout<<"Creating osg::Vertex/FragmentProgram based forest...";
        osg::Group* group = new osg::Group;

        osg::StateSet* stateset = new osg::StateSet(*dstate, osg::CopyOp::DEEP_COPY_ALL);

        {
            // vertex program
            std::ostringstream vp_oss;
            vp_oss <<
                "!!ARBvp1.0\n"

                "ATTRIB vpos = vertex.position;\n"
                "ATTRIB vcol = vertex.color;\n"
                "ATTRIB tc = vertex.texcoord[" << 0 << "];"

                "PARAM mvp[4] = { state.matrix.mvp };\n"
                "PARAM one = { 1.0, 1.0, 1.0, 1.0 };"

                "TEMP position;\n"

                // vec3 position = gl_Vertex.xyz * gl_Color.w + gl_Color.xyz;
                "MAD position, vpos, vcol.w, vcol;\n"

                // gl_Position     = gl_ModelViewProjectionMatrix * vec4(position,1.0);
                "MOV position.w, one;\n"
                "DP4 result.position.x, mvp[0], position;\n"
                "DP4 result.position.y, mvp[1], position;\n"
                "DP4 result.position.z, mvp[2], position;\n"
                "DP4 result.position.w, mvp[3], position;\n"

                // gl_FrontColor = vec4(1.0,1.0,1.0,1.0);
                "MOV result.color.front.primary, one;\n"

                // texcoord = gl_MultiTexCoord0.st;
                "MOV result.texcoord, tc;\n"
                "END\n";


            // fragment program
            std::ostringstream fp_oss;
            fp_oss <<
                "!!ARBfp1.0\n"
                "TEX result.color, fragment.texcoord[" << 0 << "], texture[" << 0 << "], 2D;"
                "END\n";

            osg::ref_ptr<osg::VertexProgram> vp = new osg::VertexProgram;
            vp->setVertexProgram(vp_oss.str());
            stateset->setAttributeAndModes(vp.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

            osg::ref_ptr<osg::FragmentProgram> fp = new osg::FragmentProgram;
            fp->setFragmentProgram(fp_oss.str());
            stateset->setAttributeAndModes(fp.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
        }

        group->addChild(createShaderGraph(cell.get(),stateset));
        if ((mask & HUD_TEXT)!=0) group->addChild(createHUDWithText("Using osg::Vertex/FragmentProgram to create a forest\n\nPress left cursor key to select osg::MatrixTransform's based forest\nPress right cursor key to select OpenGL shader based forest"));
        _techniqueSwitch->addChild(group);
        std::cout<<"done."<<std::endl;
    }

    if ((mask & SHADER_GRAPH)!=0)
    {
        std::cout<<"Creating OpenGL shader based forest...";
        osg::Group* group = new osg::Group;

        osg::StateSet* stateset = new osg::StateSet(*dstate, osg::CopyOp::DEEP_COPY_ALL);

        {
            osg::Program* program = new osg::Program;
            stateset->setAttribute(program);

#if 1
            // use inline shaders

            ///////////////////////////////////////////////////////////////////
            // vertex shader using just Vec4 coefficients
            char vertexShaderSource[] =
                "varying vec2 texcoord;\n"
                "\n"
                "void main(void)\n"
                "{\n"
                "    vec3 position = gl_Vertex.xyz * gl_Color.w + gl_Color.xyz;\n"
                "    gl_Position     = gl_ModelViewProjectionMatrix * vec4(position,1.0);\n"
                "    gl_FrontColor = vec4(1.0,1.0,1.0,1.0);\n"
                "    texcoord = gl_MultiTexCoord0.st;\n"
                "}\n";

            //////////////////////////////////////////////////////////////////
            // fragment shader
            //
            char fragmentShaderSource[] =
                "uniform sampler2D baseTexture; \n"
                "varying vec2 texcoord; \n"
                "\n"
                "void main(void) \n"
                "{ \n"
                "    gl_FragColor = texture2D( baseTexture, texcoord); \n"
                "}\n";

            osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource);
            program->addShader(vertex_shader);

            osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource);
            program->addShader(fragment_shader);

#else

            // get shaders from source
            program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("shaders/forest.vert")));
            program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("shaders/forest.frag")));

#endif

            osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
            stateset->addUniform(baseTextureSampler);
        }

        group->addChild(createShaderGraph(cell.get(),stateset));
        if ((mask & HUD_TEXT)!=0) group->addChild(createHUDWithText("Using OpenGL Shader to create a forest\n\nPress left cursor key to select osg::Vertex/FragmentProgram based forest\nPress right cursor key to select osg::Vertex/Geometry/FragmentProgram based forest"));
        _techniqueSwitch->addChild(group);
        std::cout<<"done."<<std::endl;
    }

    if ((mask & GEOMETRY_SHADER_GRAPH)!=0)
    {
        std::cout<<"Creating Geometry Shader based forest...";

        osg::StateSet* stateset = new osg::StateSet(*dstate, osg::CopyOp::DEEP_COPY_ALL);

        osg::Group* group = new osg::Group;
        group->addChild(createGeometryShaderGraph(cell.get(), stateset));
        if ((mask & HUD_TEXT)!=0) group->addChild(createHUDWithText("Using osg::Vertex/Geometry/FragmentProgram to create a forest\n\nPress left cursor key to select OpenGL Shader based forest\nPress right cursor key to select geometry instancing with Texture Buffer Object"));

        _techniqueSwitch->addChild(group);
        std::cout<<"done."<<std::endl;
    }

    if ((mask & TEXTURE_BUFFER_GRAPH)!=0)
    {
        std::cout<<"Creating forest using geometry instancing and texture buffer objects ...";

        osg::StateSet* stateset = new osg::StateSet(*dstate, osg::CopyOp::DEEP_COPY_ALL);
        {
            osg::Program* program = new osg::Program;
            stateset->setAttribute(program);

            char vertexShaderSource[] =
                "#version 420 compatibility\n"
                "uniform samplerBuffer dataBuffer;\n"
                "layout(location = 0) in vec3 VertexPosition;\n"
                "layout(location = 8) in vec3 VertexTexCoord;\n"
                "out vec2 TexCoord;\n"
                "out vec4 Color;\n"
                "void main()\n"
                "{\n"
                "   int instanceAddress = gl_InstanceID * 3;\n"
                "   vec3 position = texelFetch(dataBuffer, instanceAddress).xyz;\n"
                "   Color         = texelFetch(dataBuffer, instanceAddress + 1);\n"
                "   vec2 size     = texelFetch(dataBuffer, instanceAddress + 2).xy;\n"
                "   mat4 mvpMatrix = gl_ModelViewProjectionMatrix *\n"
                "        mat4( size.x, 0.0, 0.0, 0.0,\n"
                "              0.0, size.x, 0.0, 0.0,\n"
                "              0.0, 0.0, size.y, 0.0,\n"
                "              position.x, position.y, position.z, 1.0);\n"
                "   gl_Position = mvpMatrix * vec4(VertexPosition,1.0) ;\n"
                "   TexCoord = VertexTexCoord.xy;\n"
                "}\n";

            char fragmentShaderSource[] =
                "#version 420 core\n"
                "uniform sampler2D baseTexture; \n"
                "in vec2 TexCoord;\n"
                "in vec4 Color;\n"
                "layout(location = 0, index = 0) out vec4 FragData0;\n"
                "void main(void) \n"
                "{\n"
                "    FragData0 = Color*texture(baseTexture, TexCoord);\n"
                "}\n";

            osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource);
            program->addShader(vertex_shader);

            osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource);
            program->addShader(fragment_shader);

            osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
            stateset->addUniform(baseTextureSampler);

            osg::Uniform* dataBufferSampler = new osg::Uniform("dataBuffer",1);
            stateset->addUniform(dataBufferSampler);
        }

        osg::ref_ptr<osg::Geometry> templateGeometry = createOrthogonalQuadsNoColor(osg::Vec3(0.0f,0.0f,0.0f),1.0f,1.0f);
        templateGeometry->setUseVertexBufferObjects(true);
        templateGeometry->setUseDisplayList(false);
        osg::Node* textureBufferGraph = createTextureBufferGraph(cell.get(), templateGeometry.get());
        textureBufferGraph->setStateSet( stateset );
        osg::Group* group = new osg::Group;
        group->addChild(textureBufferGraph);
        if ((mask & HUD_TEXT)!=0) group->addChild(createHUDWithText("Using geometry instancing to create a forest\n\nPress left cursor key to select osg::Vertex/Geometry/FragmentProgram based forest\nPress right cursor key to select osg::Billboard based forest"));

        _techniqueSwitch->addChild(group);

        std::cout<<"done."<<std::endl;
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

    // construct the viewer.
    osgViewer::Viewer viewer(arguments);

    unsigned int numTreesToCreate = 10000;
    arguments.read("--trees",numTreesToCreate);

    unsigned int maxNumTreesPerCell = sqrtf(static_cast<float>(numTreesToCreate));

    arguments.read("--trees-per-cell",maxNumTreesPerCell);


    unsigned int features = ForestTechniqueManager::ALL_FEATURES;
    while(arguments.read("--features", features))
    {
        std::cout<<"features = "<<features<<std::endl;
    }

    std::string outputFilename;
    while(arguments.read("-o", outputFilename)) {}

    osg::ref_ptr<ForestTechniqueManager> ttm = new ForestTechniqueManager;

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);

    viewer.addEventHandler(new TechniqueEventHandler(ttm.get()));
    viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

    // add model to viewer.
    viewer.setSceneData( ttm->createScene(numTreesToCreate, maxNumTreesPerCell, features) );

    if (!outputFilename.empty())
    {
        osgDB::writeNodeFile(*viewer.getSceneData(), outputFilename);
        return 0;
    }

    return viewer.run();
}
