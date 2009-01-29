#include "ConvertFromInventor.h"

#include "PendulumCallback.h"
#include "ShuttleCallback.h"

// OSG headers
#include <osg/MatrixTransform>
#include <osg/Node>
#include <osg/Geode>
#include <osg/Notify>
#include <osg/LineWidth>
#include <osg/Point>
#include <osg/TexEnv>
#include <osg/Texture2D>
#include <osg/PolygonMode>
#include <osg/BlendFunc>
#include <osg/Material>
#include <osg/CullFace>
#include <osg/LightModel>
#include <osg/LightSource>
#include <osg/ShadeModel>
#include <osg/LOD>
#include <osgUtil/TransformCallback>

// Inventor headers
#include <Inventor/SoDB.h>
#include <Inventor/SoInteraction.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/nodes/SoVertexShape.h>
#include <Inventor/nodes/SoLight.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoSpotLight.h>
#include <Inventor/nodes/SoPointLight.h>
#include <Inventor/nodes/SoRotor.h>
#include <Inventor/nodes/SoPendulum.h>
#include <Inventor/nodes/SoShuttle.h>
#include <Inventor/nodes/SoLOD.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/misc/SoChildList.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/SbLinear.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoInfo.h>

#ifdef __COIN__
#include <Inventor/VRMLnodes/SoVRMLImageTexture.h>
#include <Inventor/VRMLnodes/SoVRMLTransform.h>
#include <Inventor/VRMLnodes/SoVRMLAppearance.h>
#include <Inventor/VRMLnodes/SoVRMLMaterial.h>
#endif

#include "GroupSoLOD.h"

#include <map>
#include <assert.h>
#include <math.h>
#include <string.h>
#ifdef __linux
#include <values.h>
#endif
#ifdef __APPLE__
#include <float.h>
#endif

#define DEBUG_IV_PLUGIN
///////////////////////////////////////////
ConvertFromInventor::ConvertFromInventor()
{
    numPrimitives = 0;
    transformInfoName = "";
    appearanceName = "";
    inAppearanceWithNoTexture = false;
    lightGroup = NULL;
}
///////////////////////////////////////////
ConvertFromInventor::~ConvertFromInventor()
{
}
///////////////////////////////////////////////////////////
osg::Node* ConvertFromInventor::convert(SoNode* rootIVNode)
{    
    // Transformation matrix for converting Inventor coordinate system to OSG 
    // coordinate system
    osg::Matrix ivToOSGMat(osg::Matrix(1.0, 0.0, 0.0, 0.0,
                                       0.0, 0.0, 1.0, 0.0,
                                       0.0,-1.0, 0.0, 0.0,
                                       0.0, 0.0, 0.0, 1.0));

    // Create a root node and push it onto the stack
    _root = new osg::MatrixTransform;
    _root->setMatrix(ivToOSGMat);
    groupStack.push(_root.get());

    // Push an empty list of light and push it onto the light stack
    LightList lightList;
    lightStack.push(lightList);
    
    // Create callback actions for the inventor nodes 
    // These callback functions perform the conversion
    // note: if one class is derived from the other and both callbacks
    // are registered, both functions will be called
    SoCallbackAction cbAction;
    cbAction.addPreCallback(SoShape::getClassTypeId(), preShape, this);
    cbAction.addPostCallback(SoShape::getClassTypeId(), postShape, this);
    cbAction.addPreCallback(SoGroup::getClassTypeId(), preGroup, this);
    cbAction.addPostCallback(SoGroup::getClassTypeId(), postGroup, this);
    cbAction.addPreCallback(SoTexture2::getClassTypeId(), preTexture, this);
#ifdef __COIN__
    cbAction.addPreCallback(SoVRMLImageTexture::getClassTypeId(),
                            preVRMLImageTexture, this);
    cbAction.addPreCallback(SoVRMLAppearance::getClassTypeId(),
                            preVRMLAppearance, this);
    cbAction.addPreCallback(SoInfo::getClassTypeId(), preInfo, this);
#endif
    cbAction.addPreCallback(SoLight::getClassTypeId(), preLight, this);
    cbAction.addPreCallback(SoRotor::getClassTypeId(), preRotor, this);
    cbAction.addPreCallback(SoPendulum::getClassTypeId(), prePendulum, this);
    cbAction.addPreCallback(SoShuttle::getClassTypeId(), preShuttle, this);
    cbAction.addTriangleCallback(SoShape::getClassTypeId(), addTriangleCB, this);
    cbAction.addLineSegmentCallback(SoShape::getClassTypeId(), addLineSegmentCB,
                                    this);
    cbAction.addPointCallback(SoShape::getClassTypeId(), addPointCB, this);

    // Traverse the inventor scene graph
    cbAction.apply(rootIVNode);

    // Pop all the groups that are Transforms
    // Verify that the last transform is _root .
    assert(groupStack.size() > 0 && "groupStack underflow.");
    osg::ref_ptr<osg::Group> group = groupStack.top();
    while (strcmp(group->className(), "MatrixTransform") == 0)
    {
        groupStack.pop();
        if (groupStack.empty()) break;
        group = groupStack.top();
    }
    assert(group.get() == _root.get() && "groupStack error");
    assert(groupStack.size() == 0 && "groupStack is not empty after traversal.");

    assert(soTexStack.size() == 0 && "soTexStack was left at inconsistent state.");

    assert(lightStack.size() == 1 && "lightStack was left at inconsistent state.");
    lightStack.pop();

    return _root.get(); 
}
///////////////////////////////////////////////////////////////////
SoCallbackAction::Response 
ConvertFromInventor::preShape(void* data, SoCallbackAction* action, 
                              const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    osg::notify(osg::INFO) << "preShape()    " 
              << node->getTypeId().getName().getString() << std::endl;
#endif

    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    // Normal and color binding map from Inventor to OSG
    static std::map<SoNormalBinding::Binding, osg::Geometry::AttributeBinding> 
        normBindingMap;
    static std::map<SoMaterialBinding::Binding, osg::Geometry::AttributeBinding>
        colBindingMap;
    static bool firstTime = true;
    if (firstTime)
    {
        normBindingMap[SoNormalBinding::OVERALL]  
                                        = osg::Geometry::BIND_OVERALL;
        normBindingMap[SoNormalBinding::PER_PART] 
                                        = osg::Geometry::BIND_PER_PRIMITIVE;
        normBindingMap[SoNormalBinding::PER_PART_INDEXED] 
                                        = osg::Geometry::BIND_PER_PRIMITIVE;
        normBindingMap[SoNormalBinding::PER_FACE] 
                                        = osg::Geometry::BIND_PER_PRIMITIVE;
        normBindingMap[SoNormalBinding::PER_FACE_INDEXED] 
                                        = osg::Geometry::BIND_PER_PRIMITIVE;
        normBindingMap[SoNormalBinding::PER_VERTEX] 
                                        = osg::Geometry::BIND_PER_VERTEX;
        normBindingMap[SoNormalBinding::PER_VERTEX_INDEXED] 
                                        = osg::Geometry::BIND_PER_VERTEX;

        colBindingMap[SoMaterialBinding::OVERALL] 
                                        = osg::Geometry::BIND_OVERALL;
        colBindingMap[SoMaterialBinding::PER_PART]
                                        = osg::Geometry::BIND_PER_PRIMITIVE;
        colBindingMap[SoMaterialBinding::PER_PART_INDEXED] 
                                        = osg::Geometry::BIND_PER_PRIMITIVE;
        colBindingMap[SoMaterialBinding::PER_FACE]
                                        = osg::Geometry::BIND_PER_PRIMITIVE;
        colBindingMap[SoMaterialBinding::PER_FACE_INDEXED] 
                                        = osg::Geometry::BIND_PER_PRIMITIVE;
        colBindingMap[SoMaterialBinding::PER_VERTEX] 
                                        = osg::Geometry::BIND_PER_VERTEX;
        colBindingMap[SoMaterialBinding::PER_VERTEX_INDEXED] 
                                        = osg::Geometry::BIND_PER_VERTEX;

        firstTime = false;
    }

    // Get normal and color binding
    if (node->isOfType(SoVertexShape::getClassTypeId()))
    {
        thisPtr->normalBinding = normBindingMap[action->getNormalBinding()];
        thisPtr->colorBinding = colBindingMap[action->getMaterialBinding()];
    }
    else
    {
        thisPtr->normalBinding = osg::Geometry::BIND_PER_VERTEX;
        thisPtr->colorBinding = osg::Geometry::BIND_PER_VERTEX;
    }

    // Check vertex ordering
    if (action->getVertexOrdering() == SoShapeHints::CLOCKWISE)
        thisPtr->vertexOrder = CLOCKWISE;
    else
        thisPtr->vertexOrder = COUNTER_CLOCKWISE;

    // Clear the data from the previous shape callback
    thisPtr->numPrimitives = 0;
    thisPtr->vertices.clear();
    thisPtr->normals.clear();
    thisPtr->colors.clear();
    thisPtr->textureCoords.clear();
    
    return SoCallbackAction::CONTINUE;
}
///////////////////////////////////////////////////////////
// OSG doesn't seem to have a transpose function         //
//for matrices                                           //
///////////////////////////////////////////////////////////
void ConvertFromInventor::transposeMatrix(osg::Matrix& mat)
{
    float tmp;
    for (int j = 0; j < 4; j++) 
    {
        for (int i = j + 1; i < 4; i++) 
        {
            tmp = mat.operator()(j,i);
            mat.operator()(j,i) = mat.operator()(i,j);
            mat.operator()(i,j) = tmp;
        }
    }

}
////////////////////////////////////////////////////////////////////
SoCallbackAction::Response 
ConvertFromInventor::postShape(void* data, SoCallbackAction* action, 
                               const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    osg::notify(osg::INFO) << "postShape()   "
              << node->getTypeId().getName().getString() << std::endl;
#endif

    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);


    // Create a new Geometry
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;


    osg::ref_ptr<osg::Vec3Array> coords = new osg::Vec3Array(thisPtr->vertices.size());
    for (unsigned int i = 0; i < thisPtr->vertices.size(); i++)
        (*coords)[i] = thisPtr->vertices[i];
    geometry->setVertexArray(coords.get());

    osg::ref_ptr<osg::Vec3Array> norms = NULL;
    if (thisPtr->normalBinding == osg::Geometry::BIND_OVERALL)
    {
        norms = new osg::Vec3Array(1);
        const SbVec3f &norm = action->getNormal(0);
        (*norms)[0].set(norm[0], norm[1], norm[2]);
    }
    else
    {
        norms = new osg::Vec3Array(thisPtr->normals.size());
        for (unsigned int i = 0; i < thisPtr->normals.size(); i++)
        {
            (*norms)[i] = thisPtr->normals[i];
        }
    }
    geometry->setNormalArray(norms.get());
    geometry->setNormalBinding(thisPtr->normalBinding);

    // Set the colors
    osg::ref_ptr<osg::Vec4Array> cols;
    if (thisPtr->colorBinding == osg::Geometry::BIND_OVERALL)
    {
        cols = new osg::Vec4Array(1);
        SbColor ambient, diffuse, specular, emission;
        float transparency, shininess;
        action->getMaterial(ambient, diffuse, specular, emission, shininess, 
                            transparency, 0);
        (*cols)[0].set(diffuse[0], diffuse[1], diffuse[2], 1.0 - transparency);
    }
    else
    {
        cols = new osg::Vec4Array(thisPtr->colors.size());
        for (unsigned int i = 0; i < thisPtr->colors.size(); i++)
            (*cols)[i] = thisPtr->colors[i];
    }
    geometry->setColorArray(cols.get());
    geometry->setColorBinding(thisPtr->colorBinding);


    if (thisPtr->textureCoords.empty())
        osg::notify(osg::INFO)<<"tex coords not found"<<std::endl;
    else {
        
        // report texture coordinate conditions
        if (action->getNumTextureCoordinates()>0)
            osg::notify(osg::INFO)<<"tex coords found"<<std::endl;
        else
           osg::notify(osg::INFO)<<"tex coords generated"<<std::endl;

        // Get the texture transformation matrix
        osg::Matrix textureMat;
        textureMat.set((float *) action->getTextureMatrix().getValue());

        // Transform texture coordinates if texture matrix is not an identity mat
        osg::Matrix identityMat;
        identityMat.makeIdentity();
        osg::ref_ptr<osg::Vec2Array> texCoords 
            = new osg::Vec2Array(thisPtr->textureCoords.size());
        if (textureMat == identityMat)
        {
            // Set the texture coordinates
            for (unsigned int i = 0; i < thisPtr->textureCoords.size(); i++)
                (*texCoords)[i] = thisPtr->textureCoords[i];
        }
        else
        {
            // Transform and set the texture coordinates
            for (unsigned int i = 0; i < thisPtr->textureCoords.size(); i++)
            {
                osg::Vec3 transVec = textureMat.preMult(
                        osg::Vec3(thisPtr->textureCoords[i][0], 
                                  thisPtr->textureCoords[i][1],
                                  0.0));
                (*texCoords)[i].set(transVec.x(), transVec.y());
            }
        }

        geometry->setTexCoordArray(0, texCoords.get());
    }
    
    // Set the parameters for the geometry

    geometry->addPrimitiveSet(new osg::DrawArrays(thisPtr->primitiveType,0,
                                                  coords->size()));
    // Get the StateSet for the geoset
    osg::ref_ptr<osg::StateSet> stateSet = thisPtr->getStateSet(action);
    geometry->setStateSet(stateSet.get());
    
    // Add the geoset to a geode
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable(geometry.get());

    // copy name
    std::string name = stateSet->getName();
    if (name != "") {
        geode->setName(name);
    }
    // Add geode to scenegraph
    thisPtr->groupStack.top()->addChild(geode.get());

    return SoCallbackAction::CONTINUE;
}
///////////////////////////////////////////////////////////////
SoCallbackAction::Response 
ConvertFromInventor::preTexture(void* data, SoCallbackAction *,
                                const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    osg::notify(osg::INFO) << "preTexture()  " 
              << node->getTypeId().getName().getString() << std::endl;
#endif
    
    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);
    
    if (thisPtr->soTexStack.size())
        thisPtr->soTexStack.pop();
    thisPtr->soTexStack.push(node);
        
    return SoCallbackAction::CONTINUE;
}
//////////////////////////////////////////////////////////////////////////////////
SoCallbackAction::Response 
ConvertFromInventor::preVRMLAppearance(void* data, SoCallbackAction* action,
                                         const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    osg::notify(osg::INFO) << "preVRMLAppearance()  " 
              << node->getTypeId().getName().getString() << std::endl;
#endif

#ifdef __COIN__
    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    // If there is a VRML appearance node without a texture node, then
    // we push a NULL texture onto the stack
    bool foundTex = false;
    SoChildList *kids = node->getChildren();
    for (int i=0; i<kids->getLength(); i++) {
        SoNode* kid = (SoNode*)kids->get(i);
        if (kid->isOfType(SoVRMLMaterial::getClassTypeId())) {
            thisPtr->appearanceName = kid->getName();
        }
        if (kid->isOfType(SoVRMLTexture::getClassTypeId())) {
            foundTex = true;
        }
    }
    if (!foundTex) {
        thisPtr->soTexStack.push(NULL);
        thisPtr->inAppearanceWithNoTexture = true;
    }
#endif
    return SoCallbackAction::CONTINUE;
}

//////////////////////////////////////////////////////////////////////////////////
SoCallbackAction::Response 
ConvertFromInventor::preVRMLImageTexture(void* data, SoCallbackAction* action,
                                         const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    osg::notify(osg::INFO) << "preVRMLImageTexture()  " 
              << node->getTypeId().getName().getString() << std::endl;
#endif

    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    if (thisPtr->soTexStack.size())
        thisPtr->soTexStack.pop();
    thisPtr->soTexStack.push(node);

    return SoCallbackAction::CONTINUE;
}
//////////////////////////////////////////////////////////////////
void ConvertFromInventor::transformLight(SoCallbackAction* action, 
                                         const SbVec3f& vec, 
                                         osg::Vec3& transVec)
{
    osg::Matrix modelMat;
    modelMat.set((float *)action->getModelMatrix().getValue());

    transVec.set(vec[0], vec[1], vec[2]);
    transVec = modelMat.preMult(transVec);
}
///////////////////////////////////////////////////////////////////
SoCallbackAction::Response 
ConvertFromInventor::preLight(void* data, SoCallbackAction* action, 
                              const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    osg::notify(osg::INFO) << "preLight()  " 
              << node->getTypeId().getName().getString() << std::endl;
#endif

    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);
    static int lightNum = 1;
    
    // Return if the light is not on
    const SoLight* ivLight = (const SoLight*) node;
    if (!ivLight->on.getValue())
        return SoCallbackAction::CONTINUE;

    osg::ref_ptr<osg::Light> osgLight = new osg::Light;
    osgLight->setLightNum(lightNum++);

    const char* name = ivLight->getName().getString();
    osgLight->setName(name);
    
    // Get color and intensity
    SbVec3f lightColor = ivLight->color.getValue();
    float intensity = ivLight->intensity.getValue();

    // Set color and intensity
    osgLight->setDiffuse(osg::Vec4(lightColor[0] * intensity,
                                   lightColor[1] * intensity,
                                   lightColor[2] * intensity, 1));

    if (node->isOfType(SoDirectionalLight::getClassTypeId()))
    {
        SoDirectionalLight *dirLight = (SoDirectionalLight *) node;
        
        osg::Vec3 transVec;
        thisPtr->transformLight(action, dirLight->direction.getValue(), transVec);
        osgLight->setPosition(osg::Vec4(transVec.x(), transVec.y(), 
                                        transVec.z(), 0));
    }
    else if (node->isOfType(SoPointLight::getClassTypeId()))
    {
        SoPointLight* ptLight = (SoPointLight *) node;

        osg::Vec3 transVec;
        thisPtr->transformLight(action, ptLight->location.getValue(), transVec);
        osgLight->setPosition(osg::Vec4(transVec.x(), transVec.y(), 
                                        transVec.z(), 0));
    }
    else if (node->isOfType(SoSpotLight::getClassTypeId()))
    {
        SoSpotLight* spotLight = (SoSpotLight *) node;

        osgLight->setSpotExponent(spotLight->dropOffRate.getValue() * 128.0);
        osgLight->setSpotCutoff(spotLight->cutOffAngle.getValue()*180.0/osg::PI);

        osg::Vec3 transVec;
        thisPtr->transformLight(action, spotLight->location.getValue(), transVec);
        osgLight->setPosition(osg::Vec4(transVec.x(), transVec.y(), 
                                        transVec.z(), 0));

        thisPtr->transformLight(action, spotLight->direction.getValue(),transVec);
        osgLight->setDirection(osg::Vec3(transVec.x(), transVec.y(), 
                                         transVec.z()));
    }
 
    // Add light to list in the current level
    if (thisPtr->lightStack.size())
    {
        LightList lightList;
        lightList = thisPtr->lightStack.top();
        lightList.push_back(osgLight.get());
        thisPtr->lightStack.pop();
        thisPtr->lightStack.push(lightList);
    }

    // add a light source node to the scene graph
    osg::ref_ptr<osg::LightSource> ls = new osg::LightSource();
    ls->setLight(osgLight.get());
    ls->setName(ivLight->getName().getString());
    if (thisPtr->lightGroup == NULL) {
        thisPtr->lightGroup = new osg::Group;
        thisPtr->lightGroup->setName("IvLightGroup");
        thisPtr->_root->addChild(thisPtr->lightGroup.get());
    }
    thisPtr->lightGroup->addChild(ls.get());

    return SoCallbackAction::CONTINUE;
}
///////////////////////////////////////////////////////////////////////////////////////
osg::ref_ptr<osg::StateSet>
ConvertFromInventor::getStateSet(SoCallbackAction* action)
{
    osg::ref_ptr<osg::StateSet> stateSet = new osg::StateSet;
    
    // Inherit modes from the global state
    stateSet->clear();

    // Convert the IV texture to OSG texture if any
    osg::ref_ptr<osg::Texture2D> texture;
    const SoNode *ivTexture = soTexStack.top();
    if (ivTexture)
    {
        osg::notify(osg::INFO)<<"Have texture"<<std::endl;

        // Found a corresponding OSG texture object
        if (ivToOsgTexMap[ivTexture])
            texture = ivToOsgTexMap[ivTexture];
        else
        {
            // Create a new osg texture
            texture = convertIVTexToOSGTex(ivTexture, action);

            // Add the new texture to the database
            ivToOsgTexMap[ivTexture] = texture.get();
        }
        
        stateSet->setTextureAttributeAndModes(0, texture.get(), osg::StateAttribute::ON);
        
        // propogate name
        if(texture.valid())
        {
            std::string name = texture->getName();
            if (name != "")
                stateSet->setName(name);
        }
        // Set the texture environment
        osg::ref_ptr<osg::TexEnv> texEnv = new osg::TexEnv;
        switch (action->getTextureModel())
        {
            case SoTexture2::MODULATE:
                texEnv->setMode(osg::TexEnv::MODULATE);
                break;
            case SoTexture2::DECAL:
                texEnv->setMode(osg::TexEnv::DECAL);
                break;
            case SoTexture2::BLEND:
                texEnv->setMode(osg::TexEnv::BLEND);
                break;
#if defined(__COIN__) && ((COIN_MAJOR_VERSION==2 && COIN_MINOR_VERSION>=2) || (COIN_MAJOR_VERSION>2))
            // SGI's Inventor does not have REPLACE mode, but the Coin 3D library does.
            // Coin supports REPLACE since 2.2 release, TGS Inventor from 4.0.
            // Let's convert to the TexEnv anyway.
            case SoTexture2::REPLACE:
                texEnv->setMode(osg::TexEnv::REPLACE);
                break;
#endif
            default:
                break;

        }
        stateSet->setTextureAttributeAndModes(0,texEnv.get(),osg::StateAttribute::ON);
    }

    SbColor ambient, diffuse, specular, emission;
    float shininess, transparency;

    // Get the material colors
    action->getMaterial(ambient, diffuse, specular, emission,
                shininess, transparency, 0);
    
    // Set transparency
    SbBool hasTextureTransparency = FALSE;
    if (ivTexture) {
      SbVec2s tmp;
      int bpp;
      if (ivTexture->isOfType(SoTexture2::getClassTypeId()))
        ((SoTexture2*)ivTexture)->image.getValue(tmp, bpp);
#ifdef __COIN__
      else
      if (ivTexture->isOfType(SoVRMLImageTexture::getClassTypeId())) {
        const SbImage *img = ((SoVRMLImageTexture*)ivTexture)->getImage();
        if (img) img->getValue(tmp, bpp);
        else bpp = 0;
      }
#endif
      hasTextureTransparency = bpp==4 || bpp==2;
    }

    if (transparency > 0 || hasTextureTransparency)
    {
        osg::ref_ptr<osg::BlendFunc> transparency = new osg::BlendFunc;
        stateSet->setAttributeAndModes(transparency.get(), 
                                       osg::StateAttribute::ON);
    
        // Enable depth sorting for transparent objects
        stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    }
    
    // Set linewidth
    if (action->getLineWidth())
    {
        osg::ref_ptr<osg::LineWidth> lineWidth = new osg::LineWidth;
        lineWidth->setWidth(action->getLineWidth());
        stateSet->setAttributeAndModes(lineWidth.get(), osg::StateAttribute::ON);
    }

    // Set pointsize
    if (action->getPointSize())
    {
        osg::ref_ptr<osg::Point> point = new osg::Point;
        point->setSize(action->getPointSize());
        stateSet->setAttributeAndModes(point.get(), osg::StateAttribute::ON);
    }
    
    // Set draw mode 
    switch (action->getDrawStyle())
    {
        case SoDrawStyle::FILLED:
        {
#if 0
// OSG defaults to filled draw style, so no need to set redundent state.
            osg::PolygonMode *polygonMode = new osg::PolygonMode;
            polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK, 
                                 osg::PolygonMode::FILL);
            stateSet->setAttributeAndModes(polygonMode, osg::StateAttribute::ON);
#endif
            break;
        }
        case SoDrawStyle::LINES:
        {
            osg::ref_ptr<osg::PolygonMode> polygonMode = new osg::PolygonMode;
            polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK, 
                                 osg::PolygonMode::LINE);
            stateSet->setAttributeAndModes(polygonMode.get(), osg::StateAttribute::ON);
            break;
        }
        case SoDrawStyle::POINTS:
        {
            osg::ref_ptr<osg::PolygonMode> polygonMode = new osg::PolygonMode;
            polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK, 
                                 osg::PolygonMode::POINT);
            stateSet->setAttributeAndModes(polygonMode.get(), osg::StateAttribute::ON);
            break;
        }
        case SoDrawStyle::INVISIBLE:
            // check how to handle this in osg.
            break;
    }

    // Set back face culling
    if (action->getShapeType() == SoShapeHints::SOLID)
    {
        osg::ref_ptr<osg::CullFace> cullFace = new osg::CullFace;
        cullFace->setMode(osg::CullFace::BACK);
        stateSet->setAttributeAndModes(cullFace.get(), osg::StateAttribute::ON);
    } 

    // Set lighting
    if (action->getLightModel() == SoLightModel::BASE_COLOR)
        stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    else
    {
        // Set the material
        osg::ref_ptr<osg::Material> material = new osg::Material;

        material->setAmbient(osg::Material::FRONT_AND_BACK, 
                             osg::Vec4(ambient[0], ambient[1], ambient[2], 
                                       1.0 - transparency));
        material->setDiffuse(osg::Material::FRONT_AND_BACK, 
                             osg::Vec4(diffuse[0], diffuse[1], diffuse[2], 
                                       1.0 - transparency));
        material->setSpecular(osg::Material::FRONT_AND_BACK, 
                              osg::Vec4(specular[0], specular[1], specular[2], 
                                        1.0 - transparency));
        material->setEmission(osg::Material::FRONT_AND_BACK, 
                              osg::Vec4(emission[0], emission[1], emission[2], 
                                        1.0 - transparency));
        material->setTransparency(osg::Material::FRONT_AND_BACK, transparency);
        if (specular[0] || specular[1] || specular[2])
            material->setShininess(osg::Material::FRONT_AND_BACK, 
                                   shininess*128.0);
        else
            material->setShininess(osg::Material::FRONT_AND_BACK, 0.0);

        material->setColorMode(osg::Material::DIFFUSE);

        stateSet->setAttributeAndModes(material.get(), osg::StateAttribute::ON);
        stateSet->setName(appearanceName.getString());
        stateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON);

#if 0
// disable as two sided lighting causes problem under NVidia, and the above osg::Material settings are single sided anway..
        // Set two sided lighting
        osg::LightModel* lightModel = new osg::LightModel;
        lightModel->setTwoSided(true);
        stateSet->setAttributeAndModes(lightModel, osg::StateAttribute::ON);
#endif
        // Set lights
        LightList lightList = lightStack.top();
        for (unsigned int i = 0; i < lightList.size(); i++)
            stateSet->setAttributeAndModes(lightList[i], 
                                           osg::StateAttribute::ON);
    }
    
    return stateSet;
}
////////////////////////////////////////////////////////////////////
osg::Texture2D*
ConvertFromInventor::convertIVTexToOSGTex(const SoNode* soNode,
                                          SoCallbackAction* action)
{
    osg::notify(osg::INFO)<<"convertIVTexToOSGTex of type "<<
        soNode->getTypeId().getName().getString()<<std::endl;

    SbVec2s soSize;
    int soNC;

    // Get the texture size and components
    const unsigned char* soImageData = action->getTextureImage(soSize, soNC);
    if (!soImageData) {
        osg::notify(osg::WARN) << "IV import warning: Error while loading texture data." << std::endl;
        return NULL;
    }

    // Allocate memory for image data
    unsigned char* osgImageData = new unsigned char[soSize[0] * soSize[1] * soNC];

    // Copy the texture image data from the inventor texture
    memcpy(osgImageData, soImageData, soSize[0] * soSize[1] * soNC);

    // Copy the name
    std::string name = soNode->getName().getString();

    // File name
    std::string fileName;
    if (soNode->isOfType(SoTexture2::getClassTypeId()))
        fileName = ((SoTexture2*)soNode)->filename.getValue().getString();
#ifdef __COIN__
    else
    if (soNode->isOfType(SoVRMLImageTexture::getClassTypeId()))
        fileName = ((SoVRMLImageTexture*)soNode)->url.getNum() >= 1 ? 
                   ((SoVRMLImageTexture*)soNode)->url.getValues(0)[0].getString() : "";
#endif
    else
      osg::notify(osg::WARN) << "IV import warning: Unsupported texture type: "
            << soNode->getTypeId().getName().getString() << std::endl;

    osg::notify(osg::INFO) << fileName << " -> ";
    if (fileName[0]=='\"') fileName.erase(fileName.begin());
    if (fileName.size() > 0 && fileName[fileName.size()-1]=='\"') 
        fileName.erase(fileName.begin()+fileName.size()-1);
    osg::notify(osg::INFO) << fileName << std::endl;

    // Create the osg::Image 
    osg::ref_ptr<osg::Image> osgImage = new osg::Image;
    osgImage->setFileName(fileName);
    GLenum formats[] = {GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA};
    osgImage->setImage(soSize[0], soSize[1], 0, soNC, formats[soNC-1],
                       GL_UNSIGNED_BYTE, osgImageData, osg::Image::USE_NEW_DELETE);

    // Create the osg::Texture2D
    osg::Texture2D *osgTex = new osg::Texture2D;
    osgTex->setImage(osgImage.get());
    if (name != "") {
        osgTex->setName(name);
    }

    static std::map<SoTexture2::Wrap, osg::Texture2D::WrapMode> texWrapMap;
    static bool firstTime = true;
    if (firstTime)
    {
        texWrapMap[SoTexture2::CLAMP] = osg::Texture2D::CLAMP;
        texWrapMap[SoTexture2::REPEAT] = osg::Texture2D::REPEAT;
        firstTime = false;
    }
         
    // Set texture wrap mode
#ifdef __COIN__
    if (soNode->isOfType(SoVRMLImageTexture::getClassTypeId())) {
        // It looks like there is a high probability of bug in Coin (investigated on version 2.4.6).
        // action->getTextureWrap() returns correct value on SoTexture2 (SoTexture2::CLAMP = 0x2900,
        // and REPEAT = 0x2901), but SoVRMLImageTexture returns incorrect value of
        // SoGLImage::REPEAT = 0, CLAMP = 1, CLAMP_TO_EDGE = 2).
        // So, let's not use action and try to get correct value directly from texture node.
        // PCJohn-2007-04-22
        osgTex->setWrap(osg::Texture2D::WRAP_S, ((SoVRMLImageTexture*)soNode)->repeatS.getValue() ?
            osg::Texture2D::REPEAT : osg::Texture2D::CLAMP_TO_EDGE);
        osgTex->setWrap(osg::Texture2D::WRAP_T, ((SoVRMLImageTexture*)soNode)->repeatT.getValue() ?
            osg::Texture2D::REPEAT : osg::Texture2D::CLAMP_TO_EDGE);
    }
    else
#endif
    {
        // Proper way to determine wrap mode
        osgTex->setWrap(osg::Texture2D::WRAP_S, texWrapMap[action->getTextureWrapS()]);
        osgTex->setWrap(osg::Texture2D::WRAP_T, texWrapMap[action->getTextureWrapT()]);
    }

    return osgTex; 
}
///////////////////////////////////////////////////////////////////
SoCallbackAction::Response 
ConvertFromInventor::preInfo(void* data, SoCallbackAction* action, 
                              const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    osg::notify(osg::INFO) << "preInfo()    " 
              << node->getTypeId().getName().getString() << std::endl;
#endif
    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);
    SoInfo* info = (SoInfo*)node;
    thisPtr->transformInfoName = info->string.getValue();

    return SoCallbackAction::CONTINUE;
}
    
///////////////////////////////////////////////////////////////////
SoCallbackAction::Response 
ConvertFromInventor::preGroup(void* data, SoCallbackAction* action, 
                              const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    osg::notify(osg::INFO) << "preGroup()    " 
              << node->getTypeId().getName().getString() << std::endl;
#endif

    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    // Create a new Group or LOD and add it to the stack
    osg::ref_ptr<osg::Group> group;
    if (node->isOfType(SoLOD::getClassTypeId())) {
        group = new osg::LOD;
    }
    else {
        group = new osg::Group;
    }


    thisPtr->groupStack.top()->addChild(group.get());
    thisPtr->groupStack.push(group.get());
    
    // SoTransform nodes are not the parent of the nodes they apply to
    // But are in the same separator as them 
    SoChildList *kids = node->getChildren();
    for (int i=0; i<kids->getLength(); i++) {
        SoNode* kid = (SoNode*)kids->get(i);
        if (kid->isOfType(SoTransform::getClassTypeId())) {
            SoTransform* t = (SoTransform*)kid;
            SbVec3f axis, center, trans, scale;
            float angle;

            center = t->center.getValue();
            t->rotation.getValue(axis, angle);
            trans = t->translation.getValue();
            scale = t->scaleFactor.getValue();
            std::string name = t->getName().getString();

            thisPtr->addMatrixTransform(name, axis, angle, center, trans, scale);
        }
    }
#ifdef __COIN__
    if (node->isOfType(SoVRMLTransform::getClassTypeId())) {
        std::string name;
        if (thisPtr->transformInfoName != "") {
            name = std::string("INFO_");
            name += thisPtr->transformInfoName.getString();
            name += "_trans";
        }
        else {
            name = node->getName();
        }
        
        SoVRMLTransform* vt = (SoVRMLTransform*)node;
        SbVec3f axis, center, trans, scale;
        float angle;

        center = vt->center.getValue();
        vt->rotation.getValue(axis, angle);
        trans = vt->translation.getValue();
        scale = vt->scale.getValue();

        thisPtr->addMatrixTransform(name, axis, angle, center, trans, scale);
    }
#endif
    if (node->isOfType(SoSeparator::getClassTypeId()))
    {
        if (thisPtr->soTexStack.size())
            thisPtr->soTexStack.push(thisPtr->soTexStack.top());
        else
            thisPtr->soTexStack.push(NULL);
        if (thisPtr->lightStack.size())
        {
            LightList lightList = thisPtr->lightStack.top();
            thisPtr->lightStack.push(lightList);
        }
    }

    return SoCallbackAction::CONTINUE;
}
//////////////////////////////////////////////////////////////
SoCallbackAction::Response 
ConvertFromInventor::postGroup(void* data, SoCallbackAction* action, 
                               const SoNode* node)
{
    // Handle SoLOD nodes specially
    if (node->isOfType(SoLOD::getClassTypeId()))
        return postLOD(data, action, node);

#ifdef DEBUG_IV_PLUGIN
    osg::notify(osg::INFO) << "postGroup()   " 
              << node->getTypeId().getName().getString() << std::endl;
#endif
    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    // Pop all the groups that are Transforms
    osg::ref_ptr<osg::Group> group = thisPtr->groupStack.top();
    while (strcmp(group->className(), "MatrixTransform") == 0)
    {
        thisPtr->groupStack.pop();
        group = thisPtr->groupStack.top();
    }

    // Pop the group from the stack
    thisPtr->groupStack.pop();

    // Pop the state if the group is a Separator
    if (node->isOfType(SoSeparator::getClassTypeId()))
    {
        thisPtr->soTexStack.pop();
        thisPtr->lightStack.pop();
    }
 
    return SoCallbackAction::CONTINUE;
}
////////////////////////////////////////////////////////////
SoCallbackAction::Response 
ConvertFromInventor::postLOD(void* data, SoCallbackAction *, 
                             const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    osg::notify(osg::INFO) << "postLOD()    " 
              << node->getTypeId().getName().getString() << std::endl;
#endif

    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    // Inventor and OSG LOD node
    SoLOD *ivLOD = (SoLOD *) node;
    osg::LOD *lod = dynamic_cast<osg::LOD*>(thisPtr->groupStack.top());

    // Get the center of LOD and set it
    SbVec3f ivCenter = ivLOD->center.getValue();
    lod->setCenter(osg::Vec3(ivCenter[0], ivCenter[1], ivCenter[2]));

    // Verify the number of children and range values
    int num = thisPtr->groupStack.top()->getNumChildren();
    if (ivLOD->range.getNum()+1 != num && !(num == 0 && ivLOD->range.getNum() == 0)) {
        osg::notify(osg::WARN) << "IV import warning: SoLOD does not "
            << "contain correct data in range field." << std::endl;
        if (ivLOD->range.getNum()+1 < num) {
            thisPtr->groupStack.top()->removeChildren(ivLOD->range.getNum() + 1,
                                                      num - ivLOD->range.getNum() - 1);
            num = ivLOD->range.getNum() + 1;
        }
    }

    // Get the ranges and set it
    if (num > 0) {
        if (num == 1)
            lod->setRange(0, 0.0, FLT_MAX);
        else {
            lod->setRange(0, 0.0, ivLOD->range[0]);
            for (int i = 1; i < num-2; i++)
                lod->setRange(i, ivLOD->range[i-1], ivLOD->range[i]);
            lod->setRange(num-1, ivLOD->range[num-2], FLT_MAX);
        }
    }

    // Pop the group from the stack
    thisPtr->groupStack.pop();

    return SoCallbackAction::CONTINUE;
}
/////////////////////////////////////////////////////////////
SoCallbackAction::Response 
ConvertFromInventor::preRotor(void* data, SoCallbackAction *, 
                              const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    osg::notify(osg::INFO) << "preRotor()  " 
              << node->getTypeId().getName().getString() << std::endl;
#endif

    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    // Get the parameters for the inventor Rotor
    SoRotor *ivRotor = (SoRotor *) node;
    SbVec3f ivAxis;
    float angle;
    ivRotor->rotation.getValue(ivAxis, angle);

    // Create a new osg::MatrixTransform
    osg::ref_ptr<osg::MatrixTransform> rotorTransform = new osg::MatrixTransform;
    
    // Create a Rotor Callback equivalent to the inventor Rotor
    osg::Vec3 pivot(0, 0, 0);
    osg::Vec3 axis(ivAxis[0], ivAxis[1], ivAxis[2]);
    osg::ref_ptr<osgUtil::TransformCallback> rotorCallback 
        = new osgUtil::TransformCallback(pivot, axis, 
                                         2 * osg::PI * ivRotor->speed.getValue());

    // Set the app callback
    rotorTransform->setUpdateCallback(rotorCallback.get());
    
    // Push the rotor transform onto the group stack
    thisPtr->groupStack.top()->addChild(rotorTransform.get());
    thisPtr->groupStack.push(rotorTransform.get());

    return SoCallbackAction::CONTINUE;
}
////////////////////////////////////////////////////////////////
SoCallbackAction::Response 
ConvertFromInventor::prePendulum(void* data, SoCallbackAction *, 
                                 const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    osg::notify(osg::INFO) << "prePendulum()  " 
              << node->getTypeId().getName().getString() << std::endl;
#endif

    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    // Get the parameters for the inventor Pendulum
    SoPendulum *ivPendulum = (SoPendulum *) node;
    SbVec3f ivAxis0, ivAxis1;
    float startAngle, endAngle;
    ivPendulum->rotation0.getValue(ivAxis0, startAngle);
    ivPendulum->rotation1.getValue(ivAxis1, endAngle);

    // Create a new osg::MatrixTransform
    osg::ref_ptr<osg::MatrixTransform> pendulumTransform = new osg::MatrixTransform;
    
    // Create a Pendulum Callback equivalent to the inventor Rotor
    osg::Vec3 axis(ivAxis0[0], ivAxis0[1], ivAxis0[2]);
    PendulumCallback* pendulumCallback 
        = new PendulumCallback(axis, startAngle, endAngle,
                               ivPendulum->speed.getValue());

    // Set the app callback
    pendulumTransform->setUpdateCallback(pendulumCallback);
    
    // Push the pendulum transform onto the group stack
    thisPtr->groupStack.top()->addChild(pendulumTransform.get());
    thisPtr->groupStack.push(pendulumTransform.get());

    return SoCallbackAction::CONTINUE;
}
////////////////////////////////////////////////////////////////
SoCallbackAction::Response 
ConvertFromInventor::preShuttle(void* data, SoCallbackAction *, 
                                const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    osg::notify(osg::INFO) << "preShuttle()  " 
              << node->getTypeId().getName().getString() << std::endl;
#endif

    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    // Get the parameters for the inventor Shuttle
    SoShuttle *ivShuttle = (SoShuttle *) node;
    SbVec3f ivStartPos, ivEndPos;
    ivStartPos = ivShuttle->translation0.getValue();
    ivEndPos = ivShuttle->translation1.getValue();

    // Create a new osg::MatrixTransform
    osg::ref_ptr<osg::MatrixTransform> shuttleTransform = new osg::MatrixTransform;
    
    // Create a shuttle Callback equivalent to the inventor Rotor
    osg::Vec3 startPos(ivStartPos[0], ivStartPos[1], ivStartPos[2]);
    osg::Vec3 endPos(ivEndPos[0], ivEndPos[1], ivEndPos[2]);
    ShuttleCallback* shuttleCallback 
        = new ShuttleCallback(startPos, endPos, ivShuttle->speed.getValue());

    // Set the app callback
    shuttleTransform->setUpdateCallback(shuttleCallback);
    
    // Push the shuttle transform onto the group stack
    thisPtr->groupStack.top()->addChild(shuttleTransform.get());
    thisPtr->groupStack.push(shuttleTransform.get());

    return SoCallbackAction::CONTINUE;
}
////////////////////////////////////////////////////////////
void ConvertFromInventor::addVertex(SoCallbackAction* action,
                                    const SoPrimitiveVertex *v, 
                                    int index)
{
    // Get the coordinates of the vertex
    SbVec3f pt = v->getPoint();
    vertices.push_back(osg::Vec3(pt[0], pt[1], pt[2]));

    // Get the normal of the vertex
    SbVec3f norm = v->getNormal();

    if ((normalBinding == osg::Geometry::BIND_PER_VERTEX) ||
        (normalBinding == osg::Geometry::BIND_PER_PRIMITIVE && index == 0))
    {
        if (vertexOrder == CLOCKWISE)
            normals.push_back(osg::Vec3(-norm[0], -norm[1], -norm[2]));
        else
            normals.push_back(osg::Vec3(norm[0], norm[1], norm[2]));
    }

    if (colorBinding == osg::Geometry::BIND_PER_VERTEX ||
            colorBinding == osg::Geometry::BIND_PER_PRIMITIVE)
    {
        // Get the material/color 
        SbColor ambient, diffuse, specular, emission;
        float transparency, shininess;
        action->getMaterial(ambient, diffuse, specular, emission, shininess, 
                            transparency, v->getMaterialIndex());
        if (colorBinding == osg::Geometry::BIND_PER_VERTEX)
            colors.push_back(osg::Vec4(diffuse[0], diffuse[1], diffuse[2],
                                       1.0 - transparency));
        else if (colorBinding == osg::Geometry::BIND_PER_PRIMITIVE && index == 0)
            colors.push_back(osg::Vec4(diffuse[0], diffuse[1], diffuse[2],
                                       1.0 - transparency));
    }

    // Get the texture coordinates
    SbVec4f texCoord = v->getTextureCoords();
    textureCoords.push_back(osg::Vec2(texCoord[0], texCoord[1]));
}
////////////////////////////////////////////////////////////////////////////
void ConvertFromInventor::addMatrixTransform(const std::string& name, SbVec3f axis, float angle, SbVec3f center, SbVec3f trans, SbVec3f scale)
{
    osg::Matrix mat;
    if (trans.length() != 0.0 || name != "")
    {
        mat.makeIdentity();
        mat.setTrans(trans[0], trans[1], trans[2]);
        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform(mat);
        if (name != "") {
            std::string name2 = name;
            name2 += "_t";
            mt->setName(name2);
        }
        groupStack.top()->addChild(mt.get());
        groupStack.push(mt.get());
    }

    if (center.length() != 0.0) {
        mat.makeIdentity();
        mat.setTrans(center[0], center[1], center[2]);
        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform(mat);
        groupStack.top()->addChild(mt.get());
        groupStack.push(mt.get());
    }

    if (angle != 0.0 || name != "")
    {
        osg::Quat q(angle, osg::Vec3f(axis[0], axis[1], axis[2]));
        mat.makeIdentity();
        mat.setRotate(q); 
        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform(mat);
        if (name != "") {
            std::string name2 = name;
            name2 += "_r";
            mt->setName(name2);
        }
        groupStack.top()->addChild(mt.get());
        groupStack.push(mt.get());
    }

    if (center.length() != 0.0) {
        center.negate();
        mat.makeIdentity();
        mat.setTrans(center[0], center[1], center[2]);
        osg::ref_ptr<osg::MatrixTransform>  mt = new osg::MatrixTransform(mat);
        groupStack.top()->addChild(mt.get());
        groupStack.push(mt.get());
    }

    if (scale[0] != 1.0 || scale[1] != 1.0 || scale[2] != 1.0)    {
        mat.makeIdentity();
        mat.makeScale(scale[0], scale[1], scale[2]);
        osg::ref_ptr<osg::MatrixTransform>  smt = new osg::MatrixTransform(mat);
        groupStack.top()->addChild(smt.get());
        groupStack.push(smt.get());
    }
}
////////////////////////////////////////////////////////////////////////////
void ConvertFromInventor::addTriangleCB(void* data, SoCallbackAction* action,
                                        const SoPrimitiveVertex* v0,
                                        const SoPrimitiveVertex* v1,
                                        const SoPrimitiveVertex* v2)
{
    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);
    
    switch (thisPtr->vertexOrder)
    {
        case CLOCKWISE:
            thisPtr->addVertex(action, v0, 0);
            thisPtr->addVertex(action, v2, 1);
            thisPtr->addVertex(action, v1, 2);
            break;
        case COUNTER_CLOCKWISE:
            thisPtr->addVertex(action, v0, 0);
            thisPtr->addVertex(action, v1, 1);
            thisPtr->addVertex(action, v2, 2);
            break;
    }

    thisPtr->numPrimitives++;
    thisPtr->primitiveType = osg::PrimitiveSet::TRIANGLES;
}
////////////////////////////////////////////////////////////////////////////////
void ConvertFromInventor::addLineSegmentCB(void* data, SoCallbackAction* action,
                                           const SoPrimitiveVertex* v0,
                                           const SoPrimitiveVertex* v1)
{
    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    thisPtr->addVertex(action, v0, 0);
    thisPtr->addVertex(action, v1, 1);

    thisPtr->numPrimitives++;
    thisPtr->primitiveType = osg::PrimitiveSet::LINES;
}
//////////////////////////////////////////////////////////////////////////
void ConvertFromInventor::addPointCB(void* data, SoCallbackAction* action,
                                     const SoPrimitiveVertex* v0)
{
    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    thisPtr->addVertex(action, v0, 0);

    thisPtr->numPrimitives++;
    thisPtr->primitiveType = osg::PrimitiveSet::POINTS;
}
