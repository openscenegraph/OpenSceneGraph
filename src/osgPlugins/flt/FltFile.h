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

        virtual osg::Object* readObject(const std::string& fileName);
        virtual osg::Node* readNode(const std::string& fileName);
        osg::Node* convert();
        Record* getHeaderRecord() { return _pHeaderRecord; }
        Record* readModel(const std::string& fileName);

        ColorPool*      getColorPool()      { return _colorPool.get(); }
        TexturePool*    getTexturePool()    { return _texturePool.get(); }
        MaterialPool*   getMaterialPool()   { return _materialPool.get(); }

        void setColorPool(ColorPool* colorPool)         { _colorPool = colorPool; }
        void setTexturePool(TexturePool* texturePool)   { _texturePool = texturePool; }
        void setMaterialPool(MaterialPool* materialPool){ _materialPool = materialPool; }

        inline const bool useColorPalette() const       { return _useColorPalette; }
        inline const bool useTexturePalette() const     { return _useTexturePalette; }
        inline const bool useMaterialPalette() const    { return _useMaterialPalette; }

        int getFlightVersion();

    protected:

        virtual ~FltFile() {}

        Record* readFile(const std::string& fileName);
        void readExternals(Record* pRec);

    private:

        Record*         _pHeaderRecord;

        bool            _useColorPalette;
        bool            _useTexturePalette;
        bool            _useMaterialPalette;

        osg::ref_ptr<ColorPool>     _colorPool;
        osg::ref_ptr<TexturePool>   _texturePool;
        osg::ref_ptr<MaterialPool>  _materialPool;
};



}; // end namespace flt

#endif
