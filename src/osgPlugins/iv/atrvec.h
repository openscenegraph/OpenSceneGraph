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

#ifndef __ATR_VEC_H__
#define __ATR_VEC_H__

#include "attribute.h"

class AtrVec: public Attribute {
    std::vector<float> values;
public:
    AtrVec(char *name):Attribute(name) { }
    AtrVec(char *name, float x, float y):Attribute(name) {
	values.push_back(x);
	values.push_back(y);
    }
    AtrVec(char *name, float x, float y, float z):Attribute(name) {
	values.push_back(x);
	values.push_back(y);
	values.push_back(z);
    }
    AtrVec(char *name, float x, float y, float z, float k):Attribute(name) {
	values.push_back(x);
	values.push_back(y);
	values.push_back(z);
	values.push_back(k);
    }
    void addVal(float val) { values.push_back(val); }
    float getVal(int pos) { return values[pos]; }
    float getValABS(int pos) { return values[pos]>0?values[pos]:-values[pos]; }
    float getValCut(int pos) { return values[pos]>0?values[pos]:0.0f; }
    int getSize() { return values.size(); }
    virtual char *type() { return "AtrVec"; }
};


#endif
