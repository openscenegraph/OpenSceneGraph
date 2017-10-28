#include <osgVolume/MultipassTechnique>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER(osgVolume_MultipassTechnique,
	new osgVolume::MultipassTechnique,
	osgVolume::MultipassTechnique,
	"osg::Object osgVolume::VolumeTechnique osgVolume::MultipassTechnique")
{
}
