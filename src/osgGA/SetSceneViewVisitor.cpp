#include <osgGA/SetSceneViewVisitor>
#include <osgGA/StateSetManipulator>
#include <osgGA/MatrixManipulator>

void osgGA::SetSceneViewVisitor::visit(osgGA::MatrixManipulator& cm)
{
    cm.setNode(_sceneView->getSceneData());
    cm.setByInverseMatrix(_sceneView->getViewMatrix());
    cm.init(*getGUIEventAdapter(),*getGUIActionAdapter());
    cm.home(*getGUIEventAdapter(),*getGUIActionAdapter());
}

void osgGA::SetSceneViewVisitor::visit(osgGA::StateSetManipulator& ssm)
{
    ssm.setStateSet(_sceneView->getGlobalStateSet());
}
