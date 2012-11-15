/* -*-c++-*- Present3D - Copyright (C) 1999-2006 Robert Osfield
 *
 * This software is open source and may be redistributed and/or modified under
 * the terms of the GNU General Public License (GPL) version 2.0.
 * The full license is in LICENSE.txt file included with this distribution,.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * include LICENSE.txt for more details.
*/

#include <osgPresentation/PropertyManager>

using namespace osgPresentation;

void ImageSequenceUpdateCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    float x;
    if (_propertyManager->getProperty(_propertyName,x))
    {
        double xMin = -1.0;
        double xMax = 1.0;
        double position = ((double)x-xMin)/(xMax-xMin)*_imageSequence->getLength();
        
        _imageSequence->seek(position);
    }
    else
    {
        OSG_INFO<<"ImageSequenceUpdateCallback::operator() Could not find property : "<<_propertyName<<std::endl;
    }
    
    // note, callback is responsible for scenegraph traversal so
    // they must call traverse(node,nv) to ensure that the
    // scene graph subtree (and associated callbacks) are traversed.
    traverse(node,nv);
}


bool PropertyEventCallback::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
{

    bool mouseEvent =  (ea.getEventType()==osgGA::GUIEventAdapter::MOVE ||
                        ea.getEventType()==osgGA::GUIEventAdapter::DRAG ||
                        ea.getEventType()==osgGA::GUIEventAdapter::PUSH ||
                        ea.getEventType()==osgGA::GUIEventAdapter::RELEASE);
    if(mouseEvent)
    {    
        _propertyManager->setProperty("mouse.x",ea.getX());
        _propertyManager->setProperty("mouse.x_normalized",ea.getXnormalized());
        _propertyManager->setProperty("mouse.y",ea.getX());
        _propertyManager->setProperty("mouse.y_normalized",ea.getYnormalized());
    }
    
    return false;
}
