#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/Notify>

#include <set>
#include <iostream>

using namespace osg;

ArgumentParser::ArgumentParser(int* argc,char **argv):
    _argc(argc),
    _argv(argv),
    _usage(ApplicationUsage::instance())
{
}

std::string ArgumentParser::getProgramName() const
{
    if (_argc>0) return std::string(_argv[0]);
    return "";
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

bool ArgumentParser::isOption(int pos) const
{
    return (pos<*_argc && _argv[pos][0]=='-');
}

bool ArgumentParser::isString(int pos) const
{
    return pos<*_argc && !isOption(pos);
}

bool ArgumentParser::isNumber(int pos) const
{
    if (pos>=*_argc) return false;
    
    bool hadPlusMinus = false;
    bool hadDecimalPlace = false;
    bool hadExponent = false;
    bool couldBeInt = true;
    bool couldBeFloat = true;
    int noZeroToNine = 0;

    const char* ptr = _argv[pos];
    
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
    
    ptr = _argv[pos];
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

bool ArgumentParser::read(const std::string& str,std::string& value1)
{
    int pos=find(str);
    if (pos<=0) return false;
    if (!isString(pos+1))
    {
        reportError("argument to `"+str+"` is missing");
        return false;
    }
    value1 = _argv[pos+1];
    remove(pos,2);
    return true;
}

bool ArgumentParser::read(const std::string& str,std::string& value1,std::string& value2)
{
    int pos=find(str);
    if (pos<=0) return false;
    if (!isString(pos+1) || !isString(pos+2) )
    {
        reportError("argument to `"+str+"` is missing");
        return false;
    }
    value1 = _argv[pos+1];
    value2 = _argv[pos+2];
    remove(pos,3);
    return true;
}

bool ArgumentParser::read(const std::string& str,std::string& value1,std::string& value2,std::string& value3)
{
    int pos=find(str);
    if (pos<=0) return false;
    if (!isString(pos+1) || !isString(pos+2)  || !isString(pos+3))
    {
        reportError("argument to `"+str+"` is missing");
        return false;
    }
    value1 = _argv[pos+1];
    value2 = _argv[pos+2];
    value3 = _argv[pos+3];
    remove(pos,4);
    return true;
}


bool ArgumentParser::read(const std::string& str,float& value1)
{
    int pos=find(str);
    if (pos<=0) return false;
    if (!isNumber(pos+1))
    {
        reportError("argument to `"+str+"` is missing");
        return false;
    }
    value1 = atof(_argv[pos+1]);
    remove(pos,2);
    return true;
}

bool ArgumentParser::read(const std::string& str,float& value1,float& value2)
{
    int pos=find(str);
    if (pos<=0) return false;
    if (!isNumber(pos+1) || !isNumber(pos+2) )
    {
        reportError("argument to `"+str+"` is missing");
        return false;
    }
    value1 = atof(_argv[pos+1]);
    value2 = atof(_argv[pos+2]);
    remove(pos,3);
    return true;
}

bool ArgumentParser::read(const std::string& str,float& value1,float& value2,float& value3)
{
    int pos=find(str);
    if (pos<=0) return false;
    if (!isNumber(pos+1) || !isNumber(pos+2)  || !isNumber(pos+3))
    {
        reportError("argument to `"+str+"` is missing");
        return false;
    }
    value1 = atof(_argv[pos+1]);
    value2 = atof(_argv[pos+2]);
    value3 = atof(_argv[pos+3]);
    remove(pos,4);
    return true;
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
            unsigned int prevpos = 0, pos = 0;
            while ((pos=option.find(' ',prevpos))!=std::string::npos)
            {
                if (option[prevpos]=='-') 
                {
                    // verbose approach implemented for debugging string(const string&,unsigned int,unsigned int) operation on x86-64
                    notify(INFO)<<"option=\""<<option<<"\" \tprevpos="<<prevpos<<" \tn="<<pos-prevpos;

                    std::string str(option,prevpos,pos-prevpos);
                    
                    notify(INFO)<<" \tstr=\""<<str<<"\"";
                    
                    options.insert(str);
                    
                    notify(INFO)<<" \tinserted into options set"<<std::endl;
                    
                    
                    // original op which causes a crash under x86-64
                    //options.insert(std::string(option,prevpos,pos-prevpos));
                }
                prevpos=pos+1;
            }
            if (option[prevpos]=='-') 
            {

                // verbose approach implemented for debugging string(const string&,unsigned int,unsigned int) operation on x86-64
                notify(INFO)<<"option=\""<<option<<"\"  \tprevpos="<<prevpos<<" \tn=npos";

                std::string str(option,prevpos,pos-prevpos);

                notify(INFO)<<" \tstr=\""<<str<<"\"";

                options.insert(str);

                notify(INFO)<<" \tinserted into options set"<<std::endl;

                // original op
                //options.insert(std::string(option,prevpos,std::string::npos));
            }
        }
        
    }

    for(int pos=1;pos<argc();++pos)
    {
        // if an option and havn't been previous querried for report as unrecognized.
        if (isOption(pos) && options.find(_argv[pos])==options.end()) 
        {
            reportError(getProgramName() +": unrceognized option "+_argv[pos],severity);
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
            output<< getProgramName() << ": " << itr->first << std::endl;
        }
    }
}
