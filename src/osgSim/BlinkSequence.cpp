//C++ header - Open Scene Graph Simulation - Copyright (C) 1998-2002 Robert Osfield
// Distributed under the terms of the GNU General Public License (GPL)
// as published by the Free Software Foundation.
//
// All software using osgSim must be GPL'd or excempted via the 
// purchase of the Open Scene Graph Professional License (OSGPL)
// for further information contact robert@openscenegraph.com.

#include <osgSim/BlinkSequence>

#include <stdlib.h>

using namespace osgSim;

BlinkSequence::BlinkSequence():
    _pulsePeriod(0.0),           
    _phaseShift(0.0),
    _pulseData(),
    _sequenceGroup(0)
{
}

BlinkSequence::BlinkSequence(const BlinkSequence& bs):
    Referenced(),
    _pulsePeriod(bs._pulsePeriod),
    _phaseShift(bs._phaseShift),
    _pulseData(bs._pulseData),
    _sequenceGroup(bs._sequenceGroup)
{
}


BlinkSequence::SequenceGroup::SequenceGroup()
{
    // set a random base time between 0 and 1000.0
    _baseTime = ((double)rand()/(double)RAND_MAX)*1000.0;
}

BlinkSequence::SequenceGroup::SequenceGroup(double baseTime):
    _baseTime(baseTime)
{
}
    
