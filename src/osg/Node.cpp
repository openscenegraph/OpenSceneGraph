#include "osg/Node"
#include "osg/Group"
#include "osg/NodeVisitor"

#include "osg/Input"
#include "osg/Output"

#include "osg/Registry"
#include "osg/Notify"

#include <algorithm>

using namespace osg;

RegisterObjectProxy<Node> g_NodeProxy;

Node::Node()
{
    _bsphere_computed = false;
    _userData = NULL;
    _nodeMask = 0xffffffff;
}


Node::~Node()
{
    if (_userData && _memoryAdapter.valid()) _memoryAdapter->decrementReference(_userData);
}


void Node::accept(NodeVisitor& nv)
{
    nv.apply(*this);
}

void Node::ascend(NodeVisitor& nv)
{
    std::for_each(_parents.begin(),_parents.end(),NodeAcceptOp(nv));
}

bool Node::readLocalData(Input& fr)
{
    bool iteratorAdvanced = false;

    if (Object::readLocalData(fr)) iteratorAdvanced = true;

    if (fr.matchSequence("name %s"))
    {
        _name = fr[1].takeStr();
        fr+=2;
        iteratorAdvanced = true;
    }

//     if (fr.matchSequence("user_data {"))
//     {
//         notify(DEBUG) << "Matched user_data {"<<endl;
//         int entry = fr[0].getNoNestedBrackets();
//         fr += 2;
// 
//         while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
//         {
//             Object* object = fr.readObject();
//             if (object) setUserData(object);
//             notify(DEBUG) << "read "<<object<<endl;
//             ++fr;
//         }        
//         iteratorAdvanced = true;
//     }

    while (fr.matchSequence("description {"))
    {
        notify(DEBUG) << "Matched description {"<<endl;
        int entry = fr[0].getNoNestedBrackets();
        fr += 2;

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            _descriptions.push_back(fr[0].getStr());
            notify(DEBUG) << "read "<<_descriptions.back()<<endl;
            ++fr;
        }        
        iteratorAdvanced = true;

    }

    while (fr.matchSequence("description %s"))
    {
         notify(DEBUG) << "Matched description %s"<<endl;
        _descriptions.push_back(fr[1].getStr());
        notify(DEBUG) << "read "<<_descriptions.back()<<endl;
        fr+=2;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool Node::writeLocalData(Output& fw)
{
    Object::writeLocalData(fw);
    if (!_name.empty()) fw.indent() << "name "<<'"'<<_name<<'"'<<endl;

//     if (_userData)
//     {
//         Object* object = dynamic_cast<Object*>(_userData);
//         if (object)
//         {
//             fw.indent() << "user_data {"<<endl;
//             fw.moveIn();
//             object->write(fw);
//             fw.moveOut();
//             fw.indent() << "}"<<endl;
//         }
//     }

    if (!_descriptions.empty())
    {
        if (_descriptions.size()==1)
        {
            fw.indent() << "description "<<'"'<<_descriptions.front()<<'"'<<endl;
        }
        else
        {
            fw.indent() << "description {"<<endl;
            fw.moveIn();
            for(DescriptionList::iterator ditr=_descriptions.begin();
                                          ditr!=_descriptions.end();
                                          ++ditr)
            {
                fw.indent() << '"'<<*ditr<<'"'<<endl;
            }
            fw.moveOut();
            fw.indent() << "}"<<endl;
        }
    }

    return true;
}

bool Node::computeBound()
{
    _bsphere.init();
    return false;
}

const BoundingSphere& Node::getBound()
{
    if(!_bsphere_computed) computeBound();
    return _bsphere;
}

void Node::dirtyBound()
{
    if (_bsphere_computed)
    {
        _bsphere_computed = false;

        // dirty parent bounding sphere's to ensure that all are valid.
        for(ParentList::iterator itr=_parents.begin();
                                 itr!=_parents.end();
                                 ++itr)
        {
            (*itr)->dirtyBound();
        }

    }
}
