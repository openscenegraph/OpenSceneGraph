//C++ header - Open Scene Graph - Copyright (C) 1998-2002 Robert Osfield
//Distributed under the terms of the GNU Library General Public License (LGPL)
//as published by the Free Software Foundation.

/* --------------------------------------------------------------------------
 *
 *    openscenegraph textLib / FTGL wrapper (http://homepages.paradise.net.nz/henryj/code/)
 *
 * --------------------------------------------------------------------------
 *    
 *    prog:    max rheiner;mrn@paus.ch
 *    date:    4/25/2001    (m/d/y)
 *
 * --------------------------------------------------------------------------
 *
 * --------------------------------------------------------------------------
 */

#include <osgText/EncodedText>

#include <osg/Notify>

using namespace osgText;

EncodedText::EncodedText()
{
    _encoding = ENCODING_ASCII;
    _overrideEncoding = ENCODING_SIGNATURE;
}

int EncodedText::getNextCharacter(const unsigned char*& charString) const
{
    // For more info on unicode encodings see: 
    // http://www-106.ibm.com/developerworks/unicode/library/u-encode.html
    switch(_encoding)
    {
        case ENCODING_ASCII:
        {
            return *charString++;
        }
        case ENCODING_UTF8:
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
        case ENCODING_UTF16_BE:
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
        case ENCODING_UTF16_LE:
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
        case ENCODING_UTF32_BE:
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
        case ENCODING_UTF32_LE:
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

EncodedText::Encoding EncodedText::findEncoding(const unsigned char*& charString) const
{
    switch (charString[0])
    {
        case 0xEF: // 8-bit encoding
        {
            // 8-bit signature = EF BB BF
            if ((charString[1]==0xBB) && (charString[2]==0xBF))
            {
                charString+=3;
                return ENCODING_UTF8;
            }
            break;
        }
        case 0xFE: // big-endian 16-bit
        {
            // 16-bit signature = FE FF
            if (charString[1]==0xFF)
            {
                charString+=2;
                return ENCODING_UTF16_BE;
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
                if ((charString[2]==0) && (charString[3]==0) && (_overrideEncoding != ENCODING_UTF16)) //32-bit
                {
                    charString+=4;
                    return ENCODING_UTF32_LE;
                }
                else //16-bit
                {
                    charString+=2;
                    return ENCODING_UTF16_LE;
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
                return ENCODING_UTF32_BE;
            }
            break;
        }
    }
    return ENCODING_ASCII;
}

void EncodedText::setText(const unsigned char* text, int length)
{
    _unicodeText.clear();
    if (text != NULL)
    {
        const unsigned char* textStart = text;
        if ((_overrideEncoding == ENCODING_SIGNATURE) || 
            (_overrideEncoding == ENCODING_UTF16) || 
            (_overrideEncoding == ENCODING_UTF32))
            _encoding = findEncoding(text);

        int character = getNextCharacter(text);
        int charCount = (int)(text-textStart);
        while ((character) && (length<0 || (charCount <= length)))
        {
            _unicodeText.push_back(character);
            character = getNextCharacter(text);
            charCount = (int)(text-textStart);
        }
        _unicodeText.push_back(0); //just making the null-termination explicit
    }
}

void EncodedText::setOverrideEncoding(EncodedText::Encoding encoding) 
{ 
    if (_overrideEncoding != encoding)
    {
        _overrideEncoding = encoding;
        _encoding = encoding;
        //NB As the original text is not cached we cannot confirm any ENCODING_SIGNATURE until text is set again
    }
}
