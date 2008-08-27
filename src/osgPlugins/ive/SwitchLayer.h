#ifndef IVE_SWITCHLAYER
#define IVE_SWITCHLAYER 1

#include <osgTerrain/Layer>
#include "ReadWrite.h"

namespace ive
{

class SwitchLayer : public osgTerrain::SwitchLayer, public ReadWrite
{
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};

}

#endif
