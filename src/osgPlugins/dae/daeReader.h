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

#ifndef _DAE_CONV_H_
#define _DAE_CONV_H_

#include <string>

#include <dae.h>
#include <dae/daeURI.h>
#include <dae/daeElement.h>
#include <dom/domCommon_color_or_texture_type.h>

#include <osg/Node>
#include <osg/Transform>
#include <osg/Notify>
#include <osg/PositionAttitudeTransform>
#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>
#include <osg/Material>


class domBind_material;
class domCamera;
//class domCommon_color_or_texture_type;
class domCommon_float_or_param_type;
class domGeometry;
class domInstance_controller;
class domInstance_geometry;
class domInstanceWithExtra;
class domLight;
class domLookat;
class domMatrix;
class domNode;
class domP;
class domProfile_COMMON;
class domScale;
class domSkew;
class domTranslate;
class domRotate;
class domVisual_scene;

#include <dom/domInputLocalOffset.h>

namespace osgdae {

class domSourceReader;

inline daeElement *getElementFromURI( daeURI &uri )
{
    if ( uri.getState() == daeURI::uri_loaded || uri.getState() == daeURI::uri_pending ) {
        uri.resolveElement();
    }
    return uri.getElement();
}
inline daeElement *getElementFromIDRef( daeIDRef &idref )
{
    if ( idref.getState() == daeIDRef::id_loaded || idref.getState() == daeIDRef::id_pending ) {
        idref.resolveElement();
    }
    return idref.getElement();
}

template< typename TInputArray, typename TInputType >
bool findInputSourceBySemantic( TInputArray& inputs, const char* semantic, daeElement *& element, 
                                TInputType ** input = NULL, int unit = 0 )
{
    element = NULL;
    int count = 0;
    for ( size_t i = 0; i < inputs.getCount(); i++ ) {
        if ( !strcmp(semantic, inputs[i]->getSemantic()) ) {
            if ( count == unit )
            {
                element = getElementFromURI( inputs[i]->getSource() );
                *input = (TInputType*)inputs[i];
                return true;
            }
            count++;
        }
    }
    return false;
}

/// Convert string to value using it's stream operator
template <typename T>
T parseString(const std::string& valueAsString) {
    std::stringstream str;
    str << valueAsString;
    T result;
    str >> result;
    return result;
}

inline osg::Vec3 parseVec3String(const std::string& valueAsString)
{
    std::stringstream str;
    str << valueAsString;
    osg::Vec3 result;
    str >> result.x() >> result.y() >> result.z();
    return result;
}

inline osg::Matrix parseMatrixString(const std::string& valueAsString)
{
    std::stringstream str;
    str << valueAsString;
    osg::Matrix result;
    str >> result(0,0) >> result(1,0) >> result(2,0) >> result(3,0)
        >> result(0,1) >> result(1,1) >> result(2,1) >> result(3,1)
        >> result(0,2) >> result(1,2) >> result(2,2) >> result(3,2)
        >> result(0,3) >> result(1,3) >> result(2,3) >> result(3,3);
    return result;
}


/**
@class daeReader
@brief Read a OSG scene from a DAE file 
*/ 
class daeReader {
public:
    daeReader(DAE *dae_, bool strictTransparency = false);
    virtual ~daeReader();

    bool convert( const std::string &fileURI );
    
    osg::Node* getRootNode()    { return rootNode; }

    // Additional Information
    std::string m_AssetUnitName;
    float m_AssetUnitMeter;
    domUpAxisType m_AssetUp_axis;

    // Texture unit useage
    enum
    {
        AMBIENT_OCCLUSION_UNIT = 0,
        MAIN_TEXTURE_UNIT,
        TRANSPARENCY_MAP_UNIT
    };

protected:
    //scene processing
    osg::Node*    processVisualScene( domVisual_scene *scene );
    osg::Node*    processNode( domNode *node );
    osg::Node*    processOsgMatrixTransform( domNode *node );
    //osg::Node* processInstance( domInstanceWithExtra *iwe );

    // Processing of OSG specific info stored in node extras
    osg::Node* processExtras(domNode *node);
    void processNodeExtra(osg::Node* osgNode, domNode *node);
    domTechnique* getOpenSceneGraphProfile(domExtra* extra);
    void processAsset( domAsset *node );

    osg::Node* processOsgSwitch(domTechnique* teq);
    osg::Node* processOsgMultiSwitch(domTechnique* teq);
    osg::Node* processOsgLOD(domTechnique* teq);
    osg::Node* processOsgDOFTransform(domTechnique* teq);
    osg::Node* processOsgSequence(domTechnique* teq);

    //geometry processing
    class ReaderGeometry : public osg::Geometry
    {
    public:
        std::map<int, int> _TexcoordSetMap;
    };
    osg::Geode* processInstanceGeometry( domInstance_geometry *ig );
    osg::Geode* processGeometry( domGeometry *geo );
    osg::Geode* processInstanceController( domInstance_controller *ictrl );

    typedef std::map< daeElement*, domSourceReader > SourceMap;
    typedef std::map< int, osg::IntArray*, std::less<int> > IndexMap;

    template< typename T >
    void processSinglePPrimitive(osg::Geode* geode, T *group, SourceMap &sources, GLenum mode );
    
    template< typename T >
    void processMultiPPrimitive(osg::Geode* geode, T *group, SourceMap &sources, GLenum mode );

    void processPolylist(osg::Geode* geode, domPolylist *group, SourceMap &sources );

    void resolveArrays( domInputLocalOffset_Array &inputs, osg::Geometry *geom, 
                        SourceMap &sources, IndexMap &index_map );

    void processP( domP *p, osg::Geometry *&geom, IndexMap &index_map, osg::DrawArrayLengths* dal/*GLenum mode*/ );

    //material/effect processing
    void processBindMaterial( domBind_material *bm, domGeometry *geom, osg::Geode *geode, osg::Geode *cachedGeode );
    void processMaterial(osg::StateSet *ss, domMaterial *mat );
    void processEffect(osg::StateSet *ss, domEffect *effect );
    void processProfileCOMMON(osg::StateSet *ss, domProfile_COMMON *pc );
    bool processColorOrTextureType( domCommon_color_or_texture_type *cot, 
                                    osg::Material::ColorMode channel, 
                                    osg::Material *mat, 
                                    domCommon_float_or_param_type *fop = NULL, 
                                    osg::StateAttribute **sa = NULL,
                                    bool normalizeShininess=false);
    void processTransparencySettings( domCommon_transparent_type *ctt,
                                        domCommon_float_or_param_type *pTransparency, 
                                        osg::StateSet *ss,
                                        osg::Material *material,
                                        xsNCName diffuseTextureName );
    bool GetFloat4Param(xsNCName Reference, domFloat4 &f4);
    bool GetFloatParam(xsNCName Reference, domFloat &f);

    osg::StateAttribute *processTexture( domCommon_color_or_texture_type_complexType::domTexture *tex );

    //scene objects
    osg::Node* processLight( domLight *dlight );
    osg::Node* processCamera( domCamera *dcamera );

protected:
    DAE *dae;
    osg::Node* rootNode;

    std::map<std::string,bool> _targetMap;

    int m_numlights;

    domInstance_effect *currentInstance_effect;
    domEffect *currentEffect;

    typedef std::map< domGeometry*, osg::ref_ptr<osg::Geode> >    domGeometryGeodeMap;
    typedef std::map< domMaterial*, osg::ref_ptr<osg::StateSet> > domMaterialStateSetMap;
    typedef std::map< std::string, osg::ref_ptr<osg::StateSet> >    MaterialStateSetMap;

    /// Maps geometry to a Geode
    domGeometryGeodeMap     geometryMap;
    // Maps material target to stateset
    domMaterialStateSetMap  materialMap;
    // Maps material symbol to stateset
    MaterialStateSetMap     materialMap2;

    enum AuthoringTool
    {
        UNKNOWN,
        GOOGLE_SKETCHUP
    };
    AuthoringTool m_AuthoringTool;
    bool m_StrictTransparency;
};

}

#endif


