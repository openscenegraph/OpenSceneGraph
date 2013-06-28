#ifndef OSGDB_XMLSTREAMOPERATOR
#define OSGDB_XMLSTREAMOPERATOR

#include <osgDB/StreamOperator>
#include <osgDB/XmlParser>
#include <sstream>

class XmlOutputIterator : public osgDB::OutputIterator
{
public:
    enum ReadLineType
    {
        FIRST_LINE=0,        // The first line of file
        NEW_LINE,            // A new line without checking its type
        PROP_LINE,           // A line starting with osgDB::PROPERTY
        SUB_PROP_LINE,       // A property line containing another osgDB::PROPERTY
        BEGIN_BRACKET_LINE,  // A line ending with a '{'
        END_BRACKET_LINE,    // A line starting with a '}'
        TEXT_LINE            // A text line, e.g. recording array elements
    };

    XmlOutputIterator( std::ostream* ostream, int precision )
    :   _readLineType(FIRST_LINE), _prevReadLineType(FIRST_LINE), _hasSubProperty(false)
    {
        _out = ostream;
        if (precision>0) _sstream.precision(precision);
        _root = new osgDB::XmlNode;
        _root->type = osgDB::XmlNode::GROUP;
    }

    virtual ~XmlOutputIterator() {}

    virtual bool isBinary() const { return false; }

    virtual void writeBool( bool b )
    { addToCurrentNode( b ? std::string("TRUE") : std::string("FALSE") ); }

    virtual void writeChar( char c )
    { _sstream << (short)c; addToCurrentNode( _sstream.str() ); _sstream.str(""); }

    virtual void writeUChar( unsigned char c )
    { _sstream << (unsigned short)c; addToCurrentNode( _sstream.str() ); _sstream.str(""); }

    virtual void writeShort( short s )
    { _sstream << s; addToCurrentNode( _sstream.str() ); _sstream.str(""); }

    virtual void writeUShort( unsigned short s )
    { _sstream << s; addToCurrentNode( _sstream.str() ); _sstream.str(""); }

    virtual void writeInt( int i )
    { _sstream << i; addToCurrentNode( _sstream.str() ); _sstream.str(""); }

    virtual void writeUInt( unsigned int i )
    { _sstream << i; addToCurrentNode( _sstream.str() ); _sstream.str(""); }

    virtual void writeLong( long l )
    { _sstream << l; addToCurrentNode( _sstream.str() ); _sstream.str(""); }

    virtual void writeULong( unsigned long l )
    { _sstream << l; addToCurrentNode( _sstream.str() ); _sstream.str(""); }

    virtual void writeFloat( float f )
    { _sstream << f; addToCurrentNode( _sstream.str() ); _sstream.str(""); }

    virtual void writeDouble( double d )
    { _sstream << d; addToCurrentNode( _sstream.str() ); _sstream.str(""); }

    virtual void writeString( const std::string& s )
    { addToCurrentNode( s, true ); }

    virtual void writeStream( std::ostream& (*fn)(std::ostream&) )
    {
        if ( isEndl( fn ) )
        {
            if ( _readLineType==PROP_LINE || _readLineType==END_BRACKET_LINE )
            {
                if ( _hasSubProperty )
                {
                    _hasSubProperty = false;
                    popNode();  // Exit the sub-property element
                }
                popNode();  // Exit the property element
            }
            else if ( _readLineType==SUB_PROP_LINE )
            {
                _hasSubProperty = false;
                popNode();  // Exit the sub-property element
                popNode();  // Exit the property element
            }
            else if ( _readLineType==TEXT_LINE )
                addToCurrentNode( fn );

            setLineType( NEW_LINE );
        }
        else
            addToCurrentNode( fn );
    }

    virtual void writeBase( std::ios_base& (*fn)(std::ios_base&) )
    {
        _sstream << fn;
    }

    virtual void writeGLenum( const osgDB::ObjectGLenum& value )
    {
        GLenum e = value.get();
        const std::string& enumString = osgDB::Registry::instance()->getObjectWrapperManager()->getString("GL", e);
        addToCurrentNode( enumString, true );
    }

    virtual void writeProperty( const osgDB::ObjectProperty& prop )
    {
        std::string enumString = prop._name;
        if ( prop._mapProperty )
        {
            enumString = osgDB::Registry::instance()->getObjectWrapperManager()->getString(prop._name, prop._value);
            addToCurrentNode( enumString, true );
        }
        else
        {
            if ( _readLineType==NEW_LINE || _readLineType==BEGIN_BRACKET_LINE )
            {
                pushNode( enumString );
                setLineType( PROP_LINE );
            }
            else if ( _readLineType==PROP_LINE )
            {
                pushNode( enumString );
                setLineType( SUB_PROP_LINE );
                _hasSubProperty = true;
            }
            else if ( _readLineType==SUB_PROP_LINE )
            {
                popNode();
                pushNode( enumString );
            }
        }
    }

    virtual void writeMark( const osgDB::ObjectMark& mark )
    {
        int delta = mark._indentDelta;
        if ( delta>0 )
        {
            setLineType( BEGIN_BRACKET_LINE );
        }
        else if ( delta<0 )
        {
            setLineType( END_BRACKET_LINE );
        }
    }

    virtual void writeCharArray( const char* /*s*/, unsigned int /*size*/ ) {}

    virtual void writeWrappedString( const std::string& str )
    {
        std::string realStr;
        for ( std::string::const_iterator itr=str.begin(); itr!=str.end(); ++itr )
        {
            char ch = *itr;
            if ( ch=='\"' ) realStr += '\\';
            else if ( ch=='\\' ) realStr += '\\';
            realStr += ch;
        }
        realStr.insert( std::string::size_type(0), 1, '\"' );
        realStr += '\"';
        addToCurrentNode( realStr );
    }

    virtual void flush()
    {
        osg::ref_ptr<osgDB::XmlNode> xmlRoot = new osgDB::XmlNode;
        xmlRoot->type = osgDB::XmlNode::ROOT;
        xmlRoot->children.push_back( _root.get() );
        xmlRoot->write( *_out );
    }

protected:
    void addToCurrentNode( const std::string& str, bool isString=false )
    {
        if ( _readLineType==FIRST_LINE )
        {
            _root->name = str;
            return;
        }

        if ( _readLineType==NEW_LINE )
        {
            if ( isString )
            {
                pushNode( str );
                setLineType( PROP_LINE );
                return;
            }
            else
                setLineType( TEXT_LINE );
        }

        if ( _readLineType==TEXT_LINE )
        {
            std::string& text = _nodePath.back()->properties["text"];
            text += str + ' ';
        }
        else if ( _nodePath.size()>0 )
        {
            std::string& prop = _nodePath.back()->properties["attribute"];
            if ( !prop.empty() ) prop += ' ';
            prop += str;
        }
        else
        {
            pushNode( str );
            setLineType( PROP_LINE );
        }
    }

    void addToCurrentNode( std::ostream& (*fn)(std::ostream&) )
    {
        if ( _nodePath.size()>0 )
        {
            osgDB::XmlNode* node = _nodePath.back();
            _sstream << fn;
            if ( _readLineType==TEXT_LINE ) node->properties["text"] += _sstream.str();
            else node->properties["attribute"] += _sstream.str();
            _sstream.str("");
        }
    }

    osgDB::XmlNode* pushNode( const std::string& name )
    {
        osg::ref_ptr<osgDB::XmlNode> node = new osgDB::XmlNode;
        node->type = osgDB::XmlNode::ATOM;

        // Set element name without '#' and '::' characters
        std::string realName;
        if ( name.length()>0 && name[0]=='#' )
            realName = name.substr(1);
        else
        {
            realName = name;

            std::string::size_type pos = realName.find("::");
            if ( pos!=std::string::npos )
                realName.replace( pos, 2, "--" );
        }
        node->name = realName;

        if ( _nodePath.size()>0 )
        {
            _nodePath.back()->type = osgDB::XmlNode::GROUP;
            _nodePath.back()->children.push_back(node);
        }
        else
            _root->children.push_back(node);

        _nodePath.push_back( node.get() );
        return node.get();
    }

    osgDB::XmlNode* popNode()
    {
        osgDB::XmlNode* node = NULL;
        if ( _nodePath.size()>0 )
        {
            node = _nodePath.back();
            trimEndMarkers( node, "attribute" );
            trimEndMarkers( node, "text" );
            _nodePath.pop_back();
        }
        return node;
    }

    void trimEndMarkers( osgDB::XmlNode* node, const std::string& name )
    {
        osgDB::XmlNode::Properties::iterator itr = node->properties.find(name);
        if ( itr==node->properties.end() ) return;

        std::string& str = itr->second;
        if ( !str.empty() )
        {
            std::string::size_type end = str.find_last_not_of( " \t\r\n" );
            if ( end==std::string::npos ) return;
            str.erase( end+1 );
        }

        if ( str.empty() )
            node->properties.erase(itr);
    }

    void setLineType( ReadLineType type )
    {
        _prevReadLineType = _readLineType;
        _readLineType = type;
    }

    typedef std::vector<osgDB::XmlNode*> XmlNodePath;
    XmlNodePath _nodePath;

    osg::ref_ptr<osgDB::XmlNode> _root;
    std::stringstream _sstream;

    ReadLineType _readLineType;
    ReadLineType _prevReadLineType;
    bool _hasSubProperty;
};

class XmlInputIterator : public osgDB::InputIterator
{
public:
    XmlInputIterator( std::istream* istream )
    {
        _in = istream;
        _root = osgDB::readXmlStream( *istream );

        if ( _root.valid() && _root->children.size()>0 )
            _nodePath.push_back( _root->children[0] );
    }

    virtual ~XmlInputIterator() {}

    virtual bool isBinary() const { return false; }

    virtual void readBool( bool& b )
    {
        std::string boolString;
        if ( prepareStream() ) _sstream >> boolString;
        if ( boolString=="TRUE" ) b = true;
        else b = false;
    }

    virtual void readChar( char& c )
    {
        short s = 0;
        if ( prepareStream() ) _sstream >> s;
        c = (char)s;
    }

    virtual void readSChar( signed char& c )
    {
        short s = 0;
        if ( prepareStream() ) _sstream >> s;
        c = (signed char)s;
    }

    virtual void readUChar( unsigned char& c )
    {
        unsigned short s = 0;
        if ( prepareStream() ) _sstream >> s;
        c = (unsigned char)s;
    }

    virtual void readShort( short& s )
    { std::string str; if (prepareStream()) _sstream >> str; s = static_cast<short>(strtol(str.c_str(), NULL, 0)); }

    virtual void readUShort( unsigned short& s )
    { std::string str; if (prepareStream()) _sstream >> str; s = static_cast<unsigned short>(strtoul(str.c_str(), NULL, 0)); }

    virtual void readInt( int& i )
    { std::string str; if (prepareStream()) _sstream >> str; i = static_cast<int>(strtol(str.c_str(), NULL, 0)); }

    virtual void readUInt( unsigned int& i )
    { std::string str; if (prepareStream()) _sstream >> str; i = static_cast<unsigned int>(strtoul(str.c_str(), NULL, 0)); }

    virtual void readLong( long& l )
    { std::string str; if (prepareStream()) _sstream >> str; l = strtol(str.c_str(), NULL, 0); }

    virtual void readULong( unsigned long& l )
    { std::string str; if (prepareStream()) _sstream >> str; l = strtoul(str.c_str(), NULL, 0); }

    virtual void readFloat( float& f )
    { std::string str; if (prepareStream()) _sstream >> str; f = osg::asciiToFloat(str.c_str()); }

    virtual void readDouble( double& d )
    { std::string str; if (prepareStream()) _sstream >> str; d = osg::asciiToDouble(str.c_str()); }

    virtual void readString( std::string& s )
    {
        if ( prepareStream() ) _sstream >> s;

        // Replace '--' to '::' to get correct wrapper class
        std::string::size_type pos = s.find("--");
        if ( pos!=std::string::npos )
            s.replace( pos, 2, "::" );
    }

    virtual void readStream( std::istream& (*fn)(std::istream&) )
    { if ( prepareStream() ) _sstream >> fn; }

    virtual void readBase( std::ios_base& (*fn)(std::ios_base&) )
    { _sstream >> fn; }

    virtual void readGLenum( osgDB::ObjectGLenum& value )
    {
        GLenum e = 0;
        std::string enumString;
        if ( prepareStream() ) _sstream >> enumString;
        e = osgDB::Registry::instance()->getObjectWrapperManager()->getValue("GL", enumString);
        value.set( e );
    }

    virtual void readProperty( osgDB::ObjectProperty& prop )
    {
        int value = 0;
        std::string enumString;
        if ( prepareStream() ) _sstream >> enumString;
        if ( prop._mapProperty )
        {
            value = osgDB::Registry::instance()->getObjectWrapperManager()->getValue(prop._name, enumString);
        }
        else
        {
            // Replace '--' to '::' to get correct wrapper class
            std::string::size_type pos = enumString.find("--");
            if ( pos!=std::string::npos )
                enumString.replace( pos, 2, "::" );

            if ( prop._name!=enumString )
            {
                if ( prop._name[0]=='#' )
                    enumString = '#' + enumString;
                if ( prop._name!=enumString )
                {
                    OSG_WARN << "XmlInputIterator::readProperty(): Unmatched property "
                                           << enumString << ", expecting " << prop._name << std::endl;
                }
            }
            prop._name = enumString;
        }
        prop.set( value );
    }

    virtual void readMark( osgDB::ObjectMark& /*mark*/ ) {}

    virtual void readCharArray( char* /*s*/, unsigned int /*size*/ ) {}

    virtual void readWrappedString( std::string& str )
    {
        if ( !prepareStream() ) return;

        // Read available string in the stream buffer
        unsigned int availSize = _sstream.rdbuf()->in_avail();
        std::string realStr = _sstream.str();
        _sstream.str("");

        // Find the first quot or valid character
        bool hasQuot = false;
        std::string::iterator itr = realStr.begin() + (realStr.size() - availSize);
        for ( ; itr!=realStr.end(); ++itr )
        {
            char ch = *itr;
            if ((ch==' ') || (ch=='\n') || (ch=='\r')) continue;
            else if (ch=='"') hasQuot = true;
            else str += ch;

            itr++;
            break;
        }

        for ( ; itr!=realStr.end(); ++itr )
        {
            char ch = *itr;
            if (ch=='\\')
            {
                itr++;
                if (itr == realStr.end()) break;
                str += *itr;
            }
            else if (hasQuot && ch=='"')
            {
                // Get to the end of the wrapped string
                itr++;
                break;
            }
            else
                str += ch;
        }
        if (itr != realStr.end())
        {
            _sstream << std::string(itr, realStr.end());
        }
    }

    virtual bool matchString( const std::string& str )
    {
        prepareStream();
        std::string strInStream = osgDB::trimEnclosingSpaces(_sstream.str());
        if ( strInStream==str )
        {
            std::string prop; readString( prop );
            return true;
        }
        return false;
    }

    virtual void advanceToCurrentEndBracket() {}

protected:
    bool isReadable() const { return _sstream.rdbuf()->in_avail()>0; }

    bool prepareStream()
    {
        if ( !_nodePath.size() ) return false;
        if ( isReadable() ) return true;
        _sstream.clear();

        osgDB::XmlNode* current = _nodePath.back().get();
        if ( current->type!=osgDB::XmlNode::COMMENT )
        {
            if ( !current->name.empty() )
            {
                _sstream.str( current->name );
                current->name.clear();
                return true;
            }

            if ( current->properties.size()>0 )
            {
                if ( applyPropertyToStream(current, "attribute") ) return true;
                else if ( applyPropertyToStream(current, "text") ) return true;
            }

            if ( current->children.size()>0 )
            {
                _nodePath.push_back( current->children.front() );
                current->children.erase( current->children.begin() );
                return prepareStream();
            }
        }
        _nodePath.pop_back();
        return prepareStream();
    }

    bool applyPropertyToStream( osgDB::XmlNode* node, const std::string& name )
    {
        osgDB::XmlNode::Properties::iterator itr = node->properties.find(name);
        if ( itr!=node->properties.end() )
        {
            _sstream.str( itr->second );
            node->properties.erase( itr );
            return true;
        }
        return false;
    }

    typedef std::vector< osg::ref_ptr<osgDB::XmlNode> > XmlNodePath;
    XmlNodePath _nodePath;

    osg::ref_ptr<osgDB::XmlNode> _root;
    std::stringstream _sstream;
};

#endif
