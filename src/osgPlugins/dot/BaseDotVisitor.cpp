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
#include <osg/Notify>

using namespace osg;

namespace osgDot {

BaseDotVisitor::BaseDotVisitor():
    osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
{
    // setNodeMaskOverride(0xffffff);

    _rankdir = "rankdir = LR;";
    // Set the locale used by the _nodes and _edges streams to the
    //   classic or "C" locale. This is needed because most of the
    //   Graphviz tools are not locale sensitive and get confused
    //   by id numbers containing commas or periods.
    _nodes.imbue(std::locale("C"));
    _edges.imbue(std::locale("C"));
}

BaseDotVisitor::~BaseDotVisitor()
{
}

void BaseDotVisitor::setOptions(const osgDB::Options* options)
{
    _options = const_cast<osgDB::Options*>(options);
    OSG_INFO<<"BaseDotVisitor::setOptions("<<options<<")"<<std::endl;
    if (_options.valid() && !(_options->getOptionString().empty()))
    {

        std::string optionString = _options->getOptionString();

        OSG_INFO<<"  BaseDotVisitor::optionString ("<<optionString<<")"<<std::endl;

        std::string::size_type pos = optionString.find("rankdir");
        if (pos!=std::string::npos)
        {
            std::string::size_type semi_pos = optionString.find(";",pos);
            if (semi_pos!=std::string::npos)
            {
                _rankdir = optionString.substr(pos, semi_pos-pos);
                OSG_INFO<<"  BaseDotVisitor::Set _rankdir to "<<_rankdir<<std::endl;
            }
        }
    }

}

bool BaseDotVisitor::run( osg::Node& root, std::ostream* fout )
{

    if ( fout && *fout )
    {
        root.accept( *this );

        *fout << "digraph osg_scenegraph { "<<_rankdir<< std::endl;

        *fout << "bgcolor=transparent;"<<std::endl;

        *fout << _nodes.str() << _edges.str();

        *fout << "}" << std::endl;

        _nodes.clear();
        _edges.clear();
        _objectMap.clear();

        return true;
    }

    return false;
}

void BaseDotVisitor::apply(Node& node)
{
    int id;
    if ( getOrCreateId( &node, id ) )
    {
        handle( node, id );
        handleNodeAndTraverse( node, id );
    }
}

void BaseDotVisitor::apply(Drawable& drawable)
{
    int id;
    if ( getOrCreateId( &drawable, id ) )
    {
        handle( drawable, id );
        handleNodeAndTraverse( drawable, id );
    }
}

void BaseDotVisitor::apply(Group& node)
{
    int id;

    if ( getOrCreateId( &node, id ) )
    {
        handle( node, id );
        handleNodeAndTraverse( node, id );

        unsigned int i;
        for ( i = 0; i < node.getNumChildren(); i++ )
        {
            osg::Node* child = node.getChild( i );
            //handleNodeAndTraverse( *child );
            int id2;
            getOrCreateId( child, id2 );
            handle( node, *child, id, id2 );
        }

    }
}

void BaseDotVisitor::handle(osg::Node&, int) {}

void BaseDotVisitor::handle(osg::Group&, int) {}

void BaseDotVisitor::handle(osg::Group&, osg::Node&, int, int) {}

void BaseDotVisitor::handleNodeAndTraverse(osg::Node& node, int id)
{
    osg::StateSet* ss = node.getStateSet();
    if ( ss )
    {
        int id2;
        if ( getOrCreateId( ss, id2 ) )
        {
            handle( *ss, id2 );
        }
        handle( node, *ss, id, id2 );
    }
    traverse(node);
}

void BaseDotVisitor::handle(osg::StateSet&, int) {}

void BaseDotVisitor::handle(osg::Node&, osg::StateSet&, int, int) {}

void BaseDotVisitor::handle(osg::Drawable&, int) {}

void BaseDotVisitor::handle(osg::Drawable&, osg::StateSet&, int, int) {}

bool BaseDotVisitor::getOrCreateId( osg::Object* object, int &id )
{
    ObjectMap::iterator it = _objectMap.find( object );
    if ( it != _objectMap.end() )
    {
        id = it->second;
        return false;
    }

    id = _objectMap.size();
    _objectMap[ object ] = id;
    return true;
}

} // namespace osgDot
