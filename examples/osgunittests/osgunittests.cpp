#include <osg/UnitTestFramework>
#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>

#include <osg/Matrix>


void testFrustum(double left,double right,double bottom,double top,double zNear,double zFar)
{
    osg::Matrix f;
    f.makeFrustum(left,right,bottom,top,zNear,zFar);

    
    double c_zNear = f(3,2) / (f(2,2)-1.0f);
    double c_zFar = f(3,2) / (1.0f+f(2,2));
    
    double c_left = c_zNear * (f(2,0)-1.0f) / f(0,0);
    double c_right = c_zNear * (1.0f+f(2,0)) / f(0,0);

    double c_top = c_zNear * (1+f(2,1)) / f(1,1);
    double c_bottom = c_zNear * (f(2,1)-1.0f) / f(1,1);
    
    f.getFrustum(c_left,c_right,c_bottom,c_top,c_zNear,c_zFar);

    std::cout << "testFrustum"<<std::endl;
    std::cout << "  left = "<<left<<" compute "<<c_left<<std::endl;
    std::cout << "  right = "<<right<<" compute "<<c_right<<std::endl;

    std::cout << "  bottom = "<<bottom<<" compute "<<c_bottom<<std::endl;
    std::cout << "  top = "<<top<<" compute "<<c_top<<std::endl;

    std::cout << "  zNear = "<<zNear<<" compute "<<c_zNear<<std::endl;
    std::cout << "  zFar = "<<zFar<<" compute "<<c_zFar<<std::endl;
    
    std::cout << std::endl;
}

void testOrtho(double left,double right,double bottom,double top,double zNear,double zFar)
{
    osg::Matrix f;
    f.makeOrtho(left,right,bottom,top,zNear,zFar);

    double c_zNear = (f(3,2)+1.0f) / f(2,2);
    double c_zFar = (f(3,2)-1.0f) / f(2,2);
    
    double c_left = -(1.0f+f(3,0)) / f(0,0);
    double c_right = (1.0f-f(3,0)) / f(0,0);

    double c_bottom = -(1.0f+f(3,1)) / f(1,1);
    double c_top = (1.0f-f(3,1)) / f(1,1);
    
    f.getOrtho(c_left,c_right,c_bottom,c_top,c_zNear,c_zFar);


    std::cout << "testOrtho"<<std::endl;
    std::cout << "  left = "<<left<<" compute "<<c_left<<std::endl;
    std::cout << "  right = "<<right<<" compute "<<c_right<<std::endl;

    std::cout << "  bottom = "<<bottom<<" compute "<<c_bottom<<std::endl;
    std::cout << "  top = "<<top<<" compute "<<c_top<<std::endl;

    std::cout << "  zNear = "<<zNear<<" compute "<<c_zNear<<std::endl;
    std::cout << "  zFar = "<<zFar<<" compute "<<c_zFar<<std::endl;
    
    std::cout << std::endl;
}

void testLookAt(const osg::Vec3& eye,const osg::Vec3& center,const osg::Vec3& up)
{
    osg::Matrix mv;
    mv.makeLookAt(eye,center,up);
    
    osg::Vec3 c_eye,c_center,c_up;
    mv.getLookAt(c_eye,c_center,c_up);
    
    std::cout << "testLookAt"<<std::endl;
    std::cout << "  eye "<<eye<< " compute "<<c_eye<<std::endl;
    std::cout << "  eye "<<center<< " compute "<<c_center<<std::endl;
    std::cout << "  eye "<<up<< " compute "<<c_up<<std::endl;
    
    std::cout << std::endl;
    
}

void sizeOfTest()
{
  std::cout<<"sizeof(bool)=="<<sizeof(bool)<<std::endl;
  std::cout<<"sizeof(char)=="<<sizeof(char)<<std::endl;
  std::cout<<"sizeof(short)=="<<sizeof(short)<<std::endl;
  std::cout<<"sizeof(int)=="<<sizeof(int)<<std::endl;
  std::cout<<"sizeof(long)=="<<sizeof(long)<<std::endl;
  std::cout<<"sizeof(long int)=="<<sizeof(long int)<<std::endl;

#if defined(_MSC_VER)
  // long long isn't supported on VS6.0...
  std::cout<<"sizeof(__int64)=="<<sizeof(__int64)<<std::endl;
#else
  std::cout<<"sizeof(long long)=="<<sizeof(long long)<<std::endl;
#endif
  std::cout<<"sizeof(float)=="<<sizeof(float)<<std::endl;
  std::cout<<"sizeof(double)=="<<sizeof(double)<<std::endl;
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which runs units tests.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options]");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("qt","Display qualified tests.");
    arguments.getApplicationUsage()->addCommandLineOption("sizeof","Display sizeof tests.");
    arguments.getApplicationUsage()->addCommandLineOption("matrix","Display qualified tests.");
 

    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    bool printQualifiedTest = false; 
    while (arguments.read("qt")) printQualifiedTest = true; 

    bool printMatrixTest = false; 
    while (arguments.read("matrix")) printMatrixTest = true; 

    bool printSizeOfTest = false; 
    while (arguments.read("sizeof")) printSizeOfTest = true; 

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
    

    if (printMatrixTest)
    {
        std::cout<<"******   Running matrix tests   ******"<<std::endl;

        testFrustum(-1,1,-1,1,1,1000);
        testFrustum(0,1,1,2,2.5,100000);

        testOrtho(0,1,1,2,2.1,1000);
        testOrtho(-1,10,1,20,2.5,100000);

        testLookAt(osg::Vec3(10.0,4.0,2.0),osg::Vec3(10.0,4.0,2.0)+osg::Vec3(0.0,1.0,0.0),osg::Vec3(0.0,0.0,1.0));
        testLookAt(osg::Vec3(10.0,4.0,2.0),osg::Vec3(10.0,4.0,2.0)+osg::Vec3(1.0,1.0,0.0),osg::Vec3(0.0,0.0,1.0));

    }
    
    if (printSizeOfTest)
    {
        std::cout<<"**** sizeof() tests  ******"<<std::endl;
        
        sizeOfTest();

        std::cout<<std::endl;
    }


    if (printQualifiedTest) 
    {
         std::cout<<"*****   Qualified Tests  ******"<<std::endl;

         osgUtx::QualifiedTestPrinter printer;
         osgUtx::TestGraph::instance().root()->accept( printer );    
         std::cout<<std::endl;
    }

    std::cout<<"******   Running tests   ******"<<std::endl;

    // Global Data or Context
    osgUtx::TestContext ctx;
    osgUtx::TestRunner runner( ctx );
    runner.specify("root");

    osgUtx::TestGraph::instance().root()->accept( runner );

    return 0;
}
