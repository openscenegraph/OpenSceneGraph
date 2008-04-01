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
// SprocConditionPrivateData.h - Private data structure for Condition
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef _SPROCCONDITIONPRIVATEDATA_H_
#define _SPROCCONDITIONPRIVATEDATA_H_

#include <unistd.h>
#include <sys/types.h>
#include <bstring.h>
#include <sys/time.h>
#include <ulocks.h>
#include <list>

#include <OpenThreads/Mutex>
#include <OpenThreads/Condition>

namespace OpenThreads {

class SemaLink {
    
    friend class SprocConditionPrivatedata;
    
    friend class Condition;

    friend class ConditionDebug;

private:

    SemaLink() {};

    virtual ~SemaLink() {};

    SemaLink *next;

    usema_t *sema;

    int select_cond;  // 0=pre-select, 1=in-select, 2=post-select
    
};

class SprocConditionPrivateData {

    friend class Condition;

private:

    SprocConditionPrivateData() {
	
	pid_list.clear();

    };

    virtual ~SprocConditionPrivateData() {

	pid_list.clear();

    };

    std::list<pid_t> pid_list;

    Mutex mutex;

};

}

#endif // !_SPROCCONDITIONPRIVATEDATA_H_
