#include <stdio.h>
#include <string.h>

#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>

#include <osgDB/Registry>

#include "ConvertToPerformer.h"
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

        virtual const char* className() { return "Performer Reader/Writer"; }
        virtual bool acceptsExtension(const std::string& extension) { return extension=="pfb"; }

        virtual osg::Image* readImage(const std::string& fileName, const osgDB::ReaderWriter::Options*)
        {
            osg::notify(osg::INFO)<<   "ReaderWriterPFB::readImage( "<<fileName.c_str()<<" )\n";

            initPerformer();

            pfTexture* tex = new pfTexture;
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

                osg::Image* image = new osg::Image;
                image->setFileName(fileName.c_str());
                image->setImage(s,t,r,
                    internalFormat,
                    pixelFormat,
                    dataType,
                    (unsigned char*)imageData);

                return image;
            }

            return NULL;
        }

        virtual osg::Node* readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*)
        {
            osg::notify(osg::INFO)<<   "ReaderWriterPFB::readNode( "<<fileName.c_str()<<" )\n";

            initPerformer();
            pfNode* root = pfdLoadFile_pfb(fileName.c_str());

            ConvertFromPerformer converter;
            return converter.convert(root);

        }

        virtual bool writeNode(const osg::Node& node,const std::string& fileName, const osgDB::ReaderWriter::Options*)
        {
            osg::notify(osg::INFO)<<   "ReaderWriterPFB::writeNode( "<<fileName.c_str()<<" )\n";
            initPerformer();
            ConvertToPerformer converter;
            pfNode* root = converter.convert(&node);
            if (root)
            {
                return pfdStoreFile_pfb(root,fileName.c_str())!=0;
            }
            else
            {
                return false;
            }
        }

    protected:

        bool _performerInitialised;

};

ReaderWriterPFB::ReaderWriterPFB()
{
    _performerInitialised = false;
}


ReaderWriterPFB::~ReaderWriterPFB()
{
}


void ReaderWriterPFB::initPerformer()
{
    if (_performerInitialised) return;

    _performerInitialised = true;

    pfMultiprocess(0);

    pfInit();

    //     FileList::iterator itr;
    //     for(itr=filelist.begin();itr!=filelist.end();++itr)
    //     {
    //         pfdInitConverter((*itr).c_str());
    //     }

    pfConfig();

}


// now register with sgRegistry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterPFB> g_readerWriter_PFB_Proxy;
