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

#ifndef __MATRIX_TRANSFORM_H__
#define __MATRIX_TRANSFORM_H__

#include "mynode.h"
#include "geometry.h"

class MatrixTransform: public MyNode {
    Matrix m;
public:
    MatrixTransform(Matrix m) {memcpy(this->m,m,sizeof(float)*16);}
    float getElem(int i) { return m[i]; }
    virtual char *type() { return "MatrixTransform"; }
    virtual void accept(MyNodeVisitor *v) { v->applyMatrixTransform(this); }

};

#endif
