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

#include <dom/domSource.h>

#ifdef COLLADA_DOM_2_4_OR_LATER
using namespace ColladaDOM141;
#endif

namespace osgDAE {

/**
@class domSourceReader
Converts a source to an OSG vector array as soon as you call a getter, so calling simple precision version \c getArray<osg::Vec3Array>() will force getArray<osg::Vec3dArray>() to return NULL, and vice-versa (for a Vec3 array in the example).
@brief Convert sources from DAE to OSG arrays
*/
class domSourceReader
{
public:
    enum ArrayType {None,Float,Vec2,Vec3,Vec4,Vec2d,Vec3d,Vec4d,Matrix,String};

public:

    domSourceReader();
    explicit domSourceReader( domSource *src );

    ArrayType getArrayType(bool enableDoublePrecision) const { if (srcInit) const_cast<domSourceReader*>(this)->convert(enableDoublePrecision); return m_array_type; };

    template <class OsgArrayType>
    inline OsgArrayType * getArray();

    int getCount(bool enableDoublePrecision) const { if (srcInit) const_cast<domSourceReader*>(this)->convert(enableDoublePrecision); return m_count; };

#define ASSERT_TYPE(type)       if (type!=m_array_type) { OSG_WARN<<"Wrong array type requested ("#type" != "<<m_array_type<<")"<<std::endl; }

    float getFloat( int index ) { ASSERT_TYPE( Float ); return (*m_float_array)[index]; };

    osg::Vec2 const& getVec2( int index ) { ASSERT_TYPE( Vec2 ); return (*m_vec2_array)[index]; };

    osg::Vec3 const& getVec3( int index ) { ASSERT_TYPE( Vec3 ); return (*m_vec3_array)[index]; };

    osg::Vec4 const& getVec4( int index ) { ASSERT_TYPE( Vec4 ); return (*m_vec4_array)[index]; };

    osg::Vec2d const& getVec2d( int index ) { ASSERT_TYPE( Vec2d ); return (*m_vec2d_array)[index]; };

    osg::Vec3d const& getVec3d( int index ) { ASSERT_TYPE( Vec3d ); return (*m_vec3d_array)[index]; };

    osg::Vec4d const& getVec4d( int index ) { ASSERT_TYPE( Vec4d ); return (*m_vec4d_array)[index]; };

    osg::Matrixf const& getMatrix( int index ) { ASSERT_TYPE( Matrix ); return (*m_matrix_array)[index]; };

#undef ASSERT_TYPE

protected:

    ArrayType m_array_type;
    int m_count;

    domSource * srcInit;        ///< Source used before initialization by convert(), NULL otherwise
    //bool initialized;
    void convert(bool doublePrecision);

    osg::ref_ptr<osg::FloatArray> m_float_array;
    osg::ref_ptr<osg::Vec2Array> m_vec2_array;
    osg::ref_ptr<osg::Vec3Array> m_vec3_array;
    osg::ref_ptr<osg::Vec4Array> m_vec4_array;
    osg::ref_ptr<osg::Vec2dArray> m_vec2d_array;
    osg::ref_ptr<osg::Vec3dArray> m_vec3d_array;
    osg::ref_ptr<osg::Vec4dArray> m_vec4d_array;
    osg::ref_ptr<osg::MatrixfArray> m_matrix_array;
};

template <>
inline osg::FloatArray* domSourceReader::getArray<osg::FloatArray>()
{
    if (srcInit)
        convert(false);
    return m_float_array.get();
}

template <>
inline osg::Vec2Array* domSourceReader::getArray<osg::Vec2Array>()
{
    if (srcInit)
        convert(false);
    return m_vec2_array.get();
}

template <>
inline osg::Vec3Array* domSourceReader::getArray<osg::Vec3Array>()
{
    if (srcInit)
        convert(false);
    return m_vec3_array.get();
}

template <>
inline osg::Vec4Array* domSourceReader::getArray<osg::Vec4Array>()
{
    if (srcInit)
        convert(false);
    return m_vec4_array.get();
}

template <>
inline osg::Vec2dArray* domSourceReader::getArray<osg::Vec2dArray>()
{
    if (srcInit)
        convert(true);
    return m_vec2d_array.get();
}

template <>
inline osg::Vec3dArray* domSourceReader::getArray<osg::Vec3dArray>()
{
    if (srcInit)
        convert(true);
    return m_vec3d_array.get();
}

template <>
inline osg::Vec4dArray* domSourceReader::getArray<osg::Vec4dArray>()
{
    if (srcInit)
        convert(true);
    return m_vec4d_array.get();
}

template <>
inline osg::MatrixfArray* domSourceReader::getArray<osg::MatrixfArray>()
{
    if (srcInit)
        convert(false);
    return m_matrix_array.get();
}

}

#endif
