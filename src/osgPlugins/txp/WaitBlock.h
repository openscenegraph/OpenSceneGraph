#ifndef WAIT_BLOCK_H
#define WAIT_BLOCK_H

#if !defined(WIN32) || defined(__CYGWIN__)
#include <pthread.h>

namespace osgTXP {

struct WaitBlock{
    pthread_mutex_t mut;
    pthread_cond_t  cond;

    WaitBlock()
    {
        pthread_mutex_init( &mut, 0L );
        pthread_cond_init( &cond, 0L );
    }

    void wait()
    {
        pthread_mutex_lock( &mut );
        pthread_cond_wait( &cond, &mut);
        pthread_mutex_unlock(&mut);
    }

    void release()
    {
        pthread_mutex_lock( &mut );
        pthread_cond_broadcast( &cond );
        pthread_mutex_unlock( &mut );
    }
};
}
#endif
#endif

