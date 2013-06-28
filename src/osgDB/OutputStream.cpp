/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2010 Robert Osfield
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
// Written by Wang Rui, (C) 2010

#include <osg/Version>
#include <osg/Notify>
#include <osgDB/FileUtils>
#include <osgDB/WriteFile>
#include <osgDB/ObjectWrapper>
#include <fstream>
#include <sstream>

using namespace osgDB;

OutputStream::OutputStream( const osgDB::Options* options )
:   _writeImageHint(WRITE_USE_IMAGE_HINT), _useSchemaData(false), _useRobustBinaryFormat(true)
{
    BEGIN_BRACKET.set( "{", +INDENT_VALUE );
    END_BRACKET.set( "}", -INDENT_VALUE );

    if ( !options ) return;
    _options = options;

    if ( options->getPluginStringData("RobustBinaryFormat")=="false" )
        _useRobustBinaryFormat = false;
    if ( options->getPluginStringData("SchemaData")=="true" )
        _useSchemaData = true;
    if ( !options->getPluginStringData("SchemaFile").empty() )
        _schemaName = options->getPluginStringData("SchemaFile");
    if ( !options->getPluginStringData("Compressor").empty() )
        _compressorName = options->getPluginStringData("Compressor");
    if ( !options->getPluginStringData("WriteImageHint").empty() )
    {
        std::string hintString = options->getPluginStringData("WriteImageHint");
        if ( hintString=="IncludeData" ) _writeImageHint = WRITE_INLINE_DATA;
        else if ( hintString=="IncludeFile" ) _writeImageHint = WRITE_INLINE_FILE;
        else if ( hintString=="UseExternal" ) _writeImageHint = WRITE_USE_EXTERNAL;
        else if ( hintString=="WriteOut" ) _writeImageHint = WRITE_EXTERNAL_FILE;
    }

    if ( !options->getPluginStringData("CustomDomains").empty() )
    {
        StringList domains, keyAndValue;
        split( options->getPluginStringData("CustomDomains"), domains, ';' );
        for ( unsigned int i=0; i<domains.size(); ++i )
        {
            split( domains[i], keyAndValue, ':' );
            if ( keyAndValue.size()>1 )
                _domainVersionMap[keyAndValue.front()] = atoi(keyAndValue.back().c_str());
        }
    }
}

OutputStream::~OutputStream()
{
}

int OutputStream::getFileVersion( const std::string& d ) const
{
    if ( d.empty() ) return OPENSCENEGRAPH_SOVERSION;
    VersionMap::const_iterator itr = _domainVersionMap.find(d);
    return itr==_domainVersionMap.end() ? 0 : itr->second;
}

OutputStream& OutputStream::operator<<( const osg::Vec2b& v )
{ *this << v.x() << v.y(); return *this; }

OutputStream& OutputStream::operator<<( const osg::Vec3b& v )
{ *this << v.x() << v.y() << v.z(); return *this; }

OutputStream& OutputStream::operator<<( const osg::Vec4b& v )
{ *this << v.x() << v.y() << v.z() << v.w(); return *this; }

OutputStream& OutputStream::operator<<( const osg::Vec2ub& v )
{ *this << v.x() << v.y(); return *this; }

OutputStream& OutputStream::operator<<( const osg::Vec3ub& v )
{ *this << v.x() << v.y() << v.z(); return *this; }

OutputStream& OutputStream::operator<<( const osg::Vec4ub& v )
{ *this << v.r() << v.g() << v.b() << v.a(); return *this; }

OutputStream& OutputStream::operator<<( const osg::Vec2s& v )
{ *this << v.x() << v.y(); return *this; }

OutputStream& OutputStream::operator<<( const osg::Vec3s& v )
{ *this << v.x() << v.y() << v.z(); return *this; }

OutputStream& OutputStream::operator<<( const osg::Vec4s& v )
{ *this << v.x() << v.y() << v.z() << v.w(); return *this; }

OutputStream& OutputStream::operator<<( const osg::Vec2us& v )
{ *this << v.x() << v.y(); return *this; }

OutputStream& OutputStream::operator<<( const osg::Vec3us& v )
{ *this << v.x() << v.y() << v.z(); return *this; }

OutputStream& OutputStream::operator<<( const osg::Vec4us& v )
{ *this << v.x() << v.y() << v.z() << v.w(); return *this; }

OutputStream& OutputStream::operator<<( const osg::Vec2f& v )
{ *this << v.x() << v.y(); return *this; }

OutputStream& OutputStream::operator<<( const osg::Vec3f& v )
{ *this << v.x() << v.y() << v.z(); return *this; }

OutputStream& OutputStream::operator<<( const osg::Vec4f& v )
{ *this << v.x() << v.y() << v.z() << v.w(); return *this; }

OutputStream& OutputStream::operator<<( const osg::Vec2d& v )
{ *this << v.x() << v.y(); return *this; }

OutputStream& OutputStream::operator<<( const osg::Vec3d& v )
{ *this << v.x() << v.y() << v.z(); return *this; }

OutputStream& OutputStream::operator<<( const osg::Vec4d& v )
{ *this << v.x() << v.y() << v.z() << v.w(); return *this; }

OutputStream& OutputStream::operator<<( const osg::Quat& q )
{ *this << q.x() << q.y() << q.z() << q.w(); return *this; }

OutputStream& OutputStream::operator<<( const osg::Plane& p )
{ *this << (double)p[0] << (double)p[1] << (double)p[2] << (double)p[3]; return *this; }


#if 0
OutputStream& OutputStream::operator<<( const osg::Matrixf& mat )
{
    *this << PROPERTY("Matrixf")<<BEGIN_BRACKET << std::endl;
    for ( int r=0; r<4; ++r )
    {
        *this << mat(r, 0) << mat(r, 1)
              << mat(r, 2) << mat(r, 3) << std::endl;
    }
    *this << END_BRACKET << std::endl;
    return *this;
}

OutputStream& OutputStream::operator<<( const osg::Matrixd& mat )
{
    *this << PROPERTY("Matrixd")<<BEGIN_BRACKET << std::endl;
    for ( int r=0; r<4; ++r )
    {
        *this << mat(r, 0) << mat(r, 1)
              << mat(r, 2) << mat(r, 3) << std::endl;
    }
    *this << END_BRACKET << std::endl;
    return *this;
}
#else
OutputStream& OutputStream::operator<<( const osg::Matrixf& mat )
{
    *this << BEGIN_BRACKET << std::endl;
    for ( int r=0; r<4; ++r )
    {
        *this << (double)mat(r, 0) << (double)mat(r, 1)
              << (double)mat(r, 2) << (double)mat(r, 3) << std::endl;
    }
    *this << END_BRACKET << std::endl;
    return *this;
}

OutputStream& OutputStream::operator<<( const osg::Matrixd& mat )
{
    *this << BEGIN_BRACKET << std::endl;
    for ( int r=0; r<4; ++r )
    {
        *this << mat(r, 0) << mat(r, 1)
              << mat(r, 2) << mat(r, 3) << std::endl;
    }
    *this << END_BRACKET << std::endl;
    return *this;
}
#endif

void OutputStream::writeArray( const osg::Array* a )
{
    if ( !a ) return;

    bool newID = false;
    unsigned int id = findOrCreateArrayID( a, newID );
    *this << PROPERTY("ArrayID") << id;
    if ( !newID )  // Shared array
    {
        *this << std::endl;
        return;
    }

    switch ( a->getType() )
    {
    case osg::Array::ByteArrayType:
        *this << MAPPEE(ArrayType, ID_BYTE_ARRAY);
        writeArrayImplementation( static_cast<const osg::ByteArray*>(a), a->getNumElements(), 4 );
        break;
    case osg::Array::UByteArrayType:
        *this << MAPPEE(ArrayType, ID_UBYTE_ARRAY);
        writeArrayImplementation( static_cast<const osg::UByteArray*>(a), a->getNumElements(), 4 );
        break;
    case osg::Array::ShortArrayType:
        *this << MAPPEE(ArrayType, ID_SHORT_ARRAY);
        writeArrayImplementation( static_cast<const osg::ShortArray*>(a), a->getNumElements(), 4 );
        break;
    case osg::Array::UShortArrayType:
        *this << MAPPEE(ArrayType, ID_USHORT_ARRAY);
        writeArrayImplementation( static_cast<const osg::UShortArray*>(a), a->getNumElements(), 4 );
        break;
    case osg::Array::IntArrayType:
        *this << MAPPEE(ArrayType, ID_INT_ARRAY);
        writeArrayImplementation( static_cast<const osg::IntArray*>(a), a->getNumElements(), 4 );
        break;
    case osg::Array::UIntArrayType:
        *this << MAPPEE(ArrayType, ID_UINT_ARRAY);
        writeArrayImplementation( static_cast<const osg::UIntArray*>(a), a->getNumElements(), 4 );
        break;
    case osg::Array::FloatArrayType:
        *this << MAPPEE(ArrayType, ID_FLOAT_ARRAY);
        writeArrayImplementation( static_cast<const osg::FloatArray*>(a), a->getNumElements(), 4 );
        break;
    case osg::Array::DoubleArrayType:
        *this << MAPPEE(ArrayType, ID_DOUBLE_ARRAY);
        writeArrayImplementation( static_cast<const osg::DoubleArray*>(a), a->getNumElements(), 4 );
        break;
    case osg::Array::Vec2bArrayType:
        *this << MAPPEE(ArrayType, ID_VEC2B_ARRAY);
        writeArrayImplementation( static_cast<const osg::Vec2bArray*>(a), a->getNumElements() );
        break;
    case osg::Array::Vec3bArrayType:
        *this << MAPPEE(ArrayType, ID_VEC3B_ARRAY);
        writeArrayImplementation( static_cast<const osg::Vec3bArray*>(a), a->getNumElements() );
        break;
    case osg::Array::Vec4bArrayType:
        *this << MAPPEE(ArrayType, ID_VEC4B_ARRAY);
        writeArrayImplementation( static_cast<const osg::Vec4bArray*>(a), a->getNumElements() );
        break;
    case osg::Array::Vec2ubArrayType:
        *this << MAPPEE(ArrayType, ID_VEC2UB_ARRAY);
        writeArrayImplementation( static_cast<const osg::Vec2ubArray*>(a), a->getNumElements() );
        break;
    case osg::Array::Vec3ubArrayType:
        *this << MAPPEE(ArrayType, ID_VEC3UB_ARRAY);
        writeArrayImplementation( static_cast<const osg::Vec3ubArray*>(a), a->getNumElements() );
        break;
    case osg::Array::Vec4ubArrayType:
        *this << MAPPEE(ArrayType, ID_VEC4UB_ARRAY);
        writeArrayImplementation( static_cast<const osg::Vec4ubArray*>(a), a->getNumElements() );
        break;
    case osg::Array::Vec2sArrayType:
        *this << MAPPEE(ArrayType, ID_VEC2S_ARRAY);
        writeArrayImplementation( static_cast<const osg::Vec2sArray*>(a), a->getNumElements() );
        break;
    case osg::Array::Vec3sArrayType:
        *this << MAPPEE(ArrayType, ID_VEC3S_ARRAY);
        writeArrayImplementation( static_cast<const osg::Vec3sArray*>(a), a->getNumElements() );
        break;
    case osg::Array::Vec4sArrayType:
        *this << MAPPEE(ArrayType, ID_VEC4S_ARRAY);
        writeArrayImplementation( static_cast<const osg::Vec4sArray*>(a), a->getNumElements() );
        break;
    case osg::Array::Vec2usArrayType:
        *this << MAPPEE(ArrayType, ID_VEC2US_ARRAY);
        writeArrayImplementation( static_cast<const osg::Vec2usArray*>(a), a->getNumElements() );
        break;
    case osg::Array::Vec3usArrayType:
        *this << MAPPEE(ArrayType, ID_VEC3US_ARRAY);
        writeArrayImplementation( static_cast<const osg::Vec3usArray*>(a), a->getNumElements() );
        break;
    case osg::Array::Vec4usArrayType:
        *this << MAPPEE(ArrayType, ID_VEC4US_ARRAY);
        writeArrayImplementation( static_cast<const osg::Vec4usArray*>(a), a->getNumElements() );
        break;
    case osg::Array::Vec2ArrayType:
        *this << MAPPEE(ArrayType, ID_VEC2_ARRAY);
        writeArrayImplementation( static_cast<const osg::Vec2Array*>(a), a->getNumElements() );
        break;
    case osg::Array::Vec3ArrayType:
        *this << MAPPEE(ArrayType, ID_VEC3_ARRAY);
        writeArrayImplementation( static_cast<const osg::Vec3Array*>(a), a->getNumElements() );
        break;
    case osg::Array::Vec4ArrayType:
        *this << MAPPEE(ArrayType, ID_VEC4_ARRAY);
        writeArrayImplementation( static_cast<const osg::Vec4Array*>(a), a->getNumElements() );
        break;
    case osg::Array::Vec2dArrayType:
        *this << MAPPEE(ArrayType, ID_VEC2D_ARRAY);
        writeArrayImplementation( static_cast<const osg::Vec2dArray*>(a), a->getNumElements() );
        break;
    case osg::Array::Vec3dArrayType:
        *this << MAPPEE(ArrayType, ID_VEC3D_ARRAY);
        writeArrayImplementation( static_cast<const osg::Vec3dArray*>(a), a->getNumElements() );
        break;
    case osg::Array::Vec4dArrayType:
        *this << MAPPEE(ArrayType, ID_VEC4D_ARRAY);
        writeArrayImplementation( static_cast<const osg::Vec4dArray*>(a), a->getNumElements() );
        break;
    default:
        throwException( "OutputStream::writeArray(): Unsupported array type." );
    }
}

void OutputStream::writePrimitiveSet( const osg::PrimitiveSet* p )
{
    if ( !p ) return;

    switch ( p->getType() )
    {
    case osg::PrimitiveSet::DrawArraysPrimitiveType:
        *this << MAPPEE(PrimitiveType, ID_DRAWARRAYS);
        {
            const osg::DrawArrays* da = static_cast<const osg::DrawArrays*>(p);
            *this << MAPPEE(PrimitiveType, da->getMode()) << da->getNumInstances()
                  << da->getFirst() << da->getCount() << std::endl;
        }
        break;
    case osg::PrimitiveSet::DrawArrayLengthsPrimitiveType:
        *this << MAPPEE(PrimitiveType, ID_DRAWARRAY_LENGTH);
        {
            const osg::DrawArrayLengths* dl = static_cast<const osg::DrawArrayLengths*>(p);
            *this << MAPPEE(PrimitiveType, dl->getMode()) << dl->getNumInstances() << dl->getFirst();
            writeArrayImplementation( dl, dl->size(), 4 );
        }
        break;
    case osg::PrimitiveSet::DrawElementsUBytePrimitiveType:
        *this << MAPPEE(PrimitiveType, ID_DRAWELEMENTS_UBYTE);
        {
            const osg::DrawElementsUByte* de = static_cast<const osg::DrawElementsUByte*>(p);
            *this << MAPPEE(PrimitiveType, de->getMode()) << de->getNumInstances();
            writeArrayImplementation( de, de->size(), 4 );
        }
        break;
    case osg::PrimitiveSet::DrawElementsUShortPrimitiveType:
        *this << MAPPEE(PrimitiveType, ID_DRAWELEMENTS_USHORT);
        {
            const osg::DrawElementsUShort* de = static_cast<const osg::DrawElementsUShort*>(p);
            *this << MAPPEE(PrimitiveType, de->getMode()) << de->getNumInstances();
            writeArrayImplementation( de, de->size(), 4 );
        }
        break;
    case osg::PrimitiveSet::DrawElementsUIntPrimitiveType:
        *this << MAPPEE(PrimitiveType, ID_DRAWELEMENTS_UINT);
        {
            const osg::DrawElementsUInt* de = static_cast<const osg::DrawElementsUInt*>(p);
            *this << MAPPEE(PrimitiveType, de->getMode()) << de->getNumInstances();
            writeArrayImplementation( de, de->size(), 4 );
        }
        break;
    default:
        throwException( "OutputStream::writePrimitiveSet(): Unsupported primitive type." );
    }
}

void OutputStream::writeImage( const osg::Image* img )
{
    if ( !img ) return;

    std::string name = img->libraryName();
    name += std::string("::") + img->className();

    bool newID = false;
    unsigned int id = findOrCreateObjectID( img, newID );

    *this << PROPERTY("ClassName") << name << std::endl;   // Write object name
    *this << PROPERTY("UniqueID") << id << std::endl;      // Write image ID
    if ( getException() ) return;

    if (newID)
    {
        int decision = IMAGE_EXTERNAL;
        switch ( _writeImageHint )
        {
        case OutputStream::WRITE_INLINE_DATA: decision = IMAGE_INLINE_DATA; break;
        case OutputStream::WRITE_INLINE_FILE: decision = IMAGE_INLINE_FILE; break;
        case OutputStream::WRITE_EXTERNAL_FILE: decision = IMAGE_WRITE_OUT; break;
        case OutputStream::WRITE_USE_EXTERNAL: decision = IMAGE_EXTERNAL; break;
        default:
            if ( img->getWriteHint()==osg::Image::EXTERNAL_FILE )
                decision = IMAGE_EXTERNAL;
            else if ( isBinary() )
                decision = IMAGE_INLINE_DATA;
            break;
        }


        std::string imageFileName = img->getFileName();
        if ( decision==IMAGE_WRITE_OUT || _writeImageHint==WRITE_EXTERNAL_FILE )
        {
            if (imageFileName.empty())
            {
                OSG_NOTICE<<"Empty Image::FileName resetting to image.dds"<<std::endl;
                imageFileName = "image.dds";
            }

            bool result = osgDB::writeImageFile( *img, imageFileName );
            OSG_NOTICE << "OutputStream::writeImage(): Write image data to external file " << imageFileName << std::endl;
            if ( !result )
            {
                OSG_WARN << "OutputStream::writeImage(): Failed to write " << img->getFileName() << std::endl;
            }
        }

        *this << PROPERTY("FileName"); writeWrappedString(imageFileName); *this << std::endl;
        *this << PROPERTY("WriteHint") << (int)img->getWriteHint();
        if ( getException() ) return;

        *this << decision << std::endl;

        switch ( decision )
        {
        case IMAGE_INLINE_DATA:
            if ( isBinary() )
            {
                *this << img->getOrigin();  // _origin
                *this << img->s() << img->t() << img->r(); // _s & _t & _r
                *this << img->getInternalTextureFormat();  // _internalTextureFormat
                *this << img->getPixelFormat();  // _pixelFormat
                *this << img->getDataType();  // _dataType
                *this << img->getPacking();  // _packing
                *this << img->getAllocationMode();  // _allocationMode

                // _data
                unsigned int size = img->getTotalSizeInBytesIncludingMipmaps();
                writeSize(size);

                for(osg::Image::DataIterator img_itr(img); img_itr.valid(); ++img_itr)
                {
                    writeCharArray( (char*)img_itr.data(), img_itr.size() );
                }

                // _mipmapData
                unsigned int numMipmaps = img->getNumMipmapLevels()-1;
                writeSize(numMipmaps);
                int s = img->s();
                int t = img->t();
                int r = img->r();
                unsigned int offset = 0;
                for (unsigned int i=0; i<numMipmaps; ++i)
                {
                    unsigned int size = osg::Image::computeImageSizeInBytes(s,t,r,img->getPixelFormat(),img->getDataType(),img->getPacking());
                    offset += size;

                    *this << offset;

                    s >>= 1;
                    t >>= 1;
                    r >>= 1;
                    if (s<1) s=1;
                    if (t<1) t=1;
                    if (r<1) r=1;
                }
            }
            break;
        case IMAGE_INLINE_FILE:
            if ( isBinary() )
            {
                std::string fullPath = osgDB::findDataFile( img->getFileName() );
                std::ifstream infile( fullPath.c_str(), std::ios::in|std::ios::binary );
                if ( infile )
                {
                    infile.seekg( 0, std::ios::end );
                    unsigned int size = infile.tellg();
                    writeSize(size);

                    if ( size>0 )
                    {
                        char* data = new char[size];
                        if ( !data )
                        {
                            throwException( "OutputStream::writeImage(): Out of memory." );
                            if ( getException() ) return;
                        }

                        infile.seekg( 0, std::ios::beg );
                        infile.read( data, size );
                        writeCharArray( data, size );
                        delete[] data;
                    }
                    infile.close();
                }
                else
                {
                    OSG_WARN << "OutputStream::writeImage(): Failed to open image file "
                                        << img->getFileName() << std::endl;
                    *this << (unsigned int)0;
                }
            }
            break;
        case IMAGE_EXTERNAL:
            break;
        default:
            break;
        }

        writeObjectFields( img );
    }

    // *this << END_BRACKET << std::endl;
}

void OutputStream::writeObject( const osg::Object* obj )
{
    if ( !obj ) return;

    std::string name = obj->libraryName();
    name += std::string("::") + obj->className();

    bool newID = false;
    unsigned int id = findOrCreateObjectID( obj, newID );

    *this << name << BEGIN_BRACKET << std::endl;       // Write object name
    *this << PROPERTY("UniqueID") << id << std::endl;  // Write object ID
    if ( getException() ) return;

    if (newID)
    {
        writeObjectFields(obj);
    }

    *this << END_BRACKET << std::endl;
}

void OutputStream::writeObjectFields( const osg::Object* obj )
{
    std::string name = obj->libraryName();
    name += std::string("::") + obj->className();

    ObjectWrapper* wrapper = Registry::instance()->getObjectWrapperManager()->findWrapper( name );
    if ( !wrapper )
    {
        OSG_WARN << "OutputStream::writeObject(): Unsupported wrapper class "
                                << name << std::endl;
        return;
    }

    const StringList& associates = wrapper->getAssociates();
    for ( StringList::const_iterator itr=associates.begin(); itr!=associates.end(); ++itr )
    {
        const std::string& assocName = *itr;
        ObjectWrapper* assocWrapper = Registry::instance()->getObjectWrapperManager()->findWrapper(assocName);
        if ( !assocWrapper )
        {
            OSG_WARN << "OutputStream::writeObject(): Unsupported associated class "
                                    << assocName << std::endl;
            continue;
        }
        else if ( _useSchemaData )
        {
            if ( _inbuiltSchemaMap.find(assocName)==_inbuiltSchemaMap.end() )
            {
                StringList properties;
                std::vector<int> types;
                assocWrapper->writeSchema( properties, types );

                unsigned int size = osg::minimum( properties.size(), types.size() );
                if ( size>0 )
                {
                    std::stringstream propertiesStream;
                    for ( unsigned int i=0; i<size; ++i )
                    {
                        propertiesStream << properties[i] << ":" << types[i] << " ";
                    }
                    _inbuiltSchemaMap[assocName] = propertiesStream.str();
                }
            }
        }
        _fields.push_back( assocWrapper->getName() );

        assocWrapper->write( *this, *obj );
        if ( getException() ) return;

        _fields.pop_back();
    }
}

void OutputStream::start( OutputIterator* outIterator, OutputStream::WriteType type )
{
    _fields.clear();
    _fields.push_back( "Start" );

    _out = outIterator;
    if ( !_out )
        throwException( "OutputStream: Null stream specified." );
    if ( getException() ) return;

    if ( isBinary() )
    {
        *this << (unsigned int)type << (unsigned int)OPENSCENEGRAPH_SOVERSION;

        bool useCompressSource = false;
        unsigned int attributes = 0;

        // From SOVERSION 98, start to support custom wrapper domains, enabling the attribute bit
        if ( _domainVersionMap.size()>0 ) attributes |= 0x1;

        if ( _useSchemaData )
        {
            attributes |= 0x2;  // Record if we use inbuilt schema data or not
            useCompressSource = true;
        }

        // From SOVERSION 98, start to support binary begin/end brackets so we can easily ignore
        // errors and unsupport classes, enabling the attribute bit
        if ( _useRobustBinaryFormat )
        {
            outIterator->setSupportBinaryBrackets( true );
            attributes |= 0x4;
        }
        *this << attributes;

        // Record all custom versions
        if ( _domainVersionMap.size()>0 )
        {
            unsigned int numDomains = _domainVersionMap.size();
            *this << numDomains;
            for ( VersionMap::iterator itr=_domainVersionMap.begin();
                  itr!=_domainVersionMap.end(); ++itr )
            {
                *this << itr->first << itr->second;
            }
        }

        if ( !_compressorName.empty() )
        {
            BaseCompressor* compressor = Registry::instance()->getObjectWrapperManager()->findCompressor(_compressorName);
            if ( !compressor )
            {
                OSG_WARN << "OutputStream::start(): No such compressor "
                                       << _compressorName << std::endl;
                _compressorName.clear();
            }
            else
            {
                useCompressSource = true;
            }
        }
        if ( !_compressorName.empty() ) *this << _compressorName;
        else *this << std::string("0");  // No compressor

        // Compressors and inbuilt schema use a new stream, which will be merged with the original one at the end.
        if ( useCompressSource )
        {
            _out->flush();
            _out->setStream( &_compressSource );
        }
    }
    else
    {
        std::string typeString("Unknown");
        switch ( type )
        {
        case WRITE_SCENE: typeString = "Scene"; break;
        case WRITE_IMAGE: typeString = "Image"; break;
        case WRITE_OBJECT: typeString = "Object"; break;
        default: break;
        }

        *this << typeString << std::endl;
        *this << PROPERTY("#Version") << (unsigned int)OPENSCENEGRAPH_SOVERSION << std::endl;
        *this << PROPERTY("#Generator") << std::string("OpenSceneGraph")
              << std::string(osgGetVersion()) << std::endl;
        if ( _domainVersionMap.size()>0 )
        {
            for ( VersionMap::iterator itr=_domainVersionMap.begin();
                  itr!=_domainVersionMap.end(); ++itr )
            {
                *this << PROPERTY("#CustomDomain") << itr->first << itr->second << std::endl;
            }
        }
        *this << std::endl;
    }
    _fields.pop_back();
}

void OutputStream::compress( std::ostream* ostream )
{
    _fields.clear();
    if ( !isBinary() ) return;

    std::stringstream schemaSource;
    if ( _useSchemaData )
    {
        _fields.push_back( "SchemaData" );

        std::string schemaData;
        for ( SchemaMap::iterator itr=_inbuiltSchemaMap.begin();
              itr!=_inbuiltSchemaMap.end(); ++itr )
        {
            schemaData += itr->first + '=';
            schemaData += itr->second;
            schemaData += '\n';
        }

        int size = schemaData.size();
        schemaSource.write( (char*)&size, INT_SIZE );
        schemaSource.write( schemaData.c_str(), size );

        _inbuiltSchemaMap.clear();
        _fields.pop_back();
    }

    if ( !_compressorName.empty() )
    {
        _fields.push_back( "Compression" );
        BaseCompressor* compressor = Registry::instance()->getObjectWrapperManager()->findCompressor(_compressorName);
        if ( !compressor || !ostream )
        {
            _fields.pop_back();
            return;
        }

        if ( !compressor->compress(*ostream, schemaSource.str() + _compressSource.str()) )
            throwException( "OutputStream: Failed to compress stream." );
        if ( getException() ) return;
        _fields.pop_back();
    }
    else if ( _useSchemaData )
    {
        std::string str = schemaSource.str() + _compressSource.str();
        ostream->write( str.c_str(), str.size() );
    }
}

void OutputStream::writeSchema( std::ostream& fout )
{
    // Write to external ascii stream
    const ObjectWrapperManager::WrapperMap& wrappers = Registry::instance()->getObjectWrapperManager()->getWrapperMap();
    for ( ObjectWrapperManager::WrapperMap::const_iterator itr=wrappers.begin();
          itr!=wrappers.end(); ++itr )
    {
        ObjectWrapper* wrapper = itr->second.get();
        fout << itr->first << " =";

        StringList properties;
        std::vector<int> types;
        wrapper->writeSchema( properties, types );

        std::string propertiesString;
        unsigned int size = osg::minimum( properties.size(), types.size() );
        for ( unsigned int i=0; i<size; ++i )
        {
            fout << " " << properties[i] << ":" << types[i];
        }
        fout << std::endl;
    }
}

// PROTECTED METHODS

template<typename T>
void OutputStream::writeArrayImplementation( const T* a, int write_size, unsigned int numInRow )
{
    *this << write_size << BEGIN_BRACKET;
    if ( numInRow>1 )
    {
        for ( int i=0; i<write_size; ++i )
        {
            if ( !(i%numInRow) )
            {
                *this << std::endl << (*a)[i];
            }
            else
                *this << (*a)[i];
        }
        *this << std::endl;
    }
    else
    {
        *this << std::endl;
        for ( int i=0; i<write_size; ++i )
            *this << (*a)[i] << std::endl;
    }
    *this << END_BRACKET << std::endl;
}

unsigned int OutputStream::findOrCreateArrayID( const osg::Array* array, bool& newID )
{
    ArrayMap::iterator itr = _arrayMap.find( array );
    if ( itr==_arrayMap.end() )
    {
        unsigned int id = _arrayMap.size()+1;
        _arrayMap[array] = id;
        newID = true;
        return id;
    }
    newID = false;
    return itr->second;
}

unsigned int OutputStream::findOrCreateObjectID( const osg::Object* obj, bool& newID )
{
    ObjectMap::iterator itr = _objectMap.find( obj );
    if ( itr==_objectMap.end() )
    {
        unsigned int id = _objectMap.size()+1;
        _objectMap[obj] = id;
        newID = true;
        return id;
    }
    newID = false;
    return itr->second;
}
