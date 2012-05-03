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

/*	File handling.
	dxfFile creates a dxfReader, which in turn create a readerText or a readerBinary
	depending on file type.
*/
#ifndef DXF_READER
#define DXF_READER 1

#include <string>
#include <sstream>

#include <osg/Referenced>
#include <osg/ref_ptr>

#include <osgDB/fstream>

class codeValue;

/// readerBase. abstract base class for reading a dxf file
/// and interpreting data types
class readerBase :public osg::Referenced
{
public:
    readerBase() {}
    virtual ~readerBase() {}
    bool readGroup(std::ifstream& f, codeValue& cv);

protected:
    virtual bool readGroupCode(std::ifstream& f, int &groupcode)  = 0;
    virtual bool readValue(std::ifstream& f, std::string &s)  = 0;
    virtual bool readValue(std::ifstream& f, bool &b)  = 0;
    virtual bool readValue(std::ifstream& f, short &s)  = 0;
    virtual bool readValue(std::ifstream& f, int &i)  = 0;
    virtual bool readValue(std::ifstream& f, long &l)  = 0;
    virtual bool readValue(std::ifstream& f, double &d)  = 0;
};

/// readerText. convert data using stringstream.
class readerText : public readerBase
{
public:
    readerText(char delim = '\n') : readerBase(), _lineCount(0), _delim(delim) {}
    virtual ~readerText() {}

protected:
    bool success(bool inSuccess, std::string type);
    bool getTrimmedLine(std::ifstream& f);

    virtual bool readGroupCode(std::ifstream& f, int &groupcode);
    virtual bool readValue(std::ifstream& f, std::string &s);
    virtual bool readValue(std::ifstream& f, bool &b);
    virtual bool readValue(std::ifstream& f, short &s);
    virtual bool readValue(std::ifstream& f, int &i);
    virtual bool readValue(std::ifstream& f, long &l);
    virtual bool readValue(std::ifstream& f, double &d);
    std::stringstream _str;
    unsigned long _lineCount;
    char _delim;
};


/// readerBinary. to be implemented
class readerBinary : public readerBase
{
public:
    readerBinary() : readerBase() {}
    virtual ~readerBinary() {}
protected:
    virtual bool readGroupCode(std::ifstream& /*f*/, int& /*groupcode*/) { return false; }
    virtual bool readValue(std::ifstream& /*f*/, std::string& /*s*/) { return false; }
    virtual bool readValue(std::ifstream& /*f*/, bool& /*b*/) { return false; }
    virtual bool readValue(std::ifstream& /*f*/, short& /*s*/) { return false; }
    virtual bool readValue(std::ifstream& /*f*/, int& /*i*/) { return false; }
    virtual bool readValue(std::ifstream& /*f*/, long& /*l*/) { return false; }
    virtual bool readValue(std::ifstream& /*f*/, double& /*d*/) { return false; }
};

/// dxfReader. gets you through the dxf file, one group code/value pair at a time.
/// just instantiate, openFile(), then loop while(nextGroupCode())
class dxfReader : public osg::Referenced
{
public:
    dxfReader() {}
    virtual ~dxfReader() {}
    bool    openFile(std::string fileName);
    bool    nextGroupCode(codeValue& cv);
protected:
    osgDB::ifstream                _ifs;
    osg::ref_ptr<readerBase>    _reader;
};

#endif
