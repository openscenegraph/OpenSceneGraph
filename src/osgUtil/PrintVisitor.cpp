/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2009 Robert Osfield
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

#include <osgUtil/PrintVisitor>

namespace osgUtil
{

PrintVisitor::PrintVisitor(std::ostream& out, int indent, int step):
    osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
    _out(out),
    _indent(indent),
    _step(step)
{
}

void PrintVisitor::apply(osg::Node& node)
{
    output()<<node.libraryName()<<"::"<<node.className()<<std::endl;

    enter();
    traverse(node);
    leave();
}

}
