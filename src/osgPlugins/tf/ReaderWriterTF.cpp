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

        virtual WriteResult writeTransferFunction(const osg::TransferFunction1D* tf, std::ostream& fout, float colorScale) const
        {
            if (colorScale == 0.0) colorScale = 1.0f;

            const osg::TransferFunction1D::ColorMap& cm = tf->getColorMap();
            for(osg::TransferFunction1D::ColorMap::const_iterator itr = cm.begin();
                itr != cm.end();
                ++itr)
            {
                const osg::Vec4& c = itr->second;
                fout << itr->first <<" "<< c.r()*colorScale <<" "<< c.g()*colorScale << " "<< c.b()*colorScale<<" "<<c.a()*colorScale<<std::endl;
            }

            return WriteResult::FILE_SAVED;
        }


        virtual WriteResult writeObject(const osg::Object& object, std::ostream& fout, const osgDB::ReaderWriter::Options* options) const
        {
            const osg::TransferFunction1D* tf = dynamic_cast<const osg::TransferFunction1D*>(&object);
            if (!tf) return WriteResult::FILE_NOT_HANDLED;

            float colorScale = 0.0f; // default auto-detect scale
            readColorScale(options, colorScale);

            return writeTransferFunction(tf, fout, colorScale);
        }

        virtual WriteResult writeObject(const osg::Object& object, const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            OSG_NOTICE<<"ReaderWriterTF::writeObject"<<fileName<<std::endl;

            const osg::TransferFunction1D* tf = dynamic_cast<const osg::TransferFunction1D*>(&object);
            if (!tf) return WriteResult::FILE_NOT_HANDLED;

            std::string ext = osgDB::getFileExtension(fileName);
            if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

            float colorScale = 0.0f; // default auto-detect scale
            if (ext=="tf-255") colorScale = 1.0f/255.0f;
            readColorScale(options, colorScale);

            osgDB::ofstream fout(fileName.c_str(), std::ios::out | std::ios::binary);
            if(!fout) return WriteResult::ERROR_IN_WRITING_FILE;

            return writeTransferFunction(tf, fout, colorScale);
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(tf, ReaderWriterTF)
