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
*/

#ifndef OSGVIEWER_PanoramicSphericalDisplay
#define OSGVIEWER_PanoramicSphericalDisplay 1

#include <osgViewer/View>

namespace osgViewer {

/** spherical display by rendering main scene to a panoramic 2:1 texture and then doing distortion correction to present onto a spherical display.*/
class OSGVIEWER_EXPORT PanoramicSphericalDisplay : public ViewConfig
{
    public:
        
        PanoramicSphericalDisplay(double radius=1.0, double collar=0.45, unsigned int screenNum=0, osg::Image* intensityMap=0, const osg::Matrixd& projectorMatrix = osg::Matrixd()):
            _radius(radius),
            _collar(collar),
            _screenNum(screenNum),
            _intensityMap(intensityMap),
            _projectorMatrix(projectorMatrix) {}
            
        PanoramicSphericalDisplay(const PanoramicSphericalDisplay& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
            ViewConfig(rhs, copyop),
            _radius(rhs._radius),
            _collar(rhs._collar),
            _screenNum(rhs._screenNum),
            _intensityMap(rhs._intensityMap),
            _projectorMatrix(rhs._projectorMatrix) {}
            
        
        META_Object(osgViewer,PanoramicSphericalDisplay);
        
        virtual void configure(osgViewer::View& view) const;
        
        void setRadius(double r) { _radius = r; }
        double getRadius() const { return _radius; }
        
        void setCollar(double r) { _collar = r; }
        double getCollar() const { return _collar; }
        
        void setScreenNum(unsigned int n) { _screenNum = n; }
        unsigned int getScreenNum() const { return _screenNum; }
        
        void setIntensityMap(osg::Image* im) { _intensityMap = im; }
        const osg::Image* getIntensityMap() const { return _intensityMap.get(); }
        
        void setProjectionMatrix(const osg::Matrixd& m) { _projectorMatrix = m; }
        const osg::Matrixd& getProjectionMatrix() const { return _projectorMatrix; }      
        
    protected:
        
        osg::Geometry* createParoramicSphericalDisplayDistortionMesh(const osg::Vec3& origin, const osg::Vec3& widthVector, const osg::Vec3& heightVector, double sphere_radius, double collar_radius, osg::Image* intensityMap, const osg::Matrix& projectorMatrix) const;

        double _radius;
        double _collar;
        unsigned int _screenNum;
        osg::ref_ptr<osg::Image> _intensityMap;
        osg::Matrixd _projectorMatrix;
};

}


#endif
