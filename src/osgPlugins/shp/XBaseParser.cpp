#include "XBaseParser.h"

#include <vector>
#include <string>

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(_MSC_VER) || defined(__MINGW32__)
    #include <io.h>
#else
    #include <unistd.h>
#endif

#include <fcntl.h>
#include <osg/Notify>

namespace ESRIShape
{


void XBaseHeader::print()
{
    OSG_INFO << "VersionNumber = " << (int) _versionNumber << std::endl
                           << "LastUpdate    = " << 1900 + (int) _lastUpdate[0] << "/" << (int) _lastUpdate[1] << "/" << (int) _lastUpdate[2] << std::endl
                           << "NumRecord     = " << _numRecord << std::endl
                           << "HeaderLength  = " << _headerLength << std::endl
                           << "RecordLength  = " << _recordLength << std::endl;
}

bool XBaseHeader::read(int fd)
{
    int nbytes = 0;

    if ((nbytes = ::read( fd, &_versionNumber, sizeof(_versionNumber))) <= 0) return false;
    if ((nbytes = ::read( fd, &_lastUpdate, sizeof(_lastUpdate))) <= 0) return false;
    if ((nbytes = ::read( fd, &_numRecord, sizeof(_numRecord))) <= 0) return false;
    if ((nbytes = ::read( fd, &_headerLength, sizeof(_headerLength))) <= 0) return false;
    if ((nbytes = ::read( fd, &_recordLength, sizeof(_recordLength))) <= 0) return false;
    if ((nbytes = ::read( fd, &_reserved, sizeof(_reserved))) <= 0) return false;
    if ((nbytes = ::read( fd, &_incompleteTransaction, sizeof(_incompleteTransaction))) <= 0) return false;
    if ((nbytes = ::read( fd, &_encryptionFlag, sizeof(_encryptionFlag))) <= 0) return false;
    if ((nbytes = ::read( fd, &_freeRecordThread, sizeof(_freeRecordThread))) <= 0) return false;
    if ((nbytes = ::read( fd, &_reservedMultiUser, sizeof(_reservedMultiUser))) <= 0) return false;
    if ((nbytes = ::read( fd, &_mdxflag, sizeof(_mdxflag))) <= 0) return false;
    if ((nbytes = ::read( fd, &_languageDriver, sizeof(_languageDriver))) <= 0) return false;
    if ((nbytes = ::read( fd, &_reserved2, sizeof(_reserved2))) <= 0) return false;

    return true;
}

void XBaseFieldDescriptor::print()
{
    OSG_INFO << "name           = " << _name << std::endl
                           << "type           = " << _fieldType << std::endl
                           << "length         = " << (int) _fieldLength << std::endl
                           << "decimalCount   = " << (int) _decimalCount << std::endl
                           << "workAreaID     = " << (int) _workAreaID << std::endl
                           << "setFieldFlag   = " << (int) _setFieldFlag << std::endl
                           << "indexFieldFlag = " << (int) _indexFieldFlag << std::endl;
}

bool XBaseFieldDescriptor::read(int fd)
{
    int nbytes = 0;

    if ((nbytes = ::read( fd, &_name, sizeof(_name))) <= 0) return false;
    if ((nbytes = ::read( fd, &_fieldType, sizeof(_fieldType))) <= 0) return false;
    if ((nbytes = ::read( fd, &_fieldDataAddress, sizeof(_fieldDataAddress))) <= 0) return false;
    if ((nbytes = ::read( fd, &_fieldLength, sizeof(_fieldLength))) <= 0) return false;
    if ((nbytes = ::read( fd, &_decimalCount, sizeof(_decimalCount))) <= 0) return false;
    if ((nbytes = ::read( fd, &_reservedMultiUser, sizeof(_reservedMultiUser))) <= 0) return false;
    if ((nbytes = ::read( fd, &_workAreaID, sizeof(_workAreaID))) <= 0) return false;
    if ((nbytes = ::read( fd, &_reservedMultiUser2, sizeof(_reservedMultiUser2))) <= 0) return false;
    if ((nbytes = ::read( fd, &_setFieldFlag, sizeof(_setFieldFlag))) <= 0) return false;
    if ((nbytes = ::read( fd, &_reserved, sizeof(_reserved))) <= 0) return false;
    if ((nbytes = ::read( fd, &_indexFieldFlag, sizeof(_indexFieldFlag))) <= 0) return false;

    return true;
}


XBaseParser::XBaseParser(const std::string& fileName):
    _valid(false)
{
    if (!fileName.empty())
    {
        int fd = 0;
#ifdef WIN32
        if( (fd = open( fileName.c_str(), O_RDONLY | O_BINARY )) < 0 )
#else
        if( (fd = ::open( fileName.c_str(), O_RDONLY )) < 0 )
#endif
        {
            perror( fileName.c_str() );
        }
        else
        {
            _valid = parse(fd);
            close(fd);
        }
    }
}

bool XBaseParser::parse(int fd)
{
    int nbytes;
    XBaseHeader _xBaseHeader;
    std::vector<XBaseFieldDescriptor> _xBaseFieldDescriptorList;
    XBaseFieldDescriptor _xBaseFieldDescriptorTmp;


    // ** read the header
    if (_xBaseHeader.read(fd) == false) return false;
//    _xBaseHeader.print();


    // ** read field descriptor
    bool fieldDescriptorDone = false;
    Byte nullTerminator;

    while (fieldDescriptorDone == false)
    {
        // ** store the field descriptor
        if (_xBaseFieldDescriptorTmp.read(fd) == false) return false;
        _xBaseFieldDescriptorList.push_back(_xBaseFieldDescriptorTmp);
//        _xBaseFieldDescriptorTmp.print();

        // ** check the terminator
        if ((nbytes = ::read( fd, &nullTerminator, sizeof(nullTerminator))) <= 0) return false;
        if (nullTerminator == 0x0D)
            fieldDescriptorDone = true;
        else
            ::lseek( fd, -1, SEEK_CUR);
    }


    // ** move to the end of the Header
    ::lseek( fd, _xBaseHeader._headerLength + 1, SEEK_SET);


    // ** reserve AttributeListList
    _shapeAttributeListList.reserve(_xBaseHeader._numRecord);


    // ** read each record and store them in the ShapeAttributeListList
    char* record = new char[_xBaseHeader._recordLength];

    std::vector<XBaseFieldDescriptor>::iterator it, end = _xBaseFieldDescriptorList.end();
    for (Integer i = 0; i < _xBaseHeader._numRecord; ++i)
    {
        if ((nbytes = ::read( fd, record, _xBaseHeader._recordLength)) <= 0) return false;

        char * recordPtr = record;
        osgSim::ShapeAttributeList * shapeAttributeList = new osgSim::ShapeAttributeList;
        shapeAttributeList->reserve(_xBaseFieldDescriptorList.size());

        for (it = _xBaseFieldDescriptorList.begin(); it != end; ++it)
        {
            switch (it->_fieldType)
            {
            case 'C':
            {
                char* str = new char[it->_fieldLength + 1];
                memcpy(str, recordPtr, it->_fieldLength);
                str[it->_fieldLength] = 0;
                shapeAttributeList->push_back(osgSim::ShapeAttribute((const char *) it->_name, (char*) str));
                delete [] str;
                break;
            }
            case 'N':
            {
                char* number = new char[it->_fieldLength + 1];
                memcpy(number, recordPtr, it->_fieldLength);
                number[it->_fieldLength] = 0;
                shapeAttributeList->push_back(osgSim::ShapeAttribute((const char *) it->_name, (int) atoi(number)));
                delete [] number;
                break;
            }
            case 'I':
            {
                int number;
                memcpy(&number, record, it->_fieldLength);
                shapeAttributeList->push_back(osgSim::ShapeAttribute((const char *) it->_name, (int) number));
                break;
            }
            case 'O':
            {
                double number;
                memcpy(&number, record, it->_fieldLength);
                shapeAttributeList->push_back(osgSim::ShapeAttribute((const char *) it->_name, (double) number));
                break;
            }
            default:
            {
                OSG_WARN << "ESRIShape::XBaseParser : record type "
                                       << it->_fieldType << "not supported, skipped" << std::endl;
                shapeAttributeList->push_back(osgSim::ShapeAttribute((const char *) it->_name, (double) 0));
                break;
            }


            }

            recordPtr += it->_fieldLength;
        }

        _shapeAttributeListList.push_back(shapeAttributeList);
    }

    delete [] record;

    return true;
}


}


