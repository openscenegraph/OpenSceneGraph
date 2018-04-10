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
#include <osgDB/FileUtils>
#include <osgViewer/DumpSceneHandler>

#include <iomanip>
#include <sstream>

namespace osgViewer {

DumpSceneHandler::DumpSceneHandler(const std::string &filename) :
    _useDefaultName(filename.empty()),
    _counter(0)
{
#ifdef WIN32
    std::string tempPath = getenv("TEMP");
#else
    std::string tempPath = "/tmp";
#endif
    _template = tempPath + "/scenedumpXXX.osgt" ;

    if (!filename.empty())
        _fileName = filename;
    else
        _fileName = tempPath + "/scenedump.osgt";

}

bool DumpSceneHandler::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
    if (!view) return false;

   if (ea.getKey() == osgGA::GUIEventAdapter::KEY_D && ea.getEventType() == osgGA::GUIEventAdapter::KEYUP)
    {
        if (view)
        {
            if (_useDefaultName)
            {
                do
                {
                    std::ostringstream s;
                    const std::string pattern = "XXX";
                    s << std::setfill('0') << std::setw(pattern.size()) << _counter++;
                    const std::string replace = s.str();
                    std::string tempFile = _template;
                    tempFile.replace(tempFile.find(pattern), pattern.length(), replace);
                    _fileName = tempFile;
                } while (osgDB::fileExists(_fileName));
            }
            OSG_INFO << "dumping scene to '" << _fileName << "'" << std::endl;
            return osgDB::writeNodeFile(*view->getSceneData(), _fileName) ? false : true;
        }
        return true;
    }
    return false;
}

void DumpSceneHandler::setFileName(const std::string &filename)
{
    _useDefaultName = filename.empty();
    _fileName = filename;
}

void DumpSceneHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding('d',"Dump scene to file '" + (_useDefaultName ? _template : _fileName) + "'");
}

}
