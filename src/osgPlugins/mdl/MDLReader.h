#ifndef __MDL_READER_H_
#define __MDL_READER_H_


#include <osg/Geometry>
#include <osg/Matrixd>
#include <osg/Node>
#include <osg/Object>
#include <osg/StateSet>
#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osg/Referenced>

#include "MDLLimits.h"
#include "MDLRoot.h"


namespace mdl
{


// The magic number for a Valve MDL file is 'IDST' in little-endian
// order
const int MDL_MAGIC_NUMBER = (('T'<<24)+('S'<<16)+('D'<<8)+'I');


struct MDLHeader
{
    int           magic_number;
    int           mdl_version;
    int           check_sum;
    char          mdl_name[64];
    int           mdl_length;

    osg::Vec3     eye_position;
    osg::Vec3     illum_position;
    osg::Vec3     hull_min;
    osg::Vec3     hull_max;
    osg::Vec3     view_bbox_min;
    osg::Vec3     view_bbox_max;

    int           mdl_flags;

    int           num_bones;
    int           bone_offset;

    int           num_bone_controllers;
    int           bone_controller_offset;

    int           num_hitbox_sets;
    int           hitbox_set_offset;

    int           num_local_animations;
    int           local_animation_offset;

    int           num_local_sequences;
    int           local_sequence_offset;

    mutable int   activity_list_version;
    mutable int   events_offseted;

    int           num_textures;
    int           texture_offset;

    int           num_texture_paths;
    int           texture_path_offset;

    int           num_skin_refs;
    int           num_skin_families;
    int           skin_offset;

    int           num_body_parts;
    int           body_part_offset;

    int           num_local_attachments;
    int           local_attachment_offset;

    int           num_local_nodes;
    int           local_node_offset;
    int           local_node_name_offset;

    int           num_flex_desc;
    int           flex_desc_offset;

    int           num_flex_controllers;
    int           flex_controller_offset;

    int           num_flex_rules;
    int           flex_rule_offset;

    int           num_ik_chains;
    int           ik_chain_offset;

    int           num_mouths;
    int           mouth_offset;

    int           num_local_pose_params;
    int           local_pose_param_offset;

    int           surface_prop_offset;

    int           key_value_offset;
    int           key_value_size;

    int           num_local_ik_autoplay_locks;
    int           local_ik_autoplay_lock_offset;

    float         mdl_mass;
    int           mdl_contents;

    int           num_include_models;
    int           include_model_offset;

    // Originally a mutable void * (changed for portability)
    mutable int   virtual_model;

    int           anim_block_name_offset;
    int           num_anim_blocks;
    int           anim_block_offset;

    // Originally a mutable void * (changed for portability)
    mutable int   anim_block_model;

    int           bone_table_by_name_offset;

    // Originally both void * (changed for portability)
    int           vertex_base;
    int           offset_base;

    unsigned char const_direction_light_dot;
    unsigned char root_lod;
    unsigned char unused_byte[2];

    int           zero_frame_cache_offset;

    int           unused_fields[2];
};


struct MDLTexture
{
    int              tex_name_offset;
    int              tex_flags;
    int              tex_used;

    int              unused_1;

    // Originally both mutable void * (changed for portability)
    mutable int      tex_material;
    mutable int      client_material;

    int              unused_array[10];
};


class MDLReader
{
protected:

   std::string                mdl_name;

   osg::ref_ptr<osg::Node>    root_node;

   typedef std::vector<std::string>    StringList;
   StringList                 texture_paths;

   typedef std::vector< osg::ref_ptr<osg::StateSet> >    StateSetList;
   StateSetList               state_sets;

   std::string    getToken(std::string str, const char * delim, size_t & index);
   std::string    findFileIgnoreCase(std::string filePath);

   osg::ref_ptr<osg::Texture>     readTextureFile(std::string textureName);
   osg::ref_ptr<osg::StateSet>    readMaterialFile(std::string mtlName);

   BodyPart *    processBodyPart(std::istream * str, int offset);
   Model *       processModel(std::istream * str, int offset);
   Mesh *        processMesh(std::istream * str, int offset);

public:

   MDLReader();
   virtual ~MDLReader();

   bool   readFile(const std::string & file);

   osg::ref_ptr<osg::Node>   getRootNode();
};


}

#endif
