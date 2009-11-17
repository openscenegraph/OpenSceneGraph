/**********************************************************************
 *
 *    FILE:            OcclusionQueryNode.cpp
 *
 *    DESCRIPTION:    Read/Write osg::OcclusionQueryNode in binary format to disk.
 *
 *    CREATED BY:     Copied and hacked OccluderNode.cpp by Paul Martz
 *
 *    HISTORY:        Created 2007.12.26
 *
 *    Copyright 2003 VR-C
 **********************************************************************/

#include "Exception.h"
#include "OcclusionQueryNode.h"
#include "Group.h"

using namespace ive;

void OcclusionQueryNode::write( DataOutputStream* out )
{
    // Write OcclusionQueryNode's identification.
    out->writeInt(IVEOCCLUSIONQUERYNODE);

    // If the osg class is inherited by any other class we should also write this to file.
    osg::Group*  group = dynamic_cast<osg::Group*>(this);
    if(group){
        ((ive::Group*)(group))->write(out);
    }
    else
        out_THROW_EXCEPTION("OcclusionQueryNode::write(): Could not cast this osg::OcclusionQueryNode to an osg::Group.");

    // Write OcclusionQueryNode's properties.
    out->writeBool( getQueriesEnabled() );
    out->writeUInt( getVisibilityThreshold() );
    out->writeInt( getQueryFrameCount() );
    out->writeBool( getDebugDisplay() );
}

void OcclusionQueryNode::read( DataInputStream* in )
{
    // Peek on OcclusionQueryNode's identification.
    int id = in->peekInt();
    if(id == IVEOCCLUSIONQUERYNODE)
    {
        // Read OcclusionQueryNode's identification.
        id = in->readInt();

        // If the osg class is inherited by any other class we should also read this from file.
        osg::Group*  group = dynamic_cast<osg::Group*>(this);
        if(group){
            ((ive::Group*)(group))->read(in);
        }
        else
            in_THROW_EXCEPTION("OcclusionQueryNode::read(): Could not cast this osg::OcclusionQueryNode to an osg::Group.");

        // Read OcclusionQueryNode's properties
        setQueriesEnabled( in->readBool() );
        setVisibilityThreshold( in->readUInt() );
        setQueryFrameCount( in->readInt() );
        setDebugDisplay( in->readBool() );
    }
    else{
        in_THROW_EXCEPTION("OcclusionQueryNode::read(): Expected OcclusionQueryNode identification.");
    }
}
