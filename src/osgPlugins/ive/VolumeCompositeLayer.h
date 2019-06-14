#ifndef IVE_VOLUMECOMPOSITELAYER
#define IVE_VOLUMECOMPOSITELAYER 1

#include <osgVolume/Layer>
#include "ReadWrite.h"

namespace ive
{

class VolumeCompositeLayer : public osgVolume::CompositeLayer
{
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};

}

#endif
