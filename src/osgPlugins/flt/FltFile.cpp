#include "FltFile.h"
#include "Registry.h"
#include "Record.h"
#include "RecordVisitor.h"
#include "ExternalRecord.h"
#include "flt2osg.h"             // ConvertFromFLT
#include "Input.h"

#include <osg/Node>
#include <osg/NodeVisitor>
#include <osg/Notify>

#include <osgDB/FileUtils>

#include <string>


using namespace flt;


FltFile::FltFile(
    ColorPool* pColorPool,
    TexturePool* pTexturePool,
    MaterialPool* pMaterialPool)
{
    _pHeaderRecord = NULL;

    if (pColorPool)
    {
        // use external color palette, ignore internal
        _useColorPalette = false;
        setColorPool( pColorPool );
    }
    else
    {
        // use internal color palette
        _useColorPalette = true;
        setColorPool( new ColorPool );
    }

    if (pTexturePool)
    {
        // use external texture palette, ignore internal
        _useTexturePalette = false;
        setTexturePool( pTexturePool );
    }
    else
    {
        // use internal texture palette
        _useTexturePalette = true;
        setTexturePool( new TexturePool );
    }

    if (pMaterialPool)
    {
        // use external material palette, ignore internal
        _useMaterialPalette = false;
        setMaterialPool( pMaterialPool );
    }
    else
    {
        // use internal material palette
        _useMaterialPalette = true;
        setMaterialPool( new MaterialPool );
    }
}


osg::Object* FltFile::readObject(const std::string& fileName)
{
    return readNode(fileName);
}


osg::Node* FltFile::readNode(const std::string& fileName)
{
    osg::Node* node = NULL;
    Record* pRootRec = readModel(fileName);

    if (pRootRec == NULL)
        return NULL;

    // Convert record tree to osg scene graph
    node = convert();

    pRootRec->unref();           // delete record tree

    return node;
}


osg::Node* FltFile::convert()
{
    ConvertFromFLT visit(this);
    return visit.convert(getHeaderRecord());
}


// Read flight model (include externals)
Record* FltFile::readModel(const std::string& fileName)
{
    Record* pRec = readFile(fileName);
    if (pRec == NULL) return NULL;

    readExternals(pRec);
    return pRec;
}


Record* FltFile::readFile(const std::string& fileName)
{
    FileInput fin;

    if (!fin.open(fileName)) 
    {
        // ok havn't found file, resort to using findFile...
        char* newFileName = osgDB::findFile(fileName.c_str());

        if (!newFileName) return NULL;
        if (!fin.open(newFileName)) return NULL;
    }

    osg::notify(osg::INFO) << "Loading " << fileName << " ... " << endl;

    Record* pRec = fin.readCreateRecord();
    _pHeaderRecord = pRec;
    if (pRec == NULL)
    {
        osg::notify(osg::WARN) << "File not found " << fileName << endl;
        return NULL;
    }

    if (pRec->isPrimaryNode())   // Header
        pRec->readLocalData(fin);// Read rest of file

    fin.close();

    return pRec;
}


#define REGISTER_FLT 1

class ReadExternal : public RecordVisitor
{
    public:
        ReadExternal(FltFile* fltFile)
        {
            _parentFltFile = fltFile;
            setTraverseMode(RecordVisitor::TRAVERSE_ALL_CHILDREN);
        }

        virtual void apply(ExternalRecord& rec)
        {
            SExternalReference* pSExternal = (SExternalReference*)rec.getData();

            if (pSExternal)
            {
                FltFile*        flt           = NULL;
                ColorPool*      pColorPool    = NULL;
                TexturePool*    pTexturePool  = NULL;
                MaterialPool*   pMaterialPool = NULL;
                std::string filename(pSExternal->szPath);

                osg::notify(osg::INFO) << "External=" << filename << endl;

                if (_parentFltFile && (_parentFltFile->getFlightVersion() > 13))
                {
                    if (pSExternal->diFlags & BIT0)
                        pColorPool = _parentFltFile->getColorPool();

                    if (pSExternal->diFlags & BIT2)
                        pTexturePool = _parentFltFile->getTexturePool();

                    if (pSExternal->diFlags & BIT1)
                        pMaterialPool = _parentFltFile->getMaterialPool();
                }

#if REGISTER_FLT
                flt = Registry::instance()->getFltFile(filename);
                if (flt == NULL)
                {
                    flt = new FltFile(pColorPool, pTexturePool, pMaterialPool);
                    flt->readModel(filename);
                }
                Registry::instance()->addFltFile(filename, flt);
#else
                flt = new FltFile(pColorPool, pTexturePool, pMaterialPool);
                flt->readModel(filename);
#endif

                rec.setExternal(flt);
            }
        }

    public:

        FltFile* _parentFltFile;
};


void FltFile::readExternals(Record* pRec)
{

    if (pRec)
    {
        ReadExternal visitor(this);
        pRec->accept(visitor);
    }
}


int FltFile::getFlightVersion()
{
    if (_pHeaderRecord)
    {
        SHeader* pSHeader = (SHeader*)_pHeaderRecord->getData();
        if (pSHeader)
            return pSHeader->diFormatRevLev;
    }
    return 0;
}
