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
#include "BaseDotVisitor.h"

#include <fstream>
#include <cassert>

#include <osg/Node>
#include <osg/Geode>
#include <osg/Group>

using namespace osg;

namespace osgDot {

  BaseDotVisitor::BaseDotVisitor() {
  }

  BaseDotVisitor::~BaseDotVisitor() {
  }

  bool BaseDotVisitor::run( osg::Node& root, std::ostream* fout ) {
    setTraversalMode( TRAVERSE_ALL_CHILDREN );
    if ( fout && *fout ) {
      root.accept( *this );
      
      *fout << "digraph osg_scenegraph { rankdir = LR;" << std::endl;
      
      *fout << _nodes.str() << _edges.str();
      
      *fout << "}" << std::endl;

      _nodes.clear();
      _edges.clear();
      _objectMap.clear();

      return true;
    }

    return false;
  }

  void BaseDotVisitor::apply(Node& node) { 
    int id;
    if ( getOrCreateId( &node, id ) ) {
      handle( node, id );
      handleNodeAndTraverse( node, id );
    }
  }

  void BaseDotVisitor::apply(Geode& node) {
    int id;
    if ( getOrCreateId( &node, id ) ) {
      handle( node, id );
      handleNodeAndTraverse( node, id );

      unsigned int i;
      for ( i = 0; i < node.getNumDrawables(); i++ ) {
        osg::Drawable* drawable = node.getDrawable( i );
        int id2;
        if ( getOrCreateId( drawable, id2 ) ) {
          handle( *drawable, id2 );
          osg::StateSet* s = drawable->getStateSet();
          if ( s ) {
            int id3;
            if ( getOrCreateId( s, id3 ) ) {
              handle( *s, id3 );
            }
            handle( *drawable, *s, id2, id3 );
          }
        }
        handle( node, *drawable, id, id2 );
      }

    }

  }
  
  void BaseDotVisitor::apply(Group& node) {
    int id;

    if ( getOrCreateId( &node, id ) ) {
      handle( node, id );
      handleNodeAndTraverse( node, id );

      unsigned int i;
      for ( i = 0; i < node.getNumChildren(); i++ ) {
        osg::Node* child = node.getChild( i );
        //handleNodeAndTraverse( *child );
        int id2;
        getOrCreateId( child, id2 );
        handle( node, *child, id, id2 );
      }

    }
    
  }

  void BaseDotVisitor::handle(osg::Node& node, int id) {
  }

  void BaseDotVisitor::handle(osg::Geode& node, int id) {
  }

  void BaseDotVisitor::handle(osg::Group& node, int id) {
  }

  void BaseDotVisitor::handle(osg::Group& parent, osg::Node& child, int parentID, int childID ) {
  }

  void BaseDotVisitor::handleNodeAndTraverse(osg::Node& node, int id) {
    osg::StateSet* ss = node.getStateSet();
    if ( ss ) {
      int id2;
      if ( getOrCreateId( ss, id2 ) ) {
        handle( *ss, id2 );
      }
      handle( node, *ss, id, id2 );
    }
    traverse(node);
  }

  void BaseDotVisitor::handle(osg::StateSet& stateset, int id) {
  }

  void BaseDotVisitor::handle(osg::Node& node, osg::StateSet& stateset, int parentID, int childID) {
  }
  
  void BaseDotVisitor::handle(osg::Drawable& drawable, int id) {
  }
  
  void BaseDotVisitor::handle(osg::Drawable& drawable, osg::StateSet& stateset, int parentID, int childID ) {
  }

  void BaseDotVisitor::handle(osg::Geode& geode, osg::Drawable& drawable, int parentID, int childID ) {
  }

  bool BaseDotVisitor::getOrCreateId( osg::Object* object, int &id ) {
    assert( object );
    ObjectMap::iterator it = _objectMap.find( object );
    if ( it != _objectMap.end() ) {
      id = it->second;
      return false;
    }

    id = _objectMap.size();
    _objectMap[ object ] = id;
    return true;
  }

} // namespace osgDot
