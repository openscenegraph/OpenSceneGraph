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

#include "daeUtils.h"


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

/**
@class daeReader
@brief Read a OSG scene from a DAE file 
*/ 
class daeReader {
public:
    daeReader();
    virtual ~daeReader();

    bool convert( const std::string &fileURI );
    
    osg::Node* getRootNode()    { return rootNode; }

protected:
    //scene processing
    osg::Node* processVisualScene( domVisual_scene *scene );
    osg::Node* processNode( domNode *node );
    //osg::Node* processInstance( domInstanceWithExtra *iwe );

    //transform processing
    osg::Transform* processMatrix( domMatrix *mat );
    osg::Transform* processTranslate( domTranslate *trans );
    osg::Transform* processRotate( domRotate *rot );
    osg::Transform* processScale( domScale *scale );
    osg::Transform* processLookat( domLookat *la );
    osg::Transform* processSkew( domSkew *skew );

    //geometry processing
    osg::Node* processInstance_geometry( domInstance_geometry *ig );
    osg::Node* processGeometry( domGeometry *geo );
    osg::Node* processInstance_controller( domInstance_controller *ictrl );

    typedef std::map< daeElement*, domSourceReader > SourceMap;
    typedef std::map< int, osg::IntArray*, std::less<int> > IndexMap;

    template< typename T >
    osg::Node* processSinglePPrimitive( T *group, SourceMap &sources, GLenum mode );
    
    template< typename T >
    osg::Node* processMultiPPrimitive( T *group, SourceMap &sources, GLenum mode );

    osg::Node* processPolylist( domPolylist *group, SourceMap &sources );

    void resolveArrays( domInputLocalOffset_Array &inputs, osg::Geometry *&geom, 
                        SourceMap &sources, IndexMap &index_map );

    void processP( domP *p, osg::Geometry *&geom, IndexMap &index_map, osg::DrawArrayLengths* dal/*GLenum mode*/ );

    //material/effect processing
    void processBindMaterial( domBind_material *bm, osg::Node *geo );
    osg::StateSet *processMaterial( domMaterial *mat );
    osg::StateSet *processEffect( domEffect *effect );
    osg::StateSet *processProfileCOMMON( domProfile_COMMON *pc );
    bool processColorOrTextureType( domCommon_color_or_texture_type *cot, 
        osg::Material::ColorMode channel, osg::Material *mat, domCommon_float_or_param_type *fop = NULL, osg::StateAttribute **sa = NULL );
    osg::StateAttribute *processTransparentType( domCommon_transparent_type *ctt, osg::StateSet *ss );

    osg::StateAttribute *processTexture( domCommon_color_or_texture_type_complexType::domTexture *tex );

    //scene objects
    osg::Node* processLight( domLight *dlight );
    osg::Node* processCamera( domCamera *dcamera );

protected:
    DAE *dae;
    osg::Node* rootNode;

    int m_numlights;

    domEffect *currentEffect;

    std::map< domGeometry*, osg::Node* > geometryMap;
    std::map< domMaterial*, osg::StateSet* > materialMap;
};

};

#endif


