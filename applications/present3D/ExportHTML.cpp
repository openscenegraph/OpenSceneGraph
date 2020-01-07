/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include "ExportHTML.h"
#include <osgDB/FileNameUtils>
#include <osgDB/WriteFile>
#include <sstream>
#include <iostream>

template<typename A, typename B, typename C>
std::string createString(const A& a, const B& b, const C& c)
{
    std::ostringstream hos;
    hos << a << b << c;
    return hos.str();
}

template<typename A, typename B, typename C, typename D>
std::string createString(const A& a, const B& b, const C& c, const D& d)
{
    std::ostringstream hos;
    hos << a << b << c << d;
    return hos.str();
}

class SnapImageDrawCallback : public osg::Camera::DrawCallback
{
public:

    SnapImageDrawCallback():
        _snapImageOnNextFrame(false)
    {
    }

    void setFileName(const std::string& filename) { _filename = filename; }
    const std::string& getFileName() const { return _filename; }

    void setSnapImageOnNextFrame(bool flag) { _snapImageOnNextFrame = flag; }
    bool getSnapImageOnNextFrame() const { return _snapImageOnNextFrame; }

    virtual void operator () (const osg::Camera& camera) const
    {
        if (!_snapImageOnNextFrame) return;

        int x = static_cast<int>(camera.getViewport()->x());
        int y = static_cast<int>(camera.getViewport()->y());
        unsigned int width = static_cast<unsigned int>(camera.getViewport()->width());
        unsigned int height = static_cast<unsigned int>(camera.getViewport()->height());

        osg::ref_ptr<osg::Image> image = new osg::Image;
        image->readPixels(x,y,width,height,
                          GL_RGB,GL_UNSIGNED_BYTE);

        if (osgDB::writeImageFile(*image,_filename))
        {
            osg::notify(osg::NOTICE) << "Saved screen image to `"<<_filename<<"`"<< std::endl;
        }

        _snapImageOnNextFrame = false;
    }

protected:

    std::string _filename;
    mutable bool _snapImageOnNextFrame;


};

std::string ExportHTML::createFileName(const std::string& basename, unsigned int page, const std::string& ext)
{
    if (page==0) return basename+ext;
    else return createString(basename,'_', page, ext);
}

bool ExportHTML::write(osgPresentation::SlideEventHandler* seh, osgViewer::Viewer& viewer, const std::string& filename)
{
    std::string image_basename;
    std::string image_ext;
    std::string html_basename;
    std::string html_ext;

    std::string ext = osgDB::getFileExtension(filename);
    if (ext=="html" || ext=="htm")
    {
        image_basename = osgDB::getNameLessExtension(filename);
        image_ext = ".jpg";
        html_basename = osgDB::getNameLessExtension(filename);
        html_ext = std::string(".")+ext;
    }
    else
    {
        image_basename = osgDB::getNameLessExtension(filename);
        image_ext = ".jpg";
    }

    std::cout<<"Writing slides to "<<image_basename<<"_[slidenumber]"<<image_ext<<std::endl;

    osg::ref_ptr<SnapImageDrawCallback> sidc = new SnapImageDrawCallback;

    osgViewer::Viewer::Cameras cameras;
    viewer.getCameras(cameras);

    for(osgViewer::Viewer::Cameras::iterator itr = cameras.begin();
        itr != cameras.end();
        ++itr)
    {
        (*itr)->setPostDrawCallback(sidc.get());
    }

    std::string home_file = createFileName(html_basename, 0, html_ext);

    unsigned int i;
    for(i=0; i<seh->getNumSlides(); ++i)
    {
        std::ostringstream os;
        os << image_basename <<"_"<<i<<image_ext;

        sidc->setFileName(os.str());
        sidc->setSnapImageOnNextFrame(true);

        if (!html_basename.empty())
        {
            std::string htmlFileName = createFileName(html_basename, i, html_ext);

            std::ofstream fout(htmlFileName.c_str());
            if (fout)
            {
                std::string previous_file = i>0 ? createFileName(html_basename,i-1,html_ext) : "";
                std::string next_file = i<seh->getNumSlides()-1 ? createFileName(html_basename,i+1,html_ext) : "";

                std::cout<<"Writing html slides "<<htmlFileName<<std::endl;

                fout<<"<html>"<<std::endl;
                fout<<"<table width=\"100%\">"<<std::endl;
                fout<<"<tr>"<<std::endl;
                if (!previous_file.empty())
                {
                    fout<<"<td align=\"left\" width=\"33%\"><a href=\""<<osgDB::getSimpleFileName(previous_file)<<"\"> Previous </a></td>"<<std::endl;
                }
                else
                {
                    fout<<"<td align=\"left\" width=\"33%\"></td>"<<std::endl;
                }
                if (i != 0)
                {
                    fout<<"<td align=\"center\" width=\"33%\"><a href=\""<<osgDB::getSimpleFileName(home_file)<<"\"> Home </a></td>"<<std::endl;
                }
                else
                {
                    fout<<"<td align=\"center\" width=\"33%\"></td>"<<std::endl;
                }
                if (!next_file.empty())
                {
                    fout<<"<td align=\"right\" width=\"33%\"><a href=\""<<osgDB::getSimpleFileName(next_file)<<"\"> Next </a></td>"<<std::endl;
                }
                else
                {
                    fout<<"<td align=\"right\" width=\"33%\"></td>"<<std::endl;
                }
                fout<<"</tr>"<<std::endl;
                fout<<"</table>"<<std::endl;
                fout<<"<img src=\""<<osgDB::getSimpleFileName(os.str())<<"\">"<<std::endl;
                fout<<"</html>"<<std::endl;
            }
            else
            {
                std::cout<<"Could not open '"<<filename<<"' for writing."<<std::endl;
            }
        }
        // wait for all cull and draw threads to complete.

        seh->selectSlide(i, osgPresentation::SlideEventHandler::LAST_POSITION);

        // fire off the cull and draw traversals of the scene.
        viewer.frame();
    }
    return true;
}
