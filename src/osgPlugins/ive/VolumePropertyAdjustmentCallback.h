#ifndef IVE_PROPERTYADJUSTMENTCALLBACK_H
#define IVE_PROPERTYADJUSTMENTCALLBACK_H 1

#include <osgVolume/VolumeTile>
#include "ReadWrite.h"

namespace ive{
class VolumePropertyAdjustmentCallback : public osgVolume::PropertyAdjustmentCallback {
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);
};
}

#endif
