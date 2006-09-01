#include <osgText/String>

#include <osg/Notify>
#include <osg/Math>

using namespace osgText;

////////////////////////////////////////////////////////////////////////
//
// helper class to make it safer to querry std::string's for encoding.
//
struct look_ahead_iterator
{
    look_ahead_iterator(const std::string& string):
        _string(string),
        _index(0),
        _nullCharacter(0) {}
        
    bool valid() const { return _index<_string.length(); }
    
    look_ahead_iterator& operator ++ ()
    {
        if (_index<_string.length()) ++_index; 
        return *this;
    }
    
    look_ahead_iterator operator ++ (int)
    {
        look_ahead_iterator tmp(*this); 
        if (_index<_string.length()) ++_index; 
        return tmp;
    }

    look_ahead_iterator& operator += (int offset)
    {
        if (_index<_string.length()) _index = osg::minimum((unsigned int)(_index+offset),(unsigned int)_string.length());
        return *this;
    }
    
    unsigned char operator * () const
    {
        if (_index<_string.length()) return _string[_index];
        else return _nullCharacter;
    }

    unsigned char operator [] (unsigned int offset) const
    {

        if (_index+offset<_string.length()) return _string[_index+offset];
        else return _nullCharacter;
    }
    

    const std::string&      _string;
    unsigned int            _index;
    unsigned char           _nullCharacter;
};

String::Encoding findEncoding(look_ahead_iterator& charString,String::Encoding overrideEncoding)
{
    switch (charString[0])
    {
        case 0xEF: // 8-bit encoding
        {
            // 8-bit signature = EF BB BF
            if ((charString[1]==0xBB) && (charString[2]==0xBF))
            {
                charString+=3;
                return String::ENCODING_UTF8;
            }
            break;
        }
        case 0xFE: // big-endian 16-bit
        {
            // 16-bit signature = FE FF
            if (charString[1]==0xFF)
            {
                charString+=2;
                return String::ENCODING_UTF16_BE;
            }
            break;
        }
        case 0xFF: // little-endian
        {
            // 16-bit signature = FF FE
            // 32-bit signature = FF FE 00 00
            if (charString[1]==0xFE)
            {
                // NOTE: There is an a potential problem as a 16-bit empty string
                // is identical to a 32-bit start signature
                if ((charString[2]==0) && (charString[3]==0) && (overrideEncoding != String::ENCODING_UTF16)) //32-bit
                {
                    charString+=4;
                    return String::ENCODING_UTF32_LE;
                }
                else //16-bit
                {
                    charString+=2;
                    return String::ENCODING_UTF16_LE;
                }
            }
            break;
        }
        case 0x00: // 32-bit big-endian
        {
            // 32-bit signature = 00 00 FE FF
            if ((charString[1]==0x00) && (charString[2]==0xFE) && (charString[3]==0xFF))
            {
                charString+=4;
                return String::ENCODING_UTF32_BE;
            }
            break;
        }
    }
    return String::ENCODING_ASCII;
}


unsigned int getNextCharacter(look_ahead_iterator& charString,String::Encoding encoding)
{
    // For more info on unicode encodings see: 
    // http://www-106.ibm.com/developerworks/unicode/library/u-encode.html
    switch(encoding)
    {
        case String::ENCODING_ASCII:
        {
            return *charString++;
        }
        case String::ENCODING_UTF8:
        {
            int char0 = *charString++;
            if (char0 < 0x80) // 1-byte character
            {
                return char0;
            }
            int char1 = *charString++;
            if (char0<0xe0) // 2-byte character
            {
                return ((char0&0x1f)<<6) | (char1&0x3f);
            }
            int char2 = *charString++;
            if (char0<0xf0) // 3-byte character
            {
                return ((char0&0xf)<<12) | ((char1&0x3f)<<6) | (char2&0x3f);
            }
            int char3 = *charString++;
            if (char0<0xf8) // 4-byte character
            {
                return ((char0&0x7)<<18) | ((char1&0x3f)<<12) | ((char2&0x3f)<<6) | (char3&0x3f);
            }
            break;
        }
        case String::ENCODING_UTF16_BE:
        {
            int char0 = *charString++;
            int char1 = *charString++;
            if ((char0<=0xD7) || (char0>=0xE0)) // simple character
            {
                return (char0<<8) | char1;
            }
            else if ((char0>=0xD8)&&(char0<=0xDB)) //using planes (this should get called very rarely)
            {
                int char2 = *charString++;
                int char3 = *charString++;
                int highSurrogate = (char0<<8) | char1; 
                int lowSurrogate = (char2<<8) | char3;
                if ((char2>=0xDC)&&(char2<=0xDF)) //only for the valid range of low surrogate
                {
                    // This covers the range of all 17 unicode planes
                    return ((highSurrogate-0xD800)*0x400) + (lowSurrogate-0xD800) + 0x10000;
                }
            }
            break;
        }
        case String::ENCODING_UTF16_LE:
        {
            int char1 = *charString++;
            int char0 = *charString++;
            if ((char0<=0xD7) || (char0>=0xE0)) // simple character
            {
                return (char0<<8) | char1;
            }
            else if ((char0>=0xD8)&&(char0<=0xDB)) //using planes (this should get called very rarely)
            {
                int char3 = *charString++;
                int char2 = *charString++;
                int highSurrogate = (char0<<8) | char1; 
                int lowSurrogate = (char2<<8) | char3;
                if ((char2>=0xDC)&&(char2<=0xDF)) //only for the valid range of low surrogate
                {
                    // This covers the range of all 17 unicode planes
                    return ((highSurrogate-0xD800)*0x400) + (lowSurrogate-0xD800) + 0x10000;
                }
            }
            break;
        }
        case String::ENCODING_UTF32_BE:
        {
            int character = ((((int)charString[0])<<24) | (((int)charString[1])<<16) |
                            (((int)charString[2])<<8) | charString[3]);
            charString+=4;
            if (character<0x110000) 
            { 
                // Character is constrained to the range set by the unicode standard 
                return character;
            }
            break;
        }
        case String::ENCODING_UTF32_LE:
        {
            int character = ((((int)charString[3])<<24) | (((int)charString[2])<<16) |
                            (((int)charString[1])<<8) | charString[0]);
            charString+=4;
            if (character<0x110000) 
            { 
                // Character is constrained to the range set by the unicode standard 
                return character;
            }
            break;
        }
        default:
        {
            // Should not reach this point unless the encoding is unhandled
            // ENCODING_UTF16, ENCODING_UTF32 and ENCODING_SIGNATURE should never enter this method
            osg::notify(osg::FATAL)<<"Error: Invalid string encoding"<<std::endl;    
            break;
        }
    }
    return 0;
}


////////////////////////////////////////////////////////////////////////////
//
// String implemention.
//

String::String(const String& str):
    vector_type(str)
{
}

String& String::operator = (const String& str)
{
    if (&str==this) return *this;
    
    clear();
    std::copy(str.begin(),str.end(),std::back_inserter(*this));
    
    return *this;
}

void String::set(const std::string& text)
{
    clear();
    for ( std::string::const_iterator it = text.begin(); it != text.end(); ++it )
    {
        unsigned int charcode = (unsigned char )*it;
        push_back( charcode );
    }
}

void String::set(const wchar_t* text)
{
    clear();
    while(*text)
    {
        push_back(*text++);
    }
}

void String::set(const std::string& text,Encoding encoding)
{
    clear();

    look_ahead_iterator itr(text);

    if ((encoding == ENCODING_SIGNATURE) || 
        (encoding == ENCODING_UTF16) || 
        (encoding == ENCODING_UTF32))
    {
        encoding = findEncoding(itr,encoding);
    }
    
    while(itr.valid())
    {
        unsigned int c = getNextCharacter(itr,encoding);
        if (c) push_back(c);
    }
}

std::string String::createUTF8EncodedString() const
{
    std::string utf8string;
    for(const_iterator itr=begin();
        itr!=end();
        ++itr)
    {
        unsigned int currentChar = *itr;
        if (currentChar < 0x80)
        {
            utf8string+=(char)currentChar;
        }
        else if (currentChar < 0x800)
        {
            utf8string+=(char)(0xc0 | (currentChar>>6));
            utf8string+=(char)(0x80 | currentChar & 0x3f);
        }
        else
        {
            utf8string+=(char)(0xe0 | (currentChar>>12));
            utf8string+=(char)(0x80 | (currentChar>>6) & 0x3f);
            utf8string+=(char)(0x80 | currentChar & 0x3f);
        }
    }
    return utf8string;
}
