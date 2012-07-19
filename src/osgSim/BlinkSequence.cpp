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

BlinkSequence::BlinkSequence(const BlinkSequence& bs, const osg::CopyOp& copyop):
    osg::Object(bs,copyop),
    _pulsePeriod(bs._pulsePeriod),
    _phaseShift(bs._phaseShift),
    _pulseData(bs._pulseData),
    _sequenceGroup(bs._sequenceGroup)
{
}


SequenceGroup::SequenceGroup()
{
    // set a random base time between 0 and 1000.0
    _baseTime = ((double)rand()/(double)RAND_MAX)*1000.0;
}

SequenceGroup::SequenceGroup(const SequenceGroup& sg, const osg::CopyOp& copyop):
    osg::Object(sg, copyop),
    _baseTime(sg._baseTime)
{
}

SequenceGroup::SequenceGroup(double baseTime):
    _baseTime(baseTime)
{
}

