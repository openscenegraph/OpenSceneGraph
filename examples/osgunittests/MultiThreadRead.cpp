/* OpenSceneGraph example, osgunittests.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osg/Referenced>
#include <osgDB/ReadFile>
#include <osgDB/Registry>

#include <OpenThreads/Thread>
#include <OpenThreads/ScopedLock>

struct RefBarrier : public osg::Referenced, public OpenThreads::Barrier
{
    RefBarrier(int numThreads):
        OpenThreads::Barrier(numThreads) {}
};

class ReadThread : public osg::Referenced, public OpenThreads::Thread
{
public:

    ReadThread():
        _done(false)
    {
    }

    virtual ~ReadThread()
    {
        _done = true;

        if (isRunning())
        {
            cancel();
            join();
        }
    }

    void addFileName(const std::string& filename)
    {
        _fileNames.push_back(filename);
    }

    void setStartBarrier(RefBarrier* barrier) { _startBarrier = barrier; }
    void setEndBarrier(RefBarrier* barrier) { _endBarrier = barrier; }

    virtual void run()
    {
        if (_startBarrier.valid())
        {
#if VERBOSE
            std::cout<<"Waiting on start block "<<this<<std::endl;
#endif
            _startBarrier->block();
        }

#if VERBOSE
        std::cout<<"Starting "<<this<<std::endl;
#endif

        do
        {
            if (!_fileNames.empty())
            {
                // take front filename;
                std::string filename = _fileNames.front();
                _fileNames.erase(_fileNames.begin());

#if VERBOSE
                std::cout<<"Reading "<<filename;
#endif
                osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(filename);
#if VERBOSE
                if (node.valid()) std::cout<<"..  OK"<<std::endl;
                else std::cout<<"..  FAILED"<<std::endl;
#endif
            }

        } while (!testCancel() && !_fileNames.empty() && !_done);

        if (_endBarrier.valid())
        {
#if VERBOSE
            std::cout<<"Waiting on end block "<<this<<std::endl;
#endif
            _endBarrier->block();
        }

#if VERBOSE
        std::cout<<"Completed"<<this<<std::endl;
#endif
    }

    typedef std::list<std::string> FileNames;
    FileNames                   _fileNames;
    bool                        _done;
    osg::ref_ptr<RefBarrier>    _startBarrier;
    osg::ref_ptr<RefBarrier>    _endBarrier;
};



class SerializerReadFileCallback : public osgDB::Registry::ReadFileCallback
{
public:

    virtual osgDB::ReaderWriter::ReadResult openArchive(const std::string& filename,osgDB::ReaderWriter::ArchiveStatus status, unsigned int indexBlockSizeHint, const osgDB::ReaderWriter::Options* useObjectCache)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        return osgDB::Registry::instance()->openArchiveImplementation(filename, status, indexBlockSizeHint, useObjectCache);
    }

    virtual osgDB::ReaderWriter::ReadResult readObject(const std::string& filename, const osgDB::ReaderWriter::Options* options)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        return osgDB::Registry::instance()->readObjectImplementation(filename,options);
    }

    virtual osgDB::ReaderWriter::ReadResult readImage(const std::string& filename, const osgDB::ReaderWriter::Options* options)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        return osgDB::Registry::instance()->readImageImplementation(filename,options);
    }

    virtual osgDB::ReaderWriter::ReadResult readHeightField(const std::string& filename, const osgDB::ReaderWriter::Options* options)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        return osgDB::Registry::instance()->readHeightFieldImplementation(filename,options);
    }

    virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& filename, const osgDB::ReaderWriter::Options* options)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        return osgDB::Registry::instance()->readNodeImplementation(filename,options);
    }

    virtual osgDB::ReaderWriter::ReadResult readShader(const std::string& filename, const osgDB::ReaderWriter::Options* options)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        return osgDB::Registry::instance()->readShaderImplementation(filename,options);
    }

protected:
       virtual ~SerializerReadFileCallback() {}

       OpenThreads::Mutex _mutex;
};



void runMultiThreadReadTests(int numThreads, osg::ArgumentParser& arguments)
{
#if VERBOSE
    osg::notify(osg::NOTICE)<<"runMultiThreadReadTests() -- running"<<std::endl;
#endif


    if (arguments.read("preload"))
    {
        osgDB::Registry::instance()->loadLibrary(osgDB::Registry::instance()->createLibraryNameForExtension("osg"));
        osgDB::Registry::instance()->loadLibrary(osgDB::Registry::instance()->createLibraryNameForExtension("rgb"));
        osgDB::Registry::instance()->loadLibrary(osgDB::Registry::instance()->createLibraryNameForExtension("jpeg"));
        osgDB::Registry::instance()->loadLibrary(osgDB::Registry::instance()->createLibraryNameForExtension("ive"));
    }

    if (arguments.read("serialize"))
    {
        osgDB::Registry::instance()->setReadFileCallback(new SerializerReadFileCallback());
    }

    osg::ref_ptr<RefBarrier> startBarrier = new RefBarrier(numThreads+1);
    osg::ref_ptr<RefBarrier> endBarrier = new RefBarrier(numThreads+1);

    typedef std::list< osg::ref_ptr<ReadThread> > ReadThreads;
    ReadThreads readThreads;

    for(int i=0; i<numThreads; ++i)
    {
        osg::ref_ptr<ReadThread> readThread = new ReadThread;

        readThread->setProcessorAffinity(numThreads % 4);

        readThread->setStartBarrier(startBarrier.get());
        readThread->setEndBarrier(endBarrier.get());

        readThread->addFileName("cessna.osgt");
        readThread->addFileName("glider.osgt");
        readThread->addFileName("town.ive");

        readThreads.push_back(readThread.get());

        readThread->start();

    }

    startBarrier->block();
    endBarrier->block();

#if VERBOSE
    osg::notify(osg::NOTICE)<<"runMultiThreadReadTests() -- completed."<<std::endl;
#endif
}
