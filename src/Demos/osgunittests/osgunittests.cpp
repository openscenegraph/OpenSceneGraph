#include <osg/UnitTestFramework>


int main( int /*argc*/, char** /*argv*/ )
{
//     std::cout<<"*****   Qualified Tests  ******"<<std::endl;
// 
//     osgUtx::QualifiedTestPrinter printer;
//     osgUtx::TestGraph::instance().root()->accept( printer );    
//     std::cout<<endl;


    std::cout<<"******   Running tests   ******"<<std::endl;

    // Global Data or Context
    osgUtx::TestContext ctx;
    osgUtx::TestRunner runner( ctx );
    runner.specify("root");

    osgUtx::TestGraph::instance().root()->accept( runner );

    return 0;
}
