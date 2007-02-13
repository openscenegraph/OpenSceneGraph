#include <osgIntrospection/Reflector>

namespace osgIntrospection
{
    template<>
    void Reflector<void>::init_reference_types()
    {
        // Avoid trying to register void & / const void &, which are
        // illegal types.
    }
}
