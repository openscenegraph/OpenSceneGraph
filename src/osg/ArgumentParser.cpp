#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/Notify>

#include <set>
#include <iostream>

using namespace osg;

bool ArgumentParser::isOption(const char* str)
{
    return str && str[0]=='-';
}

bool ArgumentParser::isString(const char* str)
{
    if (!str) return false;
    return true;
    //return !isOption(str);
}

bool ArgumentParser::isNumber(const char* str)
{
    if (!str) return false;

    bool hadPlusMinus = false;
    bool hadDecimalPlace = false;
    bool hadExponent = false;
    bool couldBeInt = true;
    bool couldBeFloat = true;
    int noZeroToNine = 0;

    const char* ptr = str;
    
    // check if could be a hex number.
    if (strncmp(ptr,"0x",2)==0)
    {
        // skip over leading 0x, and then go through rest of string
        // checking to make sure all values are 0...9 or a..f.
        ptr+=2;
        while (
               *ptr!=0 &&
               ((*ptr>='0' && *ptr<='9') ||
                (*ptr>='a' && *ptr<='f') ||  
                (*ptr>='A' && *ptr<='F'))
              )
        {
            ++ptr;
        }
        
        // got to end of string without failure, therefore must be a hex integer.
        if (*ptr==0) return true;
    }
    
    ptr = str;

    // check if a float or an int.
    while (*ptr!=0 && couldBeFloat)
    {
        if (*ptr=='+' || *ptr=='-')
        {
            if (hadPlusMinus)
            {
                couldBeInt = false;
                couldBeFloat = false;
            } else hadPlusMinus = true;
        }
        else if (*ptr>='0' && *ptr<='9')
        {
            noZeroToNine++;
        }
        else if (*ptr=='.')
        {
            if (hadDecimalPlace)
            {
                couldBeInt = false;
                couldBeFloat = false;
            }
            else
            {
                hadDecimalPlace = true;
                couldBeInt = false;
            }
        }
        else if (*ptr=='e' || *ptr=='E')
        {
            if (hadExponent || noZeroToNine==0)
            {
                couldBeInt = false;
                couldBeFloat = false;
            }
            else
            {
                hadExponent = true;
                couldBeInt = false;
                hadDecimalPlace = false;
                hadPlusMinus = false;
                noZeroToNine=0;
            }
        }
        else
        {
            couldBeInt = false;
            couldBeFloat = false;
        }
        ++ptr;
    }

    if (couldBeInt && noZeroToNine>0) return true;
    if (couldBeFloat && noZeroToNine>0) return true;

    return false;
    
}

bool ArgumentParser::Parameter::valid(const char* str) const
{
    switch(_type)
    {
    case Parameter::FLOAT_PARAMETER:        return isNumber(str); break;
    case Parameter::DOUBLE_PARAMETER:       return isNumber(str); break;
    case Parameter::INT_PARAMETER:          return isNumber(str); break;
    case Parameter::UNSIGNED_INT_PARAMETER: return isNumber(str); break;
    case Parameter::STRING_PARAMETER:       return isString(str); break;
    }
    return false;
}

bool ArgumentParser::Parameter::assign(const char* str)
{
    if (valid(str))
    {
        switch(_type)
        {
        case Parameter::FLOAT_PARAMETER:        *_value._float = atof(str); break;
        case Parameter::DOUBLE_PARAMETER:       *_value._double = atof(str); break;
        case Parameter::INT_PARAMETER:          *_value._int = atoi(str); break;
        case Parameter::UNSIGNED_INT_PARAMETER: *_value._uint = atoi(str); break;
        case Parameter::STRING_PARAMETER:       *_value._string = str; break;
        }
        return true;
    }
    else
    {
        return false;
    }
}



ArgumentParser::ArgumentParser(int* argc,char **argv):
    _argc(argc),
    _argv(argv),
    _usage(ApplicationUsage::instance())
{
#ifdef __APPLE__
    //On OSX, any -psn arguments need to be removed because they will 
    // confuse the application. -psn plus a concatenated argument are
    // passed by the finder to application bundles
    for(int pos=1;pos<this->argc();++pos)
    {
        if (std::string(_argv[pos]).compare(0, 4, std::string("-psn")) == 0) 
        {
            remove(pos, 1);
        }
    }
#endif
}

std::string ArgumentParser::getApplicationName() const
{
    if (_argc && *_argc>0 ) return std::string(_argv[0]);
    return "";
}

 
bool ArgumentParser::isOption(int pos) const
{
    return pos<*_argc && isOption(_argv[pos]);
}

bool ArgumentParser::isString(int pos) const
{
    return pos < *_argc && isString(_argv[pos]);
}

bool ArgumentParser::isNumber(int pos) const
{
    return pos < *_argc && isNumber(_argv[pos]);
}


int ArgumentParser::find(const std::string& str) const
{
    for(int pos=1;pos<*_argc;++pos)
    {
        if (str==_argv[pos])
        {
            return pos;
        }
    }
    return 0;
}

bool ArgumentParser::match(int pos, const std::string& str) const
{
    return pos<*_argc && str==_argv[pos];
}


bool ArgumentParser::containsOptions() const
{
    for(int pos=1;pos<*_argc;++pos)
    {
        if (isOption(pos)) return true;
    }
    return false;
}


void ArgumentParser::remove(int pos,int num)
{
    if (num==0) return;
    
    for(;pos+num<*_argc;++pos)
    {
        _argv[pos]=_argv[pos+num];
    }
    for(;pos<*_argc;++pos)
    {
        _argv[pos]=0;
    }
    *_argc-=num;
}

bool ArgumentParser::read(const std::string& str)
{
    int pos=find(str);
    if (pos<=0) return false;
    remove(pos);
    return true;
}

bool ArgumentParser::read(const std::string& str, Parameter value1)
{
    int pos=find(str);
    if (pos<=0) return false;
    return read(pos,str,value1);
}

bool ArgumentParser::read(const std::string& str, Parameter value1, Parameter value2)
{
    int pos=find(str);
    if (pos<=0) return false;
    return read(pos,str,value1, value2);
}

bool ArgumentParser::read(const std::string& str, Parameter value1, Parameter value2, Parameter value3)
{
    int pos=find(str);
    if (pos<=0) return false;
    return read(pos,str,value1, value2, value3);
}

bool ArgumentParser::read(const std::string& str, Parameter value1, Parameter value2, Parameter value3, Parameter value4)
{
    int pos=find(str);
    if (pos<=0) return false;
    return read(pos,str,value1, value2, value3, value4);
}

bool ArgumentParser::read(const std::string& str, Parameter value1, Parameter value2, Parameter value3, Parameter value4, Parameter value5)
{
    int pos=find(str);
    if (pos<=0) return false;
    return read(pos,str,value1, value2, value3, value4, value5);
}

bool ArgumentParser::read(const std::string& str, Parameter value1, Parameter value2, Parameter value3, Parameter value4, Parameter value5, Parameter value6)
{
    int pos=find(str);
    if (pos<=0) return false;
    return read(pos,str,value1, value2, value3, value4, value5, value6);
}

bool ArgumentParser::read(const std::string& str, Parameter value1, Parameter value2, Parameter value3, Parameter value4, Parameter value5, Parameter value6, Parameter value7)
{
    int pos=find(str);
    if (pos<=0) return false;
    return read(pos,str,value1, value2, value3, value4, value5, value6, value7);
}

bool ArgumentParser::read(const std::string& str, Parameter value1, Parameter value2, Parameter value3, Parameter value4, Parameter value5, Parameter value6, Parameter value7, Parameter value8)
{
    int pos=find(str);
    if (pos<=0) return false;
    return read(pos,str,value1, value2, value3, value4, value5, value6, value7, value8);
}

/** if the argument value at the position pos matches specified string, and subsequent
  * Parameters are also matched then set the Parameter values and remove the from the list of arguments.*/
bool ArgumentParser::read(int pos, const std::string& str)
{
    if (match(pos,str))
    {
        remove(pos,1);
        return true;
    }
    return false;
}

bool ArgumentParser::read(int pos, const std::string& str, Parameter value1)
{
    if (match(pos,str))
    {
        if ((pos+1)<*_argc)
        {
            if (value1.valid(_argv[pos+1]))
            {
                value1.assign(_argv[pos+1]);
                remove(pos,2);
                return true;
            }
            reportError("argument to `"+str+"` is not valid");
            return false;
        }
        reportError("argument to `"+str+"` is missing");
        return false;
    }
    return false;
}

bool ArgumentParser::read(int pos, const std::string& str, Parameter value1, Parameter value2)
{
    if (match(pos,str))
    {
        if ((pos+2)<*_argc)
        {
            if (value1.valid(_argv[pos+1]) &&
                value2.valid(_argv[pos+2]))
            {
                value1.assign(_argv[pos+1]);
                value2.assign(_argv[pos+2]);
                remove(pos,3);
                return true;
            }
            reportError("argument to `"+str+"` is not valid");
            return false;
        }
        reportError("argument to `"+str+"` is missing");
        return false;
    }
    return false;
}

bool ArgumentParser::read(int pos, const std::string& str, Parameter value1, Parameter value2, Parameter value3)
{
    if (match(pos,str))
    {
        if ((pos+3)<*_argc)
        {
            if (value1.valid(_argv[pos+1]) &&
                value2.valid(_argv[pos+2]) &&
                value3.valid(_argv[pos+3]))
            {
                value1.assign(_argv[pos+1]);
                value2.assign(_argv[pos+2]);
                value3.assign(_argv[pos+3]);
                remove(pos,4);
                return true;
            }
            reportError("argument to `"+str+"` is not valid");
            return false;
        }
        reportError("argument to `"+str+"` is missing");
        return false;
    }
    return false;
}

bool ArgumentParser::read(int pos, const std::string& str, Parameter value1, Parameter value2, Parameter value3, Parameter value4)
{
    if (match(pos,str))
    {
        if ((pos+4)<*_argc)
        {
            if (value1.valid(_argv[pos+1]) &&
                value2.valid(_argv[pos+2]) &&
                value3.valid(_argv[pos+3]) &&
                value4.valid(_argv[pos+4]))
            {
                value1.assign(_argv[pos+1]);
                value2.assign(_argv[pos+2]);
                value3.assign(_argv[pos+3]);
                value4.assign(_argv[pos+4]);
                remove(pos,5);
                return true;
            }
            reportError("argument to `"+str+"` is not valid");
            return false;
        }
        reportError("argument to `"+str+"` is missing");
        return false;
    }
    return false;
}

bool ArgumentParser::read(int pos, const std::string& str, Parameter value1, Parameter value2, Parameter value3, Parameter value4, Parameter value5)
{
    if (match(pos,str))
    {
        if ((pos+5)<*_argc)
        {
            if (value1.valid(_argv[pos+1]) &&
                value2.valid(_argv[pos+2]) &&
                value3.valid(_argv[pos+3]) &&
                value4.valid(_argv[pos+4]) &&
                value5.valid(_argv[pos+5]))
            {
                value1.assign(_argv[pos+1]);
                value2.assign(_argv[pos+2]);
                value3.assign(_argv[pos+3]);
                value4.assign(_argv[pos+4]);
                value5.assign(_argv[pos+5]);
                remove(pos,6);
                return true;
            }
            reportError("argument to `"+str+"` is not valid");
            return false;
        }
        reportError("argument to `"+str+"` is missing");
        return false;
    }
    return false;
}

bool ArgumentParser::read(int pos, const std::string& str, Parameter value1, Parameter value2, Parameter value3, Parameter value4, Parameter value5, Parameter value6)
{
    if (match(pos,str))
    {
        if ((pos+6)<*_argc)
        {
            if (value1.valid(_argv[pos+1]) &&
                value2.valid(_argv[pos+2]) &&
                value3.valid(_argv[pos+3]) &&
                value4.valid(_argv[pos+4]) &&
                value5.valid(_argv[pos+5]) &&
                value6.valid(_argv[pos+6]))
            {
                value1.assign(_argv[pos+1]);
                value2.assign(_argv[pos+2]);
                value3.assign(_argv[pos+3]);
                value4.assign(_argv[pos+4]);
                value5.assign(_argv[pos+5]);
                value6.assign(_argv[pos+6]);
                remove(pos,7);
                return true;
            }
            reportError("argument to `"+str+"` is not valid");
            return false;
        }
        reportError("argument to `"+str+"` is missing");
        return false;
    }
    return false;
}

bool ArgumentParser::read(int pos, const std::string& str, Parameter value1, Parameter value2, Parameter value3, Parameter value4, Parameter value5,  Parameter value6,  Parameter value7)
{
    if (match(pos,str))
    {
        if ((pos+7)<*_argc)
        {
            if (value1.valid(_argv[pos+1]) &&
                value2.valid(_argv[pos+2]) &&
                value3.valid(_argv[pos+3]) &&
                value4.valid(_argv[pos+4]) &&
                value5.valid(_argv[pos+5]) &&
                value6.valid(_argv[pos+6]) &&
                value7.valid(_argv[pos+7]))
            {
                value1.assign(_argv[pos+1]);
                value2.assign(_argv[pos+2]);
                value3.assign(_argv[pos+3]);
                value4.assign(_argv[pos+4]);
                value5.assign(_argv[pos+5]);
                value6.assign(_argv[pos+6]);
                value7.assign(_argv[pos+7]);
                remove(pos,8);
                return true;
            }
            reportError("argument to `"+str+"` is not valid");
            return false;
        }
        reportError("argument to `"+str+"` is missing");
        return false;
    }
    return false;
}

bool ArgumentParser::read(int pos, const std::string& str, Parameter value1, Parameter value2, Parameter value3, Parameter value4, Parameter value5,  Parameter value6,  Parameter value7,  Parameter value8)
{
    if (match(pos,str))
    {
        if ((pos+8)<*_argc)
        {
            if (value1.valid(_argv[pos+1]) &&
                value2.valid(_argv[pos+2]) &&
                value3.valid(_argv[pos+3]) &&
                value4.valid(_argv[pos+4]) &&
                value5.valid(_argv[pos+5]) &&
                value6.valid(_argv[pos+6]) &&
                value7.valid(_argv[pos+7]) &&
                value8.valid(_argv[pos+8]))
            {
                value1.assign(_argv[pos+1]);
                value2.assign(_argv[pos+2]);
                value3.assign(_argv[pos+3]);
                value4.assign(_argv[pos+4]);
                value5.assign(_argv[pos+5]);
                value6.assign(_argv[pos+6]);
                value7.assign(_argv[pos+7]);
                value8.assign(_argv[pos+8]);
                remove(pos,9);
                return true;
            }
            reportError("argument to `"+str+"` is not valid");
            return false;
        }
        reportError("argument to `"+str+"` is missing");
        return false;
    }
    return false;
}

bool ArgumentParser::errors(ErrorSeverity severity) const
{
    for(ErrorMessageMap::const_iterator itr=_errorMessageMap.begin();
        itr!=_errorMessageMap.end();
        ++itr)
    {
        if (itr->second>=severity) return true;
    }
    return false;
}

void ArgumentParser::reportError(const std::string& message,ErrorSeverity severity)
{
    _errorMessageMap[message]=severity;
}

void ArgumentParser::reportRemainingOptionsAsUnrecognized(ErrorSeverity severity)
{
    std::set<std::string> options;
    if (_usage) 
    {
        // parse the usage options to get all the option that the application can potential handle.
        for(ApplicationUsage::UsageMap::const_iterator itr=_usage->getCommandLineOptions().begin();
            itr!=_usage->getCommandLineOptions().end();
            ++itr)
        {
            const std::string& option = itr->first;
            std::string::size_type prevpos = 0, pos = 0;
            while ((pos=option.find(' ',prevpos))!=std::string::npos)
            {
                if (option[prevpos]=='-') 
                {
                    options.insert(std::string(option,prevpos,pos-prevpos));
                }
                prevpos=pos+1;
            }
            if (option[prevpos]=='-') 
            {

                options.insert(std::string(option,prevpos,std::string::npos));
            }
        }
        
    }

    for(int pos=1;pos<argc();++pos)
    {
        // if an option and havn't been previous querried for report as unrecognized.
        if (isOption(pos) && options.find(_argv[pos])==options.end()) 
        {
            reportError(getApplicationName() +": unrecognized option "+_argv[pos],severity);
        }
    }
}
void ArgumentParser::writeErrorMessages(std::ostream& output,ErrorSeverity severity)
{
    for(ErrorMessageMap::iterator itr=_errorMessageMap.begin();
        itr!=_errorMessageMap.end();
        ++itr)
    {
        if (itr->second>=severity)
        {
            output<< getApplicationName() << ": " << itr->first << std::endl;
        }
    }
}
