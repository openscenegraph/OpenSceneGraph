// FltFile.h

#ifndef __FLT_FILE_H
#define __FLT_FILE_H

#include <osg/Group>

#include <map>
#include <string>

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
        osg::Group* convert();
        bool readModel(const std::string& fileName);

        ColorPool*      getColorPool()      { return _colorPool.get(); }
        TexturePool*    getTexturePool()    { return _texturePool.get(); }
        MaterialPool*   getMaterialPool()   { return _materialPool.get(); }
        InstancePool*   getInstancePool()   { return _instancePool.get(); }

        void setColorPool(ColorPool* colorPool)         { _colorPool = colorPool; }
        void setTexturePool(TexturePool* texturePool)   { _texturePool = texturePool; }
        void setMaterialPool(MaterialPool* materialPool){ _materialPool = materialPool; }
        void setInstancePool(InstancePool* instancePool){ _instancePool = instancePool; }

        inline const bool useInternalColorPalette() const    { return _useInternalColorPalette; }
        inline const bool useInternalTexturePalette() const  { return _useInternalTexturePalette; }
        inline const bool useInternalMaterialPalette() const { return _useInternalMaterialPalette; }

        int getFlightVersion();
        inline HeaderRecord* getHeaderRecord() { return _headerRecord.get(); }

    protected:

        virtual ~FltFile() {}

        bool readFile(const std::string& fileName);
        void readExternals();

    private:

        osg::ref_ptr<HeaderRecord>        _headerRecord;

        bool                        _useInternalColorPalette;
        bool                        _useInternalTexturePalette;
        bool                        _useInternalMaterialPalette;

        std::string                 _directory;
        
        osg::ref_ptr<ColorPool>     _colorPool;
        osg::ref_ptr<TexturePool>   _texturePool;
        osg::ref_ptr<MaterialPool>  _materialPool;
        osg::ref_ptr<InstancePool>  _instancePool;
};



}; // end namespace flt

#endif
