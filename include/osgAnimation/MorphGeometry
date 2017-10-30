/*  -*-c++-*-
 *  Copyright (C) 2008 Cedric Pinson <mornifle@plopbyte.net>
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#ifndef OSGANIMATION_MORPHGEOMETRY_H
#define OSGANIMATION_MORPHGEOMETRY_H

#include <osgAnimation/Export>
#include <osgAnimation/AnimationUpdateCallback>
#include <osgAnimation/MorphTransformSoftware>
#include <osg/Geometry>
#include <algorithm>

namespace osgAnimation
{

    class OSGANIMATION_EXPORT MorphGeometry : public osg::Geometry
    {

    public:

        enum Method
        {
            NORMALIZED,
            RELATIVE
        };

        class MorphTarget
        {
        protected:
            osg::ref_ptr<osg::Geometry> _geom;
            float _weight;
        public:
            MorphTarget(osg::Geometry* geom, float w = 1.0) : _geom(geom), _weight(w) {}
            void setWeight(float weight) { _weight = weight; }
            float getWeight() const { return _weight; }
            osg::Geometry* getGeometry() { return _geom.get(); }
            const osg::Geometry* getGeometry() const { return _geom.get(); }
            void setGeometry(osg::Geometry* geom) { _geom = geom; }
        };

        typedef std::vector<MorphTarget> MorphTargetList;

        MorphGeometry();
        MorphGeometry(const osg::Geometry& b);
        MorphGeometry(const MorphGeometry& b, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

        virtual osg::Object* cloneType() const { return new MorphGeometry(); }
        virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new MorphGeometry(*this,copyop); }
        virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const MorphGeometry*>(obj)!=NULL; }
        virtual const char* libraryName() const { return "osgAnimation"; }
        virtual const char* className() const { return "MorphGeometry"; }

        // set implementation of rig method
        inline void setMorphTransformImplementation(MorphTransform*mt) { _morphTransformImplementation=mt; }
        inline MorphTransform* getMorphTransformImplementation() { return _morphTransformImplementation.get(); }
        inline const MorphTransform* getMorphTransformImplementation() const { return _morphTransformImplementation.get(); }

        /** Set the morphing method. */
        inline void setMethod(Method method) { _method = method; }
        /** Get the morphing method. */
        inline Method getMethod() const { return _method; }

        /** Set flag for morphing normals. */
        inline void setMorphNormals(bool morphNormals) { _morphNormals = morphNormals; }
        /** Get the flag for morphing normals. */
        inline bool getMorphNormals() const { return _morphNormals; }

        /** Get the list of MorphTargets.*/
        inline const MorphTargetList& getMorphTargetList() const { return _morphTargets; }

        /** Get the list of MorphTargets. Warning if you modify this array you will have to call dirty() */
        inline MorphTargetList& getMorphTargetList() { return _morphTargets; }

        /** Return the \c MorphTarget at position \c i.*/
        inline const MorphTarget& getMorphTarget( unsigned int i ) const { return _morphTargets[i]; }

        /** Return the \c MorphTarget at position \c i.*/
        inline MorphTarget& getMorphTarget( unsigned int i ) { return _morphTargets[i]; }

        /** Set source of vertices for this morph geometry */
        inline void setVertexSource(osg::Vec3Array *v) { _positionSource=v; }

        /** Get source of vertices for this morph geometry */
        inline osg::Vec3Array * getVertexSource() const { return _positionSource.get(); }

        /** Set source of normals for this morph geometry */
        inline void setNormalSource(osg::Vec3Array *n) { _normalSource=n; }

        /** Get source of normals for this morph geometry */
        inline osg::Vec3Array * getNormalSource() const { return _normalSource.get(); }

        /** Add a \c MorphTarget to the \c MorphGeometry.
          * If \c MorphTarget is not \c NULL and is not contained in the \c MorphGeometry
          * then increment its reference count, add it to the MorphTargets list and
          * dirty the bounding sphere to force it to be recomputed on the next
          * call to \c getBound().
          * @param morphTarget The \c MorphTarget to be added to the \c MorphGeometry.
          * @param weight The weight to be added to the \c MorphGeometry.
          * @return  \c true for success; \c false otherwise.
        */
        virtual void addMorphTarget( osg::Geometry *morphTarget, float weight = 1.0 )
        {
            _morphTargets.push_back(MorphTarget(morphTarget, weight));
            _dirty = true;
        }

        virtual void removeMorphTarget( osg::Geometry *morphTarget )
        {
            for(MorphTargetList::iterator iterator = _morphTargets.begin() ; iterator != _morphTargets.end() ; ++ iterator)
            {
                if(iterator->getGeometry()  == morphTarget)
                {
                    _morphTargets.erase(iterator);
                    break;
                }
            }
        }

        virtual void removeMorphTarget( const std::string& name )
        {
            for(MorphTargetList::iterator iterator = _morphTargets.begin() ; iterator != _morphTargets.end() ; ++ iterator)
            {
                if(iterator->getGeometry() && iterator->getGeometry()->getName() == name)
                {
                    _morphTargets.erase(iterator);
                    break;
                }
            }

        }

        /** update a morph target at index setting its current weight to morphWeight */
        inline void setWeight(unsigned int index, float morphWeight)
        {
            if (index < _morphTargets.size())
            {
                _morphTargets[index].setWeight(morphWeight);
                dirty();
            }
        }

        /** Set the MorphGeometry dirty.*/
        inline void dirty(bool b=true) { _dirty = b; }
        inline bool isDirty() const { return _dirty; }

        /** for retrocompatibility */
        void transformSoftwareMethod() { (*_morphTransformImplementation.get())(*this); }

    protected:
        osg::ref_ptr<MorphTransform> _morphTransformImplementation;
        /// Do we need to recalculate the morphed geometry?
        bool _dirty;

        Method          _method;
        MorphTargetList _morphTargets;

        osg::ref_ptr<osg::Vec3Array> _positionSource;
        osg::ref_ptr<osg::Vec3Array> _normalSource;

        /// Do we also morph between normals?
        bool _morphNormals;
    };

    class OSGANIMATION_EXPORT UpdateMorph : public AnimationUpdateCallback<osg::NodeCallback>
    {
    public:
        typedef std::vector<std::string> TargetNames;
        typedef std::map< int, osg::ref_ptr<osgAnimation::FloatTarget> > WeightTargets;

        META_Object(osgAnimation, UpdateMorph);

        UpdateMorph(const std::string& name = "");
        UpdateMorph(const UpdateMorph& apc,const osg::CopyOp& copyop);

        void addTarget(const std::string& name) { _targetNames.push_back(name); }
        unsigned int getNumTarget() const { return _targetNames.size(); }
        const std::string& getTargetName(unsigned int index) { return _targetNames[index]; }
        void removeTarget(const std::string& name)
        {
            TargetNames::iterator found = std::find(_targetNames.begin(), _targetNames.end(), name);
            if(found != _targetNames.end())
                _targetNames.erase(found);
        }

        // for serialization
        const std::vector<std::string>& getTargetNames() const { return _targetNames; }
        std::vector<std::string>& getTargetNames() { return _targetNames; }

        void setTargetNames(const TargetNames& targetNames) { _targetNames.assign(targetNames.begin(), targetNames.end()); }

        /** Callback method called by the NodeVisitor when visiting a node.*/
        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
        bool needLink() const;
        bool link(osgAnimation::Channel* channel);
        int link(Animation* animation);

    protected:
        WeightTargets _weightTargets;
        TargetNames _targetNames;
    };

    struct UpdateMorphGeometry : public osg::DrawableUpdateCallback
    {
        UpdateMorphGeometry() {}

        UpdateMorphGeometry(const UpdateMorphGeometry& org, const osg::CopyOp& copyop):
            osg::Object(org, copyop),
            osg::Callback(org, copyop),
            osg::DrawableUpdateCallback(org, copyop) {}

        META_Object(osgAnimation, UpdateMorphGeometry);

        virtual void update(osg::NodeVisitor*, osg::Drawable* drw)
        {
            MorphGeometry* geom = dynamic_cast<MorphGeometry*>(drw);
            if (!geom)
                return;

            if (!geom->getMorphTransformImplementation())
            {
                geom->setMorphTransformImplementation( new MorphTransformSoftware);
            }

            MorphTransform& implementation = *geom->getMorphTransformImplementation();
            (implementation)(*geom);
        }
    };
}

#endif
