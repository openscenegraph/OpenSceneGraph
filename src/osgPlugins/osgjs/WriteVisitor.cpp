#include "WriteVisitor"
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osg/UserDataContainer>
#include <osg/TextureRectangle>
#include <osg/Texture2D>
#include <osg/Texture1D>
#include <osg/Types>
#include <osg/Material>
#include <osg/BlendFunc>

#include <osgText/Text>

#include <osgAnimation/MorphGeometry>

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


osg::Array* getAnimationBonesArray(osgAnimation::RigGeometry& rigGeometry) {
    for(unsigned int i = 0 ; i < rigGeometry.getNumVertexAttribArrays() ; ++ i) {
        osg::Array* attribute = rigGeometry.getVertexAttribArray(i);
        bool isBones = false;
        if(attribute && attribute->getUserValue("bones", isBones) && isBones) {
            return attribute;
        }
    }
    return 0;
}


osg::Array* getAnimationWeightsArray(osgAnimation::RigGeometry& rigGeometry) {
    for(unsigned int i = 0 ; i < rigGeometry.getNumVertexAttribArrays() ; ++ i) {
        osg::Array* attribute = rigGeometry.getVertexAttribArray(i);
        bool isWeights = false;
        if(attribute && attribute->getUserValue("weights", isWeights) && isWeights) {
            return attribute;
        }
    }
    return 0;
}


osg::ref_ptr<JSONObject> buildRigBoneMap(osgAnimation::RigGeometry& rigGeometry) {
    osg::Array* bones = getAnimationBonesArray(rigGeometry);
    osg::ref_ptr<JSONObject> boneMap = new JSONObject;

    unsigned int paletteIndex = 0;
    while(true) {
        std::ostringstream oss;
        oss << "animationBone_" << paletteIndex;
        std::string boneName, palette = oss.str();
        if(!bones->getUserValue(palette, boneName)) {
            break;
        }
        boneMap->getMaps()[boneName] = new JSONValue<int>(paletteIndex);
        ++ paletteIndex;
    }

    return boneMap;
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

static JSONValue<std::string>* getJSONAlignmentType(osgText::Text::AlignmentType type)
{
    switch(type) {
        case osgText::Text::LEFT_TOP:
            return new JSONValue<std::string>("LEFT_TOP");
        case osgText::Text::LEFT_CENTER:
            return new JSONValue<std::string>("LEFT_CENTER");
        case osgText::Text::LEFT_BOTTOM:
            return new JSONValue<std::string>("LEFT_BOTTOM");
        case osgText::Text::CENTER_TOP:
            return new JSONValue<std::string>("CENTER_TOP");
        case osgText::Text::CENTER_CENTER:
            return new JSONValue<std::string>("CENTER_CENTER");
        case osgText::Text::CENTER_BOTTOM:
            return new JSONValue<std::string>("CENTER_BOTTOM");
        case osgText::Text::RIGHT_TOP:
            return new JSONValue<std::string>("RIGHT_TOP");
        case osgText::Text::RIGHT_CENTER:
            return new JSONValue<std::string>("RIGHT_CENTER");
        case osgText::Text::RIGHT_BOTTOM:
            return new JSONValue<std::string>("RIGHT_BOTTOM");
        case osgText::Text::LEFT_BASE_LINE:
            return new JSONValue<std::string>("LEFT_BASE_LINE");
        case osgText::Text::CENTER_BASE_LINE:
            return new JSONValue<std::string>("CENTER_BASE_LINE");
        case osgText::Text::RIGHT_BASE_LINE:
            return new JSONValue<std::string>("RIGHT_BASE_LINE");
        case osgText::Text::LEFT_BOTTOM_BASE_LINE:
            return new JSONValue<std::string>("LEFT_BOTTOM_BASE_LINE");
        case osgText::Text::CENTER_BOTTOM_BASE_LINE:
            return new JSONValue<std::string>("CENTER_BOTTOM_BASE_LINE");
        case osgText::Text::RIGHT_BOTTOM_BASE_LINE:
            return new JSONValue<std::string>("RIGHT_BOTTOM_BASE_LINE");
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
            if ( !osgDB::getFilePath(baseName).empty())
                ss << osgDB::getFilePath(baseName) << osgDB::getNativePathSeparator();
            ss << (int64_t)image << ".inline_conv_generated.png"; // write the pointer location
            std::string filename = ss.str();
            if (osgDB::writeImageFile(*image, filename)) {
                image->setFileName(filename);
            }
        }

        if (!image->getFileName().empty()) { // means that everything went ok
            if (inlineImages) {

                std::ifstream in(osgDB::findDataFile(image->getFileName()).c_str(), std::ifstream::in | std::ifstream::binary);
                if (in.is_open() && in.good())
                {
                    // read file first to iterate
                    in.seekg(0, std::ifstream::end);
                    const std::ifstream::pos_type size = in.tellg();
                    in.seekg(0, std::ifstream::beg);
                    std::vector<unsigned char> rawData;
                    rawData.resize(size);
                    in.read(reinterpret_cast<char*>(&rawData[0]),size);
                    in.seekg(std::ios_base::beg);

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



JSONObject* WriteVisitor::createJSONOsgSimUserData(osgSim::ShapeAttributeList* osgSimData) {
    JSONObject* jsonUDC = new JSONObject();
    jsonUDC->addUniqueID();

    JSONArray* jsonUDCArray = new JSONArray();
    jsonUDC->getMaps()["Values"] = jsonUDCArray;
    for (unsigned int i = 0; i < osgSimData->size(); i++) {
        const osgSim::ShapeAttribute& attr = (*osgSimData)[i];
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
    return jsonUDC;
}


JSONObject* WriteVisitor::createJSONUserDataContainer(osg::UserDataContainer* container) {
    JSONObject* jsonUDC = new JSONObject();
    jsonUDC->addUniqueID();

    if (!container->getName().empty()) {
        jsonUDC->getMaps()["Name"] = new JSONValue<std::string>(container->getName());
    }
    JSONArray* jsonUDCArray = new JSONArray();
    jsonUDC->getMaps()["Values"] = jsonUDCArray;
    for (unsigned int i = 0; i < container->getNumUserObjects(); i++) {
        osg::Object* o = container->getUserObject(i);
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
    return jsonUDC;
}


void WriteVisitor::translateObject(JSONObject* json, osg::Object* osg)
{
    if (!osg->getName().empty()) {
        json->getMaps()["Name"] = new JSONValue<std::string>(osg->getName());
    }

    JSONObject* jsonUDC = 0;

    osgSim::ShapeAttributeList* osgSimData = dynamic_cast<osgSim::ShapeAttributeList* >(osg->getUserData());
    if (osgSimData) {
        jsonUDC = this->getJSON(osgSimData);
        if(!jsonUDC) {
            jsonUDC = createJSONOsgSimUserData(osgSimData);
            this->setJSON(osgSimData, jsonUDC);
        }
    }
    else if (osg::UserDataContainer* container = osg->getUserDataContainer()) {
        jsonUDC = this->getJSON(container);
        if(!jsonUDC) {
            jsonUDC = createJSONUserDataContainer(container);
            this->setJSON(container, jsonUDC);
        }
    }

    if(jsonUDC) {
        json->getMaps()["UserDataContainer"] = jsonUDC;
    }
}




JSONObject* WriteVisitor::createJSONBufferArray(osg::Array* array, osg::Object* parent)
{
    if (_maps.find(array) != _maps.end())
        return _maps[array]->getShadowObject();

    osg::ref_ptr<JSONBufferArray> json = new JSONBufferArray(array);
    _maps[array] = json;
    if(_mergeAllBinaryFiles) {
        setBufferName(json.get(), parent, array);
    }
    return json.get();
}

JSONObject* WriteVisitor::createJSONDrawElementsUInt(osg::DrawElementsUInt* de, osg::Object* parent)
{
    if (_maps.find(de) != _maps.end())
        return _maps[de]->getShadowObject();

    JSONDrawElements<osg::DrawElementsUInt>* json = new JSONDrawElements<osg::DrawElementsUInt>(*de);
    _maps[de] = json;
    if(_mergeAllBinaryFiles) {
        setBufferName(json, parent, de);
    }
    return json;
}

JSONObject* WriteVisitor::createJSONDrawElementsUShort(osg::DrawElementsUShort* de, osg::Object* parent)
{
    if (_maps.find(de) != _maps.end())
        return _maps[de]->getShadowObject();

    JSONDrawElements<osg::DrawElementsUShort>* json = new JSONDrawElements<osg::DrawElementsUShort>(*de);
    _maps[de] = json;
    if(_mergeAllBinaryFiles) {
        setBufferName(json, parent, de);
    }
    return json;
}

JSONObject* WriteVisitor::createJSONDrawElementsUByte(osg::DrawElementsUByte* de, osg::Object* parent)
{
    if (_maps.find(de) != _maps.end())
        return _maps[de]->getShadowObject();

    JSONDrawElements<osg::DrawElementsUByte>* json = new JSONDrawElements<osg::DrawElementsUByte>(*de);
    _maps[de] = json;
    if(_mergeAllBinaryFiles) {
        setBufferName(json, parent, de);
    }
    return json;
}

// use to convert draw array quads to draw elements triangles
JSONObject* WriteVisitor::createJSONDrawElements(osg::DrawArrays* drawArray, osg::Object* parent)
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
    _maps[drawArray] = json;
    if(_mergeAllBinaryFiles) {
        setBufferName(json, parent, drawArray);
    }
    return json;
}

JSONObject* WriteVisitor::createJSONDrawArray(osg::DrawArrays* da, osg::Object* parent)
{
    if (_maps.find(da) != _maps.end())
        return _maps[da]->getShadowObject();

    osg::ref_ptr<JSONDrawArray> json = new JSONDrawArray(*da);
    _maps[da] = json;
    if(_mergeAllBinaryFiles) {
        setBufferName(json.get(), parent, da);
    }
    return json.get();
}

JSONObject* WriteVisitor::createJSONDrawArrayLengths(osg::DrawArrayLengths* da, osg::Object* parent)
{
    if (_maps.find(da) != _maps.end())
        return _maps[da]->getShadowObject();

    osg::ref_ptr<JSONDrawArrayLengths> json = new JSONDrawArrayLengths(*da);
    _maps[da] = json;
    if(_mergeAllBinaryFiles) {
        setBufferName(json.get(), parent, da);
    }
    return json.get();
}


JSONObject* WriteVisitor::createJSONGeometry(osg::Geometry* geometry, osg::Object* parent)
{
    if(!parent) {
        parent = geometry;
    }

    if (_maps.find(geometry) != _maps.end())
        return _maps[geometry]->getShadowObject();

    osg::ref_ptr<JSONObject> json = new JSONObject;
    json->addUniqueID();
    _maps[geometry] = json;

    if (geometry->getStateSet())
        createJSONStateSet(json.get(), geometry->getStateSet());

    translateObject(json.get(), geometry);

    osg::ref_ptr<JSONObject> attributes = new JSONObject;

    int nbVertexes = 0;

    if (geometry->getVertexArray()) {
        nbVertexes = geometry->getVertexArray()->getNumElements();
        attributes->getMaps()["Vertex"] = createJSONBufferArray(geometry->getVertexArray(), parent);
    }
    if (geometry->getNormalArray()) {
        attributes->getMaps()["Normal"] = createJSONBufferArray(geometry->getNormalArray(), parent);
        int nb = geometry->getNormalArray()->getNumElements();
        if (nbVertexes != nb) {
            osg::notify(osg::FATAL) << "Fatal nb normals " << nb << " != " << nbVertexes << std::endl;
            error();
        }
    }
    if (geometry->getColorArray()) {
        attributes->getMaps()["Color"] = createJSONBufferArray(geometry->getColorArray(), parent);
        int nb = geometry->getColorArray()->getNumElements();
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
        if (geometry->getTexCoordArray(i)) {
            attributes->getMaps()[ss.str()] = createJSONBufferArray(geometry->getTexCoordArray(i), parent);
            int nb = geometry->getTexCoordArray(i)->getNumElements();
            if (nbVertexes != nb) {
                osg::notify(osg::FATAL) << "Fatal nb tex coord " << i << " " << nb << " != " << nbVertexes << std::endl;
                error();
            }
        }
    }

    osg::Array* tangents = getTangentSpaceArray(*geometry);
    if (tangents) {
        attributes->getMaps()["Tangent"] = createJSONBufferArray(tangents, parent);
        int nb = tangents->getNumElements();
        if (nbVertexes != nb) {
            osg::notify(osg::FATAL) << "Fatal nb tangent " << nb << " != " << nbVertexes << std::endl;
            error();
        }
    }

    json->getMaps()["VertexAttributeList"] = attributes;

    if (!geometry->getPrimitiveSetList().empty()) {
        osg::ref_ptr<JSONArray> primitives = new JSONArray();
        for (unsigned int i = 0; i < geometry->getNumPrimitiveSets(); ++i) {
            osg::ref_ptr<JSONObject> obj = new JSONObject;
            osg::PrimitiveSet* primitive = geometry->getPrimitiveSet(i);
            if(!primitive) continue;

            if (primitive->getType() == osg::PrimitiveSet::DrawArraysPrimitiveType) {
                osg::DrawArrays* da = dynamic_cast<osg::DrawArrays*>((primitive));
                if (da)
                {
                    primitives->getArray().push_back(obj);
                    if (da->getMode() == GL_QUADS) {
                        obj->getMaps()["DrawElementsUShort"] = createJSONDrawElements(da, parent);
                    } else {
                        obj->getMaps()["DrawArrays"] = createJSONDrawArray(da, parent);
                    }
                }
            } else if (primitive->getType() == osg::PrimitiveSet::DrawElementsUIntPrimitiveType) {
                osg::DrawElementsUInt* da = dynamic_cast<osg::DrawElementsUInt*>((primitive));
                if (da)
                {
                    primitives->getArray().push_back(obj);
                    obj->getMaps()["DrawElementsUInt"] = createJSONDrawElementsUInt(da, parent);
                }
            }  else if (primitive->getType() == osg::PrimitiveSet::DrawElementsUShortPrimitiveType) {
                osg::DrawElementsUShort* da = dynamic_cast<osg::DrawElementsUShort*>((primitive));
                if (da)
                {
                    primitives->getArray().push_back(obj);
                    obj->getMaps()["DrawElementsUShort"] = createJSONDrawElementsUShort(da, parent);
                }
            }  else if (primitive->getType() == osg::PrimitiveSet::DrawElementsUBytePrimitiveType) {
                osg::DrawElementsUByte* da = dynamic_cast<osg::DrawElementsUByte*>((primitive));
                if (da)
                {
                    primitives->getArray().push_back(obj);
                    obj->getMaps()["DrawElementsUByte"] = createJSONDrawElementsUByte(da, parent);
                }
            }  else if (primitive->getType() == osg::PrimitiveSet::DrawArrayLengthsPrimitiveType) {
                osg::DrawArrayLengths* dal = dynamic_cast<osg::DrawArrayLengths*>((primitive));
                if (dal)
                {
                    primitives->getArray().push_back(obj);
                    obj->getMaps()["DrawArrayLengths"] = createJSONDrawArrayLengths(dal, parent);
                }
            } else {
                osg::notify(osg::WARN) << "Primitive Type " << geometry->getPrimitiveSetList()[i]->getType() << " not supported, skipping" << std::endl;
            }
        }
        json->getMaps()["PrimitiveSetList"] = primitives;
    }
    if (geometry->getComputeBoundingBoxCallback()) {
           osg::ref_ptr<JSONObject> jsonObj = new JSONObject;
           jsonObj->addUniqueID();
           json->getMaps()["osg.ComputeBoundingBoxCallback"] = jsonObj;
    }
    return json.get();
}

JSONObject* WriteVisitor::createJSONRigGeometry(osgAnimation::RigGeometry* rigGeometry)
{
    //TODO : Convert data to JSONVertexArray "Float32Array"
    osg::ref_ptr<JSONObject> json = new JSONObject;
    json->addUniqueID();
    osg::ref_ptr<JSONObject> sourceGeometry = new JSONObject;

    if(osgAnimation::MorphGeometry *morphGeometry = dynamic_cast<osgAnimation::MorphGeometry*>(rigGeometry->getSourceGeometry())) {
        sourceGeometry->getMaps()["osgAnimation.MorphGeometry"] = createJSONMorphGeometry(morphGeometry, rigGeometry);
    }
    else {
        osg::Geometry *geometry  = dynamic_cast<osg::Geometry*>(rigGeometry->getSourceGeometry());
        if(geometry) {
            sourceGeometry->getMaps()["osg.Geometry"] = createJSONGeometry(geometry, rigGeometry);
        }
    }

    json->getMaps()["SourceGeometry"] = sourceGeometry.get();

    osg::Array* bones = getAnimationBonesArray(*rigGeometry);
    osg::Array* weights = getAnimationWeightsArray(*rigGeometry);
    if (bones && weights) {
        json->getMaps()["BoneMap"] = buildRigBoneMap(*rigGeometry);

        json->getMaps()["VertexAttributeList"] = new JSONObject;
        osg::ref_ptr<JSONObject> attributes = json->getMaps()["VertexAttributeList"];
        int nbVertexes = rigGeometry->getSourceGeometry()->getVertexArray()->getNumElements();

        attributes->getMaps()["Bones"] = createJSONBufferArray(bones, rigGeometry);
        attributes->getMaps()["Weights"] = createJSONBufferArray(weights, rigGeometry);
        int nb = bones->getNumElements();
        if (nbVertexes != nb) {
            osg::notify(osg::FATAL) << "Fatal nb bones " << nb << " != " << nbVertexes << std::endl;
            error();
        }
        nb = weights->getNumElements();
        if (nbVertexes != nb) {
            osg::notify(osg::FATAL) << "Fatal nb weights " << nb << " != " << nbVertexes << std::endl;
            error();
        }
    }

    return json.release();
}

JSONObject* WriteVisitor::createJSONMorphGeometry(osgAnimation::MorphGeometry* morphGeometry, osg::Object* parent)
{
    if(!parent) {
        parent = morphGeometry;
    }

    JSONObject* jsonGeometry = createJSONGeometry(morphGeometry, parent);
    osg::ref_ptr<JSONArray> targetList = new JSONArray;

    osgAnimation::MorphGeometry::MorphTargetList mTargetList = morphGeometry->getMorphTargetList();
    typedef osgAnimation::MorphGeometry::MorphTargetList::iterator TargetIterator;

    for(TargetIterator ti = mTargetList.begin(); ti != mTargetList.end(); ti++) {
        osgAnimation::MorphGeometry::MorphTarget *morphTarget = &(*ti);
        if(osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(morphTarget->getGeometry())) {
            osg::ref_ptr<JSONObject> jsonGeometryObject = new JSONObject;
            geometry->setPrimitiveSetList(osg::Geometry::PrimitiveSetList()); //delete unused drawArray
            jsonGeometryObject->getMaps()["osg.Geometry"] = createJSONGeometry(geometry);
            targetList->asArray()->getArray().push_back(jsonGeometryObject);
        }
    }
    jsonGeometry->getMaps()["MorphTargets"] = targetList;

    return jsonGeometry;
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
        mode = new JSONValue<std::string>("FRONT");
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

template <class T>
JSONObject* createImageFromTexture(osg::Texture* texture, JSONObject* jsonTexture, WriteVisitor* writer)
{
    bool inlineImages = writer->getInlineImages();
    int maxTextureDimension = writer->getMaxTextureDimension();
    const std::string baseName = writer->getBaseName();

    T* text = dynamic_cast<T*>( texture);
    if (text) {
        writer->translateObject(jsonTexture,text);
        JSONObject* image = createImage(text->getImage(), inlineImages, maxTextureDimension, baseName);
        if (image)
            jsonTexture->getMaps()["File"] = image;
        return jsonTexture;
    }
    return 0;
}

JSONObject* WriteVisitor::createJSONText(osgText::Text* text)
{
    if (_maps.find(text) != _maps.end())
        return _maps[text]->getShadowObject();

    osg::ref_ptr<JSONObject> jsonText = new JSONObject;
    jsonText->addUniqueID();
    _maps[text] = jsonText;
    jsonText->getMaps()["Text"] = new JSONValue<std::string>( text->getText().createUTF8EncodedString()  );
    jsonText->getMaps()["Position"] = new JSONVec3Array(text->getPosition());
    jsonText->getMaps()["Color"] = new JSONVec4Array(osg::Vec4(text->getColor().x(),text->getColor().y(),text->getColor().z(), text->getColor().w() ));
    jsonText->getMaps()["CharacterSize"] = new JSONValue<float>(text->getCharacterHeight() );
    jsonText->getMaps()["AutoRotateToScreen"] = new JSONValue<int>(text->getAutoRotateToScreen() );
    jsonText->getMaps()["Alignment"] = getJSONAlignmentType(text->getAlignment());

    osg::ref_ptr<JSONValue<std::string> > layout = new JSONValue<std::string>("LEFT_TO_RIGHT");
    if (text->getLayout() == osgText::Text::RIGHT_TO_LEFT) {
        layout = new JSONValue<std::string>("RIGHT_TO_LEFT");
    }
    if (text->getLayout() == osgText::Text::VERTICAL) {
        layout = new JSONValue<std::string>("VERTICAL");
    }
    jsonText->getMaps()["Layout"] = layout;
    return jsonText.release();
}


JSONObject* WriteVisitor::createJSONPagedLOD(osg::PagedLOD *plod)
{
    if (!plod) { return 0; }

    if (_maps.find(plod) != _maps.end()) {
         return _maps[plod]->getShadowObject();
    }

    osg::ref_ptr<JSONObject> jsonPlod = new JSONNode;
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
    osg::ref_ptr<JSONObject> rangeObject = new JSONObject;
    for (unsigned int i =0; i< plod->getRangeList().size(); i++)
    {
        std::stringstream ss;
        ss << "Range ";
        ss << i;
        std::string str = ss.str();

        osg::Vec2 range(plod->getRangeList()[i].first, plod->getRangeList()[i].second);

        // Since OSGJS uses pixel area, use square range
        if (plod->getRangeMode() == osg::LOD::PIXEL_SIZE_ON_SCREEN) {
          range.set(pow(range.x(), 2.0f), pow(range.y(), 2.0f));
        }

        rangeObject->getMaps()[str] = new JSONVec2Array(range);
    }
    jsonPlod->getMaps()["RangeList"] = rangeObject;
    // File List

    osg::ref_ptr<JSONObject> fileObject = new JSONObject;
    for (unsigned int i =0; i< plod->getNumFileNames(); i++)
    {
        std::stringstream ss;
        ss << "File ";
        ss << i;
        std::string str = ss.str();
        // We need to convert first from osg format to osgjs format.
        osg::ref_ptr<osg::Node> n = osgDB::readRefNodeFile(plod->getDatabasePath() + plod->getFileName(i)+".gles");
        if (n)
        {
            std::string filename(osgDB::getNameLessExtension(plod->getFileName(i))+".osgjs");

            std::string fullFilePath(osgDB::getFilePath(_baseName) + osgDB::getNativePathSeparator() + filename);
            fileObject->getMaps()[str] =  new JSONValue<std::string>(_baseLodURL + filename);
            osgDB::makeDirectoryForFile(fullFilePath);
            if (_baseLodURL.empty())
                _baseLodURL = osgDB::getFilePath(filename) + osgDB::getNativePathSeparator() ;
            osg::ref_ptr<osgDB::Options> options =  osgDB::Registry::instance()->getOptions()->cloneOptions();
            options->setPluginStringData(std::string("baseLodURL"), _baseLodURL);

            osgDB::writeNodeFile(*n, fullFilePath, options.get());

        }
        else
            fileObject->getMaps()[str] =  new JSONValue<std::string>("");
     }
    jsonPlod->getMaps()["RangeDataList"] = fileObject;

    return jsonPlod.release();
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
        JSONObject* obj = createImageFromTexture<osg::Texture1D>(texture, jsonTexture.get(), this);
        if (obj) {
            return obj;
        }
    }

    {
        JSONObject* obj = createImageFromTexture<osg::Texture2D>(texture, jsonTexture.get(), this);
        if (obj) {
            return obj;
        }
    }

    {
        JSONObject* obj = createImageFromTexture<osg::TextureRectangle>(texture, jsonTexture.get(), this);
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

    if (jsonStateSet->getMaps().empty())
        return 0;
    return jsonStateSet.release();
}
