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

#ifndef __VISITANTE_OSG_H__
#define __VISITANTE_OSG_H__

#include "mynodevisitor.h"
#include <osg/Group>

class OSGVisitor: public MyNodeVisitor {
    osg::Node *root;
    osg::Transform *parent;
    Coordinate3 *coord3_active;
    TextureCoordinate *tcoord_active;
    Texture2 *texture_active;
    Material *material_active;
    int total_vert;
    bool two_sided;
public:
    OSGVisitor(MyNode *node);

    virtual void applyMyNode(MyNode* node);
    virtual void applyMaterial(Material *material);
    virtual void applyCoordinate3(Coordinate3 *coord);
    virtual void applyMatrixTransform(MatrixTransform *tr);
    virtual void applySeparator(Separator *sep);
    virtual void applyIndexedFaceSet(IndexedFaceSet *ifs);
    virtual void applyTextureCoordinate(TextureCoordinate *texc);
    virtual void applyTexture2(Texture2 *tex);
    virtual void applyTransform(Transform *trans);
    osg::Node* getRoot();

    virtual osg::Object* cloneType() const  { return new OSGVisitor(0); }

};

#endif
