#include <stdio.h>
#include <string.h>

#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>

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
        virtual bool acceptsExtension(const std::string& extension) 
	{ 
	    return 
                extension=="3ds"     ? true :
                extension=="arcinfo" ? true :
                extension=="bin"     ? true :
                extension=="bpoly"   ? true :
                extension=="bw"      ? true :
                extension=="byu"     ? true :
                extension=="closest" ? true :
                extension=="csb"     ? true :
                extension=="ct"      ? true :
                extension=="dem"     ? true :
                extension=="doublerot" ? true :
                extension=="doublescale" ? true :
                extension=="doubletrans" ? true :
                extension=="dted"    ? true :
                extension=="dwb"     ? true :
                extension=="dxf"     ? true :
                extension=="evt"     ? true :
                extension=="flt"     ? true :
                extension=="gds"     ? true :
                extension=="gfo"     ? true :
                extension=="im"      ? true :
                extension=="irtp"    ? true :
                extension=="iv20"    ? true :
                extension=="iv"      ? true :
                extension=="lodfix"  ? true :
                extension=="lsa"     ? true :
                extension=="lsb"     ? true :
                extension=="medit"   ? true :
                extension=="m"       ? true :
                extension=="nff"     ? true :
                extension=="obj"     ? true :
                extension=="pegg"    ? true :
                extension=="pfb"     ? true :
                extension=="pfs"     ? true :
                extension=="phd"     ? true :
                extension=="poly"    ? true :
                extension=="post"    ? true :
                extension=="proc"    ? true :
                extension=="projtex" ? true :
                extension=="pts"     ? true :
                extension=="rot"     ? true :
                extension=="scale"   ? true :
                extension=="sgf"     ? true :
                extension=="sgo"     ? true :
                extension=="so"      ? true :
                extension=="spf"     ? true :
                extension=="spherepatch3" ? true :
                extension=="spherepatch" ? true :
                extension=="sphere"  ? true :
                extension=="sponge"  ? true :
                extension=="star"    ? true :
                extension=="stla"    ? true :
                extension=="stlb"    ? true :
                extension=="substclip" ? true :
                extension=="sv"      ? true :
                extension=="trans"   ? true :
                extension=="tri"     ? true :
                extension=="unc"     ? true :
                extension=="vct"     ? true :
		false;
	}

        virtual ReadResult readImage(const std::string& fileName, const osgDB::ReaderWriter::Options*)
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

            return ReadResult::FILE_NOT_HANDLED;
        }

        virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*)
        {
            std::string ext = osgDB::getLowerCaseFileExtension(fileName);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            osg::notify(osg::INFO)<<   "ReaderWriterPFB::readNode( "<<fileName.c_str()<<" )\n";

            initPerformer();

            pfNode* root = pfdLoadFile(fileName.c_str());
            
            if (root)
            {

                ConvertFromPerformer converter;
                return converter.convert(root);
            }
            else
            {
                return ReadResult::FILE_NOT_HANDLED;
            }

        }

        virtual WriteResult writeNode(const osg::Node& node,const std::string& fileName, const osgDB::ReaderWriter::Options*)
        {
            std::string ext = osgDB::getLowerCaseFileExtension(fileName);
            if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;


            osg::notify(osg::INFO)<<   "ReaderWriterPFB::writeNode( "<<fileName.c_str()<<" )\n";
            initPerformer();
            ConvertToPerformer converter;
            pfNode* root = converter.convert(&node);
            if (root)
            {
                if (pfdStoreFile(root,fileName.c_str())!=0) return WriteResult::FILE_SAVED;
                else return std::string("Unable to write file from performer.");
            }
            else
            {
                return std::string("Unable to convert scene to performer, cannot write file.");
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

    pfConfig();
}


// now register with sgRegistry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterPFB> g_readerWriter_PFB_Proxy;
