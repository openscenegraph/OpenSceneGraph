//
// Copyright (C) 2007 Skew Matrix Software LLC (http://www.skew-matrix.com)
//
// This library is open source and may be redistributed and/or modified under  
// the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
// (at your option) any later version.  The full license is in LICENSE file
// included with this distribution, and on the openscenegraph.org website.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
// OpenSceneGraph Public License for more details.
//

#include <osg/OcclusionQueryNode>
#include <OpenThreads/ScopedLock>
#include <osg/Timer>
#include <osg/Notify>
#include <osg/CopyOp>
#include <osg/Vec3>
#include <osg/MatrixTransform>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/BoundingBox>
#include <osg/BoundingSphere>
#include <osg/Referenced>
#include <osg/ComputeBoundsVisitor>
#include <osg/StateSet>
#include <osg/StateAttribute>
#include <osg/PolygonMode>
#include <osg/ColorMask>
#include <osg/PolygonOffset>
#include <osg/Depth>
#include <map>
#include <vector>

#include <OpenThreads/Thread>


typedef osg::buffered_value< osg::ref_ptr< osg::Drawable::Extensions > > OcclusionQueryBufferedExtensions;
static OcclusionQueryBufferedExtensions s_OQ_bufferedExtensions;

//
// Support classes, used by (and private to) OcclusionQueryNode.
//   (Note a lot of this is historical. OcclusionQueryNode formaerly
//   existed as a NodeKit outside the core OSG distribution. Many
//   of these classes existed in their own separate header and
//   source files.)


// Create and return a StateSet appropriate for performing an occlusion
//   query test (disable lighting, texture mapping, etc). Probably some
//   room for improvement here. Could disable shaders, for example.
osg::StateSet*
initOQState()
{
    osg::StateSet* state = new osg::StateSet;
    // TBD Possible bug, need to allow user to set render bin number.
    state->setRenderBinDetails( 9, "RenderBin" );

    state->setMode( GL_LIGHTING, osg::StateAttribute::OFF |
        osg::StateAttribute::PROTECTED);
    state->setTextureMode( 0, GL_TEXTURE_2D, osg::StateAttribute::OFF |
        osg::StateAttribute::PROTECTED);
    state->setMode( GL_CULL_FACE, osg::StateAttribute::ON |
        osg::StateAttribute::PROTECTED);

    osg::ColorMask* cm = new osg::ColorMask( false, false, false, false );
    state->setAttributeAndModes( cm, osg::StateAttribute::ON |
        osg::StateAttribute::PROTECTED);
    osg::Depth* d = new osg::Depth( osg::Depth::LEQUAL, 0.f, 1.f, false );
    state->setAttributeAndModes( d, osg::StateAttribute::ON |
        osg::StateAttribute::PROTECTED);
    osg::PolygonMode* pm = new osg::PolygonMode(
        osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL );
    state->setAttributeAndModes( pm, osg::StateAttribute::ON |
        osg::StateAttribute::PROTECTED);

    osg::PolygonOffset* po = new osg::PolygonOffset( -1., -1. );
    state->setAttributeAndModes( po, osg::StateAttribute::ON |
        osg::StateAttribute::PROTECTED);

    return state;
}

// Create and return a StateSet for rendering a debug representation of query geometry.
osg::StateSet*
initOQDebugState()
{
    osg::StateSet* debugState = new osg::StateSet;

    debugState->setMode( GL_LIGHTING, osg::StateAttribute::OFF |
        osg::StateAttribute::PROTECTED);
    debugState->setTextureMode( 0, GL_TEXTURE_2D, osg::StateAttribute::OFF |
        osg::StateAttribute::PROTECTED);
    debugState->setMode( GL_CULL_FACE, osg::StateAttribute::ON |
        osg::StateAttribute::PROTECTED);

    osg::PolygonMode* pm = new osg::PolygonMode(
        osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE );
    debugState->setAttributeAndModes( pm, osg::StateAttribute::ON |
        osg::StateAttribute::PROTECTED);

    osg::PolygonOffset* po = new osg::PolygonOffset( -1., -1. );
    debugState->setAttributeAndModes( po, osg::StateAttribute::ON |
        osg::StateAttribute::PROTECTED);

    return debugState;
}


// TestResult -- stores (per context) results of an occlusion query
//   test performed by QueryGeometry. An OcclusionQueryNode has a
//   Geode owning a single QueryGeometry that
//   draws the occlusion query geometry. QueryGeometry keeps a
//   TestResult per context to store the result/status of each query.
// Accessed during the cull and draw traversals.
class TestResult : public osg::Referenced
{
public:
    TestResult() : _init( false ), _id( 0 ), _contextID( 0 ), _active( false ), _numPixels( 0 ) {}
    ~TestResult() {}

    bool _init;

    // Query ID for this context.
    GLuint _id;
    // Context ID owning this query ID.
    unsigned int _contextID;

    // Set to true when a query gets issued and set to
    //   false when the result is retrieved.
    mutable bool _active;

    // Result of last query.
    GLint _numPixels;
};

// QueryGeometry -- A Drawable that performs an occlusion query,
//   using its geometric data as the query geometry.
class QueryGeometry : public osg::Geometry
{
public:
    QueryGeometry( const std::string& oqnName=std::string("") );
    ~QueryGeometry();

    void reset();

    // TBD implement copy constructor

    virtual void drawImplementation( osg::RenderInfo& renderInfo ) const;

    unsigned int getNumPixels( const osg::Camera* cam );


    void releaseGLObjects( osg::State* state = 0 );
    static void deleteQueryObject( unsigned int contextID, GLuint handle );
    static void flushDeletedQueryObjects( unsigned int contextID, double currentTime, double& availableTime );
    static void discardDeletedQueryObjects( unsigned int contextID );
    
protected:
    typedef std::map< const osg::Camera*, TestResult > ResultMap;
    mutable ResultMap _results;
    mutable OpenThreads::Mutex _mapMutex;

    // Needed for debug only
    std::string _oqnName;
};

struct RetrieveQueriesCallback : public osg::Camera::DrawCallback
{
    typedef std::vector<TestResult*> ResultsVector;
    ResultsVector _results;

    RetrieveQueriesCallback( osg::Drawable::Extensions* ext=NULL )
      : _extensionsFallback( ext )
    {
    }

    RetrieveQueriesCallback( const RetrieveQueriesCallback&, const osg::CopyOp& ) {}
    META_Object( osgOQ, RetrieveQueriesCallback )

    virtual void operator() (const osg::Camera& camera) const
    {
        if (_results.empty())
            return;

        const osg::Timer& timer = *osg::Timer::instance();
        osg::Timer_t start_tick = timer.tick();
        double elapsedTime( 0. );
        int count( 0 );

        osg::Drawable::Extensions* ext;
        if (camera.getGraphicsContext())
        {
            // The typical path, for osgViewer-based applications or any
            //   app that has set up a valid GraphicsCOntext for the Camera.
            unsigned int contextID = camera.getGraphicsContext()->getState()->getContextID();
            RetrieveQueriesCallback* const_this = const_cast<RetrieveQueriesCallback*>( this );
            ext = const_this->getExtensions( contextID, true );
        }
        else
        {
            // No valid GraphicsContext in the Camera. This might happen in
            //   SceneView-based apps. Rely on the creating code to have passed
            //   in a valid Extensions pointer, and hope it's valid for any
            //   context that might be current.
            osg::notify( osg::DEBUG_INFO ) << "osgOQ: RQCB: Using fallback path to obtain Extensions pointer." << std::endl;
            ext = _extensionsFallback;
            if (!ext)
            {
                osg::notify( osg::FATAL ) << "osgOQ: RQCB: Extensions pointer fallback is NULL." << std::endl;
                return;
            }
        }

        ResultsVector::const_iterator it = _results.begin();
        while (it != _results.end())
        {
            TestResult* tr = const_cast<TestResult*>( *it );

            if (!tr->_active || !tr->_init)
            {
                // This test wasn't executed last frame. This is probably because
                //   a parent node failed the OQ test, this node is outside the
                //   view volume, or we didn't run the test because we had not
                //   exceeded visibleQueryFrameCount.
                // Do not obtain results from OpenGL.
                it++;
                continue;
            }

            osg::notify( osg::DEBUG_INFO ) <<
                "osgOQ: RQCB: Retrieving..." << std::endl;

#ifdef FORCE_QUERY_RESULT_AVAILABLE_BEFORE_RETRIEVAL

            // Should not need to do this, but is required on some platforms to
            // work aroung issues in the device driver. For example, without this
            // code, we've seen crashes on 64-bit Mac/Linux NVIDIA systems doing
            // multithreaded, multipipe rendering (as in a CAVE).
            // Tried with ATI and verified this workaround is not needed; the
            //   problem is specific to NVIDIA.
            GLint ready( 0 );
            while( !ready )
            {
                // Apparently, must actually sleep here to avoid issues w/ NVIDIA Quadro.
                OpenThreads::Thread::microSleep( 5 );
                ext->glGetQueryObjectiv( tr->_id, GL_QUERY_RESULT_AVAILABLE, &ready );
            };
#endif

            ext->glGetQueryObjectiv( tr->_id, GL_QUERY_RESULT, &(tr->_numPixels) );
            if (tr->_numPixels < 0)
                osg::notify( osg::WARN ) << "osgOQ: RQCB: " <<
                "glGetQueryObjectiv returned negative value (" << tr->_numPixels << ")." << std::endl;

            // Either retrieve last frame's results, or ignore it because the
            //   camera is inside the view. In either case, _active is now false.
            tr->_active = false;

            it++;
            count++;
        }

        elapsedTime = timer.delta_s(start_tick,timer.tick());
        osg::notify( osg::INFO ) << "osgOQ: RQCB: " << "Retrieved " << count <<
            " queries in " << elapsedTime << " seconds." << std::endl;
    }

    void reset()
    {
        _results.clear();
    }

    void add( TestResult* tr )
    {
        _results.push_back( tr );
    }

    osg::Drawable::Extensions* getExtensions( unsigned int contextID, bool createIfNotInitalized )
    {
        if (!s_OQ_bufferedExtensions[ contextID ] && createIfNotInitalized)
            s_OQ_bufferedExtensions[ contextID ] = new osg::Drawable::Extensions( contextID );
        return s_OQ_bufferedExtensions[ contextID ].get();
    }


    osg::Drawable::Extensions* _extensionsFallback;
};



// PreDraw callback; clears the list of Results from the PostDrawCallback (above).
struct ClearQueriesCallback : public osg::Camera::DrawCallback
{
    ClearQueriesCallback() : _rqcb( NULL ) {}
    ClearQueriesCallback( const ClearQueriesCallback&, const osg::CopyOp& ) {}
    META_Object( osgOQ, ClearQueriesCallback )

    virtual void operator() (const osg::Camera&) const
    {
        if (!_rqcb)
        {
            osg::notify( osg::FATAL ) << "osgOQ: CQCB: Invalid RQCB." << std::endl;
            return;
        }
        _rqcb->reset();
    }

    RetrieveQueriesCallback* _rqcb;
};


// static cache of deleted query objects which can only 
// be completely deleted once the appropriate OpenGL context
// is set.
typedef std::list< GLuint > QueryObjectList;
typedef osg::buffered_object< QueryObjectList > DeletedQueryObjectCache;

static OpenThreads::Mutex s_mutex_deletedQueryObjectCache;
static DeletedQueryObjectCache s_deletedQueryObjectCache;

QueryGeometry::QueryGeometry( const std::string& oqnName )
  : _oqnName( oqnName )
{
    // TBD check to see if we can have this on.
    setUseDisplayList( false );
}

QueryGeometry::~QueryGeometry()
{
    reset();
}


void
QueryGeometry::reset()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock( _mapMutex );

    ResultMap::iterator it = _results.begin();
    while (it != _results.end())
    {
        TestResult& tr = it->second;
        if (tr._init)
            QueryGeometry::deleteQueryObject( tr._contextID, tr._id );
        it++;
    }
    _results.clear();
}

// After 1.2, param 1 changed from State to RenderInfo.
// Warning: Version was still 1.2 on dev branch long after the 1.2 release,
//   and finally got bumped to 1.9 in April 2007.
void
QueryGeometry::drawImplementation( osg::RenderInfo& renderInfo ) const
{
    unsigned int contextID = renderInfo.getState()->getContextID();
    osg::Drawable::Extensions* ext = getExtensions( contextID, true );
    osg::Camera* cam = renderInfo.getCurrentCamera();

    // Add callbacks if necessary.
    if (!cam->getPostDrawCallback())
    {
        RetrieveQueriesCallback* rqcb = new RetrieveQueriesCallback( ext );
        cam->setPostDrawCallback( rqcb );

        ClearQueriesCallback* cqcb = new ClearQueriesCallback;
        cqcb->_rqcb = rqcb;
        cam->setPreDrawCallback( cqcb );
    }

    // Get TestResult from Camera map
    TestResult* tr;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock( _mapMutex );
        tr = &( _results[ cam ] );
    }

    // Add TestResult to RQCB.
    RetrieveQueriesCallback* rqcb = dynamic_cast<
        RetrieveQueriesCallback* >( cam->getPostDrawCallback() );
    if (!rqcb)
    {
        osg::notify( osg::FATAL ) << "osgOQ: QG: Invalid RQCB." << std::endl;
        return;
    }
    rqcb->add( tr );


    // Issue query
    if (!tr->_init)
    {
        ext->glGenQueries( 1, &(tr->_id) );
        tr->_contextID = contextID;
        tr->_init = true;
    }

    osg::notify( osg::DEBUG_INFO ) <<
        "osgOQ: QG: Querying for: " << _oqnName << std::endl;

    ext->glBeginQuery( GL_SAMPLES_PASSED_ARB, tr->_id );
    osg::Geometry::drawImplementation( renderInfo );
    ext->glEndQuery( GL_SAMPLES_PASSED_ARB );
    tr->_active = true;


    osg::notify( osg::DEBUG_INFO ) <<
        "osgOQ: QG. OQNName: " << _oqnName <<
        ", Ctx: " << contextID <<
        ", ID: " << tr->_id << std::endl;
#ifdef _DEBUG
    {
        GLenum err;
        if ((err = glGetError()) != GL_NO_ERROR)
            osg::notify( osg::FATAL ) <<
            "osgOQ: QG: OpenGL error: " << err << "." << std::endl;
    }
#endif


}


unsigned int
QueryGeometry::getNumPixels( const osg::Camera* cam )
{
    TestResult tr;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock( _mapMutex );
        tr =  _results[ cam ];
    }
    return tr._numPixels;
}


void
QueryGeometry::releaseGLObjects( osg::State* state )
{
    if (!state)
        // delete all query IDs for all contexts.
        reset();

    else
    {
        // Delete all query IDs for the specified context.
        unsigned int contextID = state->getContextID();
        ResultMap::iterator it = _results.begin();
        while (it != _results.end())
        {
            TestResult& tr = it->second;
            if (tr._contextID == contextID)
            {
                QueryGeometry::deleteQueryObject( contextID, tr._id );
                tr._init = false;
            }
            it++;
        }
    }
}

void
QueryGeometry::deleteQueryObject( unsigned int contextID, GLuint handle )
{
    if (handle!=0)
    {
        OpenThreads::ScopedLock< OpenThreads::Mutex > lock( s_mutex_deletedQueryObjectCache );

        // insert the handle into the cache for the appropriate context.
        s_deletedQueryObjectCache[contextID].push_back( handle );
    }
}


void
QueryGeometry::flushDeletedQueryObjects( unsigned int contextID, double /*currentTime*/, double& availableTime )
{
    // if no time available don't try to flush objects.
    if (availableTime<=0.0) return;

    const osg::Timer& timer = *osg::Timer::instance();
    osg::Timer_t start_tick = timer.tick();
    double elapsedTime = 0.0;

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedQueryObjectCache);

        const osg::Drawable::Extensions* extensions = getExtensions( contextID, true );

        QueryObjectList& qol = s_deletedQueryObjectCache[contextID];

        for(QueryObjectList::iterator titr=qol.begin();
            titr!=qol.end() && elapsedTime<availableTime;
            )
        {
            extensions->glDeleteQueries( 1L, &(*titr ) );
            titr = qol.erase(titr);
            elapsedTime = timer.delta_s(start_tick,timer.tick());
        }
    }
        
    availableTime -= elapsedTime;
}

void
QueryGeometry::discardDeletedQueryObjects( unsigned int contextID )
{
    OpenThreads::ScopedLock< OpenThreads::Mutex > lock( s_mutex_deletedQueryObjectCache );
    QueryObjectList& qol = s_deletedQueryObjectCache[ contextID ];
    qol.clear();
}

// End support classes
//



namespace osg
{


OcclusionQueryNode::OcclusionQueryNode()
  : _enabled( true ),
    _visThreshold( 500 ),
    _queryFrameCount( 5 ),
    _debugBB( false )
{
    setDataVariance( osg::Object::DYNAMIC );

    // OQN has two Geode member variables, one for doing the
    //   query and one for rendering the debug geometry.
    //   Create and initialize them.
    createSupportNodes();
}

OcclusionQueryNode::~OcclusionQueryNode()
{
}

OcclusionQueryNode::OcclusionQueryNode( const OcclusionQueryNode& oqn, const osg::CopyOp& copyop )
  : Group( oqn, copyop ),
    _passed( false )
{
    _enabled = oqn._enabled;
    _visThreshold = oqn._visThreshold;
    _queryFrameCount = oqn._queryFrameCount;
    _debugBB = oqn._debugBB;

    // Regardless of shallow or deep, create unique support nodes.
    createSupportNodes();
}


bool
OcclusionQueryNode::getPassed( const osg::Camera* camera, float distanceToEyePoint )
{
    if ( !_enabled )
        // Queries are not enabled. The caller should be osgUtil::CullVisitor,
        //   return true to traverse the subgraphs.
        return true;

    // In the future, we could hold a reference directly to the QueryDrawable
    //   to avoid the dynamic_cast.
    QueryGeometry* qg = dynamic_cast< QueryGeometry* >( _queryGeode->getDrawable( 0 ) );
    if (qg == NULL)
    {
        osg::notify( osg::FATAL ) <<
            "osgOQ: OcclusionQueryNode: No QueryGeometry." << std::endl;
        // Something's broke. Return true so we at least render correctly.
        return true;
    }

    // If the distance to the bounding sphere shell is positive, retrieve
    //   the results. Others (we're inside the BS shell) we are considered
    //   to have passed and don't need to retrieve the query.
    const osg::BoundingSphere& bs = getBound();
    float distance = distanceToEyePoint  - bs._radius;
    _passed = ( distance <= 0.f );
    if (!_passed)
    {
        int result = qg->getNumPixels( camera );
        _passed = ( (unsigned int)(result) > _visThreshold );
    }

    return _passed;
}

void
OcclusionQueryNode::traverseQuery( const osg::Camera* camera, osg::NodeVisitor& nv )
{
    bool issueQuery;
    {
        const int curFrame = nv.getTraversalNumber();

        OpenThreads::ScopedLock<OpenThreads::Mutex> lock( _frameCountMutex );
        int& lastQueryFrame = _frameCountMap[ camera ];
        issueQuery = (curFrame - lastQueryFrame >= _queryFrameCount);
        if (issueQuery)
            lastQueryFrame = curFrame;
    }
    if (issueQuery)
        _queryGeode->accept( nv );
}

void
OcclusionQueryNode::traverseDebug( osg::NodeVisitor& nv )
{
    if (_debugBB)
        // If requested, display the debug geometry
        _debugGeode->accept( nv );
}

osg::BoundingSphere
OcclusionQueryNode::computeBound() const
{
    {
        // Need to make this routine thread-safe. Typically called by the update
        //   Visitor, or just after the update traversal, but could be called by
        //   an application thread or by a non-osgViewer application.
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock( _computeBoundMutex )  ;

        // This is the logical place to put this code, but the method is const. Cast
        //   away constness to compute the bounding box and modify the query geometry.
        osg::OcclusionQueryNode* nonConstThis = const_cast<osg::OcclusionQueryNode*>( this );


        osg::ComputeBoundsVisitor cbv;
        nonConstThis->accept( cbv );
        osg::BoundingBox bb = cbv.getBoundingBox();

        osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;
        v->resize( 8 );
        (*v)[0] = osg::Vec3( bb._min.x(), bb._min.y(), bb._min.z() );
        (*v)[1] = osg::Vec3( bb._max.x(), bb._min.y(), bb._min.z() );
        (*v)[2] = osg::Vec3( bb._max.x(), bb._min.y(), bb._max.z() );
        (*v)[3] = osg::Vec3( bb._min.x(), bb._min.y(), bb._max.z() );
        (*v)[4] = osg::Vec3( bb._max.x(), bb._max.y(), bb._min.z() );
        (*v)[5] = osg::Vec3( bb._min.x(), bb._max.y(), bb._min.z() );
        (*v)[6] = osg::Vec3( bb._min.x(), bb._max.y(), bb._max.z() );
        (*v)[7] = osg::Vec3( bb._max.x(), bb._max.y(), bb._max.z() );

        osg::Geometry* geom = dynamic_cast< osg::Geometry* >( nonConstThis->_queryGeode->getDrawable( 0 ) );
        geom->setVertexArray( v.get() );

        geom = dynamic_cast< osg::Geometry* >( nonConstThis->_debugGeode->getDrawable( 0 ) );
        geom->setVertexArray( v.get() );
    }

    return Group::computeBound();
}


// Should only be called outside of cull/draw. No thread issues.
void
OcclusionQueryNode::setQueriesEnabled( bool enable )
{
    _enabled = enable;
}

// Should only be called outside of cull/draw. No thread issues.
void
OcclusionQueryNode::setDebugDisplay( bool debug )
{
    _debugBB = debug;
}
bool
OcclusionQueryNode::getDebugDisplay() const
{
    return _debugBB;
}



void
OcclusionQueryNode::setQueryStateSet( osg::StateSet* ss )
{
    if (!_queryGeode)
    {
        osg::notify( osg::WARN ) << "osgOQ: OcclusionQueryNode:: Invalid query support node." << std::endl;
        return;
    }

    _queryGeode->setStateSet( ss );
}
osg::StateSet*
OcclusionQueryNode::getQueryStateSet()
{
    if (!_queryGeode)
    {
        osg::notify( osg::WARN ) << "osgOQ: OcclusionQueryNode:: Invalid query support node." << std::endl;
        return NULL;
    }
    return _queryGeode->getStateSet();
}

const osg::StateSet*
OcclusionQueryNode::getQueryStateSet() const
{
    if (!_queryGeode)
    {
        osg::notify( osg::WARN ) << "osgOQ: OcclusionQueryNode:: Invalid query support node." << std::endl;
        return NULL;
    }
    return _queryGeode->getStateSet();
}

void
OcclusionQueryNode::setDebugStateSet( osg::StateSet* ss )
{
    if (!_debugGeode)
    {
        osg::notify( osg::WARN ) << "osgOQ: OcclusionQueryNode:: Invalid debug support node." << std::endl;
        return;
    }
    _debugGeode->setStateSet( ss );
}

osg::StateSet*
OcclusionQueryNode::getDebugStateSet()
{
    if (!_debugGeode.valid())
    {
        osg::notify( osg::WARN ) << "osgOQ: OcclusionQueryNode:: Invalid debug support node." << std::endl;
        return NULL;
    }
    return _debugGeode->getStateSet();
}
const osg::StateSet*
OcclusionQueryNode::getDebugStateSet() const
{
    if (!_debugGeode.valid())
    {
        osg::notify( osg::WARN ) << "osgOQ: OcclusionQueryNode:: Invalid debug support node." << std::endl;
        return NULL;
    }
    return _debugGeode->getStateSet();
}

bool
OcclusionQueryNode::getPassed() const
{
    return _passed;
}


void
OcclusionQueryNode::createSupportNodes()
{
    GLushort indices[] = { 0, 1, 2, 3,  4, 5, 6, 7,
        0, 3, 6, 5,  2, 1, 4, 7,
        5, 4, 1, 0,  2, 7, 6, 3 };

    {
        // Add the test geometry Geode
        _queryGeode = new osg::Geode;
        _queryGeode->setName( "OQTest" );
        _queryGeode->setDataVariance( osg::Object::DYNAMIC );

        osg::ref_ptr< QueryGeometry > geom = new QueryGeometry( getName() );
        geom->setDataVariance( osg::Object::DYNAMIC );
        geom->addPrimitiveSet( new osg::DrawElementsUShort(
                    osg::PrimitiveSet::QUADS, 24, indices ) );

        _queryGeode->addDrawable( geom.get() );
    }

    {
        // Add a Geode that is a visual representation of the
        //   test geometry for debugging purposes
        _debugGeode = new osg::Geode;
        _debugGeode->setName( "Debug" );
        _debugGeode->setDataVariance( osg::Object::DYNAMIC );

        osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
        geom->setDataVariance( osg::Object::DYNAMIC );

        osg::ref_ptr<osg::Vec4Array> ca = new osg::Vec4Array;
        ca->push_back( osg::Vec4( 1.f, 1.f, 1.f, 1.f ) );
        geom->setColorArray( ca.get() );
        geom->setColorBinding( osg::Geometry::BIND_OVERALL );

        geom->addPrimitiveSet( new osg::DrawElementsUShort(
                    osg::PrimitiveSet::QUADS, 24, indices ) );

        _debugGeode->addDrawable( geom.get() );
    }

    // Creste state sets. Note that the osgOQ visitors (which place OQNs throughout
    //   the scene graph) create a single instance of these StateSets shared
    //   between all OQNs for efficiency.
    setQueryStateSet( initOQState() );
    setDebugStateSet( initOQDebugState() );
}


void
OcclusionQueryNode::releaseGLObjects( osg::State* state ) const
{
    // Query object discard and deletion is handled by QueryGeometry support class.
    OcclusionQueryNode* nonConstThis = const_cast< OcclusionQueryNode* >( this );
    QueryGeometry* qg = dynamic_cast< QueryGeometry* >( nonConstThis->_queryGeode->getDrawable( 0 ) );
    qg->releaseGLObjects( state );
}

void
OcclusionQueryNode::flushDeletedQueryObjects( unsigned int contextID, double currentTime, double& availableTime )
{
    // Query object discard and deletion is handled by QueryGeometry support class.
    QueryGeometry::flushDeletedQueryObjects( contextID, currentTime, availableTime );
}

void
OcclusionQueryNode::discardDeletedQueryObjects( unsigned int contextID )
{
    // Query object discard and deletion is handled by QueryGeometry support class.
    QueryGeometry::discardDeletedQueryObjects( contextID );
}


}
