#include "ConvertFromInventor.h"
#include "PendulumCallback.h"
#include "ShuttleCallback.h"

// OSG headers
#include <osg/MatrixTransform>
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
#include <Inventor/misc/SoChildList.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/SbLinear.h>

#include "GroupSoLOD.h"

#include <map>
#include <math.h>
#ifdef __linux
#include <values.h>
#endif
#ifdef __APPLE__
#include <float.h>
#endif

#define DEBUG_IV_PLUGIN

ConvertFromInventor::ConvertFromInventor()
{
    numPrimitives = 0;
}

ConvertFromInventor::~ConvertFromInventor()
{
}

osg::Node* ConvertFromInventor::convert(SoNode* rootIVNode)
{    
    // Transformation matrix for converting Inventor coordinate system to OSG 
    // coordinate system
    osg::Matrix ivToOSGMat(osg::Matrix(1.0, 0.0, 0.0, 0.0,
                                       0.0, 0.0, 1.0, 0.0,
                                       0.0,-1.0, 0.0, 0.0,
                                       0.0, 0.0, 0.0, 1.0));

    // Create a root node and push it onto the stack
    osg::MatrixTransform* root = new osg::MatrixTransform;
    root->setMatrix(ivToOSGMat);
    groupStack.push(root);

    // Push an empty list of light and push it onto the light stack
    LightList lightList;
    lightStack.push(lightList);
    
    // Create callback actions for the inventor nodes 
    // These callback functions perform the conversion
    SoCallbackAction cbAction;
    cbAction.addPreCallback(SoShape::getClassTypeId(), preShape, this);
    cbAction.addPostCallback(SoShape::getClassTypeId(), postShape, this);
    cbAction.addPreCallback(SoGroup::getClassTypeId(), preGroup, this);
    cbAction.addPostCallback(SoGroup::getClassTypeId(), postGroup, this);
    cbAction.addPreCallback(SoTexture2::getClassTypeId(), preTexture, this);
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

    // Pop the root osg node
    groupStack.pop();

    lightStack.pop();

    return root; 
}

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

// OSG doesn't seem to have a transpose function for matrices
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
    osg::Geometry* geometry = new osg::Geometry;

    // Get the modeling matrix
    osg::Matrix modelMat;
    modelMat.set((float *)action->getModelMatrix().getValue());

    // Tranform the vertices based on the modeling matrix
    osg::Vec3Array* coords = new osg::Vec3Array(thisPtr->vertices.size());
    for (unsigned int i = 0; i < thisPtr->vertices.size(); i++)
        (*coords)[i] = modelMat.preMult(thisPtr->vertices[i]);

    geometry->setVertexArray(coords);


    // Normals need to be transformed using the transpose of the inverse 
    // modeling matrix
    osg::Matrix invModelMat;
    invModelMat.invert(modelMat);
    thisPtr->transposeMatrix(invModelMat);

    // Tranform the normals based on the modeling matrix
    osg::Vec3Array* norms = NULL;
    if (thisPtr->normalBinding == osg::Geometry::BIND_OVERALL)
    {
        norms = new osg::Vec3Array(1);
        const SbVec3f &norm = action->getNormal(0);
        (*norms)[0].set(norm[0], norm[1], norm[2]);
        (*norms)[0] = invModelMat.transform3x3((*norms)[0],invModelMat);
        (*norms)[0].normalize();
    }
    else
    {
        norms = new osg::Vec3Array(thisPtr->normals.size());
        for (unsigned int i = 0; i < thisPtr->normals.size(); i++)
        {
            (*norms)[i] = invModelMat.transform3x3(thisPtr->normals[i], 
                                                invModelMat);
            (*norms)[i].normalize();
        }
    }
    geometry->setNormalArray(norms);
    geometry->setNormalBinding(thisPtr->normalBinding);

    // Set the colors
    osg::Vec4Array* cols;
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
    geometry->setColorArray(cols);
    geometry->setColorBinding(thisPtr->colorBinding);


    if (!(thisPtr->textureCoords.empty()) && action->getNumTextureCoordinates()>0)
    {

        osg::notify(osg::NOTICE)<<"tex coords found"<<std::endl;

        // Get the texture transformation matrix
        osg::Matrix textureMat;
        textureMat.set((float *) action->getTextureMatrix().getValue());

        // Transform texture coordinates if texture matrix is not an identity mat
        osg::Matrix identityMat;
        identityMat.makeIdentity();
        osg::Vec2Array* texCoords 
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

        geometry->setTexCoordArray(0, texCoords);
    }
    
    // Set the parameters for the geometry

    geometry->addPrimitiveSet(new osg::DrawArrays(thisPtr->primitiveType,0,
                                                  coords->size()));
    
    // Get the StateSet for the geoset
    osg::StateSet* stateSet = thisPtr->getStateSet(action);
    geometry->setStateSet(stateSet);
    
    // Add the geoset to a geode
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geometry);

    // Add geode to scenegraph
    thisPtr->groupStack.top()->addChild(geode);

    return SoCallbackAction::CONTINUE;
}

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
        thisPtr->soTexStack.pop() ;
    thisPtr->soTexStack.push((SoTexture2 *)node) ;
        
    return SoCallbackAction::CONTINUE;
}

void ConvertFromInventor::transformLight(SoCallbackAction* action, 
                                         const SbVec3f& vec, 
                                         osg::Vec3& transVec)
{
    osg::Matrix modelMat;
    modelMat.set((float *)action->getModelMatrix().getValue());

    transVec.set(vec[0], vec[1], vec[2]);
    transVec = modelMat.preMult(transVec);
}

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

    osg::Light* osgLight = new osg::Light;
    osgLight->setLightNum(lightNum++);
    
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
        lightList.push_back(osgLight);
        thisPtr->lightStack.pop();
        thisPtr->lightStack.push(lightList);
    }

    return SoCallbackAction::CONTINUE;
}

osg::StateSet* ConvertFromInventor::getStateSet(SoCallbackAction* action)
{
    osg::StateSet* stateSet = new osg::StateSet;
    
    // Inherit modes from the global state
    stateSet->clear();

    SbColor ambient, diffuse, specular, emission;
    float shininess, transparency;

    // Get the material colors
    action->getMaterial(ambient, diffuse, specular, emission,
                shininess, transparency, 0);
    
    // Set transparency
    if (transparency > 0)
    {
        osg::BlendFunc* transparency = new osg::BlendFunc;
        stateSet->setAttributeAndModes(transparency, 
                                       osg::StateAttribute::ON);
    
        // Enable depth sorting for transparent objects
        stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    }
    
    // Set linewidth
    if (action->getLineWidth())
    {
        osg::LineWidth* lineWidth = new osg::LineWidth;
        lineWidth->setWidth(action->getLineWidth());
        stateSet->setAttributeAndModes(lineWidth, osg::StateAttribute::ON);
    }

    // Set pointsize
    if (action->getPointSize())
    {
        osg::Point* point = new osg::Point;
        point->setSize(action->getPointSize());
        stateSet->setAttributeAndModes(point, osg::StateAttribute::ON);
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
            osg::PolygonMode *polygonMode = new osg::PolygonMode;
            polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK, 
                                 osg::PolygonMode::LINE);
            stateSet->setAttributeAndModes(polygonMode, osg::StateAttribute::ON);
            break;
        }
        case SoDrawStyle::POINTS:
        {
            osg::PolygonMode *polygonMode = new osg::PolygonMode;
            polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK, 
                                 osg::PolygonMode::POINT);
            stateSet->setAttributeAndModes(polygonMode, osg::StateAttribute::ON);
            break;
        }
        case SoDrawStyle::INVISIBLE:
            // check how to handle this in osg.
            break;
    }

    // Set back face culling
    if (action->getShapeType() == SoShapeHints::SOLID)
    {
        osg::CullFace* cullFace = new osg::CullFace;
        cullFace->setMode(osg::CullFace::BACK);
        stateSet->setAttributeAndModes(cullFace, osg::StateAttribute::ON);
    } 

    // Set lighting
    if (action->getLightModel() == SoLightModel::BASE_COLOR)
        stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    else
    {
        // Set the material
        osg::Material* material = new osg::Material;

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

        stateSet->setAttributeAndModes(material, osg::StateAttribute::ON);
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
    
    // Convert the IV texture to OSG texture if any
    if (soTexStack.top())
    {
        osg::notify(osg::NOTICE)<<"Have texture"<<std::endl;
    
        osg::Texture2D* tex;
        // Found a corresponding OSG texture object
        if (ivToOsgTexMap[soTexStack.top()])
            tex = ivToOsgTexMap[soTexStack.top()];
        else
        {
            // Create a new osg texture
            tex = convertIVTexToOSGTex(soTexStack.top(), action);

            // Add the new texture to the database
            ivToOsgTexMap[soTexStack.top()] = tex;
        }
        
        stateSet->setTextureAttributeAndModes(0,tex, osg::StateAttribute::ON);

        // Set the texture environment
        osg::TexEnv* texEnv = new osg::TexEnv;
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

#ifdef COIN_BASIC_H
// This check is a very crude Coin detector.
// SGI's Inventor does not have REPLACE mode, but the Coin 3D library does.
            case SoTexture2::REPLACE:
                texEnv->setMode(osg::TexEnv::REPLACE);
                break;
#endif
        }
        stateSet->setTextureAttributeAndModes(0,texEnv,osg::StateAttribute::ON);
    }
    else
    {
        // fallback because many inventor models don't appear to have any SoTexture2..
        SbVec2s soTexSize;
        int soTexNC;
        const unsigned char* texImage = action->getTextureImage(soTexSize, soTexNC);
        if (texImage)
        {
            osg::notify(osg::NOTICE)<<"Image data found soTexSize[0]="<<soTexSize[0]<<", soTexSize[1]="<<soTexSize[1]<<"\tsoTexNC="<<soTexNC<<std::endl;

            // Allocate memory for image data
            unsigned char* imageData = new unsigned char[soTexSize[0] * soTexSize[1] * 
                                                         soTexNC];

            // Copy the texture image data from the inventor texture
            memcpy(imageData, texImage, soTexSize[0] * soTexSize[1] * soTexNC);

            // Create the osg image 
            osg::Image* osgTexImage = new osg::Image;
#if 0
            SbString iv_string;
            soTex->filename.get(iv_string);
            std::string str(iv_string.getString());
            osg::notify(osg::INFO) << str << " -> ";
            if (str[0]=='\"') str.erase(str.begin());
            if (str[str.size()-1]=='\"') str.erase(str.begin()+str.size()-1);
            osg::notify(osg::INFO) << str << std::endl;
            osgTexImage->setFileName(str);
#endif            
            
            GLenum formats[] = {GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA};
            osgTexImage->setImage(soTexSize[0], soTexSize[1], 0, soTexNC,
                    formats[soTexNC-1], GL_UNSIGNED_BYTE, imageData, 
                    osg::Image::USE_NEW_DELETE, -1);

            // Create the osg texture
            osg::Texture2D* osgTex = new osg::Texture2D;
            osgTex->setImage(osgTexImage);

            static std::map<SoTexture2::Wrap, osg::Texture2D::WrapMode> texWrapMap2;
            static bool firstTime2 = true;
            if (firstTime2)
            {
                texWrapMap2[SoTexture2::CLAMP] = osg::Texture2D::CLAMP;
                texWrapMap2[SoTexture2::REPEAT] = osg::Texture2D::REPEAT;
                firstTime2 = false;
            }

            // Set texture wrap mode
            osgTex->setWrap(osg::Texture2D::WRAP_S,texWrapMap2[action->getTextureWrapS()]);
            osgTex->setWrap(osg::Texture2D::WRAP_T,texWrapMap2[action->getTextureWrapT()]);
            
            stateSet->setTextureAttributeAndModes(0,osgTex, osg::StateAttribute::ON);

            // Set the texture environment
            osg::TexEnv* texEnv = new osg::TexEnv;
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

    #ifdef COIN_BASIC_H
    // This check is a very crude Coin detector.
    // SGI's Inventor does not have REPLACE mode, but the Coin 3D library does.
                case SoTexture2::REPLACE:
                    texEnv->setMode(osg::TexEnv::REPLACE);
                    break;
    #endif
            }
            stateSet->setTextureAttributeAndModes(0,texEnv,osg::StateAttribute::ON);
        }

    }
          
    return stateSet;
}

osg::Texture2D* 
ConvertFromInventor::convertIVTexToOSGTex(SoTexture2* soTex, 
                                          SoCallbackAction* action)
{
    osg::notify(osg::NOTICE)<<"convertIVTexToOSGTex"<<std::endl;

    SbVec2s soTexSize;
    int soTexNC;
    
    // Get the texture size and components
    const unsigned char* texImage;
    texImage = soTex->image.getValue(soTexSize, soTexNC);
    if (!texImage)
        return NULL;
    
    // Allocate memory for image data
    unsigned char* imageData = new unsigned char[soTexSize[0] * soTexSize[1] * 
                                                 soTexNC];
    
    // Copy the texture image data from the inventor texture
    memcpy(imageData, texImage, soTexSize[0] * soTexSize[1] * soTexNC);

    // Create the osg image 
    osg::Image* osgTexImage = new osg::Image;
    SbString iv_string;
    soTex->filename.get(iv_string);
    std::string str(iv_string.getString());
    osg::notify(osg::INFO) << str << " -> ";
    if (str[0]=='\"') str.erase(str.begin());
    if (str[str.size()-1]=='\"') str.erase(str.begin()+str.size()-1);
    osg::notify(osg::INFO) << str << std::endl;
    osgTexImage->setFileName(str);
    GLenum formats[] = {GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA};
    osgTexImage->setImage(soTexSize[0], soTexSize[1], 0, soTexNC,
            formats[soTexNC-1], GL_UNSIGNED_BYTE, imageData, 
            osg::Image::USE_NEW_DELETE, -1);

    // Create the osg texture
    osg::Texture2D* osgTex = new osg::Texture2D;
    osgTex->setImage(osgTexImage);

    static std::map<SoTexture2::Wrap, osg::Texture2D::WrapMode> texWrapMap;
    static bool firstTime = true;
    if (firstTime)
    {
        texWrapMap[SoTexture2::CLAMP] = osg::Texture2D::CLAMP;
        texWrapMap[SoTexture2::REPEAT] = osg::Texture2D::REPEAT;
        firstTime = false;
    }
         
    // Set texture wrap mode
    osgTex->setWrap(osg::Texture2D::WRAP_S,texWrapMap[action->getTextureWrapS()]);
    osgTex->setWrap(osg::Texture2D::WRAP_T,texWrapMap[action->getTextureWrapT()]);

    return osgTex; 
}

SoCallbackAction::Response 
ConvertFromInventor::preGroup(void* data, SoCallbackAction* action, 
                              const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    osg::notify(osg::INFO) << "preGroup()    " 
              << node->getTypeId().getName().getString() << std::endl;
#endif

    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    // Handle SoLOD nodes
    if (node->getTypeId() == GroupSoLOD::getClassTypeId())
        return preLOD(data, action, node);

    // Create a new group and add it to the stack
    osg::Group* group = new osg::Group;
    thisPtr->groupStack.push(group);

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

SoCallbackAction::Response 
ConvertFromInventor::postGroup(void* data, SoCallbackAction *, 
                               const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    osg::notify(osg::INFO) << "postGroup()   " 
              << node->getTypeId().getName().getString() << std::endl;
#endif

    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    // Pop all the groups that are Transforms and add it 
    // to the corresponding parent group
    osg::Group* group = thisPtr->groupStack.top();
    while (strcmp(group->className(), "MatrixTransform") == 0)
    {
        thisPtr->groupStack.pop();
        thisPtr->groupStack.top()->addChild(group);
        group = thisPtr->groupStack.top();
    }

    // Pop the group from the stack and add it to it's parent
    thisPtr->groupStack.pop();
    thisPtr->groupStack.top()->addChild(group);

    // Pop the state if the group is a Separator
    if (node->isOfType(SoSeparator::getClassTypeId()))
    {
        thisPtr->soTexStack.pop();
        thisPtr->lightStack.pop();
    }
 
    return SoCallbackAction::CONTINUE;
}

SoCallbackAction::Response 
ConvertFromInventor::preLOD(void* data, SoCallbackAction *, 
                            const SoNode* node)
{
#ifdef DEBUG_IV_PLUGIN
    osg::notify(osg::INFO) << "preLOD()    " 
              << node->getTypeId().getName().getString() << std::endl;
#endif

    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    // Inventor LOD node
    SoLOD *ivLOD = (SoLOD *) node;

    // Create a new LOD and add it to the stack
    osg::LOD* lod = new osg::LOD;
    thisPtr->groupStack.push(lod);

    // Get the center of LOD and set it
    SbVec3f ivCenter = ivLOD->center.getValue();
    lod->setCenter(osg::Vec3(ivCenter[0], ivCenter[1], ivCenter[2]));

    // Get the ranges and set it
    // lod->setRange(0, 0.0);
    lod->setRange(0, 0.0, ivLOD->range[0]);
    for (int i = 1; i < ivLOD->getChildren()->getLength(); i++)
        lod->setRange(i, ivLOD->range[i-1], ivLOD->range[i]);
        
    lod->setRange(ivLOD->getChildren()->getLength(), 
                  ivLOD->range[ivLOD->getChildren()->getLength()],FLT_MAX);

    return SoCallbackAction::CONTINUE;
}

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
    osg::MatrixTransform* rotorTransform = new osg::MatrixTransform;
    
    // Create a Rotor Callback equivalent to the inventor Rotor
    osg::Vec3 pivot(0, 0, 0);
    osg::Vec3 axis(ivAxis[0], ivAxis[1], ivAxis[2]);
    osgUtil::TransformCallback* rotorCallback 
        = new osgUtil::TransformCallback(pivot, axis, 
                                         2 * osg::PI * ivRotor->speed.getValue());

    // Set the app callback
    rotorTransform->setUpdateCallback(rotorCallback);
    
    // Push the rotor transform onto the group stack
    thisPtr->groupStack.push(rotorTransform);

    return SoCallbackAction::CONTINUE;
}

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
    osg::MatrixTransform* pendulumTransform = new osg::MatrixTransform;
    
    // Create a Pendulum Callback equivalent to the inventor Rotor
    osg::Vec3 axis(ivAxis0[0], ivAxis0[1], ivAxis0[2]);
    PendulumCallback* pendulumCallback 
        = new PendulumCallback(axis, startAngle, endAngle,
                               ivPendulum->speed.getValue());

    // Set the app callback
    pendulumTransform->setUpdateCallback(pendulumCallback);
    
    // Push the pendulum transform onto the group stack
    thisPtr->groupStack.push(pendulumTransform);

    return SoCallbackAction::CONTINUE;
}

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
    osg::MatrixTransform* shuttleTransform = new osg::MatrixTransform;
    
    // Create a shuttle Callback equivalent to the inventor Rotor
    osg::Vec3 startPos(ivStartPos[0], ivStartPos[1], ivStartPos[2]);
    osg::Vec3 endPos(ivEndPos[0], ivEndPos[1], ivEndPos[2]);
    ShuttleCallback* shuttleCallback 
        = new ShuttleCallback(startPos, endPos, ivShuttle->speed.getValue());

    // Set the app callback
    shuttleTransform->setUpdateCallback(shuttleCallback);
    
    // Push the shuttle transform onto the group stack
    thisPtr->groupStack.push(shuttleTransform);

    return SoCallbackAction::CONTINUE;
}


void ConvertFromInventor::addVertex(SoCallbackAction* action,
                                    const SoPrimitiveVertex *v, int index)
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

void ConvertFromInventor::addPointCB(void* data, SoCallbackAction* action,
                                     const SoPrimitiveVertex* v0)
{
    ConvertFromInventor* thisPtr = (ConvertFromInventor *) (data);

    thisPtr->addVertex(action, v0, 0);

    thisPtr->numPrimitives++;
    thisPtr->primitiveType = osg::PrimitiveSet::POINTS;
}
