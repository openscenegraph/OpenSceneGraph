// Released under the OSGPL license, as part of the OpenSceneGraph distribution.
//
// Implementation of a native TransferFunction ascii file format.
//
#include <osg/Notify>

#include <osg/TransferFunction>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class ReaderWriterTF : public osgDB::ReaderWriter
{
    public:

        ReaderWriterTF()
        {
            supportsExtension("tf","TransferFunction format");
            supportsExtension("tf-255","TransferFunction format");
        }

        virtual const char* className() const { return "TransferFunction Reader/Writer"; }

        ReadResult readTransferFunction(std::istream& fin, float colorScale) const
        {
            osg::TransferFunction1D::ColorMap colorMap;
            while(fin)
            {
                float value, red, green, blue, alpha;
                fin >> value >> red >> green >> blue >> alpha;
                if (fin)
                {
                    std::cout<<"value = "<<value<<" ("<<red<<", "<<green<<", "<<blue<<", "<<alpha<<")"<<std::endl;
                    colorMap[value] = osg::Vec4(red,green,blue,alpha);
                }
            }

            if (colorMap.empty())
            {
                return ReadResult::ERROR_IN_READING_FILE;
            }

            // colorMap[value] = osg::Vec4(red*colorScale,green*colorScale,blue*colorScale,alpha*colorScale);

            if (colorScale==0.0f)
            {
                float maxValue = 0.0f;
                for(osg::TransferFunction1D::ColorMap::iterator itr = colorMap.begin();
                    itr != colorMap.end();
                    ++itr)
                {
                    const osg::Vec4& c = itr->second;
                    if (c.r()>maxValue) maxValue = c.r();
                    if (c.g()>maxValue) maxValue = c.g();
                    if (c.b()>maxValue) maxValue = c.b();
                    if (c.a()>maxValue) maxValue = c.a();
                }

                if (maxValue>=2.0f)
                {
                    colorScale = 1.0f/255.0f;
                }
            }

            if (colorScale!=0.0)
            {
                OSG_NOTICE<<"Rescaling ColorMap by "<<colorScale<<std::endl;
                for(osg::TransferFunction1D::ColorMap::iterator itr = colorMap.begin();
                    itr != colorMap.end();
                    ++itr)
                {
                    osg::Vec4& c = itr->second;
                    c.r() *= colorScale;
                    c.g() *= colorScale;
                    c.b() *= colorScale;
                    c.a() *= colorScale;
                }
            }

            osg::TransferFunction1D* tf = new osg::TransferFunction1D;
            tf->assign(colorMap);

            return tf;
        }

        bool readColorScale(const osgDB::ReaderWriter::Options* options, float& colorScale) const
        {
            if (options && options->getOptionString().find("tf-255")!=std::string::npos)
            {
                colorScale = 1.0/255.0f;
                return true;
            }
            else
            {
                return false;
            }

        }

        virtual ReadResult readObject(std::istream& fin,const osgDB::ReaderWriter::Options* options =NULL) const
        {
            float colorScale = 0.0f; // default auto-detect scale

            readColorScale(options, colorScale);

            return readTransferFunction(fin, colorScale);
        }

        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            float colorScale = 0.0f; // default auto-detect scale
            if (ext=="tf-255") colorScale = 1.0f/255.0f;

            readColorScale(options, colorScale);

            osgDB::ifstream fin(fileName.c_str(), std::ios::in | std::ios::binary);

            if (fin) return readTransferFunction( fin, colorScale);
            else return ReadResult::ERROR_IN_READING_FILE;
        }

};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(tf, ReaderWriterTF)
