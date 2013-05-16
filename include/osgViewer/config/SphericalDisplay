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

#ifndef OSGVIEWER_SphericalDisplay
#define OSGVIEWER_SphericalDisplay 1

#include <osgViewer/View>

namespace osgViewer {

/** spherical display using 6 slave cameras rendering the 6 sides of a cube map, and 7th camera doing distortion correction to present on a spherical display.*/
class OSGVIEWER_EXPORT SphericalDisplay : public ViewConfig
{
    public:
        
        SphericalDisplay(double radius=1.0, double collar=0.45, unsigned int screenNum=0, osg::Image* intensityMap=0, const osg::Matrixd& projectorMatrix = osg::Matrixd()):
            _radius(radius),
            _collar(collar),
            _screenNum(screenNum),
            _intensityMap(intensityMap),
            _projectorMatrix(projectorMatrix) {}
            
        SphericalDisplay(const SphericalDisplay& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
            ViewConfig(rhs,copyop),
            _radius(rhs._radius),
            _collar(rhs._collar),
            _screenNum(rhs._screenNum),
            _intensityMap(rhs._intensityMap),
            _projectorMatrix(rhs._projectorMatrix) {}
        
        META_Object(osgViewer,SphericalDisplay);
        
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
        
        osg::Geometry* create3DSphericalDisplayDistortionMesh(const osg::Vec3& origin, const osg::Vec3& widthVector, const osg::Vec3& heightVector, double sphere_radius, double collar_radius,osg::Image* intensityMap, const osg::Matrix& projectorMatrix) const;

        double _radius;
        double _collar;
        unsigned int _screenNum;
        osg::ref_ptr<osg::Image> _intensityMap;
        osg::Matrixd _projectorMatrix;
};

#if 0
/** spherical display by rendering main scene to a panoramic 2:1 texture and then doing distortion correction to present onto a spherical display.*/
class OSGVIEWER_EXPORT ViewForPanoramicSphericalDisplay : public Config
{
    public:
        
        ViewForPanoramicSphericalDisplay(double radius=1.0, double collar=0.45, unsigned int screenNum=0, osg::Image* intensityMap=0, const osg::Matrixd& projectorMatrix = osg::Matrixd());
        ViewForPanoramicSphericalDisplay(const ViewForPanoramicSphericalDisplay& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
        
        META_Object(osgViewer,ViewOnSingleScreen);
        
        virtual void configure(osgViewer::View& view) const;
        
        void setRadius(double r) { _radius = r; }
        double getRadius() const { return _radius; }
        
        void setCollar(double r) { _collar = r; }
        double getCollar() const { return _collar; }
        
        void setScreenNum(unsigned int n) { _screenNum = n; }
        unsigned int getScreenNum() const { return _screenNum; }
        
        void setIntensityMap(osg::Image* im) { _intensityMap = im; }
        const osg::Image* getIntensityMap() const { return _intensityMap; }
        
        void setProjectionMatrix(const osg::Matrixd& m) { _projectorMatrix = m; }
        const osg::Matrixd& getProjectionMatrix() const { return _projectorMatrix; }      
        
    protected:
        
        double _radius;
        double _collar;
        unsigned int _screenNum;
        osg::ref_ref<osg::Image> _intensityMap;
        osg::Matrixd _projectorMatrix;
};

/** autostereoscopic Philips WoWvx display.*/
class OSGVIEWER_EXPORT ViewForWoWVxDisplay : public Config
{
    public:
        
        ViewForWoWVxDisplay();
        ViewForWoWVxDisplay(unsigned int screenNum, unsigned char wow_content, unsigned char wow_factor, unsigned char wow_offset, float wow_disparity_Zd, float wow_disparity_vz, float wow_disparity_M, float wow_disparity_C);
        ViewForWoWVxDisplay(const ViewForWoWVxDisplay& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
        
        META_Object(osgViewer,ViewForWoWVxDisplay);
        
        virtual void configure(osgViewer::View& view) const;
        
        void setScreenNum(unsigned int n) { _screenNum = n; }
        unsigned int getScreenNum() const { return _screenNum; }

        void set(unsigned char c) { _wow_content = c; }
        double get() const { return _wow_content; }

        void set(unsigned char c) { _wow_factor = c; }
        double get() const { return _wow_factor; }

        void set(unsigned char c) { _wow_offset = c; }
        double get() const { return _wow_offset; }

        void setWowDisparityZD(float c) { _wow_disparity_Zd = c; }
        float getWowDisparityZD() const { return _wow_disparity_Zd; }
        
        void setWowDisparityVZ(float c) { _wow_disparity_vz = c; }
        float getWowDisparityVZ() const { return _wow_disparity_vz; }
        
        void setWowDisparityM(float c) { _wow_disparity_M = c; }
        float getWowDisparityM() const { return _wow_disparity_M; }
        
        void setWowDisparityC(float c) { _wow_disparity_C = c; }
        float getWowDisparityC() const { return _wow_disparity_C; }
        
    protected:
    
        unsigned int _screenNum;
        unsigned char _wow_content;
        unsigned char _wow_factor;
        unsigned char _wow_offset;
        float _wow_disparity_Zd;
        float _wow_disparity_vz;
        float _wow_disparity_M;
        float _wow_disparity_C;
};
        
/** Configure view with DepthPartition.*/
class OSGVIEWER_EXPORT DepthPartition : public Config
{
    public:
        
        DepthPartition(DepthPartitionSettings* dsp=0);

        DepthPartition(const ViewForWoWVxDisplay& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
        
        META_Object(osgViewer,DepthPartition);

        void setDepthPartionSettings(DepthPartitionSettings* dsp) const { _dps = dps; }
        const DepthPartitionSettings* getDepthPartionSettings() const { return _dps; }

        /** for setting up depth partitioning on the specified camera.*/
        bool setUpDepthPartitionForCamera(osg::Camera* cameraToPartition, DepthPartitionSettings* dps=0);

        virtual void configure(osgViewer::View& view) const;
        
    protected:
};

#endif

}


#endif
