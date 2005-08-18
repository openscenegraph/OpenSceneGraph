/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/


#include <osg/GraphicsThread>
#include <osg/GraphicsContext>
#include <osg/Notify>

using namespace osg;


GraphicsThread::~GraphicsThread()
{
}

void GraphicsThread::add(Operation* operation, bool waitForCompletion)
{
    osg::BarrierOperation* barrier = 0;

    {
        // aquire the lock on the operations queue to prevent anyone else for modifying it at the same time
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

        // add the operation to the end of the list
        _operations.push_back(operation);

        if (waitForCompletion)
        {
            barrier = new BarrierOperation(2);
            _operations.push_back(barrier);
        }
    }
    
    if (barrier)
    {
        // now we wait till the barrier is joined by the graphics thread.
        barrier->block();
    }
}

void GraphicsThread::run()
{
    // make the graphics context current.
    if (_graphicsContext) _graphicsContext->makeCurrent();

    bool firstTime = false;

    bool _done = false;

    do
    {
        ref_ptr<Operation> operation;
    
        // get the front of the file request list.
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);
            if (!_operations.empty())
            {
                // get the front the queue
                operation = _operations.front();
                
                // remove it from the opeations queue
                _operations.erase(_operations.begin());
            }
            
        }
        
        if (operation.valid())
        {
            // call the graphics operation.
            (*operation)(_graphicsContext);
        }

        if (firstTime)
        {
            // do a yield to get round a peculiar thread hang when testCancel() is called 
            // in certain cirumstances - of which there is no particular pattern.
            YieldCurrentThread();
            firstTime = false;
        }

    } while (!testCancel() && !_done);

    // release the graphics context so that others can aquire it.
    if (_graphicsContext) _graphicsContext->releaseContext();

}

void SwapBufferOperation::operator () (GraphicsContext* context)
{
    if (context) context->swapBuffers();
}

void BarrierOperation::operator () (GraphicsContext*)
{
    block();
}

void ReleaseContext_Block_MakeCurrentOperation::operator () (GraphicsContext* context)
{
    if (!context) return;
    
    // release the graphics context.
    context->releaseContext();
    
    // reset the block so that it the next call to block() 
    reset();
    
    // block this thread, untill the block is released externally.
    block();
    
    // re aquire the graphcis context.
    context->makeCurrent();
}
