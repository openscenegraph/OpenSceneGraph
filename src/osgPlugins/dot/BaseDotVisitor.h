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
#ifndef __DOTBASEVISITOR_H__
#define __DOTBASEVISITOR_H__

#ifndef __cplusplus
#error "this is a c++ - header!"
#endif

#include <sstream>

#include <osg/NodeVisitor>
#include <osg/Drawable>
#include <osg/Group>
#include <osg/Geode>
#include <osg/ref_ptr>

#include <osgDB/Options>

namespace osgDot {

  class BaseDotVisitor : public osg::NodeVisitor {
  public:
    typedef std::map< osg::Object*, int > ObjectMap;

  public:
    BaseDotVisitor();

    virtual ~BaseDotVisitor();

    void setOptions(const osgDB::Options* options);

    bool run( osg::Node& root, std::ostream* ostream );

    virtual void apply(osg::Node& node);

    virtual void apply(osg::Geode& node);

    virtual void apply(osg::Group& node);


  protected:

    void handleNodeAndTraverse(osg::Node& node, int id);

    virtual void handle(osg::Node& node, int id);
    virtual void handle(osg::Geode& node, int id);
    virtual void handle(osg::Group& node, int id);
    virtual void handle(osg::StateSet& stateset, int id);
    virtual void handle(osg::Drawable& drawable, int id);

    virtual void handle(osg::Node& node, osg::StateSet& stateset, int parentID, int childID);
    virtual void handle(osg::Group& parent, osg::Node& child, int parentID, int childID);
    virtual void handle(osg::Geode& geode, osg::Drawable& drawable, int parentID, int childID);
    virtual void handle(osg::Drawable& drawable, osg::StateSet& stateset, int parentID, int childID );

    osg::ref_ptr<osgDB::Options> _options;

    std::string       _rankdir;

    std::stringstream _nodes;
    std::stringstream _edges;

  private:
    bool getOrCreateId( osg::Object* object, int& id );

    ObjectMap _objectMap;

  };

} // namespace osgDot

#endif // __DOTBASEVISITOR_H__
