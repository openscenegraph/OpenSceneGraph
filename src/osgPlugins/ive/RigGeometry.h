#pragma once

#include <osgAnimation/RigGeometry>

namespace ive {

    class DataOutputStream;
    class DataInputStream;

    class RigGeometry
        : public osgAnimation::RigGeometry 
    {
    public:
        void write(DataOutputStream* out);
        void read(DataInputStream* in);
    };
}