#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/fstream>
#include <osgDB/Registry>

#include <osgTerrain/Locator>

#include "ESRIType.h"

#include "ESRIShape.h"
#include "ESRIShapeParser.h"

#include "XBaseParser.h"


class ESRIShapeReaderWriter : public osgDB::ReaderWriter
{
    public:
        ESRIShapeReaderWriter()
        {
            supportsExtension("shp","Geospatial Shape file format");
            supportsOption("double","Read x,y,z data as double an stored as geometry in osg::Vec3dArray's.");
        }

        virtual const char* className() { return "ESRI Shape ReaderWriter"; }
        
        virtual bool acceptsExtension(const std::string& extension) const
        {
            return osgDB::equalCaseInsensitive(extension,"shp");
        }

        virtual ReadResult readObject(const std::string& fileName, const Options* opt) const
        { return readNode(fileName,opt); }

        virtual ReadResult readNode(const std::string& file, const Options* options) const
        {
            std::string ext = osgDB::getFileExtension(file);
            if (!acceptsExtension(ext)) 
                return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile(file, options);
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            bool useDouble = false;
            if (options && options->getOptionString().find("double")!=std::string::npos)
            {
                useDouble = true;
            }
            
            
            ESRIShape::ESRIShapeParser sp(fileName, useDouble);

            
            std::string xbaseFileName(osgDB::getNameLessExtension(fileName) + ".dbf");
            ESRIShape::XBaseParser xbp(xbaseFileName);
            
            
            if (sp.getGeode() && (xbp.getAttributeList().empty() == false))
            {
                if (sp.getGeode()->getNumDrawables() != xbp.getAttributeList().size())
                {
                    osg::notify(osg::WARN) << "ESRIShape loader : .dbf file containe different record number that .shp file." << std::endl
                                           << "                   .dbf record skipped." << std::endl;
                }
                else
                {
                    osg::Geode * geode = sp.getGeode();
                    unsigned int i = 0;
                    
                    ESRIShape::XBaseParser::ShapeAttributeListList::iterator it, end = xbp.getAttributeList().end();
                    for (it = xbp.getAttributeList().begin(); it != end; ++it, ++i)
                    {
                        geode->getDrawable(i)->setUserData(it->get());
                    }
                }
            }    

            if (sp.getGeode())
            {
            
                std::string projFileName(osgDB::getNameLessExtension(fileName) + ".prj");
                if (osgDB::fileExists(projFileName))
                {
                    osgDB::ifstream fin(projFileName.c_str());
                    if (fin)
                    {
                        std::string projstring;
                        while(!fin.eof())
                        {
                            char readline[4096];
                            *readline = 0;
                            fin.getline(readline, sizeof(readline));
                            if (!projstring.empty() && !fin.eof())
                            {
                                projstring += '\n';
                            }
                            projstring += readline;

                        }
                        
                        if (!projstring.empty())
                        {
                            osgTerrain::Locator* locator = new osgTerrain::Locator;
                            sp.getGeode()->setUserData(locator);

                            locator->setFormat("WKT");
                            locator->setCoordinateSystem(projstring);
                            locator->setDefinedInFile(false);
                        }
                    }
                    
                }


            }
            return sp.getGeode();
        }
};

REGISTER_OSGPLUGIN(shp, ESRIShapeReaderWriter)
