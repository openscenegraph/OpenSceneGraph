/* OpenSceneGraph example, osgintrospection.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/


#include <osg/ref_ptr>

#include <osgIntrospection/Reflection>
#include <osgIntrospection/Type>
#include <osgIntrospection/MethodInfo>
#include <osgIntrospection/PropertyInfo>

#include <osgDB/DynamicLibrary>

#include <iostream>
#include <algorithm>

using namespace osgIntrospection;


// borrowed from osgDB...
std::string createLibraryNameForWrapper(const std::string& ext)
{
#if defined(WIN32)
    // !! recheck evolving Cygwin DLL extension naming protocols !! NHV
    #ifdef __CYGWIN__
        return "cygwin_osgwrapper_"+ext+".dll";
    #elif defined(__MINGW32__)
        return "mingw_osgwrapper_"+ext+".dll";
    #else
        #ifdef _DEBUG
            return "osgwrapper_"+ext+"d.dll";
        #else
            return "osgwrapper_"+ext+".dll";
        #endif
    #endif
#elif macintosh
    return "osgwrapper_"+ext;
#elif defined(__hpux__)
    // why don't we use PLUGIN_EXT from the makefiles here?
    return "osgwrapper_"+ext+".sl";
#else
    return "osgwrapper_"+ext+".so";
#endif

}

bool type_order(const Type *v1, const Type *v2)
{
    if (!v1->isDefined()) return v2->isDefined();
    if (!v2->isDefined()) return false;
    return v1->getQualifiedName().compare(v2->getQualifiedName()) < 0;
}

void print_method(const MethodInfo &mi)
{
    std::cout << "\t    ";

    // display if the method is virtual
    if (mi.isVirtual())
        std::cout << "virtual ";

    // display the method's return type if defined
    if (mi.getReturnType().isDefined())
        std::cout << mi.getReturnType().getQualifiedName() << " ";
    else
        std::cout << "[UNDEFINED TYPE] ";

    // display the method's name
    std::cout << mi.getName() << "(";

    // display method's parameters
    const ParameterInfoList &params = mi.getParameters();
    for (ParameterInfoList::const_iterator k=params.begin(); k!=params.end(); ++k)
    {
        // get the ParameterInfo object that describes the 
        // current parameter
        const ParameterInfo &pi = **k;

        // display the parameter's modifier
        if (pi.isIn())
            std::cout << "IN";
        if (pi.isOut())
            std::cout << "OUT";
        if (pi.isIn() || pi.isOut())
            std::cout << " ";

        // display the parameter's type name
        if (pi.getParameterType().isDefined())
            std::cout << pi.getParameterType().getQualifiedName();

        // display the parameter's name if defined
        if (!pi.getName().empty())
            std::cout << " " << pi.getName();

        if ((k+1)!=params.end())
            std::cout << ", ";
    }
    std::cout << ")";
    if (mi.isConst())
        std::cout << " const";
    if (mi.isPureVirtual())
        std::cout << " = 0";
    std::cout << "\n";
}

void print_type(const Type &type)
{
    // ignore pointer types and undefined types
    if (!type.isDefined() || type.isPointer() || type.isReference())
        return;

    // print the type name
    std::cout << type.getQualifiedName() << "\n";

    // check whether the type is abstract
    if (type.isAbstract()) std::cout << "\t[abstract]\n";

    // check whether the type is atomic
    if (type.isAtomic()) std::cout << "\t[atomic]\n";

    // check whether the type is an enumeration. If yes, display
    // the list of enumeration labels
    if (type.isEnum()) 
    {
        std::cout << "\t[enum]\n";
        std::cout << "\tenumeration values:\n";
        const EnumLabelMap &emap = type.getEnumLabels();
        for (EnumLabelMap::const_iterator j=emap.begin(); j!=emap.end(); ++j)
        {
            std::cout << "\t\t" << j->second << " = " << j->first << "\n";
        }
    }

    // if the type has one or more base types, then display their
    // names
    if (type.getNumBaseTypes() > 0)
    {
        std::cout << "\tderived from: ";
        for (int j=0; j<type.getNumBaseTypes(); ++j)
        {
            const Type &base = type.getBaseType(j);
            if (base.isDefined())
                std::cout << base.getQualifiedName() << "    ";
            else
                std::cout << "[undefined type]    ";
        }
        std::cout << "\n";
    }

    // display a list of public methods defined for the current type
    const MethodInfoList &mil = type.getMethods();
    if (!mil.empty())
    {
        std::cout << "\t* public methods:\n";
        for (MethodInfoList::const_iterator j=mil.begin(); j!=mil.end(); ++j)
        {
            // get the MethodInfo object that describes the current
            // method
            const MethodInfo &mi = **j;

            print_method(mi);
        }
    }

    // display a list of protected methods defined for the current type
    const MethodInfoList &bmil = type.getMethods(Type::PROTECTED_FUNCTIONS);
    if (!bmil.empty())
    {
        std::cout << "\t* protected methods:\n";
        for (MethodInfoList::const_iterator j=bmil.begin(); j!=bmil.end(); ++j)
        {
            // get the MethodInfo object that describes the current
            // method
            const MethodInfo &mi = **j;

            print_method(mi);
        }
    }

    // display a list of properties defined for the current type
    const PropertyInfoList &pil = type.getProperties();
    if (!pil.empty())
    {
        std::cout << "\t* properties:\n";
        for (PropertyInfoList::const_iterator j=pil.begin(); j!=pil.end(); ++j)
        {
            // get the PropertyInfo object that describes the current
            // property
            const PropertyInfo &pi = **j;

            std::cout << "\t    ";

            std::cout << "{";
            std::cout << (pi.canGet()? "G": " ");
            std::cout << (pi.canSet()? "S": " ");
            std::cout << (pi.canCount()? "C": " ");
            std::cout << (pi.canAdd()? "A": " ");
            std::cout << "}  ";

            // display the property's name
            std::cout << pi.getName();

            // display the property's value type if defined
            std::cout << " (";
            if (pi.getPropertyType().isDefined())
                std::cout << pi.getPropertyType().getQualifiedName();
            else
                std::cout << "UNDEFINED TYPE";
            std::cout << ") ";

            // check whether the property is an array property
            if (pi.isArray())
            {
                std::cout << "  [ARRAY]";
            }

            // check whether the property is an indexed property
            if (pi.isIndexed())
            {
                std::cout << "  [INDEXED]\n\t\t       indices:\n";

                const ParameterInfoList &ind = pi.getIndexParameters();

                // print the list of indices
                int num = 1;
                for (ParameterInfoList::const_iterator k=ind.begin(); k!=ind.end(); ++k, ++num)
                {
                    std::cout << "\t\t           " << num << ") ";
                    const ParameterInfo &par = **k;
                    std::cout << par.getParameterType().getQualifiedName() << " " << par.getName();
                    std::cout << "\n";
                }
            }

            std::cout << "\n";
        }
    }
    std::cout << "\n" << std::string(75, '-') << "\n";
}

void print_types()
{
    // get the map of types that have been reflected
    const TypeMap &tm = Reflection::getTypes();
    
    // create a sortable list of types
    TypeList types(tm.size());
    TypeList::iterator j = types.begin();
    for (TypeMap::const_iterator i=tm.begin(); i!=tm.end(); ++i, ++j)
        *j = i->second;
    
    // sort the map
    std::sort(types.begin(), types.end(), &type_order);

    // iterate through the type map and display some
    // details for each type
    for (TypeList::const_iterator i=types.begin(); i!=types.end(); ++i)
    {
        print_type(**i);
    }
}

int main()
{
    // load the library of wrappers that reflect the 
    // classes defined in the 'osg' namespace. In the
    // future this will be done automatically under
    // certain circumstances (like deserialization).
    osg::ref_ptr<osgDB::DynamicLibrary> osg_reflectors = 
        osgDB::DynamicLibrary::loadLibrary(createLibraryNameForWrapper("osg"));
    
    // display a detailed list of reflected types
    try
    {
        print_types();
    }
    catch(const osgIntrospection::Exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

