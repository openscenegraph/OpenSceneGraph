#include <osg/UnitTestFramework>


int main( int /*argc*/, char** /*argv*/ )
{
//     cout<<"*****   Qualified Tests  ******"<<endl;
// 
//     osgUtx::QualifiedTestPrinter printer;
//     osgUtx::TestGraph::instance().root()->accept( printer );    
//     

    cout<<endl;
    cout<<"******   Running tests   ******"<<endl;

    // Global Data or Context
    osgUtx::TestContext ctx;
    osgUtx::TestRunner runner( ctx );
    runner.specify("root");

    osgUtx::TestGraph::instance().root()->accept( runner );

    return 0;
}
