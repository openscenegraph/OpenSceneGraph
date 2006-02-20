#include <stdio.h>
#include <string.h>
#include <string>

#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include "ConvertFromPerformer.h"

#include <Performer/pfdu.h>
#include <Performer/pr/pfTexture.h>

extern "C" {
extern pfNode *pfdLoadFile_pfb( const char *);
extern int pfdStoreFile_pfb( pfNode *, const char *);
}

class ReaderWriterPFB : public osgDB::ReaderWriter
{

    public:

        ReaderWriterPFB();
        ~ReaderWriterPFB();

        void initPerformer();

        virtual const char* className() const { return "Performer Reader/Writer"; }
        virtual bool acceptsExtension(const std::string& extension) const
        { 
            return 
                osgDB::equalCaseInsensitive(extension,"3ds")     ? true :
                osgDB::equalCaseInsensitive(extension,"arcinfo") ? true :
                osgDB::equalCaseInsensitive(extension,"bin")     ? true :
                osgDB::equalCaseInsensitive(extension,"bpoly")   ? true :
                osgDB::equalCaseInsensitive(extension,"bw")      ? true :
                osgDB::equalCaseInsensitive(extension,"byu")     ? true :
                osgDB::equalCaseInsensitive(extension,"closest") ? true :
                osgDB::equalCaseInsensitive(extension,"csb")     ? true :
                osgDB::equalCaseInsensitive(extension,"ct")      ? true :
                osgDB::equalCaseInsensitive(extension,"dem")     ? true :
                osgDB::equalCaseInsensitive(extension,"doublerot") ? true :
                osgDB::equalCaseInsensitive(extension,"doublescale") ? true :
                osgDB::equalCaseInsensitive(extension,"doubletrans") ? true :
                osgDB::equalCaseInsensitive(extension,"dted")    ? true :
                osgDB::equalCaseInsensitive(extension,"dwb")     ? true :
                osgDB::equalCaseInsensitive(extension,"dxf")     ? true :
                osgDB::equalCaseInsensitive(extension,"evt")     ? true :
                osgDB::equalCaseInsensitive(extension,"flt")     ? true :
                osgDB::equalCaseInsensitive(extension,"gds")     ? true :
                osgDB::equalCaseInsensitive(extension,"gfo")     ? true :
                osgDB::equalCaseInsensitive(extension,"im")      ? true :
                osgDB::equalCaseInsensitive(extension,"irtp")    ? true :
                osgDB::equalCaseInsensitive(extension,"iv20")    ? true :
                osgDB::equalCaseInsensitive(extension,"iv")      ? true :
                osgDB::equalCaseInsensitive(extension,"lodfix")  ? true :
                osgDB::equalCaseInsensitive(extension,"lsa")     ? true :
                osgDB::equalCaseInsensitive(extension,"lsb")     ? true :
                osgDB::equalCaseInsensitive(extension,"medit")   ? true :
                osgDB::equalCaseInsensitive(extension,"m")       ? true :
                osgDB::equalCaseInsensitive(extension,"nff")     ? true :
                osgDB::equalCaseInsensitive(extension,"obj")     ? true :
                osgDB::equalCaseInsensitive(extension,"pegg")    ? true :
                osgDB::equalCaseInsensitive(extension,"pfb")     ? true :
                osgDB::equalCaseInsensitive(extension,"pfs")     ? true :
                osgDB::equalCaseInsensitive(extension,"phd")     ? true :
                osgDB::equalCaseInsensitive(extension,"poly")    ? true :
                osgDB::equalCaseInsensitive(extension,"post")    ? true :
                osgDB::equalCaseInsensitive(extension,"proc")    ? true :
                osgDB::equalCaseInsensitive(extension,"projtex") ? true :
                osgDB::equalCaseInsensitive(extension,"pts")     ? true :
                osgDB::equalCaseInsensitive(extension,"rot")     ? true :
                osgDB::equalCaseInsensitive(extension,"scale")   ? true :
                osgDB::equalCaseInsensitive(extension,"sgf")     ? true :
                osgDB::equalCaseInsensitive(extension,"sgo")     ? true :
                osgDB::equalCaseInsensitive(extension,"so")      ? true :
                osgDB::equalCaseInsensitive(extension,"spf")     ? true :
                osgDB::equalCaseInsensitive(extension,"spherepatch3") ? true :
                osgDB::equalCaseInsensitive(extension,"spherepatch") ? true :
                osgDB::equalCaseInsensitive(extension,"sphere")  ? true :
                osgDB::equalCaseInsensitive(extension,"sponge")  ? true :
                osgDB::equalCaseInsensitive(extension,"star")    ? true :
                osgDB::equalCaseInsensitive(extension,"stla")    ? true :
                osgDB::equalCaseInsensitive(extension,"stlb")    ? true :
                osgDB::equalCaseInsensitive(extension,"substclip") ? true :
                osgDB::equalCaseInsensitive(extension,"sv")      ? true :
                osgDB::equalCaseInsensitive(extension,"trans")   ? true :
                osgDB::equalCaseInsensitive(extension,"tri")     ? true :
                osgDB::equalCaseInsensitive(extension,"unc")     ? true :
                osgDB::equalCaseInsensitive(extension,"vct")     ? true :
                false;
        }

        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) fileName = file; // let Peformer see if it can file the filep

            osg::notify(osg::INFO)<<"ReaderWriterPFB::readImage( "<<fileName.c_str()<<" )\n";
            //initPerformer();

            pfTexture* tex = new pfTexture;
            tex->ref();
            if (tex->loadFile(fileName.c_str()))
            {
                int s=0;
                int t=0;
                int r=0;
                int comp=0;
                unsigned int* imageData = NULL;

                tex->getImage(&imageData,&comp,&s,&t,&r);

                int internalFormat = comp;

                unsigned int pixelFormat =
                    comp == 1 ? GL_LUMINANCE :
                    comp == 2 ? GL_LUMINANCE_ALPHA :
                    comp == 3 ? GL_RGB :
                    comp == 4 ? GL_RGBA : (GLenum)-1;

                unsigned int dataType = GL_UNSIGNED_BYTE;

                // copy image data
                int size = s * t * r * comp;
                unsigned char* data = (unsigned char*) malloc(size);
                memcpy(data, imageData, size);

                osg::Image* image = new osg::Image;
                image->setFileName(fileName.c_str());
                image->setImage(s,t,r,
                                internalFormat,
                                pixelFormat,
                                dataType,data,
                                osg::Image::USE_MALLOC_FREE);

                // free texture & image data
                tex->unrefDelete();

                return image;
            }

            // free texture & image data
            tex->unrefDelete();

            return ReadResult::FILE_NOT_HANDLED;
        }

        virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            osg::notify(osg::INFO)<<"ReaderWriterPFB::readNode( "<<fileName.c_str()<<" )\n";
            //initPerformer();

            pfNode* root = pfdLoadFile(fileName.c_str());
            if (root)
            {
                ConvertFromPerformer converter;
                if (options) {
                    const std::string option = options->getOptionString();
                    if (option.find("saveImagesAsRGB") != std::string::npos)
                        converter.setSaveImagesAsRGB(true);
                    if (option.find("saveAbsoluteImagePath") != std::string::npos)
                        converter.setSaveAbsoluteImagePath(true);
                }

                root->ref();
                osg::Node* node = converter.convert(root);
                root->unrefDelete();
                return node;
            }
            else
            {
                return ReadResult::FILE_NOT_HANDLED;
            }

        }


    protected:

        bool _performerInitialised;

};

ReaderWriterPFB::ReaderWriterPFB()
{
    _performerInitialised = false;
    initPerformer();
}


ReaderWriterPFB::~ReaderWriterPFB()
{
    if (_performerInitialised)
        pfExit();
}


void ReaderWriterPFB::initPerformer()
{
    if (_performerInitialised) return;

    _performerInitialised = true;

    pfInit();

    pfMultiprocess(0);

    //     FileList::iterator itr;
    //     for(itr=filelist.begin();itr!=filelist.end();++itr)
    //     {
    //         pfdInitConverter((*itr).c_str());
    //     }
    pfdInitConverter(".pfb");

    /*
     * Tell Performer to look in OSG search path
     */
    const osgDB::FilePathList& filePath = osgDB::Registry::instance()->getDataFilePathList();
    std::string path = "";
    for (unsigned int i = 0; i < filePath.size(); i++) {
        if (i != 0)
            path += ":";
        path += filePath[i];
    }
    pfFilePath(path.c_str());

    pfConfig();
}


// now register with sgRegistry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterPFB> g_readerWriter_PFB_Proxy;
