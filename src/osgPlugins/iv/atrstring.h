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

#ifndef __ATR_STRING_H__
#define __ATR_STRING_H__

#include "attribute.h"

class AtrString: public Attribute {
    char *value;
public:
    AtrString(char *name, char *value):Attribute(name) { this->value=strdup(value); }
    ~AtrString() { free (value); }
    virtual char *type() { return "AtrString"; }
    char *getValue() { return value; }
    /** clone the an object of the same type as the node.*/
    virtual Object* cloneType() const { return new AtrString(name,value); }
};

#endif
