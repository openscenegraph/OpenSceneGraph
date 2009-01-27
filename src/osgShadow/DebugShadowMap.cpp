/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
 *
 * ViewDependentShadow codes Copyright (C) 2008 Wojciech Lewandowski
 * Thanks to to my company http://www.ai.com.pl for allowing me free this work.
*/

#include <osgShadow/DebugShadowMap>
#include <osgShadow/ConvexPolyhedron>
#include <osgUtil/RenderLeaf>
#include <osg/Geometry>
#include <osg/PrimitiveSet>
#include <osg/MatrixTransform>
#include <osg/Depth>
#include <iostream>
#include <iomanip>

using namespace osgShadow;


#define VECTOR_LENGTH( v ) ( sizeof(v)/sizeof(v[0]) )

#define DEFAULT_DEBUG_HUD_SIZE_X 256
#define DEFAULT_DEBUG_HUD_SIZE_Y 256
#define DEFAULT_DEBUG_HUD_ORIGIN_X 8
#define DEFAULT_DEBUG_HUD_ORIGIN_Y 8

DebugShadowMap::DebugShadowMap(): 
    BaseClass(),
    _hudSize( 2, 2 ),
    _hudOrigin( -1, -1 ),
    _viewportSize( DEFAULT_DEBUG_HUD_SIZE_X, DEFAULT_DEBUG_HUD_SIZE_Y ),
    _viewportOrigin( DEFAULT_DEBUG_HUD_ORIGIN_X, DEFAULT_DEBUG_HUD_ORIGIN_Y ), 
    _orthoSize( 2, 2 ),
    _orthoOrigin( -1, -1 ),
    _doDebugDraw( false )
{
    
    // Why this fancy 24 bit depth to 24 bit rainbow colors shader ?
    //
    // Depth values cannot be easily cast on color component because they are: 
    // a) 24 or 32 bit and we loose lots of precision when cast on 8 bit 
    // b) depth value distribution is non linear due to projection division
    // when cast on componenent color there is usually very minor shade variety
    // and its often difficult to notice that there is anything in the buffer
    // 
    // Shader looks complex but it is used only for debug head-up rectangle 
    // so performance impact is not significant.  
    
    _depthColorFragmentShader = new osg::Shader( osg::Shader::FRAGMENT,
#if 0 
        "uniform sampler2D texture;                                              \n"
        "                                                                        \n"
        "void main(void)                                                         \n"
        "{                                                                       \n"
        "    float f = texture2D( texture, vec3( gl_TexCoord[0].xy, 1.0).xy ).r; \n"
        "    gl_FragColor =  vec4( 0.0, 1.0 - f,  0.5 - f, 0.5 );                \n"
        "}                                                                       \n"          
#else 
        "uniform sampler2D texture;                                              \n"
        "                                                                        \n"
        "void main(void)                                                         \n"
        "{                                                                       \n"
        "    float f = texture2D( texture, vec3( gl_TexCoord[0].xy, 1.0).xy ).r; \n"
        "                                                                        \n"
        "    f = 256.0 * f;                                                      \n"
        "    float fC = floor( f ) / 256.0;                                      \n" 
        "                                                                        \n"
        "    f = 256.0 * fract( f );                                             \n"
        "    float fS = floor( f ) / 256.0;                                      \n"  
        "                                                                        \n"
        "    f = 256.0 * fract( f );                                             \n"
        "    float fH = floor( f ) / 256.0;                                      \n"
        "                                                                        \n"
        "    fS *= 0.5;                                                          \n"
        "    fH = ( fH  * 0.34 + 0.66 ) * ( 1.0 - fS );                          \n"
        "                                                                        \n"
        "    vec3 rgb = vec3( ( fC > 0.5 ? ( 1.0 - fC ) : fC ),                  \n"
        "                     abs( fC - 0.333333 ),                              \n"
        "                     abs( fC - 0.666667 ) );                            \n"
        "                                                                        \n"   
        "    rgb = min( vec3( 1.0, 1.0, 1.0 ), 3.0 * rgb );                      \n"
        "                                                                        \n"
        "    float fMax = max( max( rgb.r, rgb.g ), rgb.b );                     \n"
        "    fMax = 1.0 / fMax;                                                  \n"  
        "                                                                        \n"
        "    vec3 color = fMax * rgb;                                            \n"
        "                                                                        \n"
        "    gl_FragColor =  vec4( fS + fH * color, 1 ) * gl_Color;              \n"
        "}                                                                       \n"  
#endif
    ); // end _depthColorFragmentShader 
}

DebugShadowMap::DebugShadowMap
(const DebugShadowMap& copy, const osg::CopyOp& copyop) :
    BaseClass(copy,copyop),
    _hudSize( copy._hudSize ),
    _hudOrigin( copy._hudOrigin ),
    _viewportSize( copy._viewportSize ),
    _viewportOrigin( copy._viewportOrigin ), 
    _orthoSize( copy._viewportOrigin ),
    _orthoOrigin( copy._viewportOrigin ),
    _doDebugDraw( copy._doDebugDraw )
{
    if( copy._depthColorFragmentShader.valid() )
        _depthColorFragmentShader =
            dynamic_cast<osg::Shader*>
                ( copy._depthColorFragmentShader->clone(copyop) );
}

DebugShadowMap::~DebugShadowMap() 
{
}

void DebugShadowMap::ViewData::cull( void )
{
    if( getDebugDraw() && !_cameraDebugHUD.valid() )
        createDebugHUD();

    BaseClass::ViewData::cull( );

    cullDebugGeometry( );
}

bool DebugShadowMap::ViewData::DebugBoundingBox
    ( const osg::BoundingBox & bb, const char * name )
{
    bool result = false;
#if defined( _DEBUG    ) || defined( DEBUG )
    if( !name ) name = "";

    osg::BoundingBox & bb_prev = _boundingBoxMap[ std::string( name ) ];

    result = bb.center() != bb_prev.center() || bb.radius() != bb_prev.radius();
    if( result )
        std::cout << "Box<" << name << "> (" 
                  << ( bb._max._v[0] + bb._min._v[0] ) * 0.5 << " "
                  << ( bb._max._v[1] + bb._min._v[1] ) * 0.5 << " " 
                  << ( bb._max._v[2] + bb._min._v[2] ) * 0.5 << ") ["
                  << ( bb._max._v[0] - bb._min._v[0] ) << " "
                  << ( bb._max._v[1] - bb._min._v[1] ) << " "
                  << ( bb._max._v[2] - bb._min._v[2] ) << "] "
                  << std::endl;

    bb_prev = bb;
#endif
    return result;
}

bool DebugShadowMap::ViewData::DebugPolytope
( const osg::Polytope & p, const char * name )
{
    bool result = false;
#if defined( _DEBUG    ) || defined( DEBUG )
    if( !name ) name = "";

    osg::Polytope & p_prev = _polytopeMap[ std::string( name ) ];

    result = ( p.getPlaneList() != p_prev.getPlaneList() );

    if( result ) {
        std::cout << "Polytope<" << name 
                  << "> size(" << p.getPlaneList().size() << ")"
                  << std::endl;

        if( p.getPlaneList().size() == p_prev.getPlaneList().size() ) {
            for( unsigned i = 0; i < p.getPlaneList().size(); ++i )
            {
                if( p.getPlaneList()[i] != p_prev.getPlaneList()[i] )
                {
                    std::cout << "Plane<" << i 
                        << "> (" 
                        << p.getPlaneList()[i].asVec4()[0] << ", "
                        << p.getPlaneList()[i].asVec4()[1] << ", "
                        << p.getPlaneList()[i].asVec4()[2] << ", "
                        << p.getPlaneList()[i].asVec4()[3] << ")"
                        << std::endl;                    
                }
            }
        }
    }

    p_prev = p;
#endif
    return result;
}

bool DebugShadowMap::ViewData::DebugMatrix
    ( const osg::Matrix & m, const char * name )
{
    bool result = false;
#if defined( _DEBUG    ) || defined( DEBUG )
    if( !name ) name = "";

    osg::Matrix & m_prev = _matrixMap[ std::string( name ) ];

    result = ( m != m_prev );

    if( result )
        std::cout << "Matrix<" << name << "> " << std::endl 
            <<"[ " << m(0,0) << " " << m(0,1) << " " << m(0,2) << " " << m(0,3) << " ]  " << std::endl
            <<"[ " << m(1,0) << " " << m(1,1) << " " << m(1,2) << " " << m(1,3) << " ]  " << std::endl
            <<"[ " << m(2,0) << " " << m(2,1) << " " << m(2,2) << " " << m(2,3) << " ]  " << std::endl
            <<"[ " << m(3,0) << " " << m(3,1) << " " << m(3,2) << " " << m(3,3) << " ]  " << std::endl;

    m_prev = m;
#endif
    return result;
}

void DebugShadowMap::ViewData::setDebugPolytope
    ( const char * name, const ConvexPolyhedron & polytope, 
      osg::Vec4 colorOutline, osg::Vec4 colorInside )
{
    if( !getDebugDraw() ) return;

    if( &polytope == NULL ) { // delete        
        PolytopeGeometry & pg = _polytopeGeometryMap[ std::string( name ) ];
        for( unsigned int i = 0; i < VECTOR_LENGTH( pg._geometry ) ; i++ )
        {
            if( pg._geometry[i].valid() ) {
                if( _geode[i].valid() &&
                    _geode[i]->containsDrawable( pg._geometry[i].get() ) )
                        _geode[i]->removeDrawable( pg._geometry[i].get() );

                pg._geometry[i] = NULL;
            }
        }
        _polytopeGeometryMap.erase( std::string( name ) );
    } else { // update
        PolytopeGeometry & pg = _polytopeGeometryMap[ std::string( name ) ];

        pg._polytope = polytope;
        if( colorOutline.a() > 0 )
            pg._colorOutline = colorOutline;
        if( colorInside.a() > 0 )
            pg._colorInside = colorInside;

        for( unsigned int i = 0; i < VECTOR_LENGTH( pg._geometry ) ; i++ )
        {
            if( !pg._geometry[i].valid() ) {
                pg._geometry[i] = new osg::Geometry;
                pg._geometry[i]->setDataVariance( osg::Object::DYNAMIC );
                pg._geometry[i]->setUseDisplayList( false );
                pg._geometry[i]->setSupportsDisplayList( false );
            }

            if( _geode[i].valid() && 
                  !_geode[i]->containsDrawable( pg._geometry[i].get() ) ) {
                        osg::Geode::DrawableList & dl = 
                           const_cast< osg::Geode::DrawableList &>
                              ( _geode[i]->getDrawableList() );
                        dl.insert( dl.begin(), pg._geometry[i].get() );
            }            
        }       
    }
}

void DebugShadowMap::ViewData::updateDebugGeometry
    ( const osg::Camera * viewCam, const osg::Camera * shadowCam )
{
    if( !getDebugDraw() ) return;
    if( _polytopeGeometryMap.empty() ) return;

    const int num = 2; // = VECTOR_LENGTH( PolytopeGeometry::_geometry );
    
    osg::Matrix    
        transform[ num ] = 
            { viewCam->getViewMatrix() * 
                // use near far clamped projection ( precomputed in cullDebugGeometry )
                ( _viewCamera==viewCam ? _viewProjection : viewCam->getProjectionMatrix() ),
              shadowCam->getViewMatrix() * shadowCam->getProjectionMatrix() }, 
        inverse[ num ] = 
            { osg::Matrix::inverse( transform[0] ),
              osg::Matrix::inverse( transform[1] ) };

#if 0
    ConvexPolyhedron frustum[ num ];
    for( int i = 0; i < num; i++ ) {
        frustum[i].setToUnitFrustum( );
#if 1
        frustum[i].transform( inverse[i], transform[i] );
#else
        frustum[i].transform 
            ( osg::Matrix::inverse( camera[i]->getProjectionMatrix() ),
              camera[i]->getProjectionMatrix() );
        frustum[i].transform 
            ( osg::Matrix::inverse( camera[i]->getViewMatrix() ),
              camera[i]->getViewMatrix() );
#endif        
    };
#else
    osg::Polytope frustum[ num ];
    for( int i = 0; i < num; i++ ) {
        frustum[i].setToUnitFrustum( );
        frustum[i].transformProvidingInverse( transform[i] );
    }
#endif

    transform[0] = viewCam->getViewMatrix();
    inverse[0] = viewCam->getInverseViewMatrix();

    for( PolytopeGeometryMap::iterator itr = _polytopeGeometryMap.begin();
         itr != _polytopeGeometryMap.end();
         ++itr )
    {
        PolytopeGeometry & pg = itr->second;

        for( int i = 0; i < num ; i++ )
        {

            ConvexPolyhedron cp( pg._polytope );        
            cp.cut( frustum[i] );
            cp.transform( transform[i], inverse[i] );

            pg._geometry[i] = cp.buildGeometry
                ( pg._colorOutline, pg._colorInside, pg._geometry[i].get() );
        }
    }
}

void DebugShadowMap::ViewData::cullDebugGeometry( )
{   
    if( !getDebugDraw() ) return;
    if( !_camera.valid() ) return;

    // View camera may use clamping projection matrix after traversal. 
    // Since we need to know exact matrix for drawing the frusta,
    // we have to compute it here in exactly the same way as cull visitor 
    // will after cull traversal completes view camera subgraph.
    {
        _viewProjection = *_cv->getProjectionMatrix();
        _viewCamera = _cv->getRenderStage()->getCamera();

        if( _cv->getComputeNearFarMode() ) {

            // Redo steps from CullVisitor::popProjectionMatrix()
            // which clamps projection matrix when Camera & Projection 
            // completes traversal of their children

            // We have to do this now manually 
            // because we did not complete camera traversal yet but 
            // we need to know how this clamped projection matrix will be

            _cv->computeNearPlane();
        
            osgUtil::CullVisitor::value_type n = _cv->getCalculatedNearPlane();
            osgUtil::CullVisitor::value_type f = _cv->getCalculatedFarPlane();

            if( n < f )
                _cv->clampProjectionMatrix( _viewProjection, n, f );
        }
    }
    
    updateDebugGeometry( _viewCamera.get(), _camera.get() );
    
#if 1 // Add geometries of polytopes to main cam Render Stage 
    _transform[0]->accept( *_cv );
#else
    for( PolytopeGeometryMap::iterator itr = _polytopeGeometryMap.begin();
         itr != _polytopeGeometryMap.end();
         ++itr )
    {
        PolytopeGeometry & pg = itr->second;
        _cv->pushStateSet( _geode[0]->getStateSet() );
        _cv->addDrawableAndDepth( pg._geometry[0].get(), NULL, FLT_MAX );
        _cv->popStateSet( );
    }
#endif

    // Add geometries of polytopes to hud cam Render Stage
    _cameraDebugHUD->accept( *_cv );
}

void DebugShadowMap::ViewData::init( ThisClass *st, osgUtil::CullVisitor *cv )
{
    BaseClass::ViewData::init( st, cv );

    _doDebugDrawPtr           = &st->_doDebugDraw;

    _hudSize                  = st->_hudSize;
    _hudOrigin                = st->_hudOrigin;
    _viewportSize             = st->_viewportSize;
    _viewportOrigin           = st->_viewportOrigin;
    _orthoSize                = st->_orthoSize;
    _orthoOrigin              = st->_orthoOrigin;
    
    _depthColorFragmentShader = st->_depthColorFragmentShader;

    // create placeholder geodeds for polytope geometries
    // rest of their initialization will be performed during camera hud init
    _geode[0] = new osg::Geode;
    _geode[1] = new osg::Geode;
    
    _cameraDebugHUD           = NULL;//Force debug HUD rebuild ( if needed )
}


// Callback used by debugging hud to display Shadow Map to color buffer
// Had to do it this way because OSG does not allow to use 
// the same GL Texture Id with different glTexParams. 
// Callback simply turns compare mode off via GL while rendering hud and 
// restores it before rendering the scene with shadows. 
class DebugShadowMap::DrawableDrawWithDepthShadowComparisonOffCallback: 
    public osg::Drawable::DrawCallback
{
public:
    DrawableDrawWithDepthShadowComparisonOffCallback( osg::Texture2D *pTex ) 
        : _pTexture( pTex )
    {
    }

    virtual void drawImplementation
        ( osg::RenderInfo & ri,const osg::Drawable* drawable ) const
    {
        ri.getState()->applyTextureAttribute( 0, _pTexture.get() );

        // Turn off depth comparison mode
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE );

        drawable->drawImplementation(ri);

        // Turn it back on
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, 
            GL_COMPARE_R_TO_TEXTURE_ARB );            
    }

    osg::ref_ptr< osg::Texture2D > _pTexture;
};

void DebugShadowMap::ViewData::createDebugHUD( )
{    
    _cameraDebugHUD = new osg::Camera;

    { // Make sure default HUD layout makes sense
        if( _hudSize[0] <= 0 ) _hudSize[0] = DEFAULT_DEBUG_HUD_SIZE_X;
        if( _hudSize[1] <= 0 ) _hudSize[1] = DEFAULT_DEBUG_HUD_SIZE_Y;

        if( _viewportSize[0] <= 0 ) _viewportSize[0] = _hudSize[0];
        if( _viewportSize[1] <= 0 ) _viewportSize[1] = _hudSize[1];

        if( _orthoSize[0] <= 0 ) _orthoSize[0] = _viewportSize[0];
        if( _orthoSize[1] <= 0 ) _orthoSize[1] = _viewportSize[1];
    }

    { // Initialize hud camera    
        osg::Camera * camera = _cameraDebugHUD.get();
        camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);
        camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
        camera->setViewMatrix(osg::Matrix::identity());
        camera->setViewport( _viewportOrigin[0], _viewportOrigin[1],
                             _viewportSize[0], _viewportSize[1] );

        camera->setProjectionMatrixAsOrtho( 
                _orthoOrigin[0], _orthoOrigin[0] + _orthoSize[0], 
                _orthoOrigin[1], _orthoOrigin[1] + _orthoSize[1],
                -10, 10 );   

        camera->setClearMask(GL_DEPTH_BUFFER_BIT);
        camera->setRenderOrder(osg::Camera::POST_RENDER);
    }

    { // Add geode and drawable with BaseClass display
        // create geode to contain hud drawables
        osg::Geode* geode = new osg::Geode;
        _cameraDebugHUD->addChild(geode);

        // finally create and attach hud geometry 
        osg::Geometry* geometry = osg::createTexturedQuadGeometry
            ( osg::Vec3(_hudOrigin[0],_hudOrigin[1],0),
              osg::Vec3(_hudSize[0],0,0), 
              osg::Vec3(0,_hudSize[1],0) );
    
        osg::StateSet* stateset = _cameraDebugHUD->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0,_texture.get(),osg::StateAttribute::ON );
        stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
//        stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
        stateset->setAttributeAndModes
            ( new osg::Depth( osg::Depth::ALWAYS, 0, 1, false ) );
        stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
    
        osg::Program* program = new osg::Program;
        program->addShader( _depthColorFragmentShader.get() );
        stateset->setAttribute( program );
        stateset->addUniform( new osg::Uniform( "texture" , 0 ) );

        geometry->setDrawCallback
            ( new DrawableDrawWithDepthShadowComparisonOffCallback( _texture.get() ) );

        geode->addDrawable( geometry );
    }

    { // Create transforms and geodes to manage polytope drawing
        osg::StateSet * stateset = new osg::StateSet;
        stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        stateset->setTextureMode(0, GL_TEXTURE_2D, osg::StateAttribute::OFF);
        stateset->setTextureMode(1, GL_TEXTURE_2D, osg::StateAttribute::OFF);
        stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
        stateset->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
        stateset->setAttribute( new osg::Program() );           
        stateset->setAttributeAndModes
            ( new osg::Depth( osg::Depth::LEQUAL, 0, 1, false ) );

        for( int i = 0; i < 2; i++ ) {
            _geode[i]->setStateSet( stateset );
            _transform[i] = new osg::MatrixTransform;
            _transform[i]->addChild( _geode[i].get() );
            _transform[i]->setMatrix( osg::Matrix::identity() );
            _transform[i]->setReferenceFrame( osg::MatrixTransform::ABSOLUTE_RF );
        }

        _transform[1]->setMatrix
            ( osg::Matrix::translate( 1, 1, 0 ) * 
              osg::Matrix::scale( 0.5, 0.5, 1 ) *
              osg::Matrix::scale( _hudSize[0], _hudSize[1], 1 ) * 
              osg::Matrix::translate( _hudOrigin[0], _hudOrigin[1], 0 ) );

        _cameraDebugHUD->addChild( _transform[1].get() );
    }
}

osg::Vec3d DebugShadowMap::ViewData::computeShadowTexelToPixelError
     ( const osg::Matrix & mvpwView,
       const osg::Matrix & mvpwShadow,
       const osg::Vec3d & vWorld, 
       const osg::Vec3d & vDelta )
{
    osg::Vec3d vS0 = mvpwShadow * vWorld;
    osg::Vec3d vS1 = mvpwShadow * ( vWorld + vDelta );

    osg::Vec3d vV0 = mvpwView * vWorld;
    osg::Vec3d vV1 = mvpwView * ( vWorld + vDelta );

    osg::Vec3d dV = vV1 - vV0;
    osg::Vec3d dS = vS1 - vS0;
    
    return osg::Vec3( dS[0] / dV[0], dS[1] / dV[1], dS[2] / dV[2] );
}

void DebugShadowMap::ViewData::displayShadowTexelToPixelErrors
  ( const osg::Camera* viewCamera, 
    const osg::Camera* shadowCamera, 
    const ConvexPolyhedron* hull )
{
    osg::Matrix mvpwMain  =
        viewCamera->getViewMatrix() *
        viewCamera->getProjectionMatrix() *
        viewCamera->getViewport()->computeWindowMatrix();

    osg::Matrix mvpwShadow  =
        shadowCamera->getViewMatrix() *
        shadowCamera->getProjectionMatrix() *
        shadowCamera->getViewport()->computeWindowMatrix();

    osg::BoundingBox bb = 
        hull->computeBoundingBox( viewCamera->getViewMatrix() );

    osg::Matrix m = viewCamera->getInverseViewMatrix();

    osg::Vec3d vn = osg::Vec3d( 0, 0, bb._max[2] ) * m;
    osg::Vec3d vf = osg::Vec3d( 0, 0, bb._min[2] ) * m;
    osg::Vec3d vm = osg::Vec3d( 0, 0, ( bb._max[2] + bb._min[2] ) * 0.5 ) * m;

    osg::Vec3d vne = computeShadowTexelToPixelError( mvpwMain, mvpwShadow, vn );
    osg::Vec3d vfe = computeShadowTexelToPixelError( mvpwMain, mvpwShadow, vf );
    osg::Vec3d vme = computeShadowTexelToPixelError( mvpwMain, mvpwShadow, vm );

    std::cout << std::setprecision( 3 ) << " "
        << "ne=(" << vne[0] << "," << vne[1] << "," << vne[2] << ")  "
        << "fe=(" << vfe[0] << "," << vfe[1] << "," << vfe[2] << ")  "
        << "me=(" << vme[0] << "," << vme[1] << "," << vme[2] << ")  " 
        << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
        << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
        << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
        << std::flush;
}

