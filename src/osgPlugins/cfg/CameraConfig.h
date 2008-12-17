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

#ifndef PRODUCER_CAMERA_CONFIG
#define PRODUCER_CAMERA_CONFIG

#include <stdio.h>

#include <osg/Referenced>
#include <osg/Matrix>
#include <osgViewer/View>

#include <string>
#include <map>
#include <vector>
#include <osg/Notify>
#include <osg/Vec2>
#include <iostream>

#include "Camera.h"
#include "RenderSurface.h"

//#undef SUPPORT_CPP

namespace osgProducer {

#define notImplemented {std::cout << __FILE__ << " " << __LINE__ << std::endl;}
  class CameraGroup : public osg::Referenced {
  public:
    enum ThreadModel {
      SingleThreaded,
      ThreadPerRenderSurface,
      ThreadPerCamera
    };
    static ThreadModel getDefaultThreadModel() { return SingleThreaded;}
  };


  class InputArea : public osg::Referenced {
  public:
    void addRenderSurface(RenderSurface* s) { _rs.push_back(s); }
    std::vector<osg::ref_ptr<RenderSurface> > _rs;
  };

#undef notImplemented


class CameraConfig : public osg::Referenced
{
    public :
        CameraConfig(); 

        void beginVisual( void );

        void beginVisual( const char * name );

        void setVisualSimpleConfiguration( void );

        void setVisualByID( unsigned int id );

        void addVisualAttribute( VisualChooser::AttributeName token, int param );

        void addVisualAttribute( VisualChooser::AttributeName token );

        void addVisualExtendedAttribute( unsigned int token );

        void addVisualExtendedAttribute( unsigned int token, int param );

        void endVisual( void );

        VisualChooser *findVisual( const char *name );

        bool parseFile( const std::string &file );

        void beginRenderSurface( const char *name );

        void setRenderSurfaceVisualChooser( const char *name );

        void setRenderSurfaceVisualChooser( void );

        void setRenderSurfaceWindowRectangle( int x, int y,  unsigned int width, unsigned int height );

        void setRenderSurfaceCustomFullScreenRectangle( int x, int y, unsigned int width, unsigned int height );

        void setRenderSurfaceOverrideRedirect( bool flag );

        void setRenderSurfaceHostName( const std::string &name );

        void setRenderSurfaceDisplayNum( int n );

        void setRenderSurfaceScreen( int n );

        void setRenderSurfaceBorder( bool flag );

        void setRenderSurfaceDrawableType( RenderSurface::DrawableType drawableType );

        void setRenderSurfaceRenderToTextureMode( RenderSurface::RenderToTextureMode rttMode );

        void setRenderSurfaceReadDrawable( const char *name );

        void setRenderSurfaceInputRectangle( float x0, float x1, float y0, float y1 );

        void endRenderSurface( void );

        RenderSurface *findRenderSurface( const char *name );

        unsigned int getNumberOfRenderSurfaces();

        RenderSurface *getRenderSurface( unsigned int index );

        void addCamera( std::string name, Camera *camera );

        void beginCamera( std::string name );

        void setCameraRenderSurface( const char *name );

        void setCameraRenderSurface( void );

        void setCameraProjectionRectangle( float x0, float x1, float y0, float y1 );

        void setCameraProjectionRectangle( int x0, int x1, int y0, int y1 );

        void setCameraOrtho( float left, float right, float bottom, float top, float nearClip, float farClip,
                                float xshear=0.0, float yshear=0.0 );

        void setCameraPerspective( float hfov, float vfov, float nearClip, float farClip,
                                float xshear=0.0, float yshear=0.0 );

        void setCameraFrustum( float left, float right, float bottom, float top, float nearClip, float farClip,
                                float xshear=0.0, float yshear=0.0 );

        void setCameraLensShear( osg::Matrix::value_type xshear, osg::Matrix::value_type yshear );
        
        void setCameraShareLens( bool shared );

        void setCameraShareView( bool shared );

        void setCameraClearColor( float r, float g, float b, float a );

        void beginCameraOffset();

        void rotateCameraOffset( osg::Matrix::value_type deg, osg::Matrix::value_type x, osg::Matrix::value_type y, osg::Matrix::value_type z );

        void translateCameraOffset( osg::Matrix::value_type x, osg::Matrix::value_type y, osg::Matrix::value_type z );

        void scaleCameraOffset( osg::Matrix::value_type x, osg::Matrix::value_type y, osg::Matrix::value_type z );

        void shearCameraOffset( osg::Matrix::value_type shearx, osg::Matrix::value_type sheary );

        void setCameraOffsetMultiplyMethod( Camera::Offset::MultiplyMethod method );

        void endCameraOffset();

        void  endCamera( void );

        Camera *findCamera( const char *name );

        unsigned int getNumberOfCameras() const;
        
        const Camera *getCamera( unsigned int n ) const;

        Camera *getCamera( unsigned int n );

        void beginInputArea();

        void addInputAreaEntry( char *renderSurfaceName );

        void endInputArea() ;

        void setInputArea(InputArea *ia);

        InputArea *getInputArea();

        const InputArea *getInputArea() const;

        void realize( void );

        bool defaultConfig();

        struct StereoSystemCommand
        {
            int _screen;
            std::string _setStereoCommand;
            std::string _restoreMonoCommand;

            StereoSystemCommand(int screen, std::string setStereoCommand, std::string restoreMonoCommand ):
                _screen(screen),
                _setStereoCommand(setStereoCommand),
                _restoreMonoCommand(restoreMonoCommand) {}
        };

        static std::string findFile( std::string );

        void addStereoSystemCommand( int screen, std::string stereoCmd, std::string monoCmd );

        const std::vector<StereoSystemCommand> &getStereoSystemCommands();

        void setThreadModelDirective( CameraGroup::ThreadModel directive ) { _threadModelDirective = directive; }
        CameraGroup::ThreadModel getThreadModelDirective() { return _threadModelDirective; }

    protected:

        virtual ~CameraConfig();

    private :

        std::map <std::string, VisualChooser *> _visual_map;
        osg::ref_ptr< VisualChooser >_current_visual_chooser;
        bool _can_add_visual_attributes;

        std::map <std::string,  osg::ref_ptr<RenderSurface > > _render_surface_map;
        osg::ref_ptr<RenderSurface> _current_render_surface;
        bool _can_add_render_surface_attributes;

        std::map <std::string,  osg::ref_ptr< Camera > > _camera_map;
        osg::ref_ptr<Camera> _current_camera;
        bool _can_add_camera_attributes;

        osg::ref_ptr< InputArea > _input_area;
        bool _can_add_input_area_entries;

        unsigned int getNumberOfScreens();

        osg::Matrix::value_type  _offset_matrix[16];
        osg::Matrix::value_type _offset_shearx, _offset_sheary;

        std::vector<StereoSystemCommand> _stereoSystemCommands;

        bool _postmultiply;

        CameraGroup::ThreadModel _threadModelDirective;
};

}


#endif
