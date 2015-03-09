#include "WriteVisitor"
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osg/UserDataContainer>
#include <osg/TextureRectangle>
#include <osg/Texture2D>
#include <osg/Texture1D>
#include <osg/Material>
#include <osg/BlendFunc>
#include <osgSim/ShapeAttribute>

#include "Base64"



osg::Array* getTangentSpaceArray(osg::Geometry& geometry) {
    for(unsigned int i = 0 ; i < geometry.getNumVertexAttribArrays() ; ++ i) {
        osg::Array* attribute = geometry.getVertexAttribArray(i);
        bool isTangentArray = false;
        if(attribute && attribute->getUserValue("tangent", isTangentArray) && isTangentArray) {
            return attribute;
        }
    }
    return 0;
}

void translateObject(JSONObject* json, osg::Object* osg)
{
    if (!osg->getName().empty()) {
        json->getMaps()["Name"] = new JSONValue<std::string>(osg->getName());
    }

    osgSim::ShapeAttributeList* osgSim_userdata = dynamic_cast<osgSim::ShapeAttributeList* >(osg->getUserData());
    if (osgSim_userdata) {
        JSONObject* jsonUDC = new JSONObject();
        jsonUDC->addUniqueID();

        JSONArray* jsonUDCArray = new JSONArray();
        jsonUDC->getMaps()["Values"] = jsonUDCArray;
        for (unsigned int i = 0; i < osgSim_userdata->size(); i++) {
            const osgSim::ShapeAttribute& attr = (*osgSim_userdata)[i];
            JSONObject* jsonEntry = new JSONObject();
            jsonEntry->getMaps()["Name"] = new JSONValue<std::string>(attr.getName());
            osg::ref_ptr<JSONValue<std::string> > value;
            switch(attr.getType()) {
            case osgSim::ShapeAttribute::INTEGER:
            {
                std::stringstream ss;
                ss << attr.getInt();
                value = new JSONValue<std::string>(ss.str());
            }
            break;
            case osgSim::ShapeAttribute::DOUBLE:
            {
                std::stringstream ss;
                ss << attr.getDouble();
                value = new JSONValue<std::string>(ss.str());
            }
            break;
            case osgSim::ShapeAttribute::STRING:
            {
                std::stringstream ss;
                ss << attr.getString();
                value = new JSONValue<std::string>(ss.str());
            }
            break;
            case osgSim::ShapeAttribute::UNKNOWN:
            default:
                break;
            }
            jsonEntry->getMaps()["Value"] = value;
            jsonUDCArray->getArray().push_back(jsonEntry);
        }
        json->getMaps()["UserDataContainer"] = jsonUDC;

    } else if (osg->getUserDataContainer()) {
        JSONObject* jsonUDC = new JSONObject();
        jsonUDC->addUniqueID();

        if (!osg->getUserDataContainer()->getName().empty()) {
            jsonUDC->getMaps()["Name"] = new JSONValue<std::string>(osg->getUserDataContainer()->getName());
        }
        JSONArray* jsonUDCArray = new JSONArray();
        jsonUDC->getMaps()["Values"] = jsonUDCArray;
        for (unsigned int i = 0; i < osg->getUserDataContainer()->getNumUserObjects(); i++) {
            osg::Object* o = osg->getUserDataContainer()->getUserObject(i);
            std::string name, value;
            getStringifiedUserValue(o, name, value);
            if(!name.empty() && !value.empty())
            {
                JSONObject* jsonEntry = new JSONObject();
                jsonEntry->getMaps()["Name"] = new JSONValue<std::string>(name);
                jsonEntry->getMaps()["Value"] = new JSONValue<std::string>(value);
                jsonUDCArray->getArray().push_back(jsonEntry);
            }

        }
        json->getMaps()["UserDataContainer"] = jsonUDC;
    }
}


void getStringifiedUserValue(osg::Object* o, std::string& name, std::string& value) {
    if(getStringifiedUserValue<std::string>(o, name, value)) return;
    if(getStringifiedUserValue<char>(o, name, value)) return;
    if(getStringifiedUserValue<bool>(o, name, value)) return;
    if(getStringifiedUserValue<short>(o, name, value)) return;
    if(getStringifiedUserValue<unsigned short>(o, name, value)) return;
    if(getStringifiedUserValue<int>(o, name, value)) return;
    if(getStringifiedUserValue<unsigned int>(o, name, value)) return;
    if(getStringifiedUserValue<float>(o, name, value)) return;
    if(getStringifiedUserValue<double>(o, name, value)) return;
}


template<typename T>
bool getStringifiedUserValue(osg::Object* o, std::string& name, std::string& value) {
    osg::TemplateValueObject<T>* vo = dynamic_cast< osg::TemplateValueObject<T>* >(o);
    if (vo) {
        std::ostringstream oss;
        oss << vo->getValue();
        name = vo->getName();
        value = oss.str();
        return true;
    }
    return false;
}


static JSONValue<std::string>* getBlendFuncMode(GLenum mode) {
    switch (mode) {
    case osg::BlendFunc::DST_ALPHA: return new JSONValue<std::string>("DST_ALPHA");
    case osg::BlendFunc::DST_COLOR: return new JSONValue<std::string>("DST_COLOR");
    case osg::BlendFunc::ONE: return new JSONValue<std::string>("ONE");
    case osg::BlendFunc::ONE_MINUS_DST_ALPHA: return new JSONValue<std::string>("ONE_MINUS_DST_ALPHA");
    case osg::BlendFunc::ONE_MINUS_DST_COLOR: return new JSONValue<std::string>("ONE_MINUS_DST_COLOR");
    case osg::BlendFunc::ONE_MINUS_SRC_ALPHA: return new JSONValue<std::string>("ONE_MINUS_SRC_ALPHA");
    case osg::BlendFunc::ONE_MINUS_SRC_COLOR: return new JSONValue<std::string>("ONE_MINUS_SRC_COLOR");
    case osg::BlendFunc::SRC_ALPHA: return new JSONValue<std::string>("SRC_ALPHA");
    case osg::BlendFunc::SRC_ALPHA_SATURATE: return new JSONValue<std::string>("SRC_ALPHA_SATURATE");
    case osg::BlendFunc::SRC_COLOR: return new JSONValue<std::string>("SRC_COLOR");
    case osg::BlendFunc::CONSTANT_COLOR: return new JSONValue<std::string>("CONSTANT_COLOR");
    case osg::BlendFunc::ONE_MINUS_CONSTANT_COLOR: return new JSONValue<std::string>("ONE_MINUS_CONSTANT_COLOR");
    case osg::BlendFunc::CONSTANT_ALPHA: return new JSONValue<std::string>("CONSTANT_ALPHA");
    case osg::BlendFunc::ONE_MINUS_CONSTANT_ALPHA: return new JSONValue<std::string>("ONE_MINUS_CONSTANT_ALPHA");
    case osg::BlendFunc::ZERO: return new JSONValue<std::string>("ZERO");
    default:
        return new JSONValue<std::string>("ONE");
    }
    return new JSONValue<std::string>("ONE");
}

static JSONValue<std::string>* getJSONFilterMode(osg::Texture::FilterMode mode)
{
    switch(mode) {
    case GL_LINEAR:
        return new JSONValue<std::string>("LINEAR");
    case GL_LINEAR_MIPMAP_LINEAR:
        return new JSONValue<std::string>("LINEAR_MIPMAP_LINEAR");
    case GL_LINEAR_MIPMAP_NEAREST:
        return new JSONValue<std::string>("LINEAR_MIPMAP_NEAREST");
    case GL_NEAREST:
        return new JSONValue<std::string>("NEAREST");
    case GL_NEAREST_MIPMAP_LINEAR:
        return new JSONValue<std::string>("NEAREST_MIPMAP_LINEAR");
    case GL_NEAREST_MIPMAP_NEAREST:
        return new JSONValue<std::string>("NEAREST_MIPMAP_NEAREST");
    default:
        return 0;
    }
    return 0;
}

static JSONValue<std::string>* getJSONWrapMode(osg::Texture::WrapMode mode)
{
    switch(mode) {
    case GL_CLAMP:
        // clamp does not exist in opengles 2.0
        //return new JSONValue<std::string>("CLAMP");
        return new JSONValue<std::string>("CLAMP_TO_EDGE");
    case GL_CLAMP_TO_EDGE:
        return new JSONValue<std::string>("CLAMP_TO_EDGE");
    case GL_CLAMP_TO_BORDER_ARB:
        return new JSONValue<std::string>("CLAMP_TO_BORDER");
    case GL_REPEAT:
        return new JSONValue<std::string>("REPEAT");
    case GL_MIRRORED_REPEAT_IBM:
        return new JSONValue<std::string>("MIRROR");
    default:
        return 0;
    }
    return 0;
}


JSONObject* createImage(osg::Image* image, bool inlineImages, int maxTextureDimension, const std::string &baseName)
{
    if (!image) {
        osg::notify(osg::WARN) << "unknown image from texture2d " << std::endl;
        return new JSONValue<std::string>("/unknown.png");
    } else {
        std::string modelDir = osgDB::getFilePath(baseName);
        if (!image->getFileName().empty() && image->getWriteHint() != osg::Image::STORE_INLINE) {
            if(maxTextureDimension) {
                int new_s = osg::Image::computeNearestPowerOfTwo(image->s());
                int new_t = osg::Image::computeNearestPowerOfTwo(image->t());

                bool notValidPowerOf2 = false;
                if (new_s != image->s() || image->s() > maxTextureDimension) notValidPowerOf2 = true;
                if (new_t != image->t() || image->t() > maxTextureDimension) notValidPowerOf2 = true;

                if (notValidPowerOf2) {
                    image->ensureValidSizeForTexturing(maxTextureDimension);
                    if(osgDB::isAbsolutePath(image->getFileName()))
                        osgDB::writeImageFile(*image, image->getFileName());
                    else
                        osgDB::writeImageFile(*image,
                                              osgDB::concatPaths(modelDir,
                                                                 image->getFileName()));
                }
            }
        } else {
            // no image file so use this inline name image and create a file
            std::stringstream ss;
            ss << osgDB::getFilePath(baseName) << osgDB::getNativePathSeparator();
            ss << (long int)image << ".inline_conv_generated.png"; // write the pointer location
            std::string filename = ss.str();
            if (osgDB::writeImageFile(*image, filename)) {
                image->setFileName(filename);
            }
        }

        if (!image->getFileName().empty()) { // means that everything went ok
            if (inlineImages) {

                std::ifstream in(osgDB::findDataFile(image->getFileName()).c_str());
                if (in.is_open())
                {
                    std::stringstream out;
                    out << "data:image/" << osgDB::getLowerCaseFileExtension(image->getFileName()) << ";base64,";
                    base64::encode(std::istreambuf_iterator<char>(in),
                                   std::istreambuf_iterator<char>(),
                                   std::ostreambuf_iterator<char>(out), false);

                    return new JSONValue<std::string>(out.str());

                }
            }
            return new JSONValue<std::string>(image->getFileName());
        }
    }
    return 0;
}


JSONObject* WriteVisitor::createJSONBufferArray(osg::Array* array, osg::Geometry* geom)
{
    if (_maps.find(array) != _maps.end())
        return _maps[array]->getShadowObject();

    osg::ref_ptr<JSONBufferArray> json = new JSONBufferArray(array);
    json->addUniqueID();
    _maps[array] = json;
    if(geom && _mergeAllBinaryFiles) {
        setBufferName(json.get(), geom);
    }
    return json.get();
}

JSONObject* WriteVisitor::createJSONDrawElementsUInt(osg::DrawElementsUInt* de, osg::Geometry* geom)
{
    if (_maps.find(de) != _maps.end())
        return _maps[de]->getShadowObject();

    JSONDrawElements<osg::DrawElementsUInt>* json = new JSONDrawElements<osg::DrawElementsUInt>(*de);
    json->addUniqueID();
    _maps[de] = json;
    if(geom && _mergeAllBinaryFiles) {
        setBufferName(json, geom);
    }
    return json;
}

JSONObject* WriteVisitor::createJSONDrawElementsUShort(osg::DrawElementsUShort* de, osg::Geometry* geom)
{
    if (_maps.find(de) != _maps.end())
        return _maps[de]->getShadowObject();

    JSONDrawElements<osg::DrawElementsUShort>* json = new JSONDrawElements<osg::DrawElementsUShort>(*de);
    json->addUniqueID();
    _maps[de] = json;
    if(geom && _mergeAllBinaryFiles) {
        setBufferName(json, geom);
    }
    return json;
}

JSONObject* WriteVisitor::createJSONDrawElementsUByte(osg::DrawElementsUByte* de, osg::Geometry* geom)
{
    if (_maps.find(de) != _maps.end())
        return _maps[de]->getShadowObject();

    JSONDrawElements<osg::DrawElementsUByte>* json = new JSONDrawElements<osg::DrawElementsUByte>(*de);
    json->addUniqueID();
    _maps[de] = json;
    if(geom && _mergeAllBinaryFiles) {
        setBufferName(json, geom);
    }
    return json;
}

// use to convert draw array quads to draw elements triangles
JSONObject* WriteVisitor::createJSONDrawElements(osg::DrawArrays* drawArray, osg::Geometry* geom)
{
    if (_maps.find(drawArray) != _maps.end())
        return _maps[drawArray]->getShadowObject();

    if (drawArray->getMode() != GL_QUADS) {
        osg::notify(osg::WARN) << "" << std::endl;
        return 0;
    }

    osg::ref_ptr<osg::DrawElementsUShort> de = new osg::DrawElementsUShort(GL_TRIANGLES);

    for (int i = 0; i < drawArray->getCount()/4; ++i) {
        int base = drawArray->getFirst() + i*4;
        de->push_back(base + 0);
        de->push_back(base + 1);
        de->push_back(base + 3);

        de->push_back(base + 1);
        de->push_back(base + 2);
        de->push_back(base + 3);
    }
    JSONDrawElements<osg::DrawElementsUShort>* json = new JSONDrawElements<osg::DrawElementsUShort>(*de);
    json->addUniqueID();
    _maps[drawArray] = json;
    if(geom && _mergeAllBinaryFiles) {
        setBufferName(json, geom);
    }
    return json;
}

JSONObject* WriteVisitor::createJSONDrawArray(osg::DrawArrays* da, osg::Geometry* geom)
{
    if (_maps.find(da) != _maps.end())
        return _maps[da]->getShadowObject();

    osg::ref_ptr<JSONDrawArray> json = new JSONDrawArray(*da);
    json->addUniqueID();
    _maps[da] = json;
    if(geom && _mergeAllBinaryFiles) {
        setBufferName(json.get(), geom);
    }
    return json.get();
}

JSONObject* WriteVisitor::createJSONDrawArrayLengths(osg::DrawArrayLengths* da, osg::Geometry* geom)
{
    if (_maps.find(da) != _maps.end())
        return _maps[da]->getShadowObject();

    osg::ref_ptr<JSONDrawArrayLengths> json = new JSONDrawArrayLengths(*da);
    json->addUniqueID();
    _maps[da] = json;
    if(geom && _mergeAllBinaryFiles) {
        setBufferName(json.get(), geom);
    }
    return json.get();
}


JSONObject* WriteVisitor::createJSONGeometry(osg::Geometry* geom)
{
    if (_maps.find(geom) != _maps.end())
        return _maps[geom]->getShadowObject();

    //if (needToSplit(*geom))
    //    error();

    osg::ref_ptr<JSONObject> json = new JSONNode;
    json->addUniqueID();
    _maps[geom] = json;

    if (geom->getStateSet())
        createJSONStateSet(json.get(), geom->getStateSet());

    translateObject(json.get(), geom);

    osg::ref_ptr<JSONObject> attributes = new JSONObject;

    int nbVertexes = 0;

    if (geom->getVertexArray()) {
        nbVertexes = geom->getVertexArray()->getNumElements();
        attributes->getMaps()["Vertex"] = createJSONBufferArray(geom->getVertexArray(), geom);
    }
    if (geom->getNormalArray()) {
        attributes->getMaps()["Normal"] = createJSONBufferArray(geom->getNormalArray(), geom);
        int nb = geom->getNormalArray()->getNumElements();
        if (nbVertexes != nb) {
            osg::notify(osg::FATAL) << "Fatal nb normals " << nb << " != " << nbVertexes << std::endl;
            error();
        }
    }
    if (geom->getColorArray()) {
        attributes->getMaps()["Color"] = createJSONBufferArray(geom->getColorArray(), geom);
        int nb = geom->getColorArray()->getNumElements();
        if (nbVertexes != nb) {
            osg::notify(osg::FATAL) << "Fatal nb colors " << nb << " != " << nbVertexes << std::endl;
            error();
        }
    }

    std::stringstream ss;
    for ( int i = 0; i < 32; i++) {
        ss.str("");
        ss << "TexCoord" << i;
        //osg::notify(osg::NOTICE) << ss.str() << std::endl;
        if (geom->getTexCoordArray(i)) {
            attributes->getMaps()[ss.str()] = createJSONBufferArray(geom->getTexCoordArray(i), geom);
            int nb = geom->getTexCoordArray(i)->getNumElements();
            if (nbVertexes != nb) {
                osg::notify(osg::FATAL) << "Fatal nb tex coord " << i << " " << nb << " != " << nbVertexes << std::endl;
                error();
            }
        }
    }

    osg::Array* tangents = getTangentSpaceArray(*geom);
    if (tangents) {
        attributes->getMaps()["Tangent"] = createJSONBufferArray(tangents, geom);
        int nb = tangents->getNumElements();
        if (nbVertexes != nb) {
            osg::notify(osg::FATAL) << "Fatal nb tangent " << nb << " != " << nbVertexes << std::endl;
            error();
        }
    }

    json->getMaps()["VertexAttributeList"] = attributes;

    if (!geom->getPrimitiveSetList().empty()) {
        osg::ref_ptr<JSONArray> primitives = new JSONArray();
        for (unsigned int i = 0; i < geom->getNumPrimitiveSets(); ++i) {
            osg::ref_ptr<JSONObject> obj = new JSONObject;
            osg::PrimitiveSet* primitive = geom->getPrimitiveSet(i);
            if(!primitive) continue;

            if (primitive->getType() == osg::PrimitiveSet::DrawArraysPrimitiveType) {
                osg::DrawArrays* da = dynamic_cast<osg::DrawArrays*>((primitive));
                primitives->getArray().push_back(obj);
                if (da->getMode() == GL_QUADS) {
                    obj->getMaps()["DrawElementsUShort"] = createJSONDrawElements(da, geom);
                } else {
                    obj->getMaps()["DrawArrays"] = createJSONDrawArray(da, geom);
                }
            } else if (primitive->getType() == osg::PrimitiveSet::DrawElementsUIntPrimitiveType) {
                osg::DrawElementsUInt* da = dynamic_cast<osg::DrawElementsUInt*>((primitive));
                primitives->getArray().push_back(obj);
                obj->getMaps()["DrawElementsUInt"] = createJSONDrawElementsUInt(da, geom);

            }  else if (primitive->getType() == osg::PrimitiveSet::DrawElementsUShortPrimitiveType) {
                osg::DrawElementsUShort* da = dynamic_cast<osg::DrawElementsUShort*>((primitive));
                primitives->getArray().push_back(obj);
                obj->getMaps()["DrawElementsUShort"] = createJSONDrawElementsUShort(da, geom);

            }  else if (primitive->getType() == osg::PrimitiveSet::DrawElementsUBytePrimitiveType) {
                osg::DrawElementsUByte* da = dynamic_cast<osg::DrawElementsUByte*>((primitive));
                primitives->getArray().push_back(obj);
                obj->getMaps()["DrawElementsUByte"] = createJSONDrawElementsUByte(da, geom);

            }  else if (primitive->getType() == osg::PrimitiveSet::DrawArrayLengthsPrimitiveType) {
                osg::DrawArrayLengths* dal = dynamic_cast<osg::DrawArrayLengths*>((primitive));
                primitives->getArray().push_back(obj);
                obj->getMaps()["DrawArrayLengths"] = createJSONDrawArrayLengths(dal, geom);
            } else {
                osg::notify(osg::WARN) << "Primitive Type " << geom->getPrimitiveSetList()[i]->getType() << " not supported, skipping" << std::endl;
            }
        }
        json->getMaps()["PrimitiveSetList"] = primitives;
    }
    return json.get();
}

JSONObject* WriteVisitor::createJSONBlendFunc(osg::BlendFunc* sa)
{
    if (_maps.find(sa) != _maps.end())
        return _maps[sa]->getShadowObject();

    osg::ref_ptr<JSONObject> json = new JSONObject;
    json->addUniqueID();
    _maps[sa] = json;

    translateObject(json.get(), sa);

    json->getMaps()["SourceRGB"] = getBlendFuncMode(sa->getSource());
    json->getMaps()["DestinationRGB"] = getBlendFuncMode(sa->getDestination());
    json->getMaps()["SourceAlpha"] = getBlendFuncMode(sa->getSourceAlpha());
    json->getMaps()["DestinationAlpha"] = getBlendFuncMode(sa->getDestinationAlpha());
    return json.release();
}

JSONObject* WriteVisitor::createJSONBlendColor(osg::BlendColor* sa)
{
    if (_maps.find(sa) != _maps.end())
        return _maps[sa]->getShadowObject();

    osg::ref_ptr<JSONObject> json = new JSONObject;
    json->addUniqueID();
    _maps[sa] = json;

    translateObject(json.get(), sa);

    json->getMaps()["ConstantColor"] = new JSONVec4Array(sa->getConstantColor());
    return json.release();
}

JSONObject* WriteVisitor::createJSONCullFace(osg::CullFace* sa)
{
    if (_maps.find(sa) != _maps.end())
        return _maps[sa]->getShadowObject();

    osg::ref_ptr<JSONObject> json = new JSONObject;
    json->addUniqueID();
    _maps[sa] = json;

    translateObject(json.get(), sa);

    osg::ref_ptr<JSONValue<std::string> > mode = new JSONValue<std::string>("BACK");
    if (sa->getMode() == osg::CullFace::FRONT) {
        mode = new JSONValue<std::string>("BACK");
    }
    if (sa->getMode() == osg::CullFace::FRONT_AND_BACK) {
        mode = new JSONValue<std::string>("FRONT_AND_BACK");
    }
    json->getMaps()["Mode"] = mode;
    return json.release();
}




JSONObject* WriteVisitor::createJSONMaterial(osg::Material* material)
{
    if (_maps.find(material) != _maps.end())
        return _maps[material]->getShadowObject();

    osg::ref_ptr<JSONObject> jsonMaterial = new JSONMaterial;
    jsonMaterial->addUniqueID();
    _maps[material] = jsonMaterial;

    translateObject(jsonMaterial.get(), material);

    jsonMaterial->getMaps()["Ambient"] = new JSONVec4Array(material->getAmbient(osg::Material::FRONT));
    jsonMaterial->getMaps()["Diffuse"] = new JSONVec4Array(material->getDiffuse(osg::Material::FRONT));
    jsonMaterial->getMaps()["Specular"] = new JSONVec4Array(material->getSpecular(osg::Material::FRONT));
    jsonMaterial->getMaps()["Emission"] = new JSONVec4Array(material->getEmission(osg::Material::FRONT));
    jsonMaterial->getMaps()["Shininess"] = new JSONValue<float>(material->getShininess(osg::Material::FRONT));

    return jsonMaterial.release();
}


JSONObject* WriteVisitor::createJSONLight(osg::Light* light)
{
    if (_maps.find(light) != _maps.end())
        return _maps[light]->getShadowObject();

    osg::ref_ptr<JSONObject> jsonLight = new JSONLight;
    jsonLight->addUniqueID();
    _maps[light] = jsonLight;

    translateObject(jsonLight.get(), light);

    jsonLight->getMaps()["LightNum"] = new JSONValue<int>(light->getLightNum());
    jsonLight->getMaps()["Ambient"] = new JSONVec4Array(light->getAmbient());
    jsonLight->getMaps()["Diffuse"] = new JSONVec4Array(light->getDiffuse());
    jsonLight->getMaps()["Specular"] = new JSONVec4Array(light->getSpecular());
    jsonLight->getMaps()["Position"] = new JSONVec4Array(light->getPosition());
    jsonLight->getMaps()["Direction"] = new JSONVec3Array(light->getDirection());

    jsonLight->getMaps()["ConstantAttenuation"] = new JSONValue<float>(light->getConstantAttenuation());
    jsonLight->getMaps()["LinearAttenuation"] = new JSONValue<float>(light->getLinearAttenuation());
    jsonLight->getMaps()["QuadraticAttenuation"] = new JSONValue<float>(light->getQuadraticAttenuation());
    jsonLight->getMaps()["SpotExponent"] = new JSONValue<float>(light->getSpotExponent());
    jsonLight->getMaps()["SpotCutoff"] = new JSONValue<float>(light->getSpotCutoff());
    return jsonLight.release();
}

template <class T> JSONObject* createImageFromTexture(osg::Texture* texture, JSONObject* jsonTexture, bool inlineImages,
                                                      int maxTextureDimension, const std::string &baseName = "")
{
    T* text = dynamic_cast<T*>( texture);
    if (text) {
        translateObject(jsonTexture,text);
        JSONObject* image = createImage(text->getImage(), inlineImages, maxTextureDimension, baseName);
        if (image)
            jsonTexture->getMaps()["File"] = image;
        return jsonTexture;
    }
    return 0;
}

JSONObject* WriteVisitor::createJSONPagedLOD(osg::PagedLOD *plod)
{
    if (!plod) { return 0; }

    if (_maps.find(plod) != _maps.end()) {
         return _maps[plod]->getShadowObject();
    }

    osg::ref_ptr<JSONObject> jsonPlod = new JSONNode;
    jsonPlod->addUniqueID();
    _maps[plod] = jsonPlod;


    // Center Mode
    osg::ref_ptr<JSONValue<std::string> > centerMode = new JSONValue<std::string>("USE_BOUNDING_SPHERE_CENTER");
    if (plod->getCenterMode() == osg::LOD::USER_DEFINED_CENTER) {
        centerMode = new JSONValue<std::string>("USER_DEFINED_CENTER");
    } else if (plod->getCenterMode() == osg::LOD::UNION_OF_BOUNDING_SPHERE_AND_USER_DEFINED){
        centerMode = new JSONValue<std::string>("UNION_OF_BOUNDING_SPHERE_AND_USER_DEFINED");
    }
    jsonPlod->getMaps()["CenterMode"] = centerMode;
    // User defined center and radius
    jsonPlod->getMaps()["UserCenter"] = new JSONVec4Array(osg::Vec4(plod->getCenter().x(), plod->getCenter().y(),plod->getCenter().z(), plod->getRadius()));


    // Range Mode
    osg::ref_ptr<JSONValue<std::string> > rangeMode = new JSONValue<std::string>("DISTANCE_FROM_EYE_POINT");
    if (plod->getRangeMode() == osg::LOD::PIXEL_SIZE_ON_SCREEN) {
        rangeMode = new JSONValue<std::string>("PIXEL_SIZE_ON_SCREEN");
    }
    jsonPlod->getMaps()["RangeMode"] = rangeMode;
    // Range List
    //osg::ref_ptr<JSONArray> rangeList = new JSONArray;
    JSONObject* rangeObject = new JSONObject;
    for (unsigned int i =0; i< plod->getRangeList().size(); i++)
    {
        std::stringstream ss;
        ss << "Range ";
        ss << i;
        std::string str = ss.str();
        rangeObject->getMaps()[str] =  new JSONVec2Array(osg::Vec2(plod->getRangeList()[i].first, plod->getRangeList()[i].second));
    }
    jsonPlod->getMaps()["RangeList"] = rangeObject;
    // File List

    JSONObject* fileObject = new JSONObject;
    for (unsigned int i =0; i< plod->getNumFileNames(); i++)
    {
        std::stringstream ss;
        ss << "File ";
        ss << i;
        std::string str = ss.str();
        // We need to convert first from osg format to osgjs format.
        osg::ref_ptr<osg::Node> n = osgDB::readNodeFile(plod->getFileName(i)+".gles");
        if (n)
        {
            std::string filename(osgDB::getStrippedName(plod->getFileName(i))+".osgjs");
            osgDB::writeNodeFile(*n,filename);
            fileObject->getMaps()[str] =  new JSONValue<std::string>(filename);
        }
        else
            fileObject->getMaps()[str] =  new JSONValue<std::string>("");
     }
    jsonPlod->getMaps()["RangeDataList"] = fileObject;

    return jsonPlod.get();
}

JSONObject* WriteVisitor::createJSONTexture(osg::Texture* texture)
{
    if (!texture) {
        return 0;
    }

    if (_maps.find(texture) != _maps.end()) {
        return _maps[texture]->getShadowObject();
    }

    osg::ref_ptr<JSONObject> jsonTexture = new JSONObject;
    jsonTexture->addUniqueID();
    _maps[texture] = jsonTexture;

    jsonTexture->getMaps()["MagFilter"] = getJSONFilterMode(texture->getFilter(osg::Texture::MAG_FILTER));
    jsonTexture->getMaps()["MinFilter"] = getJSONFilterMode(texture->getFilter(osg::Texture::MIN_FILTER));

    jsonTexture->getMaps()["WrapS"] = getJSONWrapMode(texture->getWrap(osg::Texture::WRAP_S));
    jsonTexture->getMaps()["WrapT"] = getJSONWrapMode(texture->getWrap(osg::Texture::WRAP_T));


    {
        JSONObject* obj = createImageFromTexture<osg::Texture1D>(texture, jsonTexture.get(), this->_inlineImages,
                                                                 this->_maxTextureDimension, this->_baseName);
        if (obj) {
            return obj;
        }
    }

    {
        JSONObject* obj = createImageFromTexture<osg::Texture2D>(texture, jsonTexture.get(), this->_inlineImages,
                                                                 this->_maxTextureDimension, this->_baseName);
        if (obj) {
            return obj;
        }
    }

    {
        JSONObject* obj = createImageFromTexture<osg::TextureRectangle>(texture, jsonTexture.get(), this->_inlineImages,
                                                                        this->_maxTextureDimension, this->_baseName);
        if (obj) {
            return obj;
        }
    }

    return 0;
}

JSONObject* WriteVisitor::createJSONStateSet(osg::StateSet* stateset)
{

    if (_maps.find(stateset) != _maps.end()) {
        return _maps[stateset]->getShadowObject();
    }

    osg::ref_ptr<JSONObject> jsonStateSet = new JSONStateSet;
    _maps[stateset] = jsonStateSet;
    jsonStateSet->addUniqueID();

    translateObject(jsonStateSet.get(), stateset);

    if (stateset->getRenderingHint() == osg::StateSet::TRANSPARENT_BIN) {
        jsonStateSet->getMaps()["RenderingHint"] = new JSONValue<std::string>("TRANSPARENT_BIN");
    }

    bool blendEnabled = false;
    if (stateset->getMode(GL_BLEND) == osg::StateAttribute::ON) {
        blendEnabled = true;
    }

    osg::ref_ptr<JSONArray> textureAttributeList = new JSONArray;
    int lastTextureIndex = -1;
    for (int i = 0; i < 32; ++i) {
        osg::Texture* texture = dynamic_cast<osg::Texture*>(stateset->getTextureAttribute(i,osg::StateAttribute::TEXTURE));

        JSONArray* textureUnit = new JSONArray;
        JSONObject* jsonTexture = createJSONTexture(texture);
        textureAttributeList->getArray().push_back(textureUnit);

        if (jsonTexture) {
            JSONObject* textureObject = new JSONObject;
            textureObject->getMaps()["osg.Texture"] = jsonTexture;
            textureUnit->getArray().push_back(textureObject);
            lastTextureIndex = i;
        }
    }
    if (lastTextureIndex > -1) {
        textureAttributeList->getArray().resize(lastTextureIndex+1);
        jsonStateSet->getMaps()["TextureAttributeList"] = textureAttributeList;
    }


    osg::ref_ptr<JSONArray> attributeList = new JSONArray;

    osg::Material* material = dynamic_cast<osg::Material*>(stateset->getAttribute(osg::StateAttribute::MATERIAL));
    if (material) {
        JSONObject* obj = new JSONObject;
        obj->getMaps()["osg.Material"] = createJSONMaterial(material);
        attributeList->getArray().push_back(obj);
    }

    osg::BlendFunc* blendFunc = dynamic_cast<osg::BlendFunc*>(stateset->getAttribute(osg::StateAttribute::BLENDFUNC));
    if (blendFunc) {
        JSONObject* obj = new JSONObject;
        obj->getMaps()["osg.BlendFunc"] = createJSONBlendFunc(blendFunc);
        attributeList->getArray().push_back(obj);
    } else if (blendEnabled == true) {
        JSONObject* obj = new JSONObject;
        osg::ref_ptr<osg::BlendFunc> defaultBlend = new osg::BlendFunc();
        obj->getMaps()["osg.BlendFunc"] = createJSONBlendFunc(defaultBlend.get());
        attributeList->getArray().push_back(obj);
    }

    osg::ref_ptr<osg::CullFace> cullFace = dynamic_cast<osg::CullFace*>(stateset->getAttribute(osg::StateAttribute::CULLFACE));
    osg::StateAttribute::GLModeValue cullMode = stateset->getMode(GL_CULL_FACE);
    if (cullFace || cullMode != osg::StateAttribute::INHERIT) {
        JSONObject* obj = new JSONObject;
        JSONObject* cf = 0;
        if (cullMode == osg::StateAttribute::OFF) {
            osg::ref_ptr<osg::CullFace> defaultCull = new osg::CullFace();
            cf = createJSONCullFace(defaultCull.get());
            cf->getMaps()["Mode"] = new JSONValue<std::string>("DISABLE");
            obj->getMaps()["osg.CullFace"] = cf;
            attributeList->getArray().push_back(obj);
        } else {
            if (!cullFace) {
                cullFace = new osg::CullFace();
            }
            cf = createJSONCullFace(cullFace.get());
        }
        obj->getMaps()["osg.CullFace"] = cf;
        attributeList->getArray().push_back(obj);
    }

    osg::BlendColor* blendColor = dynamic_cast<osg::BlendColor*>(stateset->getAttribute(osg::StateAttribute::BLENDCOLOR));
    if (blendColor) {
        JSONObject* obj = new JSONObject;
        obj->getMaps()["osg.BlendColor"] = createJSONBlendColor(blendColor);
        attributeList->getArray().push_back(obj);
    }


    if (!attributeList->getArray().empty()) {
        jsonStateSet->getMaps()["AttributeList"] = attributeList;
    }


    osg::StateSet::ModeList modeList = stateset->getModeList();
    for (unsigned int i = 0; i < modeList.size(); ++i) {
        // add modes
    }

    if (jsonStateSet->getMaps().empty())
        return 0;
    return jsonStateSet.release();
}
