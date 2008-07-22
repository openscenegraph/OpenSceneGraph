#include "osg/ImageSequence"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool ImageSequence_readLocalData(Object& obj, Input& fr);
bool ImageSequence_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_ImageSequenceProxy
(
    new osg::ImageSequence,
    "ImageSequence",
    "Object ImageSequence",
    &ImageSequence_readLocalData,
    &ImageSequence_writeLocalData
);

bool ImageSequence_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    ImageSequence& is = static_cast<ImageSequence&>(obj);

    if (fr.matchSequence("FileNames {"))
    {
        fr += 2;
        iteratorAdvanced = true;
        int entry = fr[0].getNoNestedBrackets();
        while (!fr.eof() && fr[0].getNoNestedBrackets() >= entry)
        {
            if (fr[0].getStr())
            {
#if 1
                is.addImageFile(fr[0].getStr());
#else                           
                osg::ref_ptr<osg::Image> image = fr.readImage(fr[0].getStr());
                if (image.valid()) is.addImage(image.get());
#endif
            }
            ++fr;
        }
    }

    if (fr.matchSequence("Images {"))
    {
        fr += 2;
        iteratorAdvanced = true;
        int entry = fr[0].getNoNestedBrackets();
        while (!fr.eof() && fr[0].getNoNestedBrackets() >= entry)
        {
            if (fr[0].getStr())
            {
                osg::ref_ptr<osg::Image> image = fr.readImage(fr[0].getStr());
                if (image.valid()) is.addImage(image.get());
            }
            ++fr;
        }
    }

    return iteratorAdvanced;
}


bool ImageSequence_writeLocalData(const Object& obj, Output& fw)
{
    const ImageSequence& is = static_cast<const ImageSequence&>(obj);

    // no current image writing code here 
    // as it is all handled by osg::Registry::writeImage() via plugins.
    
    if (!is.getFileNames().empty())
    {
        fw.indent()<<"FileNames {"<<std::endl;
        fw.moveIn();

        const osg::ImageSequence::FileNames& names = is.getFileNames();
        for(osg::ImageSequence::FileNames::const_iterator itr = names.begin();
            itr != names.end();
            ++itr)
        {
            fw.indent()<<*itr<<std::endl;
        }
        
        fw.moveOut();
        fw.indent()<<"}"<<std::endl;
    }
    else 
    {
        fw.indent()<<"Images {"<<std::endl;
        fw.moveIn();
        
        const osg::ImageSequence::Images& images = is.getImages();
        for(osg::ImageSequence::Images::const_iterator itr = images.begin();
            itr != images.end();
            ++itr)
        {
            if (!(*itr)->getFileName().empty()) fw.indent()<<(*itr)->getFileName()<<std::endl;
        }
        
        fw.moveOut();
        fw.indent()<<"}"<<std::endl;
    }    

    return true;
}
