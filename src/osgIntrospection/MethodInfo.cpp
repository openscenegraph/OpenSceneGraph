#include <osgIntrospection/MethodInfo>

using namespace osgIntrospection;

void MethodInfo::getInheritedProviders(CustomAttributeProviderList &providers) const
{
	for (int i=0; i<decltype_.getNumBaseTypes(); ++i)
	{
		const MethodInfo *mi = decltype_.getBaseType(i).getMethod(name_, params_, false);
		if (mi)
		{
			providers.push_back(mi);
		}
	}
}
