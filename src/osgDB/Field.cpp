/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
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
#include <osgDB/Field>

using namespace osgDB;
using namespace std;

Field::Field()
{
    _init();
}


Field::Field(const Field& ic)
{
    _copy(ic);
}


Field::~Field()
{
    _free();
}


Field& Field::operator = (const Field& ic)
{
    if (this==&ic) return *this;
    _free();
    _copy(ic);
    return *this;
}


void Field::_free()
{
    // free all data
    if (_fieldCache) delete [] _fieldCache;

    _init();

}


void Field::_init()
{

    _fieldCacheCapacity = 256;
    _fieldCacheSize = 0;
    _fieldCache = NULL;

    _fieldType = UNINITIALISED;

    _withinQuotes = false;

    _noNestedBrackets = 0;

}


void Field::_copy(const Field& ic)
{

    // copy string cache.
    if (ic._fieldCache)
    {
        _fieldCacheCapacity = ic._fieldCacheCapacity;
        _fieldCacheSize = ic._fieldCacheSize;
        _fieldCache = new char [_fieldCacheCapacity];
        strncpy(_fieldCache,ic._fieldCache,_fieldCacheCapacity);
    }
    else
    {
        _fieldCacheCapacity = 0;
        _fieldCacheSize = 0;
        _fieldCache = NULL;
    }

    _fieldType = ic._fieldType;

    _withinQuotes = ic._withinQuotes;

    _noNestedBrackets = ic._noNestedBrackets;
}


void Field::setWithinQuotes(bool withinQuotes)
{
    _withinQuotes=withinQuotes;
    _fieldType = UNINITIALISED;
}


bool Field::getWithinQuotes()
{
    return _withinQuotes;
}


void Field::setNoNestedBrackets(int no)
{
    _noNestedBrackets=no;
}


int Field::getNoNestedBrackets()
{
    return _noNestedBrackets;
}


const char* Field::getStr() const
{
    if (_fieldCacheSize!=0) return _fieldCache;
    else return NULL;
}


char* Field::takeStr()
{
    char* field = _fieldCache;

    _fieldCache = NULL;
    _fieldCacheSize = 0;

    _fieldType = UNINITIALISED;
    _withinQuotes = false;

    return field;
}


void Field::reset()
{
    _fieldCacheSize = 0;
    if (_fieldCache)
    {
        _fieldCache[_fieldCacheSize] = 0;
    }

    _withinQuotes = false;
    _noNestedBrackets = 0;
}


void Field::addChar(char c)
{
    if (_fieldCache==NULL)
    {
        if (_fieldCacheCapacity<MIN_CACHE_SIZE) _fieldCacheCapacity=MIN_CACHE_SIZE;
        _fieldCache = new char[_fieldCacheCapacity];
        memset(_fieldCache,0,_fieldCacheCapacity);
        _fieldCacheSize = 0;
    }
    else if (_fieldCacheSize>=_fieldCacheCapacity-1)
    {
        if (_fieldCacheCapacity<MIN_CACHE_SIZE) _fieldCacheCapacity=MIN_CACHE_SIZE;
        while (_fieldCacheSize>=_fieldCacheCapacity-1) _fieldCacheCapacity *= 2;
        char* tmp_str = _fieldCache;
        _fieldCache = new char[_fieldCacheCapacity];
        memset(_fieldCache,0,_fieldCacheCapacity);
        strncpy(_fieldCache,tmp_str,_fieldCacheSize);
        delete [] tmp_str;

    }
    _fieldCache[_fieldCacheSize++] = c;
    _fieldCache[_fieldCacheSize] = 0;
    _fieldType = UNINITIALISED;
}


Field::FieldType Field::getFieldType() const
{
    if (_fieldType==UNINITIALISED && _fieldCache)
    {
        _fieldType = calculateFieldType(_fieldCache,_withinQuotes);
    }
    return _fieldType;
}


bool Field::isValid() const
{
    if (_fieldCacheSize>0  && !_withinQuotes) return true;
    else return false;
}


bool Field::isOpenBracket() const
{
    if (_fieldCacheSize==1) return _fieldCache[0]=='{';
    else return false;
}


bool Field::isCloseBracket() const
{
    if (_fieldCacheSize==1) return _fieldCache[0]=='}';
    else return false;
}


bool Field::isWord() const
{
    getFieldType();
    return (_fieldType==WORD);
}


bool Field::matchWord(const char* str) const
{
    getFieldType();
    return _fieldType==WORD && strcmp(_fieldCache,str)==0;
}


bool Field::matchWord(const char* str,int noCharacters) const
{
    getFieldType();
    return _fieldType==WORD && strncmp(_fieldCache,str,noCharacters)==0;
}


bool Field::isString() const
{
    return getNoCharacters()!=0;
}


bool Field::matchString(const char* str) const
{
    return strcmp(_fieldCache,str)==0;
}


bool Field::matchString(const char* str,int noCharacters) const
{
    return strncmp(_fieldCache,str,noCharacters)==0;
}


bool Field::isQuotedString() const
{
    return _withinQuotes;
}


bool Field::isInt() const
{
    getFieldType();
    return _fieldType==INTEGER;
}


bool Field::matchInt(int i) const
{
    getFieldType();
    if (_fieldType==INTEGER)
    {
        return strtol(_fieldCache,NULL,0)==i;
    }
    else
    {
        return false;
    }
}


bool Field::getInt(int& i) const
{
    getFieldType();
    if (_fieldType==INTEGER)
    {
        i = strtol(_fieldCache,NULL,0);
        return true;
    }
    else
    {
        return false;
    }
}

bool Field::isUInt() const
{
    getFieldType();
    return _fieldType==INTEGER;
}


bool Field::matchUInt(unsigned int i) const
{
    getFieldType();
    if (_fieldType==INTEGER)
    {
        return (unsigned int) strtoul(_fieldCache,NULL,0)==i;
    }
    else
    {
        return false;
    }
}


bool Field::getUInt(unsigned int& i) const
{
    getFieldType();
    if (_fieldType==INTEGER)
    {
        i = strtoul(_fieldCache,NULL,0);
        return true;
    }
    else
    {
        return false;
    }
}

bool Field::isFloat() const
{
    getFieldType();
    return _fieldType==REAL || _fieldType==INTEGER;
}


bool Field::matchFloat(float f) const
{
    getFieldType();
    if (_fieldType==REAL || _fieldType==INTEGER)
    {
        return (float)atof(_fieldCache)==f;
    }
    else
    {
        return false;
    }
}


bool Field::getFloat(float& f) const
{
    getFieldType();
    if (_fieldType==REAL || _fieldType==INTEGER)
    {
        f = (float)atof(_fieldCache);
        return true;
    }
    else
    {
        return false;
    }
}

bool Field::getFloat(double& f) const
{
    getFieldType();
    if (_fieldType==REAL || _fieldType==INTEGER)
    {
        f = atof(_fieldCache);
        return true;
    }
    else
    {
        return false;
    }
}


Field::FieldType Field::calculateFieldType(const char* str,bool withinQuotes)
{
    if (str==NULL) return BLANK;
    if (*str==0) return BLANK;

    if (withinQuotes) return STRING;

    bool hadPlusMinus = false;
    bool hadDecimalPlace = false;
    bool hadExponent = false;
    bool couldBeInt = true;
    bool couldBeFloat = true;
    int noZeroToNine = 0;

    const char* ptr = str;
    
    // check if could be a hex number.
    if (strncmp(ptr,"0x",2)==0)
    {
        // skip over leading 0x, and then go through rest of string
        // checking to make sure all values are 0...9 or a..f.
        ptr+=2;
        while (
               *ptr!=0 &&
               ((*ptr>='0' && *ptr<='9') ||
                (*ptr>='a' && *ptr<='f') ||  
                (*ptr>='A' && *ptr<='F'))
              )
        {
            ++ptr;
        }
        
        // got to end of string without failure, therefore must be a hex integer.
        if (*ptr==0) return INTEGER;
    }
    
    ptr = str;
    // check if a float or an int.
    while (*ptr!=0 && couldBeFloat)
    {
        if (*ptr=='+' || *ptr=='-')
        {
            if (hadPlusMinus)
            {
                couldBeInt = false;
                couldBeFloat = false;
            } else hadPlusMinus = true;
        }
        else if (*ptr>='0' && *ptr<='9')
        {
            noZeroToNine++;
        }
        else if (*ptr=='.')
        {
            if (hadDecimalPlace)
            {
                couldBeInt = false;
                couldBeFloat = false;
            }
            else
            {
                hadDecimalPlace = true;
                couldBeInt = false;
            }
        }
        else if (*ptr=='e' || *ptr=='E')
        {
            if (hadExponent || noZeroToNine==0)
            {
                couldBeInt = false;
                couldBeFloat = false;
            }
            else
            {
                hadExponent = true;
                couldBeInt = false;
                hadDecimalPlace = false;
                hadPlusMinus = false;
                noZeroToNine=0;
            }
        }
        else
        {
            couldBeInt = false;
            couldBeFloat = false;
        }
        ++ptr;
    }

    if (couldBeInt && noZeroToNine>0) return INTEGER;
    if (couldBeFloat && noZeroToNine>0) return REAL;
    if (str[0]=='{') return OPEN_BRACKET;
    if (str[0]=='}') return CLOSE_BRACKET;
    return WORD;
}
