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

        ReadResult readTransferFunction(std::istream& fin) const
        {
            osg::TransferFunction1D::ColorMap colorMap;

            float colorScale = 1.0f/255.0f;

            while(fin)
            {
                char readline[4096];
                *readline = 0;
                fin.getline(readline, sizeof(readline));

                if (*readline==0) continue;

                if (*readline=='#')
                {
                    OSG_NOTICE<<"comment = ["<<readline<<"]"<<std::endl;
                }
                else
                {
                    std::stringstream str(readline);

                    std::string value;
                    str >> value;

                    if (value=="colour-scale" || value=="color-scale")
                    {
                        std::string scaleStr;
                        str >> scaleStr;

                        OSG_NOTICE<<"color-scale = ["<<scaleStr<<"]"<<std::endl;

                        if (!scaleStr.empty())
                        {
                            colorScale = 1.0f/osg::asciiToFloat(scaleStr.c_str());
                        }
                    }
                    else
                    {

                        std::string red, green, blue, alpha;
                        str >> red >> green >> blue >> alpha;

                        *readline = 0;
                        str.getline(readline, sizeof(readline));

                        char* comment = readline;
                        while(*comment==' ' || *comment=='\t') ++comment;

                        if (*comment!=0)
                        {
                            OSG_NOTICE<<"value = "<<value<<" ("<<red<<", "<<green<<", "<<blue<<", "<<alpha<<") comment = ["<<comment<<"]"<<std::endl;
                        }
                        else
                        {
                            OSG_NOTICE<<"value = "<<value<<" ("<<red<<", "<<green<<", "<<blue<<", "<<alpha<<")"<<std::endl;
                        }

                        colorMap[osg::asciiToFloat(value.c_str())] = osg::Vec4(osg::asciiToFloat(red.c_str()),osg::asciiToFloat(green.c_str()),osg::asciiToFloat(blue.c_str()),osg::asciiToFloat(alpha.c_str()));
                    }
                }
            }

            if (colorMap.empty())
            {
                return ReadResult::ERROR_IN_READING_FILE;
            }

            // colorMap[value] = osg::Vec4(red*colorScale,green*colorScale,blue*colorScale,alpha*colorScale);

            if (colorScale!=1.0)
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

        virtual ReadResult readObject(std::istream& fin,const osgDB::ReaderWriter::Options*) const
        {
            return readTransferFunction(fin);
        }

        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            osgDB::ifstream fin(fileName.c_str(), std::ios::in | std::ios::binary);

            if (fin) return readTransferFunction( fin);
            else return ReadResult::ERROR_IN_READING_FILE;
        }

        virtual WriteResult writeTransferFunction(const osg::TransferFunction1D* tf, std::ostream& fout) const
        {
            const osg::TransferFunction1D::ColorMap& cm = tf->getColorMap();
            float colorScale = 255.0f;
            for(osg::TransferFunction1D::ColorMap::const_iterator itr = cm.begin();
                itr != cm.end();
                ++itr)
            {
                const osg::Vec4& c = itr->second;
                fout << itr->first <<" "<< c.r()*colorScale <<" "<< c.g()*colorScale << " "<< c.b()*colorScale<<" "<<c.a()*colorScale<<std::endl;
            }

            return WriteResult::FILE_SAVED;
        }


        virtual WriteResult writeObject(const osg::Object& object, std::ostream& fout, const osgDB::ReaderWriter::Options*) const
        {
            const osg::TransferFunction1D* tf = dynamic_cast<const osg::TransferFunction1D*>(&object);
            if (!tf) return WriteResult::FILE_NOT_HANDLED;

            return writeTransferFunction(tf, fout);
        }

        virtual WriteResult writeObject(const osg::Object& object, const std::string& fileName, const osgDB::ReaderWriter::Options*) const
        {
            OSG_NOTICE<<"ReaderWriterTF::writeObject"<<fileName<<std::endl;

            const osg::TransferFunction1D* tf = dynamic_cast<const osg::TransferFunction1D*>(&object);
            if (!tf) return WriteResult::FILE_NOT_HANDLED;

            std::string ext = osgDB::getFileExtension(fileName);
            if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

            osgDB::ofstream fout(fileName.c_str(), std::ios::out | std::ios::binary);
            if(!fout) return WriteResult::ERROR_IN_WRITING_FILE;

            return writeTransferFunction(tf, fout);
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(tf, ReaderWriterTF)
