#ifndef __OSG_STATESETSTR_H
#define __OSG_STATESETSTR_H

#include <osg/StateAttribute>

namespace dx {

const char *GLModeToModeStr( osg::StateAttribute::GLMode mode );

osg::StateAttribute::GLMode GLModeStrToMode( const char mode_str[] );

const char *OSGAttrToAttrStr( osg::StateAttribute::Type attr );

osg::StateAttribute::Type OSGAttrStrToAttr( const char attr_str[] );

};

#endif
