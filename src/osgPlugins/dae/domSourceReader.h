/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the SCEA Shared Source License, Version 1.0 (the "License"); you may not use this 
 * file except in compliance with the License. You may obtain a copy of the License at:
 * http://research.scea.com/scea_shared_source_license.html
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License 
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or 
 * implied. See the License for the specific language governing permissions and limitations under the 
 * License. 
 */

#ifndef _DOM_SRC_CONV_H_
#define _DOM_SRC_CONV_H_

#include <osg/Array>
#include <osg/Notify>

class domSource;

namespace osgdae {

/**
@class domSourceReader
@brief Convert sources from DAE to OSG arrays
*/ 
class domSourceReader
{
public:
    enum ArrayType {None,Float,Vec2,Vec3,Vec4};

public:

    domSourceReader();
    domSourceReader( domSource *src );

    ArrayType getArrayType() const { return m_array_type; };

    osg::FloatArray* getFloatArray() { return m_float_array.get(); };
    
    osg::Vec2Array* getVec2Array() { return m_vec2_array.get(); };

    osg::Vec3Array* getVec3Array() { return m_vec3_array.get(); };

    osg::Vec4Array* getVec4Array() { return m_vec4_array.get(); };

    int getCount() const { return m_count; };

#define ASSERT_TYPE(type)       if (type!=m_array_type) { osg::notify(osg::WARN)<<"Wrong array type requested ("#type" != "<<m_array_type<<")"<<std::endl; }

    float getFloat( int index ) { ASSERT_TYPE( Float ); return (*m_float_array)[index]; };

    osg::Vec2 const& getVec2( int index ) { ASSERT_TYPE( Vec2 ); return (*m_vec2_array)[index]; };

    osg::Vec3 const& getVec3( int index ) { ASSERT_TYPE( Vec3 ); return (*m_vec3_array)[index]; };

    osg::Vec4 const& getVec4( int index ) { ASSERT_TYPE( Vec4 ); return (*m_vec4_array)[index]; };

#undef ASSERT_TYPE

protected:

    ArrayType m_array_type;
    int m_count;

    osg::ref_ptr<osg::FloatArray> m_float_array;
    osg::ref_ptr<osg::Vec2Array> m_vec2_array;
    osg::ref_ptr<osg::Vec3Array> m_vec3_array;
    osg::ref_ptr<osg::Vec4Array> m_vec4_array;
    
};

}

#endif
