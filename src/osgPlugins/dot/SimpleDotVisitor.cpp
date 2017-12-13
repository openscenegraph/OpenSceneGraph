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

SimpleDotVisitor::SimpleDotVisitor()
{
}

SimpleDotVisitor::~SimpleDotVisitor()
{
}


void SimpleDotVisitor::handle(osg::StateSet& stateset, int id)
{
    std::stringstream label;
    label << "<top> " << stateset.className();
    if ( !stateset.getName().empty() ) { label << " | " << stateset.getName(); }
    drawNode( id, "Mrecord", "solid, filled", label.str(), "green", "black" );
}

void SimpleDotVisitor::handle(osg::Drawable& drawable, int id)
{
    std::stringstream label;
    label << "<top> " << drawable.className();
    if ( !drawable.getName().empty() ) { label << " | " << drawable.getName(); }
    drawNode( id, "record", "solid, filled", label.str(), "lightblue", "black" );
}

void SimpleDotVisitor::handle(osg::Node& node, int id)
{
    std::stringstream label;
    label << "<top> "<<node.className();
    if ( !node.getName().empty() ) { label << " | " << node.getName(); }
    drawNode( id, "record", "solid", label.str(), "black", "white" );
}

void SimpleDotVisitor::handle(osg::Group& node, int id)
{
    std::stringstream label;
    label << "<top> " << node.className();
    if ( !node.getName().empty() ) { label << " | " << node.getName(); }
    drawNode( id, "record", "solid, filled", label.str(), "yellow", "black" );
}

void SimpleDotVisitor::handle(osg::Node&, osg::StateSet&, int parentID, int childID )
{
    drawEdge( parentID, childID, "" );
}

void SimpleDotVisitor::handle(osg::Drawable&, osg::StateSet&, int parentID, int childID )
{
    drawEdge( parentID, childID, "" );
}

void SimpleDotVisitor::handle(osg::Group&, osg::Node&, int parentID, int childID )
{
    drawEdge( parentID, childID, "" );
}

void SimpleDotVisitor::drawNode( int id, const std::string& shape, const std::string& style, const std::string& label, const std::string& color, const std::string& fillColor )
{
    _nodes << id <<
        "[shape=\"" << shape <<
        "\" ,label=\"" << label <<
        "\" ,style=\"" << style <<
        "\" ,color=\"" << color <<
        "\" ,fillColor=\"" << fillColor <<
        "\"]" << std::endl;
}

void SimpleDotVisitor::drawEdge( int sourceId, int sinkId, const std::string& style )
{
    _edges
        << sourceId << ":top -> "
        << sinkId   << ":top [style=\""
        << style    << "\"];"
        << std::endl;
}

} // namespace osgDot
