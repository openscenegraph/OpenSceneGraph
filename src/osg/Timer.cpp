#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>

#include <osg/Timer>


using namespace osg;

#ifdef WIN32 // [

#include <windows.h>
#include <winbase.h>

int Timer::inited = 0;
double Timer::cpu_mhz = 0.0;

void Timer::init( void )
{
    Timer_t start_time = tick();
    Sleep (1000);
    Timer_t end_time = tick();
    cpu_mhz = (double)(end_time-start_time)*1e-6;
    inited = 1;
}

Timer::Timer( void )
{
     if( !inited ) init();
}

Timer::~Timer( void )
{
}

double Timer::delta_s( Timer_t t1, Timer_t t2 )
{
    Timer_t delta = t2 - t1;
    return (((double)delta/cpu_mhz)*1e-6);
}

double Timer::delta_m( Timer_t t1, Timer_t t2 )
{
    Timer_t delta = t2 - t1;
    return (((double)delta/cpu_mhz)*1e-3);
}

Timer_t Timer::delta_u( Timer_t t0, Timer_t t1 )
{
    return (Timer_t)((double)(t1 - t0)/cpu_mhz);
}

Timer_t Timer::delta_n( Timer_t t0, Timer_t t1 )
{
    return (Timer_t)((double)(t1 - t0) * 1e3/cpu_mhz);
}

#endif // ]

#ifdef __linux // [
#include <unistd.h>
#include <sys/mman.h>

int Timer::inited = 0;
double Timer::cpu_mhz = 0.0;

void Timer::init( void )
{
    char buff[128];
    FILE *fp = fopen( "/proc/cpuinfo", "r" );

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
    inited = 1;
}

Timer::Timer( void )
{
     if( !inited ) init();
}

Timer::~Timer( void )
{
}

double Timer::delta_s( Timer_t t1, Timer_t t2 )
{
    Timer_t delta = t2 - t1;
    return ((double)delta/cpu_mhz*1e-6);
}

double Timer::delta_m( Timer_t t1, Timer_t t2 )
{
    Timer_t delta = t2 - t1;
    return ((double)delta/cpu_mhz*1e-3);
}

Timer_t Timer::delta_u( Timer_t t0, Timer_t t1 )
{
    return (Timer_t)((double)(t1 - t0)/cpu_mhz);
}

Timer_t Timer::delta_n( Timer_t t0, Timer_t t1 )
{
    return (Timer_t)((double)(t1 - t0) * 1e3/cpu_mhz);
}

#endif // ]
#ifdef __sgi // [


#include <unistd.h>
#include <sys/syssgi.h>
#include <sys/immu.h>
#include <sys/mman.h>

unsigned long Timer::dummy = 0;


Timer::Timer( void )
{
    __psunsigned_t phys_addr, raddr;
    unsigned int cycleval;
    volatile unsigned long counter_value, *iotimer_addr;
    int fd, poffmask;

    poffmask     = getpagesize() - 1;
    phys_addr     = syssgi( SGI_QUERY_CYCLECNTR, &cycleval );
    microseconds_per_click = (double)cycleval/1e6;
    nanoseconds_per_click  = (double)cycleval/1e3;
    raddr        = phys_addr & ~poffmask;

    clk = &dummy;

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


    cycleCntrSize = syssgi( SGI_CYCLECNTR_SIZE );

    if( cycleCntrSize > 32 ) 
        ++iotimer_addr;

    clk = (unsigned long *)iotimer_addr;
}

Timer::~Timer( void )
{
}

double Timer::delta_s( Timer_t t1, Timer_t t2 )
{
    Timer_t delta = t2 - t1;
    return ((double)delta * microseconds_per_click*1e-6);
}

double Timer::delta_m( Timer_t t1, Timer_t t2 )
{
    Timer_t delta = t2 - t1;
    return ((double)delta * microseconds_per_click*1e-3);
}

Timer_t Timer::delta_u( Timer_t t1, Timer_t t2 )
{
    Timer_t delta = t2 - t1;
    return (Timer_t)((double)delta * microseconds_per_click);
}

Timer_t Timer::delta_n( Timer_t t1, Timer_t t2 )
{
    unsigned long delta = t2 - t1;
    return (Timer_t )((double)delta * nanoseconds_per_click);
}

#endif // ]
