#include <osg/UnitTestFramework>
#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>

#include <osg/Vec3>
#include <osg/Matrix>
#include <osg/io_utils>

#include "performance.h"

#include <iostream>

void testFrustum(double left,double right,double bottom,double top,double zNear,double zFar)
{
    osg::Matrix f;
    f.makeFrustum(left,right,bottom,top,zNear,zFar);

    double c_left=0;
    double c_right=0;
    double c_top=0;
    double c_bottom=0;
    double c_zNear=0;
    double c_zFar=0;
    
    
    std::cout << "testFrustum"<<f.getFrustum(c_left,c_right,c_bottom,c_top,c_zNear,c_zFar)<<std::endl;
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

    double c_left=0;
    double c_right=0;
    double c_top=0;
    double c_bottom=0;
    double c_zNear=0;
    double c_zFar=0;

    std::cout << "testOrtho "<< f.getOrtho(c_left,c_right,c_bottom,c_top,c_zNear,c_zFar) << std::endl;
    std::cout << "  left = "<<left<<" compute "<<c_left<<std::endl;
    std::cout << "  right = "<<right<<" compute "<<c_right<<std::endl;

    std::cout << "  bottom = "<<bottom<<" compute "<<c_bottom<<std::endl;
    std::cout << "  top = "<<top<<" compute "<<c_top<<std::endl;

    std::cout << "  zNear = "<<zNear<<" compute "<<c_zNear<<std::endl;
    std::cout << "  zFar = "<<zFar<<" compute "<<c_zFar<<std::endl;
    
    std::cout << std::endl;
}

void testPerspective(double fovy,double aspect,double zNear,double zFar)
{
    osg::Matrix f;
    f.makePerspective(fovy,aspect,zNear,zFar);

    double c_fovy=0;
    double c_aspect=0;
    double c_zNear=0;
    double c_zFar=0;

    std::cout << "testPerspective "<< f.getPerspective(c_fovy,c_aspect,c_zNear,c_zFar) << std::endl;
    std::cout << "  fovy = "<<fovy<<" compute "<<c_fovy<<std::endl;
    std::cout << "  aspect = "<<aspect<<" compute "<<c_aspect<<std::endl;

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
    std::cout << "  center "<<center<< " compute "<<c_center<<std::endl;
    std::cout << "  up "<<up<< " compute "<<c_up<<std::endl;
    
    std::cout << std::endl;
    
}


void testMatrixInvert(const osg::Matrix& matrix)
{
    //Invert it twice using the two inversion functions and view the results
    osg::notify(osg::NOTICE)<<"testMatrixInvert("<<std::endl;
    osg::notify(osg::NOTICE)<<matrix<<std::endl;
    osg::notify(osg::NOTICE)<<")"<<std::endl;

    osg::Matrix invM1_0;
    invM1_0.invert(matrix);
    osg::notify(osg::NOTICE)<<"Matrix::invert"<<std::endl;
    osg::notify(osg::NOTICE)<<invM1_0<<std::endl;
    osg::Matrix default_result = matrix*invM1_0;
    osg::notify(osg::NOTICE)<<"matrix * invert="<<std::endl;
    osg::notify(osg::NOTICE)<<default_result<<std::endl;;

}

void sizeOfTest()
{
  std::cout<<"sizeof(bool)=="<<sizeof(bool)<<std::endl;
  std::cout<<"sizeof(char)=="<<sizeof(char)<<std::endl;
  std::cout<<"sizeof(short)=="<<sizeof(short)<<std::endl;
  std::cout<<"sizeof(short int)=="<<sizeof(short int)<<std::endl;
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

  std::cout<<"sizeof(std::istream::pos_type)=="<<sizeof(std::istream::pos_type)<<std::endl;
  std::cout<<"sizeof(std::istream::off_type)=="<<sizeof(std::istream::off_type)<<std::endl;
  std::cout<<"sizeof(OpenThreads::Mutex)=="<<sizeof(OpenThreads::Mutex)<<std::endl;

  std::cout<<"sizeof(std::string)=="<<sizeof(std::string)<<std::endl;

}

/// Exercises the Matrix.get(Quat&) function, printout is generated on problems only
void testGetQuatFromMatrix() {
    // acceptable error range
    double eps=1e-3;

#if 1
    // wide range
    double rol1start = 0.0;
    double rol1stop = 360.0;
    double rol1step = 30.0;

    double pit1start = 0.0;
    double pit1stop = 90.0;
    double pit1step = 30.0;

    double yaw1start = 0.0;
    double yaw1stop = 360.0;
    double yaw1step = 30.0;

    double rol2start = 0.0;
    double rol2stop = 360.0;
    double rol2step = 30.0;

    double pit2start = 0.0;
    double pit2stop = 90.0;
    double pit2step = 30.0;

    double yaw2start = 0.0;
    double yaw2stop = 360.0;
    double yaw2step = 30.0;
#else
    // focussed range
    double rol1start = 0.0;
    double rol1stop = 0.0;
    double rol1step = 0.1;

    double pit1start = 0.0;
    double pit1stop = 5.0;
    double pit1step = 5.0;

    double yaw1start = 89.0;
    double yaw1stop = 91.0;
    double yaw1step = 0.1;

    double rol2start = 0.0;
    double rol2stop = 0.0;
    double rol2step = 0.1;

    double pit2start = 0.0;
    double pit2stop = 0.0;
    double pit2step = 0.1;

    double yaw2start = 89.0;
    double yaw2stop = 91.0;
    double yaw2step = 0.1;
#endif
    
    for (double rol1 = rol1start; rol1 <= rol1stop; rol1 += rol1step) {
	for (double pit1 = pit1start; pit1 <= pit1stop; pit1 += pit1step) {
	    for (double yaw1 = yaw1start; yaw1 <= yaw1stop; yaw1 += yaw1step) {
		for (double rol2 = rol2start; rol2 <= rol2stop; rol2 += rol2step) {
		    for (double pit2 = pit2start; pit2 <= pit2stop; pit2 += pit2step) {
			for (double yaw2 = yaw2start; yaw2 <= yaw2stop; yaw2 += yaw2step) {
			    // create two quats based on the roll, pitch and yaw values
			    osg::Quat rot_quat1 =
				osg::Quat(osg::DegreesToRadians(rol1),osg::Vec3d(1,0,0),
					  osg::DegreesToRadians(pit1),osg::Vec3d(0,1,0),
					  osg::DegreesToRadians(yaw1),osg::Vec3d(0,0,1));
			    
			    osg::Quat rot_quat2 =
				osg::Quat(osg::DegreesToRadians(rol2),osg::Vec3d(1,0,0),
					  osg::DegreesToRadians(pit2),osg::Vec3d(0,1,0),
					  osg::DegreesToRadians(yaw2),osg::Vec3d(0,0,1));
			    
			    // create an output quat using quaternion math
			    osg::Quat out_quat1;
			    out_quat1 = rot_quat2 * rot_quat1;
			    
			    // create two matrices based on the input quats
			    osg::Matrixd mat1,mat2;
			    mat1.makeRotate(rot_quat1);
			    mat2.makeRotate(rot_quat2);
			    
			    // create an output quat by matrix multiplication and get
			    osg::Matrixd out_mat;
			    out_mat = mat2 * mat1;
			    osg::Quat out_quat2;
			    out_quat2 = out_mat.getRotate();
			    
			    // if the output quat length is not one 
			    // or if the component magnitudes do not match,
			    // something is amiss
			    if (fabs(1.0-out_quat2.length()) > eps ||
				(fabs(out_quat1.x())-fabs(out_quat2.x())) > eps ||
				(fabs(out_quat1.y())-fabs(out_quat2.y())) > eps ||
				(fabs(out_quat1.z())-fabs(out_quat2.z())) > eps ||
				(fabs(out_quat1.w())-fabs(out_quat2.w())) > eps) {
				std::cout << "problem at: r1=" << rol1
					  << " p1=" << pit1
					  << " y1=" << yaw1
					  << " r2=" << rol2
					  << " p2=" << pit2
					  << " y2=" << yaw2 << "\n";
				std::cout << "quats:        " << out_quat1 << " length: " << out_quat1.length() << "\n";
				std::cout << "mats and get: " << out_quat2 << " length: " << out_quat2.length() << "\n\n";
			    }
			}
		    }
		}
	    }
	}
    }
}

void testQuatRotate(const osg::Vec3d& from, const osg::Vec3d& to)
{
    osg::Quat q_nicolas;
    q_nicolas.makeRotate(from,to);
    
    osg::Quat q_original;
    q_original.makeRotate_original(from,to);
    
    std::cout<<"osg::Quat::makeRotate("<<from<<", "<<to<<")"<<std::endl;
    std::cout<<"  q_nicolas = "<<q_nicolas<<std::endl;
    std::cout<<"  q_original = "<<q_original<<std::endl;
    std::cout<<"  from * M4x4(q_nicolas) = "<<from * osg::Matrixd::rotate(q_nicolas)<<std::endl;
    std::cout<<"  from * M4x4(q_original) = "<<from * osg::Matrixd::rotate(q_original)<<std::endl;
}

void testQuat()
{
    osg::Quat q1;
    q1.makeRotate(osg::DegreesToRadians(30.0),0.0f,0.0f,1.0f);

    osg::Quat q2;
    q2.makeRotate(osg::DegreesToRadians(133.0),0.0f,1.0f,1.0f);

    osg::Quat q1_2 = q1*q2;
    osg::Quat q2_1 = q2*q1;

    osg::Matrix m1 = osg::Matrix::rotate(q1);
    osg::Matrix m2 = osg::Matrix::rotate(q2);
    
    osg::Matrix m1_2 = m1*m2;
    osg::Matrix m2_1 = m2*m1;
    
    osg::Quat qm1_2;
    qm1_2.set(m1_2);
    
    osg::Quat qm2_1;
    qm2_1.set(m2_1);
    
    std::cout<<"q1*q2 = "<<q1_2<<std::endl;
    std::cout<<"q2*q1 = "<<q2_1<<std::endl;
    std::cout<<"m1*m2 = "<<qm1_2<<std::endl;
    std::cout<<"m2*m1 = "<<qm2_1<<std::endl;


    testQuatRotate(osg::Vec3d(1.0,0.0,0.0),osg::Vec3d(0.0,1.0,0.0));
    testQuatRotate(osg::Vec3d(0.0,1.0,0.0),osg::Vec3d(1.0,0.0,0.0));
    testQuatRotate(osg::Vec3d(0.0,0.0,1.0),osg::Vec3d(0.0,1.0,0.0));
    testQuatRotate(osg::Vec3d(1.0,1.0,1.0),osg::Vec3d(1.0,0.0,0.0));
    testQuatRotate(osg::Vec3d(1.0,0.0,0.0),osg::Vec3d(1.0,0.0,0.0));
    testQuatRotate(osg::Vec3d(1.0,0.0,0.0),osg::Vec3d(-1.0,0.0,0.0));
    testQuatRotate(osg::Vec3d(-1.0,0.0,0.0),osg::Vec3d(1.0,0.0,0.0));
    testQuatRotate(osg::Vec3d(0.0,1.0,0.0),osg::Vec3d(0.0,-1.0,0.0));
    testQuatRotate(osg::Vec3d(0.0,-1.0,0.0),osg::Vec3d(0.0,1.0,0.0));
    testQuatRotate(osg::Vec3d(0.0,0.0,1.0),osg::Vec3d(0.0,0.0,-1.0));
    testQuatRotate(osg::Vec3d(0.0,0.0,-1.0),osg::Vec3d(0.0,0.0,1.0));

    testGetQuatFromMatrix();
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
    arguments.getApplicationUsage()->addCommandLineOption("performance","Display qualified tests.");
 

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

    bool printQuatTest = false; 
    while (arguments.read("quat")) printQuatTest = true; 

    bool performanceTest = false; 
    while (arguments.read("p") || arguments.read("performance")) performanceTest = true; 

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
    
    if (printQuatTest)
    {
        testQuat();
    }


    if (printMatrixTest)
    {
        std::cout<<"******   Running matrix tests   ******"<<std::endl;

        testFrustum(-1,1,-1,1,1,1000);
        testFrustum(0,1,1,2,2.5,100000);

        testOrtho(0,1,1,2,2.1,1000);
        testOrtho(-1,10,1,20,2.5,100000);

        testPerspective(20,1,1,1000);
        testPerspective(90,2,1,1000);

        testLookAt(osg::Vec3(10.0,4.0,2.0),osg::Vec3(10.0,4.0,2.0)+osg::Vec3(0.0,1.0,0.0),osg::Vec3(0.0,0.0,1.0));
        testLookAt(osg::Vec3(10.0,4.0,2.0),osg::Vec3(10.0,4.0,2.0)+osg::Vec3(1.0,1.0,0.0),osg::Vec3(0.0,0.0,1.0));
        
        testMatrixInvert(osg::Matrix(0.999848,  -0.002700,  0.017242, -0.1715,
                                     0,         0.987960,   0.154710,  0.207295,
                                     -0.017452, -0.154687,  0.987809, -0.98239,
                                     0,         0,          0,         1));

        testMatrixInvert(osg::Matrix(0.999848,  -0.002700,  0.017242,   0.0,
                                     0.0,        0.987960,   0.154710,   0.0,
                                     -0.017452, -0.154687,  0.987809,   0.0,
                                     -0.1715,    0.207295,  -0.98239,   1.0));

    }
    
    if (printSizeOfTest)
    {
        std::cout<<"**** sizeof() tests  ******"<<std::endl;
        
        sizeOfTest();

        std::cout<<std::endl;
    }


    if (performanceTest)
    {
        std::cout<<"**** performance tests  ******"<<std::endl;
        
        runPerformanceTests();
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
