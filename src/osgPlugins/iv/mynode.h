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

#ifndef __NODE_H__
#define __NODE_H__

#include <vector>
#include <map>
#include <iostream>
#include "attribute.h"
#include "mynodevisitor.h"
#include "ltstr.h"

#ifdef WIN32
#  pragma warning (disable:4786)
#endif

class MyNode : public osg::Object {
public:
    typedef std::vector< osg::ref_ptr<MyNode> > MyNodeList;
    typedef std::map< const char*, osg::ref_ptr<Attribute>, ltstr > AttributeMap;
protected:
    MyNodeList children;
    AttributeMap attributes;
    bool two_sided;
public:
    MyNode() {two_sided=false;}
    MyNode(const MyNode *other) { children=other->children; attributes=other->attributes; two_sided=false; }
    virtual char *type() { return "MyNode"; } ///< Returns generic type
    void addAttribute(char *name, Attribute *atr) { attributes[name]=atr; }
    void addChild(MyNode *node) { children.push_back(node); }
    MyNodeList getChildren() { return children; }
    AttributeMap getAttributes() { return attributes; }
    Attribute *getAttribute(char *name) {
	if (attributes.find(name) != attributes.end()) {
	    return attributes[name].get();
	} else {
	    return 0;
	}
    }
    void setChildren(MyNodeList children) { this->children=children; }
    void setAttributes(AttributeMap attributes) { this->attributes=attributes; }

    void print(int indent) {
	for (int i=0;i<indent;i++) std::cout << " ";
	std::cout << type() << std::endl;
	MyNodeList::iterator iter;
	for (iter=children.begin();iter!=children.end();iter++) {
	    osg::ref_ptr<MyNode> node = *iter;
	    node->print(indent+4);
	}
    }
    void setTwoSided() { two_sided=true; }
    bool getTwoSided() { return two_sided; }
    virtual void accept(MyNodeVisitor *v) { v->applyMyNode(this); }

    // OSG Object API

    /** clone the an object of the same type as the node.*/
    virtual osg::Object* cloneType() const { return new MyNode(); }
    /** return a clone of a node, with Object* return type.*/
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new MyNode(this); }
    /** return the name of the node's library.*/
    virtual const char* libraryName() const { return "osgdb_wrl"; }
    /** return the name of the node's class type.*/
    virtual const char* className() const { return "MyNode"; }

 

};

#endif
