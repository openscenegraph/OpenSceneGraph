/*  -*-c++-*- 
 *  Copyright (C) 2011 Cedric Pinson <cedric.pinson@plopbyte.com>
 */

#include <osgAnimation/Animation>
#include <osgAnimation/Channel>
#include <osgAnimation/Sampler>
#include <osgAnimation/UpdateMatrixTransform>
#include <osgAnimation/StackedTranslateElement>
#include <osgAnimation/StackedQuaternionElement>
#include <osgAnimation/StackedRotateAxisElement>
#include "JSON_Objects"

static bool addJSONChannelVec3(osgAnimation::Vec3LinearChannel* channel, JSONObject& anim)
{
    if (channel && channel->getSampler()) {
        osg::ref_ptr<JSONObject> json = new JSONObject;
        json->getMaps()["Name"] = new JSONValue<std::string>(channel->getName());
        json->getMaps()["TargetName"] = new JSONValue<std::string>(channel->getTargetName());
        osgAnimation::Vec3KeyframeContainer* keys = channel->getSamplerTyped()->getKeyframeContainerTyped();
        JSONKeyframes* jsonKeys = new JSONKeyframes();
        //if (!keys->getName().empty()) {
        //    jsonKeys->getMaps()["Name"] = new JSONValue<std::string>(keys->getName());
        //}

        for (unsigned int i = 0; i < keys->size(); i++) {
            JSONVec4Array* kf = new JSONVec4Array(osg::Vec4((*keys)[i].getTime(),
                                                            (*keys)[i].getValue()[0],
                                                            (*keys)[i].getValue()[1],
                                                            (*keys)[i].getValue()[2]));
            jsonKeys->getArray().push_back(kf);
        }
        json->getMaps()["KeyFrames"] = jsonKeys;
            
        osg::ref_ptr<JSONObject> jsonChannel = new JSONObject();
        jsonChannel->getMaps()["osgAnimation.Vec3LerpChannel"] = json;
        anim.getMaps()["Channels"]->asArray()->getArray().push_back(jsonChannel);
        return true;
    }
    return false;
}



static bool addJSONChannelFloat(osgAnimation::FloatLinearChannel* channel, JSONObject& anim)
{
    if (channel->getSampler()) {
        osg::ref_ptr<JSONObject> json = new JSONObject;
        json->getMaps()["Name"] = new JSONValue<std::string>(channel->getName());
        json->getMaps()["TargetName"] = new JSONValue<std::string>(channel->getTargetName());
        osgAnimation::FloatKeyframeContainer* keys = channel->getSamplerTyped()->getKeyframeContainerTyped();
        JSONKeyframes* jsonKeys = new JSONKeyframes();
        //if (!keys->getName().empty()) {
        //    jsonKeys->getMaps()["Name"] = new JSONValue<std::string>(keys->getName());
        //}

        for (unsigned int i = 0; i < keys->size(); i++) {
            JSONVec2Array* kf = new JSONVec2Array(osg::Vec2((*keys)[i].getTime(),
                                                            (*keys)[i].getValue()));
            jsonKeys->getArray().push_back(kf);
        }
        json->getMaps()["KeyFrames"] = jsonKeys;
            
        osg::ref_ptr<JSONObject> jsonChannel = new JSONObject();
        jsonChannel->getMaps()["osgAnimation.FloatLerpChannel"] = json;
        anim.getMaps()["Channels"]->asArray()->getArray().push_back(jsonChannel);
        return true;
    }
    return false;
}


static bool addJSONChannelQuaternion(osgAnimation::QuatSphericalLinearChannel* channel, JSONObject& anim)
{
    if (channel->getSampler()) {
        osg::ref_ptr<JSONObject> json = new JSONObject;
        json->getMaps()["Name"] = new JSONValue<std::string>(channel->getName());
        json->getMaps()["TargetName"] = new JSONValue<std::string>(channel->getTargetName());
        osgAnimation::QuatKeyframeContainer* keys = channel->getSamplerTyped()->getKeyframeContainerTyped();
        JSONKeyframes* jsonKeys = new JSONKeyframes();

        for (unsigned int i = 0; i < keys->size(); i++) {
            JSONVec5Array* kf = new JSONVec5Array(Vec5((*keys)[i].getTime(),
                                                       (*keys)[i].getValue()[0],
                                                       (*keys)[i].getValue()[1],
                                                       (*keys)[i].getValue()[2],
                                                       (*keys)[i].getValue()[3]));
            jsonKeys->getArray().push_back(kf);
        }
        json->getMaps()["KeyFrames"] = jsonKeys;
            
        osg::ref_ptr<JSONObject> jsonChannel = new JSONObject();
        jsonChannel->getMaps()["osgAnimation.QuatSlerpChannel"] = json;
        anim.getMaps()["Channels"]->asArray()->getArray().push_back(jsonChannel);
        return true;
    }
    return false;
}

static void addJSONChannel(osgAnimation::Channel* channel, JSONObject& anim)
{
    {
        osgAnimation::Vec3LinearChannel* c = dynamic_cast<osgAnimation::Vec3LinearChannel*>(channel);
        if (c) {
            if (addJSONChannelVec3(c, anim))
                return;
        }
    }

    {
        osgAnimation::FloatLinearChannel* c = dynamic_cast<osgAnimation::FloatLinearChannel*>(channel);
        if (c) {
            if (addJSONChannelFloat(c, anim))
                return;
        }
    }

    {
        osgAnimation::QuatSphericalLinearChannel* c = dynamic_cast<osgAnimation::QuatSphericalLinearChannel*>(channel);
        if (c) {
            if (addJSONChannelQuaternion(c, anim))
                return;
        }
    }
    
}

JSONObject* createJSONAnimation(osgAnimation::Animation* anim)
{
    osg::ref_ptr<JSONObject> json = new JSONObject;
    json->getMaps()["Channels"] = new JSONArray();
    json->getMaps()["Name"] = new JSONValue<std::string>(anim->getName());

    for (unsigned int i = 0; i < anim->getChannels().size(); i++) {
        addJSONChannel(anim->getChannels()[i].get(), *json);
    }
    return json.release();
}



JSONObject* createJSONUpdateMatrixTransform(osgAnimation::UpdateMatrixTransform& acb)
{
    std::string name = acb.getName();
    osg::ref_ptr<JSONObject> json = new JSONObject;
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
    }
    if (jsonStackedArray->getArray().empty()) {
        return 0;
    }

    return json.release();
}
