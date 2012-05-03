#ifndef XBASEPARSER_H_
#define XBASEPARSER_H_



#include <vector>
#include <string>

#include "ESRIType.h"

#include <osg/ref_ptr>
#include <osgSim/ShapeAttribute>



namespace ESRIShape
{


struct XBaseHeader
{
    Byte    _versionNumber;
    Byte    _lastUpdate[3];
    Integer _numRecord;
    Short   _headerLength;
    Short   _recordLength;
    Short   _reserved;
    Byte    _incompleteTransaction;
    Byte    _encryptionFlag;
    Integer _freeRecordThread;
    Integer _reservedMultiUser[2];
    Byte    _mdxflag;
    Byte    _languageDriver;
    Short   _reserved2;

    void print();
    bool read(int fd);
};

struct XBaseFieldDescriptor
{
    Byte _name[11];
    Byte _fieldType;
    Integer _fieldDataAddress;
    Byte _fieldLength;
    Byte _decimalCount;
    Short _reservedMultiUser;
    Byte _workAreaID;
    Short _reservedMultiUser2;
    Byte _setFieldFlag;
    Byte _reserved[7];
    Byte _indexFieldFlag;

    void print();
    bool read(int fd);
};


class XBaseParser
{
    public:

        typedef std::vector< osg::ref_ptr<osgSim::ShapeAttributeList> > ShapeAttributeListList;

        XBaseParser(const std::string fileName);
        ~XBaseParser() {}
        ShapeAttributeListList & getAttributeList() { return _shapeAttributeListList; }

    private:

        XBaseParser();

        bool parse(int fd);

        ShapeAttributeListList _shapeAttributeListList;
        bool _valid;
};

}


#endif /*XBASEPARSER_H_*/
