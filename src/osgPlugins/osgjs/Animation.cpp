/*  -*-c++-*-
 *  Copyright (C) 2011 Cedric Pinson <cedric.pinson@plopbyte.com>
 */

#include <osgAnimation/Animation>
#include <osgAnimation/Channel>
#include <osgAnimation/CubicBezier>
#include <osgAnimation/Sampler>
#include <osgAnimation/UpdateMatrixTransform>
#include <osgAnimation/StackedTranslateElement>
#include <osgAnimation/StackedQuaternionElement>
#include <osgAnimation/StackedRotateAxisElement>
#include <osgAnimation/StackedMatrixElement>
#include <osgAnimation/StackedScaleElement>
#include <osg/Array>
#include "JSON_Objects"
#include "WriteVisitor"


template<typename T>
struct osg_array
{ typedef osg::Array array_type; };

#define ADD_ARRAY_TYPE(T, A)\
template<> \
struct osg_array<T> \
{ typedef A array_type; }

ADD_ARRAY_TYPE(osg::Vec3f,     osg::Vec3Array);
ADD_ARRAY_TYPE(osg::Quat,      osg::QuatArray);
ADD_ARRAY_TYPE(osg::Vec3i,     osg::Vec3iArray);
ADD_ARRAY_TYPE(osg::Vec4i,     osg::Vec4iArray);
ADD_ARRAY_TYPE(osg::Vec3ui,    osg::Vec3uiArray);
ADD_ARRAY_TYPE(osg::Vec3us,    osg::Vec3usArray);
ADD_ARRAY_TYPE(float,          osg::FloatArray);
ADD_ARRAY_TYPE(double,         osg::FloatArray);
ADD_ARRAY_TYPE(int,            osg::IntArray);
ADD_ARRAY_TYPE(unsigned int,   osg::UIntArray);
ADD_ARRAY_TYPE(unsigned short, osg::UShortArray);


template<typename T>
bool addJSONChannel(const std::string& channelType, T* channel, bool packByCoords, JSONObject& anim, WriteVisitor* writer, osg::Object* parent) {
    if (channel && channel->getSampler()) {
        osg::ref_ptr<JSONObject> json = new JSONObject;
        std::string jsonType = channelType + (packByCoords ? "Packed" : "");

        writer->translateObject(json.get(), channel);

        json->getMaps()["Name"] = new JSONValue<std::string>(channel->getName());
        json->getMaps()["TargetName"] = new JSONValue<std::string>(channel->getTargetName());

        typename T::KeyframeContainerType* keys = channel->getSamplerTyped()->getKeyframeContainerTyped();
        typedef typename T::UsingType KeyframeType;
        typedef typename osg_array<KeyframeType>::array_type KeyframeArray;

        osg::ref_ptr<JSONObject> jsKeys = new JSONObject;
        osg::ref_ptr<osg::FloatArray> timesArray = new osg::FloatArray;
        osg::ref_ptr<KeyframeArray> valuesArray = new KeyframeArray;

        for (unsigned int i = 0; i < keys->size(); i++) {
            timesArray->push_back((*keys)[i].getTime());
            valuesArray->push_back((*keys)[i].getValue());
        }

        jsKeys->getMaps()["Time"] = writer->createJSONBufferArray(timesArray.get(), parent);

        osg::ref_ptr<KeyframeArray> values;
        if(packByCoords) { // data channel packing
            values = pack<KeyframeArray, KeyframeArray>(valuesArray.get());
        }
        else {
            values = valuesArray;
        }

        jsKeys->getMaps()["Key"] = writer->createJSONBufferArray(values.get(), parent);
        json->getMaps()["KeyFrames"] = jsKeys;

        osg::ref_ptr<JSONObject> jsonChannel = new JSONObject();
        jsonChannel->getMaps()[jsonType] = json;
        anim.getMaps()["Channels"]->asArray()->getArray().push_back(jsonChannel);
        return true;
    }
    return false;
}

static bool addJSONChannelFloat(osgAnimation::FloatLinearChannel* channel, JSONObject& anim, WriteVisitor* writer, osg::Object* parent=0)
{
    if (channel->getSampler()) {
        osg::ref_ptr<JSONObject> json = new JSONObject;
        json->getMaps()["Name"] = new JSONValue<std::string>(channel->getName());
        json->getMaps()["TargetName"] = new JSONValue<std::string>(channel->getTargetName());
        osgAnimation::FloatKeyframeContainer* keys = channel->getSamplerTyped()->getKeyframeContainerTyped();
        osg::ref_ptr<JSONObject> jsKeys = new JSONObject;
        osg::ref_ptr<osg::FloatArray> timesArray = new osg::FloatArray;
        osg::ref_ptr<osg::FloatArray> keysArray = new osg::FloatArray;

        for (unsigned int i = 0; i < keys->size(); i++) {
            timesArray->push_back((*keys)[i].getTime());
            keysArray->push_back((*keys)[i].getValue());
        }

        jsKeys->getMaps()["Time"] = writer->createJSONBufferArray(timesArray.get(), parent);
        jsKeys->getMaps()["Key"] = writer->createJSONBufferArray(keysArray.get(), parent);
        json->getMaps()["KeyFrames"] = jsKeys;

        osg::ref_ptr<JSONObject> jsonChannel = new JSONObject();
        jsonChannel->getMaps()["osgAnimation.FloatLerpChannel"] = json;
        anim.getMaps()["Channels"]->asArray()->getArray().push_back(jsonChannel);
        return true;
    }
    return false;
}

static bool addJSONChannelFloatCubicBezier(osgAnimation::FloatCubicBezierChannel* channel, JSONObject& anim, WriteVisitor* writer, osg::Object* parent=0)
{
    if(channel->getSampler()) {
        osg::ref_ptr<JSONObject> json = new JSONObject;
        json->getMaps()["Name"] = new JSONValue<std::string>(channel->getName());
        json->getMaps()["TargetName"] = new JSONValue<std::string>(channel->getTargetName());

        osgAnimation::FloatCubicBezierKeyframeContainer * keys = channel->getSamplerTyped()->getKeyframeContainerTyped();
        osg::ref_ptr<osg::FloatArray> timeArray = new osg::FloatArray,
                positionArray = new osg::FloatArray,
                controlPointInArray = new osg::FloatArray,
                controlPointOutArray = new osg::FloatArray;

        for (unsigned int i = 0; i < keys->size(); i++) {
            timeArray->push_back((*keys)[i].getTime());
            positionArray->push_back((*keys)[i].getValue().getPosition());
            controlPointInArray->push_back((*keys)[i].getValue().getControlPointIn());
            controlPointOutArray->push_back((*keys)[i].getValue().getControlPointOut());
        }
        osg::ref_ptr<JSONObject> jsKeys = new JSONObject;

        jsKeys->getMaps()["ControlPointOut"] = writer->createJSONBufferArray(controlPointOutArray.get(), parent);

        jsKeys->getMaps()["ControlPointIn"] = writer->createJSONBufferArray(controlPointInArray.get(), parent);

        jsKeys->getMaps()["Position"] = writer->createJSONBufferArray(positionArray.get(), parent);

        jsKeys->getMaps()["Time"] = writer->createJSONBufferArray(timeArray.get(), parent);

        json->getMaps()["KeyFrames"] = jsKeys;

        osg::ref_ptr<JSONObject> jsonChannel = new JSONObject();
        jsonChannel->getMaps()["osgAnimation.FloatCubicBezierChannel"] = json;
        anim.getMaps()["Channels"]->asArray()->getArray().push_back(jsonChannel);
        return true;
    }
    return false;
}

static bool addJSONChannelVec3CubicBezier(osgAnimation::Vec3CubicBezierChannel* channel, JSONObject& anim, WriteVisitor* writer, osg::Object* parent=0)
{
    if(channel->getSampler()) {
        osg::ref_ptr<JSONObject> json = new JSONObject;
        json->getMaps()["Name"] = new JSONValue<std::string>(channel->getName());
        json->getMaps()["TargetName"] = new JSONValue<std::string>(channel->getTargetName());

        osgAnimation::Vec3CubicBezierKeyframeContainer * keys = channel->getSamplerTyped()->getKeyframeContainerTyped();
        osg::ref_ptr<osg::FloatArray> timeArray = new osg::FloatArray,
                positionArrayX = new osg::FloatArray,
                positionArrayY = new osg::FloatArray,
                positionArrayZ = new osg::FloatArray,

                controlPointInArrayX = new osg::FloatArray,
                controlPointInArrayY = new osg::FloatArray,
                controlPointInArrayZ = new osg::FloatArray,

                controlPointOutArrayX = new osg::FloatArray,
                controlPointOutArrayY = new osg::FloatArray,
                controlPointOutArrayZ = new osg::FloatArray;

        for (unsigned int i = 0; i < keys->size(); i++) {
            timeArray->push_back((*keys)[i].getTime());

            positionArrayX->push_back((*keys)[i].getValue().getPosition().x());
            positionArrayY->push_back((*keys)[i].getValue().getPosition().y());
            positionArrayZ->push_back((*keys)[i].getValue().getPosition().z());

            controlPointInArrayX->push_back((*keys)[i].getValue().getControlPointIn().x());
            controlPointInArrayY->push_back((*keys)[i].getValue().getControlPointIn().y());
            controlPointInArrayZ->push_back((*keys)[i].getValue().getControlPointIn().z());

            controlPointOutArrayX->push_back((*keys)[i].getValue().getControlPointOut().x());
            controlPointOutArrayY->push_back((*keys)[i].getValue().getControlPointOut().y());
            controlPointOutArrayZ->push_back((*keys)[i].getValue().getControlPointOut().z());
        }
        osg::ref_ptr<JSONObject> jsKeys = new JSONObject;

        osg::ref_ptr<JSONArray> jsControlPointOutArray = new JSONArray;
        jsControlPointOutArray->asArray()->getArray().push_back(writer->createJSONBufferArray(controlPointOutArrayX.get(), parent));
        jsControlPointOutArray->asArray()->getArray().push_back(writer->createJSONBufferArray(controlPointOutArrayY.get(), parent));
        jsControlPointOutArray->asArray()->getArray().push_back(writer->createJSONBufferArray(controlPointOutArrayZ.get(), parent));
        jsKeys->getMaps()["ControlPointOut"] = jsControlPointOutArray;

        osg::ref_ptr<JSONArray> jsControlPointInArray = new JSONArray;
        jsControlPointInArray->asArray()->getArray().push_back(writer->createJSONBufferArray(controlPointInArrayX.get(), parent));
        jsControlPointInArray->asArray()->getArray().push_back(writer->createJSONBufferArray(controlPointInArrayY.get(), parent));
        jsControlPointInArray->asArray()->getArray().push_back(writer->createJSONBufferArray(controlPointInArrayZ.get(), parent));
        jsKeys->getMaps()["ControlPointIn"] = jsControlPointInArray;

        osg::ref_ptr<JSONArray> jsPositionVertexArray = new JSONArray;
        jsPositionVertexArray->asArray()->getArray().push_back(writer->createJSONBufferArray(positionArrayX.get(), parent));
        jsPositionVertexArray->asArray()->getArray().push_back(writer->createJSONBufferArray(positionArrayY.get(), parent));
        jsPositionVertexArray->asArray()->getArray().push_back(writer->createJSONBufferArray(positionArrayZ.get(), parent));
        jsKeys->getMaps()["Position"] = jsPositionVertexArray;

        jsKeys->getMaps()["Time"] = writer->createJSONBufferArray(timeArray.get(), parent);

        json->getMaps()["KeyFrames"] = jsKeys;

        osg::ref_ptr<JSONObject> jsonChannel = new JSONObject();
        jsonChannel->getMaps()["osgAnimation.Vec3CubicBezierChannel"] = json;
        anim.getMaps()["Channels"]->asArray()->getArray().push_back(jsonChannel);
        return true;
    }
    return false;
}




static void addJSONChannel(osgAnimation::Channel* channel, JSONObject& anim, WriteVisitor* writer, osg::Object* parent=0)
{
    {
        osgAnimation::FloatLinearChannel* c = dynamic_cast<osgAnimation::FloatLinearChannel*>(channel);
        if (c) {
            if (addJSONChannelFloat(c, anim, writer, parent))
                return;
        }
    }

    {
        osgAnimation::Vec3LinearChannel* c = dynamic_cast<osgAnimation::Vec3LinearChannel*>(channel);
        if (c) {
            if (addJSONChannel<osgAnimation::Vec3LinearChannel>("osgAnimation.Vec3LerpChannel", c, false, anim, writer, parent))
                return;
        }
    }

    {
        osgAnimation::QuatSphericalLinearChannel* c = dynamic_cast<osgAnimation::QuatSphericalLinearChannel*>(channel);
        if (c) {
            if (addJSONChannel<osgAnimation::QuatSphericalLinearChannel>("osgAnimation.QuatSlerpChannel", c, false, anim, writer, parent))
                return;
        }
    }

    {
        osgAnimation::FloatCubicBezierChannel* c = dynamic_cast<osgAnimation::FloatCubicBezierChannel*>(channel);
        if (c) {
            if (addJSONChannelFloatCubicBezier(c, anim, writer, parent))
                return;
        }
    }

    {
        osgAnimation::Vec3CubicBezierChannel *c = dynamic_cast<osgAnimation::Vec3CubicBezierChannel*>(channel);
        if (c) {
            if (addJSONChannelVec3CubicBezier(c, anim, writer, parent))
                return;
        }
    }
}

JSONObject* createJSONAnimation(osgAnimation::Animation* anim, WriteVisitor* writer)
{
    osg::ref_ptr<JSONObject> json = new JSONObject;
    json->addUniqueID();
    json->getMaps()["Channels"] = new JSONArray();
    json->getMaps()["Name"] = new JSONValue<std::string>(anim->getName());

    for (unsigned int i = 0; i < anim->getChannels().size(); i++) {
        addJSONChannel(anim->getChannels()[i].get(), *json, writer, anim);
    }
    return json.release();
}

JSONObject* createJSONUpdateMatrixTransform(osgAnimation::UpdateMatrixTransform& acb, WriteVisitor* /*writer*/)
{
    osg::ref_ptr<JSONObject> json = new JSONObject;
    json->addUniqueID();
    json->getMaps()["Name"] = new JSONValue<std::string>(acb.getName());

    osg::ref_ptr<JSONArray> jsonStackedArray = new JSONArray();
    json->getMaps()["StackedTransforms"] = jsonStackedArray;

    osgAnimation::StackedTransform& st = acb.getStackedTransforms();
    for (unsigned int i = 0; i < st.size(); i++) {
        {
            osgAnimation::StackedTranslateElement* element = dynamic_cast<osgAnimation::StackedTranslateElement*>(st[i].get());
            if (element) {
                osg::ref_ptr<JSONObject> jsonElement = new JSONObject;
                jsonElement->getMaps()["Name"] = new JSONValue<std::string>(element->getName());
                jsonElement->getMaps()["Translate"] = new JSONVec3Array(element->getTranslate());

                osg::ref_ptr<JSONObject> jsonElementObject = new JSONObject;
                jsonElementObject->getMaps()["osgAnimation.StackedTranslate"] = jsonElement;
                jsonStackedArray->getArray().push_back(jsonElementObject);
                continue;
            }
        }


        {
            osgAnimation::StackedQuaternionElement* element = dynamic_cast<osgAnimation::StackedQuaternionElement*>(st[i].get());
            if (element) {
                osg::ref_ptr<JSONObject> jsonElement = new JSONObject;
                jsonElement->getMaps()["Name"] = new JSONValue<std::string>(element->getName());
                jsonElement->getMaps()["Quaternion"] = new JSONVec4Array(element->getQuaternion().asVec4());

                osg::ref_ptr<JSONObject> jsonElementObject = new JSONObject;
                jsonElementObject->getMaps()["osgAnimation.StackedQuaternion"] = jsonElement;
                jsonStackedArray->getArray().push_back(jsonElementObject);
                continue;
            }
        }


        {
            osgAnimation::StackedRotateAxisElement* element = dynamic_cast<osgAnimation::StackedRotateAxisElement*>(st[i].get());
            if (element) {
                osg::ref_ptr<JSONObject> jsonElement = new JSONObject;
                jsonElement->getMaps()["Name"] = new JSONValue<std::string>(element->getName());
                jsonElement->getMaps()["Axis"] = new JSONVec3Array(element->getAxis());
                jsonElement->getMaps()["Angle"] = new JSONValue<double>(element->getAngle());

                osg::ref_ptr<JSONObject> jsonElementObject = new JSONObject;
                jsonElementObject->getMaps()["osgAnimation.StackedRotateAxis"] = jsonElement;
                jsonStackedArray->getArray().push_back(jsonElementObject);
                continue;
            }
        }


        {
            osgAnimation::StackedMatrixElement * element = dynamic_cast<osgAnimation::StackedMatrixElement*>(st[i].get());
            if (element) {
                osg::ref_ptr<JSONObject> jsonElement = new JSONObject;
                jsonElement->getMaps()["Name"] = new JSONValue<std::string>(element->getName());
                jsonElement->getMaps()["Matrix"] = new JSONMatrix(element->getMatrix());

                osg::ref_ptr<JSONObject> jsonElementObject = new JSONObject;
                jsonElementObject->getMaps()["osgAnimation.StackedMatrix"] = jsonElement;
                jsonStackedArray->getArray().push_back(jsonElementObject);
                continue;
            }
        }

        {
            osgAnimation::StackedScaleElement * element = dynamic_cast<osgAnimation::StackedScaleElement*>(st[i].get());
            if (element) {
               osg::ref_ptr<JSONObject>  jsonElement = new JSONObject;
               jsonElement->getMaps()["Name"] = new JSONValue<std::string>(element->getName());
               jsonElement->getMaps()["Scale"] = new JSONVec3Array(element->getScale());

               osg::ref_ptr<JSONObject> jsonElementObject = new JSONObject;
               jsonElementObject->getMaps()["osgAnimation.StackedScale"] = jsonElement;
               jsonStackedArray->getArray().push_back(jsonElementObject);
               continue;
            }
        }
    }

    // In the case of a Bone, we serialize the UpdateBone element even if there are no stackedTransforms. The reason is
    // that for now, we use UpdateBone to compute the transforms from bone's parents, and for a bone without UpdateBone we
    // get a bad transform and the related geometries are messed up
    if (jsonStackedArray->getArray().empty() && !dynamic_cast<osgAnimation::UpdateBone*>(&acb)) {
        return 0;
    }

    return json.release();
}
