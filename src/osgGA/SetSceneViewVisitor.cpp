#include <osgGA/SetSceneViewVisitor>
#include <osgGA/CameraManipulator>
#include <osgGA/StateSetManipulator>

void osgGA::SetSceneViewVisitor::visit(osgGA::CameraManipulator& cm)
{
    cm.setNode(_sceneView->getSceneData());
    cm.setCamera(_sceneView->getCamera());
    cm.init(*getGUIEventAdapter(),*getGUIActionAdapter());
    cm.home(*getGUIEventAdapter(),*getGUIActionAdapter());
}

void osgGA::SetSceneViewVisitor::visit(osgGA::StateSetManipulator& ssm)
{
    ssm.setStateSet(_sceneView->getGlobalStateSet());
}
