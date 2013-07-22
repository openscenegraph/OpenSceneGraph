/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the SCEA Shared Source License, Version 1.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain a copy of the License at:
 * http://research.scea.com/scea_shared_source_license.html
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing permissions and limitations under the
 * License.
 */

#include "daeWriter.h"
#include <osgAnimation/RigGeometry>

#include <dom/domCOLLADA.h>
#include <dom/domNode.h>
#include <dom/domLibrary_geometries.h>
#include <dom/domSource.h>
#include <dom/domGeometry.h>
#include <dom/domConstants.h>
//#include <dom/domSkin.h>

#include <sstream>

using namespace osgDAE;


unsigned int daeWriter::ArrayNIndices::getDAESize()
{
    switch( mode )
    {
    case VEC2F:
    case VEC2D:
        return 2;
    case VEC3F:
    case VEC3D:
        return 3;
    case VEC4F:
    case VEC4D:
    case VEC4_UB:
        return 4;
    case NONE:
        return 0;
    }
    return 0;
}

/// Appends an OSG vector (Vec2, Vec3...) to a domListOfFloats.
template<class VecType>
inline void append(domListOfFloats & list, const VecType & vec)
{
    for(unsigned int i=0; i<VecType::num_components; ++i) list.append( vec[i] );
}

/// Appends an OSG vector array (Vec2Array, Vec3Array...) to a domListOfFloats.
bool daeWriter::ArrayNIndices::append(domListOfFloats & list)
{
    switch(getMode())
    {
    case VEC2F:
        for (osg::Vec2Array::const_iterator it=vec2->begin(), itEnd=vec2->end(); it!=itEnd; ++it) ::append<osg::Vec2>(list, *it);
        break;
    case VEC2D:
        for (osg::Vec2dArray::const_iterator it=vec2d->begin(), itEnd=vec2d->end(); it!=itEnd; ++it) ::append<osg::Vec2d>(list, *it);
        break;
    case VEC3F:
        for (osg::Vec3Array::const_iterator it=vec3->begin(), itEnd=vec3->end(); it!=itEnd; ++it) ::append<osg::Vec3>(list, *it);
        break;
    case VEC3D:
        for (osg::Vec3dArray::const_iterator it=vec3d->begin(), itEnd=vec3d->end(); it!=itEnd; ++it) ::append<osg::Vec3d>(list, *it);
        break;
    case VEC4F:
        for (osg::Vec4Array::const_iterator it=vec4->begin(), itEnd=vec4->end(); it!=itEnd; ++it) ::append<osg::Vec4>(list, *it);
        break;
    case VEC4D:
        for (osg::Vec4dArray::const_iterator it=vec4d->begin(), itEnd=vec4d->end(); it!=itEnd; ++it) ::append<osg::Vec4d>(list, *it);
        break;
    case VEC4_UB:
        for (osg::Vec4ubArray::const_iterator it=vec4ub->begin(), itEnd=vec4ub->end(); it!=itEnd; ++it) ::append<osg::Vec4ub>(list, *it);
        break;
    default:
        return false;
    }
    return true;
}





domGeometry* daeWriter::getOrCreateDomGeometry(osg::Geometry* pOsgGeometry)
{
    // See if geometry exists in cache
    OsgGeometryDomGeometryMap::iterator iter = geometryMap.find( pOsgGeometry );
    if ( iter != geometryMap.end() )
    {
        return iter->second;
    }
    else
    {
        if (!lib_geoms)
        {
            lib_geoms = daeSafeCast< domLibrary_geometries >( dom->add( COLLADA_ELEMENT_LIBRARY_GEOMETRIES ) );
        }
        domGeometry* pDomGeometry = daeSafeCast< domGeometry >( lib_geoms->add( COLLADA_ELEMENT_GEOMETRY ) );

        std::string name( pOsgGeometry->getName() );
        if (name.empty())
            name = uniquify("geometry");
        else
            name = uniquify(name);
        pDomGeometry->setId( name.c_str() );
    #ifndef EARTH_GEO
        geometryMap.insert( std::make_pair( pOsgGeometry, pDomGeometry ) );
    #endif

        if ( !processGeometry( pOsgGeometry, pDomGeometry, name ) )
        {
            daeElement::removeFromParent( pDomGeometry );
            return NULL;
        }
        return pDomGeometry;
    }
}

void daeWriter::writeRigGeometry(osgAnimation::RigGeometry *pOsgRigGeometry)
{
    // See if controller exists in cache
    OsgRigGeometryDomControllerMap::iterator iter = _osgRigGeometryDomControllerMap.find(pOsgRigGeometry);
    domController* pDomController = NULL;
    if ( iter != _osgRigGeometryDomControllerMap.end() )
    {
        pDomController = iter->second;
    }
    else
    {
        domGeometry* pDomGeometry = getOrCreateDomGeometry(pOsgRigGeometry);
        if (pDomGeometry)
        {
            if (!lib_controllers)
            {
                lib_controllers = daeSafeCast< domLibrary_controllers >( dom->add( COLLADA_ELEMENT_LIBRARY_CONTROLLERS ) );
            }

            // <controller>
            // 1 <skin>
            //   source
            //   0..1    <bind_shape_matrix>
            //   3..*    <source>
            //   1        <joints>
            //   1      <vertex_weights>
            //   0..1    <extra>
            pDomController = daeSafeCast< domController >( lib_controllers->add( COLLADA_ELEMENT_CONTROLLER) );
            std::string name( pOsgRigGeometry->getName() );
            if (name.empty())
                name = uniquify("skincontroller");
            else
                name = uniquify(name);
            pDomController->setId( name.c_str() );
            _osgRigGeometryDomControllerMap.insert( std::make_pair( pOsgRigGeometry, pDomController ) );

            // Link <skin> to cache hit or created <geometry>
            domSkin* pDomSkin = daeSafeCast< domSkin >(pDomController->add( COLLADA_ELEMENT_SKIN ));
            std::string url = "#" + std::string(pDomGeometry->getId());
            pDomSkin->setSource(url.c_str());


            domSource* pDomJointsSource = daeSafeCast< domSource >(pDomSkin->add( COLLADA_ELEMENT_SOURCE ));
            std::string skinJointsName = name + "_skin_joints";
            pDomJointsSource->setId(skinJointsName.c_str());

            domListOfNames jointNames; // TODO fill with joint ids
            int size = 0; // TODO number of animated joints

            osgAnimation::VertexInfluenceMap* vim = pOsgRigGeometry->getInfluenceMap();
            osgAnimation::VertexInfluenceMap::iterator iter =    vim->begin();
            while (iter != vim->end())
            {
                jointNames.append(iter->first.c_str());
                //iter->second.getn
                ++iter;
            }

            domName_array* pDomJointsNameArray = daeSafeCast< domName_array >(pDomJointsSource->add(COLLADA_ELEMENT_NAME_ARRAY));
            std::string jointsNameArrayName = name + "_joints_array";
            pDomJointsNameArray->setId(jointsNameArrayName.c_str());
            pDomJointsNameArray->setCount(size);
            pDomJointsNameArray->setValue(jointNames);
            {
                domSource::domTechnique_common* pDomSourceTechniqueCommon = daeSafeCast< domSource::domTechnique_common >(pDomJointsSource->add(COLLADA_ELEMENT_TECHNIQUE_COMMON));

                domAccessor* pDomAccessor = daeSafeCast< domAccessor >(pDomSourceTechniqueCommon->add(COLLADA_ELEMENT_ACCESSOR));
                std::string url = "#" + jointsNameArrayName;
                pDomAccessor->setSource(url.c_str());
                pDomAccessor->setCount(size);

                domParam* pDomParam = daeSafeCast< domParam >(pDomAccessor->add(COLLADA_ELEMENT_PARAM));
                pDomParam->setType(COLLADA_TYPE_NAME);
            }

            domSource* pDomSkinBindPoseSource = daeSafeCast< domSource >(pDomSkin->add( COLLADA_ELEMENT_SOURCE ));
            std::string skinBindPoseName = name + "_skin_bind_pose";
            pDomSkinBindPoseSource->setId(skinBindPoseName.c_str());

            domListOfFloats matrices; // TODO fill with bind matrices
            int numMatrices = 0; // TODO number of bind matrices
            domFloat_array* pDomMatricesArray = daeSafeCast< domFloat_array >(pDomSkinBindPoseSource->add(COLLADA_ELEMENT_FLOAT_ARRAY));
            std::string matricesArrayName = name + "_matrices_array";
            pDomMatricesArray->setId(matricesArrayName.c_str());
            pDomMatricesArray->setCount(numMatrices);
            pDomMatricesArray->setValue(matrices);
            {
                domSource::domTechnique_common* pDomSourceTechniqueCommon = daeSafeCast< domSource::domTechnique_common >(pDomSkinBindPoseSource->add(COLLADA_ELEMENT_TECHNIQUE_COMMON));

                domAccessor* pDomAccessor = daeSafeCast< domAccessor >(pDomSourceTechniqueCommon->add(COLLADA_ELEMENT_ACCESSOR));
                std::string url = "#" + matricesArrayName;
                pDomAccessor->setSource(url.c_str());
                pDomAccessor->setCount(size);
                pDomAccessor->setStride(16);

                domParam* pDomParam = daeSafeCast< domParam >(pDomAccessor->add(COLLADA_ELEMENT_PARAM));
                pDomParam->setType(COLLADA_TYPE_FLOAT4X4);
            }

            domSource* pDomSkinWeightsSource = daeSafeCast< domSource >(pDomSkin->add( COLLADA_ELEMENT_SOURCE ));
            std::string skinWeightsName = name + "_skin_weights";
            pDomSkinWeightsSource->setId(skinWeightsName.c_str());

            domListOfFloats weights; // TODO fill with vertex weights
            int numWeights = 0; // TODO number of vertices vertex weights
            domFloat_array* pDomWeightsArray = daeSafeCast< domFloat_array >(pDomSkinWeightsSource->add(COLLADA_ELEMENT_FLOAT_ARRAY));
            std::string weightsArrayName = name + "_weights_array";
            pDomWeightsArray->setId(weightsArrayName.c_str());
            pDomWeightsArray->setCount(numWeights);
            pDomWeightsArray->setValue(weights);
            {
                domSource::domTechnique_common* pDomSourceTechniqueCommon = daeSafeCast< domSource::domTechnique_common >(pDomSkinWeightsSource->add(COLLADA_ELEMENT_TECHNIQUE_COMMON));

                domAccessor* pDomAccessor = daeSafeCast< domAccessor >(pDomSourceTechniqueCommon->add(COLLADA_ELEMENT_ACCESSOR));
                std::string url = "#" + weightsArrayName;
                pDomAccessor->setSource(url.c_str());
                pDomAccessor->setCount(size);

                domParam* pDomParam = daeSafeCast< domParam >(pDomAccessor->add(COLLADA_ELEMENT_PARAM));
                pDomParam->setType(COLLADA_TYPE_FLOAT);
            }

            domSkin::domJoints* pDomJoints = daeSafeCast< domSkin::domJoints >(pDomSkin->add( COLLADA_ELEMENT_JOINTS ));

            domInputLocal* pDomInput = daeSafeCast< domInputLocal >(pDomJoints->add(COLLADA_ELEMENT_INPUT));
            pDomInput->setSemantic(COMMON_PROFILE_INPUT_JOINT);
            url = "#" + skinJointsName;
            pDomInput->setSource(url.c_str());

            pDomInput = daeSafeCast< domInputLocal >(pDomJoints->add(COLLADA_ELEMENT_INPUT));
            pDomInput->setSemantic(COMMON_PROFILE_INPUT_INV_BIND_MATRIX);
            url = "#" + skinBindPoseName;
            pDomInput->setSource(url.c_str());

            domSkin::domVertex_weights* pDomVertexWeights = daeSafeCast< domSkin::domVertex_weights >(pDomSkin->add( COLLADA_ELEMENT_VERTEX_WEIGHTS ));
            pDomVertexWeights->setCount(0);// TODO set number of vertex weights

            domInputLocalOffset* pDomInputLocalOffset = daeSafeCast< domInputLocalOffset >(pDomVertexWeights->add(COLLADA_ELEMENT_INPUT));
            pDomInputLocalOffset->setSemantic(COMMON_PROFILE_INPUT_JOINT);
            url = "#" + skinJointsName;
            pDomInputLocalOffset->setSource(url.c_str());
            pDomInputLocalOffset->setOffset(0);

            pDomInputLocalOffset = daeSafeCast< domInputLocalOffset >(pDomVertexWeights->add(COLLADA_ELEMENT_INPUT));
            pDomInputLocalOffset->setSemantic(COMMON_PROFILE_INPUT_WEIGHT);
            url = "#" + weightsArrayName;
            pDomInputLocalOffset->setSource(url.c_str());
            pDomInputLocalOffset->setOffset(1);

            domSkin::domVertex_weights::domVcount* pDomVcount = daeSafeCast< domSkin::domVertex_weights::domVcount >(pDomVertexWeights->add(COLLADA_ELEMENT_VCOUNT));
            domListOfUInts valueCounts;
            // TODO
            pDomVcount->setValue(valueCounts);
            domSkin::domVertex_weights::domV* pDomV = daeSafeCast< domSkin::domVertex_weights::domV >(pDomVertexWeights->add(COLLADA_ELEMENT_V));
            domListOfInts values;
            //TODO
            pDomV->setValue(values);
        }
    }

    if (pDomController)
    {
        // Link <instance_controller> to cache hit or created <controller>
        domInstance_controller* pDomInstanceController = daeSafeCast< domInstance_controller >( currentNode->add( COLLADA_ELEMENT_INSTANCE_CONTROLLER ) );
        std::string url = "#" + std::string(pDomController->getId());
        pDomInstanceController->setUrl( url.c_str() );
    }
}

void daeWriter::writeMorphGeometry(osgAnimation::MorphGeometry *pOsgMorphGeometry)
{
    // See if controller exists in cache
    OsgMorphGeometryDomControllerMap::iterator iter = _osgMorphGeometryDomControllerMap.find(pOsgMorphGeometry);
    domController* pDomController = NULL;
    if ( iter != _osgMorphGeometryDomControllerMap.end() )
    {
        pDomController = iter->second;
    }
    else
    {
        domGeometry* pDomGeometry = getOrCreateDomGeometry(pOsgMorphGeometry);
        if (pDomGeometry)
        {
            if (!lib_controllers)
            {
                lib_controllers = daeSafeCast< domLibrary_controllers >( dom->add( COLLADA_ELEMENT_LIBRARY_CONTROLLERS ) );
            }

            // <controller>
            // 1 <morph source (method)>
            //     2..*    <source>
            //   1        <targets>
            //        2..*    <input semantic source>
            //        0..*    <extra>
            //   0..* <extra>
            pDomController = daeSafeCast< domController >( lib_controllers->add( COLLADA_ELEMENT_CONTROLLER) );
            std::string name( pOsgMorphGeometry->getName() );
            if (name.empty())
                name = uniquify("morphcontroller");
            else
                name = uniquify(name);
            pDomController->setId( name.c_str() );
            _osgMorphGeometryDomControllerMap.insert( std::make_pair( pOsgMorphGeometry, pDomController ) );

            // Link <morph> to cache hit or created <geometry>
            domMorph* pDomMorph = daeSafeCast< domMorph >(pDomController->add( COLLADA_ELEMENT_MORPH ));
            std::string url = "#" + std::string(pDomGeometry->getId());
            pDomMorph->setSource(url.c_str());
            pDomMorph->setMethod(MORPHMETHODTYPE_NORMALIZED);
            //pDomMorph->setMethod(MORPHMETHODTYPE_RELATIVE);

            domSource* pDomTargetsSource = daeSafeCast< domSource >(pDomMorph->add( COLLADA_ELEMENT_SOURCE ));
            std::string targetsName = name + "_morph_targets";
            pDomTargetsSource->setId(targetsName.c_str());

            domIDREF_array* pDomIDREFArray = daeSafeCast< domIDREF_array >(pDomTargetsSource->add(COLLADA_ELEMENT_IDREF_ARRAY));
            xsIDREFS idrefs;
            osgAnimation::MorphGeometry::MorphTargetList morphTargetList = pOsgMorphGeometry->getMorphTargetList();
            for (unsigned int i=0; i < morphTargetList.size(); i++)
            {
                domGeometry* pDomGeometry = getOrCreateDomGeometry(morphTargetList[i].getGeometry());
                idrefs.append(pDomGeometry->getId());
            }
            pDomIDREFArray->setValue(idrefs);
            std::string targetsArrayName = targetsName + "_array";
            pDomIDREFArray->setId(targetsArrayName.c_str());
            pDomIDREFArray->setCount(morphTargetList.size());

            domSource::domTechnique_common* pDomTechniqueCommon = daeSafeCast< domSource::domTechnique_common >(pDomTargetsSource->add(COLLADA_ELEMENT_TECHNIQUE_COMMON));
            domAccessor* pDomAccessor = daeSafeCast< domAccessor >(pDomTechniqueCommon->add(COLLADA_ELEMENT_ACCESSOR));
            pDomAccessor->setCount(morphTargetList.size());
            url = "#" + targetsArrayName;
            pDomAccessor->setSource(url.c_str());

            domParam* pDomParam = daeSafeCast< domParam >(pDomAccessor->add(COLLADA_ELEMENT_PARAM));
            pDomParam->setName(COMMON_PROFILE_INPUT_MORPH_TARGET);
            pDomParam->setType("IDREF"); // COLLADA_TYPE_IDREF does not exist

            domSource* pDomWeightsSource = daeSafeCast< domSource >(pDomMorph->add( COLLADA_ELEMENT_SOURCE ));
            std::string weightsName = name + "_morph_weights";
            pDomWeightsSource->setId(weightsName.c_str());

            domFloat_array* pDomFloatArray = daeSafeCast< domFloat_array >(pDomWeightsSource->add(COLLADA_ELEMENT_FLOAT_ARRAY));
            domListOfFloats weights;
            for (unsigned int i=0; i < morphTargetList.size(); i++)
            {
                weights.append(morphTargetList[i].getWeight());
            }
            pDomFloatArray->setValue(weights);
            std::string weigthsArrayName = weightsName + "_array";
            pDomFloatArray->setId(weigthsArrayName.c_str());
            pDomFloatArray->setCount(morphTargetList.size());

            pDomTechniqueCommon = daeSafeCast< domSource::domTechnique_common >(pDomWeightsSource->add(COLLADA_ELEMENT_TECHNIQUE_COMMON));
            pDomAccessor = daeSafeCast< domAccessor >(pDomTechniqueCommon->add(COLLADA_ELEMENT_ACCESSOR));
            pDomAccessor->setCount(morphTargetList.size());
            url = "#" + weightsName;
            pDomAccessor->setSource(url.c_str());

            pDomParam = daeSafeCast< domParam >(pDomAccessor->add(COLLADA_ELEMENT_PARAM));
            pDomParam->setName(COMMON_PROFILE_INPUT_MORPH_WEIGHT);
            pDomParam->setType(COLLADA_TYPE_FLOAT);

            domMorph::domTargets* pDomTargets = daeSafeCast< domMorph::domTargets >(pDomMorph->add( COLLADA_ELEMENT_TARGETS ));

            domInputLocal* pDomTargetsInput = daeSafeCast< domInputLocal >(pDomTargets->add( COLLADA_ELEMENT_INPUT ));
            pDomTargetsInput->setSemantic(COMMON_PROFILE_INPUT_MORPH_TARGET);
            url = "#" + targetsName;
            pDomTargetsInput->setSource(url.c_str());

            domInputLocal* pDomWeightsInput = daeSafeCast< domInputLocal >(pDomTargets->add( COLLADA_ELEMENT_INPUT ));
            pDomWeightsInput->setSemantic(COMMON_PROFILE_INPUT_MORPH_WEIGHT);
            url = "#" + weightsName;
            pDomWeightsInput->setSource(url.c_str());
        }
    }

    if (pDomController)
    {
        // Transparency at drawable level
        if (pOsgMorphGeometry->getStateSet())
            m_CurrentRenderingHint = pOsgMorphGeometry->getStateSet()->getRenderingHint();

        pushStateSet(pOsgMorphGeometry->getStateSet());

        // Link <instance_controller> to cache hit or created <controller>
        domInstance_controller* pDomInstanceController = daeSafeCast< domInstance_controller >( currentNode->add( COLLADA_ELEMENT_INSTANCE_CONTROLLER ) );
        std::string url = "#" + std::string(pDomController->getId());
        pDomInstanceController->setUrl( url.c_str() );

        if (!stateSetStack.empty())
        {
            domBind_material *pDomBindMaterial = daeSafeCast< domBind_material >( pDomInstanceController->add( COLLADA_ELEMENT_BIND_MATERIAL ) );
            processMaterial( currentStateSet.get(), pDomBindMaterial, pOsgMorphGeometry->getName() );
        }

        popStateSet(pOsgMorphGeometry->getStateSet());
    }
}

void daeWriter::apply( osg::Geode &node )
{
    debugPrint( node );
    updateCurrentDaeNode();

    pushStateSet(node.getStateSet());
    if (NULL != node.getStateSet())
        m_CurrentRenderingHint = node.getStateSet()->getRenderingHint();

    // TODO
    // Write a Geode as a single instance_geometry if all drawables use the same vertex streams
    // Reuse an existing Geode if only statesets differ
    unsigned int count = node.getNumDrawables();
    for ( unsigned int i = 0; i < count; i++ )
    {
        osg::Geometry *g = node.getDrawable( i )->asGeometry();

        if ( g != NULL )
        {
            osgAnimation::RigGeometry *pOsgRigGeometry = dynamic_cast<osgAnimation::RigGeometry*>(g);
            if (pOsgRigGeometry)
            {
                writeRigGeometry(pOsgRigGeometry);
            }
            else
            {
                osgAnimation::MorphGeometry *pOsgMorphGeometry = dynamic_cast<osgAnimation::MorphGeometry*>(g);
                if (pOsgMorphGeometry)
                {
                    writeMorphGeometry(pOsgMorphGeometry);
                }
                else
                {
                    // Write a default osg::Geometry

                    // Transparency at drawable level
                    if (NULL != g->getStateSet())
                        m_CurrentRenderingHint = g->getStateSet()->getRenderingHint();

                    pushStateSet(g->getStateSet());

                    domGeometry* pDomGeometry = getOrCreateDomGeometry(g);
                    if (pDomGeometry)
                    {
                        // Link <instance_geometry> to cache hit or created <geometry>
                        domInstance_geometry *pDomInstanceGeometry = daeSafeCast< domInstance_geometry >( currentNode->add( COLLADA_ELEMENT_INSTANCE_GEOMETRY ) );
                        std::string url = "#" + std::string(pDomGeometry->getId());
                        pDomInstanceGeometry->setUrl( url.c_str() );

                        if (!stateSetStack.empty())
                        {
                            domBind_material *pDomBindMaterial = daeSafeCast< domBind_material >( pDomInstanceGeometry->add( COLLADA_ELEMENT_BIND_MATERIAL ) );
                            processMaterial( currentStateSet.get(), pDomBindMaterial, pDomGeometry->getId() );
                        }
                    }

                    popStateSet(g->getStateSet());
                }
            }
        }
        else
        {
            OSG_WARN << "Non-geometry drawables are not supported" << std::endl;
        }
    }

    popStateSet(node.getStateSet());
}

/** append elements (verts, normals, colors and texcoord) for file write */
void daeWriter::appendGeometryIndices(osg::Geometry *geom,
                    domP * p,
                    unsigned int vindex,
                    domSource * norm,
                    domSource * color,
                    const ArrayNIndices & verts,
                    const ArrayNIndices & normals,
                    const ArrayNIndices & colors,
                    const std::vector<ArrayNIndices> & texcoords,
                    unsigned int  ncount,
                    unsigned int  ccount)
{
  p->getValue().append( verts.inds!=NULL?verts.inds->index( vindex ):vindex );

  if ( norm != NULL )
  {
    if ( osg::getBinding(geom->getNormalArray()) == osg::Array::BIND_PER_VERTEX )
      p->getValue().append( normals.inds!=NULL?normals.inds->index( vindex ):vindex );
    else
      p->getValue().append( normals.inds!=NULL?normals.inds->index( ncount ):ncount );
  }

  if ( color != NULL )
  {
    if ( osg::getBinding(geom->getColorArray()) == osg::Array::BIND_PER_VERTEX )
      p->getValue().append( colors.inds!=NULL?colors.inds->index( vindex ):vindex );
    else
      p->getValue().append( colors.inds!=NULL?colors.inds->index( ccount ):ccount );
  }

  for ( unsigned int ti = 0; ti < texcoords.size(); ti++ )
  {
    //ArrayNIndices &tc = texcoords[ti];
    p->getValue().append( texcoords[ti].inds!=NULL?texcoords[ti].inds->index(vindex):vindex );
  }

}


bool daeWriter::processGeometry( osg::Geometry *geom, domGeometry *geo, const std::string &name )
{
    domMesh *mesh = daeSafeCast< domMesh >( geo->add( COLLADA_ELEMENT_MESH ) );
    domSource *pos = NULL;
    domSource *norm = NULL;
    domSource *color = NULL;
    std::vector< domSource * >texcoord;
    std::vector< domSource * > vertexAttribute;
    domLines *lines = NULL;
    domLinestrips *linestrips = NULL;
    domTriangles *tris = NULL;
    domTristrips *tristrips = NULL;
    domTrifans *trifans = NULL;
    domPolygons *polys = NULL;
    domPolylist *polylist = NULL;

    // make sure no deprecated indices or binding exist
    if (geom->containsDeprecatedData()) geom->fixDeprecatedData();

    ArrayNIndices verts( geom->getVertexArray(), 0 );
    ArrayNIndices normals( geom->getNormalArray(), 0 );
    ArrayNIndices colors( geom->getColorArray(), 0 );

    // RS BUG
    // getNumTexCoordArrays may return larger number
    // where getTexCoordArray(0) may have a BIND_OFF and an empty array
    std::vector<ArrayNIndices> texcoords;
    for ( unsigned int i = 0; i < geom->getNumTexCoordArrays(); i++ )
    {
        if (geom->getTexCoordArray(i))
        {
            texcoords.push_back( ArrayNIndices( geom->getTexCoordArray( i ), 0 ) );
        }
    }
    std::vector<ArrayNIndices> vertexAttributes;
    for ( unsigned int i = 0; i < geom->getNumVertexAttribArrays(); i++ )
    {
        if (geom->getVertexAttribArray(i))
        {
            vertexAttributes.push_back(ArrayNIndices( geom->getVertexAttribArray( i ), 0));
        }
    }

    // process POSITION
    std::string sName;
    {
        sName = name + "-positions";
        unsigned int elementSize = verts.getDAESize();
        unsigned int numElements = verts.valArray ? verts.valArray->getNumElements() : 0;
        pos = createSource( mesh, sName, elementSize );
        pos->getFloat_array()->setCount( numElements * elementSize );
        pos->getTechnique_common()->getAccessor()->setCount( numElements );
        if (!verts.append(pos->getFloat_array()->getValue()))
        {
            OSG_WARN << "Invalid array type for vertices" << std::endl;
        }

        //create a vertices element
        domVertices *vertices = daeSafeCast< domVertices >( mesh->add( COLLADA_ELEMENT_VERTICES ) );
        std::string vName = name + "-vertices";
        vertices->setId( vName.c_str() );

        //make a POSITION input in it
        domInputLocal *il = daeSafeCast< domInputLocal >( vertices->add( COLLADA_ELEMENT_INPUT ) );
        il->setSemantic( COMMON_PROFILE_INPUT_POSITION );
        std::string url("#" + std::string(pos->getId()) );
        il->setSource( url.c_str() );
    }

    //process NORMAL
    if ( normals.getMode() != ArrayNIndices::NONE )
    {
        sName = name + "-normals";
        unsigned int elementSize = normals.getDAESize();
        unsigned int numElements = normals.valArray ? normals.valArray->getNumElements() : 0;
        norm = createSource( mesh, sName, elementSize );
        norm->getFloat_array()->setCount( numElements * elementSize );
        norm->getTechnique_common()->getAccessor()->setCount( numElements );
        if (!normals.append(norm->getFloat_array()->getValue()))
        {
            OSG_WARN << "Invalid array type for normals" << std::endl;
        }

        //if NORMAL shares same indices as POSITION put it in the vertices
        /*if ( normalInds == vertInds && vertInds != NULL ) {
            il = daeSafeCast< domInputLocal >( vertices->add( COLLADA_ELEMENT_INPUT ) );
            il->setSemantic( COMMON_PROFILE_INPUT_NORMAL );
            url = "#" + std::string(md->norm->getId());
            il->setSource( url.c_str() );
        }*/
    }

    //process COLOR
    if ( colors.getMode() != ArrayNIndices::NONE )
    {
        sName = name + "-colors";
        unsigned int elementSize = colors.getDAESize();
        unsigned int numElements = colors.valArray ? colors.valArray->getNumElements() : 0;
        color = createSource( mesh, sName, elementSize, true );
        color->getFloat_array()->setCount( numElements * elementSize );
        color->getTechnique_common()->getAccessor()->setCount( numElements );
        if (!colors.append(color->getFloat_array()->getValue()))
        {
            OSG_WARN << "Invalid array type for colors" << std::endl;
        }

        //if COLOR shares same indices as POSITION put it in the vertices
        /*if ( colorInds == vertInds && vertInds != NULL ) {
            il = daeSafeCast< domInputLocal >( vertices->add( COLLADA_ELEMENT_INPUT ) );
            il->setSemantic( COMMON_PROFILE_INPUT_COLOR );
            url = "#" + std::string(md->color->getId());
            il->setSource( url.c_str() );
        }*/
    }

    //process TEXCOORD
    //TODO: Do the same as normal and colors for texcoods. But in a loop since you can have many
    for ( unsigned int ti = 0; ti < texcoords.size(); ti++ )
    {
        if (texcoords[ti].getMode() == ArrayNIndices::NONE) continue;

        std::ostringstream intstr;
        intstr << std::dec << ti;
        sName = name + "-texcoord_" + intstr.str();

        unsigned int elementSize = texcoords[ti].getDAESize();
        unsigned int numElements = texcoords[ti].valArray ? texcoords[ti].valArray->getNumElements() : 0;
        domSource *t = createSource( mesh, sName, elementSize, false, true );
        t->getFloat_array()->setCount( numElements * elementSize );
        t->getTechnique_common()->getAccessor()->setCount( numElements );
        if (!texcoords[ti].append(t->getFloat_array()->getValue()))
        {
            OSG_WARN << "Invalid array type for texcoord" << std::endl;
        }

        texcoord.push_back( t );
    }

    //RS
    //process VERTEX ATTRIBUTES
    //TODO: Do the same as normal and colors for texcoods. But in a loop since you can have many
    for ( unsigned int ti = 0; ti < vertexAttributes.size(); ti++ )
    {
        if (vertexAttributes[ti].getMode() == ArrayNIndices::NONE) continue;

        std::ostringstream intstr;
        intstr << std::dec << ti;
        sName = name + "-vertexAttribute_" + intstr.str();

        unsigned int elementSize = vertexAttributes[ti].getDAESize();
        unsigned int numElements = vertexAttributes[ti].valArray ? vertexAttributes[ti].valArray->getNumElements() : 0;
        domSource *t = createSource( mesh, sName, elementSize, false, true );        // Sukender: should we *REALLY* call createSource(... false, true)? (I mean with such flags)
        t->getFloat_array()->setCount( numElements * elementSize );
        t->getTechnique_common()->getAccessor()->setCount( numElements );
        if (!vertexAttributes[ti].append(t->getFloat_array()->getValue()))
        {
            OSG_WARN << "Invalid array type for vertex attribute" << ti << std::endl;
        }

        vertexAttribute.push_back( t );
    }

    //process each primitive group
    unsigned int ncount = 0; //Normal index counter
    unsigned int ccount = 0; //Color index counter
    if ( geom->getNumPrimitiveSets() == 0 )
    {
        OSG_WARN << "NO PRIMITIVE SET!!" << std::endl;
        return false;
    }
    bool valid = false;
    //for each primitive group
    for ( unsigned int i = 0; i < geom->getNumPrimitiveSets(); i++ )
    {
        osg::PrimitiveSet *ps = geom->getPrimitiveSet( i );
        GLenum mode = ps->getMode();
        unsigned int primLength;
        //unsigned int offset = 0;
        //domInputLocalOffset *ilo = NULL;

        //process primitive group
        switch( mode )
        {
            case GL_POINTS:
            {
                OSG_WARN << "Geometry contains points rendering. COLLADA does not" << std::endl;
                continue;
            }
            case GL_LINES:
            {
                if ( lines == NULL )
                {
                    lines = createPrimGroup<domLines>( COLLADA_ELEMENT_LINES, mesh, norm, color, texcoord );
                    lines->add( COLLADA_ELEMENT_P );
                    std::string mat = name + "_material";
                    lines->setMaterial( mat.c_str() );
                }
                primLength = 2;
                valid = true;
                break;
            }
            case GL_TRIANGLES:
            {
                if ( tris == NULL )
                {
                    tris = createPrimGroup<domTriangles>( COLLADA_ELEMENT_TRIANGLES, mesh, norm, color, texcoord );
                    tris->add( COLLADA_ELEMENT_P );
                    std::string mat = name + "_material";
                    tris->setMaterial( mat.c_str() );
                }
                primLength = 3;
                valid = true;
                break;
            }
            case GL_QUADS:
            {
                if ( polys == NULL )
                {
                    if (_pluginOptions.usePolygons)
                    {
                          polys = createPrimGroup<domPolygons>( COLLADA_ELEMENT_POLYGONS, mesh, norm, color, texcoord );
                          polys->add( COLLADA_ELEMENT_P );
                          std::string mat = name + "_material";
                          polys->setMaterial( mat.c_str() );
                    }
                    else
                    {
                          polylist = createPrimGroup<domPolylist>( COLLADA_ELEMENT_POLYLIST, mesh, norm, color, texcoord );

                          polylist->add( COLLADA_ELEMENT_VCOUNT );
                          polylist->add( COLLADA_ELEMENT_P );
                          std::string mat = name + "_material";
                          polylist->setMaterial( mat.c_str() );
                    }
                }
                primLength = 4;
                valid = true;
                break;
            }
            case GL_LINE_STRIP:
            {
                if ( linestrips == NULL )
                {
                    linestrips = createPrimGroup<domLinestrips>( COLLADA_ELEMENT_LINESTRIPS, mesh, norm, color, texcoord );
                    std::string mat = name + "_material";
                    linestrips->setMaterial( mat.c_str() );
                }
                primLength = 0;
                valid = true;
                break;
            }
            case GL_TRIANGLE_STRIP:
            {
                if ( tristrips == NULL )
                {
                    tristrips = createPrimGroup<domTristrips>( COLLADA_ELEMENT_TRISTRIPS, mesh, norm, color, texcoord );
                    std::string mat = name + "_material";
                    tristrips->setMaterial( mat.c_str() );
                }
                primLength = 0;
                valid = true;
                break;
            }
            case GL_TRIANGLE_FAN:
            {
                if ( trifans == NULL )
                {
                    trifans = createPrimGroup<domTrifans>( COLLADA_ELEMENT_TRIFANS, mesh, norm, color, texcoord );
                    std::string mat = name + "_material";
                    trifans->setMaterial( mat.c_str() );
                }
                primLength = 0;
                valid = true;
                break;
            }
            default:
            {
                if ( polys == NULL )
                {
                    if (_pluginOptions.usePolygons)
                    {
                        polys = createPrimGroup<domPolygons>( COLLADA_ELEMENT_POLYGONS, mesh, norm, color, texcoord );
                        polys->add( COLLADA_ELEMENT_P );
                        std::string mat = name + "_material";
                        polys->setMaterial( mat.c_str() );
                    }
                    else
                    {
                        polylist = createPrimGroup<domPolylist>( COLLADA_ELEMENT_POLYLIST, mesh, norm, color, texcoord );

                        polylist->add( COLLADA_ELEMENT_VCOUNT );
                        polylist->add( COLLADA_ELEMENT_P );
                        std::string mat = name + "_material";
                        polylist->setMaterial( mat.c_str() );
                    }
                }
                primLength = 0;
                valid = true;
                break; // compute later when =0.
            }
        }

                //process upon primitive set type
                // 1- set data source,accumulate count of primitives and write it to file
                // 2- read data source for primitive set and write it to file
        switch( ps->getType() )
        {
                //draw arrays (array of contiguous vertices)

                       //(primitive type+begin+end),(primitive type+begin+end)...
            case osg::PrimitiveSet::DrawArraysPrimitiveType:
            {
                //OSG_WARN << "DrawArrays" << std::endl;

                if ( primLength == 0 )
                {
                    primLength = ps->getNumIndices();
                }
                osg::DrawArrays* drawArray = static_cast< osg::DrawArrays* >( ps );
                unsigned int vcount = 0;
                unsigned int indexEnd = drawArray->getFirst() + drawArray->getCount();

                std::vector<domP *> p;
                switch ( mode )
                {
                    case GL_LINES:
                    {
                                                p.push_back(lines->getP());
                        lines->setCount( lines->getCount() + drawArray->getCount()/primLength );
                        break;
                    }
                    case GL_TRIANGLES:
                    {
                                                p.push_back(tris->getP());
                        tris->setCount( tris->getCount() + drawArray->getCount()/primLength );
                        break;
                    }
                    case GL_LINE_STRIP:
                    {
                                                p.push_back(daeSafeCast<domP>( linestrips->add( COLLADA_ELEMENT_P ) ));
                        linestrips->setCount( linestrips->getCount() + 1 );
                        break;
                    }
                    case GL_TRIANGLE_STRIP:
                    {
                                                p.push_back(daeSafeCast<domP>( tristrips->add( COLLADA_ELEMENT_P ) ));
                        tristrips->setCount( tristrips->getCount() + 1 );
                        break;
                    }
                    case GL_TRIANGLE_FAN:
                    {
                                                p.push_back(daeSafeCast<domP>( trifans->add( COLLADA_ELEMENT_P ) ));
                        trifans->setCount( trifans->getCount() + 1 );
                        break;
                    }
                    default:
                    {
                        //TODO : test this case
                        unsigned int nbPolygons=drawArray->getCount()/primLength;
                        if (_pluginOptions.usePolygons)
                        {
                            //for( unsigned int idx = 0; idx < nbPolygons; ++idx )
                            p.push_back(polys->getP_array()[0]);
                            polys->setCount( polys->getCount() + nbPolygons );
                        }
                        else
                        {
                            for( unsigned int idx = 0; idx < nbPolygons; ++idx )
                                    polylist->getVcount()->getValue().append( primLength );
                            p.push_back(polylist->getP());
                            polylist->setCount( polylist->getCount() + nbPolygons);
                        }
                        break;
                    }
                }

                unsigned int indexBegin = drawArray->getFirst();
                unsigned int nbVerticesPerPoly=(indexEnd-indexBegin)/p.size();
                unsigned int indexPolyEnd = indexBegin+nbVerticesPerPoly;
                for( unsigned int iPoly = 0; iPoly < p.size(); ++iPoly )
                {
                    for (unsigned int vindex=indexBegin; vindex< indexPolyEnd;vindex++)
                    {

                       appendGeometryIndices(geom,p[iPoly],vindex,
                                     norm,color,
                                     verts,normals,colors,texcoords,
                                     ncount,ccount);

                       vcount++;
                    }
                    indexBegin+=nbVerticesPerPoly;
                    indexPolyEnd+=nbVerticesPerPoly;
                }
                break;
            }
            //(primitive type) + (end1),(end2),(end3)...
            case osg::PrimitiveSet::DrawArrayLengthsPrimitiveType:
            {
                //OSG_WARN << "DrawArrayLengths" << std::endl;

                osg::DrawArrayLengths* drawArrayLengths = static_cast<osg::DrawArrayLengths*>( ps );

                unsigned int vindex = drawArrayLengths->getFirst();
                for( osg::DrawArrayLengths::iterator primItr = drawArrayLengths->begin();
                    primItr != drawArrayLengths->end();
                    ++primItr )
                {
                    unsigned int localPrimLength;
                    if ( primLength == 0 ) localPrimLength = *primItr;
                    else localPrimLength = primLength;

                    std::vector<domP *> p;
                    switch ( mode )
                    {
                        case GL_LINES:
                        {
                            p.push_back(lines->getP());
                            lines->setCount( lines->getCount() + (*primItr)/localPrimLength );
                            break;
                        }
                        case GL_TRIANGLES:
                        {
                            p.push_back(tris->getP());
                            tris->setCount( tris->getCount() + (*primItr)/localPrimLength );
                            break;
                        }
                        case GL_LINE_STRIP:
                        {
                            p.push_back(daeSafeCast<domP>( linestrips->add( COLLADA_ELEMENT_P ) ));
                            linestrips->setCount( linestrips->getCount() + 1 );
                            break;
                        }
                        case GL_TRIANGLE_STRIP:
                        {
                            p.push_back(daeSafeCast<domP>( tristrips->add( COLLADA_ELEMENT_P ) ));
                            tristrips->setCount( tristrips->getCount() + 1 );
                            break;
                        }
                        case GL_TRIANGLE_FAN:
                        {
                            p.push_back(daeSafeCast<domP>( trifans->add( COLLADA_ELEMENT_P ) ));
                            trifans->setCount( trifans->getCount() + 1 );
                            break;
                        }
                        default:
                        {

                            if (_pluginOptions.usePolygons)
                            {
                                //for( unsigned int idx = 0; idx < nbPolygons; ++idx )
                                p.push_back(polys->getP_array()[0]);
                                polys->setCount( polys->getCount() + 1 );
                            }
                            else
                            {
                                polylist->getVcount()->getValue().append( localPrimLength );
                                p.push_back(polylist->getP());
                                polylist->setCount( polylist->getCount() + 1);
                            }
                            break;
                        }
                    }

                    unsigned int indexBegin = 0;
                    unsigned int nbVerticesPerPoly=*primItr/p.size();
                    unsigned int indexEnd=indexBegin+nbVerticesPerPoly;
                    for( unsigned int iPoly = 0; iPoly < p.size(); ++iPoly )
                    {
                        // printf("indexBegin %d,indexPolyEnd %d \n",indexBegin,indexEnd);
                        for( unsigned int primCount = indexBegin; primCount < indexEnd; ++primCount )
                        {
                            appendGeometryIndices(geom,p[iPoly],vindex,
                                                  norm,color,
                                                  verts,normals,colors,texcoords,
                                                  ncount,ccount);

                            vindex++;
                        }
                        indexBegin+=nbVerticesPerPoly;
                        indexEnd+=nbVerticesPerPoly;
                     }
                }
                break;
            }

            //draw elements (array of shared vertices + array of indices)
           case osg::PrimitiveSet::DrawElementsUBytePrimitiveType:
                     {
                //OSG_WARN << "DrawElementsUByte" << std::endl;

                if ( primLength == 0 ) primLength = ps->getNumIndices();

                osg::DrawElementsUByte* drawElements = static_cast<osg::DrawElementsUByte*>( ps );

                std::vector<domP *> p;
                switch ( mode )
                {
                    case GL_LINES:
                    {
                        p.push_back(lines->getP());
                        lines->setCount( lines->getCount() + drawElements->size()/primLength );
                        break;
                    }
                    case GL_TRIANGLES:
                    {
                        p.push_back(tris->getP());
                        tris->setCount( tris->getCount() + drawElements->size()/primLength );
                        break;
                    }
                    case GL_LINE_STRIP:
                    {
                        p.push_back(daeSafeCast<domP>( linestrips->add( COLLADA_ELEMENT_P ) ));
                        linestrips->setCount( linestrips->getCount() + 1 );
                        break;
                    }
                    case GL_TRIANGLE_STRIP:
                    {
                        p.push_back(daeSafeCast<domP>( tristrips->add( COLLADA_ELEMENT_P ) ));
                        tristrips->setCount( tristrips->getCount() + 1 );
                        break;
                    }
                    case GL_TRIANGLE_FAN:
                    {
                        p.push_back(daeSafeCast<domP>( trifans->add( COLLADA_ELEMENT_P ) ));
                        trifans->setCount( trifans->getCount() + 1 );
                        break;
                    }
                    default:
                    {
                        unsigned int nbPolygons=drawElements->size()/primLength;
                        if (_pluginOptions.usePolygons)
                        {
                            //for( unsigned int idx = 0; idx < nbPolygons; ++idx ) /*idx*/
                            //huh ? why only one ?
                            p.push_back(polys->getP_array()[0]);
                            polys->setCount( polys->getCount() + nbPolygons );
                        }
                        else
                        {
                            polylist->getVcount()->getValue().append( primLength );
                            p.push_back(polylist->getP());
                            polylist->setCount( polylist->getCount() + nbPolygons );
                        }
                        break;
                    }
                }


                unsigned int primCount = 0;
                osg::DrawElementsUByte::iterator primItrBegin = drawElements->begin();
                unsigned int nbVerticesPerPoly= drawElements->size()/p.size();
                osg::DrawElementsUByte::iterator primItrEnd=primItrBegin+nbVerticesPerPoly;
                for( unsigned int iPoly = 0; iPoly < p.size(); ++iPoly )
                {
                    for( osg::DrawElementsUByte::iterator primItr = primItrBegin;primItr != primItrEnd;
                    ++primCount, ++primItr )
                    {

                        unsigned int vindex = *primItr;

                        appendGeometryIndices(geom,p[iPoly],vindex,
                                              norm,color,
                                              verts,normals,colors,texcoords,
                                              ncount,ccount);

                    }

                    primItrBegin+=nbVerticesPerPoly;
#if ( _SECURE_SCL == 1 )
                    if (primItrBegin != drawElements->end())
#endif
                    primItrEnd+=nbVerticesPerPoly;
                }
                break;
            }
            case osg::PrimitiveSet::DrawElementsUShortPrimitiveType:
            {
                //OSG_WARN << "DrawElementsUShort" << std::endl;
                if ( primLength == 0 ) primLength = ps->getNumIndices();

                osg::DrawElementsUShort* drawElements = static_cast<osg::DrawElementsUShort*>( ps );

                                std::vector<domP *> p;
                switch ( mode )
                {
                    case GL_LINES:
                    {
                        p.push_back(lines->getP());
                        lines->setCount( lines->getCount() + drawElements->size()/primLength );
                        break;
                    }
                    case GL_TRIANGLES:
                    {
                        p.push_back(tris->getP());
                        tris->setCount( tris->getCount() + drawElements->size()/primLength );
                        break;
                    }
                    case GL_LINE_STRIP:
                    {
                        p.push_back(daeSafeCast<domP>( linestrips->add( COLLADA_ELEMENT_P ) ));
                        linestrips->setCount( linestrips->getCount() + 1 );
                        break;
                    }
                    case GL_TRIANGLE_STRIP:
                    {
                        p.push_back(daeSafeCast<domP>( tristrips->add( COLLADA_ELEMENT_P ) ));
                        tristrips->setCount( tristrips->getCount() + 1 );
                        break;
                    }
                    case GL_TRIANGLE_FAN:
                    {
                        p.push_back(daeSafeCast<domP>( trifans->add( COLLADA_ELEMENT_P ) ));
                        trifans->setCount( trifans->getCount() + 1 );
                        break;
                    }
                    default:
                    {
                        unsigned int nbPolygons=drawElements->size()/primLength;
                        if (_pluginOptions.usePolygons)
                        {
                            //for( unsigned int idx = 0; idx < nbPolygons; ++idx ) /*idx*/
                            //huh ? why only one ?
                            p.push_back(polys->getP_array()[0]);
                            polys->setCount( polys->getCount() + nbPolygons );
                        }
                        else
                        {
                            polylist->getVcount()->getValue().append( primLength );
                            p.push_back(polylist->getP());
                            polylist->setCount( polylist->getCount() + nbPolygons );
                        }
                        break;
                    }
                }

                unsigned int primCount = 0;
                osg::DrawElementsUShort::iterator primItrBegin = drawElements->begin();
                unsigned int nbVerticesPerPoly= drawElements->size()/p.size();
                osg::DrawElementsUShort::iterator primItrEnd=primItrBegin+nbVerticesPerPoly;

                for( unsigned int iPoly = 0; iPoly < p.size(); ++iPoly )
                {
                  for( osg::DrawElementsUShort::iterator primItr = primItrBegin;primItr != primItrEnd;
                       ++primCount, ++primItr )
                  {

                    unsigned int vindex = *primItr;

                    appendGeometryIndices(geom,p[iPoly],vindex,
                                       norm,color,
                                       verts,normals,colors,texcoords,
                                       ncount,ccount);

                  }
                  primItrBegin+=nbVerticesPerPoly;
#if ( _SECURE_SCL == 1 )
                  if (primItrBegin != drawElements->end())
#endif
                  primItrEnd+=nbVerticesPerPoly;
                }

                break;
            }
            case osg::PrimitiveSet::DrawElementsUIntPrimitiveType:
            {
                //OSG_WARN << "DrawElementsUInt" << std::endl;

                if ( primLength == 0 ) primLength = ps->getNumIndices();

                osg::DrawElementsUInt* drawElements = static_cast<osg::DrawElementsUInt*>( ps );

                std::vector<domP *> p;
                switch ( mode )
                {
                    case GL_LINES:
                    {
                        p.push_back(lines->getP());
                        lines->setCount( lines->getCount() + drawElements->size()/primLength );
                        break;
                    }
                    case GL_TRIANGLES:
                    {
                        p.push_back(tris->getP());
                        tris->setCount( tris->getCount() + drawElements->size()/primLength );
                        break;
                    }
                    case GL_LINE_STRIP:
                    {
                        p.push_back(daeSafeCast<domP>( linestrips->add( COLLADA_ELEMENT_P ) ));
                        linestrips->setCount( linestrips->getCount() + 1 );
                        break;
                    }
                    case GL_TRIANGLE_STRIP:
                    {
                        p.push_back(daeSafeCast<domP>( tristrips->add( COLLADA_ELEMENT_P ) ));
                        tristrips->setCount( tristrips->getCount() + 1 );
                        break;
                    }
                    case GL_TRIANGLE_FAN:
                    {
                        p.push_back(daeSafeCast<domP>( trifans->add( COLLADA_ELEMENT_P ) ));
                        trifans->setCount( trifans->getCount() + 1 );
                        break;
                    }
                    default:
                    {
                        unsigned int nbPolygons=drawElements->size()/primLength;
                        if (_pluginOptions.usePolygons)
                        {
                            //for( unsigned int idx = 0; idx < nbPolygons; ++idx ) /*idx*/
                            //huh ? why only one ?
                            p.push_back(polys->getP_array()[0]);
                            polys->setCount( polys->getCount() + nbPolygons );
                        }
                        else
                        {
                            polylist->getVcount()->getValue().append( primLength );
                            p.push_back(polylist->getP());
                            polylist->setCount( polylist->getCount() + nbPolygons );
                        }
                        break;
                    }
                }

                unsigned int primCount = 0;
                osg::DrawElementsUInt::iterator primItrBegin = drawElements->begin();
                unsigned int nbVerticesPerPoly= drawElements->size()/p.size();
                osg::DrawElementsUInt::iterator primItrEnd=primItrBegin+nbVerticesPerPoly;

                for( unsigned int iPoly = 0; iPoly < p.size(); ++iPoly )
                {

                  for( osg::DrawElementsUInt::iterator primItr = primItrBegin;primItr != primItrEnd;
                       ++primCount, ++primItr )
                  {

                    unsigned int vindex = *primItr;

                    appendGeometryIndices(geom,p[iPoly],vindex,
                                       norm,color,
                                       verts,normals,colors,texcoords,
                                       ncount,ccount);

                  }
                  primItrBegin+=nbVerticesPerPoly;
#if ( _SECURE_SCL == 1 )
                  if (primItrBegin != drawElements->end())
#endif
                  primItrEnd+=nbVerticesPerPoly;
                }
                break;
            }
            default:
                OSG_WARN << "Unsupported primitiveSet" << std::endl;
                break;
        }

        if ( osg::getBinding(geom->getNormalArray()) == osg::Array::BIND_PER_PRIMITIVE_SET )
        {
            ncount++;
        }
        if ( osg::getBinding(geom->getColorArray()) == osg::Array::BIND_PER_PRIMITIVE_SET )
        {
            ccount++;
        }
    }
    return valid;
}

domSource *daeWriter::createSource( daeElement *parent, const std::string &baseName, int size, bool color, bool uv )
{
    domSource *src = daeSafeCast< domSource >( parent->add( COLLADA_ELEMENT_SOURCE ) );
    if ( src == NULL )
    {
        return NULL;
    }
    src->setId( baseName.c_str() );

    domFloat_array *fa = daeSafeCast< domFloat_array >( src->add( COLLADA_ELEMENT_FLOAT_ARRAY ) );
    std::string fName = baseName + "-array";
    fa->setId( fName.c_str() );

    domSource::domTechnique_common *teq = daeSafeCast< domSource::domTechnique_common >( src->add( COLLADA_ELEMENT_TECHNIQUE_COMMON ) );
    domAccessor *acc = daeSafeCast< domAccessor >( teq->add( COLLADA_ELEMENT_ACCESSOR ) );
    std::string url = "#" + fName;
    acc->setSource( url.c_str() );
    domParam *param;
    if ( color )
    {
        acc->setStride( size );
        param = daeSafeCast< domParam >( acc->add( COLLADA_ELEMENT_PARAM ) );
        param->setName( "R" );
        param->setType( "float" );

        param = daeSafeCast< domParam >( acc->add( COLLADA_ELEMENT_PARAM ) );
        param->setName( "G" );
        param->setType( "float" );

        param = daeSafeCast< domParam >( acc->add( COLLADA_ELEMENT_PARAM ) );
        param->setName( "B" );
        param->setType( "float" );

        if ( size == 4 )
        {
            param = daeSafeCast< domParam >( acc->add( COLLADA_ELEMENT_PARAM ) );
            param->setName( "A" );
            param->setType( "float" );
        }

    }
    else if ( uv )
    {
        const char * const type = "float";

        acc->setStride( size );
        param = daeSafeCast< domParam >( acc->add( COLLADA_ELEMENT_PARAM ) );
        param->setName( "S" );
        param->setType( type );

        param = daeSafeCast< domParam >( acc->add( COLLADA_ELEMENT_PARAM ) );
        param->setName( "T" );
        param->setType( type );

        if ( size >=3 )
        {
            param = daeSafeCast< domParam >( acc->add( COLLADA_ELEMENT_PARAM ) );
            param->setName( "P" );
            param->setType( type );
        }
    }
    else
    {
        const char * const type = "float";
        acc->setStride( size );
        param = daeSafeCast< domParam >( acc->add( COLLADA_ELEMENT_PARAM ) );
        param->setName( "X" );
        param->setType( type );

        param = daeSafeCast< domParam >( acc->add( COLLADA_ELEMENT_PARAM ) );
        param->setName( "Y" );
        param->setType( type );

        if ( size >=3 )
        {
            param = daeSafeCast< domParam >( acc->add( COLLADA_ELEMENT_PARAM ) );
            param->setName( "Z" );
            param->setType( type );

            if ( size == 4 )
            {
                param = daeSafeCast< domParam >( acc->add( COLLADA_ELEMENT_PARAM ) );
                param->setName( "W" );
                param->setType( type );
            }
        }
    }

    return src;
}

template < typename Ty >
Ty *daeWriter::createPrimGroup( daeString type, domMesh *mesh, domSource *norm, domSource *color, const std::vector< domSource* > &texcoord )
{
    unsigned int offset = 0;
    Ty *retVal = daeSafeCast< Ty >( mesh->add( type ) );
    domInputLocalOffset *ilo = daeSafeCast< domInputLocalOffset >( retVal->add( COLLADA_ELEMENT_INPUT ) );
    ilo->setOffset( offset++ );
    ilo->setSemantic( COMMON_PROFILE_INPUT_VERTEX );
    std::string url = "#" + std::string(mesh->getVertices()->getId());
    ilo->setSource( url.c_str() );
    if ( norm != NULL )
    {
        ilo = daeSafeCast< domInputLocalOffset >( retVal->add( COLLADA_ELEMENT_INPUT ) );
        ilo->setOffset( offset++ );
        ilo->setSemantic( COMMON_PROFILE_INPUT_NORMAL );
        url = "#" + std::string( norm->getId() );
        ilo->setSource( url.c_str() );
    }
    if ( color != NULL )
    {
        ilo = daeSafeCast< domInputLocalOffset >( retVal->add( COLLADA_ELEMENT_INPUT ) );
        ilo->setOffset( offset++ );
        ilo->setSemantic( COMMON_PROFILE_INPUT_COLOR );
        url = "#" + std::string( color->getId() );
        ilo->setSource( url.c_str() );
    }
    for ( unsigned int i = 0; i < texcoord.size(); i++ )
    {
        ilo = daeSafeCast< domInputLocalOffset >( retVal->add( COLLADA_ELEMENT_INPUT ) );
        ilo->setOffset( offset++ );
        ilo->setSemantic( COMMON_PROFILE_INPUT_TEXCOORD );
        ilo->setSet( i );
        url = "#" + std::string( texcoord[i]->getId() );
        ilo->setSource( url.c_str() );
    }

    return retVal;
}
