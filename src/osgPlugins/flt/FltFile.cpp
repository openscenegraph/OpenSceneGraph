// FltFile.cpp

#include <osg/OSG>
#include <osg/Node>
#include <osg/NodeVisitor>
#include <osg/Notify>

#include "FltFile.h"
#include "Record.h"
#include "RecordVisitor.h"
#include "ExternalRecord.h"
#include "flt2osg.h"            // ConvertFromFLT
#include "Input.h"

using namespace flt;


FltFile::FltFile(
    ColorPool* pColorPool,
    TexturePool* pTexturePool,
    MaterialPool* pMaterialPool)

{
      if (pColorPool)
          _pColorPool = pColorPool;
      else
        _pColorPool = &_colorPool;

      if (pTexturePool)
          _pTexturePool = pTexturePool;
      else
        _pTexturePool = &_texturePool;

     if (pMaterialPool)
         _pMaterialPool = pMaterialPool;
     else
        _pMaterialPool = &_materialPool;

    _pHeaderRecord = NULL;
}


FltFile::~FltFile()
{
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
    
    pRootRec->unref();      // delete record tree

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

    if (!fin.open(fileName)) return NULL;

    osg::notify(osg::INFO) << "Loading " << fileName << " ... " << endl;

    Record* pRec = fin.readCreateRecord();
    _pHeaderRecord = pRec;
    if (pRec == NULL)
    {
        osg::notify(osg::WARN) << "File not found " << fileName << endl;
        return NULL;
    }

    if (pRec->isPrimaryNode())      // Header
        pRec->readLocalData(fin);   // Read rest of file

    fin.close();


    return pRec;
}


class ReadExternal : public RecordVisitor
{
public:
    ReadExternal(FltFile* fltFile)
    {
        _fltFile = fltFile;
        setTraverseMode(RecordVisitor::TRAVERSE_ALL_CHILDREN);
    }

    virtual void apply(ExternalRecord& rec)
    {
        SExternalReference* pSExternal = (SExternalReference*)rec.getData();

        osg::notify(osg::INFO) << "External=" << pSExternal->szPath << endl;

        ColorPool*      pColorPool    = NULL;
        TexturePool*    pTexturePool  = NULL;
        MaterialPool*   pMaterialPool = NULL;

        if (pSExternal->diFlags & BIT0)
            pColorPool = _fltFile->getColorPool();

        if (pSExternal->diFlags & BIT2)
            pTexturePool = _fltFile->getTexturePool();

        if (pSExternal->diFlags & BIT1)
            pMaterialPool = _fltFile->getMaterialPool();

        FltFile* flt = new FltFile(pColorPool, pTexturePool, pMaterialPool);
        if (flt)
        {
            flt->readModel(pSExternal->szPath);
            rec.setExternal(flt);
        }
    }

public:
    FltFile* _fltFile;
};

void FltFile::readExternals(Record* pRec)
{

    if (pRec)
    {
        ReadExternal visitor(this);
        pRec->accept(visitor);
    }
}



