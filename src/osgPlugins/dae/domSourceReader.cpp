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

using namespace osgdae;

domSourceReader::domSourceReader() : m_array_type( None ), m_count( 0 )
{}

domSourceReader::domSourceReader( domSource *src ) : m_array_type( None ), m_count( 0 )
{
    domSource::domTechnique_common* technique = src->getTechnique_common();
    if ( technique == NULL ) {
        osg::notify(osg::WARN)<<"Warning: IntDaeSource::createFrom: Unable to find COMMON technique"<<std::endl;
        return;
    }

    domAccessor* accessor = technique->getAccessor();
    int stride = accessor->getStride();
    m_count = accessor->getCount();

    switch ( stride ) {
        case 1:
            m_array_type = Float;
            m_float_array = new osg::FloatArray();
            break;
        case 2:
            m_array_type = Vec2;
            m_vec2_array = new osg::Vec2Array();
            break;
        case 3:
            m_array_type = Vec3;
            m_vec3_array = new osg::Vec3Array();
            break;
        case 4:
            m_array_type = Vec4;
            m_vec4_array = new osg::Vec4Array();
            break;
        default:
            osg::notify(osg::WARN)<<"Unsupported stride: "<<stride<<std::endl;
            return;
    }

    // Only handle floats for now...
    daeDoubleArray* float_array = NULL;
    if ( src->getFloat_array() != NULL ) {
        float_array = &(src->getFloat_array()->getValue());
    }
    if ( !float_array ) {
        osg::notify(osg::WARN)<<"No float array found"<<std::endl;
        return;
    }
    
    daeDoubleArray& va = *float_array;
    for ( size_t i = 0; i < accessor->getCount(); i++ ) {
        switch ( accessor->getStride() ) {
            case 1:
                m_float_array->push_back( va[i] );
                break;
            case 2:
                m_vec2_array->push_back( osg::Vec2( va[i*2], va[i*2+1] ) );
                break;
            case 3:
                m_vec3_array->push_back( osg::Vec3( va[i*3], va[i*3+1], va[i*3+2] ) );
                break;
            case 4:
                m_vec4_array->push_back( osg::Vec4( va[i*4], va[i*4+1], va[i*4+2], va[i*4+3] ) );
                break;
            default:
                osg::notify(osg::WARN)<<"Unsupported stride in Source: "<<accessor->getStride()<<std::endl;
                return;
        }
    }
}
