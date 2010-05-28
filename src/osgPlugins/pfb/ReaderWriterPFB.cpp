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

        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) fileName = file; // let Peformer see if it can file the filep

            OSG_INFO<<"ReaderWriterPFB::readImage( "<<fileName.c_str()<<" )\n";
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

            OSG_INFO<<"ReaderWriterPFB::readNode( "<<fileName.c_str()<<" )\n";
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
    supportsExtension("3ds","");
    supportsExtension("arcinfo","");
    supportsExtension("bin","");
    supportsExtension("bpoly","");
    supportsExtension("bw","");
    supportsExtension("byu","");
    supportsExtension("closest","");
    supportsExtension("csb","");
    supportsExtension("ct","");
    supportsExtension("dem","");
    supportsExtension("doublerot","");
    supportsExtension("doublescale","");
    supportsExtension("doubletrans","");
    supportsExtension("dted","");
    supportsExtension("dwb","");
    supportsExtension("dxf","");
    supportsExtension("evt","");
    supportsExtension("flt","");
    supportsExtension("gds","");
    supportsExtension("gfo","");
    supportsExtension("im","");
    supportsExtension("irtp","");
    supportsExtension("iv20","");
    supportsExtension("iv","");
    supportsExtension("lodfix","");
    supportsExtension("lsa","");
    supportsExtension("lsb","");
    supportsExtension("medit","");
    supportsExtension("m","");
    supportsExtension("nff","");
    supportsExtension("obj","");
    supportsExtension("pegg","");
    supportsExtension("pfb","");
    supportsExtension("pfs","");
    supportsExtension("phd","");
    supportsExtension("poly","");
    supportsExtension("post","");
    supportsExtension("proc","");
    supportsExtension("projtex","");
    supportsExtension("pts","");
    supportsExtension("rot","");
    supportsExtension("scale","");
    supportsExtension("sgf","");
    supportsExtension("sgo","");
    supportsExtension("so","");
    supportsExtension("spf","");
    supportsExtension("spherepatch3","");
    supportsExtension("spherepatch","");
    supportsExtension("sphere","");
    supportsExtension("sponge","");
    supportsExtension("star","");
    supportsExtension("stla","");
    supportsExtension("stlb","");
    supportsExtension("substclip","");
    supportsExtension("sv","");
    supportsExtension("trans","");
    supportsExtension("tri","");
    supportsExtension("unc","");
    supportsExtension("vct","");

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
REGISTER_OSGPLUGIN(pfb, ReaderWriterPFB)
