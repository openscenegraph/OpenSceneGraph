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

#include <osgPresentation/KeyEventHandler>
#include <osgPresentation/SlideEventHandler>

#include <osgViewer/Viewer>
#include <osg/Notify>
#include <osgDB/FileUtils>

#include <stdlib.h>

using namespace osgPresentation;

KeyEventHandler::KeyEventHandler(int key, osgPresentation::Operation operation, const JumpData& jumpData):
    _key(key),
    _operation(operation),
    _jumpData(jumpData)
{
}

KeyEventHandler::KeyEventHandler(int key, const std::string& str, osgPresentation::Operation operation, const JumpData& jumpData):
    _key(key),
    _command(str),
    _operation(operation),
    _jumpData(jumpData)
{
}

KeyEventHandler::KeyEventHandler(int key, const osgPresentation::KeyPosition& keyPos, const JumpData& jumpData):
    _key(key),
    _keyPos(keyPos),
    _operation(osgPresentation::EVENT),
    _jumpData(jumpData)
{
}

bool KeyEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& /*aa*/, osg::Object*, osg::NodeVisitor* /*nv*/)
{
    if (ea.getHandled()) return false;

    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            if (ea.getKey()==_key)
            {
                doOperation();
                return true;
            }
            break;
        }
        default:
            break;
    }
    return false;
}


void KeyEventHandler::accept(osgGA::GUIEventHandlerVisitor& v)
{
    v.visit(*this);
}

void KeyEventHandler::getUsage(osg::ApplicationUsage& /*usage*/) const
{
}

void KeyEventHandler::doOperation()
{
    switch(_operation)
    {
        case(osgPresentation::RUN):
        {
            OSG_NOTICE<<"Run "<<_command<<std::endl;

#if 0
            osgDB::FilePathList& paths = osgDB::getDataFilePathList();
            if (!paths.empty())
            {
            #ifdef _WIN32
                std::string delimintor(";");
            #else
                std::string delimintor(":");
            #endif
                std::string filepath("OSG_FILE_PATH=");

                bool needDeliminator = false;
                for(osgDB::FilePathList::iterator itr = paths.begin();
                    itr != paths.end();
                    ++itr)
                {
                    if (needDeliminator) filepath += delimintor;
                    filepath += *itr;
                    needDeliminator = true;
                }
                putenv( (char*) filepath.c_str());

                std::string binpath("PATH=");
                char* path = getenv("PATH");
                if (path) binpath += path;

                needDeliminator = true;
                for(osgDB::FilePathList::iterator itr = paths.begin();
                    itr != paths.end();
                    ++itr)
                {
                    if (needDeliminator) binpath += delimintor;
                    binpath += *itr;
                    needDeliminator = true;
                }
                putenv( (char*) binpath.c_str());

            }
#endif

            bool commandRunsInBackground = (_command.find("&")!=std::string::npos);

            int result = system(_command.c_str());

            OSG_INFO<<"system("<<_command<<") result "<<result<<std::endl;

            if (commandRunsInBackground)
            {
                // Sleep breifly while command runs in background to give it a chance to open
                // a window and obscure this present3D's window avoiding this present3D from
                // rendering anything new before the new window opens.
                OpenThreads::Thread::microSleep(500000); // half second sleep.
            }

            break;
        }
        case(osgPresentation::LOAD):
        {
            OSG_NOTICE<<"Load "<<_command<<std::endl;
            break;
        }
        case(osgPresentation::EVENT):
        {
            OSG_INFO<<"Event "<<_keyPos._key<<" "<<_keyPos._x<<" "<<_keyPos._y<<std::endl;
            if (SlideEventHandler::instance()) SlideEventHandler::instance()->dispatchEvent(_keyPos);
            break;
        }
        case(osgPresentation::JUMP):
        {
            OSG_NOTICE<<"Requires jump "<<std::endl;
            break;
        }
        case(osgPresentation::FORWARD_EVENT):
            break;
    }

    if (_jumpData.requiresJump())
    {
        _jumpData.jump(SlideEventHandler::instance());
    }
    else
    {
        OSG_NOTICE<<"No jump required."<<std::endl;
    }
}

