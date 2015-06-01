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
//osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/Dragger>
#include <osgManipulator/Command>
#include <osg/Material>
#include <osgGA/EventVisitor>
#include <osgViewer/View>
#include <osg/io_utils>
using namespace osgManipulator;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// computeNodePathToRoot
//
void osgManipulator::computeNodePathToRoot(osg::Node& node, osg::NodePath& np)
{
    np.clear();

    osg::NodePathList nodePaths = node.getParentalNodePaths();

    if (!nodePaths.empty())
    {
        np = nodePaths.front();
        if (nodePaths.size()>1)
        {
            OSG_NOTICE<<"osgManipulator::computeNodePathToRoot(,) taking first parent path, ignoring others."<<std::endl;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DraggerTransformCallback
//
DraggerTransformCallback::DraggerTransformCallback(osg::MatrixTransform* transform,int handleCommandMask):
    _handleCommandMask(handleCommandMask),
    _transform(transform)
{
}

bool DraggerTransformCallback::receive(const MotionCommand& command)
{
    if (!_transform) return false;

    switch (command.getStage())
    {
        case MotionCommand::START:
        {
            // Save the current matrix
            _startMotionMatrix = _transform->getMatrix();

            // Get the LocalToWorld and WorldToLocal matrix for this node.
            osg::NodePath nodePathToRoot;
            computeNodePathToRoot(*_transform,nodePathToRoot);
            _localToWorld = osg::computeLocalToWorld(nodePathToRoot);
            _worldToLocal = osg::Matrix::inverse(_localToWorld);

            return true;
        }
        case MotionCommand::MOVE:
        {
            //OSG_NOTICE<<"MotionCommand::MOVE "<<command.getMotionMatrix()<<std::endl;

            // Transform the command's motion matrix into local motion matrix.
            osg::Matrix localMotionMatrix = _localToWorld * command.getWorldToLocal()
                                            * command.getMotionMatrix()
                                            * command.getLocalToWorld() * _worldToLocal;

            // Transform by the localMotionMatrix
            _transform->setMatrix(localMotionMatrix * _startMotionMatrix);

            return true;
        }
        case MotionCommand::FINISH:
        {
            return true;
        }
        case MotionCommand::NONE:
        default:
            return false;
    }
}

bool DraggerTransformCallback::receive(const TranslateInLineCommand& command)
{
    if ((_handleCommandMask&HANDLE_TRANSLATE_IN_LINE)!=0) return receive(static_cast<const MotionCommand&>(command));
    return false;
}

bool DraggerTransformCallback::receive(const TranslateInPlaneCommand& command)
{
    if ((_handleCommandMask&HANDLE_TRANSLATE_IN_PLANE)!=0) return receive(static_cast<const MotionCommand&>(command));
    return false;
}

bool DraggerTransformCallback::receive(const Scale1DCommand& command)
{
    if ((_handleCommandMask&HANDLE_SCALED_1D)!=0) return receive(static_cast<const MotionCommand&>(command));
    return false;
}

bool DraggerTransformCallback::receive(const Scale2DCommand& command)
{
    if ((_handleCommandMask&HANDLE_SCALED_2D)!=0) return receive(static_cast<const MotionCommand&>(command));
    return false;
}

bool DraggerTransformCallback::receive(const ScaleUniformCommand& command)
{
    if ((_handleCommandMask&HANDLE_SCALED_UNIFORM)!=0) return receive(static_cast<const MotionCommand&>(command));
    return false;
}

bool DraggerTransformCallback::receive(const Rotate3DCommand& command)
{
    if ((_handleCommandMask&HANDLE_ROTATE_3D)!=0) return receive(static_cast<const MotionCommand&>(command));
    return false;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// PointerInfo
//
PointerInfo::PointerInfo():
    _nearPoint(osg::Vec3d()),
    _farPoint(osg::Vec3d()),
    _eyeDir(osg::Vec3d(0,0,1))
{
    _hitIter = _hitList.begin();
}

bool PointerInfo::contains(const osg::Node* node) const
{
    if (node && _hitIter!=_hitList.end()) return std::find((*_hitIter).first.begin(), (*_hitIter).first.end(), node) != (*_hitIter).first.end();
    else return false;
}

bool PointerInfo::projectWindowXYIntoObject(const osg::Vec2d& windowCoord, osg::Vec3d& nearPoint, osg::Vec3d& farPoint) const
{
    nearPoint = osg::Vec3d(windowCoord.x(),windowCoord.y(),0.0)*_inverseMVPW;
    farPoint = osg::Vec3d(windowCoord.x(),windowCoord.y(),1.0)*_inverseMVPW;

    return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Dragger
//
Dragger::Dragger() :
    _handleEvents(false),
    _draggerActive(false),
    _activationModKeyMask(0),
    _activationMouseButtonMask(0),
    _activationKeyEvent(0),
    _activationPermittedByModKeyMask(false),
    _activationPermittedByMouseButtonMask(false),
    _activationPermittedByKeyEvent(false),
    _intersectionMask(0xffffffff)
{
    _parentDragger = this;
    getOrCreateStateSet()->setDataVariance(osg::Object::DYNAMIC);

    _selfUpdater = new DraggerTransformCallback(this);

}

Dragger::Dragger(const Dragger& rhs, const osg::CopyOp& copyop):
    osg::MatrixTransform(rhs, copyop),
    _handleEvents(rhs._handleEvents),
    _draggerActive(false),
    _activationModKeyMask(rhs._activationModKeyMask),
    _activationMouseButtonMask(rhs._activationMouseButtonMask),
    _activationKeyEvent(rhs._activationKeyEvent),
    _activationPermittedByModKeyMask(false),
    _activationPermittedByMouseButtonMask(false),
    _activationPermittedByKeyEvent(false),
    _intersectionMask(0xffffffff)
{
}

Dragger::~Dragger()
{
}

void Dragger::setHandleEvents(bool flag)
{
    if (_handleEvents == flag) return;

    _handleEvents = flag;

    // update the number of children that require an event traversal to make sure this dragger receives events.
    if (_handleEvents) setNumChildrenRequiringEventTraversal(getNumChildrenRequiringEventTraversal()+1);
    else if (getNumChildrenRequiringEventTraversal()>=1) setNumChildrenRequiringEventTraversal(getNumChildrenRequiringEventTraversal()-1);
}

void Dragger::addConstraint(Constraint* constraint)
{
    // check to make sure constaint hasn't already been attached.
    for(Constraints::iterator itr = _constraints.begin();
        itr != _constraints.end();
        ++itr)
    {
        if (*itr == constraint) return;
    }

    _constraints.push_back(constraint);
}

void Dragger::removeConstraint(Constraint* constraint)
{
    for(Constraints::iterator itr = _constraints.begin();
        itr != _constraints.end();
        )
    {
        if (*itr == constraint)
        {
            _constraints.erase(itr);
            return;
        } else
        {
            ++itr;
        }
    }
}

void Dragger::addTransformUpdating(osg::MatrixTransform* transform, int handleCommandMask)
{
    addDraggerCallback(new DraggerTransformCallback(transform, handleCommandMask));
}

void Dragger::removeTransformUpdating(osg::MatrixTransform* transform)
{
    for(Dragger::DraggerCallbacks::iterator itr = _draggerCallbacks.begin();
        itr != _draggerCallbacks.end();
        )
    {
        DraggerCallback* dc = itr->get();
        DraggerTransformCallback* dtc = dynamic_cast<DraggerTransformCallback*>(dc);
        if (dtc && dtc->getTransform()==transform)
        {
            itr = _draggerCallbacks.erase(itr);
        }
        else
        {
            ++itr;
        }
    }

}

void Dragger::addDraggerCallback(DraggerCallback* dc)
{
    for(DraggerCallbacks::iterator itr = _draggerCallbacks.begin();
        itr != _draggerCallbacks.end();
        ++itr)
    {
        if (*itr == dc) return;
    }

    _draggerCallbacks.push_back(dc);
}

void Dragger::removeDraggerCallback(DraggerCallback* dc)
{
    for(Dragger::DraggerCallbacks::iterator itr = _draggerCallbacks.begin();
        itr != _draggerCallbacks.end();
        )
    {
        if (dc==itr->get())
        {
            itr = _draggerCallbacks.erase(itr);
        }
        else
        {
            ++itr;
        }
    }
}


void Dragger::traverse(osg::NodeVisitor& nv)
{
    if (_handleEvents && nv.getVisitorType()==osg::NodeVisitor::EVENT_VISITOR)
    {
        osgGA::EventVisitor* ev = dynamic_cast<osgGA::EventVisitor*>(&nv);
        if (ev)
        {
            for(osgGA::EventQueue::Events::iterator itr = ev->getEvents().begin();
                itr != ev->getEvents().end();
                ++itr)
            {
                osgGA::GUIEventAdapter* ea = (*itr)->asGUIEventAdapter();
                if (ea && handle(*ea, *(ev->getActionAdapter()))) ea->setHandled(true);
            }
        }
        return;
    }

    MatrixTransform::traverse(nv);
}

bool Dragger::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    if (ea.getHandled()) return false;

    osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
    if (!view) return false;

    bool handled = false;

    bool activationPermitted = true;
    if (_activationModKeyMask!=0 || _activationMouseButtonMask!=0 || _activationKeyEvent!=0)
    {
        _activationPermittedByModKeyMask = (_activationModKeyMask!=0) ?
            ((ea.getModKeyMask() & _activationModKeyMask)!=0) :
            false;

        _activationPermittedByMouseButtonMask = (_activationMouseButtonMask!=0) ?
            ((ea.getButtonMask() & _activationMouseButtonMask)!=0) :
            false;

        if (_activationKeyEvent!=0)
        {
            switch (ea.getEventType())
            {
                case osgGA::GUIEventAdapter::KEYDOWN:
                {
                    if (ea.getKey()==_activationKeyEvent) _activationPermittedByKeyEvent = true;
                    break;
                }
                case osgGA::GUIEventAdapter::KEYUP:
                {
                    if (ea.getKey()==_activationKeyEvent) _activationPermittedByKeyEvent = false;
                    break;
                }
                default:
                    break;
            }
        }

        activationPermitted =  _activationPermittedByModKeyMask || _activationPermittedByMouseButtonMask || _activationPermittedByKeyEvent;

    }

    if (activationPermitted || _draggerActive)
    {
        switch (ea.getEventType())
        {
            case osgGA::GUIEventAdapter::PUSH:
            {
                osgUtil::LineSegmentIntersector::Intersections intersections;

                _pointer.reset();

                if (view->computeIntersections(ea ,intersections, _intersectionMask))
                {
                    for(osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
                        hitr != intersections.end();
                        ++hitr)
                    {
                        _pointer.addIntersection(hitr->nodePath, hitr->getLocalIntersectPoint());
                    }
                    for (osg::NodePath::iterator itr = _pointer._hitList.front().first.begin();
                            itr != _pointer._hitList.front().first.end();
                            ++itr)
                    {
                        osgManipulator::Dragger* dragger = dynamic_cast<osgManipulator::Dragger*>(*itr);
                        if (dragger)
                        {
                            if (dragger==this)
                            {
                                osg::Camera *rootCamera = view->getCamera();
                                osg::NodePath nodePath = _pointer._hitList.front().first;
                                osg::NodePath::reverse_iterator ritr;
                                for(ritr = nodePath.rbegin();
                                    ritr != nodePath.rend();
                                    ++ritr)
                                {
                                    osg::Camera* camera = dynamic_cast<osg::Camera*>(*ritr);
                                    if (camera && (camera->getReferenceFrame()!=osg::Transform::RELATIVE_RF || camera->getParents().empty()))
                                    {
                                         rootCamera = camera;
                                         break;
                                    }
                                }

                                _pointer.setCamera(rootCamera);
                                _pointer.setMousePosition(ea.getX(), ea.getY());

                                if(dragger->handle(_pointer, ea, aa))
                                {
                                    dragger->setDraggerActive(true);
                                    handled = true;
                                }
                            }
                        }
                    }
                }
                break;
            }
            case osgGA::GUIEventAdapter::DRAG:
            case osgGA::GUIEventAdapter::RELEASE:
            {
                if (_draggerActive)
                {
                    _pointer._hitIter = _pointer._hitList.begin();
//                    _pointer.setCamera(view->getCamera());
                    _pointer.setMousePosition(ea.getX(), ea.getY());

                    if(handle(_pointer, ea, aa))
                    {
                        handled = true;
                    }
                }
                break;
            }
            default:
                break;
        }

        if (_draggerActive && ea.getEventType() == osgGA::GUIEventAdapter::RELEASE)
        {
            setDraggerActive(false);
            _pointer.reset();
        }
    }

    return handled;
}

bool Dragger::receive(const MotionCommand& command)
{
    if (_selfUpdater.valid()) return _selfUpdater->receive(command);
    else return false;
}

void Dragger::dispatch(MotionCommand& command)
{
    // apply any constraints
    for(Constraints::iterator itr = _constraints.begin();
        itr != _constraints.end();
        ++itr)
    {
        command.accept(*(*itr));
    }

    // apply any constraints of parent dragger.
    if (getParentDragger()!=this)
    {
        for(Constraints::iterator itr = getParentDragger()->getConstraints().begin();
            itr != getParentDragger()->getConstraints().end();
            ++itr)
        {
            command.accept(*(*itr));
        }
    }

    // move self
    getParentDragger()->receive(command);

    // pass on movement to any dragger callbacks
    for(DraggerCallbacks::iterator itr = getParentDragger()->getDraggerCallbacks().begin();
        itr != getParentDragger()->getDraggerCallbacks().end();
        ++itr)
    {
        command.accept(*(*itr));
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CompositeDragger
//
CompositeDragger::CompositeDragger(const CompositeDragger& rhs, const osg::CopyOp& copyop):
    Dragger(rhs, copyop)
{
    OSG_NOTICE<<"CompositeDragger::CompositeDragger(const CompositeDragger& rhs, const osg::CopyOp& copyop) not Implemented yet."<<std::endl;
}

bool CompositeDragger::handle(const PointerInfo& pi, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    // Check if the dragger node is in the nodepath.
    if (!pi.contains(this))
        return false;

    for (DraggerList::iterator itr=_draggerList.begin(); itr!=_draggerList.end(); ++itr)
    {
        if ((*itr)->handle(pi, ea, aa))
            return true;
    }
    return false;
}
bool CompositeDragger::containsDragger( const Dragger* dragger ) const
{
    for (DraggerList::const_iterator itr = _draggerList.begin(); itr != _draggerList.end(); ++itr)
    {
        if (itr->get() == dragger) return true;
    }
    return false;
}

CompositeDragger::DraggerList::iterator CompositeDragger::findDragger( const Dragger* dragger )
{
    for (DraggerList::iterator itr = _draggerList.begin(); itr != _draggerList.end(); ++itr)
    {
        if (itr->get() == dragger) return itr;
    }
    return _draggerList.end();
}

bool CompositeDragger::addDragger(Dragger *dragger)
{
    if (dragger && !containsDragger(dragger))
    {
        _draggerList.push_back(dragger);
        return true;
    }
    else return false;

}

bool CompositeDragger::removeDragger(Dragger *dragger)
{
    DraggerList::iterator itr = findDragger(dragger);
    if (itr != _draggerList.end())
    {
        _draggerList.erase(itr);
        return true;
    }
    else return false;

}

void CompositeDragger::setParentDragger(Dragger* dragger)
{
    for (DraggerList::iterator itr = _draggerList.begin(); itr != _draggerList.end(); ++itr)
    {
        (*itr)->setParentDragger(dragger);
    }
    Dragger::setParentDragger(dragger);
}

void CompositeDragger::setIntersectionMask(osg::Node::NodeMask intersectionMask)
{
    Dragger::setIntersectionMask(intersectionMask);
    for (DraggerList::iterator itr = _draggerList.begin(); itr != _draggerList.end(); ++itr)
    {
        (*itr)->setIntersectionMask(intersectionMask);
    }
}

class ForceCullCallback : public osg::Drawable::CullCallback
{
    public:
        virtual bool cull(osg::NodeVisitor*, osg::Drawable*, osg::State*) const
        {
            return true;
        }
};

void osgManipulator::setDrawableToAlwaysCull(osg::Drawable& drawable)
{
    ForceCullCallback* cullCB = new ForceCullCallback;
    drawable.setCullCallback (cullCB);
}

void osgManipulator::setMaterialColor(const osg::Vec4& color, osg::Node& node)
{
    osg::Material* mat = dynamic_cast<osg::Material*>(node.getOrCreateStateSet()->getAttribute(osg::StateAttribute::MATERIAL));
    if (! mat)
    {
        mat = new osg::Material;
        mat->setDataVariance(osg::Object::DYNAMIC);
        node.getOrCreateStateSet()->setAttribute(mat);
    }
    mat->setDiffuse(osg::Material::FRONT_AND_BACK, color);
}
