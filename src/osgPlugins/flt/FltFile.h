// FltFile.h

#ifndef __FLT_FILE_H
#define __FLT_FILE_H


#include <map>
#include <string>

#include <osg/Node>

#include "Pool.h"
#include "HeaderRecord.h"

namespace flt {

class Record;


class FltFile : public osg::Referenced
{

public:
    FltFile(
        ColorPool* pColorPool = NULL,
        TexturePool* pTexturePool = NULL,
        MaterialPool* pMaterialPool = NULL);
    virtual ~FltFile();

    virtual osg::Object* readObject(const std::string& fileName);
    virtual osg::Node* readNode(const std::string& fileName);
    osg::Node* convert();
    Record* getHeaderRecord() { return _pHeaderRecord; }
    Record* readModel(const std::string& fileName);

    ColorPool*      getColorPool()      { return _pColorPool; }
    TexturePool*    getTexturePool()    { return _pTexturePool; }
    MaterialPool*   getMaterialPool()   { return _pMaterialPool; }

    void useLocalColorPool()            { _pColorPool = &_colorPool; }
    void useLocalTexturePool()          { _pTexturePool = &_texturePool; }
    void useLocalMaterialPool()         { _pMaterialPool = &_materialPool; }

    int getFlightVersion();

protected:

    Record* readFile(const std::string& fileName);
    void readExternals(Record* pRec);

private:

    Record*         _pHeaderRecord;

    ColorPool       _colorPool;
    TexturePool     _texturePool;
    MaterialPool    _materialPool;

    ColorPool*      _pColorPool;
    TexturePool*    _pTexturePool;
    MaterialPool*   _pMaterialPool;
};



}; // end namespace flt

#endif
