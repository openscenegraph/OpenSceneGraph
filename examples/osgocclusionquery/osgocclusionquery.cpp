/* OpenSceneGraph example, osganimate.
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

// This exampl demonstrates use of OcclusionQueryNode.
//
// In general, you use OcclusionQueryNode by simply attaching a subgraph
// or subgraphs as children, and it performs an OpenGL oclusion query
// to determine whether to draw the subgraphs or not.
//
// You can manually insert OcclusionQueryNodes at strategic locations
// in your scene graph, or you can write a NodeVisitor to insert them
// automatically, as this example shows.
//
// Run this example with no command line arguments, and it creates
// a "stock scene" to show how OcclusionQueryNode can be used.
//
// Or, run this example with a model on the command line, and the
// example uses a NodeVisitor to try to find worthwhile locations
// for OcclusionQueryNodes in your the scene graph.



#include <osg/NodeVisitor>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/StateAttribute>
#include <osg/PolygonMode>
#include <osg/ColorMask>
#include <osg/PolygonOffset>
#include <osg/Depth>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgUtil/Optimizer>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/StateSetManipulator>

#include <osg/OcclusionQueryNode>

#include <iostream>
#include <sstream>


// NodeVisitors and utility functions for OcclusionQueryNode

// Use this visitor to insert OcclusionQueryNodes (OQNs) in the
//   visited subgraph. Only one OQN will test any particular node
//   (no nesting). See also OcclusionQueryNonFlatVisitor.
class OcclusionQueryVisitor : public osg::NodeVisitor
{
public:
    OcclusionQueryVisitor();
    virtual ~OcclusionQueryVisitor();

    // Specify the vertex count threshold for performing occlusion
    //   query tests. Nodes in the scene graph whose total child geometry
    //   contains fewer vertices than the specified threshold will
    //   never be tested, just drawn. (In fact, they will br treated as
    //   potential occluders and rendered first in front-to-back order.)
    void setOccluderThreshold( int vertices );
    int getOccluderThreshold() const;

    virtual void apply( osg::OcclusionQueryNode& oqn );
    virtual void apply( osg::Group& group );
    virtual void apply( osg::Geode& geode );

protected:
    void addOQN( osg::Node& node );

    // When an OQR creates all OQNs and each OQN shares the same OQC,
    //   these methods are used to uniquely name all OQNs. Handy
    //   for debugging.
    std::string getNextOQNName();
    int getNameIdx() const { return _nameIdx; }

    osg::ref_ptr<osg::StateSet> _state;
    osg::ref_ptr<osg::StateSet> _debugState;

    unsigned int _nameIdx;

    int _occluderThreshold;
};

// Find all OQNs in the visited scene graph and set their visibility threshold.
class VisibilityThresholdVisitor : public osg::NodeVisitor
{
public:
    VisibilityThresholdVisitor( unsigned int threshold=500 )
      : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
        _visThreshold( threshold ) {}
    virtual ~VisibilityThresholdVisitor() {}

    virtual void apply( osg::OcclusionQueryNode& oqn );

protected:
    unsigned int _visThreshold;
};

// Find all OQNs in the visited scene graph and set the number of frames
//   between queries.
class QueryFrameCountVisitor : public osg::NodeVisitor
{
public:
    QueryFrameCountVisitor( int count=5 )
      : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
        _count( count ) {}
    virtual ~QueryFrameCountVisitor() {}

    virtual void apply( osg::OcclusionQueryNode& oqn );

protected:
    unsigned int _count;
};

// Find all OQNs in the visited scene graph and enable or disable queries..
class EnableQueryVisitor : public osg::NodeVisitor
{
public:
    EnableQueryVisitor( bool enable=true )
      : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
        _enabled( enable ) {}
    virtual ~EnableQueryVisitor() {}

    virtual void apply( osg::OcclusionQueryNode& oqn );

protected:
    bool _enabled;
};

// Find all OQNs in the visited scene graph and enable or disable the
//   debug bounding volume display.
class DebugDisplayVisitor : public osg::NodeVisitor
{
public:
    DebugDisplayVisitor( bool debug=true )
      : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
        _debug( debug ) {}
    virtual ~DebugDisplayVisitor() {}

    virtual void apply( osg::OcclusionQueryNode& oqn );

protected:
    bool _debug;
};

// Remove all OQNs from the visited scene graph.
class RemoveOcclusionQueryVisitor : public osg::NodeVisitor
{
public:
    RemoveOcclusionQueryVisitor();
    virtual ~RemoveOcclusionQueryVisitor();

    virtual void apply( osg::OcclusionQueryNode& oqn );

protected:
};

// Gather statistics about OQN performance in the visited scene graph.
class StatisticsVisitor : public osg::NodeVisitor
{
public:
    StatisticsVisitor( osg::NodeVisitor::TraversalMode mode=osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN );
    virtual ~StatisticsVisitor();

    virtual void apply( osg::OcclusionQueryNode& oqn );

    void reset();
    unsigned int getNumOQNs() const;
    unsigned int getNumPassed() const;

protected:
    unsigned int _numOQNs;
    unsigned int _numPassed;
};


unsigned int countGeometryVertices( osg::Geometry* geom )
{
    if (!geom->getVertexArray())
        return 0;

    // TBD This will eventually iterate over the PrimitiveSets and total the
    //   number of vertices actually used. But for now, it just returns the
    //   size of the vertex array.

    return geom->getVertexArray()->getNumElements();
}

class VertexCounter : public osg::NodeVisitor
{
public:
    VertexCounter( int limit )
      : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
        _limit( limit ),
        _total( 0 ) {}
    ~VertexCounter() {}

    int getTotal() { return _total; }
    bool exceeded() const { return _total > _limit; }
    void reset() { _total = 0; }

    virtual void apply( osg::Node& node )
    {
        // Check for early abort. If out total already exceeds the
        //   max number of vertices, no need to traverse further.
        if (exceeded())
            return;
        traverse( node );
    }

    virtual void apply( osg::Geode& geode )
    {
        // Possible early abort.
        if (exceeded())
            return;

        unsigned int i;
        for( i = 0; i < geode.getNumDrawables(); i++ )
        {
            osg::Geometry* geom = dynamic_cast<osg::Geometry *>(geode.getDrawable(i));
            if( !geom )
                continue;

            _total += countGeometryVertices( geom );

            if (_total > _limit)
                break;
        }
    }

protected:
    int _limit;
    int _total;
};



OcclusionQueryVisitor::OcclusionQueryVisitor()
  : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
    _nameIdx( 0 ),
    _occluderThreshold( 5000 )
{
    // Create a dummy OcclusionQueryNode just so we can get its state.
    // We'll then share that state between all OQNs we add to the visited scene graph.
    osg::ref_ptr<osg::OcclusionQueryNode> oqn = new osg::OcclusionQueryNode;

    _state = oqn->getQueryStateSet();
    _debugState = oqn->getDebugStateSet();
}

OcclusionQueryVisitor::~OcclusionQueryVisitor()
{
    osg::notify( osg::INFO ) <<
        "osgOQ: OcclusionQueryVisitor: Added " << getNameIdx() <<
        " OQNodes." << std::endl;
}

void
OcclusionQueryVisitor::setOccluderThreshold( int vertices )
{
    _occluderThreshold = vertices;
}
int
OcclusionQueryVisitor::getOccluderThreshold() const
{
    return _occluderThreshold;
}

void
OcclusionQueryVisitor::apply( osg::OcclusionQueryNode& oqn )
{
    // A subgraph is already under osgOQ control.
    // Don't traverse further.
    return;
}

void
OcclusionQueryVisitor::apply( osg::Group& group )
{
    if (group.getNumParents() == 0)
    {
        // Can't add an OQN above a root node.
        traverse( group );
        return;
    }

    int preTraverseOQNCount = getNameIdx();
    traverse( group );

    if (getNameIdx() > preTraverseOQNCount)
        // A least one OQN was added below the current node.
        //   Don't add one here to avoid hierarchical nesting.
        return;

    // There are no OQNs below this group. If the vertex
    //   count exceeds the threshold, add an OQN here.
    addOQN( group );
}

void
OcclusionQueryVisitor::apply( osg::Geode& geode )
{
    if (geode.getNumParents() == 0)
    {
        // Can't add an OQN above a root node.
        traverse( geode );
        return;
    }

    addOQN( geode );
}

void
OcclusionQueryVisitor::addOQN( osg::Node& node )
{
    VertexCounter vc( _occluderThreshold );
    node.accept( vc );
    if (vc.exceeded())
    {
        // Insert OQN(s) above this node.
        unsigned int np = node.getNumParents();
        while (np--)
        {
            osg::Group* parent = dynamic_cast<osg::Group*>( node.getParent( np ) );
            if (parent != NULL)
            {
                osg::ref_ptr<osg::OcclusionQueryNode> oqn = new osg::OcclusionQueryNode();
                oqn->addChild( &node );
                parent->replaceChild( &node, oqn.get() );

                oqn->setName( getNextOQNName() );
                // Set all OQNs to use the same query StateSets (instead of multiple copies
                //   of the same StateSet) for efficiency.
                oqn->setQueryStateSet( _state.get() );
                oqn->setDebugStateSet( _debugState.get() );
            }
        }
    }
}

std::string
OcclusionQueryVisitor::getNextOQNName()
{
    std::ostringstream ostr;
    ostr << "OQNode_" << _nameIdx++;
    return ostr.str();
}




//
void
VisibilityThresholdVisitor::apply( osg::OcclusionQueryNode& oqn )
{
    oqn.setVisibilityThreshold( _visThreshold );

    traverse( oqn );
}

void
QueryFrameCountVisitor::apply( osg::OcclusionQueryNode& oqn )
{
    oqn.setQueryFrameCount( _count );

    traverse( oqn );
}

void
EnableQueryVisitor::apply( osg::OcclusionQueryNode& oqn )
{
    oqn.setQueriesEnabled( _enabled );

    traverse( oqn );
}


void
DebugDisplayVisitor::apply( osg::OcclusionQueryNode& oqn )
{
    oqn.setDebugDisplay( _debug );

    traverse( oqn );
}


RemoveOcclusionQueryVisitor::RemoveOcclusionQueryVisitor()
  : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
{
}

RemoveOcclusionQueryVisitor::~RemoveOcclusionQueryVisitor()
{
}

void
RemoveOcclusionQueryVisitor::apply( osg::OcclusionQueryNode& oqn )
{
    if (oqn.getNumParents() == 0)
    {
        // Even if this is an OQN, can't delete it because it's the root.
        traverse( oqn );
        return;
    }

    osg::ref_ptr<osg::OcclusionQueryNode> oqnPtr = &oqn;

    unsigned int np = oqn.getNumParents();
    while (np--)
    {
        osg::Group* parent = dynamic_cast<osg::Group*>( oqn.getParent( np ) );
        if (parent != NULL)
        {
            // Remove OQN from parent.
            parent->removeChild( oqnPtr.get() );

            // Add OQN's children to parent.
            unsigned int nc = oqn.getNumChildren();
            while (nc--)
                parent->addChild( oqn.getChild( nc ) );
        }
    }
}



StatisticsVisitor::StatisticsVisitor( osg::NodeVisitor::TraversalMode mode )
  : osg::NodeVisitor( mode ),
    _numOQNs( 0 ),
    _numPassed( 0 )
{
}

StatisticsVisitor::~StatisticsVisitor()
{
}

void
StatisticsVisitor::apply( osg::OcclusionQueryNode& oqn )
{
    _numOQNs++;
    if (oqn.getPassed())
        _numPassed++;

    traverse( oqn );
}

void
StatisticsVisitor::reset()
{
    _numOQNs = _numPassed = 0;
}

unsigned int
StatisticsVisitor::getNumOQNs() const
{
    return _numOQNs;
}
unsigned int
StatisticsVisitor::getNumPassed() const
{
    return _numPassed;
}

// End NodeVisitors



// KetHandler --
// Allow user to do interesting things with an
// OcclusionQueryNode-enabled scene graph at run time.
class KeyHandler : public osgGA::GUIEventHandler
{
public:
    KeyHandler( osg::Node& node )
      : _node( node ),
        _enable( true ),
        _debug( false )
    {}

    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
    {
        switch( ea.getEventType() )
        {
            case(osgGA::GUIEventAdapter::KEYUP):
            {
                if (ea.getKey()==osgGA::GUIEventAdapter::KEY_F6)
                {
                    // F6 -- Toggle osgOQ testing.
                    _enable = !_enable;
                    EnableQueryVisitor eqv( _enable );
                    _node.accept( eqv );
                    return true;
                }
                else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_F7)
                {
                    // F7 -- Toggle display of OQ test bounding volumes
                    _debug = !_debug;
                    DebugDisplayVisitor ddv( _debug );
                    _node.accept( ddv );
                    return true;
                }
                else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_F8)
                {
                    // F8 -- Gether stats and display
                    StatisticsVisitor sv;
                    _node.accept( sv );
                    std::cout << "osgOQ: Stats: numOQNs " << sv.getNumOQNs() << ", numPased " << sv.getNumPassed() << std::endl;
                    return true;
                }
                else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_F9)
                {
                    // F9 -- Remove all OcclusionQueryNodes
                    RemoveOcclusionQueryVisitor roqv;
                    _node.accept( roqv );
                    return true;
                }
                else if (ea.getKey()=='o')
                {
                    if (osgDB::writeNodeFile( _node, "saved_model.osg" ))
                        osg::notify( osg::ALWAYS ) << "osgOQ: Wrote scene graph to \"saved_model.osg\"" << std::endl;
                    else
                        osg::notify( osg::ALWAYS ) << "osgOQ: Wrote failed for \"saved_model.osg\"" << std::endl;
                    return true;
                }
                return false;
            }
            default:
                break;
        }
        return false;
    }

    osg::Node& _node;

    bool _enable, _debug;
};

// Create a cube with one side missing. This makes a great simple occluder.
osg::ref_ptr<osg::Node>
createBox()
{
    osg::ref_ptr<osg::Geode> box = new osg::Geode;

    osg::StateSet* state = box->getOrCreateStateSet();
    osg::PolygonMode* pm = new osg::PolygonMode(
        osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL );
    state->setAttributeAndModes( pm,
        osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;
    geom->setVertexArray( v.get() );

    {
        const float x( 0.f );
        const float y( 0.f );
        const float z( 0.f );
        const float r( 1.1f );

        v->push_back( osg::Vec3( x-r, y-r, z-r ) ); //left -X
        v->push_back( osg::Vec3( x-r, y-r, z+r ) );
        v->push_back( osg::Vec3( x-r, y+r, z+r ) );
        v->push_back( osg::Vec3( x-r, y+r, z-r ) );

        v->push_back( osg::Vec3( x+r, y-r, z+r ) ); //right +X
        v->push_back( osg::Vec3( x+r, y-r, z-r ) );
        v->push_back( osg::Vec3( x+r, y+r, z-r ) );
        v->push_back( osg::Vec3( x+r, y+r, z+r ) );

        v->push_back( osg::Vec3( x-r, y-r, z-r ) ); // bottom -Z
        v->push_back( osg::Vec3( x-r, y+r, z-r ) );
        v->push_back( osg::Vec3( x+r, y+r, z-r ) );
        v->push_back( osg::Vec3( x+r, y-r, z-r ) );

        v->push_back( osg::Vec3( x-r, y-r, z+r ) ); // top +Z
        v->push_back( osg::Vec3( x+r, y-r, z+r ) );
        v->push_back( osg::Vec3( x+r, y+r, z+r ) );
        v->push_back( osg::Vec3( x-r, y+r, z+r ) );

        v->push_back( osg::Vec3( x-r, y+r, z-r ) ); // back +Y
        v->push_back( osg::Vec3( x-r, y+r, z+r ) );
        v->push_back( osg::Vec3( x+r, y+r, z+r ) );
        v->push_back( osg::Vec3( x+r, y+r, z-r ) );
    }

    osg::ref_ptr<osg::Vec4Array> c = new osg::Vec4Array;
    geom->setColorArray( c.get() );
    geom->setColorBinding( osg::Geometry::BIND_OVERALL );
    c->push_back( osg::Vec4( 0.f, 1.f, 1.f, 1.f ) );

    osg::ref_ptr<osg::Vec3Array> n = new osg::Vec3Array;
    geom->setNormalArray( n.get() );
    geom->setNormalBinding( osg::Geometry::BIND_PER_PRIMITIVE );
    n->push_back( osg::Vec3( -1.f, 0.f, 0.f ) );
    n->push_back( osg::Vec3( 1.f, 0.f, 0.f ) );
    n->push_back( osg::Vec3( 0.f, 0.f, -1.f ) );
    n->push_back( osg::Vec3( 0.f, 0.f, 1.f ) );
    n->push_back( osg::Vec3( 0.f, 1.f, 0.f ) );

    geom->addPrimitiveSet( new osg::DrawArrays( GL_QUADS, 0, 20 ) );
    box->addDrawable( geom.get() );

    return box.get();
}

// Make a Geometry that renders slow intentionally.
// To make sure it renders slow, we do the following:
//  * Disable display lists
//  * Force glBegin/glEnd slow path
//  * Lots of vertices and color data per vertex
//  * No vertex sharing
//  * Draw the triangles as wireframe
osg::ref_ptr<osg::Node>
createRandomTriangles( unsigned int num )
{
    osg::ref_ptr<osg::Geode> tris = new osg::Geode;

    osg::StateSet* ss = tris->getOrCreateStateSet();

    // Force wireframe. Many gfx cards handle this poorly.
    osg::PolygonMode* pm = new osg::PolygonMode(
        osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE );
    ss->setAttributeAndModes( pm, osg::StateAttribute::ON |
        osg::StateAttribute::PROTECTED);
    ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF |
        osg::StateAttribute::PROTECTED);

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    // Disable display lists to decrease performance.
    geom->setUseDisplayList( false );

    osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;
    geom->setVertexArray( v.get() );
    v->resize( num*3 );

    unsigned int i;
    srand( 0 );
#define RAND_NEG1_TO_1 ( ((rand()%20)-10)*.1 )
    for (i=0; i<num; i++)
    {
        osg::Vec3& v0 = (*v)[ i*3+0 ];
        osg::Vec3& v1 = (*v)[ i*3+1 ];
        osg::Vec3& v2 = (*v)[ i*3+2 ];
        v0 = osg::Vec3( RAND_NEG1_TO_1, RAND_NEG1_TO_1, RAND_NEG1_TO_1 );
        v1 = osg::Vec3( RAND_NEG1_TO_1, RAND_NEG1_TO_1, RAND_NEG1_TO_1 );
        v2 = osg::Vec3( RAND_NEG1_TO_1, RAND_NEG1_TO_1, RAND_NEG1_TO_1 );
    }

    osg::ref_ptr<osg::Vec4Array> c = new osg::Vec4Array;
    geom->setColorArray( c.get() );
    // Bind per primitive to force slow glBegin/glEnd path.
    geom->setColorBinding( osg::Geometry::BIND_PER_PRIMITIVE );
    c->resize( num );

#define RAND_0_TO_1 ( (rand()%10)*.1 )
    for (i=0; i<num; i++)
    {
        osg::Vec4& c0 = (*c)[ i ];
        c0 = osg::Vec4( RAND_0_TO_1, RAND_0_TO_1, RAND_0_TO_1, 1. );
    }

    geom->addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLES, 0, num*3 ) );
    tris->addDrawable( geom.get() );

    return tris.get();
}

// Create the stock scene:
// Top level Group
//   Geode (simple occluder
//   OcclusionQueryNode
//     Geode with complex, slow geometry.
osg::ref_ptr<osg::Node>
createStockScene()
{
    // Create a simple box occluder
    osg::ref_ptr<osg::Group> root = new osg::Group();
    root->addChild( createBox().get() );

    // Create a complex mess of triangles as a child below an
    //   OcclusionQueryNode. The OQN will ensure that the
    //   subgraph isn't rendered when it's not visible.
    osg::ref_ptr<osg::OcclusionQueryNode> oqn = new osg::OcclusionQueryNode;
    oqn->addChild( createRandomTriangles( 20000 ).get() );
    root->addChild( oqn.get() );

    return root.get();
}


int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" demonstrates OpenGL occlusion query in OSG using the OcclusionQueryNode.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] [filename(s)]");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display command line parameters");

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout, osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    osgViewer::Viewer viewer( arguments );

    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);

    // add the help handler
    viewer.addEventHandler(new osgViewer::HelpHandler(arguments.getApplicationUsage()));


    // load the specified model
    osg::ref_ptr<osg::Node> root = osgDB::readNodeFiles( arguments );
    if (!root)
    {
        std::cout << arguments.getApplicationName() <<": No files specified, or can't load them." << std::endl;
        root = createStockScene().get();
        if (!root)
        {
            std::cout << arguments.getApplicationName() <<": Failed to create stock scene." << std::endl;
            return 1;
        }
        std::cout << "Using stock scene instead." << std::endl;
    }
    else
    {
        // Run a NodeVisitor to insert OcclusionQueryNodes in the scene graph.
        OcclusionQueryVisitor oqv;
        root->accept( oqv );
    }

    bool optimize = arguments.read( "--opt" );

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }


    // optimize the scene graph, remove redundant nodes and state etc.
    if (optimize)
    {
        osgUtil::Optimizer optimizer;
        optimizer.optimize( root.get() );
    }

    viewer.setSceneData( root.get() );

    KeyHandler* kh = new KeyHandler( *root );
    viewer.addEventHandler( kh );

    return viewer.run();
}

