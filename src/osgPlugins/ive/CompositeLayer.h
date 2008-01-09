#ifndef IVE_COMPOSITELAYER
#define IVE_COMPOSITELAYER 1

#include <osgTerrain/Terrain>
#include "ReadWrite.h"

namespace ive
{

class CompositeLayer : public osgTerrain::CompositeLayer, public ReadWrite
{
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};

}

#endif
