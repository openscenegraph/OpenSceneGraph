#include <osg/Notify>
#include <string>

using namespace std;

osg::NotifySeverity osg::g_NotifyLevel = osg::NOTICE;
std::ofstream *osg::g_NotifyNulStream; 
bool osg::g_NotifyInit = false;

void osg::setNotifyLevel(osg::NotifySeverity severity)
{
    osg::initNotifyLevel();
    g_NotifyLevel = severity;
}


osg::NotifySeverity osg::getNotifyLevel()
{
    osg::initNotifyLevel();
    return g_NotifyLevel;
}


bool osg::initNotifyLevel()
{
    if (g_NotifyInit) return true;
    
    g_NotifyInit = true;

    // set up global notify null stream for inline notify
#if defined(WIN32) && !defined(__CYGWIN__)
    g_NotifyNulStream = new std::ofstream ("nul");
#else
    g_NotifyNulStream = new std::ofstream ("/dev/null");
#endif

    // g_NotifyLevel
    // =============

    g_NotifyLevel = osg::NOTICE; // Default value

    char *OSGNOTIFYLEVEL=getenv("OSGNOTIFYLEVEL");
    if(OSGNOTIFYLEVEL)
    {

        std::string stringOSGNOTIFYLEVEL(OSGNOTIFYLEVEL);

        // Convert to upper case
        for(std::string::iterator i=stringOSGNOTIFYLEVEL.begin();
            i!=stringOSGNOTIFYLEVEL.end();
            ++i)
        {
            *i=toupper(*i);
        }

        if(stringOSGNOTIFYLEVEL.find("ALWAYS")!=std::string::npos)          g_NotifyLevel=osg::ALWAYS;
        else if(stringOSGNOTIFYLEVEL.find("FATAL")!=std::string::npos)      g_NotifyLevel=osg::FATAL;
        else if(stringOSGNOTIFYLEVEL.find("WARN")!=std::string::npos)       g_NotifyLevel=osg::WARN;
        else if(stringOSGNOTIFYLEVEL.find("NOTICE")!=std::string::npos)     g_NotifyLevel=osg::NOTICE;
        else if(stringOSGNOTIFYLEVEL.find("INFO")!=std::string::npos)       g_NotifyLevel=osg::INFO;
        else if(stringOSGNOTIFYLEVEL.find("DEBUG_INFO")!=std::string::npos) g_NotifyLevel=osg::DEBUG_INFO;
        else if(stringOSGNOTIFYLEVEL.find("DEBUG_FP")!=std::string::npos)   g_NotifyLevel=osg::DEBUG_FP;
        else if(stringOSGNOTIFYLEVEL.find("DEBUG")!=std::string::npos)      g_NotifyLevel=osg::DEBUG_INFO;
 
    }

    return true;

}
