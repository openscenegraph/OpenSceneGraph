#include <osg/ApplicationUsage>

using namespace osg;

ApplicationUsage::ApplicationUsage(const std::string& commandLineUsage):
    _commandLineUsage(commandLineUsage)
{
}

ApplicationUsage* ApplicationUsage::instance()
{
    static ApplicationUsage s_applicationUsage;
    return &s_applicationUsage;
}

void ApplicationUsage::addUsageExplanation(Type type,const std::string& option,const std::string& explanation)
{
    switch(type)
    {
        case(COMMAND_LINE_OPTION):
            addCommandLineOption(option,explanation);
            break;
        case(ENVIRONMENTAL_VARIABLE):
            addEnvironmentalVariable(option,explanation);
            break;
        case(KEYBOARD_MOUSE_BINDING):
            addKeyboardMouseBinding(option,explanation);
            break;
    }
}

void ApplicationUsage::addCommandLineOption(const std::string& option,const std::string& explanation)
{
    _commandLineOptions[option]=explanation;
}

void ApplicationUsage::addEnvironmentalVariable(const std::string& option,const std::string& explanation)
{
    _environmentalVariables[option]=explanation;
}

void ApplicationUsage::addKeyboardMouseBinding(const std::string& option,const std::string& explanation)
{
    _keyboardMouse[option]=explanation;
}

void ApplicationUsage::write(std::ostream& output, const ApplicationUsage::UsageMap& um,unsigned int widthOfOutput)
{
    unsigned int maxNumCharsInOptions = 0;
    ApplicationUsage::UsageMap::const_iterator citr;
    for(citr=um.begin();
        citr!=um.end();
        ++citr)
    {
        maxNumCharsInOptions = std::max(maxNumCharsInOptions,citr->first.length());
    }
    
    unsigned int fullWidth = widthOfOutput;
    unsigned int optionPos = 2;
    unsigned int optionWidth = maxNumCharsInOptions;
    unsigned int explanationPos = 2+maxNumCharsInOptions+2;
    unsigned int explanationWidth = fullWidth-explanationPos;

    std::string line;
    
    for(citr=um.begin();
        citr!=um.end();
        ++citr)
    {
        line.assign(fullWidth,' ');
        line.replace(optionPos,optionWidth,citr->first);
        
        const std::string& explanation = citr->second;
        unsigned int pos = 0;
        unsigned int offset = 0;
        bool firstInLine = true;
        while (pos<explanation.length())
        {
            if (firstInLine) offset = 0;
                    
            // skip any leading white space.
            while (pos<explanation.length() && explanation[pos]==' ')
            {
                if (firstInLine) ++offset;
                ++pos;
            }
            
            firstInLine = false;
        
            unsigned int width = std::min(explanation.length()-pos,explanationWidth-offset);
            unsigned int slashn_pos = explanation.find('\n',pos);
            unsigned int extraSkip = 0;
            bool concatinated = false;
            if (slashn_pos!=std::string::npos)
            {
                if (slashn_pos<pos+width)
                {
                    width = slashn_pos-pos;
                    ++extraSkip;
                    firstInLine = true;
                }
                else if (slashn_pos==pos+width) 
                {
                    ++extraSkip;
                    firstInLine = true;
                }
            }
            
            if (pos+width<explanation.length())
            {
                // now reduce width until we get a space or a return
                // so that we ensure that whole words are printed.
                while (width>0 && 
                       explanation[pos+width]!=' ' && 
                       explanation[pos+width]!='\n') --width;
                       
                if (width==0)
                {
                    // word must be longer than a whole line so will need
                    // to concatinate it.
                    width = explanationWidth-1;
                    concatinated = true;
                }
            }
                                              
            line.replace(explanationPos+offset,explanationWidth, explanation, pos, width);
            if (concatinated) output << line << '-' << std::endl;
            else output << line << std::endl;
            
            
            // move to the next line of output.
            line.assign(fullWidth,' ');
            pos += width+extraSkip;

            
        }
                
    }
}

void ApplicationUsage::write(std::ostream& output,unsigned int widthOfOutput)
{

    output << "Usage: "<<getCommandLineUsage()<<std::endl;
    bool needspace = false;
    if (!getCommandLineOptions().empty())
    {
        if (needspace) output << std::endl;
        output << "Options:"<<std::endl;
        write(output,getCommandLineOptions(),widthOfOutput);
        needspace = true;
    }
    
    if (!getEnvironmentalVariables().empty())
    {
        if (needspace) output << std::endl;
        output << "Environmental Variables:"<<std::endl;
        write(output,getEnvironmentalVariables(),widthOfOutput);
        needspace = true;
    }

    if (!getKeyboardMouseBindings().empty())
    {
        if (needspace) output << std::endl;
        output << "Keyboard and Mouse Bindings:"<<std::endl;
        write(output,getKeyboardMouseBindings(),widthOfOutput);
        needspace = true;
    }

}

