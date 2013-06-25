// -*-c++-*-

#ifndef __CONVERTFROMPERFORMER_H
#define __CONVERTFROMPERFORMER_H

#include <map>
#include <vector>
#include <string>
#include <iostream>

// Open Scene Graph includes.
#include <osg/Node>
#include <osg/Group>
#include <osg/StateSet>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Material>

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
        void registerPfObjectForOsgObject(pfObject* pfObj,osg::Object* osgObj);

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
        osg::Drawable* visitGeoSet(osg::Geode* osgParent,pfGeoSet* geoset);
        osg::StateSet* visitGeoState(osg::Drawable* osgDrawble,pfGeoState* geostate);
        osg::Material* visitMaterial(osg::StateSet* osgStateSet,pfMaterial* front_mat,pfMaterial* back_mat);
        osg::Texture2D* visitTexture(osg::StateSet* osgStateSet,pfTexture* tex);

        typedef std::map<int,deprecated_osg::Geometry::AttributeBinding> GSetBindingMap;

        GSetBindingMap      _gsetBindMap;

        bool _saveImagesAsRGB;
        bool _saveAbsoluteImagePath;
        std::string _saveImageDirectory;

        typedef std::map<pfObject*,osg::Object*> PfObjectToOsgObjectMap;
        PfObjectToOsgObjectMap _pfToOsgMap;

        osg::Node* _osgRoot;

};

#endif
