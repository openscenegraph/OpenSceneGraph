/* -*-c++-*- Present3D - Copyright (C) 1999-2006 Robert Osfield 
 *
 * This software is open source and may be redistributed and/or modified under  
 * the terms of the GNU General Public License (GPL) version 2.0.
 * The full license is in LICENSE.txt file included with this distribution,.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * include LICENSE.txt for more details.
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
