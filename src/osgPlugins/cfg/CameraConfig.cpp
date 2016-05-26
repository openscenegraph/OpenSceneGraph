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

#include <osg/Notify>
#include <osg/ref_ptr>
#include <osg/io_utils>

#include <osgDB/FileUtils>

#ifdef _X11_IMPLEMENTATION
#  include <X11/Xlib.h>
#endif

#include <memory.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

#include "CameraConfig.h"


using namespace osgProducer;


unsigned int CameraConfig::getNumberOfScreens()
{
#if 0
    return RenderSurface::getNumberOfScreens();
#else
    return 1;
#endif
}


//////////////////
CameraConfig::CameraConfig() :
    _can_add_visual_attributes(false),
    _current_render_surface(NULL),
    _can_add_render_surface_attributes(false),
    _current_camera(NULL),
    _can_add_camera_attributes(false),
    _input_area(NULL),
    _can_add_input_area_entries(false),
    _offset_shearx(0.0f),
    _offset_sheary(0.0f),
    _postmultiply(false)

{
    _offset_matrix[0] = 1.0; _offset_matrix[1] = 0.0; _offset_matrix[2] = 0.0; _offset_matrix[3] = 0.0;
    _offset_matrix[4] = 0.0; _offset_matrix[5] = 1.0; _offset_matrix[6] = 0.0; _offset_matrix[7] = 0.0;
    _offset_matrix[8] = 0.0; _offset_matrix[9] = 0.0; _offset_matrix[10] = 1.0; _offset_matrix[11] = 0.0;
    _offset_matrix[12] = 0.0; _offset_matrix[13] = 0.0; _offset_matrix[14] = 0.0; _offset_matrix[15] = 1.0;

    _threadModelDirective = CameraGroup::getDefaultThreadModel();

}


void CameraConfig::beginVisual( void )
{
    _current_visual_chooser = new VisualChooser;
    _can_add_visual_attributes = true;
}

void CameraConfig::beginVisual( const char * name )
{
    std::pair<std::map<std::string,VisualChooser *>::iterator,bool> res =
      _visual_map.insert(std::pair<std::string,VisualChooser *>(std::string(name), new VisualChooser));
    _current_visual_chooser = (res.first)->second;
    _can_add_visual_attributes = true;
}

void CameraConfig::setVisualSimpleConfiguration( void )
{
     if( !_current_visual_chooser.valid() || _can_add_visual_attributes == false )
     {
         std::cerr << "CameraConfig::setVisualSimpleConfiguration() : ERROR no current visual\n";
         return;
     }
     _current_visual_chooser->setSimpleConfiguration();
}

void CameraConfig::setVisualByID( unsigned int id )
{
     if( !_current_visual_chooser.valid() || _can_add_visual_attributes == false )
     {
         std::cerr << "CameraConfig::setVisualByID(id) : ERROR no current visual\n";
         return;
     }
     _current_visual_chooser->setVisualID( id );
}

void CameraConfig::addVisualAttribute( VisualChooser::AttributeName token, int param )
{
     if( !_current_visual_chooser.valid() || _can_add_visual_attributes == false )
     {
         std::cerr << "CameraConfig::addVisualAttribute(token,param) : ERROR no current visual\n";
         return;
     }
     _current_visual_chooser->addAttribute( token, param );
}

void CameraConfig::addVisualAttribute( VisualChooser::AttributeName token )
{
     if( !_current_visual_chooser.valid()  || _can_add_visual_attributes == false)
     {
         std::cerr << "CameraConfig::addVisualAttribute(token) : ERROR no current visual\n";
         return;
     }
     _current_visual_chooser->addAttribute( token );
}

void CameraConfig::addVisualExtendedAttribute( unsigned int token )
{
     if( !_current_visual_chooser.valid()  || _can_add_visual_attributes == false)
     {
         std::cerr << "CameraConfig::addVisualExtendedAttribute(token) : ERROR no current visual\n";
         return;
     }
     _current_visual_chooser->addExtendedAttribute( token );
}

void CameraConfig::addVisualExtendedAttribute( unsigned int token, int param )
{
     if( !_current_visual_chooser.valid()  || _can_add_visual_attributes == false)
     {
         std::cerr << "CameraConfig::addVisualExtendedAttribute(token, param) : ERROR no current visual\n";
         return;
     }
     _current_visual_chooser->addExtendedAttribute( token, param );
}

void CameraConfig::endVisual( void )
{
    _can_add_visual_attributes = false;
}

VisualChooser *CameraConfig::findVisual( const char *name )
{
    std::map<std::string, VisualChooser *>::iterator p;
    p = _visual_map.find( std::string(name) );
    if( p == _visual_map.end() )
        return NULL;
    else
            return (*p).second;
}

void CameraConfig::beginRenderSurface( const char *name )
{
    std::pair<std::map<std::string,  osg::ref_ptr<RenderSurface> >::iterator,bool> res =
      _render_surface_map.insert(std::pair<std::string, osg::ref_ptr< RenderSurface> >(
                                      std::string(name),
                                      new RenderSurface));
    _current_render_surface = (res.first)->second.get();
    _current_render_surface->setWindowName( std::string(name) );
    _can_add_render_surface_attributes = true;
}

void CameraConfig::setRenderSurfaceVisualChooser( const char *name )
{
    VisualChooser *vc = findVisual( name );
    if( vc != NULL && _current_render_surface != NULL )
        _current_render_surface->setVisualChooser( vc );
}

void CameraConfig::setRenderSurfaceVisualChooser( void )
{
    if( _current_render_surface != NULL && _current_visual_chooser.valid() )
        _current_render_surface->setVisualChooser( _current_visual_chooser.get() );
}

void CameraConfig::setRenderSurfaceWindowRectangle( int x, int y,  unsigned int width, unsigned int height )
{
    if( _current_render_surface != NULL )
        _current_render_surface->setWindowRectangle( x, y, width, height );
}

void CameraConfig::setRenderSurfaceCustomFullScreenRectangle( int x, int y, unsigned int width, unsigned int height )
{
    if( _current_render_surface != NULL )
        _current_render_surface->setCustomFullScreenRectangle( x, y, width, height );
}

void CameraConfig::setRenderSurfaceOverrideRedirect( bool flag )
{
    if( _current_render_surface != NULL )
        _current_render_surface->useOverrideRedirect( flag );
}

void CameraConfig::setRenderSurfaceHostName( const std::string &name )
{
    if( _current_render_surface != NULL )
        _current_render_surface->setHostName( name );
}

void CameraConfig::setRenderSurfaceDisplayNum( int n )
{
    if( _current_render_surface != NULL )
        _current_render_surface->setDisplayNum( n );
}

void CameraConfig::setRenderSurfaceScreen( int n )
{
    if( _current_render_surface != NULL )
        _current_render_surface->setScreenNum( n );
}

void CameraConfig::setRenderSurfaceBorder( bool flag )
{
    if( _current_render_surface != NULL )
        _current_render_surface->useBorder( flag );
}

void CameraConfig::setRenderSurfaceDrawableType( RenderSurface::DrawableType drawableType )
{
    if( _current_render_surface != NULL )
        _current_render_surface->setDrawableType( drawableType );
}

void CameraConfig::setRenderSurfaceRenderToTextureMode( RenderSurface::RenderToTextureMode rttMode )
{
    if( _current_render_surface != NULL )
        _current_render_surface->setRenderToTextureMode( rttMode );
}

void CameraConfig::setRenderSurfaceReadDrawable( const char *name )
{
    if( _current_render_surface != NULL )
    {
        osgProducer::RenderSurface *readDrawable = findRenderSurface( name );
        if( readDrawable == NULL )
        {
            std::cerr << "setRenderSurfaceReadDrawable(): No Render Surface by name of \"" << name << "\" was found!\n";
            return;
        }
        _current_render_surface->setReadDrawable( readDrawable );
    }
}

void CameraConfig::setRenderSurfaceInputRectangle( float x0, float x1, float y0, float y1 )
{
    if( _current_render_surface != NULL )
        _current_render_surface->setInputRectangle( RenderSurface::InputRectangle(x0,x1,y0,y1) );
}

void CameraConfig::endRenderSurface( void )
{
    _can_add_render_surface_attributes = false;
}

RenderSurface *CameraConfig::findRenderSurface( const char *name )
{
    std::map<std::string,  osg::ref_ptr<RenderSurface> >::iterator p;
    p = _render_surface_map.find( std::string(name) );
    if( p == _render_surface_map.end() )
        return NULL;
    else
        return (*p).second.get();
}

unsigned int CameraConfig::getNumberOfRenderSurfaces()
{
    return _render_surface_map.size();
}

RenderSurface *CameraConfig::getRenderSurface( unsigned int index )
{
    if( index >= _render_surface_map.size() )
            return NULL;
    std::map <std::string,  osg::ref_ptr<RenderSurface> >::iterator p;

    unsigned int i = 0;
    for( p = _render_surface_map.begin(); p != _render_surface_map.end(); p++ )
        if( i++ == index )
                break;
    if(  p == _render_surface_map.end() )
            return NULL;
    return (p->second.get());
}

void CameraConfig::addCamera( std::string cameraName, Camera *camera )
{
    std::pair<std::map<std::string, osg::ref_ptr<Camera> >::iterator,bool> res =
      _camera_map.insert(std::pair<std::string, osg::ref_ptr<Camera> >(cameraName, camera));
    _current_camera = (res.first)->second.get();
    _can_add_camera_attributes = true;

    RenderSurface *rs = camera->getRenderSurface();
    if( rs->getWindowName() == osgProducer::RenderSurface::defaultWindowName )
    {
        char name[80];
        sprintf( name, "%s (%02d)", osgProducer::RenderSurface::defaultWindowName.c_str(), (int)_render_surface_map.size() );
        rs->setWindowName( name );
    }
    _render_surface_map.insert(std::pair<std::string, osg::ref_ptr<RenderSurface> >( rs->getWindowName(), rs ));
}


void CameraConfig::beginCamera( std::string name )
{
    Camera *camera = new Camera;
    std::pair<std::map<std::string,  osg::ref_ptr<Camera> >::iterator,bool> res =
      _camera_map.insert(std::pair<std::string, osg::ref_ptr<Camera> >(name, camera));
    _current_camera = (res.first)->second.get();
    _can_add_camera_attributes = true;
}

void CameraConfig::setCameraRenderSurface( const char *name )
{
    RenderSurface *rs = findRenderSurface( name );
    if( rs == NULL )
    {
        std::cerr << "setCameraRenderSurface(): No Render Surface by name of \"" << name << "\" was found!\n";
        return;
    }

    if( rs != NULL && _current_camera != NULL )
        _current_camera->setRenderSurface( rs );

}

void CameraConfig::setCameraRenderSurface( void )
{

    if( _current_camera != NULL && _current_render_surface != NULL )
        _current_camera->setRenderSurface( _current_render_surface.get() );

}

void CameraConfig::setCameraProjectionRectangle( float x0, float x1, float y0, float y1 )
{
    if( _current_camera != NULL )
        _current_camera->setProjectionRectangle( x0, x1, y0, y1 );
}

void CameraConfig::setCameraProjectionRectangle( int x0, int x1, int y0, int y1 )
{
    if( _current_camera != NULL )
        _current_camera->setProjectionRectangle( x0, x1, y0, y1 );
}

void CameraConfig::setCameraOrtho( float left, float right, float bottom, float top, float nearClip, float farClip,
                        float xshear, float yshear )
{
    if( _current_camera != NULL )
        _current_camera->setLensOrtho( left, right, bottom, top, nearClip, farClip, xshear, yshear );
}

void CameraConfig::setCameraPerspective( float hfov, float vfov, float nearClip, float farClip,
                        float xshear, float yshear )
{
    if( _current_camera != 0 )
        _current_camera->setLensPerspective( hfov, vfov, nearClip, farClip, xshear, yshear );
}

void CameraConfig::setCameraFrustum( float left, float right, float bottom, float top, float nearClip, float farClip,
                        float xshear, float yshear )
{
    if( _current_camera != 0 )
        _current_camera->setLensFrustum( left, right, bottom, top, nearClip, farClip, xshear, yshear );
}

void CameraConfig::setCameraLensShear( osg::Matrix::value_type xshear, osg::Matrix::value_type yshear )
{
    if( _current_camera != NULL )
        _current_camera->setLensShear(xshear,yshear);
}

void CameraConfig::setCameraShareLens( bool shared )
{
    if( _current_camera != NULL )
        _current_camera->setShareLens( shared );
}

void CameraConfig::setCameraShareView( bool shared )
{
    if( _current_camera != NULL )
        _current_camera->setShareView( shared );
}

void CameraConfig::setCameraClearColor( float r, float g, float b, float a )
{
    if( _current_camera != NULL )
        _current_camera->setClearColor( r, g, b, a );
}

void CameraConfig::beginCameraOffset()
{
  osg::Matrix::value_type id[] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
        };
  memcpy( _offset_matrix, id, sizeof(osg::Matrix::value_type[16]));
    _offset_shearx = _offset_sheary = 0.0;
}

void CameraConfig::shearCameraOffset( osg::Matrix::value_type shearx, osg::Matrix::value_type sheary )
{
    _offset_shearx = shearx;
    _offset_sheary = sheary;
}

void CameraConfig::setCameraOffsetMultiplyMethod( Camera::Offset::MultiplyMethod method )
{
    if( _current_camera != NULL )
        _current_camera->setOffsetMultiplyMethod(method);
}


void CameraConfig::endCameraOffset()
{
    if( _current_camera != NULL )
        _current_camera->setOffset( _offset_matrix, _offset_shearx, _offset_sheary);
}

void  CameraConfig::endCamera( void )
{
    _can_add_camera_attributes = false;
}

Camera *CameraConfig::findCamera( const char *name )
{
    std::map<std::string,  osg::ref_ptr<Camera> >::iterator p;
    p = _camera_map.find( std::string(name) );
    if( p == _camera_map.end() )
        return NULL;
    else
        return (*p).second.get();
}

unsigned int CameraConfig::getNumberOfCameras() const
{
    return _camera_map.size();
}

const Camera *CameraConfig::getCamera( unsigned int n ) const
{
    if( n >= _camera_map.size() )
            return NULL;

    unsigned int i;
    std::map <std::string,  osg::ref_ptr<Camera> >::const_iterator p;
    for( i = 0, p = _camera_map.begin(); p != _camera_map.end(); p++ )
        if( i++ == n )
                break;
    if(  p == _camera_map.end() )
            return NULL;
    return p->second.get();
}

Camera *CameraConfig::getCamera( unsigned int n )
{
    if( n >= _camera_map.size() )
            return NULL;

    unsigned int i;
    std::map <std::string,  osg::ref_ptr<Camera> >::iterator p;
    for( i = 0, p = _camera_map.begin(); p != _camera_map.end(); p++ )
        if( i++ == n )
                break;
    if(  p == _camera_map.end() )
            return NULL;
    return p->second.get();
}

void CameraConfig::beginInputArea()
{
    _input_area = new InputArea;
    _can_add_input_area_entries = true;
}

void CameraConfig::addInputAreaEntry( char *renderSurfaceName )
{
    osgProducer::RenderSurface *rs = findRenderSurface( renderSurfaceName );
    if( rs == NULL )
    {
        std::cerr << "setInputAreaEntry(): No Render Surface by name of \"" << renderSurfaceName << "\" was found!\n";
        return;
    }
    if( _input_area != NULL && _can_add_input_area_entries == true )
        _input_area->addRenderSurface( rs );
}

void CameraConfig::endInputArea()
{
    _can_add_input_area_entries = false;
}

void CameraConfig::setInputArea(InputArea *ia)
{
    _input_area=ia;
}

InputArea *CameraConfig::getInputArea()
{
    return _input_area.get();
}

const InputArea *CameraConfig::getInputArea() const
{
    return _input_area.get();
}

#if 0
void CameraConfig::realize( void )
{
    std::map <std::string,  osg::ref_ptr<RenderSurface> >::iterator p;
    for( p = _render_surface_map.begin(); p != _render_surface_map.end(); p++ )
    {
        ((*p).second)->realize();
    }
}
#endif

std::string findFile( std::string );

void CameraConfig::addStereoSystemCommand( int screen, std::string stereoCmd, std::string monoCmd )
{
    _stereoSystemCommands.push_back(StereoSystemCommand( screen, stereoCmd, monoCmd ));
}

const std::vector<CameraConfig::StereoSystemCommand> &CameraConfig::getStereoSystemCommands()
{
    return _stereoSystemCommands;
}


CameraConfig::~CameraConfig()
{
}

//////////////////////

void CameraConfig::rotateCameraOffset( osg::Matrix::value_type deg, osg::Matrix::value_type x, osg::Matrix::value_type y, osg::Matrix::value_type z )
{
  osg::Matrix m;
  m.invert(osg::Matrix::rotate( osg::DegreesToRadians(deg), x,y,z));
  m = m * osg::Matrix(_offset_matrix);
    memcpy( _offset_matrix, m.ptr(), sizeof( osg::Matrix::value_type[16] ));
}

void CameraConfig::translateCameraOffset( osg::Matrix::value_type x, osg::Matrix::value_type y, osg::Matrix::value_type z )
{
  osg::Matrix m;
    m.invert(osg::Matrix::translate( x,y,z));
    m = m * osg::Matrix(_offset_matrix);
    memcpy( _offset_matrix, m.ptr(), sizeof( osg::Matrix::value_type[16] ));
}

void CameraConfig::scaleCameraOffset( osg::Matrix::value_type x, osg::Matrix::value_type y, osg::Matrix::value_type z )
{
  osg::Matrix m = osg::Matrix::scale( x,y,z) * osg::Matrix(_offset_matrix);
  memcpy( _offset_matrix, m.ptr(), sizeof( osg::Matrix::value_type[16] ));
}

// Order of precedence:
//
std::string CameraConfig::findFile( std::string filename )
{
    if (filename.empty()) return filename;

    std::string path;
    // Check env var
    char *ptr = getenv( "PRODUCER_CONFIG_FILE_PATH");
    if( ptr != NULL )
    {
         path = std::string(ptr) + '/' + filename;
        if( osgDB::fileExists(path))
            return path;
    }

    // Check standard location(s)
    //path.clear();
    path = std::string( "/usr/local/share/Producer/Config/") + filename;
    if( osgDB::fileExists(path) )
        return path;

    //path.clear();
    path = std::string( "/usr/share/Producer/Config/") + filename;
    if( osgDB::fileExists(path) )
        return path;

    // Check local directory
    if(osgDB::fileExists(filename))
        return filename;

    // Fail
    return std::string();
}

bool CameraConfig::defaultConfig()
{
    if( getNumberOfCameras() != 0 ) return false;

    char *env = getenv( "PRODUCER_CAMERA_CONFIG_FILE" );

    // Backwards compatibility
    if( env == NULL )
        env = getenv( "PRODUCER_CONFIG_FILE" );

    if( env != NULL )
    {
        std::string file = findFile(env);
        return parseFile( file.c_str() );
    }

    unsigned int numScreens =  getNumberOfScreens();
    if( numScreens == 0 )
        return false;

    float xshear = float(numScreens-1);
    float yshear = 0.0;
    float input_xMin = -1.0f;
    float input_yMin = -1.0f;
    float input_width = 2.0f/float(numScreens);
    float input_height = 2.0f;

    // use an InputArea if there is more than one screen.
    InputArea *ia = (numScreens>1) ? new InputArea : 0;
    setInputArea(ia);

    for( unsigned int i = 0; i < numScreens; i++ )
    {
      std::stringstream sstr; sstr<<"Screen"<<i;
      std::string name = sstr.str();
      std::pair<std::map<std::string, osg::ref_ptr<Camera> >::iterator,bool> res =
      _camera_map.insert(std::pair<std::string, osg::ref_ptr<Camera> >(name, new Camera));

      ((res.first)->second)->getRenderSurface()->setScreenNum( i );
      ((res.first)->second)->setLensShear( xshear, yshear );

       RenderSurface *rs = ((res.first)->second)->getRenderSurface();
       rs->setWindowName( name );
       if (ia)
       {
           rs->setInputRectangle( RenderSurface::InputRectangle(input_xMin, input_xMin+input_width, input_yMin, input_yMin+input_height) );
           input_xMin += input_width;
           ia->addRenderSurface(rs);
       }

       _render_surface_map.insert(std::pair<std::string,
            osg::ref_ptr<RenderSurface> >( rs->getWindowName(), rs ));

      xshear -= 2.0;
    }

    _threadModelDirective = CameraGroup::getDefaultThreadModel();

    return true;
}

