#include <osgIntrospection/Type>
#include <osgIntrospection/Value>
#include <osgIntrospection/Reflection>
#include <osgIntrospection/PropertyInfo>
#include <osgIntrospection/MethodInfo>
#include <osgIntrospection/ReaderWriter>

#include <iterator>
#include <algorithm>

using namespace osgIntrospection;

namespace
{

	struct MethodMatch
	{
		int list_pos;
		int exact_args;
		const MethodInfo *method;

		bool operator < (const MethodMatch &m) const
		{
			if (exact_args > m.exact_args) return true;
			if (exact_args < m.exact_args) return false;
			if (list_pos < m.list_pos) return true;
			return false;
		}
	};

}

Type::~Type() 
{ 
	for (PropertyInfoList::const_iterator i=props_.begin(); i!=props_.end(); ++i)
		delete *i;
	for (MethodInfoList::const_iterator i=methods_.begin(); i!=methods_.end(); ++i)
		delete *i;
	delete icb_;
	delete rw_;
}

bool Type::isSubclassOf(const Type &type) const
{
	check_defined();
	for (TypeList::const_iterator i=base_.begin(); i!=base_.end(); ++i)
	{
		if (**i == type.getStdTypeInfo())
			return true;
		if ((*i)->isSubclassOf(type))
			return true;
	}
	return false;
}

const MethodInfo *Type::getCompatibleMethod(const std::string &name, const ValueList &values, bool inherit) const
{
	check_defined();

	MethodInfoList allmethods;
	const MethodInfoList *methods;
	if (inherit)
	{		
		getAllMethods(allmethods);
		methods = &allmethods;
	}
	else
		methods = &methods_;

	typedef std::vector<MethodMatch> MatchList;
	MatchList matches;

	int pos = 0;
	for (MethodInfoList::const_iterator j=methods->begin(); j!=methods->end(); ++j, ++pos)
	{
		const MethodInfo *mi = *j;
		if (mi->getName().compare(name) == 0)
		{
			const ParameterInfoList &other_params = mi->getParameters();
			if (values.size() == other_params.size())
			{
				if (values.empty()) 
					return mi;
				ParameterInfoList::const_iterator i1 = other_params.begin();
				ValueList::const_iterator i2 = values.begin();
				bool candidate = true;
				int exact_args = 0;
				for (; i1<other_params.end(); ++i1, ++i2)
				{
					if ((*i1)->getParameterType() != i2->getType())
					{
						if (i2->tryConvertTo((*i1)->getParameterType()).isEmpty())
						{
							candidate = false;
							break;
						}						
					}
					else
						++exact_args;
				}
				if (candidate) 
				{
					MethodMatch mm;
					mm.list_pos = pos;
					mm.exact_args = exact_args;
					mm.method = mi;
					matches.push_back(mm);
				}
			}
		}
	}

	if (!matches.empty())
	{
		std::sort(matches.begin(), matches.end());
		return matches.front().method;
	}

	return 0;
}

const MethodInfo *Type::getMethod(const std::string &name, const ParameterInfoList &params, bool inherit) const
{
	check_defined();
	for (MethodInfoList::const_iterator j=methods_.begin(); j!=methods_.end(); ++j)
	{
		const MethodInfo &mi = **j;
		if (mi.getName().compare(name) == 0)
		{
			const ParameterInfoList &other_params = mi.getParameters();
			if (params.size() == other_params.size())
			{
				if (params.empty()) 
					return &mi;
				ParameterInfoList::const_iterator i1 = params.begin();
				ParameterInfoList::const_iterator i2 = other_params.begin();
				for (; i1<params.end(); ++i1, ++i2)
				{
					const ParameterInfo &p1 = **i1;
					const ParameterInfo &p2 = **i2;
					if (p1.getParameterType() == p2.getParameterType() && 
						p1.getAttributes() == p2.getAttributes() &&
						p1.getPosition() == p2.getPosition())
					{
						return &mi;
					}
				}
			}
		}
	}

	if (inherit)
	{
		for (TypeList::const_iterator i=base_.begin(); i!=base_.end(); ++i)
		{
			const MethodInfo *mi = (*i)->getMethod(name, params, true);
			if (mi) return mi;
		}
	}

	return 0;
}

void Type::getInheritedProviders(CustomAttributeProviderList &providers) const
{
	check_defined();
	providers.assign(base_.begin(), base_.end());
}

const PropertyInfo *Type::getProperty(const std::string &name, const Type &ptype, const ParameterInfoList &indices, bool inherit) const
{
	check_defined();
	for (PropertyInfoList::const_iterator i=props_.begin(); i!=props_.end(); ++i)
	{
		const PropertyInfo &pi = **i;
		if (pi.getName() == name && pi.getPropertyType() == ptype)
		{
			const ParameterInfoList &other_indices = pi.getIndexParameters();
			if (indices.size() == other_indices.size())
			{
				if (indices.empty())
					return &pi;
				ParameterInfoList::const_iterator i1 = indices.begin();
				ParameterInfoList::const_iterator i2 = other_indices.begin();
				for (; i1<indices.end(); ++i1, ++i2)
				{
					const ParameterInfo &p1 = **i1;
					const ParameterInfo &p2 = **i2;
					if (p1.getParameterType() == p2.getParameterType() && 
						p1.getAttributes() == p2.getAttributes() &&
						p1.getPosition() == p2.getPosition())
					{
						return &pi;
					}
				}
			}
		}
	}

	if (inherit)
	{
		for (TypeList::const_iterator i=base_.begin(); i!=base_.end(); ++i)
		{
			const PropertyInfo *pi = (*i)->getProperty(name, ptype, indices, true);
			if (pi) return pi;
		}
	}

	return 0;
}

Value Type::invokeMethod(const std::string &name, const Value &instance, ValueList &args, bool inherit) const
{
	check_defined();
	const MethodInfo *mi = getCompatibleMethod(name, args, inherit);
	if (!mi) throw MethodNotFoundException(name, name_);
	return mi->invoke(instance, args);
}

Value Type::invokeMethod(const std::string &name, Value &instance, ValueList &args, bool inherit) const
{
	check_defined();
	const MethodInfo *mi = getCompatibleMethod(name, args, inherit);
	if (!mi) throw MethodNotFoundException(name, name_);
	return mi->invoke(instance, args);
}


void Type::getAllProperties(PropertyInfoList &props) const
{
	check_defined();
	std::copy(props_.begin(), props_.end(),	std::back_inserter(props));
	for (TypeList::const_iterator i=base_.begin(); i!=base_.end(); ++i)
	{
		(*i)->getAllProperties(props);
	}
}

void Type::getAllMethods(MethodInfoList &methods) const
{
	check_defined();
	std::copy(methods_.begin(), methods_.end(), std::back_inserter(methods));
	for (TypeList::const_iterator i=base_.begin(); i!=base_.end(); ++i)
	{
		(*i)->getAllMethods(methods);
	}
}
