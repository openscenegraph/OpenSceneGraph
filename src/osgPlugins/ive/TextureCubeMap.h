#ifndef IVE_TEXTURECUBEMAP
#define IVE_TEXTURECUBEMAP 1

#include <osg/TextureCubeMap>
#include "ReadWrite.h"

namespace ive
{

class TextureCubeMap : public osg::TextureCubeMap, public ReadWrite 
{
public:
	void write(DataOutputStream* out);
        
        void writeImage(DataOutputStream* out,bool includeImg,osg::Image* image);

	void read(DataInputStream* in);
        
	osg::Image* readImage(DataInputStream* in, bool includeImg);
        

};
}

#endif
