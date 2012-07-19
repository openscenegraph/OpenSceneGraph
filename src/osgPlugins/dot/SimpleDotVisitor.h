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

#pragma once
#ifndef __SIMPLEDOTVISITOR_H__
#define __SIMPLEDOTVISITOR_H__

#ifndef __cplusplus
#error "this is a c++ - header!"
#endif

#include "BaseDotVisitor.h"

namespace osgDot {

  class SimpleDotVisitor : public BaseDotVisitor {
  public:
    SimpleDotVisitor();

    virtual ~SimpleDotVisitor();

  protected:
    virtual void handle(osg::Node& node, int id);
    virtual void handle(osg::Geode& geode, int id);
    virtual void handle(osg::Group& node, int id);

    virtual void handle(osg::StateSet& stateset, int id);
    virtual void handle(osg::Drawable& drawable, int id);

    virtual void handle(osg::Node& node, osg::StateSet& stateset, int parentID, int childID );
    virtual void handle(osg::Geode& geometry, osg::Drawable& drawable, int parentID, int childID );
    virtual void handle(osg::Group& parent, osg::Node& child, int parentID, int childID );
    virtual void handle(osg::Drawable& drawable, osg::StateSet& stateset, int parentID, int childID );

    virtual void drawNode( int id, const std::string& shape, const std::string& style, const std::string& label, const std::string& color, const std::string& fillColor );

    virtual void drawEdge( int sourceId, int sinkId, const std::string& style );

  };

} // namespace osgDot

#endif // __SIMPLEDOTVISITOR_H__
