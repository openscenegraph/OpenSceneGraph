/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
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
#include <stdlib.h>
#include <string.h>

#include <osgUtil/Optimizer>

#include <osg/ApplicationUsage>
#include <osg/Transform>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/LOD>
#include <osg/Billboard>
#include <osg/CameraView>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/OccluderNode>
#include <osg/Sequence>
#include <osg/Switch>
#include <osg/Texture>
#include <osg/PagedLOD>
#include <osg/ProxyNode>
#include <osg/ImageStream>
#include <osg/Timer>
#include <osg/TexMat>
#include <osg/io_utils>

#include <osgUtil/TransformAttributeFunctor>
#include <osgUtil/TriStripVisitor>
#include <osgUtil/Tessellator>
#include <osgUtil/Statistics>
#include <osgUtil/MeshOptimizers>

#include <typeinfo>
#include <algorithm>
#include <numeric>
#include <sstream>

#include <iterator>

using namespace osgUtil;

// #define GEOMETRYDEPRECATED

void Optimizer::reset()
{
}

static osg::ApplicationUsageProxy Optimizer_e0(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_OPTIMIZER \"<type> [<type>]\"","OFF | DEFAULT | FLATTEN_STATIC_TRANSFORMS | FLATTEN_STATIC_TRANSFORMS_DUPLICATING_SHARED_SUBGRAPHS | REMOVE_REDUNDANT_NODES | COMBINE_ADJACENT_LODS | SHARE_DUPLICATE_STATE | MERGE_GEOMETRY | MERGE_GEODES | SPATIALIZE_GROUPS  | COPY_SHARED_NODES  | TRISTRIP_GEOMETRY | OPTIMIZE_TEXTURE_SETTINGS | REMOVE_LOADED_PROXY_NODES | TESSELLATE_GEOMETRY | CHECK_GEOMETRY |  FLATTEN_BILLBOARDS | TEXTURE_ATLAS_BUILDER | STATIC_OBJECT_DETECTION | INDEX_MESH | VERTEX_POSTTRANSFORM | VERTEX_PRETRANSFORM");

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

        if(str.find("~FLATTEN_STATIC_TRANSFORMS_DUPLICATING_SHARED_SUBGRAPHS")!=std::string::npos) options ^= FLATTEN_STATIC_TRANSFORMS_DUPLICATING_SHARED_SUBGRAPHS;
        else if(str.find("FLATTEN_STATIC_TRANSFORMS_DUPLICATING_SHARED_SUBGRAPHS")!=std::string::npos) options |= FLATTEN_STATIC_TRANSFORMS_DUPLICATING_SHARED_SUBGRAPHS;

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

        if(str.find("~TESSELLATE_GEOMETRY")!=std::string::npos) options ^= TESSELLATE_GEOMETRY;
        else if(str.find("TESSELLATE_GEOMETRY")!=std::string::npos) options |= TESSELLATE_GEOMETRY;

        if(str.find("~TRISTRIP_GEOMETRY")!=std::string::npos) options ^= TRISTRIP_GEOMETRY;
        else if(str.find("TRISTRIP_GEOMETRY")!=std::string::npos) options |= TRISTRIP_GEOMETRY;

        if(str.find("~OPTIMIZE_TEXTURE_SETTINGS")!=std::string::npos) options ^= OPTIMIZE_TEXTURE_SETTINGS;
        else if(str.find("OPTIMIZE_TEXTURE_SETTINGS")!=std::string::npos) options |= OPTIMIZE_TEXTURE_SETTINGS;

        if(str.find("~CHECK_GEOMETRY")!=std::string::npos) options ^= CHECK_GEOMETRY;
        else if(str.find("CHECK_GEOMETRY")!=std::string::npos) options |= CHECK_GEOMETRY;

        if(str.find("~MAKE_FAST_GEOMETRY")!=std::string::npos) options ^= MAKE_FAST_GEOMETRY;
        else if(str.find("MAKE_FAST_GEOMETRY")!=std::string::npos) options |= MAKE_FAST_GEOMETRY;

        if(str.find("~FLATTEN_BILLBOARDS")!=std::string::npos) options ^= FLATTEN_BILLBOARDS;
        else if(str.find("FLATTEN_BILLBOARDS")!=std::string::npos) options |= FLATTEN_BILLBOARDS;

        if(str.find("~TEXTURE_ATLAS_BUILDER")!=std::string::npos) options ^= TEXTURE_ATLAS_BUILDER;
        else if(str.find("TEXTURE_ATLAS_BUILDER")!=std::string::npos) options |= TEXTURE_ATLAS_BUILDER;

        if(str.find("~STATIC_OBJECT_DETECTION")!=std::string::npos) options ^= STATIC_OBJECT_DETECTION;
        else if(str.find("STATIC_OBJECT_DETECTION")!=std::string::npos) options |= STATIC_OBJECT_DETECTION;

        if(str.find("~INDEX_MESH")!=std::string::npos) options ^= INDEX_MESH;
        else if(str.find("INDEX_MESH")!=std::string::npos) options |= INDEX_MESH;

        if(str.find("~VERTEX_POSTTRANSFORM")!=std::string::npos) options ^= VERTEX_POSTTRANSFORM;
        else if(str.find("VERTEX_POSTTRANSFORM")!=std::string::npos) options |= VERTEX_POSTTRANSFORM;

        if(str.find("~VERTEX_PRETRANSFORM")!=std::string::npos) options ^= VERTEX_PRETRANSFORM;
        else if(str.find("VERTEX_PRETRANSFORM")!=std::string::npos) options |= VERTEX_PRETRANSFORM;

    }
    else
    {
        options = DEFAULT_OPTIMIZATIONS;
    }

    optimize(node,options);

}

void Optimizer::optimize(osg::Node* node, unsigned int options)
{
    StatsVisitor stats;

    if (osg::getNotifyLevel()>=osg::INFO)
    {
        node->accept(stats);
        stats.totalUpStats();
        OSG_NOTICE<<std::endl<<"Stats before:"<<std::endl;
        stats.print(osg::notify(osg::NOTICE));
    }

    if (options & STATIC_OBJECT_DETECTION)
    {
        StaticObjectDetectionVisitor sodv;
        node->accept(sodv);
    }

    if (options & TESSELLATE_GEOMETRY)
    {
        OSG_INFO<<"Optimizer::optimize() doing TESSELLATE_GEOMETRY"<<std::endl;

        TessellateVisitor tsv;
        node->accept(tsv);
    }

    if (options & REMOVE_LOADED_PROXY_NODES)
    {
        OSG_INFO<<"Optimizer::optimize() doing REMOVE_LOADED_PROXY_NODES"<<std::endl;

        RemoveLoadedProxyNodesVisitor rlpnv(this);
        node->accept(rlpnv);
        rlpnv.removeRedundantNodes();

    }

    if (options & COMBINE_ADJACENT_LODS)
    {
        OSG_INFO<<"Optimizer::optimize() doing COMBINE_ADJACENT_LODS"<<std::endl;

        CombineLODsVisitor clv(this);
        node->accept(clv);
        clv.combineLODs();
    }

    if (options & OPTIMIZE_TEXTURE_SETTINGS)
    {
        OSG_INFO<<"Optimizer::optimize() doing OPTIMIZE_TEXTURE_SETTINGS"<<std::endl;

        TextureVisitor tv(true,true, // unref image
                          false,false, // client storage
                          false,1.0, // anisotropic filtering
                          this );
        node->accept(tv);
    }

    if (options & SHARE_DUPLICATE_STATE)
    {
        OSG_INFO<<"Optimizer::optimize() doing SHARE_DUPLICATE_STATE"<<std::endl;

        bool combineDynamicState = false;
        bool combineStaticState = true;
        bool combineUnspecifiedState = true;

        StateVisitor osv(combineDynamicState, combineStaticState, combineUnspecifiedState, this);
        node->accept(osv);
        osv.optimize();
    }

    if (options & TEXTURE_ATLAS_BUILDER)
    {
        OSG_INFO<<"Optimizer::optimize() doing TEXTURE_ATLAS_BUILDER"<<std::endl;

        // traverse the scene collecting textures into texture atlas.
        TextureAtlasVisitor tav(this);
        node->accept(tav);
        tav.optimize();

        // now merge duplicate state, that may have been introduced by merge textures into texture atlas'
        bool combineDynamicState = false;
        bool combineStaticState = true;
        bool combineUnspecifiedState = true;

        StateVisitor osv(combineDynamicState, combineStaticState, combineUnspecifiedState, this);
        node->accept(osv);
        osv.optimize();
    }

    if (options & COPY_SHARED_NODES)
    {
        OSG_INFO<<"Optimizer::optimize() doing COPY_SHARED_NODES"<<std::endl;

        CopySharedSubgraphsVisitor cssv(this);
        node->accept(cssv);
        cssv.copySharedNodes();
    }

    if (options & FLATTEN_STATIC_TRANSFORMS)
    {
        OSG_INFO<<"Optimizer::optimize() doing FLATTEN_STATIC_TRANSFORMS"<<std::endl;

        int i=0;
        bool result = false;
        do
        {
            OSG_DEBUG << "** RemoveStaticTransformsVisitor *** Pass "<<i<<std::endl;
            FlattenStaticTransformsVisitor fstv(this);
            node->accept(fstv);
            result = fstv.removeTransforms(node);
            ++i;
        } while (result);

        // now combine any adjacent static transforms.
        CombineStaticTransformsVisitor cstv(this);
        node->accept(cstv);
        cstv.removeTransforms(node);
    }

    if (options & FLATTEN_STATIC_TRANSFORMS_DUPLICATING_SHARED_SUBGRAPHS)
    {
        OSG_INFO<<"Optimizer::optimize() doing FLATTEN_STATIC_TRANSFORMS_DUPLICATING_SHARED_SUBGRAPHS"<<std::endl;

        // now combine any adjacent static transforms.
        FlattenStaticTransformsDuplicatingSharedSubgraphsVisitor fstdssv(this);
        node->accept(fstdssv);

    }

    if (options & MERGE_GEODES)
    {
        OSG_INFO<<"Optimizer::optimize() doing MERGE_GEODES"<<std::endl;

        osg::Timer_t startTick = osg::Timer::instance()->tick();

        MergeGeodesVisitor visitor;
        node->accept(visitor);

        osg::Timer_t endTick = osg::Timer::instance()->tick();

        OSG_INFO<<"MERGE_GEODES took "<<osg::Timer::instance()->delta_s(startTick,endTick)<<std::endl;
    }

    if (options & CHECK_GEOMETRY)
    {
        OSG_INFO<<"Optimizer::optimize() doing CHECK_GEOMETRY"<<std::endl;

        CheckGeometryVisitor mgv(this);
        node->accept(mgv);
    }

    if (options & MAKE_FAST_GEOMETRY)
    {
        OSG_INFO<<"Optimizer::optimize() doing MAKE_FAST_GEOMETRY"<<std::endl;

        MakeFastGeometryVisitor mgv(this);
        node->accept(mgv);
    }

    if (options & MERGE_GEOMETRY)
    {
        OSG_INFO<<"Optimizer::optimize() doing MERGE_GEOMETRY"<<std::endl;

        osg::Timer_t startTick = osg::Timer::instance()->tick();

        MergeGeometryVisitor mgv(this);
        mgv.setTargetMaximumNumberOfVertices(10000);
        node->accept(mgv);

        osg::Timer_t endTick = osg::Timer::instance()->tick();

        OSG_INFO<<"MERGE_GEOMETRY took "<<osg::Timer::instance()->delta_s(startTick,endTick)<<std::endl;
    }

    if (options & TRISTRIP_GEOMETRY)
    {
        OSG_INFO<<"Optimizer::optimize() doing TRISTRIP_GEOMETRY"<<std::endl;

        TriStripVisitor tsv(this);
        node->accept(tsv);
        tsv.stripify();
    }

    if (options & REMOVE_REDUNDANT_NODES)
    {
        OSG_INFO<<"Optimizer::optimize() doing REMOVE_REDUNDANT_NODES"<<std::endl;

        RemoveEmptyNodesVisitor renv(this);
        node->accept(renv);
        renv.removeEmptyNodes();

        RemoveRedundantNodesVisitor rrnv(this);
        node->accept(rrnv);
        rrnv.removeRedundantNodes();

    }

    if (options & FLATTEN_BILLBOARDS)
    {
        FlattenBillboardVisitor fbv(this);
        node->accept(fbv);
        fbv.process();
    }

    if (options & SPATIALIZE_GROUPS)
    {
        OSG_INFO<<"Optimizer::optimize() doing SPATIALIZE_GROUPS"<<std::endl;

        SpatializeGroupsVisitor sv(this);
        node->accept(sv);
        sv.divide();
    }

    if (options & INDEX_MESH)
    {
        OSG_INFO<<"Optimizer::optimize() doing INDEX_MESH"<<std::endl;
        IndexMeshVisitor imv(this);
        node->accept(imv);
        imv.makeMesh();
    }

    if (options & VERTEX_POSTTRANSFORM)
    {
        OSG_INFO<<"Optimizer::optimize() doing VERTEX_POSTTRANSFORM"<<std::endl;
        VertexCacheVisitor vcv;
        node->accept(vcv);
        vcv.optimizeVertices();
    }

    if (options & VERTEX_PRETRANSFORM)
    {
        OSG_INFO<<"Optimizer::optimize() doing VERTEX_PRETRANSFORM"<<std::endl;
        VertexAccessOrderVisitor vaov;
        node->accept(vaov);
        vaov.optimizeOrder();
    }

    if (osg::getNotifyLevel()>=osg::INFO)
    {
        stats.reset();
        node->accept(stats);
        stats.totalUpStats();
        OSG_NOTICE<<std::endl<<"Stats after:"<<std::endl;
        stats.print(osg::notify(osg::NOTICE));
    }
}


////////////////////////////////////////////////////////////////////////////
// Tessellate geometry - eg break complex POLYGONS into triangles, strips, fans..
////////////////////////////////////////////////////////////////////////////
void Optimizer::TessellateVisitor::apply(osg::Geode& geode)
{
    for(unsigned int i=0;i<geode.getNumDrawables();++i)
    {
        osg::Geometry* geom = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
        if (geom) {
            osgUtil::Tessellator Tessellator;
            Tessellator.retessellatePolygons(*geom);
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
    OSG_INFO << "Num of StateSet="<<_statesets.size()<< std::endl;

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
            const osg::StateSet::AttributeList& attributes = sitr->first->getAttributeList();
            for(osg::StateSet::AttributeList::const_iterator aitr= attributes.begin();
                aitr!=attributes.end();
                ++aitr)
            {
                if (optimize(aitr->second.first->getDataVariance()))
                {
                    attributeToStateSetMap[aitr->second.first.get()].insert(StateSetUnitPair(sitr->first,NON_TEXTURE_ATTRIBUTE));
                }
            }


            const osg::StateSet::TextureAttributeList& texAttributes = sitr->first->getTextureAttributeList();
            for(unsigned int unit=0;unit<texAttributes.size();++unit)
            {
                const osg::StateSet::AttributeList& attributes = texAttributes[unit];
                for(osg::StateSet::AttributeList::const_iterator aitr= attributes.begin();
                    aitr!=attributes.end();
                    ++aitr)
                {
                    if (optimize(aitr->second.first->getDataVariance()))
                    {
                        attributeToStateSetMap[aitr->second.first.get()].insert(StateSetUnitPair(sitr->first,unit));
                    }
                }
            }


            const osg::StateSet::UniformList& uniforms = sitr->first->getUniformList();
            for(osg::StateSet::UniformList::const_iterator uitr= uniforms.begin();
                uitr!=uniforms.end();
                ++uitr)
            {
                if (optimize(uitr->second.first->getDataVariance()))
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

            OSG_INFO << "state attribute list"<< std::endl;
            for(AttributeList::iterator aaitr = attributeList.begin();
                aaitr!=attributeList.end();
                ++aaitr)
            {
                OSG_INFO << "    "<<*aaitr << "  "<<(*aaitr)->className()<< std::endl;
            }

            OSG_INFO << "searching for duplicate attributes"<< std::endl;
            // find the duplicates.
            AttributeList::iterator first_unique = attributeList.begin();
            AttributeList::iterator current = first_unique;
            ++current;
            for(; current!=attributeList.end();++current)
            {
                if (**current==**first_unique)
                {
                    OSG_INFO << "    found duplicate "<<(*current)->className()<<"  first="<<*first_unique<<"  current="<<*current<< std::endl;
                    StateSetList& statesetlist = attributeToStateSetMap[*current];
                    for(StateSetList::iterator sitr=statesetlist.begin();
                        sitr!=statesetlist.end();
                        ++sitr)
                    {
                        OSG_INFO << "       replace duplicate "<<*current<<" with "<<*first_unique<< std::endl;
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

            OSG_INFO << "state uniform list"<< std::endl;
            for(UniformList::iterator uuitr = uniformList.begin();
                uuitr!=uniformList.end();
                ++uuitr)
            {
                OSG_INFO << "    "<<*uuitr << "  "<<(*uuitr)->getName()<< std::endl;
            }

            OSG_INFO << "searching for duplicate uniforms"<< std::endl;
            // find the duplicates.
            UniformList::iterator first_unique_uniform = uniformList.begin();
            UniformList::iterator current_uniform = first_unique_uniform;
            ++current_uniform;
            for(; current_uniform!=uniformList.end();++current_uniform)
            {
                if ((**current_uniform)==(**first_unique_uniform))
                {
                    OSG_INFO << "    found duplicate uniform "<<(*current_uniform)->getName()<<"  first_unique_uniform="<<*first_unique_uniform<<"  current_uniform="<<*current_uniform<< std::endl;
                    StateSetSet& statesetset = uniformToStateSetMap[*current_uniform];
                    for(StateSetSet::iterator sitr=statesetset.begin();
                        sitr!=statesetset.end();
                        ++sitr)
                    {
                        OSG_INFO << "       replace duplicate "<<*current_uniform<<" with "<<*first_unique_uniform<< std::endl;
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

        OSG_INFO << "searching for duplicate attributes"<< std::endl;
        // find the duplicates.
        StateSetSortList::iterator first_unique = statesetSortList.begin();
        StateSetSortList::iterator current = first_unique; ++current;
        for(; current!=statesetSortList.end();++current)
        {
            if (**current==**first_unique)
            {
                OSG_INFO << "    found duplicate "<<(*current)->className()<<"  first="<<*first_unique<<"  current="<<*current<< std::endl;
                ObjectSet& objSet = _statesets[*current];
                for(ObjectSet::iterator sitr=objSet.begin();
                    sitr!=objSet.end();
                    ++sitr)
                {
                    OSG_INFO << "       replace duplicate "<<*current<<" with "<<*first_unique<< std::endl;
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
            if (dynamic_cast<const osg::PagedLOD*>(node)) return false;
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
                    if (transform->getDataVariance()!=osg::Transform::STATIC) _moreThanOneMatrixRequired=true;
                    else if (transform->getReferenceFrame()!=osg::Transform::RELATIVE_RF) _moreThanOneMatrixRequired=true;
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
                        if (!_firstMatrix.isIdentity()) _moreThanOneMatrixRequired=true;
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
                group->setStateSet(transform->getStateSet());
                group->setUserData(transform->getUserData());
                group->setDescriptions(transform->getDescriptions());
                for(unsigned int i=0;i<transform->getNumChildren();++i)
                {
                    group->addChild(transform->getChild(i));
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
                        OSG_WARN<<"Warning:: during Optimize::CollectLowestTransformsVisitor::removeTransforms(Node*)"<<std::endl;
                        OSG_WARN<<"          unhandled of setting of indentity matrix on "<< titr->first->className()<<std::endl;
                        OSG_WARN<<"          model will appear in the incorrect position."<<std::endl;
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

void Optimizer::FlattenStaticTransformsVisitor::apply(osg::PagedLOD& node)
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
            osg::Geometry *geometry = geode.getDrawable(i)->asGeometry();
            if((geometry) && (isOperationPermissibleForObject(&geode)) && (isOperationPermissibleForObject(geometry)))
            {
                if(geometry->getVertexArray() && geometry->getVertexArray()->referenceCount() > 1) {
                    geometry->setVertexArray(dynamic_cast<osg::Array*>(geometry->getVertexArray()->clone(osg::CopyOp::DEEP_COPY_ALL)));
                }
                if(geometry->getNormalArray() && geometry->getNormalArray()->referenceCount() > 1) {
                    geometry->setNormalArray(dynamic_cast<osg::Array*>(geometry->getNormalArray()->clone(osg::CopyOp::DEEP_COPY_ALL)));
                }
            }
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
// CombineStaticTransforms
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
        // remove topmost node from transform set if it exists there.
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
            if (transform->getStateSet())
            {
                if(child->getStateSet()) child->getStateSet()->merge(*transform->getStateSet());
                else child->setStateSet(transform->getStateSet());
            }

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
           geode.removeDrawables(i,1);
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
            (typeid(group)==typeid(osg::Group) || (dynamic_cast<osg::Transform*>(&group) && !dynamic_cast<osg::CameraView*>(&group))) &&
            (group.getNumChildrenRequiringUpdateTraversal()==0 && group.getNumChildrenRequiringEventTraversal()==0) )
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

bool Optimizer::RemoveRedundantNodesVisitor::isOperationPermissible(osg::Node& node)
{
    return node.getNumParents()>0 &&
           !node.getStateSet() &&
           node.getName().empty() &&
           !node.getUserDataContainer() &&
           !node.getCullCallback() &&
           !node.getEventCallback() &&
           !node.getUpdateCallback() &&
           isOperationPermissibleForObject(&node);
}

void Optimizer::RemoveRedundantNodesVisitor::apply(osg::Group& group)
{
    if (group.getNumChildren()==1 &&
        typeid(group)==typeid(osg::Group) &&
        isOperationPermissible(group))
    {
        _redundantNodeList.insert(&group);
    }

    traverse(group);
}



void Optimizer::RemoveRedundantNodesVisitor::apply(osg::Transform& transform)
{
    if (transform.getDataVariance()==osg::Object::STATIC &&
        isOperationPermissible(transform))
    {
        osg::Matrix matrix;
        transform.computeWorldToLocalMatrix(matrix,NULL);
        if (matrix.isIdentity())
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
            OSG_WARN<<"Optimizer::RemoveRedundantNodesVisitor::removeRedundantNodes() - failed dynamic_cast"<<std::endl;
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

                for(osg::Node::ParentList::iterator pitr=parents.begin();
                    pitr!=parents.end();
                    ++pitr)
                {
                    (*pitr)->removeChild(group.get());
                    for(unsigned int i=0;i<group->getNumChildren();++i)
                    {
                        osg::Node* child = group->getChild(i);
                        (*pitr)->addChild(child);
                    }

                }
            }
        }
        else
        {
            OSG_WARN<<"Optimizer::RemoveLoadedProxyNodesVisitor::removeRedundantNodes() - failed dynamic_cast"<<std::endl;
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

#define COMPARE_BINDING(lhs, rhs) \
        if (osg::getBinding(lhs)<osg::getBinding(rhs)) return true; \
        if (osg::getBinding(rhs)<osg::getBinding(lhs)) return false;


struct LessGeometry
{
    bool operator() (const osg::Geometry* lhs,const osg::Geometry* rhs) const
    {
        if (lhs->getStateSet()<rhs->getStateSet()) return true;
        if (rhs->getStateSet()<lhs->getStateSet()) return false;

        COMPARE_BINDING(lhs->getNormalArray(), rhs->getNormalArray())
        COMPARE_BINDING(lhs->getColorArray(), rhs->getColorArray())
        COMPARE_BINDING(lhs->getSecondaryColorArray(), rhs->getSecondaryColorArray())
        COMPARE_BINDING(lhs->getFogCoordArray(), rhs->getFogCoordArray())


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
        }

        for(i=0;i<lhs->getNumVertexAttribArrays();++i)
        {
            if (rhs->getVertexAttribArray(i))
            {
                if (!lhs->getVertexAttribArray(i)) return true;
            }
            else if (lhs->getVertexAttribArray(i)) return false;
        }


        if (osg::getBinding(lhs->getNormalArray())==osg::Array::BIND_OVERALL)
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

        if (osg::getBinding(lhs->getColorArray())==osg::Array::BIND_OVERALL)
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
#ifdef GEOMETRYDEPRECATED
                geom1829
                ->computeCorrectBindingsAndArraySizes();
#endif
            }
        }
    }
}

void Optimizer::MakeFastGeometryVisitor::checkGeode(osg::Geode& geode)
{
    // GeometryDeprecated CAN REMOVED
    if (isOperationPermissibleForObject(&geode))
    {
        for(unsigned int i=0;i<geode.getNumDrawables();++i)
        {
            osg::Geometry* geom = geode.getDrawable(i)->asGeometry();
            if (geom && isOperationPermissibleForObject(geom))
            {
                if (geom->checkForDeprecatedData())
                {
                    geom->fixDeprecatedData();
                }
            }
        }
    }
}

/// Shortcut to get size of an array, even if pointer is NULL.
inline unsigned int getSize(const osg::Array * a) { return a ? a->getNumElements() : 0; }

/// When merging geometries, tests if two arrays can be merged, regarding to their number of components, and the number of vertices.
bool isArrayCompatible(unsigned int numVertice1, unsigned int numVertice2, const osg::Array* compare1, const osg::Array* compare2)
{
    // Sumed up truth table:
    //  If array (1 or 2) not empty and vertices empty => error, should not happen (allows simplification in formulae below)
    //  If one side has both vertices and array, and the other side has only vertices => then arrays cannot be merged
    //  Else, arrays can be merged
    //assert(numVertice1 || !getSize(compare1));
    //assert(numVertice2 || !getSize(compare2));
    return !(   (numVertice1 && !getSize(compare1) && getSize(compare2))
             || (numVertice2 && !getSize(compare2) && getSize(compare1)) );
}

/// Return true only if both geometries have same array type and if arrays (such as TexCoords) are compatible (i.e. both empty or both filled)
bool isAbleToMerge(const osg::Geometry& g1, const osg::Geometry& g2)
{
    unsigned int numVertice1( getSize(g1.getVertexArray()) );
    unsigned int numVertice2( getSize(g2.getVertexArray()) );

    // first verify arrays size
    if (!isArrayCompatible(numVertice1,numVertice2,g1.getNormalArray(),g2.getNormalArray()) ||
        !isArrayCompatible(numVertice1,numVertice2,g1.getColorArray(),g2.getColorArray()) ||
        !isArrayCompatible(numVertice1,numVertice2,g1.getSecondaryColorArray(),g2.getSecondaryColorArray()) ||
        !isArrayCompatible(numVertice1,numVertice2,g1.getFogCoordArray(),g2.getFogCoordArray()) ||
        g1.getNumTexCoordArrays()!=g2.getNumTexCoordArrays()) return false;

    for (unsigned int eachTexCoordArray=0;eachTexCoordArray<g1.getNumTexCoordArrays();++eachTexCoordArray)
    {
        if (!isArrayCompatible(numVertice1,numVertice2,g1.getTexCoordArray(eachTexCoordArray),g2.getTexCoordArray(eachTexCoordArray))) return false;
    }

    // then verify data type compatibility
    if (g1.getVertexArray() && g2.getVertexArray() && g1.getVertexArray()->getDataType()!=g2.getVertexArray()->getDataType()) return false;
    if (g1.getNormalArray() && g2.getNormalArray() && g1.getNormalArray()->getDataType()!=g2.getNormalArray()->getDataType()) return false;
    if (g1.getColorArray() && g2.getColorArray() && g1.getColorArray()->getDataType()!=g2.getColorArray()->getDataType()) return false;
    if (g1.getSecondaryColorArray() && g2.getSecondaryColorArray() && g1.getSecondaryColorArray()->getDataType()!=g2.getSecondaryColorArray()->getDataType()) return false;
    if (g1.getFogCoordArray() && g2.getNormalArray() && g1.getFogCoordArray()->getDataType()!=g2.getFogCoordArray()->getDataType()) return false;
    return true;
}

bool Optimizer::MergeGeometryVisitor::mergeGeode(osg::Geode& geode)
{
    if (!isOperationPermissibleForObject(&geode)) return false;

    if (geode.getNumDrawables()>=2)
    {

        // OSG_NOTICE<<"Before "<<geode.getNumDrawables()<<std::endl;

        typedef std::vector<osg::Geometry*>                         DuplicateList;
        typedef std::map<osg::Geometry*,DuplicateList,LessGeometry> GeometryDuplicateMap;

        typedef std::vector<DuplicateList> MergeList;

        GeometryDuplicateMap geometryDuplicateMap;
        osg::Geode::DrawableList standardDrawables;

        unsigned int i;
        for(i=0;i<geode.getNumDrawables();++i)
        {
            osg::Geometry* geom = geode.getDrawable(i)->asGeometry();
            if (geom)
            {
                //geom->computeCorrectBindingsAndArraySizes();

                if (!geometryContainsSharedArrays(*geom) &&
                      geom->getDataVariance()!=osg::Object::DYNAMIC &&
                      isOperationPermissibleForObject(geom))
                {
                    geometryDuplicateMap[geom].push_back(geom);
                }
                else
                {
                    standardDrawables.push_back(geode.getDrawable(i));
                }
            }
            else
            {
                standardDrawables.push_back(geode.getDrawable(i));
            }
        }

#if 1
        // first try to group geometries with the same properties
        // (i.e. array types) to avoid loss of data during merging
        MergeList mergeListChecked;        // List of drawables just before merging, grouped by "compatibility" and vertex limit
        MergeList mergeList;            // Intermediate list of drawables, grouped ony by "compatibility"
        for(GeometryDuplicateMap::iterator itr=geometryDuplicateMap.begin();
            itr!=geometryDuplicateMap.end();
            ++itr)
        {
            if (itr->second.empty()) continue;
            if (itr->second.size()==1)
            {
                mergeList.push_back(DuplicateList());
                DuplicateList* duplicateList = &mergeList.back();
                duplicateList->push_back(itr->second[0]);
                continue;
            }

            std::sort(itr->second.begin(),itr->second.end(),LessGeometryPrimitiveType());

            // initialize the temporary list by pushing the first geometry
            MergeList mergeListTmp;
            mergeListTmp.push_back(DuplicateList());
            DuplicateList* duplicateList = &mergeListTmp.back();
            duplicateList->push_back(itr->second[0]);

            for(DuplicateList::iterator dupItr=itr->second.begin()+1;
                dupItr!=itr->second.end();
                ++dupItr)
            {
                osg::Geometry* geomToPush = *dupItr;

                // try to group geomToPush with another geometry
                MergeList::iterator eachMergeList=mergeListTmp.begin();
                for(;eachMergeList!=mergeListTmp.end();++eachMergeList)
                {
                    if (!eachMergeList->empty() && eachMergeList->front()!=NULL
                        && isAbleToMerge(*eachMergeList->front(),*geomToPush))
                    {
                        eachMergeList->push_back(geomToPush);
                        break;
                    }
                }

                // if no suitable group was found, then a new one is created
                if (eachMergeList==mergeListTmp.end())
                {
                    mergeListTmp.push_back(DuplicateList());
                    duplicateList = &mergeListTmp.back();
                    duplicateList->push_back(geomToPush);
                }
            }

            // copy the group in the mergeListChecked
            for(MergeList::iterator eachMergeList=mergeListTmp.begin();eachMergeList!=mergeListTmp.end();++eachMergeList)
            {
                mergeListChecked.push_back(*eachMergeList);
            }
        }

        // then build merge list using _targetMaximumNumberOfVertices
        bool needToDoMerge = false;
        // dequeue each DuplicateList when vertices limit is reached or when all elements has been checked
        for(;!mergeListChecked.empty();)
        {
            MergeList::iterator itr=mergeListChecked.begin();
            DuplicateList& duplicateList(*itr);
            if (duplicateList.size()==0)
            {
                mergeListChecked.erase(itr);
                continue;
            }

            if (duplicateList.size()==1)
            {
                mergeList.push_back(duplicateList);
                mergeListChecked.erase(itr);
                continue;
            }

            unsigned int numVertices(duplicateList.front()->getVertexArray() ? duplicateList.front()->getVertexArray()->getNumElements() : 0);
            DuplicateList::iterator eachGeom(duplicateList.begin()+1);
            // until all geometries have been checked or _targetMaximumNumberOfVertices is reached
            for (;eachGeom!=duplicateList.end(); ++eachGeom)
            {
                unsigned int numAddVertices((*eachGeom)->getVertexArray() ? (*eachGeom)->getVertexArray()->getNumElements() : 0);
                if (numVertices+numAddVertices<_targetMaximumNumberOfVertices)
                {
                    break;
                }
                else
                {
                    numVertices += numAddVertices;
                }
            }

            // push back if bellow the limit
            if (numVertices<_targetMaximumNumberOfVertices)
            {
                if (duplicateList.size()>1) needToDoMerge = true;
                mergeList.push_back(duplicateList);
                mergeListChecked.erase(itr);
            }
            // else split the list to store what is below the limit and retry on what is above
            else
            {
                mergeList.push_back(DuplicateList());
                DuplicateList* duplicateListResult = &mergeList.back();
                duplicateListResult->insert(duplicateListResult->end(),duplicateList.begin(),eachGeom);
                duplicateList.erase(duplicateList.begin(),eachGeom);
                if (duplicateListResult->size()>1) needToDoMerge = true;
            }
        }

        if (needToDoMerge)
        {
            // first take a reference to all the drawables to prevent them being deleted prematurely
            osg::Geode::DrawableList keepDrawables;
            keepDrawables.resize(geode.getNumDrawables());
            for(i=0; i<geode.getNumDrawables(); ++i)
            {
                keepDrawables[i] = geode.getDrawable(i);
            }

            // now clear the drawable list of the Geode so we don't have to remove items one by one (which is slow)
            geode.removeDrawables(0, geode.getNumDrawables());

            // add back in the standard drawables which arn't possible to merge.
            for(osg::Geode::DrawableList::iterator sitr = standardDrawables.begin();
                sitr != standardDrawables.end();
                ++sitr)
            {
                geode.addDrawable(sitr->get());
            }

            // now do the merging of geometries
            for(MergeList::iterator mitr = mergeList.begin();
                mitr != mergeList.end();
                ++mitr)
            {
                DuplicateList& duplicateList = *mitr;
                if (duplicateList.size()>1)
                {
                    osg::Geometry* lhs = duplicateList.front();
                    geode.addDrawable(lhs);
                    for(DuplicateList::iterator ditr = duplicateList.begin()+1;
                        ditr != duplicateList.end();
                        ++ditr)
                    {
                        mergeGeometry(*lhs,**ditr);
                    }
                }
                else if (duplicateList.size()>0)
                {
                    geode.addDrawable(duplicateList.front());
                }
            }
        }

#else
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
                        OSG_INFO<<"merged and removed Geometry "<<++co<<std::endl;
                    }
                }
            }
        }
#endif

        // OSG_NOTICE<<"After "<<geode.getNumDrawables()<<std::endl;

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

    // now merge any compatible primitives.
    for(i=0;i<geode.getNumDrawables();++i)
    {
        osg::Geometry* geom = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
        if (geom)
        {
            if (geom->getNumPrimitiveSets()>0 &&
                osg::getBinding(geom->getNormalArray())!=osg::Array::BIND_PER_PRIMITIVE_SET &&
                osg::getBinding(geom->getColorArray())!=osg::Array::BIND_PER_PRIMITIVE_SET &&
                osg::getBinding(geom->getSecondaryColorArray())!=osg::Array::BIND_PER_PRIMITIVE_SET &&
                osg::getBinding(geom->getFogCoordArray())!=osg::Array::BIND_PER_PRIMITIVE_SET)
            {

#if 1
                bool doneCombine = false;

                osg::Geometry::PrimitiveSetList& primitives = geom->getPrimitiveSetList();
                unsigned int lhsNo=0;
                unsigned int rhsNo=1;
                while(rhsNo<primitives.size())
                {
                    osg::PrimitiveSet* lhs = primitives[lhsNo].get();
                    osg::PrimitiveSet* rhs = primitives[rhsNo].get();

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
                            combine = false;
                            break;
                        }
                    }

                    if (combine)
                    {
                        // make this primitive set as invalid and needing cleaning up.
                        rhs->setMode(0xffffff);
                        doneCombine = true;
                        ++rhsNo;
                    }
                    else
                    {
                        lhsNo = rhsNo;
                        ++rhsNo;
                    }
                }

    #if 1
                if (doneCombine)
                {
                    // now need to clean up primitiveset so it no longer contains the rhs combined primitives.

                    // first swap with a empty primitiveSet to empty it completely.
                    osg::Geometry::PrimitiveSetList oldPrimitives;
                    primitives.swap(oldPrimitives);

                    // now add the active primitive sets
                    for(osg::Geometry::PrimitiveSetList::iterator pitr = oldPrimitives.begin();
                        pitr != oldPrimitives.end();
                        ++pitr)
                    {
                        if ((*pitr)->getMode()!=0xffffff) primitives.push_back(*pitr);
                    }
                }
    #endif

#else

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
#endif
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

    // shift the indices of the incoming primitives to account for the pre existing geometry.
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
    protected:
        osg::Array* _lhs;
        int         _offset;
    public:
        MergeArrayVisitor() :
            _lhs(0),
            _offset(0) {}


        /// try to merge the content of two arrays.
        bool merge(osg::Array* lhs,osg::Array* rhs, int offset=0)
        {
            if (lhs==0 || rhs==0) return true;
            if (lhs->getType()!=rhs->getType()) return false;

            _lhs = lhs;
            _offset = offset;

            rhs->accept(*this);
            return true;
        }

        template<typename T>
        void _merge(T& rhs)
        {
            T* lhs = static_cast<T*>(_lhs);
            lhs->insert(lhs->end(),rhs.begin(),rhs.end());
        }

        template<typename T>
        void _mergeAndOffset(T& rhs)
        {
            T* lhs = static_cast<T*>(_lhs);

            typename T::iterator itr;
            for(itr = rhs.begin();
                itr != rhs.end();
                ++itr)
            {
                lhs->push_back(*itr + _offset);
            }
        }

        virtual void apply(osg::Array&) { OSG_WARN << "Warning: Optimizer's MergeArrayVisitor cannot merge Array type." << std::endl; }

        virtual void apply(osg::ByteArray& rhs) { if (_offset) _mergeAndOffset(rhs); else  _merge(rhs); }
        virtual void apply(osg::ShortArray& rhs) { if (_offset) _mergeAndOffset(rhs); else  _merge(rhs); }
        virtual void apply(osg::IntArray& rhs) { if (_offset) _mergeAndOffset(rhs); else  _merge(rhs); }
        virtual void apply(osg::UByteArray& rhs) { if (_offset) _mergeAndOffset(rhs); else  _merge(rhs); }
        virtual void apply(osg::UShortArray& rhs) { if (_offset) _mergeAndOffset(rhs); else  _merge(rhs); }
        virtual void apply(osg::UIntArray& rhs) { if (_offset) _mergeAndOffset(rhs); else  _merge(rhs); }

        virtual void apply(osg::Vec4ubArray& rhs) { _merge(rhs); }
        virtual void apply(osg::FloatArray& rhs) { _merge(rhs); }
        virtual void apply(osg::Vec2Array& rhs) { _merge(rhs); }
        virtual void apply(osg::Vec3Array& rhs) { _merge(rhs); }
        virtual void apply(osg::Vec4Array& rhs) { _merge(rhs); }

        virtual void apply(osg::DoubleArray& rhs) { _merge(rhs); }
        virtual void apply(osg::Vec2dArray& rhs) { _merge(rhs); }
        virtual void apply(osg::Vec3dArray& rhs) { _merge(rhs); }
        virtual void apply(osg::Vec4dArray& rhs) { _merge(rhs); }

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
        if (!merger.merge(lhs.getVertexArray(),rhs.getVertexArray()))
        {
            OSG_DEBUG << "MergeGeometry: vertex array not merged. Some data may be lost." <<std::endl;
        }
    }
    else if (rhs.getVertexArray())
    {
        base = 0;
        lhs.setVertexArray(rhs.getVertexArray());
    }


    if (lhs.getNormalArray() && rhs.getNormalArray() && lhs.getNormalArray()->getBinding()!=osg::Array::BIND_OVERALL)
    {
        if (!merger.merge(lhs.getNormalArray(),rhs.getNormalArray()))
        {
            OSG_DEBUG << "MergeGeometry: normal array not merged. Some data may be lost." <<std::endl;
        }
    }
    else if (rhs.getNormalArray())
    {
        lhs.setNormalArray(rhs.getNormalArray());
    }


    if (lhs.getColorArray() && rhs.getColorArray() && lhs.getColorArray()->getBinding()!=osg::Array::BIND_OVERALL)
    {
        if (!merger.merge(lhs.getColorArray(),rhs.getColorArray()))
        {
            OSG_DEBUG << "MergeGeometry: color array not merged. Some data may be lost." <<std::endl;
        }
    }
    else if (rhs.getColorArray())
    {
        lhs.setColorArray(rhs.getColorArray());
    }

    if (lhs.getSecondaryColorArray() && rhs.getSecondaryColorArray() && lhs.getSecondaryColorArray()->getBinding()!=osg::Array::BIND_OVERALL)
    {
        if (!merger.merge(lhs.getSecondaryColorArray(),rhs.getSecondaryColorArray()))
        {
            OSG_DEBUG << "MergeGeometry: secondary color array not merged. Some data may be lost." <<std::endl;
        }
    }
    else if (rhs.getSecondaryColorArray())
    {
        lhs.setSecondaryColorArray(rhs.getSecondaryColorArray());
    }

    if (lhs.getFogCoordArray() && rhs.getFogCoordArray() && lhs.getFogCoordArray()->getBinding()!=osg::Array::BIND_OVERALL)
    {
        if (!merger.merge(lhs.getFogCoordArray(),rhs.getFogCoordArray()))
        {
            OSG_DEBUG << "MergeGeometry: fog coord array not merged. Some data may be lost." <<std::endl;
        }
    }
    else if (rhs.getFogCoordArray())
    {
        lhs.setFogCoordArray(rhs.getFogCoordArray());
    }


    unsigned int unit;
    for(unit=0;unit<lhs.getNumTexCoordArrays();++unit)
    {
        if (!merger.merge(lhs.getTexCoordArray(unit),rhs.getTexCoordArray(unit)))
        {
            OSG_DEBUG << "MergeGeometry: tex coord array not merged. Some data may be lost." <<std::endl;
        }
    }

    for(unit=0;unit<lhs.getNumVertexAttribArrays();++unit)
    {
        if (!merger.merge(lhs.getVertexAttribArray(unit),rhs.getVertexAttribArray(unit)))
        {
            OSG_DEBUG << "MergeGeometry: vertex attrib array not merged. Some data may be lost." <<std::endl;
        }
    }


    // shift the indices of the incoming primitives to account for the pre existing geometry.
    osg::Geometry::PrimitiveSetList::iterator primItr;
    for(primItr=rhs.getPrimitiveSetList().begin(); primItr!=rhs.getPrimitiveSetList().end(); ++primItr)
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

    for(primItr=rhs.getPrimitiveSetList().begin(); primItr!=rhs.getPrimitiveSetList().end(); ++primItr)
    {
        lhs.addPrimitiveSet(primItr->get());
    }

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

void Optimizer::SpatializeGroupsVisitor::apply(osg::Geode& geode)
{
    if (typeid(geode)==typeid(osg::Geode))
    {
        if (isOperationPermissibleForObject(&geode))
        {
            _geodesToDivideList.insert(&geode);
        }
    }
    traverse(geode);
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

    for(GeodesToDivideList::iterator geode_itr=_geodesToDivideList.begin();
        geode_itr!=_geodesToDivideList.end();
        ++geode_itr)
    {
        if (divide(*geode_itr,maxNumTreesPerCell)) divided = true;
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

    OSG_INFO<<"Dividing "<<group->className()<<"  num children = "<<group->getNumChildren()<<"  xAxis="<<xAxis<<"  yAxis="<<yAxis<<"   zAxis="<<zAxis<<std::endl;

    if (!xAxis && !yAxis && !zAxis)
    {
        OSG_INFO<<"  No axis to divide, stopping division."<<std::endl;
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
    group->removeChildren(0,group->getNumChildren());

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

bool Optimizer::SpatializeGroupsVisitor::divide(osg::Geode* geode, unsigned int maxNumTreesPerCell)
{

    if (geode->getNumDrawables()<=maxNumTreesPerCell) return false;

    // create the original box.
    osg::BoundingBox bb;
    unsigned int i;
    for(i=0; i<geode->getNumDrawables(); ++i)
    {
        bb.expandBy(geode->getDrawable(i)->getBound().center());
    }

    float radius = bb.radius();
    float divide_distance = radius*0.7f;
    bool xAxis = (bb.xMax()-bb.xMin())>divide_distance;
    bool yAxis = (bb.yMax()-bb.yMin())>divide_distance;
    bool zAxis = (bb.zMax()-bb.zMin())>divide_distance;

    OSG_INFO<<"INFO "<<geode->className()<<"  num drawables = "<<geode->getNumDrawables()<<"  xAxis="<<xAxis<<"  yAxis="<<yAxis<<"   zAxis="<<zAxis<<std::endl;

    if (!xAxis && !yAxis && !zAxis)
    {
        OSG_INFO<<"  No axis to divide, stopping division."<<std::endl;
        return false;
    }

    osg::Node::ParentList parents = geode->getParents();
    if (parents.empty())
    {
        OSG_INFO<<"  Cannot perform spatialize on root Geode, add a Group above it to allow subdivision."<<std::endl;
        return false;
    }

    osg::ref_ptr<osg::Group> group = new osg::Group;
    group->setName(geode->getName());
    group->setStateSet(geode->getStateSet());
    for(i=0; i<geode->getNumDrawables(); ++i)
    {
        osg::Geode* newGeode = new osg::Geode;
        newGeode->addDrawable(geode->getDrawable(i));
        group->addChild(newGeode);
    }

    divide(group.get(), maxNumTreesPerCell);

    // keep reference around to prevent it being deleted.
    osg::ref_ptr<osg::Geode> keepRefGeode = geode;

    for(osg::Node::ParentList::iterator itr = parents.begin();
        itr != parents.end();
        ++itr)
    {
        (*itr)->replaceChild(geode, group.get());
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
//  Duplicated subgraphs which are shared
//

void Optimizer::CopySharedSubgraphsVisitor::apply(osg::Node& node)
{
    if (node.getNumParents()>1 && isOperationPermissibleForObject(&node))
    {
        _sharedNodeList.insert(&node);
    }
    traverse(node);
}

void Optimizer::CopySharedSubgraphsVisitor::copySharedNodes()
{
    OSG_INFO<<"Shared node "<<_sharedNodeList.size()<<std::endl;
    for(SharedNodeList::iterator itr=_sharedNodeList.begin();
        itr!=_sharedNodeList.end();
        ++itr)
    {
        OSG_INFO<<"   No parents "<<(*itr)->getNumParents()<<std::endl;
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
        unsigned numImageStreams = 0;
        for (unsigned int i=0; i<texture.getNumImages(); ++i)
        {
            osg::ImageStream* is = dynamic_cast<osg::ImageStream*>(texture.getImage(i));
            if (is) ++numImageStreams;
        }

        if (numImageStreams==0)
        {
            texture.setUnRefImageDataAfterApply(_valueAutoUnRef);
        }
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
    if (typeid(group)==typeid(osg::Group)) mergeGeodes(group);
    traverse(group);
}

struct LessGeode
{
    bool operator() (const osg::Geode* lhs,const osg::Geode* rhs) const
    {
        if (lhs->getNodeMask()<rhs->getNodeMask()) return true;
        if (lhs->getNodeMask()>rhs->getNodeMask()) return false;

        return (lhs->getStateSet()<rhs->getStateSet());
    }
};

bool Optimizer::MergeGeodesVisitor::mergeGeodes(osg::Group& group)
{
    if (!isOperationPermissibleForObject(&group)) return false;

    typedef std::vector< osg::Geode* >                      DuplicateList;
    typedef std::map<osg::Geode*,DuplicateList,LessGeode>   GeodeDuplicateMap;

    unsigned int i;
    osg::NodeList children;
    children.resize(group.getNumChildren());
    for (i=0; i<group.getNumChildren(); ++i)
    {
        // keep a reference to this child so we can safely clear the group of all children
        // this is done so we don't have to do a search and remove from the list later on.
        children[i] = group.getChild(i);
    }

    // remove all children
    group.removeChildren(0,group.getNumChildren());

    GeodeDuplicateMap geodeDuplicateMap;
    for (i=0; i<children.size(); ++i)
    {
        osg::Node* child = children[i].get();

        if (typeid(*child)==typeid(osg::Geode))
        {
            osg::Geode* geode = static_cast<osg::Geode*>(child);
            geodeDuplicateMap[geode].push_back(geode);
        }
        else
        {
            // not a geode so just add back into group as its a normal child
            group.addChild(child);
        }
    }

    // if no geodes then just return.
    if (geodeDuplicateMap.empty()) return false;

    OSG_INFO<<"mergeGeodes in group '"<<group.getName()<<"' "<<geodeDuplicateMap.size()<<std::endl;

    // merge
    for(GeodeDuplicateMap::iterator itr=geodeDuplicateMap.begin();
        itr!=geodeDuplicateMap.end();
        ++itr)
    {
        if (itr->second.size()>1)
        {
            osg::Geode* lhs = itr->second[0];

            // add geode back into group
            group.addChild(lhs);

            for(DuplicateList::iterator dupItr=itr->second.begin()+1;
                dupItr!=itr->second.end();
                ++dupItr)
            {
                osg::Geode* rhs = *dupItr;
                mergeGeode(*lhs,*rhs);
            }
        }
        else
        {
            osg::Geode* lhs = itr->second[0];

            // add geode back into group
            group.addChild(lhs);
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



////////////////////////////////////////////////////////////////////////////
// FlattenBillboardVisitor
////////////////////////////////////////////////////////////////////////////
void Optimizer::FlattenBillboardVisitor::reset()
{
    _billboards.clear();
}

void Optimizer::FlattenBillboardVisitor::apply(osg::Billboard& billboard)
{
    _billboards[&billboard].push_back(getNodePath());
}

void Optimizer::FlattenBillboardVisitor::process()
{
    for(BillboardNodePathMap::iterator itr = _billboards.begin();
        itr != _billboards.end();
        ++itr)
    {
        bool mergeAcceptable = true;

        osg::ref_ptr<osg::Billboard> billboard = itr->first;

        NodePathList& npl = itr->second;
        osg::Group* mainGroup = 0;
        if (npl.size()>1)
        {
            for(NodePathList::iterator nitr = npl.begin();
                nitr != npl.end();
                ++nitr)
            {
                osg::NodePath& np = *nitr;
                if (np.size()>=3)
                {
                    osg::Group* group = dynamic_cast<osg::Group*>(np[np.size()-3]);
                    if (mainGroup==0) mainGroup = group;

                    osg::MatrixTransform* mt = dynamic_cast<osg::MatrixTransform*>(np[np.size()-2]);

                    if (group == mainGroup &&
                        np[np.size()-1]==billboard.get() &&
                        mt && mt->getDataVariance()==osg::Object::STATIC &&
                        mt->getNumChildren()==1)
                    {
                        const osg::Matrix& m = mt->getMatrix();
                        mergeAcceptable = (m(0,0)==1.0 && m(0,1)==0.0 && m(0,2)==0.0 && m(0,3)==0.0 &&
                                           m(1,0)==0.0 && m(1,1)==1.0 && m(1,2)==0.0 && m(1,3)==0.0 &&
                                           m(2,0)==0.0 && m(2,1)==0.0 && m(2,2)==1.0 && m(2,3)==0.0 &&
                                           m(3,3)==1.0);
                    }
                    else
                    {
                       mergeAcceptable = false;
                    }
                }
                else
                {
                    mergeAcceptable = false;
                }
            }
        }
        else
        {
            mergeAcceptable = false;
        }

        if (mergeAcceptable)
        {
            osg::Billboard* new_billboard = new osg::Billboard;
            new_billboard->setMode(billboard->getMode());
            new_billboard->setAxis(billboard->getAxis());
            new_billboard->setStateSet(billboard->getStateSet());
            new_billboard->setName(billboard->getName());

            mainGroup->addChild(new_billboard);

            typedef std::set<osg::MatrixTransform*> MatrixTransformSet;
            MatrixTransformSet mts;

            for(NodePathList::iterator nitr = npl.begin();
                nitr != npl.end();
                ++nitr)
            {
                osg::NodePath& np = *nitr;
                osg::MatrixTransform* mt = dynamic_cast<osg::MatrixTransform*>(np[np.size()-2]);
                mts.insert(mt);
            }

            for(MatrixTransformSet::iterator mitr = mts.begin();
                mitr != mts.end();
                ++mitr)
            {
                osg::MatrixTransform* mt = *mitr;
                for(unsigned int i=0; i<billboard->getNumDrawables(); ++i)
                {
                    new_billboard->addDrawable(billboard->getDrawable(i),
                                               billboard->getPosition(i)*mt->getMatrix());
                }
                mainGroup->removeChild(mt);
            }
        }
    }

}



////////////////////////////////////////////////////////////////////////////
// TextureAtlasBuilder
////////////////////////////////////////////////////////////////////////////

Optimizer::TextureAtlasBuilder::TextureAtlasBuilder():
    _maximumAtlasWidth(2048),
    _maximumAtlasHeight(2048),
    _margin(8)
{
}

void Optimizer::TextureAtlasBuilder::reset()
{
    _sourceList.clear();
    _atlasList.clear();
}

void Optimizer::TextureAtlasBuilder::setMaximumAtlasSize(int width, int height)
{
    _maximumAtlasWidth = width;
    _maximumAtlasHeight = height;
}

void Optimizer::TextureAtlasBuilder::setMargin(int margin)
{
    _margin = margin;
}

void Optimizer::TextureAtlasBuilder::addSource(const osg::Image* image)
{
    if (!getSource(image)) _sourceList.push_back(new Source(image));
}

void Optimizer::TextureAtlasBuilder::addSource(const osg::Texture2D* texture)
{
    if (!getSource(texture)) _sourceList.push_back(new Source(texture));
}


void Optimizer::TextureAtlasBuilder::completeRow(unsigned int indexAtlas)
{
    AtlasList::iterator aitr = _atlasList.begin() + indexAtlas;
    //SourceList::iterator sitr = _sourceList.begin() + indexSource;
    Atlas * atlas = aitr->get();
    if(atlas->_indexFirstOfRow < atlas->_sourceList.size())
    {
        //Try to fill the row with smaller images.
        int x_max = atlas->_width  - _margin;
        int y_max = atlas->_height - _margin;
        //int x_max = atlas->_maximumAtlasWidth  - _margin;
        //int y_max = atlas->_maximumAtlasHeight - _margin;

        // Fill last Row
        for(SourceList::iterator sitr3 = _sourceList.begin(); sitr3 != _sourceList.end(); ++sitr3)
        {
            int x_min = atlas->_x + _margin;
            int y_min = atlas->_y + _margin;
            if (y_min >= y_max || x_min >= x_max) continue;

            Source * source = sitr3->get();
            if (source->_atlas || atlas->_image->getPixelFormat() != source->_image->getPixelFormat() ||
                atlas->_image->getDataType() != source->_image->getDataType())
            {
                continue;
            }

            int image_s = source->_image->s();
            int image_t = source->_image->t();
            if (x_min + image_s <= x_max && y_min + image_t <= y_max)        // Test if the image can fit in the empty space.
            {
                source->_x = x_min;
                source->_y = y_min;
                //assert(source->_x + source->_image->s()+_margin <= atlas->_maximumAtlasWidth );        // "+_margin" and not "+2*_margin" because _x already takes the margin into account
                //assert(source->_y + source->_image->t()+_margin <= atlas->_maximumAtlasHeight);
                //assert(source->_x >= _margin);
                //assert(source->_y >= _margin);
                atlas->_x += image_s + 2*_margin;
                //assert(atlas->_x <= atlas->_maximumAtlasWidth);
                source->_atlas = atlas;
                atlas->_sourceList.push_back(source);
            }
        }

        // Fill the last column
        SourceList srcListTmp;
        for(SourceList::iterator sitr4 = atlas->_sourceList.begin() + atlas->_indexFirstOfRow;
            sitr4 != atlas->_sourceList.end(); ++sitr4)
        {
            Source * srcAdded = sitr4->get();
            int y_min = srcAdded->_y + srcAdded->_image->t() + 2 * _margin;
            int x_min = srcAdded->_x;
            int x_max = x_min + srcAdded->_image->s();        // Hides upper block's x_max
            if (y_min >= y_max || x_min >= x_max) continue;

            Source * maxWidthSource = NULL;
            for(SourceList::iterator sitr2 = _sourceList.begin(); sitr2 != _sourceList.end(); ++sitr2)
            {
                Source * source = sitr2->get();
                if (source->_atlas || atlas->_image->getPixelFormat() != source->_image->getPixelFormat() ||
                    atlas->_image->getDataType() != source->_image->getDataType())
                {
                    continue;
                }
                int image_s = source->_image->s();
                int image_t = source->_image->t();
                if(x_min + image_s <= x_max && y_min + image_t <= y_max)        // Test if the image can fit in the empty space.
                {
                    if (maxWidthSource == NULL || maxWidthSource->_image->s() < source->_image->s())
                    {
                        maxWidthSource = source; //Keep the maximum width for source.
                    }
                }
            }
            if (maxWidthSource)
            {
                // Add the source with the max width to the atlas
                maxWidthSource->_x = x_min;
                maxWidthSource->_y = y_min;
                maxWidthSource->_atlas = atlas;
                srcListTmp.push_back(maxWidthSource); //Store the mawWidth source in the temporary vector.
            }
        }
        for(SourceList::iterator itTmp = srcListTmp.begin(); itTmp != srcListTmp.end(); ++itTmp)
        {
            //Add the sources to the general list (wasn't possible in the loop using the iterator on the same list)
            atlas->_sourceList.push_back(*itTmp);
        }
        atlas->_indexFirstOfRow = atlas->_sourceList.size();
    }
}

void Optimizer::TextureAtlasBuilder::buildAtlas()
{
    std::sort(_sourceList.begin(), _sourceList.end(), CompareSrc());        // Sort using the height of images
    _atlasList.clear();
    for(SourceList::iterator sitr = _sourceList.begin();
        sitr != _sourceList.end();
        ++sitr)
    {
        Source * source = sitr->get();
        if (!source->_atlas && source->suitableForAtlas(_maximumAtlasWidth,_maximumAtlasHeight,_margin))
        {
            bool addedSourceToAtlas = false;
            for(AtlasList::iterator aitr = _atlasList.begin();
                aitr != _atlasList.end() && !addedSourceToAtlas;
                ++aitr)
            {
                if(!(*aitr)->_image ||
                    ((*aitr)->_image->getPixelFormat() == (*sitr)->_image->getPixelFormat() &&
                    (*aitr)->_image->getPacking() == (*sitr)->_image->getPacking()))
                {
                    OSG_INFO<<"checking source "<<source->_image->getFileName()<<" to see it it'll fit in atlas "<<aitr->get()<<std::endl;
                    Optimizer::TextureAtlasBuilder::Atlas::FitsIn fitsIn = (*aitr)->doesSourceFit(source);
                    if (fitsIn == Optimizer::TextureAtlasBuilder::Atlas::FITS_IN_CURRENT_ROW)
                    {
                        addedSourceToAtlas = true;
                        (*aitr)->addSource(source); // Add in the currentRow.
                    }
                    else if(fitsIn == Optimizer::TextureAtlasBuilder::Atlas::IN_NEXT_ROW)
                    {
                        completeRow(aitr - _atlasList.begin()); //Fill Empty spaces.
                        addedSourceToAtlas = true;
                        (*aitr)->addSource(source); // Add the source in the new row.
                    }
                    else
                    {
                        completeRow(aitr - _atlasList.begin()); //Fill Empty spaces before creating a new atlas.
                    }
                }
            }

            if (!addedSourceToAtlas)
            {
                OSG_INFO<<"creating new Atlas for "<<source->_image->getFileName()<<std::endl;

                osg::ref_ptr<Atlas> atlas = new Atlas(_maximumAtlasWidth,_maximumAtlasHeight,_margin);
                _atlasList.push_back(atlas);
                if (!source->_atlas) atlas->addSource(source);
            }
        }
    }

    // build the atlas which are suitable for use, and discard the rest.
    AtlasList activeAtlasList;
    for(AtlasList::iterator aitr = _atlasList.begin();
        aitr != _atlasList.end();
        ++aitr)
    {
        osg::ref_ptr<Atlas> atlas = *aitr;

        if (atlas->_sourceList.size()==1)
        {
            // no point building an atlas with only one entry
            // so disconnect the source.
            Source * source = atlas->_sourceList[0].get();
            source->_atlas = 0;
            atlas->_sourceList.clear();
        }

        if (!(atlas->_sourceList.empty()))
        {
            std::stringstream ostr;
            ostr<<"atlas_"<<activeAtlasList.size()<<".rgb";
            atlas->_image->setFileName(ostr.str());
            activeAtlasList.push_back(atlas);
            atlas->clampToNearestPowerOfTwoSize();
            atlas->copySources();
        }
    }
    // keep only the active atlas'
    _atlasList.swap(activeAtlasList);

}

osg::Image* Optimizer::TextureAtlasBuilder::getImageAtlas(unsigned int i)
{
    Source* source = _sourceList[i].get();
    Atlas* atlas = source ? source->_atlas : 0;
    return atlas ? atlas->_image.get() : 0;
}

osg::Texture2D* Optimizer::TextureAtlasBuilder::getTextureAtlas(unsigned int i)
{
    Source* source = _sourceList[i].get();
    Atlas* atlas = source ? source->_atlas : 0;
    return atlas ? atlas->_texture.get() : 0;
}

osg::Matrix Optimizer::TextureAtlasBuilder::getTextureMatrix(unsigned int i)
{
    Source* source = _sourceList[i].get();
    return source ? source->computeTextureMatrix() : osg::Matrix();
}

osg::Image* Optimizer::TextureAtlasBuilder::getImageAtlas(const osg::Image* image)
{
    Source* source = getSource(image);
    Atlas* atlas = source ? source->_atlas : 0;
    return atlas ? atlas->_image.get() : 0;
}

osg::Texture2D* Optimizer::TextureAtlasBuilder::getTextureAtlas(const osg::Image* image)
{
    Source* source = getSource(image);
    Atlas* atlas = source ? source->_atlas : 0;
    return atlas ? atlas->_texture.get() : 0;
}

osg::Matrix Optimizer::TextureAtlasBuilder::getTextureMatrix(const osg::Image* image)
{
    Source* source = getSource(image);
    return source ? source->computeTextureMatrix() : osg::Matrix();
}

osg::Image* Optimizer::TextureAtlasBuilder::getImageAtlas(const osg::Texture2D* texture)
{
    Source* source = getSource(texture);
    Atlas* atlas = source ? source->_atlas : 0;
    return atlas ? atlas->_image.get() : 0;
}

osg::Texture2D* Optimizer::TextureAtlasBuilder::getTextureAtlas(const osg::Texture2D* texture)
{
    Source* source = getSource(texture);
    Atlas* atlas = source ? source->_atlas : 0;
    return atlas ? atlas->_texture.get() : 0;
}

osg::Matrix Optimizer::TextureAtlasBuilder::getTextureMatrix(const osg::Texture2D* texture)
{
    Source* source = getSource(texture);
    return source ? source->computeTextureMatrix() : osg::Matrix();
}

Optimizer::TextureAtlasBuilder::Source* Optimizer::TextureAtlasBuilder::getSource(const osg::Image* image)
{
    for(SourceList::iterator itr = _sourceList.begin();
        itr != _sourceList.end();
        ++itr)
    {
        if ((*itr)->_image == image) return itr->get();
    }
    return 0;
}

Optimizer::TextureAtlasBuilder::Source* Optimizer::TextureAtlasBuilder::getSource(const osg::Texture2D* texture)
{
    for(SourceList::iterator itr = _sourceList.begin();
        itr != _sourceList.end();
        ++itr)
    {
        if ((*itr)->_texture == texture) return itr->get();
    }
    return 0;
}

bool Optimizer::TextureAtlasBuilder::Source::suitableForAtlas(int maximumAtlasWidth, int maximumAtlasHeight, int margin)
{
    if (!_image) return false;

    // size too big?
    if (_image->s()+margin*2 > maximumAtlasWidth) return false;
    if (_image->t()+margin*2 > maximumAtlasHeight) return false;

    switch(_image->getPixelFormat())
    {
        case(GL_COMPRESSED_ALPHA_ARB):
        case(GL_COMPRESSED_INTENSITY_ARB):
        case(GL_COMPRESSED_LUMINANCE_ALPHA_ARB):
        case(GL_COMPRESSED_LUMINANCE_ARB):
        case(GL_COMPRESSED_RGBA_ARB):
        case(GL_COMPRESSED_RGB_ARB):
        case(GL_COMPRESSED_RGB_S3TC_DXT1_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT):
            // can't handle compressed textures inside an atlas
            return false;
        default:
            break;
    }

    if ((_image->getPixelSizeInBits() % 8) != 0)
    {
        // pixel size not byte aligned so report as not suitable to prevent other atlas code from having problems with byte boundaries.
        return false;
    }
    if (_texture.valid())
    {

        if (_texture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::REPEAT ||
            _texture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::MIRROR)
        {
            // can't support repeating textures in texture atlas
            return false;
        }

        if (_texture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::REPEAT ||
            _texture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::MIRROR)
        {
            // can't support repeating textures in texture atlas
            return false;
        }

        if (_texture->getReadPBuffer()!=0)
        {
            // pbuffer textures not suitable
            return false;
        }
    }

    return true;
}

osg::Matrix Optimizer::TextureAtlasBuilder::Source::computeTextureMatrix() const
{
    if (!_atlas) return osg::Matrix();
    if (!_image) return osg::Matrix();
    if (!(_atlas->_image)) return osg::Matrix();

    typedef osg::Matrix::value_type Float;
    return osg::Matrix::scale(Float(_image->s())/Float(_atlas->_image->s()), Float(_image->t())/Float(_atlas->_image->t()), 1.0)*
           osg::Matrix::translate(Float(_x)/Float(_atlas->_image->s()), Float(_y)/Float(_atlas->_image->t()), 0.0);
}

Optimizer::TextureAtlasBuilder::Atlas::FitsIn Optimizer::TextureAtlasBuilder::Atlas::doesSourceFit(Source* source)
{
    // does the source have a valid image?
    const osg::Image* sourceImage = source->_image.get();
    if (!sourceImage) return DOES_NOT_FIT_IN_ANY_ROW;

    // does pixel format match?
    if (_image.valid())
    {
        if (_image->getPixelFormat() != sourceImage->getPixelFormat()) return DOES_NOT_FIT_IN_ANY_ROW;
        if (_image->getDataType() != sourceImage->getDataType()) return DOES_NOT_FIT_IN_ANY_ROW;
    }

    const osg::Texture2D* sourceTexture = source->_texture.get();
    if (sourceTexture)
    {
        if (sourceTexture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::REPEAT ||
            sourceTexture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::MIRROR)
        {
            // can't support repeating textures in texture atlas
            return DOES_NOT_FIT_IN_ANY_ROW;
        }

        if (sourceTexture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::REPEAT ||
            sourceTexture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::MIRROR)
        {
            // can't support repeating textures in texture atlas
            return DOES_NOT_FIT_IN_ANY_ROW;
        }

        if (sourceTexture->getReadPBuffer()!=0)
        {
            // pbuffer textures not suitable
            return DOES_NOT_FIT_IN_ANY_ROW;
        }

        if (_texture.valid())
        {

            bool sourceUsesBorder = sourceTexture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::CLAMP_TO_BORDER ||
                                    sourceTexture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::CLAMP_TO_BORDER;

            bool atlasUsesBorder = sourceTexture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::CLAMP_TO_BORDER ||
                                   sourceTexture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::CLAMP_TO_BORDER;

            if (sourceUsesBorder!=atlasUsesBorder)
            {
                // border wrapping does not match
                return DOES_NOT_FIT_IN_ANY_ROW;
            }

            if (sourceUsesBorder)
            {
                // border colours don't match
                if (_texture->getBorderColor() != sourceTexture->getBorderColor()) return DOES_NOT_FIT_IN_ANY_ROW;
            }

            if (_texture->getFilter(osg::Texture2D::MIN_FILTER) != sourceTexture->getFilter(osg::Texture2D::MIN_FILTER))
            {
                // inconsitent min filters
                return DOES_NOT_FIT_IN_ANY_ROW;
            }

            if (_texture->getFilter(osg::Texture2D::MAG_FILTER) != sourceTexture->getFilter(osg::Texture2D::MAG_FILTER))
            {
                // inconsitent mag filters
                return DOES_NOT_FIT_IN_ANY_ROW;
            }

            if (_texture->getMaxAnisotropy() != sourceTexture->getMaxAnisotropy())
            {
                // anisotropy different.
                return DOES_NOT_FIT_IN_ANY_ROW;
            }

            if (_texture->getInternalFormat() != sourceTexture->getInternalFormat())
            {
                // internal formats inconistent
                return DOES_NOT_FIT_IN_ANY_ROW;
            }

            if (_texture->getShadowCompareFunc() != sourceTexture->getShadowCompareFunc())
            {
                // shadow functions inconsitent
                return DOES_NOT_FIT_IN_ANY_ROW;
            }

            if (_texture->getShadowTextureMode() != sourceTexture->getShadowTextureMode())
            {
                // shadow texture mode inconsitent
                return DOES_NOT_FIT_IN_ANY_ROW;
            }

            if (_texture->getShadowAmbient() != sourceTexture->getShadowAmbient())
            {
                // shadow ambient inconsitent
                return DOES_NOT_FIT_IN_ANY_ROW;
            }
        }
    }

    if (sourceImage->s() + 2*_margin > _maximumAtlasWidth)
    {
        // image too big for Atlas
        return DOES_NOT_FIT_IN_ANY_ROW;
    }

    if (sourceImage->t() + 2*_margin > _maximumAtlasHeight)
    {
        // image too big for Atlas
        return DOES_NOT_FIT_IN_ANY_ROW;
    }

    if ((_y + sourceImage->t() + 2*_margin) > _maximumAtlasHeight)
    {
        // image doesn't have up space in height axis.
        return DOES_NOT_FIT_IN_ANY_ROW;
    }

    // does the source fit in the current row?
    if ((_x + sourceImage->s() + 2*_margin) <= _maximumAtlasWidth)
    {
        // yes it fits :-)
        OSG_INFO<<"Fits in current row"<<std::endl;
        return FITS_IN_CURRENT_ROW;
    }

    // does the source fit in the new row up?
    if ((_height + sourceImage->t() + 2*_margin) <= _maximumAtlasHeight)
    {
        // yes it fits :-)
        OSG_INFO<<"Fits in next row"<<std::endl;
        return IN_NEXT_ROW;
    }

    // no space for the texture
    return DOES_NOT_FIT_IN_ANY_ROW;
}

bool Optimizer::TextureAtlasBuilder::Atlas::addSource(Source* source)
{
    // double check source is compatible
    if (!doesSourceFit(source))
    {
        OSG_INFO<<"source "<<source->_image->getFileName()<<" does not fit in atlas "<<this<<std::endl;
        return false;
    }
    const osg::Image* sourceImage = source->_image.get();
    const osg::Texture2D* sourceTexture = source->_texture.get();

    if (!_image)
    {
        // need to create an image of the same pixel format to store the atlas in
        _image = new osg::Image;
        _image->setPacking(sourceImage->getPacking());
        _image->setPixelFormat(sourceImage->getPixelFormat());
        _image->setDataType(sourceImage->getDataType());
    }

    if (!_texture && sourceTexture)
    {
        _texture = new osg::Texture2D(_image.get());

        _texture->setWrap(osg::Texture2D::WRAP_S, sourceTexture->getWrap(osg::Texture2D::WRAP_S));
        _texture->setWrap(osg::Texture2D::WRAP_T, sourceTexture->getWrap(osg::Texture2D::WRAP_T));

        _texture->setBorderColor(sourceTexture->getBorderColor());
        _texture->setBorderWidth(0);

        _texture->setFilter(osg::Texture2D::MIN_FILTER, sourceTexture->getFilter(osg::Texture2D::MIN_FILTER));
        _texture->setFilter(osg::Texture2D::MAG_FILTER, sourceTexture->getFilter(osg::Texture2D::MAG_FILTER));

        _texture->setMaxAnisotropy(sourceTexture->getMaxAnisotropy());

        _texture->setInternalFormat(sourceTexture->getInternalFormat());

        _texture->setShadowCompareFunc(sourceTexture->getShadowCompareFunc());
        _texture->setShadowTextureMode(sourceTexture->getShadowTextureMode());
        _texture->setShadowAmbient(sourceTexture->getShadowAmbient());

    }

    // now work out where to fit it, first try current row.
    if ((_x + sourceImage->s() + 2*_margin) <= _maximumAtlasWidth)
    {
        // yes it fits, so add the source to the atlas's list of sources it contains
        _sourceList.push_back(source);

        OSG_INFO<<"current row insertion, source "<<source->_image->getFileName()<<" "<<_x<<","<<_y<<" fits in row of atlas "<<this<<std::endl;

        // set up the source so it knows where it is in the atlas
        source->_x = _x + _margin;
        source->_y = _y + _margin;
        source->_atlas = this;

        // move the atlas' cursor along to the right
        _x += sourceImage->s() + 2*_margin;

        if (_x > _width) _width = _x;

        int localTop = _y + sourceImage->t() + 2*_margin;
        if ( localTop > _height) _height = localTop;

        return true;
    }

    // does the source fit in the new row up?
    if ((_height + sourceImage->t() + 2*_margin) <= _maximumAtlasHeight)
    {
        // now row so first need to reset the atlas cursor
        _x = 0;
        _y = _height;

        // yes it fits, so add the source to the atlas' list of sources it contains
        _sourceList.push_back(source);

        OSG_INFO<<"next row insertion, source "<<source->_image->getFileName()<<" "<<_x<<","<<_y<<" fits in row of atlas "<<this<<std::endl;

        // set up the source so it knows where it is in the atlas
        source->_x = _x + _margin;
        source->_y = _y + _margin;
        source->_atlas = this;

        // move the atlas' cursor along to the right
        _x += sourceImage->s() + 2*_margin;

        if (_x > _width) _width = _x;

        _height = _y + sourceImage->t() + 2*_margin;

        OSG_INFO<<"source "<<source->_image->getFileName()<<" "<<_x<<","<<_y<<" fits in row of atlas "<<this<<std::endl;

        return true;
    }

    OSG_INFO<<"source "<<source->_image->getFileName()<<" does not fit in atlas "<<this<<std::endl;

    // shouldn't get here, unless doesSourceFit isn't working...
    return false;
}

void Optimizer::TextureAtlasBuilder::Atlas::clampToNearestPowerOfTwoSize()
{
    int w = 1;
    while (w<_width) w *= 2;

    int h = 1;
    while (h<_height) h *= 2;

    OSG_INFO<<"Clamping "<<_width<<", "<<_height<<" to "<<w<<","<<h<<std::endl;

    _width = w;
    _height = h;
}


void Optimizer::TextureAtlasBuilder::Atlas::copySources()
{
    GLenum pixelFormat = _image->getPixelFormat();
    GLenum dataType = _image->getDataType();
    GLenum packing = _image->getPacking();
    OSG_INFO<<"Allocated to "<<_width<<","<<_height<<std::endl;
    _image->allocateImage(_width,_height,1,
                          pixelFormat, dataType,
                          packing);

    {
        // clear memory
        unsigned int size = _image->getTotalSizeInBytes();
        unsigned char* str = _image->data();
        for(unsigned int i=0; i<size; ++i) *(str++) = 0;
    }

    OSG_INFO<<"Atlas::copySources() "<<std::endl;

    for(SourceList::iterator itr = _sourceList.begin();
        itr !=_sourceList.end();
        ++itr)
    {
        Source* source = itr->get();
        Atlas* atlas = source->_atlas;

        if (atlas == this)
        {
            OSG_INFO<<"Copying image "<<source->_image->getFileName()<<" to "<<source->_x<<" ,"<<source->_y<<std::endl;
            OSG_INFO<<"        image size "<<source->_image->s()<<","<<source->_image->t()<<std::endl;

            const osg::Image* sourceImage = source->_image.get();
            osg::Image* atlasImage = atlas->_image.get();
            //assert(sourceImage->getPacking() == atlasImage->getPacking()); //Test if packings are equals.
            unsigned int rowSize = sourceImage->getRowSizeInBytes();
            unsigned int pixelSizeInBits = sourceImage->getPixelSizeInBits();
            unsigned int pixelSizeInBytes = pixelSizeInBits/8;
            unsigned int marginSizeInBytes = pixelSizeInBytes*_margin;

            //assert(atlas->_width  == static_cast<int>(atlasImage->s()));
            //assert(atlas->_height == static_cast<int>(atlasImage->t()));
            //assert(source->_x + static_cast<int>(source->_image->s())+_margin <= static_cast<int>(atlas->_image->s()));        // "+_margin" and not "+2*_margin" because _x already takes the margin into account
            //assert(source->_y + static_cast<int>(source->_image->t())+_margin <= static_cast<int>(atlas->_image->t()));
            //assert(source->_x >= _margin);
            //assert(source->_y >= _margin);
            int x = source->_x;
            int y = source->_y;

            int t;
            for(t=0; t<sourceImage->t(); ++t, ++y)
            {
                unsigned char* destPtr = atlasImage->data(x, y);
                const unsigned char* sourcePtr = sourceImage->data(0, t);
                for(unsigned int i=0; i<rowSize; i++)
                {
                    *(destPtr++) = *(sourcePtr++);
                }
            }

            // copy top row margin
            y = source->_y + sourceImage->t();
            int m;
            for(m=0; m<_margin; ++m, ++y)
            {
                unsigned char* destPtr = atlasImage->data(x, y);
                const unsigned char* sourcePtr = sourceImage->data(0, sourceImage->t()-1);
                for(unsigned int i=0; i<rowSize; i++)
                {
                    *(destPtr++) = *(sourcePtr++);
                }

            }



            // copy bottom row margin
            y = source->_y-1;
            for(m=0; m<_margin; ++m, --y)
            {
                unsigned char* destPtr = atlasImage->data(x, y);
                const unsigned char* sourcePtr = sourceImage->data(0, 0);
                for(unsigned int i=0; i<rowSize; i++)
                {
                    *(destPtr++) = *(sourcePtr++);
                }

            }

            // copy left column margin
            y = source->_y;
            for(t=0; t<sourceImage->t(); ++t, ++y)
            {
                x = source->_x-1;
                for(m=0; m<_margin; ++m, --x)
                {
                    unsigned char* destPtr = atlasImage->data(x, y);
                    const unsigned char* sourcePtr = sourceImage->data(0, t);
                    for(unsigned int i=0; i<pixelSizeInBytes; i++)
                    {
                        *(destPtr++) = *(sourcePtr++);
                    }
                }
            }

            // copy right column margin
            y = source->_y;
            for(t=0; t<sourceImage->t(); ++t, ++y)
            {
                x = source->_x + sourceImage->s();
                for(m=0; m<_margin; ++m, ++x)
                {
                    unsigned char* destPtr = atlasImage->data(x, y);
                    const unsigned char* sourcePtr = sourceImage->data(sourceImage->s()-1, t);
                    for(unsigned int i=0; i<pixelSizeInBytes; i++)
                    {
                        *(destPtr++) = *(sourcePtr++);
                    }
                }
            }

            // copy top left corner margin
            y = source->_y + sourceImage->t();
            for(m=0; m<_margin; ++m, ++y)
            {
                unsigned char* destPtr = atlasImage->data(source->_x - _margin, y);
                unsigned char* sourcePtr = atlasImage->data(source->_x - _margin, y-1); // copy from row below
                for(unsigned int i=0; i<marginSizeInBytes; i++)
                {
                    *(destPtr++) = *(sourcePtr++);
                }
            }

            // copy top right corner margin
            y = source->_y + sourceImage->t();
            for(m=0; m<_margin; ++m, ++y)
            {
                unsigned char* destPtr = atlasImage->data(source->_x + sourceImage->s(), y);
                unsigned char* sourcePtr = atlasImage->data(source->_x + sourceImage->s(), y-1); // copy from row below
                for(unsigned int i=0; i<marginSizeInBytes; i++)
                {
                    *(destPtr++) = *(sourcePtr++);
                }
            }

            // copy bottom left corner margin
            y = source->_y - 1;
            for(m=0; m<_margin; ++m, --y)
            {
                unsigned char* destPtr = atlasImage->data(source->_x - _margin, y);
                unsigned char* sourcePtr = atlasImage->data(source->_x - _margin, y+1); // copy from row below
                for(unsigned int i=0; i<marginSizeInBytes; i++)
                {
                    *(destPtr++) = *(sourcePtr++);
                }
            }

            // copy bottom right corner margin
            y = source->_y - 1;
            for(m=0; m<_margin; ++m, --y)
            {
                unsigned char* destPtr = atlasImage->data(source->_x + sourceImage->s(), y);
                unsigned char* sourcePtr = atlasImage->data(source->_x + sourceImage->s(), y+1); // copy from row below
                for(unsigned int i=0; i<marginSizeInBytes; i++)
                {
                    *(destPtr++) = *(sourcePtr++);
                }
            }

        }
    }
}


void Optimizer::TextureAtlasVisitor::reset()
{
    _statesetMap.clear();
    _statesetStack.clear();
    _textures.clear();
    _builder.reset();
}

bool Optimizer::TextureAtlasVisitor::pushStateSet(osg::StateSet* stateset)
{
    const osg::StateSet::TextureAttributeList& tal = stateset->getTextureAttributeList();

    // if no textures ignore
    if (tal.empty()) return false;

    bool pushStateState = false;

    // if already in stateset list ignore
    if (_statesetMap.count(stateset)>0)
    {
        pushStateState = true;
    }
    else
    {
        bool containsTexture2D = false;
        for(unsigned int unit=0; unit<tal.size(); ++unit)
        {
            osg::Texture2D* texture2D = dynamic_cast<osg::Texture2D*>(stateset->getTextureAttribute(unit,osg::StateAttribute::TEXTURE));
            if (texture2D)
            {
                containsTexture2D = true;
                _textures.insert(texture2D);
            }
        }

        if (containsTexture2D)
        {
            _statesetMap[stateset];
            pushStateState = true;
        }
    }

    if (pushStateState)
    {
        _statesetStack.push_back(stateset);
    }


    return pushStateState;
}

void Optimizer::TextureAtlasVisitor::popStateSet()
{
    _statesetStack.pop_back();
}

void Optimizer::TextureAtlasVisitor::apply(osg::Node& node)
{
    bool pushedStateState = false;

    osg::StateSet* ss = node.getStateSet();
    if (ss && ss->getDataVariance()==osg::Object::STATIC)
    {
        if (isOperationPermissibleForObject(&node) &&
            isOperationPermissibleForObject(ss))
        {
            pushedStateState = pushStateSet(ss);
        }
    }

    traverse(node);

    if (pushedStateState) popStateSet();
}

void Optimizer::TextureAtlasVisitor::apply(osg::Geode& geode)
{
    if (!isOperationPermissibleForObject(&geode)) return;

    osg::StateSet* ss = geode.getStateSet();


    bool pushedGeodeStateState = false;

    if (ss && ss->getDataVariance()==osg::Object::STATIC)
    {
        if (isOperationPermissibleForObject(ss))
        {
            pushedGeodeStateState = pushStateSet(ss);
        }
    }
    for(unsigned int i=0;i<geode.getNumDrawables();++i)
    {

        osg::Drawable* drawable = geode.getDrawable(i);
        if (drawable)
        {
            bool pushedDrawableStateState = false;

            ss = drawable->getStateSet();
            if (ss && ss->getDataVariance()==osg::Object::STATIC)
            {
                if (isOperationPermissibleForObject(drawable) &&
                    isOperationPermissibleForObject(ss))
                {
                    pushedDrawableStateState = pushStateSet(ss);
                }
            }

            if (!_statesetStack.empty())
            {
                for(StateSetStack::iterator ssitr = _statesetStack.begin();
                    ssitr != _statesetStack.end();
                    ++ssitr)
                {
                    _statesetMap[*ssitr].insert(drawable);
                }
            }

            if (pushedDrawableStateState) popStateSet();
        }

    }

    if (pushedGeodeStateState) popStateSet();
}

void Optimizer::TextureAtlasVisitor::optimize()
{
    _builder.reset();

    if (_textures.size()<2)
    {
        // nothing to optimize
        return;
    }

    Textures texturesThatRepeat;
    Textures texturesThatRepeatAndAreOutOfRange;

    StateSetMap::iterator sitr;
    for(sitr = _statesetMap.begin();
        sitr != _statesetMap.end();
        ++sitr)
    {
        osg::StateSet* stateset = sitr->first;
        Drawables& drawables = sitr->second;

        const osg::StateSet::TextureAttributeList& tal = stateset->getTextureAttributeList();
        for(unsigned int unit=0; unit<tal.size(); ++unit)
        {
            osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(stateset->getTextureAttribute(unit,osg::StateAttribute::TEXTURE));
            if (texture)
            {
                bool s_repeat = texture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::REPEAT ||
                                texture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::MIRROR;

                bool t_repeat = texture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::REPEAT ||
                                texture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::MIRROR;

                if (s_repeat || t_repeat)
                {
                    texturesThatRepeat.insert(texture);

                    bool s_outOfRange = false;
                    bool t_outOfRange = false;

                    float s_min = -0.001;
                    float s_max = 1.001;

                    float t_min = -0.001;
                    float t_max = 1.001;

                    for(Drawables::iterator ditr = drawables.begin();
                        ditr != drawables.end();
                        ++ditr)
                    {
                        osg::Geometry* geom = (*ditr)->asGeometry();
                        osg::Vec2Array* texcoords = geom ? dynamic_cast<osg::Vec2Array*>(geom->getTexCoordArray(unit)) : 0;
                        if (texcoords && !texcoords->empty())
                        {
                            for(osg::Vec2Array::iterator titr = texcoords->begin();
                                titr != texcoords->end() /*&& !s_outOfRange && !t_outOfRange*/;
                                ++titr)
                            {
                                osg::Vec2 tc = *titr;
                                if (tc[0]<s_min) { s_min = tc[0]; s_outOfRange = true; }
                                if (tc[0]>s_max) { s_max = tc[0]; s_outOfRange = true; }

                                if (tc[1]<t_min) { t_min = tc[1]; t_outOfRange = true; }
                                if (tc[1]>t_max) { t_max = tc[1]; t_outOfRange = true; }
                            }
                        }
                        else
                        {
                            // if no texcoords then texgen must be being used, therefore must assume that texture is truely repeating
                            s_outOfRange = true;
                            t_outOfRange = true;
                        }
                    }

                    if (s_outOfRange || t_outOfRange)
                    {
                        texturesThatRepeatAndAreOutOfRange.insert(texture);
                    }

                }
            }
        }
    }

    // now change any texture that repeat but all texcoords to them
    // are in 0 to 1 range than converting the to CLAMP mode, to allow them
    // to be used in an atlas.
    Textures::iterator titr;
    for(titr = texturesThatRepeat.begin();
        titr != texturesThatRepeat.end();
        ++titr)
    {
        osg::Texture2D* texture = *titr;
        if (texturesThatRepeatAndAreOutOfRange.count(texture)==0)
        {
            // safe to convert into CLAMP wrap mode.
            OSG_INFO<<"Changing wrap mode to CLAMP"<<std::endl;
            texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
            texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
        }
    }
    //typedef std::list<osg::Texture2D *> SourceListTmp;
    //SourceListTmp sourceToAdd;
    // add the textures as sources for the TextureAtlasBuilder
    for(titr = _textures.begin();
        titr != _textures.end();
        ++titr)
    {
        osg::Texture2D* texture = *titr;

        bool s_repeat = texture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::REPEAT ||
                        texture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::MIRROR;

        bool t_repeat = texture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::REPEAT ||
                        texture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::MIRROR;

        if (texture->getImage() && !s_repeat && !t_repeat)
        {
            _builder.addSource(*titr);
        }
    }

    _builder.buildAtlas();


    typedef std::set<osg::StateSet*> StateSetSet;
    typedef std::map<osg::Drawable*, StateSetSet> DrawableStateSetMap;
    DrawableStateSetMap dssm;
    for(sitr = _statesetMap.begin();
        sitr != _statesetMap.end();
        ++sitr)
    {
        Drawables& drawables = sitr->second;
        for(Drawables::iterator ditr = drawables.begin();
            ditr != drawables.end();
            ++ditr)
        {
            dssm[(*ditr)->asGeometry()].insert(sitr->first);
        }
    }

    Drawables drawablesThatHaveMultipleTexturesOnOneUnit;
    for(DrawableStateSetMap::iterator ditr = dssm.begin();
        ditr != dssm.end();
        ++ditr)
    {
        osg::Drawable* drawable = ditr->first;
        StateSetSet& ssm = ditr->second;
        if (ssm.size()>1)
        {
            typedef std::map<unsigned int, Textures> UnitTextureMap;
            UnitTextureMap unitTextureMap;
            for(StateSetSet::iterator ssm_itr = ssm.begin();
                ssm_itr != ssm.end();
                ++ssm_itr)
            {
                osg::StateSet* ss = *ssm_itr;
                unsigned int numTextureUnits = ss->getTextureAttributeList().size();
                for(unsigned int unit=0; unit<numTextureUnits; ++unit)
                {
                    osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(ss->getTextureAttribute(unit, osg::StateAttribute::TEXTURE));
                    if (texture) unitTextureMap[unit].insert(texture);
                }
            }
            bool drawablesHasMultiTextureOnOneUnit = false;
            for(UnitTextureMap::iterator utm_itr = unitTextureMap.begin();
                utm_itr != unitTextureMap.end() && !drawablesHasMultiTextureOnOneUnit;
                ++utm_itr)
            {
                if (utm_itr->second.size()>1)
                {
                    drawablesHasMultiTextureOnOneUnit = true;
                }
            }
            if (drawablesHasMultiTextureOnOneUnit)
            {
                drawablesThatHaveMultipleTexturesOnOneUnit.insert(drawable);
            }

        }
    }

    // remap the textures in the StateSet's
    for(sitr = _statesetMap.begin();
        sitr != _statesetMap.end();
        ++sitr)
    {
        osg::StateSet* stateset = sitr->first;
        const osg::StateSet::TextureAttributeList& tal = stateset->getTextureAttributeList();
        for(unsigned int unit=0; unit<tal.size(); ++unit)
        {
            osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(stateset->getTextureAttribute(unit,osg::StateAttribute::TEXTURE));
            if (texture)
            {
                bool s_repeat = texture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::REPEAT ||
                                texture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::MIRROR;

                bool t_repeat = texture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::REPEAT ||
                                texture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::MIRROR;

                osg::Texture2D* newTexture = _builder.getTextureAtlas(texture);
                if (newTexture && newTexture!=texture)
                {
                    if (s_repeat || t_repeat)
                    {
                        OSG_NOTICE<<"Warning!!! shouldn't get here"<<std::endl;
                    }

                    stateset->setTextureAttribute(unit, newTexture);

                    Drawables& drawables = sitr->second;

                    osg::Matrix matrix = _builder.getTextureMatrix(texture);

                    // first check to see if all drawables are ok for applying texturematrix to.
                    bool canTexMatBeFlattenedToAllDrawables = true;
                    for(Drawables::iterator ditr = drawables.begin();
                        ditr != drawables.end() && canTexMatBeFlattenedToAllDrawables;
                        ++ditr)
                    {
                        osg::Geometry* geom = (*ditr)->asGeometry();
                        osg::Vec2Array* texcoords = geom ? dynamic_cast<osg::Vec2Array*>(geom->getTexCoordArray(unit)) : 0;

                        if (!texcoords)
                        {
                            canTexMatBeFlattenedToAllDrawables = false;
                        }

                        if (drawablesThatHaveMultipleTexturesOnOneUnit.count(*ditr)!=0)
                        {
                            canTexMatBeFlattenedToAllDrawables = false;
                        }
                    }

                    if (canTexMatBeFlattenedToAllDrawables)
                    {
                        // OSG_NOTICE<<"All drawables can be flattened "<<drawables.size()<<std::endl;
                        for(Drawables::iterator ditr = drawables.begin();
                            ditr != drawables.end();
                            ++ditr)
                        {
                            osg::Geometry* geom = (*ditr)->asGeometry();
                            osg::Vec2Array* texcoords = geom ? dynamic_cast<osg::Vec2Array*>(geom->getTexCoordArray(unit)) : 0;
                            if (texcoords)
                            {
                                for(osg::Vec2Array::iterator titr = texcoords->begin();
                                    titr != texcoords->end();
                                    ++titr)
                                {
                                    osg::Vec2 tc = *titr;
                                    (*titr).set(tc[0]*matrix(0,0) + tc[1]*matrix(1,0) + matrix(3,0),
                                              tc[0]*matrix(0,1) + tc[1]*matrix(1,1) + matrix(3,1));
                                }
                            }
                            else
                            {
                                OSG_NOTICE<<"Error, Optimizer::TextureAtlasVisitor::optimize() shouldn't ever get here..."<<std::endl;
                            }
                        }
                    }
                    else
                    {
                        // OSG_NOTICE<<"Applying TexMat "<<drawables.size()<<std::endl;
                        stateset->setTextureAttribute(unit, new osg::TexMat(matrix));
                    }
                }
            }
        }

    }
}




////////////////////////////////////////////////////////////////////////////
// StaticObjectDectionVisitor
////////////////////////////////////////////////////////////////////////////

void Optimizer::StaticObjectDetectionVisitor::apply(osg::Node& node)
{
    if (node.getStateSet()) applyStateSet(*node.getStateSet());

    traverse(node);
}

void Optimizer::StaticObjectDetectionVisitor::apply(osg::Geode& geode)
{
    if (geode.getStateSet()) applyStateSet(*geode.getStateSet());

    for(unsigned int i=0; i<geode.getNumDrawables(); ++i)
    {
        applyDrawable(*geode.getDrawable(i));
    }
}

void Optimizer::StaticObjectDetectionVisitor::applyStateSet(osg::StateSet& stateset)
{
    stateset.computeDataVariance();
}


void Optimizer::StaticObjectDetectionVisitor::applyDrawable(osg::Drawable& drawable)
{

    if (drawable.getStateSet()) applyStateSet(*drawable.getStateSet());

    drawable.computeDataVariance();
}



////////////////////////////////////////////////////////////////////////////
// FlattenStaticTransformsDuplicatingSharedSubgraphsVisitor
////////////////////////////////////////////////////////////////////////////

void Optimizer::FlattenStaticTransformsDuplicatingSharedSubgraphsVisitor::reset()
{
    _matrixStack.clear();
}

void Optimizer::FlattenStaticTransformsDuplicatingSharedSubgraphsVisitor::apply(osg::Group& group)
{
    // only continue if there is a parent
    const unsigned int nodepathsize = _nodePath.size();
    if(!_matrixStack.empty() && group.getNumParents() > 1 && nodepathsize > 1)
    {
        // copy this Group
        osg::ref_ptr<osg::Object> new_obj = group.clone(osg::CopyOp::DEEP_COPY_NODES | osg::CopyOp::DEEP_COPY_DRAWABLES | osg::CopyOp::DEEP_COPY_ARRAYS);
        osg::Group* new_group = dynamic_cast<osg::Group*>(new_obj.get());

        // New Group should only be added to parent through which this Group
        // was traversed, not to all parents of this Group.
        osg::Group* parent_group = dynamic_cast<osg::Group*>(_nodePath[nodepathsize-2]);
        if(parent_group)
        {
            parent_group->replaceChild(&group, new_group);
            // also replace the node in the nodepath
            _nodePath[nodepathsize-1] = new_group;
            // traverse the new Group
            traverse(*(new_group));
        }
        else
        {
            OSG_NOTICE << "No parent for this Group" << std::endl;
        }
    }
    else
    {
        // traverse original node
        traverse(group);
    }
}


void Optimizer::FlattenStaticTransformsDuplicatingSharedSubgraphsVisitor::apply(osg::Transform& transform)
{
    bool pushed = false;

    // only continue if there is a parent and this is a STATIC transform
    const unsigned int nodepathsize = _nodePath.size();
    if(transform.getDataVariance() == osg::Object::STATIC && nodepathsize > 1)
    {
        osg::Matrix matrix;
        if(!_matrixStack.empty())
            matrix = _matrixStack.back();
        transform.computeLocalToWorldMatrix(matrix, this);
        _matrixStack.push_back(matrix);
        pushed = true;

        // convert this Transform to a Group
        osg::ref_ptr<osg::Group> group = new osg::Group(dynamic_cast<osg::Group&>(transform),
            osg::CopyOp::DEEP_COPY_NODES | osg::CopyOp::DEEP_COPY_DRAWABLES | osg::CopyOp::DEEP_COPY_ARRAYS);

        // New Group should only be added to parent through which this Transform
        // was traversed, not to all parents of this Transform.
        osg::Group* parent_group = dynamic_cast<osg::Group*>(_nodePath[nodepathsize-2]);
        if(parent_group)
        {
            parent_group->replaceChild(&transform, group.get());
            // also replace the node in the nodepath
            _nodePath[nodepathsize-1] = group.get();
            // traverse the new Group
            traverse(*(group.get()));
        }
        else
        {
            OSG_NOTICE << "No parent for this Group" << std::endl;
        }
    }
    else
    {
        // traverse original node
        traverse(transform);
    }

    // pop matrix off of stack
    if(pushed)
        _matrixStack.pop_back();
}


void Optimizer::FlattenStaticTransformsDuplicatingSharedSubgraphsVisitor::apply(osg::LOD& lod)
{
    const unsigned int nodepathsize = _nodePath.size();
    if(!_matrixStack.empty() && lod.getNumParents() > 1 && nodepathsize > 1)
    {
        osg::ref_ptr<osg::LOD> new_lod = new osg::LOD(lod,
            osg::CopyOp::DEEP_COPY_NODES | osg::CopyOp::DEEP_COPY_DRAWABLES | osg::CopyOp::DEEP_COPY_ARRAYS);

        // New LOD should only be added to parent through which this LOD
        // was traversed, not to all parents of this LOD.
        osg::Group* parent_group = dynamic_cast<osg::Group*>(_nodePath[nodepathsize-2]);
        if(parent_group)
        {
            parent_group->replaceChild(&lod, new_lod.get());
            // also replace the node in the nodepath
            _nodePath[nodepathsize-1] = new_lod.get();
            // move center point
            if(!_matrixStack.empty())
                new_lod->setCenter(new_lod->getCenter() * _matrixStack.back());

            // traverse the new Group
            traverse(*(new_lod.get()));
        }
        else
            OSG_NOTICE << "No parent for this LOD" << std::endl;
    }
    else
    {
        // move center point
        if(!_matrixStack.empty())
            lod.setCenter(lod.getCenter() * _matrixStack.back());

        traverse(lod);
    }
}


void Optimizer::FlattenStaticTransformsDuplicatingSharedSubgraphsVisitor::apply(osg::Geode& geode)
{
    if(!_matrixStack.empty())
    {
        // If there is only one parent, just transform all vertices and normals
        if(geode.getNumParents() == 1)
        {
            transformGeode(geode);
        }
        else
        {
            // Else make a copy and then transform
            const unsigned int nodepathsize = _nodePath.size();
            if(nodepathsize > 1)
            {
                // convert this Transform to a Group
                osg::ref_ptr<osg::Geode> new_geode = new osg::Geode(geode,
                    osg::CopyOp::DEEP_COPY_NODES | osg::CopyOp::DEEP_COPY_DRAWABLES | osg::CopyOp::DEEP_COPY_ARRAYS);

                // New Group should only be added to parent through which this Transform
                // was traversed, not to all parents of this Transform.
                osg::Group* parent_group = dynamic_cast<osg::Group*>(_nodePath[nodepathsize-2]);
                if(parent_group)
                    parent_group->replaceChild(&geode, new_geode.get());
                else
                    OSG_NOTICE << "No parent for this Geode" << std::endl;

                transformGeode(*(new_geode.get()));
            }
        }
    }
}


void Optimizer::FlattenStaticTransformsDuplicatingSharedSubgraphsVisitor::apply(osg::Billboard& billboard)
{
    if(!_matrixStack.empty())
    {
        // If there is only one parent, just transform this Billboard
        if(billboard.getNumParents() == 1)
        {
            transformBillboard(billboard);
        }
        else
        {
            // Else make a copy and then transform
            const unsigned int nodepathsize = _nodePath.size();
            if(nodepathsize > 1)
            {
                // convert this Transform to a Group
                osg::ref_ptr<osg::Billboard> new_billboard = new osg::Billboard(billboard,
                    osg::CopyOp::DEEP_COPY_NODES | osg::CopyOp::DEEP_COPY_DRAWABLES | osg::CopyOp::DEEP_COPY_ARRAYS);

                // New Billboard should only be added to parent through which this Billboard
                // was traversed, not to all parents of this Billboard.
                osg::Group* parent_group = dynamic_cast<osg::Group*>(_nodePath[nodepathsize-2]);
                if(parent_group)
                    parent_group->replaceChild(&billboard, new_billboard.get());
                else
                    OSG_NOTICE << "No parent for this Billboard" << std::endl;

                transformBillboard(*(new_billboard.get()));
            }
        }
    }
}


void Optimizer::FlattenStaticTransformsDuplicatingSharedSubgraphsVisitor::transformGeode(osg::Geode& geode)
{
    for(unsigned int i=0; i<geode.getNumDrawables(); i++)
    {
        transformDrawable(*geode.getDrawable(i));
    }

    geode.dirtyBound();
}


void Optimizer::FlattenStaticTransformsDuplicatingSharedSubgraphsVisitor::transformDrawable(osg::Drawable& drawable)
{
    osg::Geometry* geometry = drawable.asGeometry();
    if(geometry)
    {
        // transform all geometry
        osg::Vec3Array* verts = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
        if(verts)
        {
            for(unsigned int j=0; j<verts->size(); j++)
            {
                (*verts)[j] = (*verts)[j] * _matrixStack.back();
            }
        }
        else
        {
            osg::Vec4Array* verts = dynamic_cast<osg::Vec4Array*>(geometry->getVertexArray());
            if(verts)
            {
                for(unsigned int j=0; j<verts->size(); j++)
                {
                    (*verts)[j] = _matrixStack.back() * (*verts)[j];
                }
            }
        }
        osg::Vec3Array* normals = dynamic_cast<osg::Vec3Array*>(geometry->getNormalArray());
        if(normals)
        {
            for(unsigned int j=0; j<normals->size(); j++)
                (*normals)[j] = osg::Matrix::transform3x3((*normals)[j], _matrixStack.back());
        }

        geometry->dirtyBound();
        geometry->dirtyDisplayList();
    }
}


void Optimizer::FlattenStaticTransformsDuplicatingSharedSubgraphsVisitor::transformBillboard(osg::Billboard& billboard)
{
    osg::Vec3 axis = osg::Matrix::transform3x3(billboard.getAxis(), _matrixStack.back());
    axis.normalize();
    billboard.setAxis(axis);

    osg::Vec3 normal = osg::Matrix::transform3x3(billboard.getNormal(), _matrixStack.back());
    normal.normalize();
    billboard.setNormal(normal);

    for(unsigned int i=0; i<billboard.getNumDrawables(); i++)
    {
        osg::Vec3 originalBillboardPosition = billboard.getPosition(i);
        billboard.setPosition(i, originalBillboardPosition * _matrixStack.back());

        osg::Matrix matrixForDrawable = _matrixStack.back();
        matrixForDrawable.preMult(osg::Matrix::translate(originalBillboardPosition));
        matrixForDrawable.postMult(osg::Matrix::translate(-billboard.getPosition(i)));

        _matrixStack.push_back(matrixForDrawable);
        transformDrawable(*billboard.getDrawable(i));
        _matrixStack.pop_back();
    }

    billboard.dirtyBound();
}



