#include <osgIntrospection/ConstructorInfo>

using namespace osgIntrospection;

void ConstructorInfo::getInheritedProviders(CustomAttributeProviderList &providers) const
{
    for (int i=0; i<decltype_.getNumBaseTypes(); ++i)
    {
        const ConstructorInfo *ci = decltype_.getBaseType(i).getConstructor(params_);
        if (ci)
        {
            providers.push_back(ci);
        }
    }
}
