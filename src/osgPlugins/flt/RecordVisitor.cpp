// RecordVisitor.cpp

#include "RecordVisitor.h"
#include <stdlib.h>

using namespace flt;

RecordVisitor::RecordVisitor(TraversalMode tm)
{
    _traverseVisitor = NULL;
    _traverseMode = tm;
}


RecordVisitor::~RecordVisitor()
{
    // if (_traverseVisitor) detach from _traverseVisitor;
}


void RecordVisitor::setTraverseMode(TraversalMode mode)
{
    if (_traverseMode==mode) return;
    if (mode==TRAVERSE_VISITOR)
    {
        if (_traverseVisitor==NULL) _traverseMode = TRAVERSE_NONE;
        else _traverseMode = TRAVERSE_VISITOR;
    }
    else
    {
        if (_traverseVisitor) _traverseVisitor=NULL;
        _traverseMode = mode;
    }
}


void RecordVisitor::setTraverseVisitor(RecordVisitor* rv)
{
    if (_traverseVisitor==rv) return;
    // if (_traverseVisitor) detach from _traverseVisitor;
    _traverseVisitor = rv;
    if (_traverseVisitor) _traverseMode = TRAVERSE_VISITOR;
    else _traverseMode = TRAVERSE_NONE;
    // attach to _traverseVisitor;
}
