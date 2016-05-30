/* OpenSceneGraph example, osgphotoalbum.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include "PhotoArchive.h"

#include <osg/GLU>
#include <osg/Notify>
#include <osgDB/ReadFile>
#include <osgDB/fstream>

#include <osg/GraphicsContext>

#include <iostream>
#include <string.h>

const std::string FILE_IDENTIFER("osgphotoalbum photo archive");

PhotoArchive::PhotoArchive(const std::string& filename)
{
    readPhotoIndex(filename);
}

bool PhotoArchive::readPhotoIndex(const std::string& filename)
{
    osgDB::ifstream in(filename.c_str());

    char* fileIndentifier = new char [FILE_IDENTIFER.size()];
    in.read(fileIndentifier,FILE_IDENTIFER.size());
    if (FILE_IDENTIFER!=fileIndentifier)
    {
        delete [] fileIndentifier;
        return false;
    }
    delete [] fileIndentifier;

    unsigned int numPhotos;
    in.read((char*)&numPhotos,sizeof(numPhotos));

    _photoIndex.resize(numPhotos);

    in.read((char*)&_photoIndex.front(),sizeof(PhotoHeader)*numPhotos);

    // success record filename.
    _archiveFileName = filename;

    return true;
}

void PhotoArchive::getImageFileNameList(FileNameList& filenameList)
{
    for(PhotoIndexList::const_iterator itr=_photoIndex.begin();
        itr!=_photoIndex.end();
        ++itr)
    {
        filenameList.push_back(std::string(itr->filename));
    }

}

osg::ref_ptr<osg::Image> PhotoArchive::readImage(const std::string& filename,
                                    unsigned int target_s, unsigned target_t,
                                    float& original_s, float& original_t)
{
    for(PhotoIndexList::const_iterator itr=_photoIndex.begin();
        itr!=_photoIndex.end();
        ++itr)
    {
        if (filename==itr->filename)
        {
            const PhotoHeader& photoHeader = *itr;

            if  (target_s <= photoHeader.thumbnail_s &&
                 target_t <= photoHeader.thumbnail_t &&
                 photoHeader.thumbnail_position != 0)
            {
                osgDB::ifstream in(_archiveFileName.c_str(),std::ios::in | std::ios::binary);

                // find image
                in.seekg(photoHeader.thumbnail_position);

                // read image header
                ImageHeader imageHeader;
                in.read((char*)&imageHeader,sizeof(ImageHeader));
                unsigned char* data = new unsigned char[imageHeader.size];
                in.read((char*)data,imageHeader.size);

                osg::ref_ptr<osg::Image> image = new osg::Image;
                image->setImage(photoHeader.thumbnail_s,photoHeader.thumbnail_t,1,
                                imageHeader.pixelFormat,imageHeader.pixelFormat,imageHeader.type,
                                data,osg::Image::USE_NEW_DELETE);

                original_s =  photoHeader.original_s;
                original_t =  photoHeader.original_t;

                return image;
            }

            if  (photoHeader.fullsize_s &&
                 photoHeader.fullsize_t &&
                 photoHeader.fullsize_position != 0)
            {
                osgDB::ifstream in(_archiveFileName.c_str(),std::ios::in | std::ios::binary);

                // find image
                in.seekg(photoHeader.fullsize_position);

                // read image header
                ImageHeader imageHeader;
                in.read((char*)&imageHeader,sizeof(ImageHeader));
                unsigned char* data = new unsigned char[imageHeader.size];
                in.read((char*)data,imageHeader.size);

                osg::ref_ptr<osg::Image> image = new osg::Image;
                image->setImage(photoHeader.fullsize_s,photoHeader.fullsize_t,1,
                                imageHeader.pixelFormat,imageHeader.pixelFormat,imageHeader.type,
                                data,osg::Image::USE_NEW_DELETE);

                original_s =  photoHeader.original_s;
                original_t =  photoHeader.original_t;

                return image;
           }

       }

    }
    return NULL;
}

void PhotoArchive::buildArchive(const std::string& filename, const FileNameList& imageList, unsigned int thumbnailSize, unsigned int maximumSize, bool /*compressed*/)
{

    PhotoIndexList photoIndex;
    photoIndex.reserve(imageList.size());
    for(FileNameList::const_iterator fitr=imageList.begin();
        fitr!=imageList.end();
        ++fitr)
    {
        PhotoHeader header;

        // set name
        strncpy(header.filename,fitr->c_str(),255);
        header.filename[255]=0;

        header.thumbnail_s = thumbnailSize;
        header.thumbnail_t = thumbnailSize;
        header.thumbnail_position = 0;

        header.fullsize_s = thumbnailSize;
        header.fullsize_t = thumbnailSize;
        header.fullsize_position = 0;

        photoIndex.push_back(header);

    }

    std::cout<<"Building photo archive containing "<<photoIndex.size()<<" pictures"<<std::endl;

    // open up the archive for writing to
    osgDB::ofstream out(filename.c_str(), std::ios::out | std::ios::binary);

    // write out file identifier.
    out.write(FILE_IDENTIFER.c_str(),FILE_IDENTIFER.size());

    unsigned int numPhotos = photoIndex.size();
    out.write((char*)&numPhotos,sizeof(unsigned int));

    // write the photo index to ensure we can the correct amount of space
    // available.
    unsigned int startOfPhotoIndex = out.tellp();
    out.write((char*)&photoIndex.front(),sizeof(PhotoHeader)*photoIndex.size());

    unsigned int photoCount=1;
    for(PhotoIndexList::iterator pitr=photoIndex.begin();
        pitr!=photoIndex.end();
        ++pitr,++photoCount)
    {
        PhotoHeader& photoHeader = *pitr;


        std::cout<<"Processing image "<<photoCount<<" of "<< photoIndex.size()<<" filename="<< photoHeader.filename << std::endl;
        std::cout<<"    reading image...";std::cout.flush();

        osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile(photoHeader.filename);

        std::cout<<"done."<< std::endl;

        photoHeader.original_s = image->s();
        photoHeader.original_t = image->t();

        {

            std::cout<<"    creating thumbnail image...";
            // first need to rescale image to the require thumbnail size
            unsigned int newTotalSize =
                image->computeRowWidthInBytes(thumbnailSize,image->getPixelFormat(),image->getDataType(),image->getPacking())*
                thumbnailSize;

            // need to sort out what size to really use...
            unsigned char* newData = new unsigned char [newTotalSize];
            if (!newData)
            {
                // should we throw an exception???  Just return for time being.
                osg::notify(osg::FATAL) << "Error scaleImage() did not succeed : out of memory."<<newTotalSize<<std::endl;
                return;
            }

            osg::PixelStorageModes psm;
            psm.pack_alignment = image->getPacking();
            psm.pack_row_length = image->getRowLength();
            psm.unpack_alignment = image->getPacking();

            GLint status = osg::gluScaleImage(&psm, image->getPixelFormat(),
                image->s(),
                image->t(),
                image->getDataType(),
                image->data(),
                thumbnailSize,
                thumbnailSize,
                image->getDataType(),
                newData);

            if (status!=0)
            {
                delete [] newData;
                osg::notify(osg::WARN) << "Error scaleImage() did not succeed : errorString = "<<osg::gluErrorString((GLenum)status)<<std::endl;
                return;
            }

            // now set up the photo header.
            photoHeader.thumbnail_s = thumbnailSize;
            photoHeader.thumbnail_t = thumbnailSize;
            photoHeader.thumbnail_position = (unsigned int)out.tellp();

            // set up image header
            ImageHeader imageHeader;
            imageHeader.s = thumbnailSize;
            imageHeader.t = thumbnailSize;
            imageHeader.internalTextureformat = image->getInternalTextureFormat();
            imageHeader.pixelFormat = image->getPixelFormat();
            imageHeader.type = image->getDataType();
            imageHeader.size = newTotalSize;

            // write out image header and image data.
            out.write((char*)&imageHeader,sizeof(ImageHeader));
            out.write((char*)newData,imageHeader.size);

            delete [] newData;

            std::cout<<"done."<< std::endl;

        }

        {
            std::cout<<"    creating fullsize image...";std::cout.flush();


            photoHeader.fullsize_s = osg::minimum((unsigned int)image->s(),maximumSize);
            photoHeader.fullsize_t = osg::minimum((unsigned int)image->t(),maximumSize);
            photoHeader.fullsize_position = (unsigned int)out.tellp();

            // first need to rescale image to the require thumbnail size
            unsigned int newTotalSize =
                image->computeRowWidthInBytes(photoHeader.fullsize_s,image->getPixelFormat(),image->getDataType(),image->getPacking())*
                photoHeader.fullsize_t;

            // need to sort out what size to really use...
            unsigned char* newData = new unsigned char [newTotalSize];
            if (!newData)
            {
                // should we throw an exception???  Just return for time being.
                osg::notify(osg::FATAL) << "Error scaleImage() did not succeed : out of memory."<<newTotalSize<<std::endl;
                return;
            }

            osg::PixelStorageModes psm;
            psm.pack_alignment = image->getPacking();
            psm.pack_row_length = image->getRowLength();
            psm.unpack_alignment = image->getPacking();

            GLint status = osg::gluScaleImage(&psm, image->getPixelFormat(),
                image->s(),
                image->t(),
                image->getDataType(),
                image->data(),
                photoHeader.fullsize_s,
                photoHeader.fullsize_t,
                image->getDataType(),
                newData);

            if (status!=0)
            {
                delete [] newData;
                osg::notify(osg::WARN) << "Error scaleImage() did not succeed : errorString = "<<osg::gluErrorString((GLenum)status)<<std::endl;
                return;
            }

            ImageHeader imageHeader;
            imageHeader.s = photoHeader.fullsize_s;
            imageHeader.t = photoHeader.fullsize_t;
            imageHeader.internalTextureformat = image->getInternalTextureFormat();
            imageHeader.pixelFormat = image->getPixelFormat();
            imageHeader.type = image->getDataType();
            imageHeader.size = newTotalSize;

            out.write((char*)&imageHeader,sizeof(ImageHeader));
            out.write((char*)newData,imageHeader.size);
            //out.write((char*)image->data(),imageHeader.size);

            delete [] newData;

            std::cout<<"done."<< std::endl;
        }

    }

    // rewrite photo index now it has the correct sizes set
    out.seekp(startOfPhotoIndex);
    out.write((char*)&photoIndex.front(),sizeof(PhotoHeader)*photoIndex.size());

}
