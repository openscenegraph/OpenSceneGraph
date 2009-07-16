
#ifndef __VBSP_ENTITY_H_
#define __VBSP_ENTITY_H_


#include <osg/Group>
#include <osg/Vec3f>

#include "VBSPData.h"


namespace bsp
{


enum EntityClass
{
    ENTITY_WORLDSPAWN,
    ENTITY_ENV,
    ENTITY_FUNC_BRUSH,
    ENTITY_PROP,
    ENTITY_INFO_DECAL,
    ENTITY_ITEM,
    ENTITY_OTHER
};


class VBSPEntity
{
protected:

    VBSPData *                  bsp_data;

    EntityClass                 entity_class;
    std::string                 class_name;

    typedef std::pair<std::string, std::string>   EntityParameter;
    typedef std::map<std::string, std::string>    EntityParameters;
    EntityParameters            entity_parameters;

    bool                        entity_visible;
    bool                        entity_transformed;
    int                         entity_model_index;
    std::string                 entity_model;
    osg::Vec3f                  entity_origin;
    osg::Vec3f                  entity_angles;
    osg::ref_ptr<osg::Group>    entity_geometry;

    void    processWorldSpawn();
    void    processEnv();
    void    processFuncBrush();
    void    processProp();
    void    processInfoDecal();
    void    processItem();

    osg::Vec3f     getVector(std::string str);
    std::string    getToken(std::string str, size_t & index);
    void           parseParameters(std::string & entityText);

    osg::ref_ptr<osg::Group>    createBrushGeometry();
    osg::ref_ptr<osg::Group>    createModelGeometry();

public:

    VBSPEntity(std::string & entityText, VBSPData * bspData);
    ~VBSPEntity();

    EntityClass                 getClass();
    bool                        isVisible();

    osg::ref_ptr<osg::Group>    createGeometry();
};

}


#endif

