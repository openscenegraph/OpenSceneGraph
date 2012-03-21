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

#include "SimpleDotVisitor.h"

namespace osgDot {

  SimpleDotVisitor::SimpleDotVisitor() {
  }

  SimpleDotVisitor::~SimpleDotVisitor() {
  }

  void SimpleDotVisitor::handle(osg::Node& node, int id) {
    std::stringstream label;
    label << "<top> Node";
    if ( !node.getName().empty() ) { label << "| " << node.getName(); }
    drawNode( id, "record", "solid", label.str(), "black", "white" );
  }

  void SimpleDotVisitor::handle(osg::Geode& node, int id) {
    std::stringstream label;
    label << "<top> " << node.className();
    if ( !node.getName().empty() ) { label << "| " << node.getName(); }
    drawNode( id, "record", "solid", label.str(), "brown", "white" );
  }

  void SimpleDotVisitor::handle(osg::Group& node, int id) {
    std::stringstream label;
    label << "<top> " << node.className();
    if ( !node.getName().empty() ) { label << "| " << node.getName(); }
    drawNode( id, "record", "solid", label.str(), "black", "white" );
  }

  void SimpleDotVisitor::handle(osg::Group& parent, osg::Node& child, int parentID, int childID ) {
    drawEdge( parentID, childID, "setlinewidth(2)" );
  }

  void SimpleDotVisitor::handle(osg::StateSet& stateset, int id) {
    std::stringstream label;
    label << "<top> " << stateset.className();
    if ( !stateset.getName().empty() ) { label << "| " << stateset.getName(); }
    drawNode( id, "Mrecord", "solid", label.str(), "green", "white" );
  }

  void SimpleDotVisitor::handle(osg::Node& node, osg::StateSet& stateset, int parentID, int childID ) {
    drawEdge( parentID, childID, "dashed" );
  }

  void SimpleDotVisitor::handle(osg::Drawable& drawable, int id) {
    std::stringstream label;
    label << "<top> " << drawable.className();
    if ( !drawable.getName().empty() ) { label << "| " << drawable.getName(); }
    drawNode( id, "record", "solid", label.str(), "blue", "white" );
  }

  void SimpleDotVisitor::handle(osg::Geode& geode, osg::Drawable& drawable, int parentID, int childID ) {
    drawEdge( parentID, childID, "dashed" );
  }

  void SimpleDotVisitor::handle(osg::Drawable& drawable, osg::StateSet& stateset, int parentID, int childID ) {
    drawEdge( parentID, childID, "dashed" );
  }

  void SimpleDotVisitor::drawNode( int id, const std::string& shape, const std::string& style, const std::string& label, const std::string& color, const std::string& fillColor ) {
    _nodes << id <<
      "[shape=\"" << shape <<
      "\" ,label=\"" << label <<
      "\" ,style=\"" << style <<
      "\" ,color=\"" << color <<
      "\" ,fillColor=\"" << fillColor <<
      "\"]" << std::endl;
  }

  void SimpleDotVisitor::drawEdge( int sourceId, int sinkId, const std::string& style ) {
    _edges
      << sourceId << ":top -> "
      << sinkId   << ":top [style=\""
      << style    << "\"];"
      << std::endl;
  }

} // namespace osgDot
