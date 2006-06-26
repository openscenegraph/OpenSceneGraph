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


#ifndef DXF_CODE_VALUE
#define DXF_CODE_VALUE 1

#include <string>

/// a group code / value pair handler
/// to do: write accessors which check for the correct value
/// being asked for (each group code has a value type
/// associated with it).
class codeValue {
public:
    codeValue() { reset(); }
    void            reset()
    {
        _groupCode = -100;
        _type = 0;
        _bool = false;
        _short = 0;
        _int = 0;
        _long = 0;
        _double = 0;
        _string = "";
    }
    int                _groupCode;
    int                _type;
    std::string        _unknown;
    std::string        _string;
    bool            _bool;
    short            _short;
    int                _int;
    long            _long;
    double            _double;
};

typedef std::vector<codeValue> VariableList; // this may be too big, find another way

#endif
