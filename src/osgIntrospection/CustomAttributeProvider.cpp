#include <osgIntrospection/CustomAttributeProvider>
#include <osgIntrospection/Type>

using namespace osgIntrospection;

bool CustomAttributeProvider::isDefined(const Type &type, bool inherit) const
{
    for (CustomAttributeList::const_iterator i=attribs_.begin(); i!=attribs_.end(); ++i)
        if (typeid(**i) == type.getStdTypeInfo()) return true;

    if (inherit)
    {
        CustomAttributeProviderList providers;
        getInheritedProviders(providers);

        for (CustomAttributeProviderList::const_iterator i=providers.begin(); i!=providers.end(); ++i)
        {
            if ((*i)->isDefined(type, true)) return true;
        }
    }

    return false;
}

const CustomAttribute *CustomAttributeProvider::getAttribute(const Type &type, bool inherit) const
{
    for (CustomAttributeList::const_iterator i=attribs_.begin(); i!=attribs_.end(); ++i)
        if (typeid(**i) == type.getStdTypeInfo()) return *i;

    if (inherit)
    {
        CustomAttributeProviderList providers;
        getInheritedProviders(providers);

        for (CustomAttributeProviderList::const_iterator i=providers.begin(); i!=providers.end(); ++i)
        {
            const CustomAttribute *ca = (*i)->getAttribute(type, true);
            if (ca) return ca;
        }
    }

    return 0;
}
