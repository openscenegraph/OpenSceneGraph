#ifndef WAIT_BLOCK_H
#define WAIT_BLOCK_H

#include <OpenThreads/Condition>

namespace osgTXP {

struct WaitBlock{
	OpenThreads::Mutex		mut;
    OpenThreads::Condition cond;

    WaitBlock()
    {
    }

    void wait()
    {
        mut.lock();
        cond.wait(&mut);
        mut.unlock();
    }

    void release()
    {
//        mut.lock();
        cond.broadcast();
//        mut.unlock();
    }
};
}
#endif

