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

#ifndef __MY_NODE_VISITOR_H__
#define __MY_NODE_VISITOR_H__

class MyNode;
class Material;
class Coordinate3;
class MatrixTransform;
class Separator;
class IndexedFaceSet;
class TextureCoordinate;
class Texture2;
class Transform;

#include <osg/Referenced>

class MyNodeVisitor: public osg::Referenced {
public:
    virtual void applyMyNode(MyNode *node)=0;
    virtual void applyMaterial(Material *material)=0;
    virtual void applyCoordinate3(Coordinate3 *coord)=0;
    virtual void applyMatrixTransform(MatrixTransform *tr)=0;
    virtual void applySeparator(Separator *sep)=0;
    virtual void applyIndexedFaceSet(IndexedFaceSet *ifs)=0;
    virtual void applyTextureCoordinate(TextureCoordinate *texc)=0;
    virtual void applyTexture2(Texture2 *tex)=0;
    virtual void applyTransform(Transform *trans)=0;
};


#endif

