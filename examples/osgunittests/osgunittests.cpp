#include <osg/UnitTestFramework>
#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getProgramName()+" [options]");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("qt","Display qualified tests.");
 

    bool printQualifiedTest = false; 
    while (arguments.read("qt")) printQualifiedTest = true; 

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        std::cout<<arguments.getApplicationUsage()->getCommandLineUsage()<<std::endl;
        arguments.getApplicationUsage()->write(std::cout,arguments.getApplicationUsage()->getCommandLineOptions());
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }


    if (printQualifiedTest) 
    {
         std::cout<<"*****   Qualified Tests  ******"<<std::endl;

         osgUtx::QualifiedTestPrinter printer;
         osgUtx::TestGraph::instance().root()->accept( printer );    
         std::cout<<endl;
    }

    std::cout<<"******   Running tests   ******"<<std::endl;

    // Global Data or Context
    osgUtx::TestContext ctx;
    osgUtx::TestRunner runner( ctx );
    runner.specify("root");

    osgUtx::TestGraph::instance().root()->accept( runner );

    return 0;
}
