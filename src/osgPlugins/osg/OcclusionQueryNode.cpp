//
// Copyright (C) 2007 Skew Matrix Software LLC (http://www.skew-matrix.com)
//
// This library is open source and may be redistributed and/or modified under  
// the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
// (at your option) any later version.  The full license is in LICENSE file
// included with this distribution, and on the openscenegraph.org website.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
// OpenSceneGraph Public License for more details.
//

#include <osg/OcclusionQueryNode>

#include <iostream>
#include <string>

#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

bool OQN_readLocalData( osg::Object &obj, osgDB::Input &fr );
bool OQN_writeLocalData( const osg::Object &obj, osgDB::Output &fw );

osgDB::RegisterDotOsgWrapperProxy OcclusionQueryNode_Proxy
(
    new osg::OcclusionQueryNode,
    "OcclusionQueryNode",
    "Object Node OcclusionQueryNode Group",
    OQN_readLocalData,
    OQN_writeLocalData
);

bool OQN_readLocalData( osg::Object &obj, osgDB::Input &fr )
{
    osg::OcclusionQueryNode& oqn = static_cast<osg::OcclusionQueryNode&>( obj );
    bool advanced( false );

    if (fr[0].matchWord( "QueriesEnabled" ))
    {
        bool enable( fr[1].getStr() == std::string("TRUE") );
        oqn.setQueriesEnabled( enable );
        fr+=2;
        advanced = true;
    }

    return advanced;
}

bool OQN_writeLocalData( const osg::Object &obj, osgDB::Output &fw )
{
    const osg::OcclusionQueryNode& oqn = static_cast<const osg::OcclusionQueryNode&>( obj );

    //fw.writeObject( oqn.getOQN(i));

    fw.indent() << "QueriesEnabled " <<
        (oqn.getQueriesEnabled() ? "TRUE" : "FALSE")
        << std::endl;

    return true;
}
