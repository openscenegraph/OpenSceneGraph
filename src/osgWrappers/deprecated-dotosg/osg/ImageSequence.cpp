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
        else if (modeStr=="LOAD_AND_RETAIN_IN_UPDATE_TRAVERSAL")
        {
            is.setMode(osg::ImageSequence::LOAD_AND_RETAIN_IN_UPDATE_TRAVERSAL);
        }
        else if (modeStr=="LOAD_AND_DISCARD_IN_UPDATE_TRAVERSAL")
        {
            is.setMode(osg::ImageSequence::LOAD_AND_DISCARD_IN_UPDATE_TRAVERSAL);
        }
    }

    double length;
    if (fr.read("Duration", length) || fr.read("Length", length) )
    {
        is.setLength(length);
    }

    if (fr.matchSequence("FileNames {"))
    {
        if (fr.getOptions()) is.setReadOptions(new osgDB::Options(*fr.getOptions()));
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
        case(osg::ImageSequence::LOAD_AND_RETAIN_IN_UPDATE_TRAVERSAL):
            fw.indent()<<"Mode LOAD_AND_RETAIN_IN_UPDATE_TRAVERSAL"<<std::endl;
            break;
        case(osg::ImageSequence::LOAD_AND_DISCARD_IN_UPDATE_TRAVERSAL):
            fw.indent()<<"Mode LOAD_AND_DISCARD_IN_UPDATE_TRAVERSAL"<<std::endl;
            break;
    }

    fw.indent()<<"Length "<<is.getLength()<<std::endl;

    if (is.getNumImageData()>0)
    {
        fw.indent()<<"FileNames {"<<std::endl;
        fw.moveIn();

        const osg::ImageSequence::ImageDataList& id = is.getImageDataList();
        for(osg::ImageSequence::ImageDataList::const_iterator itr = id.begin();
            itr != id.end();
            ++itr)
        {
            fw.indent()<<fw.wrapString(itr->_filename)<<std::endl;
        }

        fw.moveOut();
        fw.indent()<<"}"<<std::endl;
    }

    return true;
}
