/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/
#include <osgUtil/Optimizer>

#include <osg/ApplicationUsage>
#include <osg/Transform>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/LOD>
#include <osg/Billboard>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/OccluderNode>
#include <osg/Sequence>
#include <osg/Switch>
#include <osg/Texture>
#include <osg/PagedLOD>
#include <osg/ProxyNode>

#include <osgUtil/TransformAttributeFunctor>
#include <osgUtil/TriStripVisitor>
#include <osgUtil/Tesselator>

#include <typeinfo>
#include <algorithm>
#include <numeric>

using namespace osgUtil;



void Optimizer::reset()
{
}

static osg::ApplicationUsageProxy Optimizer_e0(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_OPTIMIZER \"<type> [<type>]\"","OFF | DEFAULT | FLATTEN_STATIC_TRANSFORMS | REMOVE_REDUNDANT_NODES | COMBINE_ADJACENT_LODS | SHARE_DUPLICATE_STATE | MERGE_GEOMETRY | SPATIALIZE_GROUPS  | COPY_SHARED_NODES  | TRISTRIP_GEOMETRY | OPTIMIZE_TEXTURE_SETTINGS | REMOVE_LOADED_PROXY_NODES | TESSELATE_GEOMETRY | CHECK_GEOMETRY");

void Optimizer::optimize(osg::Node* node)
{
    unsigned int options = 0;
    
    const char* env = getenv("OSG_OPTIMIZER");
    if (env)
    {
        std::string str(env);


        if(str.find("OFF")!=std::string::npos) options = 0;

        if(str.find("~DEFAULT")!=std::string::npos) options ^= DEFAULT_OPTIMIZATIONS;
        else if(str.find("DEFAULT")!=std::string::npos) options |= DEFAULT_OPTIMIZATIONS;

        if(str.find("~FLATTEN_STATIC_TRANSFORMS")!=std::string::npos) options ^= FLATTEN_STATIC_TRANSFORMS;
        else if(str.find("FLATTEN_STATIC_TRANSFORMS")!=std::string::npos) options |= FLATTEN_STATIC_TRANSFORMS;

        if(str.find("~REMOVE_REDUNDANT_NODES")!=std::string::npos) options ^= REMOVE_REDUNDANT_NODES;
        else if(str.find("REMOVE_REDUNDANT_NODES")!=std::string::npos) options |= REMOVE_REDUNDANT_NODES;

        if(str.find("~REMOVE_LOADED_PROXY_NODES")!=std::string::npos) options ^= REMOVE_LOADED_PROXY_NODES;
        else if(str.find("REMOVE_LOADED_PROXY_NODES")!=std::string::npos) options |= REMOVE_LOADED_PROXY_NODES;

        if(str.find("~COMBINE_ADJACENT_LODS")!=std::string::npos) options ^= COMBINE_ADJACENT_LODS;
        else if(str.find("COMBINE_ADJACENT_LODS")!=std::string::npos) options |= COMBINE_ADJACENT_LODS;

        if(str.find("~SHARE_DUPLICATE_STATE")!=std::string::npos) options ^= SHARE_DUPLICATE_STATE;
        else if(str.find("SHARE_DUPLICATE_STATE")!=std::string::npos) options |= SHARE_DUPLICATE_STATE;

        if(str.find("~MERGE_GEODES")!=std::string::npos) options ^= MERGE_GEODES;
        else if(str.find("MERGE_GEODES")!=std::string::npos) options |= MERGE_GEODES;

        if(str.find("~MERGE_GEOMETRY")!=std::string::npos) options ^= MERGE_GEOMETRY;
        else if(str.find("MERGE_GEOMETRY")!=std::string::npos) options |= MERGE_GEOMETRY;

        if(str.find("~SPATIALIZE_GROUPS")!=std::string::npos) options ^= SPATIALIZE_GROUPS;
        else if(str.find("SPATIALIZE_GROUPS")!=std::string::npos) options |= SPATIALIZE_GROUPS;

        if(str.find("~COPY_SHARED_NODES")!=std::string::npos) options ^= COPY_SHARED_NODES;
        else if(str.find("COPY_SHARED_NODES")!=std::string::npos) options |= COPY_SHARED_NODES;

        if(str.find("~TESSELATE_GEOMETRY")!=std::string::npos) options ^= TESSELATE_GEOMETRY;
        else if(str.find("TESSELATE_GEOMETRY")!=std::string::npos) options |= TESSELATE_GEOMETRY;

        if(str.find("~TRISTRIP_GEOMETRY")!=std::string::npos) options ^= TRISTRIP_GEOMETRY;
        else if(str.find("TRISTRIP_GEOMETRY")!=std::string::npos) options |= TRISTRIP_GEOMETRY;

        if(str.find("~OPTIMIZE_TEXTURE_SETTINGS")!=std::string::npos) options ^= OPTIMIZE_TEXTURE_SETTINGS;
        else if(str.find("OPTIMIZE_TEXTURE_SETTINGS")!=std::string::npos) options |= OPTIMIZE_TEXTURE_SETTINGS;

        if(str.find("~CHECK_GEOMETRY")!=std::string::npos) options ^= CHECK_GEOMETRY;
        else if(str.find("CHECK_GEOMETRY")!=std::string::npos) options |= CHECK_GEOMETRY;

    }
    else
    {
        options = DEFAULT_OPTIMIZATIONS;
    }

    optimize(node,options);
}

void Optimizer::optimize(osg::Node* node, unsigned int options)
{
    if (options & TESSELATE_GEOMETRY)
    {
        osg::notify(osg::INFO)<<"Optimizer::optimize() doing TESSELATE_GEOMETRY"<<std::endl;

        TesselateVisitor tsv;
        node->accept(tsv);        
    }
    
    if (options & REMOVE_LOADED_PROXY_NODES)
    {
        osg::notify(osg::INFO)<<"Optimizer::optimize() doing REMOVE_LOADED_PROXY_NODES"<<std::endl;

        RemoveLoadedProxyNodesVisitor rlpnv(this);
        node->accept(rlpnv);
        rlpnv.removeRedundantNodes();

    }

    if (options & COMBINE_ADJACENT_LODS)
    {
        osg::notify(osg::INFO)<<"Optimizer::optimize() doing COMBINE_ADJACENT_LODS"<<std::endl;

        CombineLODsVisitor clv(this);
        node->accept(clv);        
        clv.combineLODs();
    }
    
    if (options & OPTIMIZE_TEXTURE_SETTINGS)
    {
        osg::notify(osg::INFO)<<"Optimizer::optimize() doing OPTIMIZE_TEXTURE_SETTINGS"<<std::endl;

        TextureVisitor tv(true,true, // unref image 
                          false,false, // client storage
                          false,1.0, // anisotropic filtering
                          this );
        node->accept(tv);
    }

    if (options & SHARE_DUPLICATE_STATE)
    {
        osg::notify(osg::INFO)<<"Optimizer::optimize() doing SHARE_DUPLICATE_STATE"<<std::endl;

        StateVisitor osv(this);
        node->accept(osv);
        osv.optimize();
    }
    
    if (options & COPY_SHARED_NODES)
    {
        osg::notify(osg::INFO)<<"Optimizer::optimize() doing COPY_SHARED_NODES"<<std::endl;

        CopySharedSubgraphsVisitor cssv(this);
        node->accept(cssv);
        cssv.copySharedNodes();
    }
    
    if (options & FLATTEN_STATIC_TRANSFORMS)
    {
        osg::notify(osg::INFO)<<"Optimizer::optimize() doing FLATTEN_STATIC_TRANSFORMS"<<std::endl;

        int i=0;
        bool result = false;
        do
        {
            osg::notify(osg::DEBUG_INFO) << "** RemoveStaticTransformsVisitor *** Pass "<<i<<std::endl;
            FlattenStaticTransformsVisitor fstv(this);
            node->accept(fstv);
            result = fstv.removeTransforms(node);
            ++i;
        } while (result);

        // no combine any adjacent static transforms.
        CombineStaticTransformsVisitor cstv(this);
        node->accept(cstv);
        cstv.removeTransforms(node);

    }
    

    if (options & REMOVE_REDUNDANT_NODES)
    {
        osg::notify(osg::INFO)<<"Optimizer::optimize() doing REMOVE_REDUNDANT_NODES"<<std::endl;

        RemoveEmptyNodesVisitor renv(this);
        node->accept(renv);
        renv.removeEmptyNodes();

        RemoveRedundantNodesVisitor rrnv(this);
        node->accept(rrnv);
        rrnv.removeRedundantNodes();

    }
    
    if (options & MERGE_GEODES)
    {
        osg::notify(osg::INFO)<<"Optimizer::optimize() doing MERGE_GEODES"<<std::endl;

        MergeGeodesVisitor visitor;
        node->accept(visitor);
    }

    if (options & CHECK_GEOMETRY)
    {
        osg::notify(osg::INFO)<<"Optimizer::optimize() doing CHECK_GEOMETRY"<<std::endl;

        CheckGeometryVisitor mgv(this);
        node->accept(mgv);
    }

    if (options & MERGE_GEOMETRY)
    {
        osg::notify(osg::INFO)<<"Optimizer::optimize() doing MERGE_GEOMETRY"<<std::endl;

        MergeGeometryVisitor mgv(this);
        mgv.setTargetMaximumNumberOfVertices(10000);
        node->accept(mgv);
    }
    
    if (options & TRISTRIP_GEOMETRY)
    {
        osg::notify(osg::INFO)<<"Optimizer::optimize() doing TRISTRIP_GEOMETRY"<<std::endl;

        TriStripVisitor tsv(this);
        node->accept(tsv);
        tsv.stripify();
    }

    if (options & SPATIALIZE_GROUPS)
    {
        osg::notify(osg::INFO)<<"Optimizer::optimize() doing SPATIALIZE_GROUPS"<<std::endl;

        SpatializeGroupsVisitor sv(this);
        node->accept(sv);
        sv.divide();
    }
    
}


////////////////////////////////////////////////////////////////////////////
// Tesselate geometry - eg break complex POLYGONS into triangles, strips, fans..
////////////////////////////////////////////////////////////////////////////
void Optimizer::TesselateVisitor::apply(osg::Geode& geode)
{
    for(unsigned int i=0;i<geode.getNumDrawables();++i)
    {
        osg::Geometry* geom = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
        if (geom) {
            osgUtil::Tesselator tesselator;
            tesselator.retesselatePolygons(*geom);
        }
    }
    traverse(geode);
}


////////////////////////////////////////////////////////////////////////////
// Optimize State Visitor
////////////////////////////////////////////////////////////////////////////

template<typename T>
struct LessDerefFunctor 
{
    bool operator () (const T* lhs,const T* rhs) const
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
    if (ss && ss->getDataVariance()==osg::Object::STATIC) 
    {
        if (isOperationPermissibleForObject(&node) &&
            isOperationPermissibleForObject(ss))
        {
            addStateSet(ss,&node);
        }
    }

    traverse(node);
}

void Optimizer::StateVisitor::apply(osg::Geode& geode)
{
    if (!isOperationPermissibleForObject(&geode)) return;

    osg::StateSet* ss = geode.getStateSet();
    
    
    if (ss && ss->getDataVariance()==osg::Object::STATIC)
    {
        if (isOperationPermissibleForObject(ss))
        {
            addStateSet(ss,&geode);
        }
    }
    for(unsigned int i=0;i<geode.getNumDrawables();++i)
    {
        osg::Drawable* drawable = geode.getDrawable(i);
        if (drawable)
        {
            ss = drawable->getStateSet();
            if (ss && ss->getDataVariance()==osg::Object::STATIC)
            {
                if (isOperationPermissibleForObject(drawable) &&
                    isOperationPermissibleForObject(ss))
                {
                    addStateSet(ss,drawable);
                }
            }
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
        AttributeToStateSetMap attributeToStateSetMap;

        // create map from uniforms to stateset when contain them.
        typedef std::set<osg::StateSet*>                    StateSetSet;
        typedef std::map<osg::Uniform*,StateSetSet>         UniformToStateSetMap;
        
        const unsigned int NON_TEXTURE_ATTRIBUTE = 0xffffffff;
        
        UniformToStateSetMap  uniformToStateSetMap;

        // NOTE - TODO will need to track state attribute override value too.

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
                    attributeToStateSetMap[aitr->second.first.get()].insert(StateSetUnitPair(sitr->first,NON_TEXTURE_ATTRIBUTE));
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
                        attributeToStateSetMap[aitr->second.first.get()].insert(StateSetUnitPair(sitr->first,unit));
                    }
                }
            }


            osg::StateSet::UniformList& uniforms = sitr->first->getUniformList();
            for(osg::StateSet::UniformList::iterator uitr= uniforms.begin();
                uitr!=uniforms.end();
                ++uitr)
            {
                if (uitr->second.first->getDataVariance()==osg::Object::STATIC)
                {
                    uniformToStateSetMap[uitr->second.first.get()].insert(sitr->first);
                }
            }

        }

        if (attributeToStateSetMap.size()>=2)
        {
            // create unique set of state attribute pointers.
            typedef std::vector<osg::StateAttribute*> AttributeList;
            AttributeList attributeList;

            for(AttributeToStateSetMap::iterator aitr=attributeToStateSetMap.begin();
                aitr!=attributeToStateSetMap.end();
                ++aitr)
            {
                attributeList.push_back(aitr->first);
            }

            // sort the attributes so that equal attributes sit along side each
            // other.
            std::sort(attributeList.begin(),attributeList.end(),LessDerefFunctor<osg::StateAttribute>());

            osg::notify(osg::INFO) << "state attribute list"<< std::endl;
            for(AttributeList::iterator aaitr = attributeList.begin();
                aaitr!=attributeList.end();
                ++aaitr)
            {
                osg::notify(osg::INFO) << "    "<<*aaitr << "  "<<(*aaitr)->className()<< std::endl;
            }

            osg::notify(osg::INFO) << "searching for duplicate attributes"<< std::endl;
            // find the duplicates.
            AttributeList::iterator first_unique = attributeList.begin();
            AttributeList::iterator current = first_unique;
            ++current;
            for(; current!=attributeList.end();++current)
            {
                if (**current==**first_unique)
                {
                    osg::notify(osg::INFO) << "    found duplicate "<<(*current)->className()<<"  first="<<*first_unique<<"  current="<<*current<< std::endl;
                    StateSetList& statesetlist = attributeToStateSetMap[*current];
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
        
                
        if (uniformToStateSetMap.size()>=2)
        {
            // create unique set of uniform pointers.
            typedef std::vector<osg::Uniform*> UniformList;
            UniformList uniformList;

            for(UniformToStateSetMap::iterator aitr=uniformToStateSetMap.begin();
                aitr!=uniformToStateSetMap.end();
                ++aitr)
            {
                uniformList.push_back(aitr->first);
            }

            // sort the uniforms so that equal uniforms sit along side each
            // other.
            std::sort(uniformList.begin(),uniformList.end(),LessDerefFunctor<osg::Uniform>());

            osg::notify(osg::INFO) << "state uniform list"<< std::endl;
            for(UniformList::iterator uuitr = uniformList.begin();
                uuitr!=uniformList.end();
                ++uuitr)
            {
                osg::notify(osg::INFO) << "    "<<*uuitr << "  "<<(*uuitr)->getName()<< std::endl;
            }

            osg::notify(osg::INFO) << "searching for duplicate uniforms"<< std::endl;
            // find the duplicates.
            UniformList::iterator first_unique_uniform = uniformList.begin();
            UniformList::iterator current_uniform = first_unique_uniform; 
            ++current_uniform;
            for(; current_uniform!=uniformList.end();++current_uniform)
            {
                if ((**current_uniform)==(**first_unique_uniform))
                {
                    osg::notify(osg::INFO) << "    found duplicate uniform "<<(*current_uniform)->getName()<<"  first_unique_uniform="<<*first_unique_uniform<<"  current_uniform="<<*current_uniform<< std::endl;
                    StateSetSet& statesetset = uniformToStateSetMap[*current_uniform];
                    for(StateSetSet::iterator sitr=statesetset.begin();
                        sitr!=statesetset.end();
                        ++sitr)
                    {
                        osg::notify(osg::INFO) << "       replace duplicate "<<*current_uniform<<" with "<<*first_unique_uniform<< std::endl;
                        osg::StateSet* stateset = *sitr;
                        stateset->addUniform(*first_unique_uniform);
                    }
                }
                else first_unique_uniform = current_uniform;
            }
        }

    }
    
    // duplicate state attributes removed.
    // now need to look at duplicate state sets.
    if (_statesets.size()>=2)
    {
        // create the list of stateset's.
        typedef std::vector<osg::StateSet*> StateSetSortList;
        StateSetSortList statesetSortList;
        for(StateSetMap::iterator ssitr=_statesets.begin();
            ssitr!=_statesets.end();
            ++ssitr)
        {
            statesetSortList.push_back(ssitr->first);
        }


        // sort the StateSet's so that equal StateSet's sit along side each
        // other.
        std::sort(statesetSortList.begin(),statesetSortList.end(),LessDerefFunctor<osg::StateSet>());

        osg::notify(osg::INFO) << "searching for duplicate attributes"<< std::endl;
        // find the duplicates.
        StateSetSortList::iterator first_unique = statesetSortList.begin();
        StateSetSortList::iterator current = first_unique; ++current;
        for(; current!=statesetSortList.end();++current)
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

class CollectLowestTransformsVisitor : public BaseOptimizerVisitor
{
    public:


        CollectLowestTransformsVisitor(Optimizer* optimizer=0):
                    BaseOptimizerVisitor(optimizer,Optimizer::FLATTEN_STATIC_TRANSFORMS),
                    _transformFunctor(osg::Matrix())
        {
            setTraversalMode(osg::NodeVisitor::TRAVERSE_PARENTS);
        }

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
        

        void collectDataFor(osg::Node* node)
        {
            _currentObjectList.push_back(node);
            
            node->accept(*this);
            
            _currentObjectList.pop_back();
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
        bool removeTransforms(osg::Node* nodeWeCannotRemove);

        inline bool isOperationPermissibleForObject(const osg::Object* object) const
        {
            const osg::Drawable* drawable = dynamic_cast<const osg::Drawable*>(object);
            if (drawable) return isOperationPermissibleForObject(drawable);
            
            const osg::Node* node = dynamic_cast<const osg::Node*>(object);
            if (node) return isOperationPermissibleForObject(node);

            return true;
        }

        inline bool isOperationPermissibleForObject(const osg::Drawable* drawable) const
        {
            // disable if cannot apply transform functor.
            if (drawable && !drawable->supports(_transformFunctor)) return false;
            return BaseOptimizerVisitor::isOperationPermissibleForObject(drawable);
        }
            
        inline bool isOperationPermissibleForObject(const osg::Node* node) const
        {
            // disable if object is a light point node.
            if (strcmp(node->className(),"LightPointNode")==0) return false;
            if (dynamic_cast<const osg::ProxyNode*>(node)) return false;
            return BaseOptimizerVisitor::isOperationPermissibleForObject(node);
        }

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
                    else if (transform->getReferenceFrame()==osg::Transform::ABSOLUTE_RF) _moreThanOneMatrixRequired=true;
                    else
                    {
                        if (_transformSet.empty()) transform->computeLocalToWorldMatrix(_firstMatrix,0);
                        else
                        {
                            osg::Matrix matrix;
                            transform->computeLocalToWorldMatrix(matrix,0);
                            if (_firstMatrix!=matrix) _moreThanOneMatrixRequired=true;
                        }
                    }
                }
                else
                {
                    if (!_transformSet.empty()) 
                    {
                        if (_firstMatrix!=_identity_matrix) _moreThanOneMatrixRequired=true;
                    }

                }
                _transformSet.insert(transform);
            }

            bool            _canBeApplied;
            bool            _moreThanOneMatrixRequired;
            osg::Matrix     _firstMatrix;
            TransformSet    _transformSet;
            osg::Matrix     _identity_matrix; 

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
        
        osgUtil::TransformAttributeFunctor _transformFunctor;
        TransformMap    _transformMap;
        ObjectMap       _objectMap;
        ObjectList      _currentObjectList;

};


void CollectLowestTransformsVisitor::doTransform(osg::Object* obj,osg::Matrix& matrix)
{
    osg::Drawable* drawable = dynamic_cast<osg::Drawable*>(obj);
    if (drawable)
    {
        osgUtil::TransformAttributeFunctor tf(matrix);
        drawable->accept(tf);
        drawable->dirtyBound();
        drawable->dirtyDisplayList();

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
  
        osgUtil::TransformAttributeFunctor tf(matrix_no_trans);

        osg::Vec3 axis = osg::Matrix::transform3x3(tf._im,billboard->getAxis());
        axis.normalize();
        billboard->setAxis(axis);

        osg::Vec3 normal = osg::Matrix::transform3x3(tf._im,billboard->getNormal());
        normal.normalize();
        billboard->setNormal(normal);


        for(unsigned int i=0;i<billboard->getNumDrawables();++i)
        {
            billboard->setPosition(i,billboard->getPosition(i)*matrix);
            billboard->getDrawable(i)->accept(tf);
            billboard->getDrawable(i)->dirtyBound();
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
    // and disable all objects that the operation is not permisable for)
    for(oitr=_objectMap.begin();
        oitr!=_objectMap.end();
        ++oitr)
    {
        osg::Object* object = oitr->first;
        ObjectStruct& os = oitr->second;
        if (os._canBeApplied)
        {
            if (os._moreThanOneMatrixRequired || !isOperationPermissibleForObject(object))
            {
                disableObject(oitr);
            }
        }
    }

}

bool CollectLowestTransformsVisitor::removeTransforms(osg::Node* nodeWeCannotRemove)
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
            if (titr->first!=nodeWeCannotRemove)
            {
                transformRemoved = true;

                osg::ref_ptr<osg::Transform> transform = titr->first;
                osg::ref_ptr<osg::Group>     group = new osg::Group;
                group->setName( transform->getName() );
                group->setDataVariance(osg::Object::STATIC);
                group->setNodeMask(transform->getNodeMask());
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
            else
            {
                osg::MatrixTransform* mt = dynamic_cast<osg::MatrixTransform*>(titr->first);
                if (mt) mt->setMatrix(osg::Matrix::identity());
                else
                {
                    osg::PositionAttitudeTransform* pat = dynamic_cast<osg::PositionAttitudeTransform*>(titr->first);
                    if (pat)
                    {
                        pat->setPosition(osg::Vec3(0.0f,0.0f,0.0f));
                        pat->setAttitude(osg::Quat());
                        pat->setPivotPoint(osg::Vec3(0.0f,0.0f,0.0f));
                    }
                    else
                    {
                        osg::notify(osg::WARN)<<"Warning:: during Optimize::CollectLowestTransformsVisitor::removeTransforms(Node*)"<<std::endl;
                        osg::notify(osg::WARN)<<"          unhandled of setting of indentity matrix on "<< titr->first->className()<<std::endl;
                        osg::notify(osg::WARN)<<"          model will appear in the incorrect position."<<std::endl;
                    }
                }
                
            }                
        }
    }
    _objectMap.clear();
    _transformMap.clear();
    
    return transformRemoved;
}

void Optimizer::FlattenStaticTransformsVisitor::apply(osg::Node& node)
{
    if (strcmp(node.className(),"LightPointNode")==0)
    {
        _excludedNodeSet.insert(&node);
    }
    traverse(node);
}


void Optimizer::FlattenStaticTransformsVisitor::apply(osg::ProxyNode& node)
{
    _excludedNodeSet.insert(&node);

    traverse(node);
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

bool Optimizer::FlattenStaticTransformsVisitor::removeTransforms(osg::Node* nodeWeCannotRemove)
{
    CollectLowestTransformsVisitor cltv(_optimizer);

    for(NodeSet::iterator nitr=_excludedNodeSet.begin();
        nitr!=_excludedNodeSet.end();
        ++nitr)
    {
        cltv.collectDataFor(*nitr);
    }

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
    

    return cltv.removeTransforms(nodeWeCannotRemove);
}

////////////////////////////////////////////////////////////////////////////
// CombineStatucTransforms
////////////////////////////////////////////////////////////////////////////

void Optimizer::CombineStaticTransformsVisitor::apply(osg::MatrixTransform& transform)
{
    if (transform.getDataVariance()==osg::Object::STATIC &&
        transform.getNumChildren()==1 &&
        transform.getChild(0)->asTransform()!=0 &&
        transform.getChild(0)->asTransform()->asMatrixTransform()!=0 &&
        transform.getChild(0)->asTransform()->getDataVariance()==osg::Object::STATIC &&
        isOperationPermissibleForObject(&transform) && isOperationPermissibleForObject(transform.getChild(0)))
    {
        _transformSet.insert(&transform);
    }
    
    traverse(transform);
}

bool Optimizer::CombineStaticTransformsVisitor::removeTransforms(osg::Node* nodeWeCannotRemove)
{
    if (nodeWeCannotRemove && nodeWeCannotRemove->asTransform()!=0 && nodeWeCannotRemove->asTransform()->asMatrixTransform()!=0)
    {
        // remove topmost node if it exists in the transform set.
        TransformSet::iterator itr = _transformSet.find(nodeWeCannotRemove->asTransform()->asMatrixTransform());
        if (itr!=_transformSet.end()) _transformSet.erase(itr);
    }
    
    bool transformRemoved = false;

    while (!_transformSet.empty())
    {
        // get the first available transform to combine.
        osg::ref_ptr<osg::MatrixTransform> transform = *_transformSet.begin();
        _transformSet.erase(_transformSet.begin());
        
        if (transform->getNumChildren()==1 &&
            transform->getChild(0)->asTransform()!=0 &&
            transform->getChild(0)->asTransform()->asMatrixTransform()!=0 &&
            transform->getChild(0)->asTransform()->getDataVariance()==osg::Object::STATIC)
        {
            // now combine with its child.
            osg::MatrixTransform* child = transform->getChild(0)->asTransform()->asMatrixTransform();
            
            osg::Matrix newMatrix = child->getMatrix()*transform->getMatrix();
            child->setMatrix(newMatrix);
            
            transformRemoved = true;

            osg::Node::ParentList parents = transform->getParents();
            for(osg::Node::ParentList::iterator pitr=parents.begin();
                pitr!=parents.end();
                ++pitr)
            {
                (*pitr)->replaceChild(transform.get(),child);
            }
            
        }
        
    }
    return transformRemoved;
}

////////////////////////////////////////////////////////////////////////////
// RemoveEmptyNodes.
////////////////////////////////////////////////////////////////////////////

void Optimizer::RemoveEmptyNodesVisitor::apply(osg::Geode& geode)
{
    for(int i=geode.getNumDrawables()-1;i>=0;--i)
    {
        osg::Geometry* geom = geode.getDrawable(i)->asGeometry();
        if (geom && geom->empty() && isOperationPermissibleForObject(geom))
        {
           geode.removeDrawable(i);
        }
    }

    if (geode.getNumParents()>0)
    {
        if (geode.getNumDrawables()==0 && isOperationPermissibleForObject(&geode)) _redundantNodeList.insert(&geode);
    }
}

void Optimizer::RemoveEmptyNodesVisitor::apply(osg::Group& group)
{
    if (group.getNumParents()>0)
    {
        // only remove empty groups, but not empty occluders.
        if (group.getNumChildren()==0 && isOperationPermissibleForObject(&group) && 
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
                    !dynamic_cast<osg::Switch*>(parent) && 
                    strcmp(parent->className(),"MultiSwitch")!=0)
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
                if (isOperationPermissibleForObject(&group))
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
    if (transform.getNumParents()>0 && 
        transform.getDataVariance()==osg::Object::STATIC &&
        isOperationPermissibleForObject(&transform))
    {
        static osg::Matrix identity;
        osg::Matrix matrix;
        transform.computeWorldToLocalMatrix(matrix,NULL);
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
            osg::notify(osg::WARN)<<"Optimizer::RemoveRedundantNodesVisitor::removeRedundantNodes() - failed dynamic_cast"<<std::endl;
        }                                
    }
    _redundantNodeList.clear();
}


////////////////////////////////////////////////////////////////////////////
// RemoveLoadedProxyNodesVisitor.
////////////////////////////////////////////////////////////////////////////

void Optimizer::RemoveLoadedProxyNodesVisitor::apply(osg::ProxyNode& proxyNode)
{
    if (proxyNode.getNumParents()>0 && proxyNode.getNumFileNames()==proxyNode.getNumChildren())
    {
        if (isOperationPermissibleForObject(&proxyNode))
        {
            _redundantNodeList.insert(&proxyNode);
        }
    }
    traverse(proxyNode);
}

void Optimizer::RemoveLoadedProxyNodesVisitor::removeRedundantNodes()
{

    for(NodeList::iterator itr=_redundantNodeList.begin();
        itr!=_redundantNodeList.end();
        ++itr)
    {
        osg::ref_ptr<osg::Group> group = dynamic_cast<osg::Group*>(*itr);
        if (group.valid())
        {
            
            // first check to see if data was attached to the ProxyNode that we need to keep.
            bool keepData = false;
            if (!group->getName().empty()) keepData = true;
            if (!group->getDescriptions().empty()) keepData = true;
            if (group->getNodeMask()) keepData = true;
            if (group->getUpdateCallback()) keepData = true;
            if (group->getEventCallback()) keepData = true;
            if (group->getCullCallback()) keepData = true;

            if (keepData)
            {
                // create a group to store all proxy's children and data.
                osg::ref_ptr<osg::Group> newGroup = new osg::Group(*group,osg::CopyOp::SHALLOW_COPY);
                

                // take a copy of parents list since subsequent removes will modify the original one.
                osg::Node::ParentList parents = group->getParents();

                for(osg::Node::ParentList::iterator pitr=parents.begin();
                    pitr!=parents.end();
                    ++pitr)
                {
                    (*pitr)->replaceChild(group.get(),newGroup.get());
                }

            }
            else
            {
                // take a copy of parents list since subsequent removes will modify the original one.
                osg::Node::ParentList parents = group->getParents();

                for(unsigned int i=0;i<group->getNumChildren();++i)
                {
                    osg::Node* child = group->getChild(i);
                    for(osg::Node::ParentList::iterator pitr=parents.begin();
                        pitr!=parents.end();
                        ++pitr)
                    {
                        (*pitr)->replaceChild(group.get(),child);
                    }

                }
            }
        }
        else
        {
            osg::notify(osg::WARN)<<"Optimizer::RemoveLoadedProxyNodesVisitor::removeRedundantNodes() - failed dynamic_cast"<<std::endl;
        }                                
    }
    _redundantNodeList.clear();
}



////////////////////////////////////////////////////////////////////////////
// combine LOD's.
////////////////////////////////////////////////////////////////////////////
void Optimizer::CombineLODsVisitor::apply(osg::LOD& lod)
{
    if (dynamic_cast<osg::PagedLOD*>(&lod)==0)
    {    
        for(unsigned int i=0;i<lod.getNumParents();++i)
        {
            if (typeid(*lod.getParent(i))==typeid(osg::Group))
            {
                if (isOperationPermissibleForObject(&lod))
                {
                    _groupList.insert(lod.getParent(i));
                }
            }
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

        typedef std::set<osg::LOD*>  LODSet;

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
                osg::LOD* newLOD = new osg::LOD;
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
        
        if (rhs->getVertexIndices()) { if (!lhs->getVertexIndices()) return true; }
        else if (lhs->getVertexIndices()) return false;
        
        if (lhs->getNormalBinding()<rhs->getNormalBinding()) return true;
        if (rhs->getNormalBinding()<lhs->getNormalBinding()) return false;

        if (rhs->getNormalIndices()) { if (!lhs->getNormalIndices()) return true; }
        else if (lhs->getNormalIndices()) return false;

        if (lhs->getColorBinding()<rhs->getColorBinding()) return true;
        if (rhs->getColorBinding()<lhs->getColorBinding()) return false;
        
        if (rhs->getColorIndices()) { if (!lhs->getColorIndices()) return true; }
        else if (lhs->getColorIndices()) return false;

        if (lhs->getSecondaryColorBinding()<rhs->getSecondaryColorBinding()) return true;
        if (rhs->getSecondaryColorBinding()<lhs->getSecondaryColorBinding()) return false;

        if (rhs->getSecondaryColorIndices()) { if (!lhs->getSecondaryColorIndices()) return true; }
        else if (lhs->getSecondaryColorIndices()) return false;

        if (lhs->getFogCoordBinding()<rhs->getFogCoordBinding()) return true;
        if (rhs->getFogCoordBinding()<lhs->getFogCoordBinding()) return false;

        if (rhs->getFogCoordIndices()) { if (!lhs->getFogCoordIndices()) return true; }
        else if (lhs->getFogCoordIndices()) return false;

        if (lhs->getNumTexCoordArrays()<rhs->getNumTexCoordArrays()) return true;
        if (rhs->getNumTexCoordArrays()<lhs->getNumTexCoordArrays()) return false;
    
        // therefore lhs->getNumTexCoordArrays()==rhs->getNumTexCoordArrays()
        
        unsigned int i;
        for(i=0;i<lhs->getNumTexCoordArrays();++i)
        {
            if (rhs->getTexCoordArray(i)) 
            {
                if (!lhs->getTexCoordArray(i)) return true;
            }
            else if (lhs->getTexCoordArray(i)) return false;

            if (rhs->getTexCoordIndices(i)) { if (!lhs->getTexCoordIndices(i)) return true; }
            else if (lhs->getTexCoordIndices(i)) return false;
        }
        
        for(i=0;i<lhs->getNumVertexAttribArrays();++i)
        {
            if (rhs->getVertexAttribArray(i)) 
            {
                if (!lhs->getVertexAttribArray(i)) return true;
            }
            else if (lhs->getVertexAttribArray(i)) return false;

            if (rhs->getVertexAttribIndices(i)) { if (!lhs->getVertexAttribIndices(i)) return true; }
            else if (lhs->getVertexAttribIndices(i)) return false;
        }
        
        
        if (lhs->getNormalBinding()==osg::Geometry::BIND_OVERALL)
        {
            // assumes that the bindings and arrays are set up correctly, this
            // should be the case after running computeCorrectBindingsAndArraySizes();
            const osg::Array* lhs_normalArray = lhs->getNormalArray();
            const osg::Array* rhs_normalArray = rhs->getNormalArray();
            if (lhs_normalArray->getType()<rhs_normalArray->getType()) return true;
            if (rhs_normalArray->getType()<lhs_normalArray->getType()) return false;
            switch(lhs_normalArray->getType())
            {
            case(osg::Array::Vec3bArrayType):
                if ((*static_cast<const osg::Vec3bArray*>(lhs_normalArray))[0]<(*static_cast<const osg::Vec3bArray*>(rhs_normalArray))[0]) return true;
                if ((*static_cast<const osg::Vec3bArray*>(rhs_normalArray))[0]<(*static_cast<const osg::Vec3bArray*>(lhs_normalArray))[0]) return false;
                break;
            case(osg::Array::Vec3sArrayType):
                if ((*static_cast<const osg::Vec3sArray*>(lhs_normalArray))[0]<(*static_cast<const osg::Vec3sArray*>(rhs_normalArray))[0]) return true;
                if ((*static_cast<const osg::Vec3sArray*>(rhs_normalArray))[0]<(*static_cast<const osg::Vec3sArray*>(lhs_normalArray))[0]) return false;
                break;
            case(osg::Array::Vec3ArrayType):
                if ((*static_cast<const osg::Vec3Array*>(lhs_normalArray))[0]<(*static_cast<const osg::Vec3Array*>(rhs_normalArray))[0]) return true;
                if ((*static_cast<const osg::Vec3Array*>(rhs_normalArray))[0]<(*static_cast<const osg::Vec3Array*>(lhs_normalArray))[0]) return false;
                break;
            default:
                break;
            }
        }
        
        if (lhs->getColorBinding()==osg::Geometry::BIND_OVERALL)
        {
            const osg::Array* lhs_colorArray = lhs->getColorArray();
            const osg::Array* rhs_colorArray = rhs->getColorArray();
            if (lhs_colorArray->getType()<rhs_colorArray->getType()) return true;
            if (rhs_colorArray->getType()<lhs_colorArray->getType()) return false;
            switch(lhs_colorArray->getType())
            {
                case(osg::Array::Vec4ubArrayType):
                    if ((*static_cast<const osg::Vec4ubArray*>(lhs_colorArray))[0]<(*static_cast<const osg::Vec4ubArray*>(rhs_colorArray))[0]) return true;
                    if ((*static_cast<const osg::Vec4ubArray*>(rhs_colorArray))[0]<(*static_cast<const osg::Vec4ubArray*>(lhs_colorArray))[0]) return false;
                    break;
                case(osg::Array::Vec3ArrayType):
                    if ((*static_cast<const osg::Vec3Array*>(lhs_colorArray))[0]<(*static_cast<const osg::Vec3Array*>(rhs_colorArray))[0]) return true;
                    if ((*static_cast<const osg::Vec3Array*>(rhs_colorArray))[0]<(*static_cast<const osg::Vec3Array*>(lhs_colorArray))[0]) return false;
                    break;
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

void Optimizer::CheckGeometryVisitor::checkGeode(osg::Geode& geode)
{
    if (isOperationPermissibleForObject(&geode))
    {
        for(unsigned int i=0;i<geode.getNumDrawables();++i)
        {
            osg::Geometry* geom = geode.getDrawable(i)->asGeometry();
            if (geom && isOperationPermissibleForObject(geom))
            {
                geom->computeCorrectBindingsAndArraySizes();
            }
        }
    }
}

bool Optimizer::MergeGeometryVisitor::mergeGeode(osg::Geode& geode)
{
    if (!isOperationPermissibleForObject(&geode)) return false;

#if 1

    if (geode.getNumDrawables()>=2)
    {
    
        typedef std::vector<osg::Geometry*>                         DuplicateList;
        typedef std::map<osg::Geometry*,DuplicateList,LessGeometry> GeometryDuplicateMap;

        GeometryDuplicateMap geometryDuplicateMap;

        unsigned int i;
        for(i=0;i<geode.getNumDrawables();++i)
        {
            osg::Geometry* geom = geode.getDrawable(i)->asGeometry();
            if (geom)
            {
                //geom->computeCorrectBindingsAndArraySizes();

                if (!geometryContainsSharedArrays(*geom))
                {
                    geometryDuplicateMap[geom].push_back(geom);
                }
            }
        }


        // don't merge geometry if its above a maximum number of vertices.
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
                    
                    if (lhs->getVertexArray() && lhs->getVertexArray()->getNumElements()>=_targetMaximumNumberOfVertices)
                    {
                        lhs = rhs;
                        continue;
                    }

                    if (rhs->getVertexArray() && rhs->getVertexArray()->getNumElements()>=_targetMaximumNumberOfVertices)
                    {
                        continue;
                    }
                    
                    if (lhs->getVertexArray() && rhs->getVertexArray() && 
                        (lhs->getVertexArray()->getNumElements()+rhs->getVertexArray()->getNumElements())>=_targetMaximumNumberOfVertices)
                    {
                        continue;
                    }
                    
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


#endif

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

bool Optimizer::MergeGeometryVisitor::geometryContainsSharedArrays(osg::Geometry& geom)
{
    if (geom.getVertexArray() && geom.getVertexArray()->referenceCount()>1) return true;
    if (geom.getNormalArray() && geom.getNormalArray()->referenceCount()>1) return true;
    if (geom.getColorArray() && geom.getColorArray()->referenceCount()>1) return true;
    if (geom.getSecondaryColorArray() && geom.getSecondaryColorArray()->referenceCount()>1) return true;
    if (geom.getFogCoordArray() && geom.getFogCoordArray()->referenceCount()>1) return true;
    

    for(unsigned int unit=0;unit<geom.getNumTexCoordArrays();++unit)
    {
        osg::Array* tex = geom.getTexCoordArray(unit);
        if (tex && tex->referenceCount()>1) return true;
    }
    
    // shift the indices of the incomming primitives to account for the pre exisiting geometry.
    for(osg::Geometry::PrimitiveSetList::iterator primItr=geom.getPrimitiveSetList().begin();
        primItr!=geom.getPrimitiveSetList().end();
        ++primItr)
    {
        if ((*primItr)->referenceCount()>1) return true;
    }
    
    
    return false;
}


class MergeArrayVisitor : public osg::ArrayVisitor
{
    public:
    
        osg::Array* _lhs;
    
        MergeArrayVisitor() :
            _lhs(0) {}
            
            
        void merge(osg::Array* lhs,osg::Array* rhs)
        {
            if (lhs==0 || rhs==0) return;
            if (lhs->getType()!=rhs->getType()) return;
            
            _lhs = lhs;
            
            rhs->accept(*this);
        }
        
        template<typename T>
        void _merge(T& rhs)
        {
            T* lhs = static_cast<T*>(_lhs); 
            lhs->insert(lhs->end(),rhs.begin(),rhs.end());
        }
        
            
        virtual void apply(osg::Array&) { osg::notify(osg::WARN) << "Warning: Optimizer's MergeArrayVisitor cannot merge Array type." << std::endl; }
        virtual void apply(osg::ByteArray& rhs) { _merge(rhs); }
        virtual void apply(osg::ShortArray& rhs) { _merge(rhs); }
        virtual void apply(osg::IntArray& rhs) { _merge(rhs); }
        virtual void apply(osg::UByteArray& rhs) { _merge(rhs); }
        virtual void apply(osg::UShortArray& rhs) { _merge(rhs); }
        virtual void apply(osg::UIntArray& rhs) { _merge(rhs); }
        virtual void apply(osg::Vec4ubArray& rhs) { _merge(rhs); }
        virtual void apply(osg::FloatArray& rhs) { _merge(rhs); }
        virtual void apply(osg::Vec2Array& rhs) { _merge(rhs); }
        virtual void apply(osg::Vec3Array& rhs) { _merge(rhs); }
        virtual void apply(osg::Vec4Array& rhs) { _merge(rhs); }
        
        virtual void apply(osg::Vec2bArray&  rhs) { _merge(rhs); }
        virtual void apply(osg::Vec3bArray&  rhs) { _merge(rhs); }
        virtual void apply(osg::Vec4bArray&  rhs) { _merge(rhs); }        
        virtual void apply(osg::Vec2sArray& rhs) { _merge(rhs); }
        virtual void apply(osg::Vec3sArray& rhs) { _merge(rhs); }
        virtual void apply(osg::Vec4sArray& rhs) { _merge(rhs); }
};

bool Optimizer::MergeGeometryVisitor::mergeGeometry(osg::Geometry& lhs,osg::Geometry& rhs)
{

    MergeArrayVisitor merger;
 
    unsigned int base = 0;
    if (lhs.getVertexArray() && rhs.getVertexArray())
    {

        base = lhs.getVertexArray()->getNumElements();

        merger.merge(lhs.getVertexArray(),rhs.getVertexArray());

    }
    else if (rhs.getVertexArray())
    {
        base = 0;
        lhs.setVertexArray(rhs.getVertexArray());
    }
    
    if (lhs.getVertexIndices() && rhs.getVertexIndices())
    {

        base = lhs.getVertexIndices()->getNumElements();
        merger.merge(lhs.getVertexIndices(),rhs.getVertexIndices());

    }
    else if (rhs.getVertexIndices())
    {
        base = 0;
        lhs.setVertexIndices(rhs.getVertexIndices());
    }
    

    if (lhs.getNormalArray() && rhs.getNormalArray() && lhs.getNormalBinding()!=osg::Geometry::BIND_OVERALL)
    {
        merger.merge(lhs.getNormalArray(),rhs.getNormalArray());
    }
    else if (rhs.getNormalArray())
    {
        lhs.setNormalArray(rhs.getNormalArray());
    }

    if (lhs.getNormalIndices() && rhs.getNormalIndices() && lhs.getNormalBinding()!=osg::Geometry::BIND_OVERALL)
    {
        merger.merge(lhs.getNormalIndices(),rhs.getNormalIndices());
    }
    else if (rhs.getNormalIndices())
    {
        lhs.setNormalIndices(rhs.getNormalIndices());
    }


    if (lhs.getColorArray() && rhs.getColorArray() && lhs.getColorBinding()!=osg::Geometry::BIND_OVERALL)
    {
        merger.merge(lhs.getColorArray(),rhs.getColorArray());
    }
    else if (rhs.getColorArray())
    {
        lhs.setColorArray(rhs.getColorArray());
    }
    
    if (lhs.getColorIndices() && rhs.getColorIndices() && lhs.getColorBinding()!=osg::Geometry::BIND_OVERALL)
    {
        merger.merge(lhs.getColorIndices(),rhs.getColorIndices());
    }
    else if (rhs.getColorIndices())
    {
        lhs.setColorIndices(rhs.getColorIndices());
    }

    if (lhs.getSecondaryColorArray() && rhs.getSecondaryColorArray() && lhs.getSecondaryColorBinding()!=osg::Geometry::BIND_OVERALL)
    {
        merger.merge(lhs.getSecondaryColorArray(),rhs.getSecondaryColorArray());
    }
    else if (rhs.getSecondaryColorArray())
    {
        lhs.setSecondaryColorArray(rhs.getSecondaryColorArray());
    }
    
    if (lhs.getSecondaryColorIndices() && rhs.getSecondaryColorIndices() && lhs.getSecondaryColorBinding()!=osg::Geometry::BIND_OVERALL)
    {
        merger.merge(lhs.getSecondaryColorIndices(),rhs.getSecondaryColorIndices());
    }
    else if (rhs.getSecondaryColorIndices())
    {
        lhs.setSecondaryColorIndices(rhs.getSecondaryColorIndices());
    }

    if (lhs.getFogCoordArray() && rhs.getFogCoordArray() && lhs.getFogCoordBinding()!=osg::Geometry::BIND_OVERALL)
    {
        merger.merge(lhs.getFogCoordArray(),rhs.getFogCoordArray());
    }
    else if (rhs.getFogCoordArray())
    {
        lhs.setFogCoordArray(rhs.getFogCoordArray());
    }

    if (lhs.getFogCoordIndices() && rhs.getFogCoordIndices() && lhs.getFogCoordBinding()!=osg::Geometry::BIND_OVERALL)
    {
        merger.merge(lhs.getFogCoordIndices(),rhs.getFogCoordIndices());
    }
    else if (rhs.getFogCoordIndices())
    {
        lhs.setFogCoordIndices(rhs.getFogCoordIndices());
    }


    unsigned int unit;
    for(unit=0;unit<lhs.getNumTexCoordArrays();++unit)
    {
        merger.merge(lhs.getTexCoordArray(unit),rhs.getTexCoordArray(unit));
        if (lhs.getTexCoordIndices(unit) && rhs.getTexCoordIndices(unit))
        {
            merger.merge(lhs.getTexCoordIndices(unit),rhs.getTexCoordIndices(unit));
        }
    }
    
    for(unit=0;unit<lhs.getNumVertexAttribArrays();++unit)
    {
        merger.merge(lhs.getVertexAttribArray(unit),rhs.getVertexAttribArray(unit));
        if (lhs.getVertexAttribIndices(unit) && rhs.getVertexAttribIndices(unit))
        {
            merger.merge(lhs.getVertexAttribIndices(unit),rhs.getVertexAttribIndices(unit));
        }
    }


    // shift the indices of the incomming primitives to account for the pre exisiting geometry.
    for(osg::Geometry::PrimitiveSetList::iterator primItr=rhs.getPrimitiveSetList().begin();
        primItr!=rhs.getPrimitiveSetList().end();
        ++primItr)
    {
        osg::PrimitiveSet* primitive = primItr->get();
        switch(primitive->getType())
        {
        case(osg::PrimitiveSet::DrawElementsUBytePrimitiveType):
            {
                osg::DrawElementsUByte* primitiveUByte = static_cast<osg::DrawElementsUByte*>(primitive);
                unsigned int currentMaximum = 0;
                for(osg::DrawElementsUByte::iterator eitr=primitiveUByte->begin();
                    eitr!=primitiveUByte->end();
                    ++eitr)
                {
                    currentMaximum = osg::maximum(currentMaximum,(unsigned int)*eitr);
                }
                if ((base+currentMaximum)>=65536)
                {
                    // must promote to a DrawElementsUInt
                    osg::DrawElementsUInt* new_primitive = new osg::DrawElementsUInt(primitive->getMode());
                    std::copy(primitiveUByte->begin(),primitiveUByte->end(),std::back_inserter(*new_primitive));
                    new_primitive->offsetIndices(base);
                    (*primItr) = new_primitive;
                } else if ((base+currentMaximum)>=256)
                {
                    // must promote to a DrawElementsUShort
                    osg::DrawElementsUShort* new_primitive = new osg::DrawElementsUShort(primitive->getMode());
                    std::copy(primitiveUByte->begin(),primitiveUByte->end(),std::back_inserter(*new_primitive));
                    new_primitive->offsetIndices(base);
                    (*primItr) = new_primitive;
                }
                else
                {
                    primitive->offsetIndices(base);
                }
            }
            break;

        case(osg::PrimitiveSet::DrawElementsUShortPrimitiveType):
            {
                osg::DrawElementsUShort* primitiveUShort = static_cast<osg::DrawElementsUShort*>(primitive);
                unsigned int currentMaximum = 0;
                for(osg::DrawElementsUShort::iterator eitr=primitiveUShort->begin();
                    eitr!=primitiveUShort->end();
                    ++eitr)
                {
                    currentMaximum = osg::maximum(currentMaximum,(unsigned int)*eitr);
                }
                if ((base+currentMaximum)>=65536)
                {
                    // must promote to a DrawElementsUInt
                    osg::DrawElementsUInt* new_primitive = new osg::DrawElementsUInt(primitive->getMode());
                    std::copy(primitiveUShort->begin(),primitiveUShort->end(),std::back_inserter(*new_primitive));
                    new_primitive->offsetIndices(base);
                    (*primItr) = new_primitive;
                }
                else
                {
                    primitive->offsetIndices(base);
                }
            }
            break;

        case(osg::PrimitiveSet::DrawArraysPrimitiveType):
        case(osg::PrimitiveSet::DrawArrayLengthsPrimitiveType):
        case(osg::PrimitiveSet::DrawElementsUIntPrimitiveType):
        default:
            primitive->offsetIndices(base);
            break;
        }

        
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


////////////////////////////////////////////////////////////////////////////////////////////
//
//  Spatialize the scene to accelerate culling
//

void Optimizer::SpatializeGroupsVisitor::apply(osg::Group& group)
{
    if (typeid(group)==typeid(osg::Group) || group.asTransform())
    {
        if (isOperationPermissibleForObject(&group))
        {
            _groupsToDivideList.insert(&group);
        }
    }
    traverse(group);
}

bool Optimizer::SpatializeGroupsVisitor::divide(unsigned int maxNumTreesPerCell)
{
    bool divided = false;
    for(GroupsToDivideList::iterator itr=_groupsToDivideList.begin();
        itr!=_groupsToDivideList.end();
        ++itr)
    {
        if (divide(*itr,maxNumTreesPerCell)) divided = true;
    }
    return divided;
}

bool Optimizer::SpatializeGroupsVisitor::divide(osg::Group* group, unsigned int maxNumTreesPerCell)
{
    if (group->getNumChildren()<=maxNumTreesPerCell) return false;
    
    // create the original box.
    osg::BoundingBox bb;
    unsigned int i;
    for(i=0;i<group->getNumChildren();++i)
    {
        bb.expandBy(group->getChild(i)->getBound().center());
    }
    
    float radius = bb.radius();
    float divide_distance = radius*0.7f;
    bool xAxis = (bb.xMax()-bb.xMin())>divide_distance;
    bool yAxis = (bb.yMax()-bb.yMin())>divide_distance;
    bool zAxis = (bb.zMax()-bb.zMin())>divide_distance;

    osg::notify(osg::INFO)<<"Dividing "<<group->className()<<"  num children = "<<group->getNumChildren()<<"  xAxis="<<xAxis<<"  yAxis="<<yAxis<<"   zAxis="<<zAxis<<std::endl;
    
    if (!xAxis && !yAxis && !zAxis)
    {
        osg::notify(osg::INFO)<<"  No axis to divide, stopping division."<<std::endl;
        return false;
    }
    
    unsigned int numChildrenOnEntry = group->getNumChildren();
    
    typedef std::pair< osg::BoundingBox, osg::ref_ptr<osg::Group> > BoxGroupPair;
    typedef std::vector< BoxGroupPair > Boxes;
    Boxes boxes;
    boxes.push_back( BoxGroupPair(bb,new osg::Group) );

    // divide up on each axis    
    if (xAxis)
    {
        unsigned int numCellsToDivide=boxes.size();
        for(unsigned int i=0;i<numCellsToDivide;++i)
        {
            osg::BoundingBox& orig_cell = boxes[i].first;
            osg::BoundingBox new_cell = orig_cell;

            float xCenter = (orig_cell.xMin()+orig_cell.xMax())*0.5f;
            orig_cell.xMax() = xCenter;
            new_cell.xMin() = xCenter;

            boxes.push_back(BoxGroupPair(new_cell,new osg::Group));
        }
    }

    if (yAxis)
    {
        unsigned int numCellsToDivide=boxes.size();
        for(unsigned int i=0;i<numCellsToDivide;++i)
        {
            osg::BoundingBox& orig_cell = boxes[i].first;
            osg::BoundingBox new_cell = orig_cell;

            float yCenter = (orig_cell.yMin()+orig_cell.yMax())*0.5f;
            orig_cell.yMax() = yCenter;
            new_cell.yMin() = yCenter;

            boxes.push_back(BoxGroupPair(new_cell,new osg::Group));
        }
    }

    if (zAxis)
    {
        unsigned int numCellsToDivide=boxes.size();
        for(unsigned int i=0;i<numCellsToDivide;++i)
        {
            osg::BoundingBox& orig_cell = boxes[i].first;
            osg::BoundingBox new_cell = orig_cell;

            float zCenter = (orig_cell.zMin()+orig_cell.zMax())*0.5f;
            orig_cell.zMax() = zCenter;
            new_cell.zMin() = zCenter;

            boxes.push_back(BoxGroupPair(new_cell,new osg::Group));
        }
    }


    // create the groups to drop the children into
    

    // bin each child into associated bb group
    typedef std::vector< osg::ref_ptr<osg::Node> > NodeList;
    NodeList unassignedList;
    for(i=0;i<group->getNumChildren();++i)
    {
        bool assigned = false;
        osg::Vec3 center = group->getChild(i)->getBound().center();
        for(Boxes::iterator itr=boxes.begin();
            itr!=boxes.end() && !assigned;
            ++itr)
        {
            if (itr->first.contains(center))
            {
                // move child from main group into bb group.
                (itr->second)->addChild(group->getChild(i));
                assigned = true;
            }
        }
        if (!assigned)
        {
            unassignedList.push_back(group->getChild(i));
        }
    }


    // now transfer nodes across, by :
    //      first removing from the original group,
    //      add in the bb groups
    //      add then the unassigned children.
    
    
    // first removing from the original group,
    group->removeChild(0,group->getNumChildren());
    
    // add in the bb groups
    typedef std::vector< osg::ref_ptr<osg::Group> > GroupList;
    GroupList groupsToDivideList;
    for(Boxes::iterator itr=boxes.begin();
        itr!=boxes.end();
        ++itr)
    {
        // move child from main group into bb group.
        osg::Group* bb_group = (itr->second).get();
        if (bb_group->getNumChildren()>0)
        {
            if (bb_group->getNumChildren()==1)
            {
                group->addChild(bb_group->getChild(0));
            }
            else
            {
                group->addChild(bb_group);
                if (bb_group->getNumChildren()>maxNumTreesPerCell)
                {
                    groupsToDivideList.push_back(bb_group);
                }
            }
        }
    }
    
    
    // add then the unassigned children.
    for(NodeList::iterator nitr=unassignedList.begin();
        nitr!=unassignedList.end();
        ++nitr)
    {
        group->addChild(nitr->get());
    }

    // now call divide on all groups that require it.
    for(GroupList::iterator gitr=groupsToDivideList.begin();
        gitr!=groupsToDivideList.end();
        ++gitr)
    {
        divide(gitr->get(),maxNumTreesPerCell);
    }

    return (numChildrenOnEntry<group->getNumChildren());
    
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//  Spatialize the scene to accelerate culling
//

void Optimizer::CopySharedSubgraphsVisitor::apply(osg::Node& node)
{
    if (node.getNumParents()>1 && !isOperationPermissibleForObject(&node))
    {
        _sharedNodeList.insert(&node);
    }
    traverse(node);
}

void Optimizer::CopySharedSubgraphsVisitor::copySharedNodes()
{
    osg::notify(osg::INFO)<<"Shared node "<<_sharedNodeList.size()<<std::endl;
    for(SharedNodeList::iterator itr=_sharedNodeList.begin();
        itr!=_sharedNodeList.end();
        ++itr)
    {
        osg::notify(osg::INFO)<<"   No parents "<<(*itr)->getNumParents()<<std::endl;
        osg::Node* node = *itr;
        for(unsigned int i=node->getNumParents()-1;i>0;--i)
        {
            // create a clone.
            osg::ref_ptr<osg::Object> new_object = node->clone(osg::CopyOp::DEEP_COPY_NODES |
                                                          osg::CopyOp::DEEP_COPY_DRAWABLES);
            // cast it to node.                                              
            osg::Node* new_node = dynamic_cast<osg::Node*>(new_object.get());

            // replace the node by new_new 
            if (new_node) node->getParent(i)->replaceChild(node,new_node);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//  Set the attributes of textures up.
//


void Optimizer::TextureVisitor::apply(osg::Node& node)
{
    
    osg::StateSet* ss = node.getStateSet();
    if (ss && 
        isOperationPermissibleForObject(&node) &&
        isOperationPermissibleForObject(ss))
    {
        apply(*ss);
    }

    traverse(node);
}

void Optimizer::TextureVisitor::apply(osg::Geode& geode)
{
    if (!isOperationPermissibleForObject(&geode)) return;

    osg::StateSet* ss = geode.getStateSet();
    
    if (ss && isOperationPermissibleForObject(ss))
    {
        apply(*ss);
    }

    for(unsigned int i=0;i<geode.getNumDrawables();++i)
    {
        osg::Drawable* drawable = geode.getDrawable(i);
        if (drawable)
        {
            ss = drawable->getStateSet();
            if (ss &&
               isOperationPermissibleForObject(drawable) &&
               isOperationPermissibleForObject(ss))
            {
                apply(*ss);
            }
        }
    }
}

void Optimizer::TextureVisitor::apply(osg::StateSet& stateset)
{
    for(unsigned int i=0;i<stateset.getTextureAttributeList().size();++i)
    {
        osg::StateAttribute* sa = stateset.getTextureAttribute(i,osg::StateAttribute::TEXTURE);
        osg::Texture* texture = dynamic_cast<osg::Texture*>(sa);
        if (texture && isOperationPermissibleForObject(texture))
        {
            apply(*texture);
        }
    }
}

void Optimizer::TextureVisitor::apply(osg::Texture& texture)
{
    if (_changeAutoUnRef)
    {
        texture.setUnRefImageDataAfterApply(_valueAutoUnRef);
    }
    
    if (_changeClientImageStorage)
    {
        texture.setClientStorageHint(_valueClientImageStorage);
    }
    
    if (_changeAnisotropy)
    {
        texture.setMaxAnisotropy(_valueAnisotropy);
    }

}

////////////////////////////////////////////////////////////////////////////
// Merge geodes
////////////////////////////////////////////////////////////////////////////

void Optimizer::MergeGeodesVisitor::apply(osg::Group& group)
{
    mergeGeodes(group);
    traverse(group);
}

struct LessGeode
{
    bool operator() (const osg::Geode* lhs,const osg::Geode* rhs) const
    {
        if (lhs->getStateSet()<rhs->getStateSet()) return true;
        return false;
    }
};

bool Optimizer::MergeGeodesVisitor::mergeGeodes(osg::Group& group)
{
    if (!isOperationPermissibleForObject(&group)) return false;

    typedef std::vector<osg::Geode*>                      DuplicateList;
    typedef std::map<osg::Geode*,DuplicateList,LessGeode> GeodeDuplicateMap;

    GeodeDuplicateMap geodeDuplicateMap;

    for (unsigned int i=0; i<group.getNumChildren(); ++i)
    {
        osg::Node* child = group.getChild(i);
        if (typeid(*child)==typeid(osg::Geode)) 
        {
            osg::Geode* geode = (osg::Geode*)child;
            geodeDuplicateMap[geode].push_back(geode);
        }
    }

    // merge
    for(GeodeDuplicateMap::iterator itr=geodeDuplicateMap.begin();
        itr!=geodeDuplicateMap.end();
        ++itr)
    {
        if (itr->second.size()>1)
        {
            osg::Geode* lhs = itr->second[0];
            for(DuplicateList::iterator dupItr=itr->second.begin()+1;
                dupItr!=itr->second.end();
                ++dupItr)
            {
                osg::Geode* rhs = *dupItr;
                
                if (mergeGeode(*lhs,*rhs))
                {
                    group.removeChild(rhs);

                    static int co = 0;
                    osg::notify(osg::INFO)<<"merged and removed Geode "<<++co<<std::endl;
                }
            }
        }
    }

    return true;
}

bool Optimizer::MergeGeodesVisitor::mergeGeode(osg::Geode& lhs, osg::Geode& rhs)
{
    for (unsigned int i=0; i<rhs.getNumDrawables(); ++i)
    {
        lhs.addDrawable(rhs.getDrawable(i));
    }

    return true;
}

