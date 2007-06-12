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

#ifndef PHOTOARCHIVE_H
#define PHOTOARCHIVE_H

#include <osg/Image>

class PhotoArchive : public osg::Referenced
{
public:

    static PhotoArchive* open(const std::string& filename)
    {
        osg::ref_ptr<PhotoArchive> archive = new PhotoArchive(filename);
        if (!archive->empty()) return archive.release();
        else return 0;
    }

    typedef std::vector<std::string> FileNameList;
    
    bool empty() { return _photoIndex.empty(); }

    void getImageFileNameList(FileNameList& filenameList);
    
    static void buildArchive(const std::string& filename, const FileNameList& imageList, unsigned int thumbnailSize=256, unsigned int maximumSize=1024, bool compressed=true);

    osg::Image* readImage(const std::string& filename,
                          unsigned int target_s, unsigned target_t,
                          float& original_s, float& original_t);
                          
    
    
protected:

    PhotoArchive(const std::string& filename);

    virtual ~PhotoArchive() {}

    bool readPhotoIndex(const std::string& filename);
    
    struct PhotoHeader
    {
        PhotoHeader():
            original_s(0),
            original_t(0),
            thumbnail_s(0),
            thumbnail_t(0),
            thumbnail_position(0),
            fullsize_s(0),
            fullsize_t(0),
            fullsize_position(0)
        {
            filename[0]='\0';
        }
    
        char filename[256];
        unsigned int original_s;
        unsigned int original_t;

        unsigned int thumbnail_s;
        unsigned int thumbnail_t;
        unsigned int thumbnail_position;

        unsigned int fullsize_s;
        unsigned int fullsize_t;
        unsigned int fullsize_position;
    };

    
    struct ImageHeader
    {
        ImageHeader():
            s(0),
            t(0),
            internalTextureformat(0),
            pixelFormat(0),
            type(0),
            size(0) {}
    
        unsigned int s;
        unsigned int t;
        GLint internalTextureformat;
        GLenum pixelFormat;
        GLenum type;
        unsigned int size;
    };


    typedef std::vector<PhotoHeader> PhotoIndexList;

    std::string     _archiveFileName;
    PhotoIndexList  _photoIndex;    

};

#endif
