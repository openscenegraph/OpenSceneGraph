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
// PThreadConditionPrivateData.h - Private data structure for Condition
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

#ifndef _PTHREADCONDITIONPRIVATEDATA_H_
#define _PTHREADCONDITIONPRIVATEDATA_H_

#include <pthread.h>
#include <OpenThreads/Condition>

namespace OpenThreads {

class PThreadConditionPrivateData {

    friend class Condition;

private:

    PThreadConditionPrivateData() {};

    virtual ~PThreadConditionPrivateData() {};

    pthread_cond_t condition;

};

}

#endif // !_PTHREADCONDITIONPRIVATEDATA_H_
