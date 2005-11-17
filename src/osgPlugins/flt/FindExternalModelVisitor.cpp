// FindExternalModelVisitor.cpp

#include "FindExternalModelVisitor.h"

using namespace flt;


/**
 *
 */
void FindExternalModelVisitor::apply( osg::Node &node )
{
  if ( node.getName() == _modelName )
    _model = &node;   // Store the node.  No need to process children.
  else
      traverse( node );
}

