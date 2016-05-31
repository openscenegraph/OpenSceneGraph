#ifndef IVE_HIEGHTFIELD
#define IVE_HIEGHTFIELD 1

#include <osg/Shape>
#include "ReadWrite.h"

namespace ive{

class Sphere : public osg::Sphere {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};

class Box : public osg::Box {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};

class Cone : public osg::Cone {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};

class Cylinder : public osg::Cylinder {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};

class Capsule : public osg::Capsule {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};

class HeightField : public osg::HeightField {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};

}

#endif
