#include "RigGeometry.h"
#include "Geometry.h"
#include "DataOutputStream.h"
#include "DataInputStream.h"
#include "Object.h"
#include "Exception.h"
#include <osgAnimation/VertexInfluence>

namespace ive
{

    namespace {
        void writeVertexInfluence(DataOutputStream& out, const osgAnimation::VertexInfluence & vertexInfluence)
        {
            out.writeString(vertexInfluence.getName());
            out.writeUInt(vertexInfluence.size());
            std::for_each(vertexInfluence.cbegin(), vertexInfluence.cend(), 
                [&](osgAnimation::VertexInfluence::const_reference item_) {
                out.writeUInt(item_.first);
                out.writeFloat(item_.second);
            });
        }

        void readVertexInfluence(DataInputStream & in, osgAnimation::VertexInfluence & vertexInfluence)
        {
            vertexInfluence.setName(in.readString());
            vertexInfluence.reserve(in.readUInt());
            for (uint32_t i = 0; i < vertexInfluence.capacity(); ++i) {
                osgAnimation::VertexIndexWeight viw;
                viw.first = in.readUInt();
                viw.second = in.readFloat();
                vertexInfluence.push_back(std::move(viw));
            }
        }
    }

    void RigGeometry::write(DataOutputStream* out)
    {
        out->writeInt(IVERIGGEOMETRY);
        static_cast<ive::Geometry*>(static_cast<osg::Geometry*>(this))->write(out);
        auto geometry_ = getSourceGeometry();
        if (nullptr != geometry_) {
            out->writeBool(true);
            static_cast<ive::Geometry*>(geometry_)->write(out);
        }
        else {
            out->writeBool(false);
        }
        auto vertexInfluenceMap_ = getInfluenceMap();
        if (vertexInfluenceMap_) {
            out->writeBool(true);
            out->writeString(vertexInfluenceMap_->getName());
            out->writeUInt(vertexInfluenceMap_->size());
            std::for_each(vertexInfluenceMap_->cbegin(), vertexInfluenceMap_->cend(), 
                [&](osgAnimation::VertexInfluenceMap::const_reference item_) {
                out->writeString(item_.first);
                writeVertexInfluence(*out, item_.second);
            });
        }
        else {
            out->writeBool(false);
        }
    }

    void RigGeometry::read(DataInputStream* in)
    {
        int id = in->peekInt();
        if (id == IVERIGGEOMETRY) {
            id = in->readInt();
            static_cast<ive::Geometry*>(static_cast<osg::Geometry*>(this))->read(in);
            bool hasSourceGeometrySource = in->readBool();
            if (hasSourceGeometrySource) {
                auto sourceGeometry = new osg::Geometry();
                static_cast<ive::Geometry*>(sourceGeometry)->read(in);
                setSourceGeometry(sourceGeometry);
                copyFrom(*sourceGeometry);
            }
            bool hasInfluenceMap = in->readBool();
            if (hasInfluenceMap) {
                osgAnimation::VertexInfluenceMap *vertexInfluenceMap_ = new osgAnimation::VertexInfluenceMap();
                vertexInfluenceMap_->setName(in->readString());
                for (uint32_t i = 0, c = in->readUInt(); i < c; ++i) {
                    readVertexInfluence(*in, (*vertexInfluenceMap_)[in->readString()]);
                }
                setInfluenceMap(vertexInfluenceMap_);   
            }
        }
        else {
            in_THROW_EXCEPTION("RigGeometry::read(): Expected RigGeometry identification");
        }
    }
}