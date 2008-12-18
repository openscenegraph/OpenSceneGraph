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
REGISTER_DOTOSGWRAPPER(ImageSequence)
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

    std::string modeStr;
    if (fr.read("Mode",modeStr))
    {
        if (modeStr=="PRE_LOAD_ALL_IMAGES") 
        {
            is.setMode(osg::ImageSequence::PRE_LOAD_ALL_IMAGES);
        }
        else if (modeStr=="PAGE_AND_RETAIN_IMAGES")
        {
            is.setMode(osg::ImageSequence::PAGE_AND_RETAIN_IMAGES);
        }
        else if (modeStr=="PAGE_AND_DISCARD_USED_IMAGES")
        {
            is.setMode(osg::ImageSequence::PAGE_AND_DISCARD_USED_IMAGES);
        }
    }
    
    double length;
    if (fr.read("Duration", length) || fr.read("Length", length) )
    {
        is.setLength(length);
    }
    
    if (fr.matchSequence("FileNames {"))
    {
        fr += 2;
        iteratorAdvanced = true;
        int entry = fr[0].getNoNestedBrackets();
        while (!fr.eof() && fr[0].getNoNestedBrackets() >= entry)
        {
            if (fr[0].getStr())
            {
                is.addImageFile(fr[0].getStr());
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

    switch(is.getMode())
    {    
        case(osg::ImageSequence::PRE_LOAD_ALL_IMAGES): 
            fw.indent()<<"Mode PRE_LOAD_ALL_IMAGES"<<std::endl; 
            break;
        case(osg::ImageSequence::PAGE_AND_RETAIN_IMAGES):
            fw.indent()<<"Mode PAGE_AND_RETAIN_IMAGES"<<std::endl; 
            break;
        case(osg::ImageSequence::PAGE_AND_DISCARD_USED_IMAGES):
            fw.indent()<<"Mode PAGE_AND_DISCARD_USED_IMAGES"<<std::endl; 
            break;
    }

    fw.indent()<<"Length "<<is.getLength()<<std::endl;
    
    if (!is.getFileNames().empty())
    {
        fw.indent()<<"FileNames {"<<std::endl;
        fw.moveIn();

        const osg::ImageSequence::FileNames& names = is.getFileNames();
        for(osg::ImageSequence::FileNames::const_iterator itr = names.begin();
            itr != names.end();
            ++itr)
        {
            fw.indent()<<fw.wrapString(*itr)<<std::endl;
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
            if (!(*itr)->getFileName().empty()) fw.indent()<<fw.wrapString((*itr)->getFileName())<<std::endl;
        }
        
        fw.moveOut();
        fw.indent()<<"}"<<std::endl;
    }    

    return true;
}
