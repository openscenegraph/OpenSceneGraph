#include "osg/Switch"
#include "osg/Registry"
#include "osg/Input"
#include "osg/Output"

#include <algorithm>

using namespace osg;

RegisterObjectProxy<Switch> g_SwitchProxy;

Switch::Switch()
{
    _value = ALL_CHILDREN_OFF;
}

void Switch::traverse(NodeVisitor& nv)
{
    switch(nv.getTraverseMode())
    {
    case(NodeVisitor::TRAVERSE_ALL_CHILDREN):
        std::for_each(_children.begin(),_children.end(),NodeAcceptOp(nv));
        break;
    case(NodeVisitor::TRAVERSE_ACTIVE_CHILDREN):
        switch(_value)
        {
        case(ALL_CHILDREN_ON):
            std::for_each(_children.begin(),_children.end(),NodeAcceptOp(nv));
            break;
        case(ALL_CHILDREN_OFF):
            return;
        default:
            if (_value>=0 && (unsigned int)_value<_children.size()) _children[_value]->accept(nv);
            break;
        }
        break;
    default:
        break;
    }
}

bool Switch::readLocalData(Input& fr)
{
    bool iteratorAdvanced = false;

    if (fr.matchSequence("value"))
    {
        if (fr[1].matchWord("ALL_CHILDREN_ON"))
        {
            _value = ALL_CHILDREN_ON;
            iteratorAdvanced = true;
            fr+=2;
        }
        else if (fr[1].matchWord("ALL_CHILDREN_ON"))
        {
            _value = ALL_CHILDREN_OFF;
            iteratorAdvanced = true;
            fr+=2;
        }
        else if (fr[1].isInt())
        {
            fr[1].getInt(_value);
            iteratorAdvanced = true;
            fr+=2;
        }
    }

    if (Group::readLocalData(fr)) iteratorAdvanced = true;

    return iteratorAdvanced;
}

bool Switch::writeLocalData(Output& fw)
{
    fw.indent() << "value ";
    switch(_value)
    {
    case(ALL_CHILDREN_ON): fw<<"ALL_CHILDREN_ON"<<endl;break;
    case(ALL_CHILDREN_OFF): fw<<"ALL_CHILDREN_OFF"<<endl;break;
    default: fw<<_value<<endl;break;
    }
    
    Group::writeLocalData(fw);

    return true;
}

