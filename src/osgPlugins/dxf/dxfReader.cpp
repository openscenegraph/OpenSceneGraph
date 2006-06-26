/* dxfReader for OpenSceneGraph  Copyright (C) 2005 by GraphArchitecture ( grapharchitecture.com )
 * Programmed by Paul de Repentigny <pdr@grapharchitecture.com>
 * 
 * OpenSceneGraph is (C) 2004 Robert Osfield
 * 
 * This library is provided as-is, without support of any kind.
 *
 * Read DXF docs or OSG docs for any related questions.
 * 
 * You may contact the author if you have suggestions/corrections/enhancements.
 */
#include "dxfReader.h"
#include "dxfDataTypes.h"
#include "codeValue.h"

#include <map>
#include <vector>
#include <iostream>
#include <utility>
#include <sstream>
#include <math.h>
#include <iomanip>

using namespace std;

static 
std::string
trim(const string& str)
{
    if (!str.size()) return str;
    string::size_type first = str.find_first_not_of(" \t");
    string::size_type last = str.find_last_not_of("  \t\r\n");
    return str.substr(first, last-first+1);
}

/*
************** readerBase
*/
bool readerBase::readGroup(std::ifstream& f, codeValue& cv) 
{ 
    cv.reset();
    if (readGroupCode(f, cv._groupCode)) {
        cv._type = dxfDataType::typeForCode(cv._groupCode);
        switch (cv._type) {
            case dxfDataType::BOOL: 
                return readValue(f, cv._bool);
                break;
            case dxfDataType::SHORT: 
                return readValue(f, cv._short);
                break;
            case dxfDataType::INT: 
                return readValue(f, cv._int);
                break;
            case dxfDataType::LONG: 
                return readValue(f, cv._long);
                break;
            case dxfDataType::DOUBLE: 
                return readValue(f, cv._double);
                break;
            case dxfDataType::UNKNOWN:
            case dxfDataType::STRING: 
            case dxfDataType::HEX: 
            default: // to do: default case an error
                return readValue(f, cv._string);
                break;

        }
    } else {
        cv._type = dxfDataType::UNKNOWN;
        cv._groupCode = -1;
    }
    return false;
}

/*
************** readerText
*/

bool readerText::success(bool inSuccess, string type)
{
    if (!inSuccess)
        cout << "Error converting line " << _lineCount << " to type " << type << endl;
    return inSuccess;
}

bool readerText::getTrimmedLine(std::ifstream& f)
{
    static string line = "";
    if (getline(f, line, _delim)) {
        ++_lineCount;
        _str.clear();
        _str.str(trim(line));
        return true;
    }
    return false;
}


bool readerText::readGroupCode(std::ifstream& f, int &groupcode)
{
    if (getTrimmedLine(f)) {
        _str >> groupcode;
        return success(!_str.fail(), "int");
    } else {
        return false;
    }
}
bool readerText::readValue(std::ifstream& f, string &s)
{
    if (getTrimmedLine(f)) {
        getline(_str, s);
        // empty string is valid
        return success((!_str.fail() || s == ""), "string");
    } else {
        return false;
    }
}
bool readerText::readValue(std::ifstream& f, bool &b)
{
    if (getTrimmedLine(f)) {
        _str >> b;
        return success(!_str.fail(), "bool");
    } else {
        return false;
    }
}
bool readerText::readValue(std::ifstream& f, short &s)
{
    if (getTrimmedLine(f)) {
        _str >> s;
        return success(!_str.fail(), "short");
    } else {
        return false;
    }
}
bool readerText::readValue(std::ifstream& f, int &i)
{
    if (getTrimmedLine(f)) {
        _str >> i;
        return success(!_str.fail(), "int");
    } else {
        return false;
    }
}

bool readerText::readValue(std::ifstream& f, long &l)
{
    if (getTrimmedLine(f)) {
        _str >> l;
        return success(!_str.fail(), "long");
    } else {
        return false;
    }
}
bool readerText::readValue(std::ifstream& f, double &d)
{
    if (getTrimmedLine(f)) {
        _str >> d;
        return success(!_str.fail(), "double");
    } else {
        return false;
    }
}

/*
************** dxfReader
*/


bool    
dxfReader::openFile(std::string fileName)
{
    if (fileName.size() == 0) return false;
    _ifs.open(fileName.c_str(), ios::binary); // found mac autocad with \r delimiters
    if (!_ifs) {
            cout << " Can't open " << fileName << endl;
            return false;
    }
    // A binary file starts with "AutoCAD Binary DXF<CR><LF><SUB><NULL>"

    char buf[255];
    _ifs.get(buf, 255);
    string sentinel(buf);
    if (trim(sentinel) == "AutoCAD Binary DXF") {
        cout << " Binary DXF not supported. For now. Come back soon." << endl;
        return false;
    }
    //string::size_type lf = sentinel.find('\n');
    //string::size_type cr = sentinel.find('\r');
    // gossage. un mac peut mettre juste CR. ca fonctionne pas:
//    if (cr > 0 && (!lf || lf > cr + 1))
//        _reader = new readerText('\r');
//    else 
        _reader = new readerText;
    _ifs.seekg(0, ios::beg);
    return true;
}

bool
dxfReader::nextGroupCode(codeValue& cv)
{
    return (_reader->readGroup(_ifs, cv));
}

