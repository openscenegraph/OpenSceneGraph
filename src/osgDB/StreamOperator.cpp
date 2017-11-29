/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2011 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include <osgDB/StreamOperator>
#include <osgDB/InputStream>

using namespace osgDB;

void InputIterator::checkStream() const
{
    if (_in->rdstate()&_in->failbit)
    {
        OSG_NOTICE<<"InputIterator::checkStream() : _in->rdstate() "<<_in->rdstate()<<", "<<_in->failbit<<std::endl;
        OSG_NOTICE<<"                               _in->tellg() = "<<_in->tellg()<<std::endl;
        _failed = true;
    }
}

void InputIterator::readComponentArray( char* s, unsigned int numElements, unsigned int numComponentsPerElements, unsigned int componentSizeInBytes)
{
    unsigned int size = numElements * numComponentsPerElements * componentSizeInBytes;
    if ( size>0 )
    {
        readCharArray( s, size);

        if (_byteSwap && componentSizeInBytes>1)
        {
            char* ptr = s;
            for(unsigned int i=0; i<numElements; ++i)
            {
                for(unsigned int j=0; j<numComponentsPerElements; ++j)
                {
                    osg::swapBytes( ptr, componentSizeInBytes );
                    ptr += componentSizeInBytes;
                }
            }
        }
    }
}

void InputIterator::throwException( const std::string& msg )
{
    if (_inputStream) _inputStream->throwException(msg);
    else OSG_WARN << msg << std::endl;
}

