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

#include "domSourceReader.h"

#include <dom/domSource.h>

using namespace osgDAE;

domSourceReader::domSourceReader() : m_array_type( None ), m_count( 0 ), srcInit( NULL )//, initialized( false )
{}

domSourceReader::domSourceReader( domSource *src ) : m_array_type( None ), m_count( 0 ), srcInit( src )//, initialized( false )
{
}

void domSourceReader::convert(bool doublePrecision)
{
    domSource * src = srcInit;
    srcInit = NULL;
    domSource::domTechnique_common* technique = src->getTechnique_common();
    if ( technique == NULL )
    {
        OSG_WARN<<"Warning: IntDaeSource::createFrom: Unable to find COMMON technique"<<std::endl;
        return;
    }

    domAccessor* accessor = technique->getAccessor();
    int stride = accessor->getStride();
    m_count = accessor->getCount();

    // Only handle floats or name array for now...
    daeDoubleArray* float_array = NULL;
    if (src->getFloat_array())
    {
        float_array = &(src->getFloat_array()->getValue());
    }
    else if (src->getName_array())
    {
        m_array_type = String;
        return;
    }

    switch (stride)
    {
        case 1:
            m_array_type = Float;
            m_float_array = new osg::FloatArray();
            break;
        case 2:
            if (!doublePrecision)
            {
                m_array_type = Vec2;
                m_vec2_array = new osg::Vec2Array();
            }
            else
            {
                m_array_type = Vec2d;
                m_vec2d_array = new osg::Vec2dArray();
            }
            break;
        case 3:
            if (!doublePrecision)
            {
                m_array_type = Vec3;
                m_vec3_array = new osg::Vec3Array();
            }
            else
            {
                m_array_type = Vec3d;
                m_vec3d_array = new osg::Vec3dArray();
            }
            break;
        case 4:
            if (!doublePrecision)
            {
                m_array_type = Vec4;
                m_vec4_array = new osg::Vec4Array();
            }
            else
            {
                m_array_type = Vec4d;
                m_vec4d_array = new osg::Vec4dArray();
            }
            break;
        case 16:
            m_array_type = Matrix;
            m_matrix_array = new osg::MatrixfArray();
            break;
        default:
            OSG_WARN<<"Unsupported stride: "<<stride<<std::endl;
            return;
    }

    if (float_array)
    {
        daeDoubleArray& va = *float_array;
        switch (m_array_type)
        {
        case Float:
            for ( size_t i = 0; i < accessor->getCount(); i++ ) {
                m_float_array->push_back(va[i]);
            }
            break;
        case Vec2:
            for ( size_t i = 0; i < accessor->getCount(); i++ ) {
                m_vec2_array->push_back( osg::Vec2( va[i*2], va[i*2+1]));
            }
            break;
        case Vec2d:
            for ( size_t i = 0; i < accessor->getCount(); i++ ) {
                m_vec2d_array->push_back( osg::Vec2d( va[i*2], va[i*2+1]));
            }
            break;
        case Vec3:
            for ( size_t i = 0; i < accessor->getCount(); i++ ) {
                m_vec3_array->push_back( osg::Vec3( va[i*3], va[i*3+1], va[i*3+2]));
            }
            break;
        case Vec3d:
            for ( size_t i = 0; i < accessor->getCount(); i++ ) {
                m_vec3d_array->push_back( osg::Vec3d( va[i*3], va[i*3+1], va[i*3+2]));
            }
            break;
        case Vec4:
            for ( size_t i = 0; i < accessor->getCount(); i++ ) {
                m_vec4_array->push_back( osg::Vec4( va[i*4], va[i*4+1], va[i*4+2], va[i*4+3]));
            }
            break;
        case Vec4d:
            for ( size_t i = 0; i < accessor->getCount(); i++ ) {
                m_vec4d_array->push_back( osg::Vec4d( va[i*4], va[i*4+1], va[i*4+2], va[i*4+3]));
            }
            break;
        case Matrix:
            for ( size_t i = 0; i < accessor->getCount(); i++ ) {
                m_matrix_array->push_back(osg::Matrixf( va[i*16+0],    va[i*16+4],    va[i*16+8],    va[i*16+12],
                                                        va[i*16+1],    va[i*16+5],    va[i*16+9],    va[i*16+13],
                                                        va[i*16+2],    va[i*16+6],    va[i*16+10],    va[i*16+14],
                                                        va[i*16+3],    va[i*16+7],    va[i*16+11],    va[i*16+15]));
            }
            break;
        default:
            OSG_WARN << "Unsupported stride in Source: " << stride << std::endl;
            return;
        }
    }
    else
    {
        OSG_WARN << "No float array found" << std::endl;
    }
}
