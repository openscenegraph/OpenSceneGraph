#include "Bone.h"
#include "DataOutputStream.h"
#include "DataInputStream.h"
#include "MatrixTransform.h"
#include "Exception.h"
#include <osgAnimation/UpdateBone>
#include <osgAnimation/StackedTransform>
#include <osgAnimation/StackedMatrixElement>
#include <osgAnimation/StackedQuaternionElement>
#include <osgAnimation/StackedRotateAxisElement>
#include <osgAnimation/StackedScaleElement>
#include <osgAnimation/StackedTranslateElement>

namespace ive{

    namespace {
        void writeStackedTransformElement(DataOutputStream& out,
            const osgAnimation::StackedMatrixElement & stackedMatrixElement)
        {
            out.writeUInt(IVESTACKEDMATRIXELEMENT);
            out.writeString(stackedMatrixElement.getName());
            auto target_ = dynamic_cast<const osgAnimation::MatrixTarget*>(stackedMatrixElement.getTarget());
            if (nullptr != target_) {
                out.writeMatrixf(target_->getValue());
            }
            else {
                out.writeMatrixf(stackedMatrixElement.getMatrix());
            }
        }

        void writeStackedTransformElement(DataOutputStream& out,
            const osgAnimation::StackedQuaternionElement & stackedQuaternionElement)
        {
            out.writeUInt(IVESTACKEDQUATERNIONELEMENT);
            out.writeString(stackedQuaternionElement.getName());
            auto target_ = dynamic_cast<const osgAnimation::QuatTarget*>(stackedQuaternionElement.getTarget());
            if (nullptr != target_) {
                out.writeQuat(target_->getValue());
            }
            else {
                out.writeQuat(stackedQuaternionElement.getQuaternion());
            }
        }

        void writeStackedTransformElement(DataOutputStream& out,
            const osgAnimation::StackedRotateAxisElement & stackedRotateAxisElement)
        {
            out.writeUInt(IVESTACKEDROTATEAXISELEMENT);
            out.writeString(stackedRotateAxisElement.getName());
            out.writeVec3(stackedRotateAxisElement.getAxis());
            auto target_ = dynamic_cast<const osgAnimation::FloatTarget*>(stackedRotateAxisElement.getTarget());
            if (nullptr != target_) {
                out.writeDouble(target_->getValue());
            }
            else {
                out.writeDouble(stackedRotateAxisElement.getAngle());
            }
        }

        void writeStackedTransformElement(DataOutputStream& out,
            const osgAnimation::StackedTranslateElement & stackedTranslateElement)
        {
            out.writeUInt(IVESTACKEDTRANSLATEELEMENT);
            out.writeString(stackedTranslateElement.getName());
            auto target_ = dynamic_cast<const osgAnimation::Vec3Target*>(stackedTranslateElement.getTarget());
            if (nullptr != target_) {
                out.writeVec3(target_->getValue());
            }
            else {
                out.writeVec3(stackedTranslateElement.getTranslate());
            }
        }

        void writeStackedTransformElement(DataOutputStream& out,
            const osgAnimation::StackedScaleElement & stackedScaleElement)
        {
            out.writeUInt(IVESTACKEDSCALEELEMENT);
            out.writeString(stackedScaleElement.getName());
            auto target_ = dynamic_cast<const osgAnimation::Vec3Target*>(stackedScaleElement.getTarget());
            if (nullptr != target_) {
                out.writeVec3(target_->getValue());
            }
            else {
                out.writeVec3(stackedScaleElement.getScale());
            }
        }

        void writeStackedTransformElement(DataOutputStream& out,
            osgAnimation::StackedTransformElement & stackedTransformElement)
        {
            if (nullptr != dynamic_cast<osgAnimation::StackedMatrixElement*>(&stackedTransformElement)) {
                writeStackedTransformElement(out,
                    static_cast<osgAnimation::StackedMatrixElement&>(stackedTransformElement));
            }
            else if (nullptr != dynamic_cast<osgAnimation::StackedQuaternionElement*>(&stackedTransformElement)) {
                writeStackedTransformElement(out,
                    static_cast<osgAnimation::StackedQuaternionElement&>(stackedTransformElement));
            }
            else if (nullptr != dynamic_cast<osgAnimation::StackedRotateAxisElement*>(&stackedTransformElement)) {
                writeStackedTransformElement(out,
                    static_cast<osgAnimation::StackedRotateAxisElement&>(stackedTransformElement));
            }
            else if (nullptr != dynamic_cast<osgAnimation::StackedTranslateElement*>(&stackedTransformElement)) {
                writeStackedTransformElement(out,
                    static_cast<osgAnimation::StackedTranslateElement&>(stackedTransformElement));
            }
            else if (nullptr != dynamic_cast<osgAnimation::StackedScaleElement*>(&stackedTransformElement)) {
                writeStackedTransformElement(out,
                    static_cast<osgAnimation::StackedScaleElement&>(stackedTransformElement));
            }
            else {
                out.throwException("Unknown osgAnimation::StackedTransformElement in Bone::write()");
            }
        }

        osg::ref_ptr<osgAnimation::StackedMatrixElement> readStackedMatrixElement(DataInputStream& in)
        {
            auto n_ = in.readString();
            auto m_ = in.readMatrixf();
            return new osgAnimation::StackedMatrixElement(n_, m_);
        }

        osg::ref_ptr<osgAnimation::StackedQuaternionElement> readStackedQuaternionElement(DataInputStream& in)
        {
            auto n_ = in.readString();
            auto q_ = in.readQuat();
            return new osgAnimation::StackedQuaternionElement(n_, q_);
        }

        osg::ref_ptr<osgAnimation::StackedRotateAxisElement> readStackedRotateAxisElement(DataInputStream& in)
        {
            auto n_ = in.readString();
            auto axis_ = in.readVec3();
            auto f_ = in.readFloat();
            return new osgAnimation::StackedRotateAxisElement(n_, axis_, f_);
        }

        osg::ref_ptr<osgAnimation::StackedTranslateElement> readStackedTranslateElement(DataInputStream& in)
        {
            auto n_ = in.readString();
            auto v_ = in.readVec3();
            return new osgAnimation::StackedTranslateElement(n_, v_);
        }

        osg::ref_ptr<osgAnimation::StackedScaleElement> readStackedScaleElement(DataInputStream& in)
        {
            auto n_ = in.readString();
            auto v_ = in.readVec3();
            return new osgAnimation::StackedScaleElement(n_, v_);
        }

        osg::ref_ptr<osgAnimation::StackedTransformElement> readStackedTransformElement(DataInputStream & in)
        {
            osg::ref_ptr<osgAnimation::StackedTransformElement> r = nullptr;
            switch (in.readUInt()) {
            case IVESTACKEDMATRIXELEMENT:
                r = readStackedMatrixElement(in);
                break;
            case IVESTACKEDQUATERNIONELEMENT:
                r = readStackedQuaternionElement(in);
                break;
            case IVESTACKEDROTATEAXISELEMENT:
                r = readStackedRotateAxisElement(in);
                break;
            case IVESTACKEDTRANSLATEELEMENT:
                r = readStackedTranslateElement(in);
                break;
            case IVESTACKEDSCALEELEMENT:
                r = readStackedScaleElement(in);
                break;
            default:
                in.throwException("Bone::read(): Expected Bone identification");
                break;
            }
            return r;
        }
    }

    void Bone::write(DataOutputStream* out) 
    {
        out->writeInt(IVEBONE);
        static_cast<ive::MatrixTransform*>(static_cast<osg::MatrixTransform*>(this))->write(out);
        out->writeMatrixf(getInvBindMatrixInSkeletonSpace());
        out->writeMatrixf(getMatrixInSkeletonSpace());
        auto updateBone = dynamic_cast<osgAnimation::UpdateBone*>(getUpdateCallback());
        if (nullptr != updateBone) {
            out->writeString(updateBone->getName());
            const auto & stackedTransforms_ = updateBone->getStackedTransforms();
            out->writeUInt(stackedTransforms_.size());
            std::for_each(stackedTransforms_.begin(), stackedTransforms_.end(), 
                [&](const osg::ref_ptr < osgAnimation::StackedTransformElement> ste_) {
                writeStackedTransformElement(*out, *ste_);
            });
        }
        else {
            out->writeString(getName());
            out->writeUInt(0);
        }
    }

    void Bone::read(DataInputStream* in) 
    {
        int id = in->peekInt();
        if (id == IVEBONE) {
            id = in->readInt();
            static_cast<ive::MatrixTransform*>(static_cast<osg::MatrixTransform*>(this))->read(in);
            setInvBindMatrixInSkeletonSpace(in->readMatrixf());
            setMatrixInSkeletonSpace(in->readMatrixf());
            setDefaultUpdateCallback(in->readString());
            auto updateBone = dynamic_cast<osgAnimation::UpdateBone*>(getUpdateCallback());
            if (nullptr != updateBone) {
                auto & stackedTransforms_ = updateBone->getStackedTransforms();
                stackedTransforms_.reserve(in->readUInt());
                for (uint32_t i =0; i < stackedTransforms_.capacity(); ++i){
                    stackedTransforms_.push_back(readStackedTransformElement(*in));
                }
            }
        }
        else {
            in_THROW_EXCEPTION("Bone::read(): Expected Bone identification");
        }
    }
}