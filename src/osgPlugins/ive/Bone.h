#pragma once

#include <osgAnimation/Bone>

namespace ive {

    class DataOutputStream;
    class DataInputStream;

    class Bone
        : public osgAnimation::Bone 
    {
    public:
        void write(DataOutputStream* out);
        void read(DataInputStream* in);
    };
}