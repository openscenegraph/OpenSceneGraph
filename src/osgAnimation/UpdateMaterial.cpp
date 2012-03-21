/*  -*-c++-*-
 *  Copyright (C) 2009 Cedric Pinson <cedric.pinson@plopbyte.net>
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

#include <osgAnimation/UpdateMaterial>
#include <osg/NodeVisitor>

using namespace osgAnimation;

UpdateMaterial::UpdateMaterial(const UpdateMaterial& apc,const osg::CopyOp& copyop)
    : osg::Object(apc, copyop),
      AnimationUpdateCallback<osg::StateAttributeCallback>(apc, copyop)
{
    _diffuse = new osgAnimation::Vec4Target(apc._diffuse->getValue());
}

UpdateMaterial::UpdateMaterial(const std::string& name):
    AnimationUpdateCallback<osg::StateAttributeCallback>(name)
{
    _diffuse = new osgAnimation::Vec4Target(osg::Vec4(1,0,1,1));
}

/** Callback method called by the NodeVisitor when visiting a node.*/
void UpdateMaterial::operator()(osg::StateAttribute* sa, osg::NodeVisitor* nv)
{
    if (nv && nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
    {
        osg::Material* material = dynamic_cast<osg::Material*>(sa);
        if (material)
            update(*material);
    }
}


osgAnimation::Vec4Target* UpdateMaterial::getDiffuse() { return _diffuse.get(); }
void UpdateMaterial::update(osg::Material& material)
{
    osg::Vec4 diffuse = _diffuse->getValue();
    material.setDiffuse(osg::Material::FRONT_AND_BACK, diffuse);
}

bool UpdateMaterial::link(osgAnimation::Channel* channel)
{
    if (channel->getName().find("diffuse") != std::string::npos)
    {
        return channel->setTarget(_diffuse.get());
    }
    else
    {
        OSG_WARN << "Channel " << channel->getName() << " does not contain a valid symbolic name for this class " << className() << std::endl;
    }
    return false;
}
