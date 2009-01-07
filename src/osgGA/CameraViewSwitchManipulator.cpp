#include <osgGA/CameraViewSwitchManipulator>
#include <osg/Quat>
#include <osg/Notify>
#include <osg/BoundsChecking>


using namespace osg;
using namespace osgGA;

class CollectCameraViewsNodeVisitor : public osg::NodeVisitor
{
public:
    CollectCameraViewsNodeVisitor(CameraViewSwitchManipulator::CameraViewList* cameraViews):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _cameraViews(cameraViews)
    {}
    
    virtual void apply(CameraView& node)
    {
        _cameraViews->push_back(&node);
    }
    
    CameraViewSwitchManipulator::CameraViewList* _cameraViews;
};

void CameraViewSwitchManipulator::setNode(osg::Node* node)
{
    _node = node;

    _cameraViews.clear();
    CollectCameraViewsNodeVisitor visitor(&_cameraViews);

    _node->accept(visitor);
}

void CameraViewSwitchManipulator::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("CameraViewSwitcher: [","Decrease current camera number");
    usage.addKeyboardMouseBinding("CameraViewSwitcher: ]","Increase current camera number");
}

bool CameraViewSwitchManipulator::handle(const GUIEventAdapter& ea,GUIActionAdapter&)
{
    if (ea.getHandled()) return false;

    switch(ea.getEventType())
    {
 
        case(GUIEventAdapter::KEYDOWN):
            if (ea.getKey()=='[')
            {
                if (_currentView == 0)
                    _currentView = _cameraViews.size()-1;
                else
                    _currentView--;
                return true;
            }
            else if (ea.getKey()==']')
            {
                _currentView++;
                if (_currentView >= _cameraViews.size())
                    _currentView = 0;
                return true;
            }
            return false;

        default:
            return false;
    }
}

osg::Matrixd CameraViewSwitchManipulator::getMatrix() const
{
    osg::Matrix mat;
    if (_currentView < _cameraViews.size())
    {
        NodePathList parentNodePaths = _cameraViews[_currentView]->getParentalNodePaths();

        if (!parentNodePaths.empty())
        {
            mat = osg::computeLocalToWorld(parentNodePaths[0]);
            // TODO take into account the position and attitude of the CameraView
        }
        else
        {
            osg::notify(osg::NOTICE)<<"CameraViewSwitchManipulator::getMatrix(): Unable to calculate matrix due to empty parental path."<<std::endl;
        }
    }
    return mat;    
}

osg::Matrixd CameraViewSwitchManipulator::getInverseMatrix() const
{
    osg::Matrix mat;
    if (_currentView < _cameraViews.size())
    {
        NodePathList parentNodePaths = _cameraViews[_currentView]->getParentalNodePaths();

        if (!parentNodePaths.empty())
        {
            mat = osg::computeWorldToLocal(parentNodePaths[0]);
            // TODO take into account the position and attitude of the CameraView
        }
        else
        {
            osg::notify(osg::NOTICE)<<"CameraViewSwitchManipulator::getInverseMatrix(): Unable to calculate matrix due to empty parental path."<<std::endl;
        }
    }
    return mat;
}
