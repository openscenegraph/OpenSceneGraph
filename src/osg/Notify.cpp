#include "osg/Notify"
#include <string>

using namespace osg;

int osg::NotifyInit::count_ = 0;
NotifySeverity osg::g_NotifyLevel = osg::NOTICE;
ofstream *osg::g_absorbStreamPtr = NULL;

void osg::setNotifyLevel(NotifySeverity severity)
{
    g_NotifyLevel = severity; 
}

int osg::getNotifyLevel()
{
    return g_NotifyLevel;
}

#ifndef WIN32
ostream& osg::notify(NotifySeverity severity)
{
    if (severity<=g_NotifyLevel || g_absorbStreamPtr==NULL)
    {
        if (severity<=osg::WARN) return cerr;
        else return cout;
    }
    return *g_absorbStreamPtr;
}
#endif

NotifyInit::NotifyInit()
{
    if (count_++ == 0) {

	// g_NotifyLevel
	// =============

	g_NotifyLevel = osg::NOTICE;	// Default value

	char *OSGNOTIFYLEVEL=getenv("OSGNOTIFYLEVEL");
	if(OSGNOTIFYLEVEL){

	    std::string stringOSGNOTIFYLEVEL(OSGNOTIFYLEVEL);

	    // Convert to upper case
	    for(std::string::iterator i=stringOSGNOTIFYLEVEL.begin();
		i!=stringOSGNOTIFYLEVEL.end();
		++i) *i=toupper(*i);

	    if(stringOSGNOTIFYLEVEL.find("ALWAYS")!=std::string::npos)	        g_NotifyLevel=osg::ALWAYS;
	    else if(stringOSGNOTIFYLEVEL.find("FATAL")!=std::string::npos)	g_NotifyLevel=osg::FATAL;
	    else if(stringOSGNOTIFYLEVEL.find("WARN")!=std::string::npos)	g_NotifyLevel=osg::WARN;
	    else if(stringOSGNOTIFYLEVEL.find("NOTICE")!=std::string::npos)	g_NotifyLevel=osg::NOTICE;
	    else if(stringOSGNOTIFYLEVEL.find("INFO")!=std::string::npos)	g_NotifyLevel=osg::INFO;
	    else if(stringOSGNOTIFYLEVEL.find("DEBUG")!=std::string::npos)	g_NotifyLevel=osg::DEBUG;
	    else if(stringOSGNOTIFYLEVEL.find("FP_DEBUG")!=std::string::npos)	g_NotifyLevel=osg::FP_DEBUG;

	}

	// g_absorbStreamPtr
	// =================

	char *OSGNOTIFYABSORBFILE=getenv("OSGNOTIFYABSORBFILE");
        if (OSGNOTIFYABSORBFILE)
        {
	    g_absorbStreamPtr=new ofstream(OSGNOTIFYABSORBFILE);
        }
        else
        {
#ifdef WIN32
	    // What's the Windows equivalent of /dev/null?
	    g_absorbStreamPtr=new ofstream("C:/Windows/Tmp/osg.log");
#else
	    g_absorbStreamPtr=new ofstream("/dev/null");
#endif
        }
    }
}

NotifyInit::~NotifyInit()
{
    if(--count_ == 0) {
	delete g_absorbStreamPtr;
	g_absorbStreamPtr=NULL;
    }
}
