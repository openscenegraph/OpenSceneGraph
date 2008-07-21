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

#include <osgDB/ImagePager>

#include <osg/Notify>

using namespace osgDB;

//////////////////////////////////////////////////////////////////////////////////////
//
// ImageThread
//
ImagePager::ImageThread::ImageThread(ImagePager* pager, Mode mode, const std::string& name)
{
}

ImagePager::ImageThread::ImageThread(const ImageThread& dt, ImagePager* pager)
{
}

ImagePager::ImageThread::~ImageThread()
{
}

int ImagePager::ImageThread::cancel()
{
    return OpenThreads::Thread::cancel();
}

void ImagePager::ImageThread::run()
{
}

//////////////////////////////////////////////////////////////////////////////////////
//
// ImagePager
//
ImagePager::ImagePager()
{
}

ImagePager::~ImagePager()
{
}

bool ImagePager::requiresUpdateSceneGraph() const
{
    //osg::notify(osg::NOTICE)<<"ImagePager::requiresUpdateSceneGraph()"<<std::endl;
    return false;
}

void ImagePager::updateSceneGraph(double currentFrameTime)
{
    //osg::notify(osg::NOTICE)<<"ImagePager::updateSceneGraph(double currentFrameTime)"<<std::endl;
}

