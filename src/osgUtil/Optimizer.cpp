#include <osgUtil/Optimizer>

#include <osg/Transform>
#include <osg/LOD>
#include <osg/Impostor>
#include <osg/Billboard>
#include <osg/Geometry>
#include <osg/GeoSet>
#include <osg/Notify>

#include <typeinfo>
#include <algorithm>

using namespace osgUtil;

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
        FlattenStaticTransformsVisitor fstv;
        node->accept(fstv);
        fstv.removeTransforms();

// the following RemoveLowestStaticTransformVisitor doesn't yet work
// properly, will need further work.... Robert Osfield, June 2002.
//         int i=0;
//         bool result = false;
//         do
//         {
//             cout << "************ RemoveLowestStaticTransformsVisitor "<<i<<std::endl;
//             RemoveLowestStaticTransformsVisitor fstv;
//             node->accept(fstv);
//             result = fstv.removeTransforms();
//             ++i;
//         } while (result);
        
        
        
    }
    
    if (options & REMOVE_REDUNDENT_NODES)
    {

        RemoveEmptyNodesVisitor renv;
        node->accept(renv);
        renv.removeEmptyNodes();

        RemoveRedundentNodesVisitor rrnv;
        node->accept(rrnv);
        rrnv.removeRedundentNodes();

    }
    
    // convert the old style GeoSet to Geometry
//    ConvertGeoSetsToGeometryVisitor cgtg;
//    node->accept(cgtg);


    if (options & SHARE_DUPLICATE_STATE)
    {
    #if (defined(_MSC_VER) && _MSC_VER<1300 && !defined(_STLPORT_VERSION))
        osg::notify(osg::NOTICE)<<"Warning: VisualStudio 6.0 native STL build, unable to run state optimizer due to bugs in its STL."<<std::endl; 
        osg::notify(osg::NOTICE)<<"         This may impare performance significantly for larger models.  It is recommend that one "<<std::endl; 
        osg::notify(osg::NOTICE)<<"         compiles against STLport or use Visual Studio .NET compiler which also fix these VS 6.0 STL bugs "<<std::endl; 
    #else
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
// Convert GeoSet To Geometry Visitor.
////////////////////////////////////////////////////////////////////////////

void Optimizer::ConvertGeoSetsToGeometryVisitor::apply(osg::Geode& geode)
{
    for(int i=0;i<geode.getNumDrawables();++i)
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
    for(int i=0;i<geode.getNumDrawables();++i)
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

void Optimizer::FlattenStaticTransformsVisitor::apply(osg::Geode& geode)
{

    osg::Matrix matrix;
    if (!_matrixStack.empty()) matrix = _matrixStack.back();

    for(int i=0;i<geode.getNumDrawables();++i)
    {
        // register each drawable with the objectMap.
        _objectMap[geode.getDrawable(i)].add(_transformStack,matrix);
    }

}

void Optimizer::FlattenStaticTransformsVisitor::apply(osg::Billboard& billboard)
{
    if (!_matrixStack.empty())
    {
        // register ourselves with the objectMap.
        _objectMap[&billboard].add(_transformStack,_matrixStack.back());
    }
}

void Optimizer::FlattenStaticTransformsVisitor::apply(osg::LOD& lod)
{
    if (!_matrixStack.empty())
    {
        // register ourselves with the objectMap.
        _objectMap[&lod].add(_transformStack,_matrixStack.back());
    }
    
    traverse(lod);
}

void Optimizer::FlattenStaticTransformsVisitor::apply(osg::Transform& transform)
{
    if (_ignoreDynamicTransforms && transform.getDataVariance()==osg::Object::DYNAMIC) 
    {
        // simple traverse the children as if this Transform didn't exist.
        traverse(transform);
    }
    else
    {        
        osg::ref_ptr<osg::Matrix> matrix;
        if (_matrixStack.empty())
        {
            matrix = osgNew osg::Matrix;
        }
        else
        {
            matrix = osgNew osg::Matrix(_matrixStack.back());
        }

        transform.getLocalToWorldMatrix(*matrix,this);

        _matrixStack.push_back(*matrix);

        _transformStack.push_back(&transform);

        // simple traverse the children as if this Transform didn't exist.
        traverse(transform);

        _transformStack.pop_back();
        
        _matrixStack.pop_back();
    }
}

void Optimizer::FlattenStaticTransformsVisitor::doTransform(osg::Object* obj,osg::Matrix& matrix)
{
    osg::Drawable* drawable = dynamic_cast<osg::Drawable*>(obj);
    if (drawable)
    {
        TransformFunctor tf(matrix);
        drawable->applyAttributeOperation(tf);
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
            lod->setRange(i,lod->getRange(i)*ratio);
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

        for(int i=0;i<billboard->getNumDrawables();++i)
        {
            billboard->setPos(i,billboard->getPos(i)*matrix);
            billboard->getDrawable(i)->applyAttributeOperation(tf);
        }
        
        billboard->dirtyBound();

        return;
    }
}

void Optimizer::FlattenStaticTransformsVisitor::disableObject(ObjectMap::iterator itr)
{
    if (itr==_objectMap.end())
    {
        // Euston we have a problem..
        osg::notify(osg::WARN)<<"Warning: internal error Optimizer::FlattenStaticTransformsVisitor::disableObject()"<<std::endl;
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

void Optimizer::FlattenStaticTransformsVisitor::disableTransform(osg::Transform* transform)
{
    TransformMap::iterator itr=_transformMap.find(transform);
    if (itr==_transformMap.end())
    {
        // Euston we have a problem..
        osg::notify(osg::WARN)<<"Warning: internal error Optimizer::FlattenStaticTransformsVisitor::disableTransform()"<<std::endl;
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

void Optimizer::FlattenStaticTransformsVisitor::removeTransforms()
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

    // transform the objects that can be applied.
    for(oitr=_objectMap.begin();
        oitr!=_objectMap.end();
        ++oitr)
    {
        osg::Object* object = oitr->first;
        ObjectStruct& os = oitr->second;
        if (os._canBeApplied)
        {
            doTransform(object,os._matrix);
        }
    }


    // clean up the transforms.
    for(TransformMap::iterator titr=_transformMap.begin();
        titr!=_transformMap.end();
        ++titr)
    {
        if (titr->second._canBeApplied)
        {
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
    _transformStack.clear();
    _matrixStack.clear();
}

////////////////////////////////////////////////////////////////////////////
// Flatten static transforms
////////////////////////////////////////////////////////////////////////////

void Optimizer::RemoveLowestStaticTransformsVisitor::apply(osg::Geode& geode)
{
    osg::Transform* transform = NULL;
    if (!_transformStack.back()) transform = _transformStack.back();

    osg::Matrix matrix;
    if (!_matrixStack.empty()) matrix = _matrixStack.back();

    for(int i=0;i<geode.getNumDrawables();++i)
    {
        // register each drawable with the objectMap.
        _objectMap[geode.getDrawable(i)].add(transform,matrix);
    }
}

void Optimizer::RemoveLowestStaticTransformsVisitor::apply(osg::Billboard& billboard)
{
    if (!_matrixStack.empty())
    {
        // register ourselves with the objectMap.
        _objectMap[&billboard].add(_transformStack.back(),_matrixStack.back());
    }
}

void Optimizer::RemoveLowestStaticTransformsVisitor::apply(osg::LOD& lod)
{
    if (!_matrixStack.empty())
    {
        // register ourselves with the objectMap.
        _objectMap[&lod].add(_transformStack.back(),_matrixStack.back());
    }
    
    traverse(lod);
}

void Optimizer::RemoveLowestStaticTransformsVisitor::apply(osg::Transform& transform)
{


    if (_ignoreDynamicTransforms && transform.getDataVariance()==osg::Object::DYNAMIC) 
    {
        // simple traverse the children as if this Transform didn't exist.
        traverse(transform);
    }
    else
    {        
        if (!_matrixStack.empty())
        {
            _transformMap[&transform]._containsTransform=false;
        }

        osg::Matrix matrix;
        transform.getLocalToWorldMatrix(matrix,this);

        _matrixStack.push_back(matrix);

        _transformStack.push_back(&transform);

        // simple traverse the children as if this Transform didn't exist.
        traverse(transform);

        _transformStack.pop_back();
        
        _matrixStack.pop_back();
    }
}

void Optimizer::RemoveLowestStaticTransformsVisitor::doTransform(osg::Object* obj,osg::Matrix& matrix)
{
    osg::Drawable* drawable = dynamic_cast<osg::Drawable*>(obj);
    if (drawable)
    {
        TransformFunctor tf(matrix);
        drawable->applyAttributeOperation(tf);
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
            lod->setRange(i,lod->getRange(i)*ratio);
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

        for(int i=0;i<billboard->getNumDrawables();++i)
        {
            billboard->setPos(i,billboard->getPos(i)*matrix);
            billboard->getDrawable(i)->applyAttributeOperation(tf);
        }
        
        billboard->dirtyBound();

        return;
    }
}

void Optimizer::RemoveLowestStaticTransformsVisitor::disableObject(ObjectMap::iterator itr)
{
    if (itr==_objectMap.end())
    {
        // Euston we have a problem..
        osg::notify(osg::WARN)<<"Warning: internal error Optimizer::RemoveLowestStaticTransformsVisitor::disableObject()"<<std::endl;
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

void Optimizer::RemoveLowestStaticTransformsVisitor::disableTransform(osg::Transform* transform)
{
    TransformMap::iterator itr=_transformMap.find(transform);
    if (itr==_transformMap.end())
    {
        // Euston we have a problem..
        osg::notify(osg::WARN)<<"Warning: internal error Optimizer::RemoveLowestStaticTransformsVisitor::disableTransform()"<<std::endl;
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

bool Optimizer::RemoveLowestStaticTransformsVisitor::removeTransforms()
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
    
    TransformMap::iterator titr;
    for(titr=_transformMap.begin();
        titr!=_transformMap.end();
        ++titr)
    {
        TransformStruct& ts = titr->second;
        if (ts._containsTransform)
        {
            disableTransform(titr->first);
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


    // transform the objects that can be applied.
    for(oitr=_objectMap.begin();
        oitr!=_objectMap.end();
        ++oitr)
    {
        osg::Object* object = oitr->first;
        ObjectStruct& os = oitr->second;
        if (os._canBeApplied)
        {
            doTransform(object,os._matrix);
        }
    }


    bool transformsRemoved = false;

    // clean up the transforms.
    for(titr=_transformMap.begin();
        titr!=_transformMap.end();
        ++titr)
    {
        if (titr->second._canBeApplied && titr->first)
        {
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
            transformsRemoved = true;
        }
    }
    _objectMap.clear();
    _transformMap.clear();
    _transformStack.clear();
    _matrixStack.clear();
    
    return transformsRemoved;
}


////////////////////////////////////////////////////////////////////////////
// RemoveEmptyNodes.
////////////////////////////////////////////////////////////////////////////

void Optimizer::RemoveEmptyNodesVisitor::apply(osg::Geode& geode)
{
    if (geode.getNumParents()>0)
    {
        if (geode.getNumDrawables()==0) _redundentNodeList.insert(&geode);
    }
}

void Optimizer::RemoveEmptyNodesVisitor::apply(osg::Group& group)
{
    if (group.getNumParents()>0)
    {
        if (group.getNumChildren()==0)
        {
            _redundentNodeList.insert(&group);
        }
    }
    traverse(group);
}

void Optimizer::RemoveEmptyNodesVisitor::removeEmptyNodes()
{

    NodeList newEmptyGroups;

    // keep iterator through until scene graph is cleaned of empty nodes.
    while (!_redundentNodeList.empty())
    {
        for(NodeList::iterator itr=_redundentNodeList.begin();
            itr!=_redundentNodeList.end();
            ++itr)
        {
            
            osg::Node* nodeToRemove = (*itr);
            
            // take a copy of parents list since subsequent removes will modify the original one.
            osg::Node::ParentList parents = nodeToRemove->getParents();

            for(osg::Node::ParentList::iterator pitr=parents.begin();
                pitr!=parents.end();
                ++pitr)
            {
                (*pitr)->removeChild(nodeToRemove);
                if ((*pitr)->getNumChildren()==0) newEmptyGroups.insert(*pitr);
            }                
        }

        _redundentNodeList.clear();
        _redundentNodeList.swap(newEmptyGroups);
    }
}


////////////////////////////////////////////////////////////////////////////
// RemoveRedundentNodes.
////////////////////////////////////////////////////////////////////////////

void Optimizer::RemoveRedundentNodesVisitor::apply(osg::Group& group)
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
                    _redundentNodeList.insert(&group);
                }
            }
        }
    }
    traverse(group);
}

void Optimizer::RemoveRedundentNodesVisitor::apply(osg::Transform& transform)
{
    if (transform.getNumParents()>0 && transform.getDataVariance()==osg::Object::STATIC)
    {
        static osg::Matrix identity;
        osg::Matrix matrix;
        transform.getWorldToLocalMatrix(matrix,NULL);
        if (matrix==identity)
        {
            _redundentNodeList.insert(&transform);
        }
    }
    traverse(transform);
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
    _redundentNodeList.clear();
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
                    for(unsigned int i=0;i<lod->getNumRanges()-1;++i)
                    {
                        if (maxRange<lod->getRange(i+1)) maxRange = lod->getRange(i+1);
                        rangeMap.insert(RangeMap::value_type(RangePair(lod->getRange(i),lod->getRange(i+1)),lod->getChild(i)));
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

