#include <osgGA/GUIEventHandlerVisitor>
#include <osgGA/GUIEventHandler>

void osgGA::GUIEventHandlerVisitor::visit(osgGA::CompositeGUIEventHandler& cgeh)
{
    for(int i=0; i<cgeh.getNumChildren(); i++){
        cgeh.getChild(i)->accept(*this);
    }
}
