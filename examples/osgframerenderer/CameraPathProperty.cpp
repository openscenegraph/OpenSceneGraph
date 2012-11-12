#include "CameraPathProperty.h"

using namespace gsc;

void CameraPathProperty::update(osgViewer::View* view)
{
    osg::Camera* camera = view->getCamera();
    osg::FrameStamp* fs = view->getFrameStamp();

    if (_animationPath.valid())
    {
        osg::AnimationPath::ControlPoint cp;
        _animationPath->getInterpolatedControlPoint( fs->getSimulationTime(), cp );

        OSG_NOTICE<<"CameraPathProperty "<<fs->getFrameNumber()<<" "<<fs->getSimulationTime()<<std::endl;

        osg::Matrixd matrix;
        cp.getMatrix( matrix );
        camera->setViewMatrix( osg::Matrix::inverse(matrix) );
    }
}

void CameraPathProperty::loadAnimationPath()
{
    _animationPath = new osg::AnimationPath;;
    //_animationPath->setLoopMode(osg::AnimationPath::LOOP);

    osgDB::ifstream in(_filename.c_str());
    if (!in)
    {
        OSG_WARN << "CameraPathProperty: Cannot open animation path file \"" << _filename << "\".\n";
        return;
    }

    _animationPath->read(in);
}

bool CameraPathProperty::getTimeRange(double& startTime, double& endTime) const
{
    if (!_animationPath) return false;
    
    const osg::AnimationPath::TimeControlPointMap& tcpm = _animationPath->getTimeControlPointMap();
    if (tcpm.empty()) return false;

    startTime = tcpm.begin()->first;
    endTime = tcpm.rbegin()->first;
    
    return true;
}

void CameraPathProperty::resetTimeRange(double startTime, double endTime)
{
    if (!_animationPath) return;

    osg::AnimationPath::TimeControlPointMap& tcpm = _animationPath->getTimeControlPointMap();
    if (tcpm.empty()) return;

    osg::AnimationPath::TimeControlPointMap copy_tcpm = tcpm;

    double offset = tcpm.begin()->first;
    double originalLength = tcpm.rbegin()->first - tcpm.begin()->first ;
    double scale = originalLength>0.0 ? (endTime-startTime)/originalLength : 1.0;

    tcpm.clear();

    for(osg::AnimationPath::TimeControlPointMap::iterator itr = copy_tcpm.begin();
        itr != copy_tcpm.end();
        ++itr)
    {
        tcpm[startTime + (itr->first-offset)*scale] = itr->second;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Serialization support
//
REGISTER_OBJECT_WRAPPER( gsc_CameraPathProperty,
                         new gsc::CameraPathProperty,
                         gsc::CameraPathProperty,
                         "osg::Object gsc::CameraPathProperty" )
{
    ADD_STRING_SERIALIZER( AnimationPathFileName, "" );
}




