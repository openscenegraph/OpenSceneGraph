/*  -*-c++-*- 
 *  Copyright (C) 2011 Cedric Pinson <cedric.pinson@plopbyte.com>
 */
#ifndef JSON_ANIMATION_H
#define JSON_ANIMATION_H
#include <osgAnimation/Animation>
#include <osgAnimation/UpdateMatrixTransform>

class WriteVisitor;

JSONObject* createJSONAnimation(osgAnimation::Animation* anim, WriteVisitor* writer);
JSONObject* createJSONUpdateMatrixTransform(osgAnimation::UpdateMatrixTransform& acb, WriteVisitor* writer);


// Remaps: [(u0, v0,..., w0), (u1, v1, ... w1), ..., (un, vn, ..., wn)]
// into: [(u0, u1, ...), ..., (v0, v1, ...), ..., (w0, w1, ...), ...]
template<typename In, typename Out>
Out* pack(const In* array) {
    unsigned int inSize = array->getNumElements();
    unsigned int inDim = In::ElementDataType::num_components;
    unsigned int outDim = Out::ElementDataType::num_components;

    Out* interleaved = new Out(static_cast<unsigned int>(.5 + (inSize * inDim) * 1. / outDim));

    for(unsigned int i = 0 ; i < inSize ; ++ i) {
        for(unsigned int j = 0 ; j < inDim ; ++ j) {
            unsigned int index = j * inSize + i;
            (*interleaved)[index / outDim][index % outDim] = (*array)[i][j];
        }
    }
    return interleaved;
}

#endif
