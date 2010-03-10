#ifndef OSG2_BINARYSTREAMOPERATOR
#define OSG2_BINARYSTREAMOPERATOR

#include <osgDB/StreamOperator>

class BinaryOutputIterator : public osgDB::OutputIterator
{
public:
    BinaryOutputIterator( std::ostream* ostream ) { _out = ostream; }
    virtual ~BinaryOutputIterator() {}
    
    virtual bool isBinary() const { return true; }
    
    virtual void writeBool( bool b )
    { char c = b?1:0; _out->write( &c, CHAR_SIZE ); }
    
    virtual void writeChar( char c )
    { _out->write( &c, CHAR_SIZE ); }
    
    virtual void writeUChar( unsigned char c )
    { _out->write( (char*)&c, CHAR_SIZE ); }
    
    virtual void writeShort( short s )
    { _out->write( (char*)&s, SHORT_SIZE ); }
    
    virtual void writeUShort( unsigned short s )
    { _out->write( (char*)&s, SHORT_SIZE ); }
    
    virtual void writeInt( int i )
    { _out->write( (char*)&i, INT_SIZE ); }
    
    virtual void writeUInt( unsigned int i )
    { _out->write( (char*)&i, INT_SIZE ); }
    
    virtual void writeLong( long l )
    { _out->write( (char*)&l, LONG_SIZE ); }
    
    virtual void writeULong( unsigned long l )
    { _out->write( (char*)&l, LONG_SIZE ); }
    
    virtual void writeFloat( float f )
    { _out->write( (char*)&f, FLOAT_SIZE ); }
    
    virtual void writeDouble( double d )
    { _out->write((char*)&d, DOUBLE_SIZE); }
    
    virtual void writeString( const std::string& s )
    {
        int size = s.size();
        _out->write( (char*)&size, INT_SIZE );
        _out->write( s.c_str(), s.size() );
    }
    
    virtual void writeStream( std::ostream& (*fn)(std::ostream&) ) {}
    
    virtual void writeBase( std::ios_base& (*fn)(std::ios_base&) ) {}
    
    virtual void writeGLenum( const osgDB::ObjectGLenum& value )
    { GLenum e = value.get(); _out->write((char*)&e, GLENUM_SIZE); }
    
    virtual void writeProperty( const osgDB::ObjectProperty& prop )
    { if (prop._mapProperty) _out->write((char*)&(prop._value), INT_SIZE); }
    
    virtual void writeMark( const osgDB::ObjectMark& mark ) {}
    
    virtual void writeCharArray( const char* s, unsigned int size )
    { if ( size>0 ) _out->write( s, size ); }
    
    virtual void writeWrappedString( const std::string& str )
    { writeString( str ); }
};

class BinaryInputIterator : public osgDB::InputIterator
{
public:
    BinaryInputIterator( std::istream* istream ) : _byteSwap(0) { _in = istream; }
    virtual ~BinaryInputIterator() {}
    
    virtual bool isBinary() const { return true; }
    
    virtual void readBool( bool& b )
    {
        char c = 0;
        _in->read( &c, CHAR_SIZE );
        b = (c!=0);
    }
    
    virtual void readChar( char& c )
    { _in->read( &c, CHAR_SIZE ); }
    
    virtual void readSChar( signed char& c )
    { _in->read( (char*)&c, CHAR_SIZE ); }
    
    virtual void readUChar( unsigned char& c )
    { _in->read( (char*)&c, CHAR_SIZE ); }
    
    virtual void readShort( short& s )
    {
        _in->read( (char*)&s, SHORT_SIZE );
        if ( _byteSwap ) osg::swapBytes( (char*)&s, SHORT_SIZE );
    }
    
    virtual void readUShort( unsigned short& s )
    {
        _in->read( (char*)&s, SHORT_SIZE );
        if ( _byteSwap ) osg::swapBytes( (char*)&s, SHORT_SIZE );
    }
    
    virtual void readInt( int& i )
    {
        _in->read( (char*)&i, INT_SIZE );
        if ( _byteSwap ) osg::swapBytes( (char*)&i, INT_SIZE );
    }
    
    virtual void readUInt( unsigned int& i )
    {
        _in->read( (char*)&i, INT_SIZE );
        if ( _byteSwap ) osg::swapBytes( (char*)&i, INT_SIZE );
    }
    
    virtual void readLong( long& l )
    {
        _in->read( (char*)&l, LONG_SIZE );
        if ( _byteSwap ) osg::swapBytes( (char*)&l, LONG_SIZE );
    }
    
    virtual void readULong( unsigned long& l )
    {
        _in->read( (char*)&l, LONG_SIZE );
        if ( _byteSwap ) osg::swapBytes( (char*)&l, LONG_SIZE );
    }
    
    virtual void readFloat( float& f )
    {
        _in->read( (char*)&f, FLOAT_SIZE );
        if ( _byteSwap ) osg::swapBytes( (char*)&f, FLOAT_SIZE );
    }
    
    virtual void readDouble( double& d )
    {
        _in->read( (char*)&d, DOUBLE_SIZE );
        if ( _byteSwap ) osg::swapBytes( (char*)&d, DOUBLE_SIZE );
    }
    
    virtual void readString( std::string& s )
    {
        int size = 0; readInt( size );
        if ( size )
        {
            s.resize( size );
            _in->read( (char*)s.c_str(), size );
        }
    }
    
    virtual void readStream( std::istream& (*fn)(std::istream&) ) {}
    
    virtual void readBase( std::ios_base& (*fn)(std::ios_base&) ) {}
    
    virtual void readGLenum( osgDB::ObjectGLenum& value )
    {
        GLenum e = 0;
        _in->read( (char*)&e, GLENUM_SIZE );
        if ( _byteSwap ) osg::swapBytes( (char*)&e, GLENUM_SIZE );
        value.set( e );
    }
    
    virtual void readProperty( osgDB::ObjectProperty& prop )
    {
        int value = 0;
        if ( prop._mapProperty )
        {
            _in->read( (char*)&value, INT_SIZE );
            if ( _byteSwap ) osg::swapBytes( (char*)&value, INT_SIZE );
        }
        prop.set( value );
    }
    
    virtual void readMark( osgDB::ObjectMark& mark ) {}
    
    virtual void readCharArray( char* s, unsigned int size )
    { if ( size>0 ) _in->read( s, size ); }
    
    virtual void readWrappedString( std::string& str )
    { readString( str ); }
    
protected:
    int _byteSwap;
};

#endif
