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

#ifndef __TEXTURE_COORDINATE_H__
#define __TEXTURE_COORDINATE_H__

#include "mynode.h"

class TextureCoordinate: public MyNode {
    TextureCoordList tcoords;
public:
    TextureCoordinate() {}
    TextureCoordinate(TextureCoordList *t) {
        tcoords=*t;
    }
    virtual char *type() { return "TextureCoordinate"; }
    virtual void accept(MyNodeVisitor *v) { v->applyTextureCoordinate(this); }
    TextureCoordList getTextureCoords() { return tcoords; }
};

#endif
