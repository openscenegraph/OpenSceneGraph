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

#ifndef __ATTRIBUTE_H__
#define __ATTRIBUTE_H__

#include <osg/Object>

class SG_EXPORT Attribute: public osg::Object {
public:
    char *name;
    virtual char *type()=0;
    char *getName() { return name; }

    // OSG Object API

    /** clone the an object of the same type as the node.*/
    virtual Object* cloneType() const =0;
    /** return a clone of a node, with Object* return type.*/
    virtual Object* clone(const osg::CopyOp& copyop) const { return cloneType(); }
    /** return the name of the node's library.*/
    virtual const char* libraryName() const { return "osgdb_wrl"; }
    /** return the name of the node's class type.*/
    virtual const char* className() const { return "Attribute"; }
protected:
    Attribute(char *name) {this->name=strdup(name); }
    ~Attribute() {free(name); }
};

#endif
