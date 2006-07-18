/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
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
#include <osg/UnitTestFramework>

#include <algorithm>

namespace osgUtx
{

//////////////////////////////////////////////////////////////////////////////

TestContext::TestContext()
{
}

void TestContext::setTraceLevel(TraceLevel tl)
{
    _tout.setTraceLevel(tl);
}

TestContext::TraceLevel TestContext::getTraceLevel() const
{
    return _tout.getTraceLevel();
}

std::ostream& TestContext::tout(TraceLevel tl) const
{
    return _tout.stream(tl);
}

//////////////////////////////////////////////////////////////////////////////


TestContext::TraceStream::TraceStream(std::ostream& o, TraceLevel tl):
    _traceLevel(tl),
    _outputStreamPtr(&o),
#if defined(WIN32) && !(defined(__CYGWIN__) || defined(__MINGW32__))
    _nullStream("nul")
#else
    _nullStream("/dev/null")
#endif
{
}

TestContext::TraceStream::~TraceStream()
{
    _nullStream.close();
}

void TestContext::TraceStream::setTraceLevel(TraceLevel tl)
{
    _traceLevel = tl;
}

TestContext::TraceLevel TestContext::TraceStream::getTraceLevel() const
{
    return _traceLevel;
}

std::ostream& TestContext::TraceStream::stream(TestContext::TraceLevel tl)
{
    if(_traceLevel >= tl){
        return *_outputStreamPtr;
    }
    return _nullStream;
}

//////////////////////////////////////////////////////////////////////////////

TestGraph& TestGraph::instance()
{
    static TestGraph instance_;
    return instance_;
}

TestSuite* TestGraph::root()
{
    return root_.get();
}

TestSuite* TestGraph::suite(const std::string& path, TestSuite* tsuite, bool createIfNecessary)
{
    using namespace std;

    list<string> pathComponents;

    std::string::const_iterator it1 = path.begin();
    std::string::const_iterator it2 = it1;

    // Dissect the path into it's constituent components
    do{

        while( *it2 != '.' && it2 != path.end() ) ++it2;

        // Consider a check for "" empty strings?
        pathComponents.push_back( std::string(it1,it2) );

        if( it2 != path.end()) ++it2;

        it1 = it2;

    }while( it2 != path.end());

    return suite(pathComponents.begin(), pathComponents.end(),
            tsuite, createIfNecessary);

}

TestSuite* TestGraph::suite(
        std::list<std::string>::iterator it,
        std::list<std::string>::iterator end,
        TestSuite* tsuite, bool createIfNecessary)
{
    using namespace std;

    if( ! tsuite) tsuite = root();

    // Make sure these tie up
    if(*it != tsuite->name()) return 0;

    ++it;
    if(it == end) return tsuite;

    Test* child = tsuite->findChild(*it);

    if(child){

        // We've found a child with the right name. But is it a 
        // test suite?

        if(TestSuite* childSuite = dynamic_cast<TestSuite*>(child)){
            return suite(it, end, childSuite, createIfNecessary);
        }

        // We could return 0 here, to indicate that someone is
        // trying to add a TestSuite named 'xxx' to a suite with a
        // Test already named 'xxx'. But we don't enforce uniqueness
        // the other way round, so we don't do it this way round
        // either. Carry on as normal, and create a TestSuite of
        // the same name if createIfNecessary is true.

    }

    if(createIfNecessary){

        TestSuite* childSuite = new TestSuite(*it);
        tsuite->add(childSuite);
        return suite(it, end, childSuite, createIfNecessary);
    }

    return 0;
}

TestGraph::TestGraph(): root_(new TestSuite("root"))
{
}


//////////////////////////////////////////////////////////////////////////////

bool TestQualifier::visitEnter( TestSuite* pSuite )
{
    _path.append( pSuite->name() );
    _path += SEPCHAR; 
    return true;
}

// Leaving a composite: Pop its name from the Path
bool TestQualifier::visitLeave( TestSuite* pSuite )
{
//    assert( _path.rfind( pSuite->name() + static_cast<const char>(SEPCHAR))
//                == _path.size() - pSuite->name().size()  - 1);

    _path.erase( _path.size() - pSuite->name().size() -1 );
    return true;
}

// Provide read-only access to the current qualifier
const std::string& TestQualifier::currentPath() const
{
    return _path;
}

//////////////////////////////////////////////////////////////////////////////

osg::Timer TestRecord::timer_;

void TestRecord::start()
{
    start_ = timer_.tick();
}

void TestRecord::stop()
{
    stop_ = timer_.tick();
}

void TestRecord::log(const TestFailureX& e)
{
    stop();
    result_ = Failure;
    problem_ = e.what();
}

void TestRecord::log(const TestErrorX& e)
{
    stop();
    result_ = Error;
    problem_ = e.what();
}

void TestRecord::log(const std::exception& e)
{
    stop();
    result_ = Error;
    problem_ = e.what();
}

void TestRecord::log(const std::string& s)
{
    stop();
    result_ = Error;
    problem_ = s;
}

TestRecord::TestRecord(const std::string& name):
    name_(name),
    start_(0),
    stop_(0),
    result_(Success),
    problem_("No problem")
{
}

std::ostream& operator<<(std::ostream& o,const TestRecord& tr)
{
    if(tr.result_ == TestRecord::Success)         o<<"pass";
    else if(tr.result_ == TestRecord::Failure)    o<<"fail";
    else                                          o<<"error";

    o<<"\t"<<tr.name_;


    //o<<tr.start_<<'\t'<<tr.stop_<<'\t'<<TestRecord::timer_.delta_s(tr.start_,tr.stop_);

    // Just print out the duration
    o<<'\t'<<TestRecord::timer_.delta_s(tr.start_,tr.stop_)<<'s';

    if(tr.result_ != TestRecord::Success){
        o<<'\t'<<tr.problem_;
    }

    return o;
}

//////////////////////////////////////////////////////////////////////////////

TestRunner::TestRunner( TestContext& ctx ) : _ctx( ctx )
{
}

void TestRunner::specify( const std::string& sQualifiedName )
{
    _tests.push_back( sQualifiedName );
}

bool TestRunner::visitEnter( TestSuite* pSuite )
{
    TestQualifier::visitEnter( pSuite );
    return !_ctx.shouldStop();
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

namespace osgUtx{

struct isSpecified{

    const std::string& pTestName_;

    isSpecified(const std::string& s): pTestName_(s) {}

    bool operator()(const std::string& specifiedTest){
        return pTestName_.find(specifiedTest) == 0;
    }
};

}

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

bool TestRunner::visit( TestCase* pTest )
{
    if ( std::find_if(_tests.begin(),_tests.end(),
                      osgUtx::isSpecified(currentPath() + pTest->name() ) ) != _tests.end()) perform( pTest );

    return !_ctx.shouldStop();
}

bool TestRunner::visitLeave( TestSuite* pSuite )
{
    TestQualifier::visitLeave( pSuite );
    return !_ctx.shouldStop();
}

void TestRunner::perform( TestCase* pTest )
{
    TestRecord& record = _db.createRecord( currentPath() + pTest->name() );

    try
    {
        record.start();
        pTest->run( _ctx );
        record.stop();
    }

    catch ( const TestFailureX& e )
    {
        record.log( e );
    }
    catch ( const TestErrorX& e )
    {
        record.log( e );
    }
    catch ( const std::exception& e )
    {
        record.log( e );
    }
    catch ( ... )
    {
        record.log( std::string("Unknown") );
    }


    _ctx.tout(TestContext::Results) << record << std::endl;
}

//////////////////////////////////////////////////////////////////////////////

TestSuite::TestSuite( const std::string& name ) : Test( name )
{
}

void TestSuite::add( Test* pTest )
{
    _tests.push_back( pTest );
}

Test* TestSuite::findChild(const std::string& name)
{
    for(Tests::iterator it = _tests.begin();
        it != _tests.end();
        ++it){

        if ((*it)->name() == name) return (*it).get();
    }

    return 0;
}

bool TestSuite::accept( Test::Visitor& v )
{
    if ( v.visitEnter( this ) )
    {
        Tests::iterator end = _tests.end();
        for ( Tests::iterator at = _tests.begin(); at != end; ++at )
            if ( !(*at)->accept( v ) )
                break;
    }

    return v.visitLeave( this );   // continue with siblings?
}

//////////////////////////////////////////////////////////////////////////////

bool QualifiedTestPrinter::visit( TestCase* pTest )
{
    osg::notify(osg::NOTICE) << currentPath() + pTest->name() << std::endl;
    return true;
}

//////////////////////////////////////////////////////////////////////////////


};
