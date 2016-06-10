/* -*-c++-*- Producer - Copyright (C) 2001-2004  Don Burns
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
 */

#ifdef WIN32
#include <windows.h>
#endif

#include <string.h>
#include <algorithm>

#include "Camera.h"

using namespace osgProducer;

Camera::Camera( void )
{
    _index = 0;

    _projrectLeft   = 0.0;
    _projrectRight  = 1.0;
    _projrectBottom = 0.0;
    _projrectTop    = 1.0;

    osg::Matrix::value_type  id[] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    memcpy( _viewMatrix, id, sizeof(osg::Matrix::value_type[16]));
    _offset._xshear = _offset._yshear = 0.0f;
    memcpy( _offset._matrix, id, sizeof(osg::Matrix::value_type[16]));
    _offset._multiplyMethod = Offset::PreMultiply;

    _lens = new Lens;
    _lens->setAutoAspect(true);
    _rs = new RenderSurface;

    _clear_color[0] = 0.2f;
    _clear_color[1] = 0.2f;
    _clear_color[2] = 0.4f;
    _clear_color[3] = 1.0f;

    _focal_distance = 1.0;

    _shareLens = true;
    _shareView = true;

    _enabled = true;
    _initialized = false;

}

Camera::~Camera( void )
{
}


const osg::Matrix::value_type * Camera::getViewMatrix( void ) const
{
    return _viewMatrix;
}

void Camera::setViewByMatrix( const osg::Matrix &mat )
{
    osg::Matrix m;
    if ( _offset._multiplyMethod == Offset::PostMultiply )
      m = osg::Matrix( _offset._matrix ) * mat;
    else if( _offset._multiplyMethod == Offset::PreMultiply )
        m = mat * osg::Matrix( _offset._matrix );
    memcpy( _viewMatrix, m.ptr(), sizeof( osg::Matrix::value_type[16] ));
}

void Camera::setViewByLookat( const osg::Vec3 &eye, const osg::Vec3 &center, const osg::Vec3 &up )
{
    osg::Matrix m;
    m.makeLookAt(eye,center,up);
    setViewByMatrix( m );
}

void Camera::setViewByLookat( float eyeX, float eyeY, float eyeZ,
                              float centerX, float centerY, float centerZ,
                              float upX, float upY, float upZ )
{
    setViewByLookat( osg::Vec3(eyeX, eyeY, eyeZ),
                     osg::Vec3(centerX, centerY, centerZ ),
                     osg::Vec3(upX, upY, upZ) );
}


Camera::Lens::Lens( void )
{
// original defaults.
//     _left         = -0.5;
//     _right        =  0.5;
//     _bottom       = -0.5;
//     _top          =  0.5;

    // Setting of the frustum which are appropriate for
    // a monitor which is 26cm high, 50cm distant from the
    // viewer and an horzintal/vetical aspect ratio of 1.25.
    // This assumed to be a reasonable average setting for end users.
    _left         = -0.32;
    _right        =  0.32;
    _bottom       = -0.26;
    _top          =  0.26;
    _ortho_left   = -1.0;
    _ortho_right  =  1.0;
    _ortho_bottom = -1.0;
    _ortho_top    =  1.0;
    _nearClip     =  1.0;
    _farClip      =  1e6;
    _auto_aspect  = false;
    _updateFOV();
    _projection = Perspective;
}

void Camera::Lens::setAspectRatio( double aspectRatio )
{
    _aspect_ratio = aspectRatio;
    _left  = -0.5 * (_top - _bottom) * _aspect_ratio;
    _right =  0.5 * (_top - _bottom) * _aspect_ratio;
    _ortho_left  = -0.5 * (_ortho_top - _ortho_bottom) * _aspect_ratio;
    _ortho_right =  0.5 * (_ortho_top - _ortho_bottom) * _aspect_ratio;

    if( _projection == Perspective )
    _updateFOV();
}

void Camera::Lens::setPerspective( double hfov,   double vfov,
                       double nearClip,   double farClip )
{
    _hfov = osg::DegreesToRadians(hfov);
    _vfov = osg::DegreesToRadians(vfov);
    _aspect_ratio = tan(0.5*_hfov)/tan(0.5*_vfov);

    _nearClip = nearClip;
    _farClip  = farClip;

    _left   = -_nearClip * tan(_hfov/2.0);
    _right  =  _nearClip * tan(_hfov/2.0);
    _bottom = -_nearClip * tan(_vfov/2.0);
    _top    =  _nearClip * tan(_vfov/2.0);

    _projection = Perspective;
    setAutoAspect(false);
}

void Camera::Lens::setFrustum( double left,   double right,
                                  double bottom, double top,
                   double nearClip,   double farClip )
{
    _left = left;
    _right = right;
    _bottom = bottom;
    _top = top;
    _nearClip = nearClip;
    _farClip = farClip;
    _projection = Perspective;
    _updateFOV();
    setAutoAspect(false);
}

void Camera::Lens::setOrtho( double left, double right,
               double bottom, double top,
               double nearClip, double farClip )
{
    _ortho_left = left;
    _ortho_right = right;
    _ortho_bottom = bottom;
    _ortho_top = top;
    _nearClip = nearClip;
    _farClip = farClip;
    _projection = Orthographic;
    setAutoAspect(false);
}

void Camera::Lens::setMatrix( const osg::Matrix::value_type matrix[16] )
{
    memcpy( _matrix, matrix, sizeof(osg::Matrix::value_type[16]) );
    _projection = Manual;
    setAutoAspect(false);
}

bool Camera::Lens::getFrustum( double& left, double& right,
                double& bottom, double& top,
                double& zNear, double& zFar ) const
{
    //The following code was taken from osg's matrix implementation of getFrustum
    if (_matrix[3]!=0.0 || _matrix[7]!=0.0 || _matrix[11]!=-1.0 || _matrix[15]!=0.0) return false;

    zNear = _matrix[14] / (_matrix[10]-1.0);
    zFar = _matrix[14] / (1.0+_matrix[10]);

    left = zNear * (_matrix[8]-1.0) / _matrix[0];
    right = zNear * (1.0+_matrix[8]) / _matrix[0];

    top = zNear * (1.0+_matrix[9]) / _matrix[5];
    bottom = zNear * (_matrix[9]-1.0) / _matrix[5];

    return true;
}

bool Camera::Lens::getOrtho( double& left, double& right,
                double& bottom, double& top,
                double& zNear, double& zFar ) const
{
    //The following code was taken from osg's matrix implementation of getOrtho
    if (_matrix[3]!=0.0 || _matrix[7]!=0.0 || _matrix[11]!=0.0 || _matrix[15]!=1.0) return false;

    zNear = (_matrix[14]+1.0) / _matrix[10];
    zFar = (_matrix[14]-1.0) / _matrix[10];

    left = -(1.0+_matrix[12]) / _matrix[0];
    right = (1.0-_matrix[12]) / _matrix[0];

    bottom = -(1.0+_matrix[13]) / _matrix[5];
    top = (1.0-_matrix[13]) / _matrix[5];

    return true;
}

bool Camera::Lens::convertToOrtho( float d )
{

    if( _projection == Manual )
    {
        //Need to extract frustum values from manual matrix
        if( !getFrustum(_left,_right,_bottom,_top,_nearClip,_farClip) )
            return false;

        _updateFOV();
    }

    double s = d * tan(_vfov*0.5);
    _ortho_bottom = -s;
    _ortho_top = s;
    _ortho_left = -s*_aspect_ratio;
    _ortho_right = s*_aspect_ratio;
    _projection = Orthographic;
    return true;
}

bool Camera::Lens::convertToPerspective( float d )
{

    if( _projection == Manual )
    {
        //Need to extract ortho values from manual matrix
        if( !getOrtho(_ortho_left,_ortho_right,_ortho_bottom,_ortho_top,_nearClip,_farClip) )
            return false;
    }

    double hfov = 2 * atan( 0.5 * (_ortho_right - _ortho_left)/d);
    double vfov = 2 * atan( 0.5 * (_ortho_top - _ortho_bottom)/d);

    _left   = -_nearClip * tan(hfov*0.5);
    _right  =  _nearClip * tan(hfov*0.5);
    _bottom = -_nearClip * tan(vfov*0.5);
    _top    =  _nearClip * tan(vfov*0.5);

    _projection = Perspective;
    //_updateMatrix();

    return true;
}

void Camera::Lens::apply(float xshear, float yshear)
{
    generateMatrix(xshear,yshear,_matrix);
}

void Camera::Lens::getParams( double &left, double &right, double &bottom, double &top,
                double &nearClip, double &farClip )
{
    if( _projection == Perspective )
    {
        left   = _left;
        right  = _right;
        bottom = _bottom;
        top    = _top;
    }
    else if( _projection == Orthographic )
    {
        left   = _ortho_left;
        right  = _ortho_right;
        bottom = _ortho_bottom;
        top    = _ortho_top;
    }
    else if( _projection == Manual ) // could only be Manual, but best to make this clear
    {
        // Check if Manual matrix is either a valid perspective or orthographic matrix
        // If neither, then return bogus values -- nothing better we can do
        if(getFrustum(left,right,bottom,top,nearClip,farClip))
            return;

        if(getOrtho(left,right,bottom,top,nearClip,farClip))
            return;

        left   = _left;
        right  = _right;
        bottom = _bottom;
        top    = _top;
    }
    nearClip   = _nearClip;
    farClip    = _farClip;
}

void Camera::setProjectionRectangle( const float left, const float right,
                    const float bottom, const float top )
{
    _projrectLeft   = left;
    _projrectRight  = right;
    _projrectBottom = bottom;
    _projrectTop    = top;
}

void Camera::getProjectionRectangle( float &left, float &right,
                float &bottom, float &top ) const
{
    left   = _projrectLeft;
    right  = _projrectRight;
    bottom = _projrectBottom;
    top    = _projrectTop;
}

void Camera::setProjectionRectangle( int x, int y, unsigned int width, unsigned int height )
{
    int _x, _y;
    unsigned int _w, _h;

    _rs->getWindowRectangle( _x, _y, _w, _h );
#if 0
    if( _w == osgProducer::RenderSurface::UnknownDimension || _h == Producer::RenderSurface::UnknownDimension)
    {
        unsigned int ww;
        unsigned int hh;
        _rs->getScreenSize( ww, hh );
        if( _w == osgProducer::RenderSurface::UnknownDimension )
            _w = ww;
        if( _h == osgProducer::RenderSurface::UnknownDimension )
            _h = hh;
    }
#endif

    _projrectLeft  = float(x - _x)/float(_w);
    _projrectRight = float((x + width) - _x)/float(_w);
    _projrectBottom = float(y - _y)/float(_h);
    _projrectTop    = float((y+height) - _y)/float(_h);
}

void Camera::getProjectionRectangle( int &x, int &y, unsigned int &width, unsigned int &height ) const
{
    int _x, _y;
    unsigned int _w, _h;
    float fx, fy, fw, fh;

    _rs->getWindowRectangle( _x, _y, _w, _h );
#if 0
    if( _w == Producer::RenderSurface::UnknownDimension || _h == Producer::RenderSurface::UnknownDimension )
    {
        unsigned int ww;
        unsigned int hh;
        _rs->getScreenSize( ww, hh );
        if( _w == Producer::RenderSurface::UnknownDimension )
            _w = ww;
        if( _h == Producer::RenderSurface::UnknownDimension )
            _h = hh;
    }
#endif

    fx = _projrectLeft * _w;
    fy = _projrectBottom * _h;
    fw = _w * _projrectRight;
    fh = _h * _projrectTop;

    x = int(fx);
    y = int(fy);

    width = int(fw) - x;
    height = int(fh) - y;
}

void Camera::setClearColor( float r, float g, float b, float a )
{
    _clear_color[0] = r;
    _clear_color[1] = g;
    _clear_color[2] = b;
    _clear_color[3] = a;
}

void Camera::getClearColor( float& red, float& green, float& blue, float& alpha)
{
    red = _clear_color[0];
    green = _clear_color[1];
    blue = _clear_color[2];
    alpha = _clear_color[3];
}


void Camera::clear( void )
{
#if 0
    if( !_initialized ) _initialize();
    int x, y;
    unsigned int w, h;
    getProjectionRectangle( x, y, w, h );
    glViewport( x, y, w, h );
    glScissor( x, y, w, h );
    glClearColor( _clear_color[0], _clear_color[1], _clear_color[2], _clear_color[3] );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
#endif
}


#if 0
void Camera::Lens::_updateMatrix( void )
{
    switch( _projection )
    {
        case Perspective :
            _matrix[ 0] = (2 * _nearClip)/(_right - _left);
            _matrix[ 1] = 0.0;
            _matrix[ 2] = 0.0;
            _matrix[ 3] = 0.0;

            _matrix[ 4] = 0.0;
            _matrix[ 5] = (2 * _nearClip)/(_top-_bottom);
            _matrix[ 6] = 0.0;
            _matrix[ 7] = 0.0;

            _matrix[ 8] = (_right + _left)/(_right-_left);
            _matrix[ 9] = (_top+_bottom)/(_top-_bottom);
            _matrix[10] = -(_farClip + _nearClip)/(_farClip-_nearClip);
            _matrix[11] = -1.0;

            _matrix[12] = 0.0;
            _matrix[13] = 0.0;
            _matrix[14] = -(2 * _farClip * _nearClip)/(_farClip-_nearClip);
            _matrix[15] = 0.0;

            _matrix[ 8] += -_xshear;
            _matrix[ 9] += -_yshear;

        _hfov = 2.0 * atan(((_right - _left) * 0.5)/_nearClip);
        _vfov = 2.0 * atan(((_top - _bottom) * 0.5)/_nearClip);

        break;

    case Orthographic :

            _matrix[ 0] = 2/(_ortho_right - _ortho_left);
            _matrix[ 1] = 0.0;
            _matrix[ 2] = 0.0;
            _matrix[ 3] = 0.0;

            _matrix[ 4] = 0.0;
            _matrix[ 5] = 2/(_ortho_top - _ortho_bottom);
            _matrix[ 6] = 0.0;
            _matrix[ 7] = 0.0;

            _matrix[ 8] = 0.0;
            _matrix[ 9] = 0.0;
            //_matrix[10] = -2.0/(_farClip - (-_farClip));
            _matrix[10] = -2.0/(_farClip - _nearClip);
            _matrix[11] = 0.0;

            _matrix[12] = -(_ortho_right+_ortho_left)/(_ortho_right-_ortho_left);
            _matrix[13] = -(_ortho_top+_ortho_bottom)/(_ortho_top-_ortho_bottom);
            //_matrix[14] = -(_farClip+(-_farClip))/(_farClip-(-_farClip));
            _matrix[14] = -(_farClip+_nearClip)/(_farClip-_nearClip);
            _matrix[15] = 1.0;

            _matrix[12] += _xshear;
            _matrix[13] += _yshear;

            //_hfov = 0.0;
            //_vfov = 0.0;

        break;
    }
}
#endif

void Camera::Lens::generateMatrix(float xshear, float yshear, osg::Matrix::value_type matrix[16] )
{
    switch( _projection )
    {
        case Perspective :
            matrix[ 0] = (2 * _nearClip)/(_right - _left);
            matrix[ 1] = 0.0;
            matrix[ 2] = 0.0;
            matrix[ 3] = 0.0;

            matrix[ 4] = 0.0;
            matrix[ 5] = (2 * _nearClip)/(_top-_bottom);
            matrix[ 6] = 0.0;
            matrix[ 7] = 0.0;

            matrix[ 8] = (_right + _left)/(_right-_left);
            matrix[ 9] = (_top+_bottom)/(_top-_bottom);
            matrix[10] = -(_farClip + _nearClip)/(_farClip-_nearClip);
            matrix[11] = -1.0;

            matrix[12] = 0.0;
            matrix[13] = 0.0;
            matrix[14] = -(2 * _farClip * _nearClip)/(_farClip-_nearClip);
            matrix[15] = 0.0;

            matrix[ 8] += -xshear;
            matrix[ 9] += -yshear;


        break;

    case Orthographic :

            matrix[ 0] = 2/(_ortho_right - _ortho_left);
            matrix[ 1] = 0.0;
            matrix[ 2] = 0.0;
            matrix[ 3] = 0.0;

            matrix[ 4] = 0.0;
            matrix[ 5] = 2/(_ortho_top - _ortho_bottom);
            matrix[ 6] = 0.0;
            matrix[ 7] = 0.0;

            matrix[ 8] = 0.0;
            matrix[ 9] = 0.0;
            //_matrix[10] = -2.0/(_farClip - (-_farClip));
            matrix[10] = -2.0/(_farClip - _nearClip);
            matrix[11] = 0.0;

            matrix[12] = -(_ortho_right+_ortho_left)/(_ortho_right-_ortho_left);
            matrix[13] = -(_ortho_top+_ortho_bottom)/(_ortho_top-_ortho_bottom);
            //_matrix[14] = -(_farClip+(-_farClip))/(_farClip-(-_farClip));
            matrix[14] = -(_farClip+_nearClip)/(_farClip-_nearClip);
            matrix[15] = 1.0;

            matrix[12] += xshear;
            matrix[13] += yshear;

        break;

    case Manual:

        memcpy( matrix, _matrix, sizeof(osg::Matrix::value_type[16]));

        if(xshear || yshear)
        {
            if (matrix[3]!=0.0 || matrix[7]!=0.0 || matrix[11]!=0.0 || matrix[15]!=1.0)
            {
                // It's not an orthographic matrix so just assume a perspective shear
                matrix[ 8] += -xshear;
                matrix[ 9] += -yshear;
            }
            else
            {
                 matrix[12] += xshear;
                 matrix[13] += yshear;
            }
        }
        break;
    }
}

void Camera::Lens::_updateFOV()
{
    _hfov = 2.0 * atan(((_right - _left) * 0.5)/_nearClip);
    _vfov = 2.0 * atan(((_top - _bottom) * 0.5)/_nearClip);
    _aspect_ratio = tan(0.5*_hfov)/tan(0.5*_vfov);
}


void Camera::setOffset( const osg::Matrix::value_type matrix[16],  const osg::Matrix::value_type xshear,  const osg::Matrix::value_type yshear )
{
    memcpy( _offset._matrix, matrix, sizeof(osg::Matrix::value_type[16]));
    _offset._xshear = xshear;
    _offset._yshear = yshear;
}

void Camera::setOffset(  double xshear, double yshear )
{
    _offset._xshear = xshear;
    _offset._yshear = yshear;
}

#if 0
void Camera::setSyncBarrier( RefBarrier *b )
{
    _syncBarrier = b;
}

void Camera::setFrameBarrier( RefBarrier *b )
{
    _frameBarrier = b;
}

int Camera::cancel()
{
#if 1
    _done = true;
#endif

    Thread::cancel();
    return 0;
}

void Camera::advance()
{
    _rs->makeCurrent();
    _rs->swapBuffers();
}

void Camera::run( void )
{
    if( !_syncBarrier.valid() || !_frameBarrier.valid() )
    {
        std::cerr << "Camera::run() : Threaded Camera requires a Barrier\n";
        return;
    }

    _done = false;

    _initialize();
    _syncBarrier->block();
    while( !_done )
    {
        // printf("   Camera::run before frame block\n");

        _frameBarrier->block();

        if (_done) break;

        // printf("   Camera::run after frame block\n");

        frame(false);

        if (_done) break;

        // printf("   Camera::run before sycn block\n");

        _syncBarrier->block();

        if (_done) break;

        // printf("   Camera::run after sycn block\n");

        advance();
    }

    // printf("Exiting Camera::run cleanly\n");
}



bool Camera::removePreCullCallback( Callback *cb )
{
    return _removeCallback( preCullCallbacks, cb );
}

bool Camera::removePostCullCallback( Callback *cb )
{
    return _removeCallback( postCullCallbacks, cb );
}

bool Camera::removePreDrawCallback( Callback *cb )
{
    return _removeCallback( preDrawCallbacks, cb );
}

bool Camera::removePostDrawCallback( Callback *cb )
{
    return _removeCallback( postDrawCallbacks, cb );
}

bool Camera::removePostSwapCallback( Callback *cb )
{
    return _removeCallback( postSwapCallbacks, cb );
}


bool Camera::_removeCallback( std::vector < ref_ptr<Callback> > &callbackList, Callback *callback )
{
    std::vector < Producer::ref_ptr< Producer::Camera::Callback> >::iterator p;
    p = std::find( callbackList.begin(), callbackList.end(), callback );
    if( p == callbackList.end() )
        return false;

    callbackList.erase( p );
    return true;
}

Camera::FrameTimeStampSet::FrameTimeStampSet():
    _pipeStatsDoubleBufferIndex(0),
    _pipeStatsFirstSync(true),
    _initialized(false)
{
    for( unsigned int i = 0; i < LastPipeStatsID; i++ )
        _pipeStats[i] = 0.0;
}

Camera::FrameTimeStampSet::~FrameTimeStampSet()
{
}

void Camera::FrameTimeStampSet::syncPipeStats()
{
    if( !_initialized )
        return;

    if( _pipeStatsFirstSync == true )
    {
        _pipeStatsFirstSync = false;
        return;
    }

    // We get the stats from the previous frame
    for( int i = 0; i < LastPipeStatsID; i++ )
    {
        if( _pipeStatsSetMask[1 - _pipeStatsDoubleBufferIndex]  & (1<<i) )
            _pipeStats[i] = PipeTimer::instance()->getElapsedTime( _pipeStatsNames[i][ 1 - _pipeStatsDoubleBufferIndex] );
    }

    _pipeStatsFrameNumber = _frameNumber - 1;
    _pipeStatsDoubleBufferIndex = 1 - _pipeStatsDoubleBufferIndex;
    _pipeStatsSetMask[_pipeStatsDoubleBufferIndex] = 0;
}

void Camera::FrameTimeStampSet::beginPipeTimer( PipeStatsID id)
{
    if( !_initialized )
        _init();

    PipeTimer::instance()->begin( _pipeStatsNames[id][_pipeStatsDoubleBufferIndex] );
    _pipeStatsSetMask[_pipeStatsDoubleBufferIndex] |= (1<<id);
}

void Camera::FrameTimeStampSet::endPipeTimer()
{
    if( !_initialized )
        return;

    PipeTimer::instance()->end() ;
}

void Camera::FrameTimeStampSet::_init()
{
    if( _initialized )
        return;

    for( unsigned int i = 0; i < (unsigned int)LastPipeStatsID; i++ )
        PipeTimer::instance()->genQueries( 2, _pipeStatsNames[i] );

    _pipeStatsSetMask[0] = 0;
    _pipeStatsSetMask[1] = 0;

    _initialized = true;
}

const Camera::FrameTimeStampSet &Camera::getFrameStats()
{
    return _frameStamps;
}
#endif
