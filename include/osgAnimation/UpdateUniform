#ifndef OSGANIMATION_UPDATE_UNIFORM
#define OSGANIMATION_UPDATE_UNIFORM 1

#include <osgAnimation/AnimationUpdateCallback>
#include <osgAnimation/Export>
#include <osg/NodeVisitor>
#include <osg/Uniform>

namespace osgAnimation
{
    template <typename T>
    class UpdateUniform : public AnimationUpdateCallback<osg::UniformCallback>
    {
    protected:
        osg::ref_ptr< TemplateTarget<T> > _uniformTarget;

    public:
        UpdateUniform(const std::string& aName = "")
        :    AnimationUpdateCallback<osg::UniformCallback>(aName)
        {
            _uniformTarget = new TemplateTarget<T>();        // NOTE: initial value is undefined
        }

        UpdateUniform(const UpdateUniform& updateuniform,
                      const osg::CopyOp& copyop) :
            AnimationUpdateCallback<osg::UniformCallback>(updateuniform, copyop)
        {
            _uniformTarget = new TemplateTarget<T>(*(updateuniform._uniformTarget));
        }

        META_Object(osgAnimation, UpdateUniform<T>);

        /** Callback method called by the NodeVisitor when visiting a node.*/
        virtual void operator () (osg::Uniform* uniform, osg::NodeVisitor* nv)
        {
            if (nv && nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
                update(*uniform);

            traverse(uniform, nv);
        }

        bool link(Channel* channel)
        {
            if (channel->getName().find("uniform") != std::string::npos)
                return channel->setTarget(_uniformTarget.get());
            else
                OSG_WARN << "Channel " << channel->getName() << " does not contain a valid symbolic name for this class " << className() << std::endl;

            return false;
        }

        void update(osg::Uniform& uniform)
        {
            T value = _uniformTarget->getValue();
            uniform.set(value);
        }

    };

    // float
    struct UpdateFloatUniform : public UpdateUniform<float>
    {
        UpdateFloatUniform(const std::string& aName = "") : UpdateUniform<float>(aName) { }
        UpdateFloatUniform(const UpdateFloatUniform& ufu,
                           const osg::CopyOp& copyop) :
            osg::Object(ufu, copyop),        // copy name
            UpdateUniform<float>(ufu, copyop) { }

        META_Object(osgAnimation, UpdateFloatUniform);
    };

    // Vec2f
    struct UpdateVec2fUniform : public UpdateUniform<osg::Vec2f>
    {
        UpdateVec2fUniform(const std::string& aName = "") : UpdateUniform<osg::Vec2f>(aName) { }
        UpdateVec2fUniform(const UpdateVec2fUniform& uv2fu,
                           const osg::CopyOp& copyop) :
            osg::Object(uv2fu, copyop),        // copy name
            UpdateUniform<osg::Vec2f>(uv2fu, copyop) { }

        META_Object(osgAnimation, UpdateVec2fUniform);
    };

    // Vec3f
    struct UpdateVec3fUniform : public UpdateUniform<osg::Vec3f>
    {
        UpdateVec3fUniform(const std::string& aName = "") : UpdateUniform<osg::Vec3f>(aName) { }
        UpdateVec3fUniform(const UpdateVec3fUniform& uv3fu,
                           const osg::CopyOp& copyop) :
            osg::Object(uv3fu, copyop),        // copy name
            UpdateUniform<osg::Vec3f>(uv3fu, copyop) { }

        META_Object(osgAnimation, UpdateVec3fUniform);
    };

    // Vec4f
    struct UpdateVec4fUniform : public UpdateUniform<osg::Vec4f>
    {
        UpdateVec4fUniform(const std::string& aName = "") : UpdateUniform<osg::Vec4f>(aName) { }
        UpdateVec4fUniform(const UpdateVec4fUniform& uv4fu,
                           const osg::CopyOp& copyop) :
            osg::Object(uv4fu, copyop),        // copy name
            UpdateUniform<osg::Vec4f>(uv4fu, copyop) { }

        META_Object(osgAnimation, UpdateVec4fUniform);
    };

    // Matrixf
    struct UpdateMatrixfUniform : public UpdateUniform<osg::Matrixf>
    {
        UpdateMatrixfUniform(const std::string& aName = "") : UpdateUniform<osg::Matrixf>(aName) { }
        UpdateMatrixfUniform(const UpdateMatrixfUniform& umfu,
                           const osg::CopyOp& copyop) :
            osg::Object(umfu, copyop),        // copy name
            UpdateUniform<osg::Matrixf>(umfu, copyop) { }

        META_Object(osgAnimation, UpdateMatrixfUniform);
    };
}

#endif
