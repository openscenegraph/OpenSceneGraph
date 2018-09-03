/*
 * OsgAndroidNotifyHandler.hpp
 *
 *  Created on: 31/05/2011
 *      Author: Jorge Izquierdo Ciges
 */

#ifndef OSGANDROIDNOTIFYHANDLER_HPP_
#define OSGANDROIDNOTIFYHANDLER_HPP_

#include <android/log.h>

#include <osg/Notify>

#include <string>

class OSG_EXPORT OsgAndroidNotifyHandler : public osg::NotifyHandler
{
private:
    std::string _tag;
public:
    void setTag(std::string tag);
    void notify(osg::NotifySeverity severity, const char *message);
};

#endif /* OSGANDROIDNOTIFYHANDLER_HPP_ */
