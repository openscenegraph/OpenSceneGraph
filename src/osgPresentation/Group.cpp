/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2013 Robert Osfield
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

#include <osg/UserDataContainer>
#include <osgPresentation/Group>

using namespace osgPresentation;

class PropertyVisitor : public osg::NodeVisitor
{
    public:
      PropertyVisitor(const std::string& name) : _name(name), _object(0) {}

      void apply(osg::Node& node)
      {
        osg::UserDataContainer* udc = node.getUserDataContainer();
        _object = udc ? udc->getUserObject(_name) : 0;
        if (!_object) traverse(node);
      };

      std::string       _name;
      osg::Object*      _object;
};

osg::Object* Group::getPropertyObject(const std::string& name, bool checkParents)
{
    PropertyVisitor pv(name);
    if (checkParents) pv.setTraversalMode(osg::NodeVisitor::TRAVERSE_PARENTS);
    accept(pv);
    return pv._object;
}