// FindExternalModelVisitor.h

#ifndef __FIND_EXTERNAL_MODEL_VISITOR_H_
#define __FIND_EXTERNAL_MODEL_VISITOR_H_

#include <iostream>
#include <osg/Node>
#include <osg/NodeVisitor>

namespace flt {

/**
 * A visitor to find a named node.
 */
class FindExternalModelVisitor : public osg::NodeVisitor
{
public:
	FindExternalModelVisitor( ) : osg::NodeVisitor( TRAVERSE_ALL_CHILDREN ) {}

	virtual void apply( osg::Node &node );

	void setModelName( std::string modelName ) { _modelName = modelName; }

  osg::Node *getModel() { return _model.get(); }

private:

  std::string              _modelName;
  osg::ref_ptr<osg::Node>  _model;

};

}; // end namespace flt

#endif // __FIND_EXTERNAL_MODEL_VISITOR_H_

