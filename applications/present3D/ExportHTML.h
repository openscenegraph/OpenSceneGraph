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

#ifndef EXPORTHTML_H
#define EXPORTHTML_H 1

#include <osgPresentation/SlideEventHandler>
#include <osgViewer/Viewer>

class ExportHTML
{
public:
    static bool write(osgPresentation::SlideEventHandler* seh, osgViewer::Viewer& viewer, const std::string& filename);

    static std::string createFileName(const std::string& basename, unsigned int page, const std::string& ext);
};

#endif
