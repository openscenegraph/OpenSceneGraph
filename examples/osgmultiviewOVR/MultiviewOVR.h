#ifndef OSGVIEWER_MultiviewOVR
#define OSGVIEWER_MultiviewOVR 1

#include <osgViewer/View>

/** spherical display using 6 slave cameras rendering the 6 sides of a cube map, and 7th camera doing distortion correction to present on a spherical display.*/
class MultiviewOVR : public osgViewer::ViewConfig
{
    public:
        
        MultiviewOVR(unsigned int screenNum=0):
            _screenNum(screenNum) {}
            
        MultiviewOVR(const MultiviewOVR& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
            ViewConfig(rhs,copyop),
            _screenNum(rhs._screenNum) {}
        
        META_Object(osgViewer,MultiviewOVR);
        
        virtual void configure(osgViewer::View& view) const;
        
        void setScreenNum(unsigned int n) { _screenNum = n; }
        unsigned int getScreenNum() const { return _screenNum; }
        
    protected:
        
        osg::ref_ptr<osg::Node> createStereoMesh(const osg::Vec3& origin, const osg::Vec3& widthVector, const osg::Vec3& heightVector) const;

        unsigned int _screenNum;
};

#endif
