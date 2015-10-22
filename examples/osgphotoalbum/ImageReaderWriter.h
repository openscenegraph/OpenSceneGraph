/* -*-c++-*-
*
*  OpenSceneGraph example, osgphotoalbum.
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

#ifndef IMAGEREADERWRITER_H
#define IMAGEREADERWRITER_H

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/ImageOptions>

#include <OpenThreads/ScopedLock>
#include <OpenThreads/ReentrantMutex>

#include "PhotoArchive.h"

#define SERIALIZER() OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_serializerMutex)

class ImageReaderWriter : public osgDB::ReaderWriter
{
    public:

        ImageReaderWriter();

        virtual const char* className() const { return "ImageReader"; }

        void addPhotoArchive(PhotoArchive* archive) { _photoArchiveList.push_back(archive); }

        std::string insertReference(const std::string& fileName, unsigned int res, float width, float height, bool backPage)
        {
            SERIALIZER();
            return const_cast<ImageReaderWriter*>(this)->local_insertReference(fileName, res, width, height, backPage);
        }

        virtual ReadResult readNode(const std::string& fileName, const Options* options) const
        {
            SERIALIZER();
            return const_cast<ImageReaderWriter*>(this)->local_readNode(fileName, options);
        }


    protected:

        std::string local_insertReference(const std::string& fileName, unsigned int res, float width, float height, bool backPage);

        ReadResult local_readNode(const std::string& fileName, const Options*);

        mutable OpenThreads::ReentrantMutex _serializerMutex;

        struct DataReference
        {
            DataReference();
            DataReference(const std::string& fileName, unsigned int res, float width, float height,bool backPage);
            DataReference(const DataReference& rhs);

            std::string     _fileName;
            unsigned int    _resolutionX;
            unsigned int    _resolutionY;
            osg::Vec3       _center;
            osg::Vec3       _maximumWidth;
            osg::Vec3       _maximumHeight;
            unsigned int    _numPointsAcross;
            unsigned int    _numPointsUp;
            bool            _backPage;
        };

        osg::ref_ptr<osg::Image> readImage_Archive(DataReference& dr, float& s,float& t);

        osg::ref_ptr<osg::Image> readImage_DynamicSampling(DataReference& dr, float& s,float& t);

        typedef std::map< std::string,DataReference > DataReferenceMap;
        typedef std::vector< osg::ref_ptr<PhotoArchive> > PhotoArchiveList;

        DataReferenceMap            _dataReferences;
        PhotoArchiveList            _photoArchiveList;


};

#endif
