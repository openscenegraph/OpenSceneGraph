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
#include <osgDB/FileNameUtils>

#include <osgSim/GeographicLocation>

#include <string>


using namespace flt;


FltFile::FltFile(
    ColorPool* pColorPool,
    TexturePool* pTexturePool,
    MaterialPool* pMaterialPool,
    LtPtAppearancePool* pLtPtAppearancePool,
    LtPtAnimationPool* pLtPtAnimationPool,
    osgDB::ReaderWriter::Options* options):
        _useTextureAlphaForTransparancyBinning(true),
        _doUnitsConversion(true),
        _defaultDOFAnimationState(false),
        _desiredUnits(ConvertToMeters)
{
    
    
    

    if (pColorPool)
    {
        // use external color palette, ignore internal
        _useInternalColorPalette = false;
        setColorPool( pColorPool );
    }
    else
    {
        // use internal color palette
        _useInternalColorPalette = true;
        setColorPool( new ColorPool );
    }

    if (pTexturePool)
    {
        // use external texture palette, ignore internal
        _useInternalTexturePalette = false;
        setTexturePool( pTexturePool );
    }
    else
    {
        // use internal texture palette
        _useInternalTexturePalette = true;
        setTexturePool( new TexturePool );
    }

    if (pMaterialPool)
    {
        // use external material palette, ignore internal
        _useInternalMaterialPalette = false;
        setMaterialPool( pMaterialPool );
    }
    else
    {
        // use internal material palette
        _useInternalMaterialPalette = true;
        setMaterialPool( new MaterialPool );
    }

    if (pLtPtAppearancePool && pLtPtAnimationPool) // Can only be non-NULL if parent is 15.8.
    {
        // use external light point appearance and animation palettes, ignore internal
        _useInternalLtPtPalettes = false;
        setLtPtAppearancePool( pLtPtAppearancePool );
        setLtPtAnimationPool( pLtPtAnimationPool );
    }
    else
    {
        // If they aren't both set, then they must both be NULL.
        assert( (pLtPtAppearancePool==NULL) && (pLtPtAppearancePool==NULL) );
        // use internal light point palettes
        _useInternalLtPtPalettes = true;
        setLtPtAppearancePool( new LtPtAppearancePool );
        setLtPtAnimationPool( new LtPtAnimationPool );
    }

    // no support for external light palettes
    setLightPool( new LightPool );

    // instances are always internally defined 
    setInstancePool( new InstancePool );
    
    _options = options;
}


osg::Object* FltFile::readObject(const std::string& fileName)
{
    return readNode(fileName);
}


osg::Node* FltFile::readNode(const std::string& fileName)
{
    _directory = osgDB::getFilePath(fileName);

    if (readModel(fileName))
    {
        // Convert record tree to osg scene graph
        osg::Node* model = convert();
        
        if (model)
        {
            // Store model origin in returned Node userData.
            osg::ref_ptr<osgSim::GeographicLocation> loc = new osgSim::GeographicLocation;
            double lat, lon;
            getOrigin( lat, lon );
            loc->set( lat, lon );
            model->setUserData( loc.get() );
            
            osg::notify(osg::INFO)<<"FltFile::readNode("<<fileName<<") lat="<<lat<<" lon="<<lon<<std::endl;
        
            return model;
        }
    }

    return NULL;
}


osg::Group* FltFile::convert()
{
    ConvertFromFLT visit;
    visit.setUseTextureAlphaForTransparancyBinning(getUseTextureAlphaForTransparancyBinning());
    visit.setDoUnitsConversion(getDoUnitsConversion());
    return visit.convert(getHeaderRecord());
}


// Read flight model (include externals)
bool FltFile::readModel(const std::string& fileName)
{
    if (readFile(fileName))
    {
        readExternals();
        return getHeaderRecord() ? true : false;
    }
    return false;
}


bool FltFile::readFile(const std::string& fileName)
{

    // havn't found file, look in OSGFILEPATH
    std::string foundFileName = osgDB::findDataFile(fileName, _options.get());
    if (foundFileName.empty()) return false;

    FileInput fin;
    if (!fin.open(foundFileName)) return false;

    Record* pRec = fin.readCreateRecord(this);
    if (pRec == NULL)
    {
        osg::notify(osg::WARN) << "File not found " << fileName << std::endl;
        return false;
    }

    _headerRecord = (HeaderRecord*)pRec;
    if (pRec->isPrimaryNode())   // Header
        pRec->readLocalData(fin);// Read rest of file

    fin.close();

    return true;
}


#define REGISTER_FLT 1

// This class was originally scoped within FltFile::readExternals() function.
// Irix 7.3 compilers hork on this.

    class ReadExternal : public RecordVisitor
    {
        public:
            ReadExternal(FltFile* fltFile)
            {
                _pFltFile = fltFile;
                setTraverseMode(RecordVisitor::TRAVERSE_ALL_CHILDREN);
            }

            virtual void apply(ExternalRecord& rec)
            {
                SExternalReference* pSExternal = (SExternalReference*)rec.getData();

                if (pSExternal)
                {
                    FltFile*        pExternalFltFile = NULL;
                    ColorPool*      pColorPool    = NULL;
                    TexturePool*    pTexturePool  = NULL;
                    MaterialPool*   pMaterialPool = NULL;
                    LtPtAppearancePool* pLtPtAppearancePool = NULL;
                    LtPtAnimationPool* pLtPtAnimationPool = NULL;
                    std::string filename( rec.getFilename() );

                    osg::notify(osg::INFO) << "External=" << filename << std::endl;

                    if (rec.getFlightVersion() > 13)
                    {
                        if (!(pSExternal->dwFlags & ExternalRecord::COLOR_PALETTE_OVERRIDE))
                            pColorPool = _pFltFile->getColorPool();

                        if (!(pSExternal->dwFlags & ExternalRecord::TEXTURE_PALETTE_OVERRIDE))
                            pTexturePool = _pFltFile->getTexturePool();

                        if (!(pSExternal->dwFlags & ExternalRecord::MATERIAL_PALETTE_OVERRIDE))
                            pMaterialPool = _pFltFile->getMaterialPool();

                        if (rec.getFlightVersion() >= 1580)
                        {
                            if (!(pSExternal->dwFlags & ExternalRecord::LIGHT_POINT_PALETTE_OVERRIDE))
                            {
                                pLtPtAppearancePool = _pFltFile->getLtPtAppearancePool();
                                pLtPtAnimationPool = _pFltFile->getLtPtAnimationPool();
                            }
                        }
                    }


                #if REGISTER_FLT
                    bool registerFLT = true;
                #else
                    bool registerFLT = false;
                #endif
    
                    pExternalFltFile = registerFLT ? Registry::instance()->getFltFile(filename) : NULL;
                    if (pExternalFltFile == NULL)
                    {
                        osg::ref_ptr<osgDB::ReaderWriter::Options> options = 
                            _pFltFile->getOptions() ? _pFltFile->getOptions() : 
                                                      new osgDB::ReaderWriter::Options;

                        //Path for Nested external references
                        osgDB::FilePathList& fpl = options->getDatabasePathList();
                        const std::string& filePath = osgDB::getFilePath(filename);
                        std::string pushAndPopPath;
                        //If absolute path
                        if( (filePath.length()>0 && filePath.find_first_of("/\\")==0) ||
                            (filePath.length()>2 && filePath.substr(1,1)==":" && filePath.find_first_of("/\\")==2) )
                        {
                            pushAndPopPath = filePath;
                        }
                        else
                        {
                            pushAndPopPath = (fpl.empty() | fpl.back().empty() ? "." : fpl.back()) + "/" + filePath;
                        }

                        /*char optionsString[256];
                        sprintf(optionsString,"FLT_VER %d",rec.getFlightVersion());
                        options->setOptionString(optionsString);*/

                        //osg::notify(osg::NOTICE)<<"Create local path"<<pushAndPopPath<<std::endl;

                        fpl.push_back(pushAndPopPath);
                       

                        pExternalFltFile = new FltFile( pColorPool, pTexturePool, pMaterialPool,
                            pLtPtAppearancePool, pLtPtAnimationPool, options.get() );

                        if (registerFLT)
                        {
                            Registry::instance()->addFltFile(filename, pExternalFltFile);
                        }

                        pExternalFltFile->readModel(filename);

                        fpl.pop_back();
                    }

                    rec.setExternal(pExternalFltFile);
                }
            }

        public:

            FltFile* _pFltFile;
    };

void FltFile::readExternals()
{
    ReadExternal visitor(this);
    _headerRecord->accept(visitor);
}


int FltFile::getFlightVersion() const
{
    if (_headerRecord.get())
    {
        SHeader* pSHeader = (SHeader*)_headerRecord.get()->getData();
        if (pSHeader)
            return pSHeader->diFormatRevLev;
    }
    return 0;
}


void FltFile::getOrigin( double& latitude, double& longitude ) const
{
    if (_headerRecord.get())
    {
        SHeader* pSHeader = (SHeader*)_headerRecord.get()->getData();
        if (pSHeader)
        {
            latitude = pSHeader->Origin.x();
            longitude = pSHeader->Origin.y();
        }
    }
}


std::string FltFile::getDesiredUnitsString() const
{
    switch (_desiredUnits)
    {
    case ConvertToMeters:
        return "ConvertToMeters";
        break;
    case ConvertToKilometers:
        return "ConvertToKilometers";
        break;
    case ConvertToFeet:
        return "ConvertToFeet";
        break;
    case ConvertToInches:
        return "ConvertToInches";
        break;
    case ConvertToNauticalMiles:
        return "ConvertToNauticalMiles";
        break;
    default:
        return "Invalid";
        break;
    }
}


