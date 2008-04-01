/* -*-c++-*- OpenThreads library, Copyright (C) 2002 - 2007  The Open Thread Group
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


//
// PThreadMutexPrivateData.h - Private data structure for Mutex
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//

#ifndef _PTHREADMUTEXPRIVATEDATA_H_
#define _PTHREADMUTEXPRIVATEDATA_H_

#include <pthread.h>
#include <OpenThreads/Mutex>

namespace OpenThreads {

class PThreadMutexPrivateData {

    friend class Mutex;

    friend class Condition;

private:

    PThreadMutexPrivateData() {};

    virtual ~PThreadMutexPrivateData() {};

    pthread_mutex_t mutex;

};

}

#endif // !_PTHREADMUTEXPRIVATEDATA_H_

