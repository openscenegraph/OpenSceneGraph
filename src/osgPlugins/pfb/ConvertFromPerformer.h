#ifndef __CONVERTFROMPERFORMER_H
#define __CONVERTFROMPERFORMER_H

#include <map>
#include <vector>
#include <string>

// Open Scene Graph includes.
#include <osg/Node>
#include <osg/Group>
#include <osg/GeoSet>
#include <osg/GeoState>

// Performer includes.
#include <Performer/pf/pfNode.h>

class ConvertFromPerformer {
    public:

        ConvertFromPerformer();
        ~ConvertFromPerformer();

        osg::Node* convert(pfNode* node);

        void setSaveImageDirectory(const std::string& directory) { _saveImageDirectory = directory; }
        void setSaveImagesAsRGB(bool b) { _saveImagesAsRGB=b; }
        void setSaveAbsoluteImagePath(bool b) { _saveAbsoluteImagePath = b; }

    private:

        osg::Object* getOsgObject(pfObject* pfObj);
        void regisiterPfObjectForOsgObject(pfObject* pfObj,osg::Object* osgObj);

        osg::Node* visitNode(osg::Group* osgParent,pfNode* node);
        osg::Node* visitScene(osg::Group* osgParent,pfScene* scene);
        osg::Node* visitGroup(osg::Group* osgParent,pfGroup* group);
        osg::Node* visitDCS(osg::Group* osgParent,pfDCS* dcs);
        osg::Node* visitLOD(osg::Group* osgParent,pfLOD* lod);
        osg::Node* visitSwitch(osg::Group* osgParent,pfSwitch* switchNode);
        osg::Node* visitSequence(osg::Group* osgParent,pfSequence* sequence);
        osg::Node* visitSCS(osg::Group* osgParent,pfSCS* scs);
        osg::Node* visitGeode(osg::Group* osgParent,pfGeode* geode);
        osg::Node* visitBillboard(osg::Group* osgParent,pfBillboard* billboard);

        int getNumVerts(pfGeoSet *gset);
        osg::GeoSet* visitGeoSet(osg::Geode* osgParent,pfGeoSet* geoset);
        osg::GeoState* visitGeoState(osg::GeoSet* osgGeoSet,pfGeoState* geostate);
        osg::Material* visitMaterial(osg::GeoState* osgGeoState,pfMaterial* front_mat,pfMaterial* back_mat);
        osg::Texture* visitTexture(osg::GeoState* osgGeoState,pfTexture* tex);

        typedef std::map<int,osg::GeoSet::PrimitiveType> GSetPrimitiveMap;
        typedef std::map<int,osg::GeoSet::BindingType> GSetBindingMap;
        typedef std::map<int,osg::GeoState::AttributeType> GStateTypeMap;
        typedef std::map<int,osg::GeoState::AttributeMode> GStateModeMap;

        GSetPrimitiveMap    _gsetPrimMap;
        GSetBindingMap      _gsetBindMap;
        GStateTypeMap       _gstateTypeMap;
        GStateModeMap       _gstateModeMap;

        bool _saveImagesAsRGB;
        bool _saveAbsoluteImagePath;
        std::string _saveImageDirectory;

        typedef std::map<pfObject*,osg::Object*> PfObjectToOsgObjectMap;
        PfObjectToOsgObjectMap _pfToOsgMap;

        osg::Node* _osgRoot;

};

#endif
