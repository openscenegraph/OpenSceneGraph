/*
 * osgDB::wrl - a VRML 1.0 loader for OpenSceneGraph
 * Copyright (C) 2002 Ruben Lopez <ryu@gpul.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __CACHE_NODOS_H__
#define __CACHE_NODOS_H__

#include <map>
#include "mynode.h"
#include "ltstr.h"

class NodeCache {
    typedef std::map<char *, osg::ref_ptr<MyNode>, ltstr > NodeMap;
    static NodeMap nodes;
public:
    static void addNode(char *name, MyNode *node) {
        nodes[name]=node;
    }
    static MyNode* getNode(char *name) {
	if (nodes.find(name) != nodes.end()) {
            return nodes[name].get();
	} else {
	    std::cerr << "Node does not exist" << std::endl;
	    return new MyNode();
	}
    }
};

#endif
