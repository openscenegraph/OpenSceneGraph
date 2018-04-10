/* -*-c++-*- OpenSceneGraph - Copyright (C) 2018 Ralf Habacker, Daniel Wendt
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

#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>
#include <osgViewer/DumpSceneHandler>

#include <iomanip>
#include <sstream>

namespace osgViewer {

DumpSceneHandler::DumpSceneHandler(const std::string &filename) :
    _filename(filename),
    _autoinc(0)
{
}

std::string DumpSceneHandler::generateFileName() const
{
    std::stringstream ss;
    ss << osgDB::getNameLessExtension(_filename);
    if (_autoinc != -1)
    {
        ss << "_"<<std::setfill('0') << std::setw(2) << _autoinc;
    }
    ss << "."<<osgDB::getFileExtension(_filename);
    return ss.str();
}

bool DumpSceneHandler::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
    if (!view)
        return false;

   if (ea.getKey() == osgGA::GUIEventAdapter::KEY_D && ea.getEventType() == osgGA::GUIEventAdapter::KEYUP)
    {
        if (_filename.empty())
            return false;

        std::string filename = generateFileName();
        if (_autoinc != -1)
            _autoinc++;
        OSG_INFO << "dumping scene to '" << filename << "'" << std::endl;
        return osgDB::writeNodeFile(*view->getSceneData(), filename) ? false : true;
    }
    return false;
}

void DumpSceneHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding('d', "Dump scene to file.");
}

}
