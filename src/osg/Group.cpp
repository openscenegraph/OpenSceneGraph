#include <stdio.h>
#include <math.h>
#include "osg/Group"
#include "osg/Input"
#include "osg/Output"
#include "osg/Registry"
#include "osg/BoundingBox"

#include <algorithm>

// #ifdef __sgi
// using std::find;
// using std::for_each;
// using std::string;
// #endif

#define square(x)   ((x)*(x))

using namespace osg;

RegisterObjectProxy<Group> g_GroupProxy;

Group::Group()
{
}


Group::~Group()
{

    for(ChildList::iterator itr=_children.begin();
                            itr!=_children.end();
                            ++itr)
    {
        Node* child = itr->get();
        ParentList::iterator pitr = std::find(child->_parents.begin(),child->_parents.end(),this);
        if (pitr!=child->_parents.end()) child->_parents.erase(pitr);
    }

}


void Group::traverse(NodeVisitor& nv)
{
    for(ChildList::iterator itr=_children.begin();
                            itr!=_children.end();
                            ++itr)
    {
        (*itr)->accept(nv);
    }
}


bool Group::addChild( Node *child )
{
    if (child && !containsNode(child))
    {
        // note ref_ptr<> automatically handles incrementing child's reference count.
        _children.push_back(child);

        // register as parent of child.
        child->_parents.push_back(this);

        dirtyBound();

        return true;
    }
    else return false;
}

bool Group::removeChild( Node *child )
{
    ChildList::iterator itr = findNode(child);
    if (itr!=_children.end())
    {
        // note ref_ptr<> automatically handles decrementing child's reference count.
        _children.erase(itr);
        dirtyBound();

        ParentList::iterator pitr = std::find(child->_parents.begin(),child->_parents.end(),child);
        if (pitr!=child->_parents.end()) child->_parents.erase(pitr);

        return true;
    }
    else return false;
}

bool Group::replaceChild( Node *origNode, Node *newNode )
{
    if (newNode==NULL || origNode==newNode) return false;

    ChildList::iterator itr = findNode(origNode);
    if (itr!=_children.end())
    {
        ParentList::iterator pitr = std::find(origNode->_parents.begin(),origNode->_parents.end(),origNode);
        if (pitr!=origNode->_parents.end()) origNode->_parents.erase(pitr);

        // note ref_ptr<> automatically handles decrementing origNode's reference count,
        // and inccrementing newNode's reference count.
        *itr = newNode;

        // register as parent of child.
        newNode->_parents.push_back(this);

        dirtyBound();
        return true;
    }
    else return false;
    
}

bool Group::readLocalData(Input& fr)
{
    bool iteratorAdvanced = false;
    if (Node::readLocalData(fr)) iteratorAdvanced = true;

    int num_children;
    if (fr[0].matchWord("num_children") &&
        fr[1].getInt(num_children))
    {
        // could allocate space for children here...
        fr+=2;
        iteratorAdvanced = true;
    }

    Node* node = NULL;
    while((node=fr.readNode())!=NULL)
    {
        addChild(node);
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool Group::writeLocalData(Output& fw)
{
    Node::writeLocalData(fw);

    fw.indent() << "num_children " << getNumChildren() << endl;
    for(int i=0;i<getNumChildren();++i)
    {
        getChild(i)->write(fw);
    }
    return true;
}


bool Group::computeBound()
{

    _bsphere_computed = true;

    _bsphere.init();
    if (_children.empty()) return false;
    
    BoundingBox bb;
    bb.init();
    ChildList::iterator itr;
    for(itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        bb.expandBy((*itr)->getBound());
    }                   

    if (!bb.isValid()) return false;
    
    _bsphere._center = bb.center();
    _bsphere._radius = 0.0f;
    for(itr=_children.begin();
        itr!=_children.end();
        ++itr)
    {
        _bsphere.expandRadiusBy((*itr)->getBound());
    }                   

    return true;
}
