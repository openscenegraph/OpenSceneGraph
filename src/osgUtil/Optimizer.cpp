#include <osgUtil/Optimizer>

#include <osg/Transform>
#include <osg/LOD>
#include <osg/Impostor>
#include <osg/Billboard>
#include <osg/Geometry>
#include <osg/GeoSet>
#include <osg/Notify>
#include <osg/OccluderNode>
#include <osg/Sequence>
#include <osg/Switch>

#include <typeinfo>
#include <algorithm>
#include <numeric>

using namespace osgUtil;

// #define CONVERT_GEOSET_TO_GEOMETRY

////////////////////////////////////////////////////////////////////////////
// Overall Optimizetion function.
////////////////////////////////////////////////////////////////////////////

void Optimizer::optimize(osg::Node* node, unsigned int options)
{

    if (options & COMBINE_ADJACENT_LODS)
    {
        CombineLODsVisitor clv;
        node->accept(clv);        
        clv.combineLODs();
    }
    
    if (options & FLATTEN_STATIC_TRANSFORMS)
    {

        int i=0;
        bool result = false;
        do
        {
            osg::notify(osg::DEBUG_INFO) << "** RemoveStaticTransformsVisitor *** Pass "<<i<<std::endl;
            FlattenStaticTransformsVisitor fstv;
            node->accept(fstv);
            result = fstv.removeTransforms();
            ++i;
        } while (result);

    }
    

    if (options & REMOVE_REDUNDANT_NODES)
    {

        RemoveEmptyNodesVisitor renv;
        node->accept(renv);
        renv.removeEmptyNodes();

        RemoveRedundantNodesVisitor rrnv;
        node->accept(rrnv);
        rrnv.removeRedundantNodes();

    }
    

#if defined(CONVERT_GEOSET_TO_GEOMETRY)
    // convert the old style GeoSet to Geometry
    ConvertGeoSetsToGeometryVisitor cgtg;
    node->accept(cgtg);
#endif

    if (options & SHARE_DUPLICATE_STATE)
    {
        StateVisitor osv;
        node->accept(osv);
        osv.optimize();

        MergeGeometryVisitor mgv;
        node->accept(mgv);
    }



}

class TransformFunctor : public osg::Drawable::AttributeFunctor
{
    public:
    
        osg::Matrix _m;
        osg::Matrix _im;

        TransformFunctor(const osg::Matrix& m)
        {
            _m = m;
            _im.invert(_m);
        }
            
        virtual ~TransformFunctor() {}

        virtual void apply(osg::Drawable::AttributeType type,unsigned int count,osg::Vec3* begin)
        {
            if (type == osg::Drawable::VERTICES)
            {
                osg::Vec3* end = begin+count;
                for (osg::Vec3* itr=begin;itr<end;++itr)
                {
                    (*itr) = (*itr)*_m;
                }
            }
            else if (type == osg::Drawable::NORMALS)
            {
                osg::Vec3* end = begin+count;
                for (osg::Vec3* itr=begin;itr<end;++itr)
                {
                    // note post mult by inverse for normals.
                    (*itr) = osg::Matrix::transform3x3(_im,(*itr));
                    (*itr).normalize();
                }
            }
        }

};

////////////////////////////////////////////////////////////////////////////
// Convert GeoSet To Geometry Visitor.
////////////////////////////////////////////////////////////////////////////

void Optimizer::ConvertGeoSetsToGeometryVisitor::apply(osg::Geode& geode)
{
    for(unsigned int i=0;i<geode.getNumDrawables();++i)
    {
        osg::GeoSet* geoset = dynamic_cast<osg::GeoSet*>(geode.getDrawable(i));
        if (geoset)
        {
            osg::Geometry* geom = geoset->convertToGeometry();
            if (geom)
            {
                std::cout<<"Successfully converted GeoSet to Geometry"<<std::endl;
                geode.replaceDrawable(geoset,geom);
            }
            else
            {
                std::cout<<"*** Failed to convert GeoSet to Geometry"<<std::endl;
            }

        }
    }
}




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
    if (ss && ss->getDataVariance()==osg::Object::STATIC) addStateSet(ss,&node);

    traverse(node);
}

void Optimizer::StateVisitor::apply(osg::Geode& geode)
{
    osg::StateSet* ss = geode.getStateSet();
    if (ss && ss->getDataVariance()==osg::Object::STATIC) addStateSet(ss,&geode);
    for(unsigned int i=0;i<geode.getNumDrawables();++i)
    {
        osg::Drawable* drawable = geode.getDrawable(i);
        if (drawable)
        {
            ss = drawable->getStateSet();
            if (ss && ss->getDataVariance()==osg::Object::STATIC) addStateSet(ss,drawable);
        }
    }
}

void Optimizer::StateVisitor::optimize()
{
    osg::notify(osg::INFO) << "Num of StateSet="<<_statesets.size()<< std::endl;

    {
        // create map from state attributes to stateset which contain them.
        typedef std::pair<osg::StateSet*,unsigned int>      StateSetUnitPair;
        typedef std::set<StateSetUnitPair>                  StateSetList;
        typedef std::map<osg::StateAttribute*,StateSetList> AttributeToStateSetMap;
        
        const unsigned int NON_TEXTURE_ATTRIBUTE = 0xffffffff;
        
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
                if (aitr->second.first->getDataVariance()==osg::Object::STATIC)
                {
                    _attributeToStateSetMap[aitr->second.first.get()].insert(StateSetUnitPair(sitr->first,NON_TEXTURE_ATTRIBUTE));
                }
            }


            osg::StateSet::TextureAttributeList& texAttributes = sitr->first->getTextureAttributeList();
            for(unsigned int unit=0;unit<texAttributes.size();++unit)
            {
                osg::StateSet::AttributeList& attributes = texAttributes[unit];
                for(osg::StateSet::AttributeList::iterator aitr= attributes.begin();
                    aitr!=attributes.end();
                    ++aitr)
                {
                    if (aitr->second.first->getDataVariance()==osg::Object::STATIC)
                    {
                        _attributeToStateSetMap[aitr->second.first.get()].insert(StateSetUnitPair(sitr->first,unit));
                    }
                }
            }

        }

        if (_attributeToStateSetMap.size()<2)
        {
            osg::notify(osg::INFO) << "Too few state attributes to optimize."<< std::endl;
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


        osg::notify(osg::INFO) << "state attribute list"<< std::endl;
        for(AttributeList::iterator aaitr = _attributeList.begin();
            aaitr!=_attributeList.end();
            ++aaitr)
        {
            osg::notify(osg::INFO) << "    "<<*aaitr << "  "<<(*aaitr)->className()<< std::endl;
        }

        osg::notify(osg::INFO) << "searching for duplicate attributes"<< std::endl;
        // find the duplicates.
        AttributeList::iterator first_unique = _attributeList.begin();
        AttributeList::iterator current = first_unique; ++current;
        for(; current!=_attributeList.end();++current)
        {
            if (**current==**first_unique)
            {
                osg::notify(osg::INFO) << "    found duplicate "<<(*current)->className()<<"  first="<<*first_unique<<"  current="<<*current<< std::endl;
                StateSetList& statesetlist = _attributeToStateSetMap[*current];
                for(StateSetList::iterator sitr=statesetlist.begin();
                    sitr!=statesetlist.end();
                    ++sitr)
                {
                    osg::notify(osg::INFO) << "       replace duplicate "<<*current<<" with "<<*first_unique<< std::endl;
                    osg::StateSet* stateset = sitr->first;
                    unsigned int unit = sitr->second;
                    if (unit==NON_TEXTURE_ATTRIBUTE) stateset->setAttribute(*first_unique);
                    else stateset->setTextureAttribute(unit,*first_unique);
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

        osg::notify(osg::INFO) << "searching for duplicate attributes"<< std::endl;
        // find the duplicates.
        StateSetSortList::iterator first_unique = _statesetSortList.begin();
        StateSetSortList::iterator current = first_unique; ++current;
        for(; current!=_statesetSortList.end();++current)
        {
            if (**current==**first_unique)
            {
                osg::notify(osg::INFO) << "    found duplicate "<<(*current)->className()<<"  first="<<*first_unique<<"  current="<<*current<< std::endl;
                ObjectSet& objSet = _statesets[*current];
                for(ObjectSet::iterator sitr=objSet.begin();
                    sitr!=objSet.end();
                    ++sitr)
                {
                    osg::notify(osg::INFO) << "       replace duplicate "<<*current<<" with "<<*first_unique<< std::endl;
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

class CollectLowestTransformsVisitor : public osg::NodeVisitor
{
    public:


        CollectLowestTransformsVisitor():
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_PARENTS) {}


        virtual void apply(osg::Node& node)
        {
            if (node.getNumParents())
            {
                traverse(node);
            }
            else
            {
                // for all current objects mark a NULL transform for them.
                registerWithCurrentObjects(0);
            }
        }

        virtual void apply(osg::LOD& lod)
        {
            _currentObjectList.push_back(&lod);

            traverse(lod);

            _currentObjectList.pop_back();
        }

        virtual void apply(osg::Transform& transform)
        {
            // for all current objects associated this transform with them.
            registerWithCurrentObjects(&transform);
        }
        
        virtual void apply(osg::Geode& geode)
        {
            traverse(geode);
        }
        
        virtual void apply(osg::Billboard& geode)
        {
            traverse(geode);
        }
        

        void collectDataFor(osg::Billboard* billboard)
        {
            _currentObjectList.push_back(billboard);
            
            billboard->accept(*this);
            
            _currentObjectList.pop_back();
        }
        
        void collectDataFor(osg::Drawable* drawable)
        {
            _currentObjectList.push_back(drawable);

            const osg::Drawable::ParentList& parents = drawable->getParents();
            for(osg::Drawable::ParentList::const_iterator itr=parents.begin();
                itr!=parents.end();
                ++itr)
            {
                (*itr)->accept(*this);
            }

            _currentObjectList.pop_back();
        }

        void setUpMaps();
        void disableTransform(osg::Transform* transform);
        bool removeTransforms();

    protected:        

        struct TransformStruct
        {
            typedef std::set<osg::Object*> ObjectSet;

            TransformStruct():_canBeApplied(true) {}

            void add(osg::Object* obj)
            {
                _objectSet.insert(obj);
            }

            bool        _canBeApplied;
            ObjectSet   _objectSet;
        };

        struct ObjectStruct
        {
            typedef std::set<osg::Transform*> TransformSet;

            ObjectStruct():_canBeApplied(true),_moreThanOneMatrixRequired(false) {}

            void add(osg::Transform* transform)
            {
                if (transform)
                {
                    if (transform->getDataVariance()==osg::Transform::DYNAMIC) _moreThanOneMatrixRequired=true;
                    else if (transform->getReferenceFrame()==osg::Transform::RELATIVE_TO_ABSOLUTE) _moreThanOneMatrixRequired=true;
                    else
                    {
                        if (_transformSet.empty()) transform->getLocalToWorldMatrix(_firstMatrix,0);
                        else
                        {
                            osg::Matrix matrix;
                            transform->getLocalToWorldMatrix(_firstMatrix,0);
                            if (_firstMatrix!=matrix) _moreThanOneMatrixRequired=true;
                        }
                    }
                }
                else
                {
                    if (!_transformSet.empty()) 
                    {
                        if (_firstMatrix!=osg::Matrix::identity()) _moreThanOneMatrixRequired=true;
                    }

                }
                _transformSet.insert(transform);
            }

            bool            _canBeApplied;
            bool            _moreThanOneMatrixRequired;
            osg::Matrix     _firstMatrix;
            TransformSet    _transformSet;

        };    


        void registerWithCurrentObjects(osg::Transform* transform)
        {
            for(ObjectList::iterator itr=_currentObjectList.begin();
                itr!=_currentObjectList.end();
                ++itr)
            {
                _objectMap[*itr].add(transform);
            }
        }

        typedef std::map<osg::Transform*,TransformStruct>   TransformMap;
        typedef std::map<osg::Object*,ObjectStruct>         ObjectMap;
        typedef std::vector<osg::Object*>                   ObjectList;

        void disableObject(osg::Object* object)
        {
            disableObject(_objectMap.find(object));
        }

        void disableObject(ObjectMap::iterator itr);
        void doTransform(osg::Object* obj,osg::Matrix& matrix);
        

        TransformMap    _transformMap;
        ObjectMap       _objectMap;
        ObjectList      _currentObjectList;

};


void CollectLowestTransformsVisitor::doTransform(osg::Object* obj,osg::Matrix& matrix)
{
    osg::Drawable* drawable = dynamic_cast<osg::Drawable*>(obj);
    if (drawable)
    {
        TransformFunctor tf(matrix);
        drawable->accept(tf);
        drawable->dirtyBound();
        return;
    }

    osg::LOD* lod = dynamic_cast<osg::LOD*>(obj);
    if (lod)
    {
        osg::Matrix matrix_no_trans = matrix;
        matrix_no_trans.setTrans(0.0f,0.0f,0.0f);
        
        osg::Vec3 v111(1.0f,1.0f,1.0f);
        osg::Vec3 new_v111 = v111*matrix_no_trans;
        float ratio = new_v111.length()/v111.length();

        // move center point.
        lod->setCenter(lod->getCenter()*matrix);
        
        // adjust ranges to new scale.
        for(unsigned int i=0;i<lod->getNumRanges();++i)
        {
            lod->setRange(i,lod->getMinRange(i)*ratio,lod->getMaxRange(i)*ratio);
        }
        
        lod->dirtyBound();
        return;
    }

    osg::Billboard* billboard = dynamic_cast<osg::Billboard*>(obj);
    if (billboard)
    {
        osg::Matrix matrix_no_trans = matrix;
        matrix_no_trans.setTrans(0.0f,0.0f,0.0f);
  
        TransformFunctor tf(matrix_no_trans);

        osg::Vec3 axis = osg::Matrix::transform3x3(tf._im,billboard->getAxis());
        axis.normalize();
        billboard->setAxis(axis);

        for(unsigned int i=0;i<billboard->getNumDrawables();++i)
        {
            billboard->setPos(i,billboard->getPos(i)*matrix);
            billboard->getDrawable(i)->accept(tf);
        }
        
        billboard->dirtyBound();

        return;
    }
}

void CollectLowestTransformsVisitor::disableObject(ObjectMap::iterator itr)
{
    if (itr==_objectMap.end())
    {
        return;
    }    

    if (itr->second._canBeApplied)
    {
        // we havn't been disabled yet so we need to disable,
        itr->second._canBeApplied = false;

        // and then inform everybody we have been disabled.
        for(ObjectStruct::TransformSet::iterator titr = itr->second._transformSet.begin();
            titr != itr->second._transformSet.end();
            ++titr)
        {
            disableTransform(*titr);
        }
    }
}

void CollectLowestTransformsVisitor::disableTransform(osg::Transform* transform)
{
    TransformMap::iterator itr=_transformMap.find(transform);
    if (itr==_transformMap.end())
    {
        return;
    }    

    if (itr->second._canBeApplied)
    {
    
        // we havn't been disabled yet so we need to disable,
        itr->second._canBeApplied = false;
        // and then inform everybody we have been disabled.
        for(TransformStruct::ObjectSet::iterator oitr = itr->second._objectSet.begin();
            oitr != itr->second._objectSet.end();
            ++oitr)
        {
            disableObject(*oitr);
        }
    }
}

void CollectLowestTransformsVisitor::setUpMaps()
{

    // create the TransformMap from the ObjectMap
    ObjectMap::iterator oitr;
    for(oitr=_objectMap.begin();
        oitr!=_objectMap.end();
        ++oitr)
    {
        osg::Object* object = oitr->first;
        ObjectStruct& os = oitr->second;
        
        for(ObjectStruct::TransformSet::iterator titr = os._transformSet.begin();
            titr != os._transformSet.end();
            ++titr)
        {
            _transformMap[*titr].add(object);
        }
    }
    

    // disable all the objects which have more than one matrix associated 
    // with them, and then disable all transforms which have an object associated 
    // them that can't be applied, and then disable all objects which have
    // disabled transforms associated, recursing until all disabled 
    // associativity.
    for(oitr=_objectMap.begin();
        oitr!=_objectMap.end();
        ++oitr)
    {
        ObjectStruct& os = oitr->second;
        if (os._canBeApplied)
        {
            if (os._moreThanOneMatrixRequired)
            {
                disableObject(oitr);
            }
        }
    }

}

bool CollectLowestTransformsVisitor::removeTransforms()
{
    // transform the objects that can be applied.
    for(ObjectMap::iterator oitr=_objectMap.begin();
        oitr!=_objectMap.end();
        ++oitr)
    {
        osg::Object* object = oitr->first;
        ObjectStruct& os = oitr->second;
        if (os._canBeApplied)
        {
            doTransform(object,os._firstMatrix);
        }
    }


    bool transformRemoved = false;

    // clean up the transforms.
    for(TransformMap::iterator titr=_transformMap.begin();
        titr!=_transformMap.end();
        ++titr)
    {
        if (titr->second._canBeApplied)
        {
            transformRemoved = true;
        
            osg::ref_ptr<osg::Transform> transform = titr->first;
            osg::ref_ptr<osg::Group>     group = osgNew osg::Group;
            group->setDataVariance(osg::Object::STATIC);
            for(unsigned int i=0;i<transform->getNumChildren();++i)
            {
                for(unsigned int j=0;j<transform->getNumParents();++j)
                {
                    group->addChild(transform->getChild(i));
                }
            }

            for(int i2=transform->getNumParents()-1;i2>=0;--i2)
            {
                transform->getParent(i2)->replaceChild(transform.get(),group.get());
            }                
        }
    }
    _objectMap.clear();
    _transformMap.clear();
    
    return transformRemoved;
}

void Optimizer::FlattenStaticTransformsVisitor::apply(osg::Geode& geode)
{

    if (!_transformStack.empty())
    {
        for(unsigned int i=0;i<geode.getNumDrawables();++i)
        {
            _drawableSet.insert(geode.getDrawable(i));
        }
    }
}

void Optimizer::FlattenStaticTransformsVisitor::apply(osg::Billboard& billboard)
{
    if (!_transformStack.empty())
    {
        _billboardSet.insert(&billboard);
    }
}

void Optimizer::FlattenStaticTransformsVisitor::apply(osg::Transform& transform)
{
    if (!_transformStack.empty())
    {
        // we need to disable any transform higher in the list.
        _transformSet.insert(_transformStack.back());
    }

    _transformStack.push_back(&transform);

    

    // simple traverse the children as if this Transform didn't exist.
    traverse(transform);

    _transformStack.pop_back();
}

bool Optimizer::FlattenStaticTransformsVisitor::removeTransforms()
{
    CollectLowestTransformsVisitor cltv;

    for(DrawableSet::iterator ditr=_drawableSet.begin();
        ditr!=_drawableSet.end();
        ++ditr)
    {
        cltv.collectDataFor(*ditr);
    }

    for(BillboardSet::iterator bitr=_billboardSet.begin();
        bitr!=_billboardSet.end();
        ++bitr)
    {
        cltv.collectDataFor(*bitr);
    }
    
    cltv.setUpMaps();
    
    for(TransformSet::iterator titr=_transformSet.begin();
        titr!=_transformSet.end();
        ++titr)
    {
        cltv.disableTransform(*titr);
    }
    

    return cltv.removeTransforms();
}


////////////////////////////////////////////////////////////////////////////
// RemoveEmptyNodes.
////////////////////////////////////////////////////////////////////////////

void Optimizer::RemoveEmptyNodesVisitor::apply(osg::Geode& geode)
{
    if (geode.getNumParents()>0)
    {
        if (geode.getNumDrawables()==0) _redundantNodeList.insert(&geode);
    }
}

void Optimizer::RemoveEmptyNodesVisitor::apply(osg::Group& group)
{
    if (group.getNumParents()>0)
    {
        // only remove empty groups, but not empty occluders.
        if (group.getNumChildren()==0 && 
            (typeid(group)==typeid(osg::Group) || dynamic_cast<osg::Transform*>(&group)))
        {
            _redundantNodeList.insert(&group);
        }
    }
    traverse(group);
}

void Optimizer::RemoveEmptyNodesVisitor::removeEmptyNodes()
{

    NodeList newEmptyGroups;

    // keep iterator through until scene graph is cleaned of empty nodes.
    while (!_redundantNodeList.empty())
    {
        for(NodeList::iterator itr=_redundantNodeList.begin();
            itr!=_redundantNodeList.end();
            ++itr)
        {
            
            osg::ref_ptr<osg::Node> nodeToRemove = (*itr);
            
            // take a copy of parents list since subsequent removes will modify the original one.
            osg::Node::ParentList parents = nodeToRemove->getParents();

            for(osg::Node::ParentList::iterator pitr=parents.begin();
                pitr!=parents.end();
                ++pitr)
            {
                osg::Group* parent = *pitr;
                if (!dynamic_cast<osg::Sequence*>(parent) &&
                    !dynamic_cast<osg::Switch*>(parent))
                {
                    parent->removeChild(nodeToRemove.get());
                    if (parent->getNumChildren()==0) newEmptyGroups.insert(*pitr);
                }
            }                
        }

        _redundantNodeList.clear();
        _redundantNodeList.swap(newEmptyGroups);
    }
}


////////////////////////////////////////////////////////////////////////////
// RemoveRedundantNodes.
////////////////////////////////////////////////////////////////////////////

void Optimizer::RemoveRedundantNodesVisitor::apply(osg::Group& group)
{
    if (group.getNumParents()>0)
    {
        if (group.getNumChildren()==1 && typeid(group)==typeid(osg::Group))
        {
            if (group.getNumParents()>0 && group.getNumChildren()<=1)
            {
                if (!group.getUserData() &&
                    !group.getAppCallback() &&
                    !group.getStateSet() &&
                    group.getNodeMask()==0xffffffff)
                {
                    _redundantNodeList.insert(&group);
                }
            }
        }
    }
    traverse(group);
}

void Optimizer::RemoveRedundantNodesVisitor::apply(osg::Transform& transform)
{
    if (transform.getNumParents()>0 && transform.getDataVariance()==osg::Object::STATIC)
    {
        static osg::Matrix identity;
        osg::Matrix matrix;
        transform.getWorldToLocalMatrix(matrix,NULL);
        if (matrix==identity)
        {
            _redundantNodeList.insert(&transform);
        }
    }
    traverse(transform);
}


void Optimizer::RemoveRedundantNodesVisitor::removeRedundantNodes()
{

    for(NodeList::iterator itr=_redundantNodeList.begin();
        itr!=_redundantNodeList.end();
        ++itr)
    {
        osg::ref_ptr<osg::Group> group = dynamic_cast<osg::Group*>(*itr);
        if (group.valid())
        {
            // take a copy of parents list since subsequent removes will modify the original one.
            osg::Node::ParentList parents = group->getParents();

            if (group->getNumChildren()==1)
            {
                osg::Node* child = group->getChild(0);
                for(osg::Node::ParentList::iterator pitr=parents.begin();
                    pitr!=parents.end();
                    ++pitr)
                {
                    (*pitr)->replaceChild(group.get(),child);
                }

            }

        }
        else
        {
            std::cout<<"failed dynamic_cast"<<std::endl;
        }                                
    }
    _redundantNodeList.clear();
}



////////////////////////////////////////////////////////////////////////////
// combine LOD's.
////////////////////////////////////////////////////////////////////////////
void Optimizer::CombineLODsVisitor::apply(osg::LOD& lod)
{
    for(unsigned int i=0;i<lod.getNumParents();++i)
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

        for(unsigned int i=0;i<group->getNumChildren();++i)
        {
            osg::Node* child = group->getChild(i);
            osg::LOD* lod = dynamic_cast<osg::LOD*>(child);
            if (lod)
            {
                lodChildren.insert(lod);
            }
        }

        if (lodChildren.size()>=2)
        {
            osg::BoundingBox bb;
            LODSet::iterator lod_itr;
            float smallestRadius=FLT_MAX;
            for(lod_itr=lodChildren.begin();
                lod_itr!=lodChildren.end();
                ++lod_itr)
            {
                float r = (*lod_itr)->getBound().radius();
                if (r>=0 && r<smallestRadius) smallestRadius = r;
                bb.expandBy((*lod_itr)->getCenter());
            }
            if (bb.radius()<smallestRadius*0.1f)
            {
                typedef std::pair<float,float> RangePair;
                typedef std::multimap<RangePair,osg::Node*> RangeMap;
                RangeMap rangeMap;
                for(lod_itr=lodChildren.begin();
                    lod_itr!=lodChildren.end();
                    ++lod_itr)
                {

                    osg::LOD* lod = *lod_itr;
                    for(unsigned int i=0;i<lod->getNumRanges();++i)
                    {
                        rangeMap.insert(RangeMap::value_type(RangePair(lod->getMinRange(i),lod->getMaxRange(i)),lod->getChild(i)));
                    }

                }

                // create new LOD containing all other LOD's children.
                osg::LOD* newLOD = osgNew osg::LOD;
                newLOD->setName("newLOD");
                newLOD->setCenter(bb.center());

                int i=0;
                for(RangeMap::iterator c_itr=rangeMap.begin();
                    c_itr!=rangeMap.end();
                    ++c_itr,++i)
                {
                    newLOD->setRange(i,c_itr->first.first,c_itr->first.second);
                    newLOD->addChild(c_itr->second);
                }

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

////////////////////////////////////////////////////////////////////////////
// code to merge geometry object which share, state, and attribute bindings.
////////////////////////////////////////////////////////////////////////////

struct LessGeometry
{
    bool operator() (const osg::Geometry* lhs,const osg::Geometry* rhs) const
    {
        if (lhs->getStateSet()<rhs->getStateSet()) return true;
        if (rhs->getStateSet()<lhs->getStateSet()) return false;
        
        if (lhs->getNormalBinding()<rhs->getNormalBinding()) return true;
        if (rhs->getNormalBinding()<lhs->getNormalBinding()) return false;

        if (lhs->getColorBinding()<rhs->getColorBinding()) return true;
        if (rhs->getColorBinding()<lhs->getColorBinding()) return false;
        
        if (lhs->getSecondaryColorBinding()<rhs->getSecondaryColorBinding()) return true;
        if (rhs->getSecondaryColorBinding()<lhs->getSecondaryColorBinding()) return false;

        if (lhs->getFogCoordBinding()<rhs->getFogCoordBinding()) return true;
        if (rhs->getFogCoordBinding()<lhs->getFogCoordBinding()) return false;

        if (lhs->getNumTexCoordArrays()<rhs->getNumTexCoordArrays()) return true;
        if (rhs->getNumTexCoordArrays()<lhs->getNumTexCoordArrays()) return false;
    
        // therefore lhs->getNumTexCoordArrays()==rhs->getNumTexCoordArrays()
        
        for(unsigned int i=0;i<lhs->getNumTexCoordArrays();++i)
        {
            if (rhs->getTexCoordArray(i))
            {
                if (!lhs->getTexCoordArray(i)) return true;
            }
            else if (lhs->getTexCoordArray(i)) return false;
        }
        
        if (lhs->getNormalBinding()==osg::Geometry::BIND_OVERALL)
        {
            // assumes that the bindings and arrays are set up correctly, this
            // should be the case after running computeCorrectBindingsAndArraySizes();
            const osg::Vec3& lhs_normal = (*(lhs->getNormalArray()))[0];
            const osg::Vec3& rhs_normal = (*(rhs->getNormalArray()))[0];
            if (lhs_normal<rhs_normal) return true;            
            if (rhs_normal<lhs_normal) return false;            
        }
        
        if (lhs->getColorBinding()==osg::Geometry::BIND_OVERALL)
        {
            const osg::Array* lhs_colorArray = lhs->getColorArray();
            const osg::Array* rhs_colorArray = rhs->getColorArray();
            if (lhs_colorArray->getType()<rhs_colorArray->getType()) return true;
            if (rhs_colorArray->getType()<lhs_colorArray->getType()) return false;
            switch(lhs_colorArray->getType())
            {
                case(osg::Array::UByte4ArrayType):
                    if ((*static_cast<const osg::UByte4Array*>(lhs_colorArray))[0]<(*static_cast<const osg::UByte4Array*>(rhs_colorArray))[0]) return true;
                    if ((*static_cast<const osg::UByte4Array*>(rhs_colorArray))[0]<(*static_cast<const osg::UByte4Array*>(lhs_colorArray))[0]) return false;
                case(osg::Array::Vec3ArrayType):
                    if ((*static_cast<const osg::Vec3Array*>(lhs_colorArray))[0]<(*static_cast<const osg::Vec3Array*>(rhs_colorArray))[0]) return true;
                    if ((*static_cast<const osg::Vec3Array*>(rhs_colorArray))[0]<(*static_cast<const osg::Vec3Array*>(lhs_colorArray))[0]) return false;
                case(osg::Array::Vec4ArrayType):
                    if ((*static_cast<const osg::Vec4Array*>(lhs_colorArray))[0]<(*static_cast<const osg::Vec4Array*>(rhs_colorArray))[0]) return true;
                    if ((*static_cast<const osg::Vec4Array*>(rhs_colorArray))[0]<(*static_cast<const osg::Vec4Array*>(lhs_colorArray))[0]) return false;
                    break;
                default:
                    break;
            }
            
        }

        return false;

    }
};

struct LessGeometryPrimitiveType
{
    bool operator() (const osg::Geometry* lhs,const osg::Geometry* rhs) const
    {
        for(unsigned int i=0;
            i<lhs->getNumPrimitiveSets() && i<rhs->getNumPrimitiveSets();
            ++i)
        {
            if (lhs->getPrimitiveSet(i)->getType()<rhs->getPrimitiveSet(i)->getType()) return true;
            else if (rhs->getPrimitiveSet(i)->getType()<lhs->getPrimitiveSet(i)->getType()) return false;

            if (lhs->getPrimitiveSet(i)->getMode()<rhs->getPrimitiveSet(i)->getMode()) return true;
            else if (rhs->getPrimitiveSet(i)->getMode()<lhs->getPrimitiveSet(i)->getMode()) return false;

        }
        return lhs->getNumPrimitiveSets()<rhs->getNumPrimitiveSets();
    }
};

bool Optimizer::MergeGeometryVisitor::mergeGeode(osg::Geode& geode)
{
    if (geode.getNumDrawables()>=2)
    {
    
        typedef std::vector<osg::Geometry*>                         DuplicateList;
        typedef std::map<osg::Geometry*,DuplicateList,LessGeometry> GeometryDuplicateMap;

        GeometryDuplicateMap geometryDuplicateMap;

        unsigned int i;
        for(i=0;i<geode.getNumDrawables();++i)
        {
            osg::Geometry* geom = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
            if (geom)
            {
                geom->computeCorrectBindingsAndArraySizes();

                geometryDuplicateMap[geom].push_back(geom);
            }
        }

        for(GeometryDuplicateMap::iterator itr=geometryDuplicateMap.begin();
            itr!=geometryDuplicateMap.end();
            ++itr)
        {
            if (itr->second.size()>1)
            {
                std::sort(itr->second.begin(),itr->second.end(),LessGeometryPrimitiveType());
                osg::Geometry* lhs = itr->second[0];
                for(DuplicateList::iterator dupItr=itr->second.begin()+1;
                    dupItr!=itr->second.end();
                    ++dupItr)
                {
                    osg::Geometry* rhs = *dupItr;
                    if (mergeGeometry(*lhs,*rhs))
                    {
                        geode.removeDrawable(rhs);

                        static int co = 0;
                        osg::notify(osg::INFO)<<"merged and removed Geometry "<<++co<<std::endl;
                    }
                }
            }
        }
    }

    // convert all polygon primitives which has 3 indices into TRIANGLES, 4 indices into QUADS.
    unsigned int i;
    for(i=0;i<geode.getNumDrawables();++i)
    {
        osg::Geometry* geom = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
        if (geom)
        {
            osg::Geometry::PrimitiveSetList& primitives = geom->getPrimitiveSetList();
            for(osg::Geometry::PrimitiveSetList::iterator itr=primitives.begin();
                itr!=primitives.end();
                ++itr)
            {
                osg::PrimitiveSet* prim = itr->get();
                if (prim->getMode()==osg::PrimitiveSet::POLYGON)
                {
                    if (prim->getNumIndices()==3)
                    {
                        prim->setMode(osg::PrimitiveSet::TRIANGLES);
                    }
                    else if (prim->getNumIndices()==4)
                    {
                        prim->setMode(osg::PrimitiveSet::QUADS);
                    }
                }
            }
        }
    }


    // now merge any compatible primtives.
    for(i=0;i<geode.getNumDrawables();++i)
    {
        osg::Geometry* geom = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
        if (geom)
        {
            if (geom->getNumPrimitiveSets()>0 &&
                geom->getNormalBinding()!=osg::Geometry::BIND_PER_PRIMITIVE_SET &&
                geom->getColorBinding()!=osg::Geometry::BIND_PER_PRIMITIVE_SET &&
                geom->getSecondaryColorBinding()!=osg::Geometry::BIND_PER_PRIMITIVE_SET &&
                geom->getFogCoordBinding()!=osg::Geometry::BIND_PER_PRIMITIVE_SET)
            {
                osg::Geometry::PrimitiveSetList& primitives = geom->getPrimitiveSetList();
                unsigned int primNo=0;
                while(primNo+1<primitives.size())
                {
                    osg::PrimitiveSet* lhs = primitives[primNo].get();
                    osg::PrimitiveSet* rhs = primitives[primNo+1].get();
 
                    bool combine = false;

                    if (lhs->getType()==rhs->getType() &&
                        lhs->getMode()==rhs->getMode())
                    {

                        switch(lhs->getMode())
                        {
                        case(osg::PrimitiveSet::POINTS):
                        case(osg::PrimitiveSet::LINES):
                        case(osg::PrimitiveSet::TRIANGLES):
                        case(osg::PrimitiveSet::QUADS):
                            combine = true;       
                            break;
                        }
                        
                    }

                    if (combine)
                    {
                    
                        switch(lhs->getType())
                        {
                        case(osg::PrimitiveSet::DrawArraysPrimitiveType):
                            combine = mergePrimitive(*(static_cast<osg::DrawArrays*>(lhs)),*(static_cast<osg::DrawArrays*>(rhs)));
                            break;
                        case(osg::PrimitiveSet::DrawArrayLengthsPrimitiveType):
                            combine = mergePrimitive(*(static_cast<osg::DrawArrayLengths*>(lhs)),*(static_cast<osg::DrawArrayLengths*>(rhs)));
                            break;
                        case(osg::PrimitiveSet::DrawElementsUBytePrimitiveType):
                            combine = mergePrimitive(*(static_cast<osg::DrawElementsUByte*>(lhs)),*(static_cast<osg::DrawElementsUByte*>(rhs)));
                            break;
                        case(osg::PrimitiveSet::DrawElementsUShortPrimitiveType):
                            combine = mergePrimitive(*(static_cast<osg::DrawElementsUShort*>(lhs)),*(static_cast<osg::DrawElementsUShort*>(rhs)));
                            break;
                        case(osg::PrimitiveSet::DrawElementsUIntPrimitiveType):
                            combine = mergePrimitive(*(static_cast<osg::DrawElementsUInt*>(lhs)),*(static_cast<osg::DrawElementsUInt*>(rhs)));
                            break;
                        default:
                            break;
                        }
                    }
                    if (combine)
                    {
                        primitives.erase(primitives.begin()+primNo+1);
                    }

                    if (!combine)
                    {
                        primNo++;
                    }
                }
            }
        }
    }

//    geode.dirtyBound();


    return false;
}

bool Optimizer::MergeGeometryVisitor::mergeGeometry(osg::Geometry& lhs,osg::Geometry& rhs)
{
    unsigned int base = 0;
    if (lhs.getVertexArray() && rhs.getVertexArray())
    {
        base = lhs.getVertexArray()->size();
        lhs.getVertexArray()->insert(lhs.getVertexArray()->end(),rhs.getVertexArray()->begin(),rhs.getVertexArray()->end());
    }
    else if (rhs.getVertexArray())
    {
        lhs.setVertexArray(rhs.getVertexArray());
    }
    
    if (lhs.getNormalArray() && rhs.getNormalArray() && lhs.getNormalBinding()!=osg::Geometry::BIND_OVERALL)
    {
        lhs.getNormalArray()->insert(lhs.getNormalArray()->end(),rhs.getNormalArray()->begin(),rhs.getNormalArray()->end());
    }
    else if (rhs.getNormalArray())
    {
        lhs.setNormalArray(rhs.getNormalArray());
    }

    if (lhs.getColorArray() && rhs.getColorArray() && lhs.getColorBinding()!=osg::Geometry::BIND_OVERALL)
    {
        // we need to add the handling of the other array types...
        osg::Vec4Array* col_lhs = dynamic_cast<osg::Vec4Array*>(lhs.getColorArray());
        osg::Vec4Array* col_rhs = dynamic_cast<osg::Vec4Array*>(rhs.getColorArray());
        
        if (col_lhs && col_rhs)
        {
            col_lhs->insert(col_lhs->end(),col_rhs->begin(),col_rhs->end());
        }
    }
    else if (rhs.getColorArray())
    {
        lhs.setColorArray(rhs.getColorArray());
    }
    
    // need to implement handle secondary color array.
    
    // need to implement handle fog coord array.

    for(unsigned int unit=0;unit<lhs.getNumTexCoordArrays();++unit)
    {
        // we need to add the handling of the other array types...
        osg::Vec2Array* tex_lhs = dynamic_cast<osg::Vec2Array*>(lhs.getTexCoordArray(unit));
        osg::Vec2Array* tex_rhs = dynamic_cast<osg::Vec2Array*>(rhs.getTexCoordArray(unit));
        
        if (tex_lhs && tex_rhs)
        {
            tex_lhs->insert(tex_lhs->end(),tex_rhs->begin(),tex_rhs->end());
        }
    }
    
    // shift the indices of the incomming primitives to account for the pre exisiting geometry.
    for(osg::Geometry::PrimitiveSetList::iterator primItr=rhs.getPrimitiveSetList().begin();
        primItr!=rhs.getPrimitiveSetList().end();
        ++primItr)
    {
        osg::PrimitiveSet* primitive = primItr->get();
        primitive->offsetIndices(base);
    }
    
    lhs.getPrimitiveSetList().insert(lhs.getPrimitiveSetList().end(),
                                     rhs.getPrimitiveSetList().begin(),rhs.getPrimitiveSetList().end());

    lhs.dirtyBound();
    lhs.dirtyDisplayList();
    
    return true;
}

bool Optimizer::MergeGeometryVisitor::mergePrimitive(osg::DrawArrays& lhs,osg::DrawArrays& rhs)
{
    if (lhs.getFirst()+lhs.getCount()==rhs.getFirst())
    {
        lhs.setCount(lhs.getCount()+rhs.getCount());
        return true;
    }
    return false;
}

bool Optimizer::MergeGeometryVisitor::mergePrimitive(osg::DrawArrayLengths& lhs,osg::DrawArrayLengths& rhs)
{
    int lhs_count = std::accumulate(lhs.begin(),lhs.end(),0);

    if (lhs.getFirst()+lhs_count==rhs.getFirst())
    {
        lhs.insert(lhs.end(),rhs.begin(),rhs.end());
        return true;
    }
    return false;
}

bool Optimizer::MergeGeometryVisitor::mergePrimitive(osg::DrawElementsUByte& lhs,osg::DrawElementsUByte& rhs)
{
    lhs.insert(lhs.end(),rhs.begin(),rhs.end());
    return true;
}

bool Optimizer::MergeGeometryVisitor::mergePrimitive(osg::DrawElementsUShort& lhs,osg::DrawElementsUShort& rhs)
{
    lhs.insert(lhs.end(),rhs.begin(),rhs.end());
    return true;
}

bool Optimizer::MergeGeometryVisitor::mergePrimitive(osg::DrawElementsUInt& lhs,osg::DrawElementsUInt& rhs)
{
    lhs.insert(lhs.end(),rhs.begin(),rhs.end());
    return true;
}
