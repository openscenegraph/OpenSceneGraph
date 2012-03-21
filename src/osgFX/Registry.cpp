#include <osgFX/Registry>

using namespace osgFX;


Registry* Registry::instance()
{
    static osg::ref_ptr<Registry> s_instance = new Registry;
    return s_instance.get();
}

Registry::Registry()
{
}
