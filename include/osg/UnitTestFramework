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

#ifndef OSG_UNITTESTFRAMEWORK
#define OSG_UNITTESTFRAMEWORK 1

#include <osg/Export>
#include <osg/Referenced>
#include <osg/ref_ptr>
#include <osg/Timer>
#include <osg/Notify>

#include <string>
#include <vector>
#include <list>
#include <fstream>

/**

\namespace osgUtx

The osgUtx is a unit test framework.
*/

namespace osgUtx{

class TestVisitor;


/**
Test, an abstract base class, is the Composite pattern's \em component
class for our graph of test cases, and defines the basic interface
for all Test components. It is a referent, and may be pointed
to by an osg::ref_ptr.
*/
class OSG_EXPORT Test: public osg::Referenced
{
    public:

    typedef TestVisitor Visitor;    // Test is redundant

    Test( const std::string& sName ) : _name( sName ) {}

    const std::string& name() const { return _name; }

    virtual bool accept( Visitor& ) = 0;

    protected:

        virtual ~Test() {}

    std::string _name;
};


/**
TestContext wraps up information which is passed to tests as they are run,
and may contain test-specific information or 'global' test objects, such
as an output stream for verbose output during the running of tests.

\todo Improve the output stream code by providing a filtering stream.
*/
class OSG_EXPORT TestContext
{
public:

    TestContext();

    bool shouldStop()    { return false; }
    bool isVerbose()    { return true; }

    enum TraceLevel{
        Off,        ///< All tracing turned off
        Results,    ///< Output results only
        Full        ///< Full test diagnostic output
    };

    void setTraceLevel(TraceLevel tl);
    TraceLevel getTraceLevel() const;

    std::ostream& tout(TraceLevel tl=Full) const;

private:

    TestContext(const TestContext&);
    TestContext operator=(const TestContext&);

#ifndef DOXYGEN_SHOULD_SKIP_THIS

    class OSG_EXPORT TraceStream{

    public:
        TraceStream(std::ostream& o=osg::notify(osg::NOTICE), TraceLevel tl=Results);
        ~TraceStream();

        void setTraceLevel(TraceLevel tl);
        TraceLevel getTraceLevel() const;

        std::ostream& stream(TraceLevel tl);

    private:

        TraceLevel    _traceLevel;
        std::ostream*    _outputStreamPtr;
        std::ofstream    _nullStream;
    };

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

    mutable TraceStream _tout;

};


class TestSuite;
class TestCase;

/**
Visits while maintaining the current hierarchical context. Also allows
the traversal to be short-circuited at any point during the visitation.
*/
class TestVisitor
{
    public:

    //..Should we enter this node and its children?
    virtual bool visitEnter( TestSuite* ) { return true; }

    //..Returns true to continue to next Leaf
    virtual bool visit( TestCase* ) = 0;

    //..Returns true to continue to next Composite
    virtual bool visitLeave( TestSuite* ) { return true; }
        
    protected:

    TestVisitor() {}
    TestVisitor( const TestVisitor& ) {}
    virtual ~TestVisitor()    {}
};

/**
TestCase, supplies the interface for a Composite pattern's
\em leaf class, though it is not a leaf in itself.
*/
class TestCase : public Test
{
    public:

    typedef TestContext Context; // Test in TestContext? is redundant

    TestCase( const std::string& sName ) : Test( sName ) {}

    virtual bool accept( Visitor& v ) { return v.visit( this ); }

    virtual void run( const Context& ) = 0;  // Subclass OSGUTX_EXPORT Responsibility

    protected:

        virtual ~TestCase() {}
};

/**
Base class catchable for the exceptions which may be thrown to
indicate problems during the run of a TestCase.
*/
class TestX
{
    public:

    TestX(const std::string& s):_what(s)    {}
    virtual ~TestX() {}

    const std::string& what() const { return _what; }

    private:
    std::string    _what;
};

/**
A TestFailureX indicates a failure in the tested component.
*/
class TestFailureX: public TestX
{
    public:
    TestFailureX(const std::string& s):TestX(s)    {}
};

/**
A TestErrorX indicates an error while testing a component,
which prevents the test from being run. It does not indicate
a problem with the component, but rather a problem during the
run which prevents the component from being tested.
*/
class TestErrorX: public TestX
{
    public:
    TestErrorX(const std::string& s):TestX(s)    {}
};

/**
TestCase_ is a class template for a leaf TestCase, which allows TestFixture
classes to be easily collected into the tree of tests, and have their public
test methods called. It is worth noting that, for a given TestCase_, an
instance of the test fixture class will be constructed prior to the
test method being called, and destructed afterwards. This prevents 'leakage'
of information from one test case to the next.
*/
template< typename FixtureT >
class TestCase_ : public TestCase
{
    typedef void (FixtureT::*TestMethodPtr)( const Context& );

    public:

    // Constructor adds the TestMethod pointer
    TestCase_( const std::string& sName, TestMethodPtr pTestMethod ) :
            TestCase( sName ),
            _pTestMethod( pTestMethod )
    {
    }

    // Create a TestFixture instance and invoke TestMethod?
    virtual void run( const Context& ctx )
    {
        ( FixtureT().*_pTestMethod )( ctx );
    }

    protected:

        virtual ~TestCase_() {}

    TestMethodPtr _pTestMethod;
};

/**
A TestSuite is the \em composite component of the Composite pattern,
and allows aggregation of Tests into hierarchies.
*/
class OSG_EXPORT TestSuite : public Test
{
    public:

    TestSuite( const std::string& name );

    /** Adds a Test to the suite. */
    void add( Test* pTest );

    /**
    @returns    The immediate child denoted by name, or 0 if not found.
    */
    Test* findChild(const std::string& name);

    virtual bool accept( Test::Visitor& v );

    protected:

        virtual ~TestSuite() {}

    typedef std::vector< osg::ref_ptr<Test> > Tests;
    Tests _tests;  // Collection of Suites and/or Cases
};

/**
TestGraph is a singleton providing central access to the tree of tests;
primarily, it provides access to the root suite.
*/
class OSG_EXPORT TestGraph
{

    public:

    static TestGraph& instance();

    /**
        @return a pointer to the root TestSuite.
    */
    TestSuite* root();

    /**
        A utility function for accessing an arbitrary suite by pathname, relative to
        the suite 'tsuite' (defaults to root if null), and with the option of creating
        the \em TestSuite designated by \em path, if it does not already exist.

        This method may return 0 if the suite either cannot be found (and createIfNecssary
        is 0), or the first component of \em path is not the same as the name of the
        TestSuite \em tsuite.

        This was written to aid the auto-registration of tests at specific points in
        the test tree, where the tests' AutoRegistrationAgents may be distributed across
        several files, and cannot be guaranteed to run in a given order. E.g. You cannot
        register a test "root.osg.MyTest" unless you know that the the suite "root.osg"
        already exists.
        

        @param path                    The name of the TestSuite to return.
        @param tsuite                The suite to 'start from'. Path is relative to this
                                    suite (defaults to root suite).
        @param createIfNecessary    Optionally create the TestSuite(s) denoted by path if
                                    they do not exist.
    */
    TestSuite* suite(const std::string& path, TestSuite* tsuite = 0,bool createIfNecessary = false);

    private:

    /**
        Does the same job as the version of suite listed above, but the path
        is passed in as components in a list, represented by the iterator parameters.
    */
    TestSuite* suite(
        std::list<std::string>::iterator it,
        std::list<std::string>::iterator end,
        TestSuite* tsuite, bool createIfNecessary);

    TestGraph();

    TestGraph(const TestGraph&);
    TestGraph& operator=(const TestGraph&);

    osg::ref_ptr<TestSuite>    root_;

};


/**
Maintains a string that when accessed in the "visit" member, returns the
current qualified TestSuite path.
*/
class OSG_EXPORT TestQualifier : public TestVisitor
{
    enum { SEPCHAR = '.' };

    public:

    // Entering a composite: Push its name on the Path
    virtual bool visitEnter( TestSuite* pSuite );

    // Leaving a composite: Pop its name from the Path
    virtual bool visitLeave( TestSuite* pSuite );

    // Provide read-only access to the current qualifier
    const std::string& currentPath() const;

    private:

    std::string _path;    // Current qualifier
};

/**
QualifiedTestPrinter prints to standard output a list of fully
qualified tests.
*/
class OSG_EXPORT QualifiedTestPrinter : public TestQualifier
{
public:


    virtual bool visit( TestCase* pTest );
};

/**
A TestRecord records the output of a given test case, i.e. its start/stop time,
its result, and a textual description of any problems.

\todo    Consider adding accessor methods if necessary, to get the details
        stored in the TestRecord.
*/
class OSG_EXPORT TestRecord
{
    public:

        void start();
        void stop();
        void log(const TestFailureX& e);
        void log(const TestErrorX& e);
        void log(const std::exception& e);
        void log(const std::string& s);

        // Default copy construction and assignment are OK

        // FIXME: Add accessors?

    private:

        // Onlye a TestReport can create a TestRecord
        friend class TestReport;
        TestRecord(const std::string& name);

        enum Result{
            Success,Failure,Error
        };

        friend std::ostream& operator<<(std::ostream& o,const TestRecord& tr);

        static osg::Timer    timer_;    // To time tests

        std::string        name_;
        osg::Timer_t    start_;
        osg::Timer_t    stop_;
        Result            result_;
        std::string        problem_;

};

/**
A TestReport represents the complete set of results (TestRecords) for a
given test run.

\todo    Add support for printing the test report in various formats:
        e.g. text, XML, CSV
*/
class OSG_EXPORT TestReport
{
public:

    TestRecord&    createRecord(const std::string& s){
        _records.push_back(TestRecord(s));
        return _records.back();
    }

private:
    std::list<TestRecord>    _records;

};







/**
A TestRunner is a visitor which will run specified tests as it traverses the
test graph.

\todo    Consider an accessor method to get at the TestReport if necessary.
*/
class OSG_EXPORT TestRunner : public TestQualifier
{
public:

    TestRunner( TestContext& ctx );

    /**
        Tests may be specified by partial names. E.g. specifiying "root"
        will run all tests below root, i.e. all tests.
        Specifiying    "root.osg" will run all tests below \em root.osg.
        Specifying "root.osg.de" will run all tests (and suites) below
        \em root.osg with names beginning with the \em de.
    */
    void specify( const std::string& sQualifiedName );

    bool visitEnter( TestSuite* pSuite );
    bool visit( TestCase* pTest );
    bool visitLeave( TestSuite* pSuite );


protected:

    void perform( TestCase* pTest );

private:

    TestReport                   _db;            // Results
    TestContext&                 _ctx;            // The Global Testing Context
    std::vector<std::string>    _tests;          // Specified Tests
};

}

/**
Starts a TestSuite singleton function
@see OSGUTX_ADD_TESTCASE, OSGUTX_END_TESTSUITE
*/
#define OSGUTX_BEGIN_TESTSUITE( tsuite ) \
    osgUtx::TestSuite* tsuite##_TestSuite() \
    { \
        static osg::ref_ptr<osgUtx::TestSuite> s_suite = 0; \
        if ( s_suite == 0 ) { \
            s_suite = new osgUtx::TestSuite( #tsuite );



/**
Adds a test case to a suite object being created in a TestSuite singleton function.
@see OSGUTX_BEGIN_TESTSUITE, OSGUTX_END_TESTSUITE
*/
#define OSGUTX_ADD_TESTCASE( tfixture, tmethod ) \
            s_suite->add( new osgUtx::TestCase_<tfixture>(  \
                                #tmethod, &tfixture::tmethod ) );

/**
Ends a TestSuite singleton function
@see OSGUTX_BEGIN_TESTSUITE, OSGUTX_ADD_TESTCASE
*/
#define OSGUTX_END_TESTSUITE \
        } \
        return s_suite.get(); \
    }

/** Define a TestSuite accessor */
#define OSGUTX_TESTSUITE( tsuite ) \
    tsuite##_TestSuite()


/**
Adds a suite to a suite - allows composition of test suites.
@see OSGUTX_BEGIN_TESTSUITE, OSGUTX_END_TESTSUITE
*/
#define OSGUTX_ADD_TESTSUITE( childSuite ) \
    s_suite->add( childSuite##_TestSuite() );


/** Autoregister a testsuite with the root suite at startup */
#define OSGUTX_AUTOREGISTER_TESTSUITE( tsuite ) \
    static osgUtx::TestSuiteAutoRegistrationAgent tsuite##_autoRegistrationObj__( tsuite##_TestSuite() );

/** Auto register a testsuite with at designated point in the suite graph at startup */
#define OSGUTX_AUTOREGISTER_TESTSUITE_AT( tsuite , path ) \
    static osgUtx::TestSuiteAutoRegistrationAgent tsuite##_autoRegistrationObj__( tsuite##_TestSuite(), #path );

namespace osgUtx{

/**
A helper struct to perform automatic registration at program startup; not for
direct use, it should be used via the following macros. (It's a secret agent :-)

@see OSGUTX_AUTOREGISTER_TESTSUITE, OSGUTX_AUTOREGISTER_TESTSUITE_AT
*/
struct TestSuiteAutoRegistrationAgent
{
    TestSuiteAutoRegistrationAgent(TestSuite* tsuite, const char* path = 0)
    {
        if( ! path ) path = "root";

        // Find the suite named in 'path', create it if necessary
        TestSuite *regSuite = osgUtx::TestGraph::instance().suite( path, 0, true );

        if(!regSuite){
            osg::notify(osg::WARN)<<"Warning, unable to register test suite named \""<<tsuite->name()<<"\" at "
                              <<path<<", falling back to root suite."<<std::endl;
            regSuite = osgUtx::TestGraph::instance().root();
        }

        regSuite->add(tsuite);
    }
};

}

/**
OSGUTX_TEST_F is a convenience macro, analogous to assert(), which will
throw an osgUtx::TestFailureX if \em expr evaluates to false; this should be
used to test for failure in a given test, as opposed to an actual error
in the test owing to some other reason than the tested code being faulty.

The exception will indicate the file and line number of the failed expression,
along with expression itself.
*/
#define OSGUTX_TEST_F( expr ) \
    if( !(expr) ){ \
        std::stringstream ss; \
        ss<< #expr <<" failure: "<<__FILE__<<", line "<<__LINE__<<std::ends; \
        throw osgUtx::TestFailureX(ss.str()); \
    }

/**
OSGUTX_TEST_E is a convenience macro, analogous to assert(), which will
throw an osgUtx::TestErrorX if \em expr evaluates to false; this should be
used to test for an error in a given test, as opposed to a failure
in the tested code.

The exception will indicate the file and line number of the failed expression,
along with expression itself.
*/
#define OSGUTX_TEST_E( expr ) \
    if( !(expr) ){ \
        std::stringstream ss; \
        ss<< #expr <<" error: "<<__FILE__<<", line "<<__LINE__<<std::ends; \
        throw osgUtx::TestErrorX(ss.str()); \
    }


#endif // OSG_UNITTESTFRAMEWORK
