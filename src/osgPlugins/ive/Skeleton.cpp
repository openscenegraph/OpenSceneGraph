#include "Skeleton.h"
#include "DataOutputStream.h"
#include "DataInputStream.h"
#include "MatrixTransform.h"
#include "Exception.h"

namespace ive
{
    void Skeleton::write(DataOutputStream* out)
    {
        out->writeInt(IVESKELETON);
        static_cast<ive::MatrixTransform*>(static_cast<osg::MatrixTransform*>(this))->write(out);
    }

    void Skeleton::read(DataInputStream* in)
    {
        int id = in->peekInt();
        if (IVESKELETON == id) {
            id = in->readInt();
            static_cast<ive::MatrixTransform*>(static_cast<osg::MatrixTransform*>(this))->read(in);
        }
        else {
            in_THROW_EXCEPTION("Skeleton::read(): Expected Skeleton identification");
        }
    }
}