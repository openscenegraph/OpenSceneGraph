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
// PThreadBarrierPrivateData.h - private data structure for barrier
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

#ifndef _PTHREADBARRIERPRIVATEDATA_H_
#define _PTHREADBARRIERPRIVATEDATA_H_

#include <pthread.h>
#include <OpenThreads/Barrier>

namespace OpenThreads {

class PThreadBarrierPrivateData {

    friend class Barrier;

private:

    PThreadBarrierPrivateData() {};
    
    virtual ~PThreadBarrierPrivateData() {};

    pthread_cond_t     cond;            // cv for waiters at barrier

    pthread_mutex_t    lock;            // mutex for waiters at barrier

    volatile int       maxcnt;          // number of threads to wait for

    volatile int       cnt;             // number of waiting threads

    volatile int       phase;           // flag to seperate two barriers

};

}

#endif //_PTHREADBARRIERPRIVATEDATA_H_
