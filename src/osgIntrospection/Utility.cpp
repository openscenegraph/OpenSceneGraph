#include <osgIntrospection/Utility>

using namespace osgIntrospection;

bool osgIntrospection::areParametersCompatible(const ParameterInfoList &pl1, const ParameterInfoList &pl2)
{
    if (pl1.size() == pl2.size())
    {
        if (pl1.empty()) 
            return true;

        ParameterInfoList::const_iterator i1 = pl1.begin();
        ParameterInfoList::const_iterator i2 = pl2.begin();
        for (; i1<pl1.end(); ++i1, ++i2)
        {
            const ParameterInfo &p1 = **i1;
            const ParameterInfo &p2 = **i2;
            if (p1.getParameterType() == p2.getParameterType() && 
                p1.getAttributes() == p2.getAttributes())
            {
                return true;
            }
        }
    }

	return false;
}

bool osgIntrospection::areArgumentsCompatible(const ValueList &vl, const ParameterInfoList &pl, float &match)
{
	if (pl.empty())
	{
		if (vl.empty())
		{
			match = 1.0f;
			return true;
		}
		return false;
	}

    ParameterInfoList::const_iterator i1 = pl.begin();
    ValueList::const_iterator i2 = vl.begin();

    int exact_args = 0;

    for (; i1<pl.end(); ++i1)
    {
		if (i2 == vl.end())
		{
			if ((*i1)->getDefaultValue().isEmpty())
				return false;
			continue;
		}

        if ((*i1)->getParameterType() != i2->getType())
        {
            if (i2->tryConvertTo((*i1)->getParameterType()).isEmpty())
            {
				return false;
            }                        
        }
        else
            ++exact_args;

		++i2;
    }

	match = static_cast<float>(exact_args) / pl.size();
	return true;
}

