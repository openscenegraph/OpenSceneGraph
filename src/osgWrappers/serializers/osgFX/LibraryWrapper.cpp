#include <osgDB/Registry>

USE_SERIALIZER_WRAPPER(osgFX_AnisotropicLighting)
USE_SERIALIZER_WRAPPER(osgFX_BumpMapping)
USE_SERIALIZER_WRAPPER(osgFX_Cartoon)
USE_SERIALIZER_WRAPPER(osgFX_Effect)
USE_SERIALIZER_WRAPPER(osgFX_MultiTextureControl)
USE_SERIALIZER_WRAPPER(osgFX_Outline)
USE_SERIALIZER_WRAPPER(osgFX_Scribe)
USE_SERIALIZER_WRAPPER(osgFX_SpecularHighlights)

extern "C" void wrapper_serializer_library_osgFX(void) {}

