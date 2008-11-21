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
#include <osg/Material>

using namespace osgManipulator;

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

Dragger::Dragger() : _commandManager(0)
{
    _parentDragger = this;
    getOrCreateStateSet()->setDataVariance(osg::Object::DYNAMIC);
}

Dragger::~Dragger()
{
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
void CompositeDragger::setCommandManager(CommandManager* cm)
{
    for (DraggerList::iterator itr = _draggerList.begin(); itr != _draggerList.end(); ++itr)
    {
        (*itr)->setCommandManager(cm);
    }
    Dragger::setCommandManager(cm);
}

void CompositeDragger::setParentDragger(Dragger* dragger)
{
    for (DraggerList::iterator itr = _draggerList.begin(); itr != _draggerList.end(); ++itr)
    {
        (*itr)->setParentDragger(dragger);
    }
    Dragger::setParentDragger(dragger);
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
