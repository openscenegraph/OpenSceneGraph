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

using namespace osgPresentation;

PickEventHandler::PickEventHandler(osgPresentation::Operation operation,bool relativeJump, int slideNum, int layerNum):
    _operation(operation),
    _relativeJump(relativeJump),
    _slideNum(slideNum),
    _layerNum(layerNum)
{
}

PickEventHandler::PickEventHandler(const std::string& str, osgPresentation::Operation operation,bool relativeJump, int slideNum, int layerNum):
    _command(str),
    _operation(operation),
    _relativeJump(relativeJump),
    _slideNum(slideNum),
    _layerNum(layerNum)
{
}

PickEventHandler::PickEventHandler(const osgPresentation::KeyPosition& keyPos,bool relativeJump, int slideNum, int layerNum):
    _keyPos(keyPos),
    _operation(osgPresentation::EVENT),
    _relativeJump(relativeJump),
    _slideNum(slideNum),
    _layerNum(layerNum)
{
}


bool PickEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* nv)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::MOVE):
        case(osgGA::GUIEventAdapter::PUSH):
        case(osgGA::GUIEventAdapter::RELEASE):
        {
            osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
            osgUtil::LineSegmentIntersector::Intersections intersections;
            if (viewer->computeIntersections(ea.getX(),ea.getY(), nv->getNodePath(), intersections))
            {
                for(osgUtil::LineSegmentIntersector::Intersections::iterator hitr=intersections.begin();
                    hitr!=intersections.end();
                    ++hitr)
                {
                    if (ea.getEventType()==osgGA::GUIEventAdapter::MOVE)
                    {
                        osg::notify(osg::INFO)<<"Tooltip..."<<std::endl;
                    }
                    else if (ea.getEventType()==osgGA::GUIEventAdapter::RELEASE)
                    {
                        doOperation();
                        return true;
                    }
                }
            }
            break;
        }
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            //osg::notify(osg::NOTICE)<<"PickEventHandler KEYDOWN "<<(char)ea.getKey()<<std::endl;
            //if (object) osg::notify(osg::NOTICE)<<"    "<<object->className()<<std::endl;
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

void PickEventHandler::setRelativeJump(int slideNum, int layerNum)
{
    _relativeJump = true;
    _slideNum = slideNum;
    _layerNum = layerNum;
}

void PickEventHandler::setAbsoluteJump(int slideNum, int layerNum)
{
    _relativeJump = false;
    _slideNum = slideNum;
    _layerNum = layerNum;
}


void PickEventHandler::doOperation()
{
    switch(_operation)
    {
        case(osgPresentation::RUN):
        {
            osg::notify(osg::NOTICE)<<"Run "<<_command<<std::endl;

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
            int result = system(_command.c_str());

            osg::notify(osg::INFO)<<"system("<<_command<<") result "<<result<<std::endl;

            break;
        }
        case(osgPresentation::LOAD):
        {
            osg::notify(osg::NOTICE)<<"Load "<<_command<<std::endl;
            break;
        }
        case(osgPresentation::EVENT):
        {
            osg::notify(osg::INFO)<<"Event "<<_keyPos._key<<" "<<_keyPos._x<<" "<<_keyPos._y<<std::endl;
            if (SlideEventHandler::instance()) SlideEventHandler::instance()->dispatchEvent(_keyPos);
            break;
        }
        case(osgPresentation::JUMP):
        {
            osg::notify(osg::NOTICE)<<"Requires jump "<<std::endl;
            break;
        }
    }
    
    if (requiresJump())
    {
        osg::notify(osg::NOTICE)<<"Requires jump "<<_relativeJump<<", "<<_slideNum<<", "<<_layerNum<<std::endl;

        if (_relativeJump)
        {
            int previousSlide = SlideEventHandler::instance()->getActiveSlide();
            int previousLayer = SlideEventHandler::instance()->getActiveLayer();
            int newSlide = previousSlide + _slideNum;
            int newLayer = previousLayer + _layerNum;
            if (newLayer<0) 
            {
                newLayer = 0;
            }

            osg::notify(osg::NOTICE)<<"   jump to "<<newSlide<<", "<<newLayer<<std::endl;

            SlideEventHandler::instance()->selectSlide(newSlide, newLayer);
        }
        else
        {
            SlideEventHandler::instance()->selectSlide(_slideNum,_layerNum);
        }
    }
    else
    {
        osg::notify(osg::NOTICE)<<"No jump required."<<std::endl;
    }
}

