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

#ifndef __INDEXED_TRI_STRIP_SET_H__
#define __INDEXED_TRI_STRIP_SET_H__

#include "mynode.h"

class IndexedTriStripSet: public MyNode {
    PolygonList polys;
    PolygonList textureIndices; // Indexed texture set.
    bool _hasTextureIndices;
public:
    IndexedTriStripSet() {}

    IndexedTriStripSet(PolygonList *p) {
	polys=*p;
        _hasTextureIndices=false;
    }

    IndexedTriStripSet(PolygonList *p, PolygonList *t) {
	polys=*p;
        textureIndices=*t;
        _hasTextureIndices=true;
    }

    bool hasTextureIndices() { return _hasTextureIndices; }
    PolygonList getPolygons() { return polys; }
    PolygonList getTextureIndices() { return textureIndices; }
    virtual char *type() { return "IndexedTriStripSet"; }
    virtual void accept(MyNodeVisitor *v) { v->applyIndexedTriStripSet(this); }

};


#endif
