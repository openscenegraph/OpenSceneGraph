#ifndef OSGVIEWER_StandardStereo
#define OSGVIEWER_StandardStereo 1

#include <osgViewer/View>

/** spherical display using 6 slave cameras rendering the 6 sides of a cube map, and 7th camera doing distortion correction to present on a spherical display.*/
class StandardStereo : public osgViewer::ViewConfig
{
    public:
        
        StandardStereo(unsigned int screenNum=0):
            _screenNum(screenNum) {}
            
        StandardStereo(const StandardStereo& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
            ViewConfig(rhs,copyop),
            _screenNum(rhs._screenNum) {}
        
        META_Object(osgViewer,StandardStereo);
        
        virtual void configure(osgViewer::View& view) const;
        
        void setScreenNum(unsigned int n) { _screenNum = n; }
        unsigned int getScreenNum() const { return _screenNum; }
        
    protected:
        
        osg::ref_ptr<osg::Node> createStereoMesh(const osg::Vec3& origin, const osg::Vec3& widthVector, const osg::Vec3& heightVector) const;

        unsigned int _screenNum;
};

#endif
