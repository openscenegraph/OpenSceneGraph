//#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <osg/Timer>

using namespace osg;

// follows are the constructors of the Timer class, once version
// for each OS combination.  The order is WIN32, FreeBSD, Linux, IRIX,
// and the rest of the world.
//
// all the rest of the timer methods are implemented within the header.

#ifdef WIN32

    #include <sys/types.h>
    #include <fcntl.h>
    #include <windows.h>
    #include <winbase.h>
    Timer::Timer()
    {
        _useStandardClock = false;
        
        if (_useStandardClock)
        {
            _secsPerClick = (1.0 / (double) CLOCKS_PER_SEC);            
        }
        else
        {

            Timer_t start_time = tick();
            Sleep (1000);
            Timer_t end_time = tick();

            _secsPerClick = 1.0/(double)(end_time-start_time);
        }
    }

#elif defined(__FreeBSD__)

    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/sysctl.h>
    #include <sys/types.h>
    Timer::Timer()
    {
        _useStandardClock = false;
        
        if (_useStandardClock)
        {
            _secsPerClick = 1e-6; // gettimeofday()'s precision.
        }
        else
        {
            int cpuspeed;
            size_t len;

            len = sizeof(cpuspeed);
            if (sysctlbyname("machdep.tsc_freq", &cpuspeed, &len, NULL, NULL) == -1)
            {
                _useStandardClock = true;
	        perror("sysctlbyname(machdep.tsc_freq)");
                return;
            }

            _secsPerClick = 1.0/cpuspeed;
        }
    }
    
#elif defined(__linux) 

    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/mman.h>
    #include <sys/types.h>

    Timer::Timer()
    {
        _useStandardClock = false;
        
        if (_useStandardClock)
        {
            _secsPerClick = 1e-6; // gettimeofday()'s precision.
        }
        else
        {
            char buff[128];
            FILE *fp = fopen( "/proc/cpuinfo", "r" );

            double cpu_mhz=0.0f;

            while( fgets( buff, sizeof( buff ), fp ) > 0 )
            {
                if( !strncmp( buff, "cpu MHz", strlen( "cpu MHz" )))
	        {
	            char *ptr = buff;

	            while( ptr && *ptr != ':' ) ptr++;
	            if( ptr ) 
	            {
	              ptr++;
	              sscanf( ptr, "%lf", &cpu_mhz );
	            }
	            break;
	        }
            }
            fclose( fp );

            if (cpu_mhz==0.0f)
            {
                // error - no cpu_mhz found.
                Timer_t start_time = tick();
                sleep (1);
                Timer_t end_time = tick();
                _secsPerClick = 1.0/(double)(end_time-start_time);
            }
            else
            {
                _secsPerClick = 1e-6/cpu_mhz;
            }
        }        
    }

#elif defined(__sgi)

    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/syssgi.h>
    #include <sys/immu.h>
    #include <sys/mman.h>

    unsigned long Timer::_dummy = 0;

    Timer::Timer( void )
    {
        _useStandardClock = false;

        if (_useStandardClock)
        {
            _secsPerClick = 1e-6; // gettimeofday()'s precision.
        }
        else
        {
            __psunsigned_t phys_addr, raddr;
            unsigned int cycleval;
            volatile unsigned long counter_value, *iotimer_addr;
            int fd, poffmask;

            poffmask     = getpagesize() - 1;
            phys_addr     = syssgi( SGI_QUERY_CYCLECNTR, &cycleval );
            raddr        = phys_addr & ~poffmask;

            _clockAddress = &_dummy;

            if( (fd = open( "/dev/mmem", O_RDONLY )) < 0 )
            {
                perror( "/dev/mmem" );
                return;
            }

            iotimer_addr = (volatile unsigned long *)mmap(
                (void *)0L,
                (size_t)poffmask,
                (int)PROT_READ,
                (int)MAP_PRIVATE, fd, (off_t)raddr);

            iotimer_addr = (unsigned long *)(
                (__psunsigned_t)iotimer_addr + (phys_addr & poffmask)
                );

            _cycleCntrSize = syssgi( SGI_CYCLECNTR_SIZE );

            if( _cycleCntrSize > 32 )
                ++iotimer_addr;

            _clockAddress = (unsigned long *)iotimer_addr;
            _secsPerClick = (double)(cycleval)* 1e-12;
        }
    }
#elif defined (__APPLE_CC__)  || defined (macintosh)

#if defined (__APPLE_CC__)
	#include <Carbon/Carbon.h>         // do I really have to link against the Carbon framework just for this?
#else	
	#include <MacTypes.h>
	#include <Timer.h>
#endif

Timer_t Timer::tick(void)
{
	UnsignedWide usecs;
	Microseconds(&usecs);
	
	return (usecs.hi * 4294967296.0) + usecs.lo;
}

Timer::Timer( void )
{
	_useStandardClock = false;
	_secsPerClick = 1e-6; // Carbon timer's precision.

}

#elif defined(unix)

    Timer::Timer( void )
    {
        _useStandardClock = true;
        _secsPerClick = 1e-6; // gettimeofday()'s precision.
    }

#else 

    // handle the rest of the OS world by just using the std::clock,

    Timer::Timer( void )
    {
        _useStandardClock = true;
        _secsPerClick = (1.0 / (double) CLOCKS_PER_SEC);
    }

#endif
