#pragma once

#include <osgAnimation/Skeleton>

namespace ive {

    class DataOutputStream;
    class DataInputStream;

    class Skeleton 
        : public osgAnimation::Skeleton
    {
    public:
        void write(DataOutputStream* out);
        void read(DataInputStream* in);
    };
}