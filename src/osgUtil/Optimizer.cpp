#include <osgUtil/Optimizer>

#include <osg/Transform>
#include <osg/LOD>
#include <osg/Impostor>
#include <osg/Billboard>
#include <osg/Notify>

#include <typeinfo>
#include <algorithm>

using namespace osgUtil;

////////////////////////////////////////////////////////////////////////////
// Overall Optimizetion function.
////////////////////////////////////////////////////////////////////////////

void Optimizer::optimize(osg::Node* node, unsigned int options)
{

    if (options & FLATTEN_STATIC_TRANSFORMS)
    {
        FlattenStaticTransformsVisitor fstv;
        node->accept(fstv);
        fstv.removeTransforms();
    }
    
    if (options & REMOVE_REDUNDENT_NODES)
    {
        RemoveRedundentNodesVisitor rrnv;
        node->accept(rrnv);
        rrnv.removeRedundentNodes();
    }
    
    if (options & COMBINE_ADJACENT_LODS)
    {
        CombineLODsVisitor clv;
        node->accept(clv);        
        clv.combineLODs();
    }
    
    if (options & SHARE_DUPLICATE_STATE)
    {
    #if !defined(WIN32) || defined(_STLPORT_VERSION)
        StateVisitor osv;
        node->accept(osv);
        osv.optimize();
    #endif
    }
    
}

class TransformFunctor : public osg::Drawable::AttributeFunctor
{
    public:
    
        osg::Matrix _m;
        osg::Matrix _im;

        TransformFunctor(const osg::Matrix& m):
            osg::Drawable::AttributeFunctor(osg::Drawable::COORDS|osg::Drawable::NORMALS)
        {
            _m = m;
            _im.invert(_m);
        }
            
        virtual ~TransformFunctor() {}

        virtual bool apply(osg::Drawable::AttributeBitMask abm,osg::Vec3* begin,osg::Vec3* end)
        {
            if (abm == osg::Drawable::COORDS)
            {
                for (osg::Vec3* itr=begin;itr<end;++itr)
                {
                    (*itr) = (*itr)*_m;
                }
                return true;
            }
            else if (abm == osg::Drawable::NORMALS)
            {
                for (osg::Vec3* itr=begin;itr<end;++itr)
                {
                    // note post mult by inverse for normals.
                    (*itr) = osg::Matrix::transform3x3(_im,(*itr));
                    (*itr).normalize();
                }
                return true;
            }
            return false;

        }

};

////////////////////////////////////////////////////////////////////////////
// Optimize State Visitor
////////////////////////////////////////////////////////////////////////////

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



void Optimizer::StateVisitor::reset()
{
    _statesets.clear();
}

void Optimizer::StateVisitor::addStateSet(osg::StateSet* stateset,osg::Object* obj)
{
    _statesets[stateset].insert(obj);
}

void Optimizer::StateVisitor::apply(osg::Node& node)
{
    osg::StateSet* ss = node.getStateSet();
    if (ss) addStateSet(ss,&node);

    traverse(node);
}

void Optimizer::StateVisitor::apply(osg::Geode& geode)
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

void Optimizer::StateVisitor::optimize()
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

////////////////////////////////////////////////////////////////////////////
// Flatten static transforms
////////////////////////////////////////////////////////////////////////////

void Optimizer::FlattenStaticTransformsVisitor::apply(osg::Geode& geode)
{
    if (!_matrixStack.empty())
    {
        TransformFunctor tf(_matrixStack.back());
        for(int i=0;i<geode.getNumDrawables();++i)
        {
            geode.getDrawable(i)->applyAttributeOperation(tf);
        }
    }
}

void Optimizer::FlattenStaticTransformsVisitor::apply(osg::Billboard& billboard)
{
    if (!_matrixStack.empty())
    {
        osg::Matrix& matrix = _matrixStack.back();
        TransformFunctor tf(matrix);

        osg::Vec3 axis = osg::Matrix::transform3x3(tf._im,billboard.getAxis());
        billboard.setAxis(axis);

        for(int i=0;i<billboard.getNumDrawables();++i)
        {
            billboard.setPos(i,billboard.getPos(i)*matrix);
            billboard.getDrawable(i)->applyAttributeOperation(tf);
        }
    }
}

void Optimizer::FlattenStaticTransformsVisitor::apply(osg::LOD& lod)
{
    if (!_matrixStack.empty())
    {
        lod.setCenter(lod.getCenter()*_matrixStack.back());
    }
    traverse(lod);
}

void Optimizer::FlattenStaticTransformsVisitor::apply(osg::Transform& transform)
{
    if (_matrixStack.empty())
    {
        _matrixStack.push_back(transform.getMatrix());
    }
    else
    {
        _matrixStack.push_back(transform.getMatrix()*_matrixStack.back());
    }

    traverse(transform);

    _transformList.insert(&transform);

    // reset the matrix to identity.
    transform.getMatrix().makeIdent();

    _matrixStack.pop_back();
}

void Optimizer::FlattenStaticTransformsVisitor::removeTransforms()
{
    for(TransformList::iterator itr=_transformList.begin();
        itr!=_transformList.end();
        ++itr)
    {
        osg::ref_ptr<osg::Transform> transform = *itr;
        osg::ref_ptr<osg::Group>     group = new osg::Group;

        int i;
        for(i=0;i<transform->getNumChildren();++i)
        {
            for(int j=0;j<transform->getNumParents();++j)
            {
                group->addChild(transform->getChild(i));
            }
        }

        for(i=transform->getNumParents()-1;i>=0;--i)
        {
            transform->getParent(i)->replaceChild(transform.get(),group.get());
        }                

    }
    _transformList.clear();
}


////////////////////////////////////////////////////////////////////////////
// RemoveRedundentNodes.
////////////////////////////////////////////////////////////////////////////

void Optimizer::RemoveRedundentNodesVisitor::apply(osg::Group& group)
{
    if (typeid(group)==typeid(osg::Group))
    {
        if (group.getNumParents()>0 && group.getNumChildren()<=1)
        {
            _redundentNodeList.insert(&group);
        }
    }
    traverse(group);
}


void Optimizer::RemoveRedundentNodesVisitor::removeRedundentNodes()
{
    for(NodeList::iterator itr=_redundentNodeList.begin();
        itr!=_redundentNodeList.end();
        ++itr)
    {
        osg::ref_ptr<osg::Group> group = dynamic_cast<osg::Group*>(*itr);
        if (group.valid())
        {

            for(int j=group->getNumParents()-1;j>=0;--j)
            {
                for(int i=0;i<group->getNumChildren();++i)
                {
                    group->getParent(j)->addChild(group->getChild(i));
                }
                group->getParent(j)->removeChild(group.get());
            }
        }                                
    }
    _redundentNodeList.clear();
}




////////////////////////////////////////////////////////////////////////////
// combine LOD's.
////////////////////////////////////////////////////////////////////////////
void Optimizer::CombineLODsVisitor::apply(osg::LOD& lod)
{
    for(int i=0;i<lod.getNumParents();++i)
    {
        if (typeid(*lod.getParent(i))==typeid(osg::Group))
        {
            _groupList.insert(lod.getParent(i));
        }
    }
    traverse(lod);
}


void Optimizer::CombineLODsVisitor::combineLODs()
{
    for(GroupList::iterator itr=_groupList.begin();
        itr!=_groupList.end();
        ++itr)
    {
        osg::Group* group = *itr;

        typedef std::set<osg::LOD*>                 LODSet;

        LODSet    lodChildren;

        for(int i=0;i<group->getNumChildren();++i)
        {
            osg::Node* child = group->getChild(i);
            osg::LOD* lod = dynamic_cast<osg::LOD*>(child);
            if (lod)
            {
                if (lod->getNumRanges()-1==lod->getNumChildren())
                {
                    lodChildren.insert(lod);
                }
                else
                {
                    // wonky LOD, numRanges should = numChildren+1
                }
            }
        }

        if (lodChildren.size()>=2)
        {
            osg::BoundingBox bb;
            LODSet::iterator lod_itr;
            for(lod_itr=lodChildren.begin();
                lod_itr!=lodChildren.end();
                ++lod_itr)
            {

                bb.expandBy((*lod_itr)->getCenter());
            }
            if (bb.radius()<1e-2)
            {
                typedef std::pair<float,float> RangePair;
                typedef std::multimap<RangePair,osg::Node*> RangeMap;
                RangeMap rangeMap;
                float maxRange = 0.0f;
                for(lod_itr=lodChildren.begin();
                    lod_itr!=lodChildren.end();
                    ++lod_itr)
                {

                    osg::LOD* lod = *lod_itr;
                    for(int i=0;i<lod->getNumRanges()-1;++i)
                    {
                        if (maxRange<lod->getRange(i+1)) maxRange = lod->getRange(i+1);
                        rangeMap.insert(RangeMap::value_type(RangePair(lod->getRange(i),lod->getRange(i+1)),lod->getChild(i)));
                    }

                }

                // create new LOD containing all other LOD's children.
                osg::LOD* newLOD = new osg::LOD;
                newLOD->setName("newLOD");
                newLOD->setCenter(bb.center());

                int i=0;
                for(RangeMap::iterator c_itr=rangeMap.begin();
                    c_itr!=rangeMap.end();
                    ++c_itr,++i)
                {
                    newLOD->setRange(i,c_itr->first.first);
                    newLOD->addChild(c_itr->second);
                }
                newLOD->setRange(i,maxRange);

                // add LOD into parent.
                group->addChild(newLOD);

                // remove all the old LOD's from group.
                for(lod_itr=lodChildren.begin();
                    lod_itr!=lodChildren.end();
                    ++lod_itr)
                {
                    group->removeChild(*lod_itr);
                }

            }

        }
    }
    _groupList.clear();
}

