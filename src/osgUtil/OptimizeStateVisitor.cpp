#include <osg/Notify>
#include <osg/Geode>

#include <osgUtil/OptimizeStateVisitor>

#include <typeinfo>
#include <algorithm>

using namespace osgUtil;


struct LessAttributeFunctor
{
    bool operator () (const osg::StateAttribute* lhs,const osg::StateAttribute* rhs) const
    {
        return (*lhs<*rhs); 
    }
};

struct LessStateSetFunctor
{
    bool operator () (const osg::StateSet* lhs,const osg::StateSet* rhs) const
    {
        return (*lhs<*rhs); 
    }
};

void OptimizeStateVisitor::reset()
{
    _statesets.clear();
}

void OptimizeStateVisitor::addStateSet(osg::StateSet* stateset,osg::Object* obj)
{
    _statesets[stateset].insert(obj);
}

void OptimizeStateVisitor::apply(osg::Node& node)
{
    osg::StateSet* ss = node.getStateSet();
    if (ss) addStateSet(ss,&node);

    traverse(node);
}

void OptimizeStateVisitor::apply(osg::Geode& geode)
{
    osg::StateSet* ss = geode.getStateSet();
    if (ss) addStateSet(ss,&geode);
    for(int i=0;i<geode.getNumDrawables();++i)
    {
        osg::Drawable* drawable = geode.getDrawable(i);
        if (drawable)
        {
            ss = drawable->getStateSet();
            if (ss) addStateSet(ss,drawable);
        }
    }
}

void OptimizeStateVisitor::optimize()
{
    osg::notify(osg::INFO) << "Num of StateSet="<<_statesets.size()<<endl;


    {
        // create map from state attributes to stateset which contain them.
        typedef std::set<osg::StateSet*>                    StateSetList;
        typedef std::map<osg::StateAttribute*,StateSetList> AttributeToStateSetMap;
        
        AttributeToStateSetMap _attributeToStateSetMap;

        // NOTE will need to track state attribute override value too.

        for(StateSetMap::iterator sitr=_statesets.begin();
            sitr!=_statesets.end();
            ++sitr)
        {
            osg::StateSet::AttributeList& attributes = sitr->first->getAttributeList();
            for(osg::StateSet::AttributeList::iterator aitr= attributes.begin();
                aitr!=attributes.end();
                ++aitr)
            {
                _attributeToStateSetMap[aitr->second.first.get()].insert(sitr->first);
            }
        }

        if (_attributeToStateSetMap.size()<2)
        {
            osg::notify(osg::INFO) << "Too few state attributes to optimize."<<endl;
            return;
        }

        // create unique set of state attribute pointers.
        typedef std::vector<osg::StateAttribute*> AttributeList;
        AttributeList _attributeList;

        for(AttributeToStateSetMap::iterator aitr=_attributeToStateSetMap.begin();
            aitr!=_attributeToStateSetMap.end();
            ++aitr)
        {
            _attributeList.push_back(aitr->first);
        }

        // sort the attributes so that equal attributes sit along side each
        // other.
        std::sort(_attributeList.begin(),_attributeList.end(),LessAttributeFunctor());


        osg::notify(osg::INFO) << "state attribute list"<<endl;
        for(AttributeList::iterator aaitr = _attributeList.begin();
            aaitr!=_attributeList.end();
            ++aaitr)
        {
            osg::notify(osg::INFO) << "    "<<*aaitr << "  "<<(*aaitr)->className()<<endl;
        }

        osg::notify(osg::INFO) << "searching for duplicate attributes"<<endl;
        // find the duplicates.
        AttributeList::iterator first_unique = _attributeList.begin();
        AttributeList::iterator current = first_unique; ++current;
        for(; current!=_attributeList.end();++current)
        {
            if (**current==**first_unique)
            {
                osg::notify(osg::INFO) << "    found duplicate "<<(*current)->className()<<"  first="<<*first_unique<<"  current="<<*current<<endl;
                StateSetList& statesetlist = _attributeToStateSetMap[*current];
                for(StateSetList::iterator sitr=statesetlist.begin();
                    sitr!=statesetlist.end();
                    ++sitr)
                {
                    osg::notify(osg::INFO) << "       replace duplicate "<<*current<<" with "<<*first_unique<< endl;
                    osg::StateSet* stateset = *sitr;
                    stateset->setAttribute(*first_unique);
                }
            }
            else first_unique = current;
        }
        
    }
    // duplicate state attributes removed.
    // now need to look at duplicate state sets.
    
    {
        // create the list of stateset's.
        typedef std::vector<osg::StateSet*> StateSetSortList;
        StateSetSortList _statesetSortList;
        for(StateSetMap::iterator ssitr=_statesets.begin();
            ssitr!=_statesets.end();
            ++ssitr)
        {
            _statesetSortList.push_back(ssitr->first);
        }


        // sort the StateSet's so that equal StateSet's sit along side each
        // other.
        std::sort(_statesetSortList.begin(),_statesetSortList.end(),LessStateSetFunctor());

        osg::notify(osg::INFO) << "searching for duplicate attributes"<<endl;
        // find the duplicates.
        StateSetSortList::iterator first_unique = _statesetSortList.begin();
        StateSetSortList::iterator current = first_unique; ++current;
        for(; current!=_statesetSortList.end();++current)
        {
            if (**current==**first_unique)
            {
                osg::notify(osg::INFO) << "    found duplicate "<<(*current)->className()<<"  first="<<*first_unique<<"  current="<<*current<<endl;
                ObjectSet& objSet = _statesets[*current];
                for(ObjectSet::iterator sitr=objSet.begin();
                    sitr!=objSet.end();
                    ++sitr)
                {
                    osg::notify(osg::INFO) << "       replace duplicate "<<*current<<" with "<<*first_unique<< endl;
                    osg::Object* obj = *sitr;
                    osg::Drawable* drawable = dynamic_cast<osg::Drawable*>(obj);
                    if (drawable)
                    {
                        drawable->setStateSet(*first_unique);
                    }
                    else 
                    {
                        osg::Node* node = dynamic_cast<osg::Node*>(obj);
                        if (node)
                        {
                            node->setStateSet(*first_unique);
                        }
                    }
                }
            }
            else first_unique = current;
        }
    }
    
}
