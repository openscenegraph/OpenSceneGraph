#include <osg/Notify>
#include <string>

using namespace std;

osg::NotifySeverity g_NotifyLevel = osg::NOTICE;

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
    static bool s_NotifyInit = false;

    if (s_NotifyInit) return true;
    
    s_NotifyInit = true;

    // g_NotifyLevel
    // =============

    g_NotifyLevel = osg::NOTICE; // Default value

    char* OSGNOTIFYLEVEL=getenv("OSG_NOTIFY_LEVEL");
    if (!OSGNOTIFYLEVEL) OSGNOTIFYLEVEL=getenv("OSGNOTIFYLEVEL");
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

std::ostream& osg::notify(const osg::NotifySeverity severity)
{
    // set up global notify null stream for inline notify
#if defined(WIN32) && !(defined(__CYGWIN__) || defined(__MINGW32__))
    static std::ofstream s_NotifyNulStream("nul");
#else
    static std::ofstream s_NotifyNulStream("/dev/null");
#endif

    static bool initialized = osg::initNotifyLevel();
    initialized=initialized; // statement with no effect to stop GCC warning.

    if (severity<=g_NotifyLevel)
    {
        if (severity<=osg::WARN) return std::cerr;
        else return std::cout;
    }
    return s_NotifyNulStream;
}
