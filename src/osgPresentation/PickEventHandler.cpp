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

#include <osgPresentation/PickEventHandler>
#include <osgPresentation/SlideEventHandler>

#include <osgViewer/Viewer>
#include <osg/Notify>
#include <osgDB/FileUtils>
#include <osg/io_utils>

#include <stdlib.h>

using namespace osgPresentation;

PickEventHandler::PickEventHandler(osgPresentation::Operation operation, const JumpData& jumpData):
    _operation(operation),
    _jumpData(jumpData),
    _drawablesOnPush()
{
    OSG_INFO<<"PickEventHandler::PickEventHandler(operation="<<operation<<", jumpData.relativeJump="<<jumpData.relativeJump<<", jumpData.="<<jumpData.slideNum<<", jumpData.layerNum="<<jumpData.layerNum<<std::endl;
}

PickEventHandler::PickEventHandler(const std::string& str, osgPresentation::Operation operation, const JumpData& jumpData):
    _command(str),
    _operation(operation),
    _jumpData(jumpData),
    _drawablesOnPush()
{
    OSG_INFO<<"PickEventHandler::PickEventHandler(str="<<str<<", operation="<<operation<<", jumpData.relativeJump="<<jumpData.relativeJump<<", jumpData.="<<jumpData.slideNum<<", jumpData.layerNum="<<jumpData.layerNum<<std::endl;
}

PickEventHandler::PickEventHandler(const osgPresentation::KeyPosition& keyPos, const JumpData& jumpData):
    _keyPos(keyPos),
    _operation(osgPresentation::EVENT),
    _jumpData(jumpData),
    _drawablesOnPush()
{
    OSG_INFO<<"PickEventHandler::PickEventHandler(keyPos="<<keyPos._key<<", jumpData.relativeJump="<<jumpData.relativeJump<<", jumpData.="<<jumpData.slideNum<<", jumpData.layerNum="<<jumpData.layerNum<<std::endl;
}


bool PickEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* nv)
{
    if (ea.getHandled()) return false;

    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::MOVE):
        case(osgGA::GUIEventAdapter::PUSH):
        case(osgGA::GUIEventAdapter::DRAG):
        case(osgGA::GUIEventAdapter::RELEASE):
        {
            if(ea.getEventType() == osgGA::GUIEventAdapter::PUSH)
            {
                _drawablesOnPush.clear();
            }
            osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
            osgUtil::LineSegmentIntersector::Intersections intersections;
            if (viewer->computeIntersections(ea, nv->getNodePath(), intersections))
            {
                for(osgUtil::LineSegmentIntersector::Intersections::iterator hitr=intersections.begin();
                    hitr!=intersections.end();
                    ++hitr)
                {
                    if (_operation == FORWARD_EVENT)
                    {
                        osg::ref_ptr<osgGA::GUIEventAdapter> cloned_ea = osg::clone(&ea);
                        const osg::BoundingBox bb(hitr->drawable->getBound());
                        const osg::Vec3& p(hitr->localIntersectionPoint);
                        
                        float transformed_x = (p.x() - bb.xMin()) / (bb.xMax() - bb.xMin());
                        float transformed_y = (p.z() - bb.zMin()) / (bb.zMax() - bb.zMin());
                        
                        cloned_ea->setX(ea.getXmin() + transformed_x * (ea.getXmax() - ea.getXmin()));
                        cloned_ea->setY(ea.getYmin() + transformed_y * (ea.getYmax() - ea.getYmin()));
                        cloned_ea->setMouseYOrientation(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);
                        
                        // std::cout << transformed_x << "/" << transformed_x << " -> " << cloned_ea->getX() << "/" <<cloned_ea->getY() << std::endl;
                        
                        
                        // dispatch cloned event to devices
                        osgViewer::View::Devices& devices = viewer->getDevices();
                        for(osgViewer::View::Devices::iterator i = devices.begin(); i != devices.end(); ++i)
                        {
                            if((*i)->getCapabilities() & osgGA::Device::SEND_EVENTS)
                            {
                                (*i)->sendEvent(*cloned_ea);
                            }
                        }
                    }
                    else 
                    {
                        if (ea.getEventType()==osgGA::GUIEventAdapter::PUSH)
                        {
                            _drawablesOnPush.insert( hitr->drawable.get() );
                        }
                        else if (ea.getEventType()==osgGA::GUIEventAdapter::MOVE)
                        {
                            OSG_INFO<<"Tooltip..."<<std::endl;
                        }
                        else if (ea.getEventType()==osgGA::GUIEventAdapter::RELEASE)
                        {
                            if (_drawablesOnPush.find(hitr->drawable.get()) != _drawablesOnPush.end())
                                doOperation();
                            return true;
                        }
                    }
                }
            }
            break;
        }
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            //OSG_NOTICE<<"PickEventHandler KEYDOWN "<<(char)ea.getKey()<<std::endl;
            //if (object) OSG_NOTICE<<"    "<<object->className()<<std::endl;
            break;
        }
        default:
            break;
    }
    return false;
}


void PickEventHandler::accept(osgGA::GUIEventHandlerVisitor& v)
{
    v.visit(*this);
}

void PickEventHandler::getUsage(osg::ApplicationUsage& /*usage*/) const
{
}

void PickEventHandler::doOperation()
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
            OSG_NOTICE<<"Event "<<_keyPos._key<<" "<<_keyPos._x<<" "<<_keyPos._y<<std::endl;
            if (SlideEventHandler::instance()) SlideEventHandler::instance()->dispatchEvent(_keyPos);
            break;
        }
        case(osgPresentation::JUMP):
        {
            OSG_INFO<<"Requires jump "<<std::endl;
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
        OSG_INFO<<"No jump required."<<std::endl;
    }
}

