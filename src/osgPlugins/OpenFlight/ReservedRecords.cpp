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

//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
//

#include "Registry.h"

using namespace flt;

// Prevent "unknown record" message for the following reserved records:
RegisterRecordProxy<DummyRecord> g_Reserved_103(103);
RegisterRecordProxy<DummyRecord> g_Reserved_104(104);
RegisterRecordProxy<DummyRecord> g_Reserved_117(117);
RegisterRecordProxy<DummyRecord> g_Reserved_118(118);
RegisterRecordProxy<DummyRecord> g_Reserved_120(120);
RegisterRecordProxy<DummyRecord> g_Reserved_121(121);
RegisterRecordProxy<DummyRecord> g_Reserved_124(124);
RegisterRecordProxy<DummyRecord> g_Reserved_125(125);




