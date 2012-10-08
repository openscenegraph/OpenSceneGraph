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
#include <dom/domInputLocalOffset.h>
#include <dom/domInstance_controller.h>

#include <osg/Node>
#include <osg/Notify>
#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>
#include <osg/Material>
#include <osg/Texture2D>
#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/Bone>
#include <osgAnimation/Skeleton>

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

namespace osgDAE
{

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
    enum TessellateMode
    {
        TESSELLATE_NONE,                 ///< Do not tessellate at all (Polygons are stored as GL_POLYGON - not suitable for concave polygons)
        TESSELLATE_POLYGONS_AS_TRIFAN,   ///< Tessellate the old way, interpreting polygons as triangle fans (faster, but does not work for concave polygons)
        TESSELLATE_POLYGONS              ///< Use full tessellation of polygons (slower, works for concave polygons)
    };

    struct Options
    {
        Options();
        bool strictTransparency;
        int precisionHint;              ///< Precision hint flags, as specified in osgDB::Options::PrecisionHint
        bool usePredefinedTextureUnits;
        TessellateMode tessellateMode;
    };

    daeReader(DAE *dae_, const Options * pluginOptions);
    virtual ~daeReader();

    bool convert( std::istream &fin );
    bool convert( const std::string &fileURI );

    osg::Node* getRootNode()    { return _rootNode; }

    const std::string& getAssetUnitName() const {return _assetUnitName;}
    float getAssetUnitMeter() const {return _assetUnitMeter;}
    domUpAxisType getAssetUpAxis() const {return _assetUp_axis;}

    enum TextureUnitUsage
    {
        AMBIENT_OCCLUSION_UNIT = 0,
        MAIN_TEXTURE_UNIT,
        TRANSPARENCY_MAP_UNIT
    };

    enum InterpolationType
    {
        INTERPOLATION_UNKNOWN,
        INTERPOLATION_STEP,
        INTERPOLATION_LINEAR,
        INTERPOLATION_BEZIER,
        INTERPOLATION_HERMITE,
        INTERPOLATION_CARDINAL,
        INTERPOLATION_BSPLINE,

        //COLLADA spec states that if interpolation is not specified then
        //interpolation is application defined. Linear is a sensible default.
        INTERPOLATION_DEFAULT = INTERPOLATION_LINEAR
    };

    enum AuthoringTool
    {
        UNKNOWN,
        BLENDER,
        DAZ_STUDIO,
        FBX_CONVERTER,
        AUTODESK_3DS_MAX = FBX_CONVERTER,//3ds Max exports to DAE via Autodesk's FBX converter
        GOOGLE_SKETCHUP,
        MAYA
    };

    class TextureParameters
    {
    public:
        TextureParameters()
            : wrap_s(osg::Texture::REPEAT), wrap_t(osg::Texture::REPEAT),
            filter_min(osg::Texture::LINEAR_MIPMAP_LINEAR), filter_mag(osg::Texture::LINEAR),
            transparent(false), opaque(FX_OPAQUE_ENUM_A_ONE), transparency(1.0f)
        {}

        bool operator < (const TextureParameters& rhs) const
        {
            int diffStr = filename.compare(rhs.filename);
            if (diffStr) return diffStr < 0;
            if (wrap_s != rhs.wrap_s) return wrap_s < rhs.wrap_s;
            if (wrap_t != rhs.wrap_t) return wrap_t < rhs.wrap_t;
            if (filter_min != rhs.filter_min) return filter_min < rhs.filter_min;
            if (filter_mag != rhs.filter_mag) return filter_mag < rhs.filter_mag;
            if (transparency != rhs.transparency) return transparency < rhs.transparency;
            if (opaque != rhs.opaque) return opaque < rhs.opaque;
            if (transparent != rhs.transparent) return transparent < rhs.transparent;
            return border < rhs.border;
        }

        std::string filename;
        osg::Texture::WrapMode wrap_s, wrap_t;
        osg::Texture::FilterMode filter_min, filter_mag;
        osg::Vec4 border;

        //The following parameters are for transparency textures, to handle
        //COLLADA's horrible transparency spec.
        bool transparent;
        domFx_opaque_enum opaque;
        float transparency;
    };

    class ChannelPart : public osg::Referenced
    {
    public:
        std::string name;
        osg::ref_ptr<osgAnimation::KeyframeContainer> keyframes;
        InterpolationType interpolation;
    };

    typedef std::map<domGeometry*, osg::ref_ptr<osg::Geode> >    domGeometryGeodeMap;
    typedef std::map<domMaterial*, osg::ref_ptr<osg::StateSet> > domMaterialStateSetMap;
    typedef std::map<std::string, osg::ref_ptr<osg::StateSet> >    MaterialStateSetMap;
    typedef std::multimap< daeElement*, domChannel*> daeElementDomChannelMap;
    typedef std::map<domChannel*, osg::ref_ptr<osg::NodeCallback> > domChannelOsgAnimationUpdateCallbackMap;
    typedef std::map<domNode*, osg::ref_ptr<osgAnimation::Bone> > domNodeOsgBoneMap;
    typedef std::map<domNode*, osg::ref_ptr<osgAnimation::Skeleton> > domNodeOsgSkeletonMap;
    typedef std::map<TextureParameters, osg::ref_ptr<osg::Texture2D> > TextureParametersMap;
    typedef std::map<std::pair<const osg::StateSet*, TextureUnitUsage>, std::string> TextureToCoordSetMap;
    typedef std::map<std::string, size_t> IdToCoordIndexMap;

    typedef std::map< daeElement*, domSourceReader > SourceMap;
    typedef std::map< int, osg::IntArray*, std::less<int> > IndexMap;
    typedef std::map< int, osg::Array*, std::less<int> > ArrayMap;

    typedef std::multimap< osgAnimation::Target*, osg::ref_ptr<ChannelPart> > TargetChannelPartMap;
    typedef std::multimap<std::pair<const domMesh*, unsigned>, std::pair<osg::ref_ptr<osg::Geometry>, GLuint> > OldToNewIndexMap;

private:
    bool processDocument( const std::string& );
    void clearCaches();

    // If the node is a bone then it should be added before any other types of
    // node, this function makes that happen.
    static void addChild(osg::Group*, osg::Node*);

     //scene processing
    osg::Group* turnZUp();
    osg::Group*    processVisualScene( domVisual_scene *scene );
    osg::Node*    processNode( domNode *node, bool skeleton );
    osg::Transform*    processOsgMatrixTransform( domNode *node, bool isBone);

    template <typename T>
    inline void getTransparencyCounts(daeDatabase*, int& zero, int& one) const;

    /** Earlier versions of the COLLADA 1.4 spec state that transparency values
    of 0 mean 100% opacity, but this has been changed in later versions to state
    that transparency values of 1 mean 100% opacity. Documents created by
    different tools at different times adhere to different versions of the
    standard. This function looks at all transparency values in the database and
    heuristically decides which way the values should be interpreted.*/
    bool findInvertTransparency(daeDatabase*) const;

    osgAnimation::BasicAnimationManager* processAnimationLibraries(domCOLLADA* document);
    void processAnimationClip(osgAnimation::BasicAnimationManager* pOsgAnimationManager, domAnimation_clip* pDomAnimationClip);
    void processAnimationMap(const TargetChannelPartMap&, osgAnimation::Animation* pOsgAnimation);
    ChannelPart* processSampler(domChannel* pDomChannel, SourceMap &sources);
    void processAnimationChannels(domAnimation* pDomAnimation, TargetChannelPartMap& tcm);
    void processChannel(domChannel* pDomChannel, SourceMap &sources, TargetChannelPartMap& tcm);
    void extractTargetName(const std::string&, std::string&, std::string&, std::string&);

    // Processing of OSG specific info stored in node extras
    osg::Group* processExtras(domNode *node);
    void processNodeExtra(osg::Node* osgNode, domNode *node);
    domTechnique* getOpenSceneGraphProfile(domExtra* extra);
    void processAsset( domAsset *node );

    osg::Group* processOsgSwitch(domTechnique* teq);
    osg::Group* processOsgMultiSwitch(domTechnique* teq);
    osg::Group* processOsgLOD(domTechnique* teq);
    osg::Group* processOsgDOFTransform(domTechnique* teq);
    osg::Group* processOsgSequence(domTechnique* teq);

    // geometry processing
    osg::Geode* getOrCreateGeometry(domGeometry *geom, domBind_material* pDomBindMaterial, const osg::Geode** ppOriginalGeode = NULL);
    osgAnimation::Bone* getOrCreateBone(domNode *pDomNode);
    osgAnimation::Skeleton* getOrCreateSkeleton(domNode *pDomNode);
    osg::Geode* processInstanceGeometry( domInstance_geometry *ig );

    osg::Geode* processMesh(domMesh* pDomMesh);
    osg::Geode* processConvexMesh(domConvex_mesh* pDomConvexMesh);
    osg::Geode* processSpline(domSpline* pDomSpline);
    osg::Geode* processGeometry(domGeometry *pDomGeometry);

    typedef std::vector<domInstance_controller*> domInstance_controllerList;

    void processSkins();
    //Process skins attached to one skeleton
    void processSkeletonSkins(domNode* skeletonRoot, const domInstance_controllerList&);
    void processSkin(domSkin* pDomSkin, domNode* skeletonRoot, osgAnimation::Skeleton*, domBind_material* pDomBindMaterial);
    osg::Node* processMorph(domMorph* pDomMorph, domBind_material* pDomBindMaterial);
    osg::Node* processInstanceController( domInstance_controller *ictrl );

    template< typename T >
    void processSinglePPrimitive(osg::Geode* geode, const domMesh* pDomMesh, const T* group, SourceMap& sources, GLenum mode);

    template< typename T >
    void processMultiPPrimitive(osg::Geode* geode, const domMesh* pDomMesh, const T* group, SourceMap& sources, GLenum mode);

    void processPolylist(osg::Geode* geode, const domMesh* pDomMesh, const domPolylist *group, SourceMap &sources, TessellateMode tessellateMode);

    template< typename T >
    void processPolygons(osg::Geode* geode, const domMesh* pDomMesh, const T *group, SourceMap &sources, GLenum mode, TessellateMode tessellateMode);

    void resolveMeshArrays(const domP_Array&,
        const domInputLocalOffset_Array& inputs, const domMesh* pDomMesh,
        osg::Geometry* geometry, SourceMap &sources,
        std::vector<std::vector<GLuint> >& vertexLists);

    //material/effect processing
    void processBindMaterial( domBind_material *bm, domGeometry *geom, osg::Geode *geode, osg::Geode *cachedGeode );
    void processMaterial(osg::StateSet *ss, domMaterial *mat );
    void processEffect(osg::StateSet *ss, domEffect *effect );
    void processProfileCOMMON(osg::StateSet *ss, domProfile_COMMON *pc );
    bool processColorOrTextureType(const osg::StateSet*,
                                    domCommon_color_or_texture_type *cot,
                                    osg::Material::ColorMode channel,
                                    osg::Material *mat,
                                    domCommon_float_or_param_type *fop = NULL,
                                    osg::Texture2D **sa = NULL,
                                    bool normalizeShininess=false);
    void processTransparencySettings( domCommon_transparent_type *ctt,
                                        domCommon_float_or_param_type *pTransparency,
                                        osg::StateSet*,
                                        osg::Material *material,
                                        unsigned int diffuseTextureUnit );
    bool GetFloat4Param(xsNCName Reference, domFloat4 &f4) const;
    bool GetFloatParam(xsNCName Reference, domFloat &f) const;

    std::string processImagePath(const domImage*) const;
    osg::Image* processImageTransparency(const osg::Image*, domFx_opaque_enum, float transparency) const;
    osg::Texture2D* processTexture( domCommon_color_or_texture_type_complexType::domTexture *tex, const osg::StateSet*, TextureUnitUsage, domFx_opaque_enum = FX_OPAQUE_ENUM_A_ONE, float transparency = 1.0f);
    bool copyTextureCoordinateSet(const osg::StateSet* ss, const osg::Geometry* cachedGeometry, osg::Geometry* clonedGeometry, const domInstance_material* im, TextureUnitUsage tuu, unsigned int textureUnit);

    //scene objects
    osg::Node* processLight( domLight *dlight );
    osg::Node* processCamera( domCamera *dcamera );

    domNode* getRootJoint(domNode*) const;
    domNode* findJointNode(daeElement* searchFrom, domInstance_controller*) const;
    domNode* findSkeletonNode(daeElement* searchFrom, domInstance_controller*) const;

    /// Return whether the node is used as a bone. Note that while many files
    /// identify joints with type="JOINT", some don't do this, while others
    /// incorrectly identify every node as a joint.
    bool isJoint(const domNode* node) const {return _jointSet.find(node) != _jointSet.end();}

private:

    DAE *_dae;
    osg::Node* _rootNode;
    osg::ref_ptr<osg::StateSet> _rootStateSet;
    domCOLLADA* _document;
    domVisual_scene* _visualScene;

    std::map<std::string,bool> _targetMap;

    int _numlights;

    domInstance_effect *_currentInstance_effect;
    domEffect *_currentEffect;

    /// Maps an animated element to a domchannel to quickly find which animation influence this element
    // TODO a single element can be animated by multiple channels (with different members like translate.x or morphweights(2) )
    daeElementDomChannelMap _daeElementDomChannelMap;
    /// Maps a domchannel to an animationupdatecallback
    domChannelOsgAnimationUpdateCallbackMap _domChannelOsgAnimationUpdateCallbackMap;
    /// Maps geometry to a Geode
    domGeometryGeodeMap _geometryMap;
    /// All nodes in the document that are used as joints.
    std::set<const domNode*> _jointSet;
    /// Maps a node (of type joint) to a osgAnimation::Bone
    domNodeOsgBoneMap _jointMap;
    /// Maps a node (of type joint) to a osgAnimation::Skeleton
    domNodeOsgSkeletonMap _skeletonMap;
    // Maps material target to stateset
    domMaterialStateSetMap _materialMap;
    // Maps material symbol to stateset
    MaterialStateSetMap _materialMap2;
    TextureParametersMap _textureParamMap;
    TextureToCoordSetMap _texCoordSetMap;
    IdToCoordIndexMap    _texCoordIdMap;
    domInstance_controllerList _skinInstanceControllers;
    OldToNewIndexMap _oldToNewIndexMap;

    AuthoringTool _authoringTool;
    bool _invertTransparency;
    Options _pluginOptions;

    // Additional Information
    std::string _assetUnitName;
    float _assetUnitMeter;
    domUpAxisType _assetUp_axis;
};

}

#endif


