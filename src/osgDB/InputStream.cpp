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

#include <osg/Notify>
#include <osg/ImageSequence>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/XmlParser>
#include <osgDB/FileNameUtils>
#include <osgDB/ObjectWrapper>
#include <osgDB/ConvertBase64>

using namespace osgDB;

static std::string s_lastSchema;

InputStream::InputStream( const osgDB::Options* options )
    :   _fileVersion(0), _useSchemaData(false), _forceReadingImage(false), _dataDecompress(0)
{
    BEGIN_BRACKET.set( "{", +INDENT_VALUE );
    END_BRACKET.set( "}", -INDENT_VALUE );

    if ( !options ) return;
    _options = options;

    if ( options->getPluginStringData("ForceReadingImage")=="true" )
        _forceReadingImage = true;

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

    std::string schema;
    if ( !options->getPluginStringData("SchemaFile").empty() )
    {
        schema = options->getPluginStringData("SchemaFile");
        if ( s_lastSchema!=schema )
        {
            osgDB::ifstream schemaStream( schema.c_str(), std::ios::in );
            if ( !schemaStream.fail() ) readSchema( schemaStream );
            schemaStream.close();
            s_lastSchema = schema;
        }
    }

    if ( schema.empty() )
    {
        resetSchema();
        s_lastSchema.clear();
    }

    // assign dummy object to used for reading field properties that will be discarded.
    _dummyReadObject = new osg::DummyObject;
}

InputStream::~InputStream()
{
    if (_dataDecompress)
        delete _dataDecompress;
}

int InputStream::getFileVersion( const std::string& d ) const
{
    if ( d.empty() ) return _fileVersion;
    VersionMap::const_iterator itr = _domainVersionMap.find(d);
    return itr==_domainVersionMap.end() ? 0 : itr->second;
}

InputStream& InputStream::operator>>( osg::Vec2b& v )
{
    char x, y; *this >> x >> y;
    v.set( x, y );
    return *this;
}

InputStream& InputStream::operator>>( osg::Vec3b& v )
{
    char x, y, z; *this >> x >> y >> z;
    v.set( x, y, z );
    return *this;
}

InputStream& InputStream::operator>>( osg::Vec4b& v )
{
    char x, y, z, w; *this >> x >> y >> z >> w;
    v.set( x, y, z, w );
    return *this;
}

InputStream& InputStream::operator>>( osg::Vec2ub& v )
{
    unsigned char x, y; *this >> x >> y;
    v.set( x, y );
    return *this;
}

InputStream& InputStream::operator>>( osg::Vec3ub& v )
{
    unsigned char x, y, z; *this >> x >> y >> z;
    v.set( x, y, z );
    return *this;
}

InputStream& InputStream::operator>>( osg::Vec4ub& v )
{
    unsigned char r, g, b, a; *this >> r >> g >> b >> a;
    v.set( r, g, b, a );
    return *this;
}

InputStream& InputStream::operator>>( osg::Vec2s& v )
{ *this >> v.x() >> v.y(); return *this; }

InputStream& InputStream::operator>>( osg::Vec3s& v )
{ *this >> v.x() >> v.y() >> v.z(); return *this; }

InputStream& InputStream::operator>>( osg::Vec4s& v )
{ *this >> v.x() >> v.y() >> v.z() >> v.w(); return *this; }

InputStream& InputStream::operator>>( osg::Vec2us& v )
{ *this >> v.x() >> v.y(); return *this; }

InputStream& InputStream::operator>>( osg::Vec3us& v )
{ *this >> v.x() >> v.y() >> v.z(); return *this; }

InputStream& InputStream::operator>>( osg::Vec4us& v )
{ *this >> v.x() >> v.y() >> v.z() >> v.w(); return *this; }


InputStream& InputStream::operator>>( osg::Vec2i& v )
{ *this >> v.x() >> v.y(); return *this; }

InputStream& InputStream::operator>>( osg::Vec3i& v )
{ *this >> v.x() >> v.y() >> v.z(); return *this; }

InputStream& InputStream::operator>>( osg::Vec4i& v )
{ *this >> v.x() >> v.y() >> v.z() >> v.w(); return *this; }


InputStream& InputStream::operator>>( osg::Vec2ui& v )
{ *this >> v.x() >> v.y(); return *this; }

InputStream& InputStream::operator>>( osg::Vec3ui& v )
{ *this >> v.x() >> v.y() >> v.z(); return *this; }

InputStream& InputStream::operator>>( osg::Vec4ui& v )
{ *this >> v.x() >> v.y() >> v.z() >> v.w(); return *this; }


InputStream& InputStream::operator>>( osg::Vec2f& v )
{ *this >> v.x() >> v.y(); return *this; }

InputStream& InputStream::operator>>( osg::Vec3f& v )
{ *this >> v.x() >> v.y() >> v.z(); return *this; }

InputStream& InputStream::operator>>( osg::Vec4f& v )
{ *this >> v.x() >> v.y() >> v.z() >> v.w(); return *this; }


InputStream& InputStream::operator>>( osg::Vec2d& v )
{ *this >> v.x() >> v.y(); return *this; }

InputStream& InputStream::operator>>( osg::Vec3d& v )
{ *this >> v.x() >> v.y() >> v.z(); return *this; }

InputStream& InputStream::operator>>( osg::Vec4d& v )
{ *this >> v.x() >> v.y() >> v.z() >> v.w(); return *this; }


InputStream& InputStream::operator>>( osg::Quat& q )
{ *this >> q.x() >> q.y() >> q.z() >> q.w(); return *this; }

InputStream& InputStream::operator>>( osg::Plane& p )
{
    double p0, p1, p2, p3; *this >> p0 >> p1 >> p2 >> p3;
    p.set( p0, p1, p2, p3 ); return *this;
}

#if 0
InputStream& InputStream::operator>>( osg::Matrixf& mat )
{
   ObjectProperty property("");
   *this >> property  >> BEGIN_BRACKET;

   if (property._name == "Matrixf")
   {
        // stream has same type as what we want to read so read directly
        for ( int r=0; r<4; ++r )
        {
            *this >> mat(r, 0) >> mat(r, 1) >> mat(r, 2) >> mat(r, 3);
        }
   }
   else if (property._name == "Matrixd")
   {
        // stream has different type than what we want to read so read stream into
        // a temporary and then copy across to the final matrix
        double value;
        for ( int r=0; r<4; ++r )
        {
            for ( int c=0; c<4; ++c)
            {
                *this >> value;
                mat(r,c) = static_cast<float>(value);
            }
        }
   }

   *this >> END_BRACKET;
   return *this;
}

InputStream& InputStream::operator>>( osg::Matrixd& mat )
{
   ObjectProperty property("");
   *this >> property  >> BEGIN_BRACKET;

   if (property._name == "Matrixf")
   {
        // stream has different type than what we want to read so read stream into
        // a temporary and then copy across to the final matrix
        float value;
        for ( int r=0; r<4; ++r )
        {
            for ( int c=0; c<4; ++c)
            {
                *this >> value;
                mat(r,c) = static_cast<float>(value);
            }
        }
   }
   else if (property._name == "Matrixd")
   {
        // stream has same type as what we want to read so read directly
        for ( int r=0; r<4; ++r )
        {
            *this >> mat(r, 0) >> mat(r, 1) >> mat(r, 2) >> mat(r, 3);
        }
   }

   *this >> END_BRACKET;
   return *this;
}
#else
InputStream& InputStream::operator>>( osg::Matrixf& mat )
{
    *this >> BEGIN_BRACKET;

    // stream has different type than what we want to read so read stream into
    // a temporary and then copy across to the final matrix
    double value;
    for ( int r=0; r<4; ++r )
    {
        for ( int c=0; c<4; ++c)
        {
            *this >> value;
            mat(r,c) = static_cast<float>(value);
        }
    }

    *this >> END_BRACKET;
    return *this;
}

InputStream& InputStream::operator>>( osg::Matrixd& mat )
{
    *this >> BEGIN_BRACKET;

    for ( int r=0; r<4; ++r )
    {
        *this >> mat(r, 0) >> mat(r, 1) >> mat(r, 2) >> mat(r, 3);
    }

    *this >> END_BRACKET;
    return *this;
}
#endif

InputStream& InputStream::operator>>( osg::BoundingBoxf& bb)
{
    float p0, p1, p2, p3, p4, p5; *this >> p0 >> p1 >> p2 >> p3>> p4>> p5;
    bb.set( p0, p1, p2, p3, p4, p5 ); return *this;
}

InputStream& InputStream::operator>>( osg::BoundingBoxd& bb)
{
    double p0, p1, p2, p3, p4, p5; *this >> p0 >> p1 >> p2 >> p3>> p4>> p5;
    bb.set( p0, p1, p2, p3, p4, p5 ); return *this;
}

InputStream& InputStream::operator>>( osg::BoundingSpheref& bs)
{
    float p0, p1, p2, p3; *this >> p0 >> p1 >> p2 >> p3;
    bs.set( osg::Vec3f(p0, p1, p2), p3 ); return *this;
}

InputStream& InputStream::operator>>( osg::BoundingSphered& bs)
{
    double p0, p1, p2, p3; *this >> p0 >> p1 >> p2 >> p3;
    bs.set( osg::Vec3d(p0, p1, p2), p3 ); return *this;
}



osg::Array* InputStream::readArray()
{
    osg::ref_ptr<osg::Array> array = NULL;

    unsigned int id = 0;
    *this >> PROPERTY("ArrayID") >> id;

    ArrayMap::iterator itr = _arrayMap.find( id );
    if ( itr!=_arrayMap.end() )
    {
        return itr->second.get();
    }

    DEF_MAPPEE(ArrayType, type);
    *this >> type;
    switch ( type.get() )
    {
    case ID_BYTE_ARRAY:
        {
            osg::ByteArray* ba = new osg::ByteArray;
            readArrayImplementation( ba, 1, CHAR_SIZE);
            array = ba;
        }
        break;
    case ID_UBYTE_ARRAY:
        {
            osg::UByteArray* uba = new osg::UByteArray;
            readArrayImplementation( uba, 1, CHAR_SIZE );
            array = uba;
        }
        break;
    case ID_SHORT_ARRAY:
        {
            osg::ShortArray* sa = new osg::ShortArray;
            readArrayImplementation( sa, 1, SHORT_SIZE );
            array = sa;
        }
        break;
    case ID_USHORT_ARRAY:
        {
            osg::UShortArray* usa = new osg::UShortArray;
            readArrayImplementation( usa, 1, SHORT_SIZE );
            array = usa;
        }
        break;
    case ID_INT_ARRAY:
        {
            osg::IntArray* ia = new osg::IntArray;
            readArrayImplementation( ia, 1, INT_SIZE );
            array = ia;
        }
        break;
    case ID_UINT_ARRAY:
        {
            osg::UIntArray* uia = new osg::UIntArray;
            readArrayImplementation( uia, 1, INT_SIZE );
            array = uia;
        }
        break;
    case ID_FLOAT_ARRAY:
        {
            osg::FloatArray* fa = new osg::FloatArray;
            readArrayImplementation( fa, 1, FLOAT_SIZE );
            array = fa;
        }
        break;
    case ID_DOUBLE_ARRAY:
        {
            osg::DoubleArray* da = new osg::DoubleArray;
            readArrayImplementation( da, 1, DOUBLE_SIZE );
            array = da;
        }
        break;
    case ID_VEC2B_ARRAY:
        {
            osg::Vec2bArray* va = new osg::Vec2bArray;
            readArrayImplementation( va, 2, CHAR_SIZE );
            array = va;
        }
        break;
    case ID_VEC3B_ARRAY:
        {
            osg::Vec3bArray* va = new osg::Vec3bArray;
            readArrayImplementation( va, 3, CHAR_SIZE );
            array = va;
        }
        break;
    case ID_VEC4B_ARRAY:
        {
            osg::Vec4bArray* va = new osg::Vec4bArray;
            readArrayImplementation( va, 4, CHAR_SIZE );
            array = va;
        }
        break;
    case ID_VEC2UB_ARRAY:
        {
            osg::Vec2ubArray* va = new osg::Vec2ubArray;
            readArrayImplementation( va, 2, CHAR_SIZE );
            array = va;
        }
        break;
    case ID_VEC3UB_ARRAY:
        {
            osg::Vec3ubArray* va = new osg::Vec3ubArray;
            readArrayImplementation( va, 3, CHAR_SIZE );
            array = va;
        }
        break;
    case ID_VEC4UB_ARRAY:
        {
            osg::Vec4ubArray* va = new osg::Vec4ubArray;
            readArrayImplementation( va, 4, CHAR_SIZE );
            array = va;
        }
        break;
    case ID_VEC2S_ARRAY:
        {
            osg::Vec2sArray* va = new osg::Vec2sArray;
            readArrayImplementation( va, 2, SHORT_SIZE );
            array = va;
        }
        break;
    case ID_VEC3S_ARRAY:
        {
            osg::Vec3sArray* va = new osg::Vec3sArray;
            readArrayImplementation( va, 3, SHORT_SIZE );
            array = va;
        }
        break;
    case ID_VEC4S_ARRAY:
        {
            osg::Vec4sArray* va = new osg::Vec4sArray;
            readArrayImplementation( va, 4, SHORT_SIZE );
            array = va;
        }
        break;
    case ID_VEC2US_ARRAY:
        {
            osg::Vec2usArray* va = new osg::Vec2usArray;
            readArrayImplementation( va, 2, SHORT_SIZE );
            array = va;
        }
        break;
    case ID_VEC3US_ARRAY:
        {
            osg::Vec3usArray* va = new osg::Vec3usArray;
            readArrayImplementation( va, 3, SHORT_SIZE );
            array = va;
        }
        break;
    case ID_VEC4US_ARRAY:
        {
            osg::Vec4usArray* va = new osg::Vec4usArray;
            readArrayImplementation( va, 4, SHORT_SIZE );
            array = va;
        }
        break;
    case ID_VEC2_ARRAY:
        {
            osg::Vec2Array* va = new osg::Vec2Array;
            readArrayImplementation( va, 2, FLOAT_SIZE );
            array = va;
        }
        break;
    case ID_VEC3_ARRAY:
        {
            osg::Vec3Array* va = new osg::Vec3Array;
            readArrayImplementation( va, 3, FLOAT_SIZE );
            array = va;
        }
        break;
    case ID_VEC4_ARRAY:
        {
            osg::Vec4Array* va = new osg::Vec4Array;
            readArrayImplementation( va, 4, FLOAT_SIZE );
            array = va;
        }
        break;
    case ID_VEC2D_ARRAY:
        {
            osg::Vec2dArray* va = new osg::Vec2dArray;
            readArrayImplementation( va, 2, DOUBLE_SIZE );
            array = va;
        }
        break;
    case ID_VEC3D_ARRAY:
        {
            osg::Vec3dArray* va = new osg::Vec3dArray;
            readArrayImplementation( va, 3, DOUBLE_SIZE );
            array = va;
        }
        break;
    case ID_VEC4D_ARRAY:
        {
            osg::Vec4dArray* va = new osg::Vec4dArray;
            readArrayImplementation( va, 4, DOUBLE_SIZE );
            array = va;
        }
        break;

    case ID_VEC2I_ARRAY:
        {
            osg::Vec2iArray* va = new osg::Vec2iArray;
            readArrayImplementation( va, 2, INT_SIZE );
            array = va;
        }
        break;
    case ID_VEC3I_ARRAY:
        {
            osg::Vec3iArray* va = new osg::Vec3iArray;
            readArrayImplementation( va, 3, INT_SIZE );
            array = va;
        }
        break;
    case ID_VEC4I_ARRAY:
        {
            osg::Vec4iArray* va = new osg::Vec4iArray;
            readArrayImplementation( va, 4, INT_SIZE );
            array = va;
        }
        break;

    case ID_VEC2UI_ARRAY:
        {
            osg::Vec2uiArray* va = new osg::Vec2uiArray;
            readArrayImplementation( va, 2, INT_SIZE );
            array = va;
        }
        break;
    case ID_VEC3UI_ARRAY:
        {
            osg::Vec3uiArray* va = new osg::Vec3uiArray;
            readArrayImplementation( va, 3, INT_SIZE );
            array = va;
        }
        break;
    case ID_VEC4UI_ARRAY:
        {
            osg::Vec4uiArray* va = new osg::Vec4uiArray;
            readArrayImplementation( va, 4, INT_SIZE );
            array = va;
        }
        break;

    default:
        throwException( "InputStream::readArray(): Unsupported array type." );
    }

    if ( getException() ) return NULL;
    _arrayMap[id] = array;

    return array.release();
}

osg::PrimitiveSet* InputStream::readPrimitiveSet()
{
    osg::ref_ptr<osg::PrimitiveSet> primitive = NULL;

    DEF_MAPPEE(PrimitiveType, type);
    DEF_MAPPEE(PrimitiveType, mode);
    unsigned int numInstances = 0u;
    *this >> type >> mode;
    if ( _fileVersion>96 )
    {
        *this >> numInstances;
    }

    switch ( type.get() )
    {
    case ID_DRAWARRAYS:
        {
            int first = 0, count = 0;
            *this >> first >> count;
            osg::DrawArrays* da = new osg::DrawArrays( mode.get(), first, count );
            primitive = da;
            primitive->setNumInstances( numInstances );
        }
        break;
    case ID_DRAWARRAY_LENGTH:
        {
            int first = 0, value = 0; unsigned int size = 0;
            *this >> first >> size >> BEGIN_BRACKET;
            osg::DrawArrayLengths* dl = new osg::DrawArrayLengths( mode.get(), first );
            for ( unsigned int i=0; i<size; ++i )
            {
                *this >> value;
                dl->push_back( value );
            }
            *this >> END_BRACKET;
            primitive = dl;
            primitive->setNumInstances( numInstances );
        }
        break;
    case ID_DRAWELEMENTS_UBYTE:
        {
            osg::DrawElementsUByte* de = new osg::DrawElementsUByte( mode.get() );
            unsigned int size = 0; unsigned char value = 0;
            *this >> size >> BEGIN_BRACKET;
            for ( unsigned int i=0; i<size; ++i )
            {
                *this >> value;
                de->push_back( value );
            }
            *this >> END_BRACKET;
            primitive = de;
            primitive->setNumInstances( numInstances );
        }
        break;
    case ID_DRAWELEMENTS_USHORT:
        {
            osg::DrawElementsUShort* de = new osg::DrawElementsUShort( mode.get() );
            unsigned int size = 0; unsigned short value = 0;
            *this >> size >> BEGIN_BRACKET;
            for ( unsigned int i=0; i<size; ++i )
            {
                *this >> value;
                de->push_back( value );
            }
            *this >> END_BRACKET;
            primitive = de;
            primitive->setNumInstances( numInstances );
        }
        break;
    case ID_DRAWELEMENTS_UINT:
        {
            osg::DrawElementsUInt* de = new osg::DrawElementsUInt( mode.get() );
            unsigned int size = 0, value = 0;
            *this >> size >> BEGIN_BRACKET;
            for ( unsigned int i=0; i<size; ++i )
            {
                *this >> value;
                de->push_back( value );
            }
            *this >> END_BRACKET;
            primitive = de;
            primitive->setNumInstances( numInstances );
        }
        break;
    default:
        throwException( "InputStream::readPrimitiveSet(): Unsupported array type." );
    }

    if ( getException() ) return NULL;
    return primitive.release();
}

osg::Image* InputStream::readImage(bool readFromExternal)
{
    std::string className = "osg::Image";
    if ( _fileVersion>94 )  // ClassName property is only supported in 3.1.4 and higher
        *this >> PROPERTY("ClassName") >> className;

    unsigned int id = 0;
    *this >> PROPERTY("UniqueID") >> id;
    if ( getException() ) return NULL;

    IdentifierMap::iterator itr = _identifierMap.find( id );
    if ( itr!=_identifierMap.end() )
    {
        return static_cast<osg::Image*>( itr->second.get() );
    }

    std::string name;
    int writeHint, decision = IMAGE_EXTERNAL;
    *this >> PROPERTY("FileName"); readWrappedString(name);
    *this >> PROPERTY("WriteHint") >> writeHint >> decision;
    if ( getException() ) return NULL;

    osg::ref_ptr<osg::Image> image = NULL;
    switch ( decision )
    {
    case IMAGE_INLINE_DATA:
        if ( isBinary() )
        {
            // _origin, _s & _t & _r, _internalTextureFormat
            int origin, s, t, r, internalFormat;
            *this >> origin >> s >> t >> r >> internalFormat;

            // _pixelFormat, _dataType, _packing, _allocationMode
            int pixelFormat, dataType, packing, mode;
            *this >> pixelFormat >> dataType >> packing >> mode;

            // _data
            unsigned int size = 0; *this >> size;
            if ( size )
            {
                char* data = new char[size];
                if ( !data )
                    throwException( "InputStream::readImage() Out of memory." );
                if ( getException() ) return NULL;

                readCharArray( data, size );
                image = new osg::Image;
                image->setOrigin( (osg::Image::Origin)origin );
                image->setImage( s, t, r, internalFormat, pixelFormat, dataType,
                    (unsigned char*)data, osg::Image::USE_NEW_DELETE, packing );
            }

            // _mipmapData
            unsigned int levelSize = readSize();
            osg::Image::MipmapDataType levels(levelSize);
            for ( unsigned int i=0; i<levelSize; ++i )
            {
                *this >> levels[i];
            }
            if ( image && levelSize>0 )
                image->setMipmapLevels( levels );
            readFromExternal = false;
        } else { // ASCII
            // _origin, _s & _t & _r, _internalTextureFormat
            int origin, s, t, r, internalFormat;
            *this >> PROPERTY("Origin") >> origin;
            *this >> PROPERTY("Size") >> s >> t >> r;
            *this >> PROPERTY("InternalTextureFormat") >> internalFormat;

            // _pixelFormat, _dataType, _packing, _allocationMode
            int pixelFormat, dataType, packing, mode;
            *this >> PROPERTY("PixelFormat") >> pixelFormat;
            *this >> PROPERTY("DataType") >> dataType;
            *this >> PROPERTY("Packing") >> packing;
            *this >> PROPERTY("AllocationMode") >> mode;

            *this >> PROPERTY("Data");
            unsigned int levelSize = readSize()-1;
            *this >> BEGIN_BRACKET;

            // _data
            std::vector<std::string> encodedData;
            encodedData.resize(levelSize+1);
            readWrappedString(encodedData.at(0));

            // Read all mipmap levels and to also add them to char* data
            // _mipmapData
            osg::Image::MipmapDataType levels(levelSize);
            for ( unsigned int i=1; i<=levelSize; ++i )
            {
                //*this >> levels[i];
                readWrappedString(encodedData.at(i));
            }

            Base64decoder d;
            char* data = d.decode(encodedData, levels);
            // remove last item as we do not need the actual size
            // of the image including all mipmaps
            levels.pop_back();

            *this >> END_BRACKET;

            if ( !data )
                throwException( "InputStream::readImage() Decoding of stream failed. Out of memory." );
            if ( getException() ) return NULL;

            image = new osg::Image;
            image->setOrigin( (osg::Image::Origin)origin );
            image->setImage( s, t, r, internalFormat, pixelFormat, dataType,
                (unsigned char*)data, (osg::Image::AllocationMode)mode, packing );

            // Level positions (size of mipmap data)
            // from actual size of mipmap data read before
            if ( image && levelSize>0 )
                image->setMipmapLevels( levels );

            readFromExternal = false;
        }
        break;
    case IMAGE_INLINE_FILE:
        if ( isBinary() )
        {
            unsigned int size = readSize();
            if ( size>0 )
            {
                char* data = new char[size];
                if ( !data )
                {
                    throwException( "InputStream::readImage(): Out of memory." );
                    if ( getException() ) return NULL;
                }
                readCharArray( data, size );

                std::string ext = osgDB::getFileExtension( name );
                osgDB::ReaderWriter* reader =
                    osgDB::Registry::instance()->getReaderWriterForExtension( ext );
                if ( reader )
                {
                    std::stringstream inputStream;
                    inputStream.write( data, size );

                    osgDB::ReaderWriter::ReadResult rr = reader->readImage( inputStream );
                    if ( rr.validImage() )
                        image = rr.takeImage();
                    else
                    {
                        OSG_WARN << "InputStream::readImage(): "
                                               << rr.message() << std::endl;
                    }
                }
                else
                {
                    OSG_WARN << "InputStream::readImage(): Unable to find a plugin for "
                                           << ext << std::endl;
                }
                delete[] data;
            }
            readFromExternal = false;
        }
        break;
    case IMAGE_EXTERNAL: case IMAGE_WRITE_OUT:
        break;
    default:
        break;
    }

    bool loadedFromCache = false;
    if ( readFromExternal && !name.empty() )
    {
        ReaderWriter::ReadResult rr = Registry::instance()->readImage(name, getOptions());
        if (rr.validImage())
        {
            image = rr.takeImage();
            loadedFromCache = rr.loadedFromCache();
        }
        else
        {
            if (rr.error()) OSG_WARN << rr.message() << std::endl;
        }

        if ( !image && _forceReadingImage ) image = new osg::Image;
    }

    if (loadedFromCache)
    {
        // we don't want to overwrite the properties of the image in the cache as this could cause theading problems if the object is currently being used
        // so we read the properties from the file into a dummy object and discard the changes.
        osg::ref_ptr<osg::Object> temp_obj = readObjectFields("osg::Object", id, _dummyReadObject.get() );
        _identifierMap[id] = image;
    }
    else
    {
        image = static_cast<osg::Image*>( readObjectFields("osg::Object", id, image.get()) );
        if ( image.valid() )
        {
            image->setFileName( name );
            image->setWriteHint( (osg::Image::WriteHint)writeHint );
        }
    }
    return image.release();
}

osg::Object* InputStream::readObject( osg::Object* existingObj )
{
    std::string className;
    unsigned int id = 0;
    *this >> className;

    if (className=="NULL")
    {
        return 0;
    }

    *this >> BEGIN_BRACKET >> PROPERTY("UniqueID") >> id;
    if ( getException() ) return NULL;

    IdentifierMap::iterator itr = _identifierMap.find( id );
    if ( itr!=_identifierMap.end() )
    {
        advanceToCurrentEndBracket();
        return itr->second.get();
    }

    osg::ref_ptr<osg::Object> obj = readObjectFields( className, id, existingObj );

    advanceToCurrentEndBracket();

    return obj.release();
}

osg::Object* InputStream::readObjectFields( const std::string& className, unsigned int id, osg::Object* existingObj )
{
    ObjectWrapper* wrapper = Registry::instance()->getObjectWrapperManager()->findWrapper( className );
    if ( !wrapper )
    {
        OSG_WARN << "InputStream::readObject(): Unsupported wrapper class "
                               << className << std::endl;
        return NULL;
    }

    osg::ref_ptr<osg::Object> obj = existingObj ? existingObj : wrapper->createInstance();
    _identifierMap[id] = obj;
    if ( obj.valid() )
    {
        const StringList& associates = wrapper->getAssociates();
        for ( StringList::const_iterator itr=associates.begin(); itr!=associates.end(); ++itr )
        {
            ObjectWrapper* assocWrapper = Registry::instance()->getObjectWrapperManager()->findWrapper(*itr);
            if ( !assocWrapper )
            {
                OSG_WARN << "InputStream::readObject(): Unsupported associated class "
                                       << *itr << std::endl;
                continue;
            }
            _fields.push_back( assocWrapper->getName() );
            assocWrapper->read( *this, *obj );
            if ( getException() ) return NULL;

            _fields.pop_back();
        }
    }
    return obj.release();
}

void InputStream::readSchema( std::istream& fin )
{
    // Read from external ascii stream
    std::string line;
    while ( std::getline(fin, line) )
    {
        if ( line[0]=='#' ) continue;  // Comment

        StringList keyAndValue;
        split( line, keyAndValue, '=' );
        if ( keyAndValue.size()<2 ) continue;

        setWrapperSchema( osgDB::trimEnclosingSpaces(keyAndValue[0]),
                          osgDB::trimEnclosingSpaces(keyAndValue[1]) );
    }
}

InputStream::ReadType InputStream::start( InputIterator* inIterator )
{
    _fields.clear();
    _fields.push_back( "Start" );

    ReadType type = READ_UNKNOWN;
    _in = inIterator;
    if ( !_in )
        throwException( "InputStream: Null stream specified." );
    if ( getException() ) return type;

    _in->setInputStream(this);

    // Check OSG header information
    unsigned int version = 0;
    if ( isBinary() )
    {
        unsigned int typeValue;
        *this >> typeValue >> version;
        type = static_cast<ReadType>(typeValue);

        unsigned int attributes; *this >> attributes;
        if ( attributes&0x4 ) inIterator->setSupportBinaryBrackets( true );
        if ( attributes&0x2 ) _useSchemaData = true;

        // Record custom domains
        if ( attributes&0x1 )
        {
            unsigned int numDomains; *this >> numDomains;
            for ( unsigned int i=0; i<numDomains; ++i )
            {
                std::string domainName; *this >> domainName;
                int domainVersion; *this >> domainVersion;
                _domainVersionMap[domainName] = domainVersion;
            }
        }
    }
    if ( !isBinary() )
    {
        std::string typeString; *this >> typeString;
        if ( typeString=="Scene" ) type = READ_SCENE;
        else if ( typeString=="Image" ) type = READ_IMAGE;
        else if ( typeString=="Object" ) type = READ_OBJECT;

        std::string osgName, osgVersion;
        *this >> PROPERTY("#Version") >> version;
        *this >> PROPERTY("#Generator") >> osgName >> osgVersion;

        while ( matchString("#CustomDomain") )
        {
            std::string domainName; *this >> domainName;
            int domainVersion; *this >> domainVersion;
            _domainVersionMap[domainName] = domainVersion;
        }
    }

    // Record file version for back-compatibility checking of wrappers
    _fileVersion = version;
    _fields.pop_back();
    return type;
}

void InputStream::decompress()
{
    if ( !isBinary() ) return;
    _fields.clear();

    std::string compressorName; *this >> compressorName;
    if ( compressorName!="0" )
    {
        std::string data;
        _fields.push_back( "Decompression" );

        BaseCompressor* compressor = Registry::instance()->getObjectWrapperManager()->findCompressor(compressorName);
        if ( !compressor )
        {
            OSG_WARN << "InputStream::decompress(): No such compressor "
                                   << compressorName << std::endl;
        }

        if ( !compressor->decompress(*(_in->getStream()), data) )
            throwException( "InputStream: Failed to decompress stream." );
        if ( getException() ) return;

        _dataDecompress = new std::stringstream(data);
        _in->setStream( _dataDecompress );
        _fields.pop_back();
    }

    if ( _useSchemaData )
    {
        _fields.push_back( "SchemaData" );
        std::string schemaSource; *this >> schemaSource;
        std::istringstream iss( schemaSource );
        readSchema( iss );
        _fields.pop_back();
    }
}

// PROTECTED METHODS

void InputStream::setWrapperSchema( const std::string& name, const std::string& properties )
{
    ObjectWrapper* wrapper = Registry::instance()->getObjectWrapperManager()->findWrapper(name);
    if ( !wrapper )
    {
        OSG_WARN << "InputStream::setSchema(): Unsupported wrapper class "
                               << name << std::endl;
        return;
    }

    StringList schema, methods, keyAndValue;
    ObjectWrapper::TypeList types;
    split( properties, schema );
    for ( StringList::iterator itr=schema.begin(); itr!=schema.end(); ++itr )
    {
        split( *itr, keyAndValue, ':' );
        if ( keyAndValue.size()>1 )
        {
            methods.push_back( keyAndValue.front() );
            types.push_back( static_cast<BaseSerializer::Type>(atoi(keyAndValue.back().c_str())) );
        }
        else
        {
            methods.push_back( *itr );
            types.push_back( BaseSerializer::RW_UNDEFINED );
        }
        keyAndValue.clear();
    }
    wrapper->readSchema( methods, types );
}

void InputStream::resetSchema()
{
    const ObjectWrapperManager::WrapperMap& wrappers = Registry::instance()->getObjectWrapperManager()->getWrapperMap();
    for ( ObjectWrapperManager::WrapperMap::const_iterator itr=wrappers.begin();
          itr!=wrappers.end(); ++itr )
    {
        ObjectWrapper* wrapper = itr->second.get();
        wrapper->resetSchema();
    }
}

template<typename T>
void InputStream::readArrayImplementation( T* a, unsigned int numComponentsPerElements, unsigned int componentSizeInBytes )
{
    int size = 0;
    *this >> size >> BEGIN_BRACKET;
    if ( size )
    {
        a->resize( size );
        if ( isBinary() )
        {
            readComponentArray( (char*)&((*a)[0]), size, numComponentsPerElements, componentSizeInBytes );
            checkStream();
        }
        else
        {
            for ( int i=0; i<size; ++i )
                *this >> (*a)[i];
        }
    }
    *this >> END_BRACKET;
}
