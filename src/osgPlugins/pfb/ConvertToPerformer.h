#ifndef __CONVERTTOPERFORMER_H
#define __CONVERTTOPERFORMER_H

#include <map>
#include <vector>
#include <string>

// Open Scene Graph includes.
#include <osg/Node>
#include <osg/NodeVisitor>
#include <osg/Group>
#include <osg/GeoSet>
#include <osg/GeoState>

// Performer includes.
#include <Performer/pf/pfNode.h>

class ConvertToPerformer : protected osg::NodeVisitor {
    public:

        ConvertToPerformer();
        ~ConvertToPerformer();

        pfNode* convert(osg::Node* node);

        virtual void apply(osg::Node&);
        
        virtual void apply(osg::Geode& node);
        virtual void apply(osg::Billboard& node);
        
        virtual void apply(osg::Group& node);
        virtual void apply(osg::DCS& node);
        virtual void apply(osg::Switch& node);
        virtual void apply(osg::LOD& node);
        virtual void apply(osg::Scene& node);


    private:

        pfGroup* _pfParent;
        pfNode* _pfRoot;

        virtual pfObject* getPfObject(osg::Object* osgObj);
        virtual void regisiterOsgObjectForPfObject(osg::Object* osgObj,pfObject* pfObj);

        pfGeoSet* visitGeoSet(osg::GeoSet* geoset);
        pfGeoState* visitGeoState(osg::GeoState* geostate);
        pfMaterial* visitMaterial(osg::Material* material);
        pfTexture* visitTexture(osg::Texture* tex);


        typedef std::map<osg::Object*,pfObject*> OsgObjectToPfObjectMap;
        typedef std::map<osg::GeoSet::PrimitiveType,int> GSetPrimitiveMap;
        typedef std::map<osg::GeoSet::BindingType,int> GSetBindingMap;
        typedef std::map<osg::GeoState::AttributeType,int> GStateTypeMap;
        typedef std::map<osg::GeoState::AttributeMode,int> GStateModeMap;

        OsgObjectToPfObjectMap _osgToPfMap;
        GSetPrimitiveMap    _gsetPrimMap;
        GSetBindingMap      _gsetBindMap;
        GStateTypeMap       _gstateTypeMap;
        GStateModeMap       _gstateModeMap;

};

#endif
