/*******************************************************
      Lightwave Scene Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#ifndef LWOSG_SCENELOADER_
#define LWOSG_SCENELOADER_

#include <osg/Group>
#include <osg/Vec3>
#include <osg/AnimationPath>
#include <osg/Vec3>
#include <osg/Vec4>

#include <osgDB/ReaderWriter>

#include <string>
#include <vector>
#include <map>

namespace lwosg
{

    class CoordinateSystemFixer: public osg::Referenced {
    public:
        virtual osg::Vec3 fix_point(const osg::Vec3 &P) const = 0;
        virtual osg::Vec4 fix_point(const osg::Vec4 &P) const = 0;
        virtual osg::Vec3 fix_vector(const osg::Vec3 &V) const = 0;
        virtual osg::Vec4 fix_vector(const osg::Vec4 &V) const = 0;
        virtual inline bool invert_winding() const { return false; }

    protected:
        virtual ~CoordinateSystemFixer() {}
        CoordinateSystemFixer &operator=(const CoordinateSystemFixer &) { return *this; }
    };

    class LwoCoordFixer: public CoordinateSystemFixer {
    public:
        inline osg::Vec3 fix_point(const osg::Vec3 &P) const;
        inline osg::Vec4 fix_point(const osg::Vec4 &P) const;
        inline osg::Vec3 fix_vector(const osg::Vec3 &V) const;
        inline osg::Vec4 fix_vector(const osg::Vec4 &V) const;
        inline bool invert_winding() const { return true; }

    protected:
        virtual ~LwoCoordFixer() {}
        LwoCoordFixer &operator=(const LwoCoordFixer &) { return *this; }
    };


    class SceneLoader {
    public:
        struct Options {
            osg::ref_ptr<CoordinateSystemFixer> csf;
            Options(): csf(new LwoCoordFixer) {}
        };

        typedef std::vector<osg::ref_ptr<osg::AnimationPath> > Animation_list;

        SceneLoader();
        SceneLoader(const Options &options);

        osg::Group *load(const std::string &filename, const osgDB::ReaderWriter::Options *options, bool search = false);

        inline osg::Group *get_root() { return root_.get(); }
        inline const osg::Group *get_root() const { return root_.get(); }

        inline const Options &get_options() const { return options_; }
        inline Options &get_options() { return options_; }
        inline void set_options(const Options &options) { options_ = options; }

        inline const Animation_list &get_camera_animations() const { return camera_animations_; }
        inline Animation_list &get_camera_animations() { return camera_animations_; }

        struct Motion_envelope {
            struct Key {
                osg::Vec3 position;
                osg::Vec3 ypr;
                osg::Vec3 scale;
                Key(): scale(1, 1, 1) {}
            };
            typedef std::map<double, Key> Key_map;
            Key_map keys;
        };

    protected:
        bool parse_block(const std::string &name, const std::string &data);
        bool parse_block(const std::string &name, const std::vector<std::string> &data);
        void clear();

    private:
        typedef std::map<std::string, osg::ref_ptr<osg::Group> > Object_map;
        Object_map objects_;

        Animation_list camera_animations_;

        struct Scene_object {
            osg::ref_ptr<osg::Node> layer_node;
            int parent;
            osg::Vec3 pivot;
            osg::Vec3 pivot_rot;
            Motion_envelope motion;
            std::string name;

            Scene_object(): parent(-1) {}
        };

        struct Scene_camera {
            Motion_envelope motion;
        };

        typedef std::vector<Scene_object> Scene_object_list;
        Scene_object_list scene_objects_;

        typedef std::vector<Scene_camera> Scene_camera_list;
        Scene_camera_list scene_cameras_;

        osg::ref_ptr<osg::Group> root_;

        int current_channel_;
        int channel_count_;

        bool capture_obj_motion_;
        bool capture_cam_motion_;

        Options options_;
        unsigned int version_;
    };

    // INLINE METHODS

    inline osg::Vec3 LwoCoordFixer::fix_point(const osg::Vec3 &P) const
    {
        return osg::Vec3(P.x(), P.z(), P.y());
    }

    inline osg::Vec4 LwoCoordFixer::fix_point(const osg::Vec4 &P) const
    {
        return osg::Vec4(fix_point(osg::Vec3(P.x(), P.y(), P.z())), P.w());
    }

    inline osg::Vec3 LwoCoordFixer::fix_vector(const osg::Vec3 &V) const
    {
        return fix_point(V);
    }

    inline osg::Vec4 LwoCoordFixer::fix_vector(const osg::Vec4 &V) const
    {
        return fix_point(V);
    }

}

#endif
