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
#include <osg/FrameStamp>

using namespace osg;

FrameStamp::FrameStamp():Referenced(true)
{
    _frameNumber=0;
    _referenceTime=0;
    _simulationTime=0;

    tm_sec=0;            /* Seconds.        [0-60] (1 leap second) */
    tm_min=0;            /* Minutes.        [0-59] */
    tm_hour=0;           /* Hours.          [0-23] */
    tm_mday=0;           /* Day.            [1-31] */
    tm_mon=0;            /* Month.          [0-11] */
    tm_year=0;           /* Year            - 1900.  */
    tm_wday=0;           /* Day of week.    [0-6] */
    tm_yday=0;           /* Days in year.   [0-365]    */
    tm_isdst=0;           /* DST.           [-1/0/1]*/
}

FrameStamp::FrameStamp(const FrameStamp& fs):Referenced(true)
{
    _frameNumber = fs._frameNumber;
    _referenceTime = fs._referenceTime;
    _simulationTime = fs._simulationTime;

    tm_sec = fs.tm_sec;            /* Seconds.    [0-60] (1 leap second) */
    tm_min = fs.tm_min;            /* Minutes.    [0-59] */
    tm_hour = fs.tm_hour;            /* Hours.    [0-23] */
    tm_mday = fs.tm_mday;            /* Day.        [1-31] */
    tm_mon = fs.tm_mon;            /* Month.    [0-11] */
    tm_year = fs.tm_year;            /* Year    - 1900.  */
    tm_wday = fs.tm_wday;            /* Day of week.    [0-6] */
    tm_yday = fs.tm_yday;            /* Days in year.[0-365]    */
    tm_isdst = fs.tm_isdst;            /* DST.        [-1/0/1]*/
}

FrameStamp::~FrameStamp()
{
}

FrameStamp& FrameStamp::operator = (const FrameStamp& fs)
{
    if (this==&fs) return *this;

    _frameNumber = fs._frameNumber;
    _referenceTime = fs._referenceTime;
    _simulationTime = fs._simulationTime;

    tm_sec = fs.tm_sec;            /* Seconds.    [0-60] (1 leap second) */
    tm_min = fs.tm_min;            /* Minutes.    [0-59] */
    tm_hour = fs.tm_hour;            /* Hours.    [0-23] */
    tm_mday = fs.tm_mday;            /* Day.        [1-31] */
    tm_mon = fs.tm_mon;            /* Month.    [0-11] */
    tm_year = fs.tm_year;            /* Year    - 1900.  */
    tm_wday = fs.tm_wday;            /* Day of week.    [0-6] */
    tm_yday = fs.tm_yday;            /* Days in year.[0-365]    */
    tm_isdst = fs.tm_isdst;            /* DST.        [-1/0/1]*/

    return *this;
}

void FrameStamp::setCalendarTime(const tm& ct)
{
    tm_sec = ct.tm_sec;            /* Seconds.    [0-60] (1 leap second) */
    tm_min = ct.tm_min;            /* Minutes.    [0-59] */
    tm_hour = ct.tm_hour;            /* Hours.    [0-23] */
    tm_mday = ct.tm_mday;            /* Day.        [1-31] */
    tm_mon = ct.tm_mon;            /* Month.    [0-11] */
    tm_year = ct.tm_year;            /* Year    - 1900.  */
    tm_wday = ct.tm_wday;            /* Day of week.    [0-6] */
    tm_yday = ct.tm_yday;            /* Days in year.[0-365]    */
    tm_isdst = ct.tm_isdst;            /* DST.        [-1/0/1]*/
}

void FrameStamp::getCalendarTime(tm& ct) const
{
    ct.tm_sec = tm_sec;            /* Seconds.    [0-60] (1 leap second) */
    ct.tm_min = tm_min;            /* Minutes.    [0-59] */
    ct.tm_hour = tm_hour;            /* Hours.    [0-23] */
    ct.tm_mday = tm_mday;            /* Day.        [1-31] */
    ct.tm_mon = tm_mon;            /* Month.    [0-11] */
    ct.tm_year = tm_year;            /* Year    - 1900.  */
    ct.tm_wday = tm_wday;            /* Day of week.    [0-6] */
    ct.tm_yday = tm_yday;            /* Days in year.[0-365]    */
    ct.tm_isdst = tm_isdst;            /* DST.        [-1/0/1]*/
}
