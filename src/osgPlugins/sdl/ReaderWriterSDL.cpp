// Released under the OSGPL license, as part of the OpenSceneGraph distribution.
//
// ReaderWriter for sgi's .rgb format.
// specification can be found at http://local.wasp.uwa.edu.au/~pbourke/dataformats/sgirgb/sgiversion.html

#include <osg/Notify>

#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

#include "JoystickDevice.h"


class ReaderWriterSDL : public osgDB::ReaderWriter
{
    public:

        ReaderWriterSDL()
        {
            supportsExtension("sdl","SDL Device Integration");
        }

        virtual const char* className() const { return "SDL Device Integration plugin"; }

        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options =NULL) const
        {
            if (file=="joystick.sdl")
            {
                return new JoystickDevice;
            }
            return ReadResult::FILE_NOT_HANDLED;
        }

};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(sdl, ReaderWriterSDL)
