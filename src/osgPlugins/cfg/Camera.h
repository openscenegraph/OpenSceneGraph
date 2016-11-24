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

#ifndef OSGPRODUCER_CAMERA
#define OSGPRODUCER_CAMERA

#include <osg/Referenced>
#include <osg/Matrix>
#include <osg/Vec3>

#include "RenderSurface.h"

#include <vector>

namespace osgProducer {

class CameraGroup;
class RenderSurface;

/**
    \class Camera
    \brief A Camera provides a programming interface for 3D
    graphics applications by means of an abstract camera analogy

    The Camera contains a Lens class and has a RenderSurface.  Methods
    are provided to give the programmer control over the OpenGL PROJECTION
    matrix throught the Lens and over the initial MODELVIEW matrix through
    the camera's position and attitude.

    The programmer must provide a class derived from Camera::SceneHandler
    to prepare and render the scene.  The Camera does not provide direct
    control over rendering itself.
*/



class Camera : public osg::Referenced
{

    public :
         /**
            \class SceneHandler
            \brief A Scene Handler handles the preparation and rendering
                   of a scene for Camera
         */

        /**
            \class Lens
            \brief A Lens provides control over the PROJECTION matrix.

            It is entirely contained within the Camera class.  A Lens may
            be of type Perspective or Orthographic and set with one of the
            setFrustum, setProjection() or setOrtho().  The Lens type is
            implied by the method used to set it */
        class Lens : public osg::Referenced
        {
            public :

                /** Projection types */
                enum Projection {
                    Perspective,
                    Orthographic,
                    Manual
                };

                Lens();

                /** setMatrix() exists to allow external projection-management tools
                    (like elumens' spiclops) to do their magic and still work with producer */
                void setMatrix( const osg::Matrix::value_type matrix[16] );

                /** Set the Projection type to be of Perspective and provide
                    the following parameters to set the Projection matrix.
                         hfov  - Horizontal Field of View in degrees
                         vfov  - Vertical Field of View in degrees
                         nearClip  - Distance from the viewer to the near plane of the
                                 viewing frustum.
                         farClip   - Distance from the viewer to the far plane of the
                                 viewing frustum.
                         xshear- Asymmetrical shear in viewing frustum in the horizontal
                                 direction.  Value given in normalized device coordinates
                                 (see setShear() below).
                         yshear- Asymmetrical shear in viewing frustum in the vertical
                                 direction.  Value given in normalized device coordinates
                                 (see setShear() below).
                 */
                void setPerspective( double hfov, double vfov,
                        double nearClip, double farClip );

                /** Set the Projection type to be of Perspective and provide
                    the dimensions of the left, right, bottom, top, nearClip and farClip
                    extents of the viewing frustum as indicated.
                         xshear- Asymmetrical shear in viewing frustum in the horizontal
                                 direction.  Value given in normalized device coordinates
                                 (see setShear() below).
                         yshear- Asymmetrical shear in viewing frustum in the vertical
                                 direction.  Value given in normalized device coordinates
                                 (see setShear() below).
                 */
                void setFrustum( double left, double right,
                                 double bottom, double top,
                                 double nearClip, double farClip );

                /** Set the Projection type to be of Orthographic and provide
                     the left, right, bottom dimensions of the 2D rectangle*/
                   void setOrtho( double left, double right,
                        double bottom, double top,
                        double nearClip, double farClip  );

                /** convertToOrtho() converts the current perspective view to an
                    orthographic view with dimensions that conserve the scale of the
                    objects at distance d.
                    convertToPerspective() converts the current orthographic view
                    to a perspective view with parameters that conserve the scale of
                    objects at distance d. */
                bool convertToOrtho( float d);
                bool convertToPerspective( float d);

                /** apply the lens.  This generates a projection matrix for OpenGL */
                void apply(float xshear=0.0f, float yshear=0.0);
                void generateMatrix( float xshear, float yshear, osg::Matrix::value_type matrix[16] );
                Projection getProjectionType() const { return _projection; }
                void getParams( double &left, double &right, double &bottom, double &top,
                                double &nearClip, double &farClip ); //, double &xshear, double &yshear );

                float getHorizontalFov() const { return osg::RadiansToDegrees(_hfov); }
                float getVerticalFov() const { return osg::RadiansToDegrees(_vfov); }
                void setAutoAspect(bool ar) { _auto_aspect = ar; }
                bool getAutoAspect() const { return _auto_aspect; }
                void setAspectRatio( double aspectRatio );
                double getAspectRatio() { return _aspect_ratio; }


            protected:
                ~Lens(){}

                /* Internal convenience methods */
                bool getFrustum( double& left, double& right,
                                       double& bottom, double& top,
                                       double& zNear, double& zFar ) const;
                bool getOrtho( double& left, double& right,
                                       double& bottom, double& top,
                                       double& zNear, double& zFar ) const;


            private :

                double _ortho_left, _ortho_right, _ortho_bottom, _ortho_top;
                double _left, _right, _bottom, _top, _nearClip, _farClip;
                Projection _projection;
                double _aspect_ratio;
                bool _auto_aspect;
                float _hfov, _vfov;
                osg::Matrix::value_type _matrix[16];

            private :
                void _updateFOV( void );
        };

        struct Offset {
            enum MultiplyMethod {
                PreMultiply,
                PostMultiply
            };
            double _xshear;
            double _yshear;
            osg::Matrix::value_type _matrix[16];
            MultiplyMethod _multiplyMethod;
            Offset():
                _xshear(0.0),
                _yshear(0.0),
                _multiplyMethod(PreMultiply) {}
        };


    public :
        Camera( void );

        void setRenderSurface( RenderSurface *rs ) { _rs = rs; }
        RenderSurface *getRenderSurface() { return _rs.get(); }
        const RenderSurface *getRenderSurface() const { return _rs.get(); }

        void setRenderSurfaceWindowRectangle( int x, int y, unsigned int width, unsigned int height, bool resize=true )
        { _rs->setWindowRectangle(x,y,width,height, resize); }

        void  setLens( Lens *lens )
        {
            if( _lens.get() != lens )
                _lens = lens;
        }

        Lens *getLens() { return _lens.get(); }
        const Lens *getLens() const { return _lens.get(); }

        //////////////////////////////////////////////////////////////////////////////////////
        /** Convenience method for setting the Lens Perspective.
            See Camera::Lens::setPerspective(). */
        void setLensPerspective( double hfov, double vfov,
                        double nearClip, double farClip,
                        double xshear=0, double yshear=0 )
        {
            _offset._xshear = xshear;
            _offset._yshear = yshear;
            _lens->setPerspective(hfov,vfov,nearClip,farClip);
        }

        /** Convenience method for setting the Lens Frustum.
            See Camera::Lens::setFrustum(). */
        void setLensFrustum( double left, double right,
                                 double bottom, double top,
                                 double nearClip, double farClip,
                                 double xshear=0, double yshear=0 )
        {
            _offset._xshear = xshear;
            _offset._yshear = yshear;
            _lens->setFrustum(left,right,bottom,top,nearClip, farClip);
        }

        /** Convenience method for setting the lens Orthographic projection.
            See Camera::Lens::setOrtho() */
        void setLensOrtho( double left, double right,
                        double bottom, double top,
                        double nearClip, double farClip ,
                        double xshear=0, double yshear=0 )
        {
            _offset._xshear = xshear;
            _offset._yshear = yshear;
            _lens->setOrtho( left, right, bottom, top, nearClip, farClip);
        }

        /** Convenience method for setting the lens shear. See Camera::Lens::setShear()*/
        void setLensShear( double xshear, double yshear )
        {
            _offset._xshear = xshear;
            _offset._yshear = yshear;
        }

        /** Convenience method for getting the lens shear. See Camera::Lens::getShear() */
        void getLensShear( double &xshear, double &yshear )
        {
            xshear = _offset._xshear;
            yshear = _offset._yshear;
        }

        /** Convenience method for converting the Perpective lens to an
            Orthographic lens. see Camera::lens:convertToOrtho() */
        bool convertLensToOrtho( float d) { return _lens->convertToOrtho(d); }

        /** Convenience method for converting the Orthographic lens to an
            Perspective lens. see Camera::lens:convertToPerspective() */
        bool convertLensToPerspective( float d) { return _lens->convertToPerspective(d); }

        /** Convenience method for getting the lens projection type.
            See Camera::Lens::setAspectRatio() */
        Lens::Projection getLensProjectionType() { return _lens->getProjectionType(); }

        /** Convenience method for applying the lens.  See Camera::Lens::apply() */
        void applyLens() { _lens->apply(_offset._xshear, _offset._yshear); }

        /** Convenience method for getting the Lens parameters.
            See Camera::Lens::apply() */
        void getLensParams( double &left, double &right, double &bottom, double &top,
                            double &nearClip, double &farClip, double &xshear, double &yshear )
        {
            _lens->getParams(left,right,bottom,top,nearClip,farClip );
            xshear = _offset._xshear;
            yshear = _offset._yshear;
        }

        /** Convenience method for getting the Lens Horizontal field of view.
            See Camera::Lens::getHorizontalFov() */
        float getLensHorizontalFov() { return _lens->getHorizontalFov(); }

        /** Convenience method for getting the Lens Horizontal field of view.
            See Camera::Lens::getVerticalFov() */
        float getLensVerticalFov() { return _lens->getVerticalFov(); }

        /** Convenience method for setting the Lens ProjectionMatrix.
            See Camera::Lens::setMatrix() */
        // DEPRECATE
        //void setLensMatrix( float mat[16] ) { _lens->setMatrix(mat); }

        /** Convenience method for getting the Lens ProjectionMatrix.
            See Camera::Lens::getMatrix() */
        void getLensMatrix(osg::Matrix::value_type matrix[16] )
        {
            _lens->generateMatrix(_offset._xshear, _offset._yshear, matrix );
        }

        /** Convenience method for setting AutoAspect on the lens.
            See Camera::Lens::setAutoAspect() */
        void setLensAutoAspect(bool ar) { _lens->setAutoAspect(ar); }

        /** Convenience method for getting AutoAspect on the lens.
            See Camera::Lens::getAutoAspect() */
        bool getLensAutoAspect() { return _lens->getAutoAspect(); }

        /** Convenience method for setting the lens Aspect Ratio.
            See Camera::Lens::setAspectRatio() */
        void setLensAspectRatio( double aspectRatio ) { _lens->setAspectRatio(aspectRatio); }
        double getLensAspectRatio() { return _lens->getAspectRatio(); }

        //////////////////////////////////////////////////////////////////////////////////////

        void setProjectionRectangle( const float left, const float right,
                                const float bottom, const float top );

        void getProjectionRectangle( float &left, float &right,
                                float &bottom, float &top ) const;

        void setProjectionRectangle( int x, int y, unsigned int width, unsigned int height );
        void getProjectionRectangle( int &x, int &y, unsigned int &width, unsigned int &height ) const ;

        osg::Matrix::value_type *getProjectionMatrix ()
        {
            _lens->generateMatrix(_offset._xshear, _offset._yshear, _projectionMatrix );
            return _projectionMatrix;
        }

        void setViewByLookat( float eyex, float eyey, float eyez,
                      float centerx, float centery, float centerz,
                      float upx, float upy, float upz );
        void setViewByLookat( const osg::Vec3 &eye, const osg::Vec3 &center, const osg::Vec3 &up );
        void setViewByMatrix( const osg::Matrix &mat );
        void setFocalDistance( double focal_distance ) { _focal_distance = focal_distance; }
        const osg::Matrix::value_type *getViewMatrix( void ) const;
        const osg::Matrix::value_type *getPositionAndAttitudeMatrix( void ) const { return _viewMatrix; }
        void applyView();

        void setOffset( const osg::Matrix::value_type matrix[16],
                        osg::Matrix::value_type _xshear=0.0,
                        osg::Matrix::value_type _yshear=0.0);
        void setOffset( double _xshear, double _yshear);
        void setOffsetMultiplyMethod( Offset::MultiplyMethod method )
        {
            _offset._multiplyMethod = method;
        }


        void setClearColor( float red, float green, float blue, float alpha);
        void getClearColor( float& red, float& green, float& blue, float& alpha);

        void clear( void );

        void setIndex( unsigned int index ) { _index = index; }
        unsigned int getIndex() const { return _index; }

        void setShareLens( bool flag ) { _shareLens = flag; }
        bool getShareLens() { return _shareLens; }
        void setShareView( bool flag ) { _shareView = flag; }
        bool getShareView() { return _shareView; }

    protected :

        virtual ~Camera( void );
        osg::ref_ptr<Lens>                     _lens;
        osg::ref_ptr<RenderSurface>  _rs;
        unsigned int                                _index;

    private :
        bool _initialized;
        bool _initialize(void);

        bool _enabled;

        float _projrectLeft,
              _projrectRight,
              _projrectBottom,
              _projrectTop;

        Offset _offset;
        osg::Matrix::value_type _projectionMatrix[16];
        osg::Matrix::value_type _viewMatrix[16];
        float  _clear_color[4];
        double _focal_distance;


        friend class CameraGroup;

        bool _shareLens;
        bool _shareView;
};

}

#endif

