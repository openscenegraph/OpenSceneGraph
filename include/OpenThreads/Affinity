/* -*-c++-*- OpenThreads library, Copyright (C) 2016  Robert Osfield
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
// Affinity - C++ Affinity class
// ~~~~~~~~
//

#ifndef _OPENTHREADS_AFFINITY_
#define _OPENTHREADS_AFFINITY_

#include <set>
#include <OpenThreads/Mutex>

namespace OpenThreads {

/**
 *  @class Affinity
 *  @brief  Simple container for specifying which CPU a thread should have affinity with.
 *  An empty Affinity.activeCPUs/default constructed Affinity signifies that a thread should not have any specific affinity and be able to run on all available CPUs.
 */
class Affinity
{
public:

    Affinity() {}

    Affinity(unsigned int cpuNumber) { activeCPUs.insert(cpuNumber); }

    Affinity(unsigned int cpuNumber, unsigned int cpuCount) { while(cpuCount>0) { activeCPUs.insert(cpuNumber++); --cpuCount; } }

    Affinity(const Affinity& rhs) : activeCPUs(rhs.activeCPUs) {}

    Affinity& operator = (const Affinity& rhs) { if (&rhs!=this) { activeCPUs = rhs.activeCPUs; } return *this; }


    /** add a specified cpu core from the list to have affinity to. */
    void add(unsigned int cpuNmber) { activeCPUs.insert(cpuNmber); }

    /** remove a specified cpu core from the list to have affinity to. */
    void remove(unsigned int cpuNmber) { activeCPUs.erase(cpuNmber); }

    /** return true if affinity has been provided for specific CPU cores.*/
    operator bool () const { return !activeCPUs.empty(); }

    typedef std::set<unsigned int> ActiveCPUs;

    /** Set of CPUs that a thread should have affinity to.*/
    ActiveCPUs activeCPUs;
};

}

#endif // !_OPENTHREADS_THREAD_
