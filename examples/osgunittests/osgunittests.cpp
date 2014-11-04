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

#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>

#include <osg/Vec3>
#include <osg/Matrix>
#include <osg/Polytope>
#include <osg/Timer>
#include <osg/io_utils>

#include <OpenThreads/Thread>

#include "UnitTestFramework.h"
#include "performance.h"
#include "MultiThreadRead.h"

#include <iostream>

extern void runFileNameUtilsTest(osg::ArgumentParser& arguments);

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

/// Exercise the Matrix.getRotate function.
/// Compare the output of:
///  q1 * q2
/// versus
///  (mat(q1)*mat(q2)*scale).getRotate()
/// for a range of rotations
void testGetQuatFromMatrix(const osg::Vec3d& scale)
{

    // Options

    // acceptable error range
    double eps=1e-6;

    // scale matrix
    // To not test with scale, use 1,1,1
    // Not sure if 0's or negative values are acceptable
    osg::Matrixd scalemat;
    scalemat.makeScale(scale);

    // range of rotations
#if 1
    // wide range
    double rol1start = 0.0;
    double rol1stop = 360.0;
    double rol1step = 20.0;

    double pit1start = 0.0;
    double pit1stop = 90.0;
    double pit1step = 20.0;

    double yaw1start = 0.0;
    double yaw1stop = 360.0;
    double yaw1step = 20.0;

    double rol2start = 0.0;
    double rol2stop = 360.0;
    double rol2step = 20.0;

    double pit2start = 0.0;
    double pit2stop = 90.0;
    double pit2step = 20.0;

    double yaw2start = 0.0;
    double yaw2stop = 360.0;
    double yaw2step = 20.0;
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

    std::cout << std::endl << "Starting testGetQuatFromMatrix, it can take a while ..." << std::endl;

    osg::Timer_t tstart, tstop;
    tstart = osg::Timer::instance()->tick();
    int count=0;
    for (double rol1 = rol1start; rol1 <= rol1stop; rol1 += rol1step) {
        for (double pit1 = pit1start; pit1 <= pit1stop; pit1 += pit1step) {
            for (double yaw1 = yaw1start; yaw1 <= yaw1stop; yaw1 += yaw1step) {
                for (double rol2 = rol2start; rol2 <= rol2stop; rol2 += rol2step) {
                    for (double pit2 = pit2start; pit2 <= pit2stop; pit2 += pit2step) {
                        for (double yaw2 = yaw2start; yaw2 <= yaw2stop; yaw2 += yaw2step)
                        {
                            count++;
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

                            // create an output quat by matrix multiplication and getRotate
                            osg::Matrixd out_mat;
                            out_mat = mat2 * mat1;
                            // add matrix scale for even more nastiness
                            out_mat = out_mat * scalemat;
                            osg::Quat out_quat2;
                            out_quat2 = out_mat.getRotate();

                            // If the quaternion W is <0, then we should reflect
                            // to get it into the positive W.
                            // Unfortunately, when W is very small (close to 0), the sign
                            // does not really make sense because of precision problems
                            // and the reflection might not work.
                            if(out_quat1.w()<0) out_quat1 = out_quat1 * -1.0;
                            if(out_quat2.w()<0) out_quat2 = out_quat2 * -1.0;

                            // if the output quat length is not one
                            // or if the components do not match,
                            // something is amiss

                            bool componentsOK = false;
                            if ( ((fabs(out_quat1.x()-out_quat2.x())) < eps) &&
                                 ((fabs(out_quat1.y()-out_quat2.y())) < eps) &&
                                 ((fabs(out_quat1.z()-out_quat2.z())) < eps) &&
                                 ((fabs(out_quat1.w()-out_quat2.w())) < eps) )
                                    {
                                componentsOK = true;
                            }
                            // We should also test for q = -q which is valid, so reflect
                            // one quat.
                            out_quat2 = out_quat2 * -1.0;
                            if ( ((fabs(out_quat1.x()-out_quat2.x())) < eps) &&
                                 ((fabs(out_quat1.y()-out_quat2.y())) < eps) &&
                                 ((fabs(out_quat1.z()-out_quat2.z())) < eps) &&
                                 ((fabs(out_quat1.w()-out_quat2.w())) < eps) )
                            {
                                componentsOK = true;
                            }

                            bool lengthOK = false;
                            if (fabs(1.0-out_quat2.length()) < eps)
                            {
                                lengthOK = true;
                            }

                            if (!lengthOK || !componentsOK)
                            {
                                std::cout << "testGetQuatFromMatrix problem at: \n"
                                      << " r1=" << rol1
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
    tstop = osg::Timer::instance()->tick();
    double duration = osg::Timer::instance()->delta_s(tstart,tstop);
    std::cout << "Time for testGetQuatFromMatrix with " << count << " iterations: " << duration << std::endl << std::endl;
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

void testQuat(const osg::Vec3d& quat_scale)
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

    // Test a range of rotations
    testGetQuatFromMatrix(quat_scale);

    // This is a specific test case for a matrix containing scale and rotation
    osg::Matrix matrix(0.5, 0.0, 0.0, 0.0,
                       0.0, 0.5, 0.0, 0.0,
                       0.0, 0.0, 0.5, 0.0,
                       1.0, 1.0, 1.0, 1.0);

    osg::Quat quat;
    matrix.get(quat);

    osg::notify(osg::NOTICE)<<"Matrix = "<<matrix<<"rotation = "<<quat<<", expected quat = (0,0,0,1)"<<std::endl;
}

void testDecompose()
{
    double angx = osg::DegreesToRadians(30.0);
    double angy = osg::DegreesToRadians(30.0);
    double angz = osg::DegreesToRadians(30.0);

    osg::Quat qx, qy, qz;
    qx.makeRotate(angx, osg::Vec3f (1.0f, 0.0f, 0.0f));
    qy.makeRotate(angy, osg::Vec3f (0.0f, 1.0f, 0.0f));
    qz.makeRotate(angz, osg::Vec3f (0.0f, 0.0f, 1.0f));

    osg::Quat rotation = qx * qy * qz;

    osg::Matrixf matf;
    matf.makeRotate(rotation);

    printf ("Test - Matrix::decompos(), input rotation  : %f %f %f %f\n", rotation._v[0], rotation._v[1], rotation._v[2], rotation._v[3]);

    osg::Vec3f transf;
    osg::Quat  rotf;
    osg::Vec3f sclf;
    osg::Quat  sof;
    matf.decompose (transf, rotf, sclf, sof);
    printf ("Matrixf::decomposef\n");
    printf ("Translation      : %f %f %f\n", transf.x(), transf.y(), transf.z());
    printf ("Rotation         : %f %f %f %f\n", rotf._v[0], rotf._v[1], rotf._v[2], rotf._v[3]);
    printf ("Scale            : %f %f %f\n", sclf.x(), sclf.y(), sclf.z());
    printf ("Scale Orientation: %f %f %f %f\n", sof._v[0], sof._v[1], sof._v[2], sof._v[3]);

    osg::Matrixd matd;
    matd.makeRotate(rotation);

    osg::Vec3f transd;
    osg::Quat  rotd;
    osg::Vec3f scld;
    osg::Quat  sod;
    matd.decompose (transd, rotd, scld, sod);
    printf ("Matrixd::decompose\n");
    printf ("Translation      : %f %f %f\n", transd.x(), transd.y(), transd.z());
    printf ("Rotation         : %f %f %f %f\n", rotd._v[0], rotd._v[1], rotd._v[2], rotd._v[3]);
    printf ("Scale            : %f %f %f\n", scld.x(), scld.y(), scld.z());
    printf ("Scale Orientation: %f %f %f %f\n", sod._v[0], sod._v[1], sod._v[2], sod._v[3]);

    osg::notify(osg::NOTICE)<<std::endl;
}

class MyThread : public OpenThreads::Thread {
public:
    void run(void) { }
};

class NotifyThread : public OpenThreads::Thread {
public:

    NotifyThread(osg::NotifySeverity level, const std::string& message):
    _done(false),
    _level(level),
    _message(message) {}

    ~NotifyThread()
    {
        _done = true;
        if (isRunning())
        {
            cancel();
            join();
        }
    }

    void run(void)
    {
        std::cout << "Entering thread ..." <<_message<< std::endl;

        unsigned int count=0;

        while(!_done)
        {
            ++count;
#if 1
            osg::notify(_level)<<_message<<this<<"\n";
#else
            osg::notify(_level)<<_message<<this<<std::endl;
#endif
        }

        std::cout << "Leaving thread ..." <<_message<< " count="<<count<<std::endl;
    }

    bool                  _done;
    osg::NotifySeverity   _level;
    std::string           _message;

};

void testThreadInitAndExit()
{
    std::cout<<"******   Running thread start and delete test   ****** "<<std::endl;

    {
        MyThread thread;
        thread.startThread();
    }

    // add a sleep to allow the thread start to fall over it its going to.
    OpenThreads::Thread::microSleep(500000);

    std::cout<<"pass    thread start and delete test"<<std::endl<<std::endl;


    std::cout<<"******   Running notify thread test   ****** "<<std::endl;

    {
        NotifyThread thread1(osg::INFO,"thread one:");
        NotifyThread thread2(osg::INFO,"thread two:");
        NotifyThread thread3(osg::INFO,"thread three:");
        NotifyThread thread4(osg::INFO,"thread four:");
        thread1.startThread();
        thread2.startThread();
        thread3.startThread();
        thread4.startThread();

        // add a sleep to allow the thread start to fall over it its going to.
        OpenThreads::Thread::microSleep(5000000);
    }

    std::cout<<"pass    noitfy thread test."<<std::endl<<std::endl;
}

void testPolytope()
{
    osg::Polytope pt;
    pt.setToBoundingBox(osg::BoundingBox(-1000, -1000, -1000, 1000, 1000, 1000));
    bool bContains = pt.contains(osg::Vec3(0, 0, 0));
    if (bContains)
    {
        std::cout<<"Polytope pt.contains(osg::Vec3(0, 0, 0)) has succeeded."<<std::endl;
    }
    else
    {
        std::cout<<"Polytope pt.contains(osg::Vec3(0, 0, 0)) has failed."<<std::endl;
    }

}


int main( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which runs units tests.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options]");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("qt","Display qualified tests.");
    arguments.getApplicationUsage()->addCommandLineOption("quat","Display extended quaternion tests.");
    arguments.getApplicationUsage()->addCommandLineOption("quat_scaled sx sy sz","Display extended quaternion tests of pre scaled matrix.");
    arguments.getApplicationUsage()->addCommandLineOption("sizeof","Display sizeof tests.");
    arguments.getApplicationUsage()->addCommandLineOption("matrix","Display qualified tests.");
    arguments.getApplicationUsage()->addCommandLineOption("performance","Display qualified tests.");
    arguments.getApplicationUsage()->addCommandLineOption("read-threads <numthreads>","Run multi-thread reading test.");


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

    bool printFileNameUtilsTests = false;
    while (arguments.read("filenames")) printFileNameUtilsTests = true;

    bool printQuatTest = false;
    while (arguments.read("quat")) printQuatTest = true;

    int numReadThreads = 0;
    while (arguments.read("read-threads", numReadThreads)) {}

    bool printPolytopeTest = false;
    while (arguments.read("polytope")) printPolytopeTest = true;

    bool doTestThreadInitAndExit = false;
    while (arguments.read("thread")) doTestThreadInitAndExit = true;

    osg::Vec3d quat_scale(1.0,1.0,1.0);
    while (arguments.read("quat_scaled", quat_scale.x(), quat_scale.y(), quat_scale.z() )) printQuatTest = true;

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

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    if (printQuatTest)
    {
        testQuat(quat_scale);
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

        testDecompose();

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

    if (numReadThreads>0)
    {
        runMultiThreadReadTests(numReadThreads, arguments);
        return 0;
    }


    if (printPolytopeTest)
    {
        testPolytope();
    }


    if (printQualifiedTest)
    {
         std::cout<<"*****   Qualified Tests  ******"<<std::endl;

         osgUtx::QualifiedTestPrinter printer;
         osgUtx::TestGraph::instance().root()->accept( printer );
         std::cout<<std::endl;
    }

    if (printFileNameUtilsTests)
    {
        runFileNameUtilsTest(arguments);
    }


    if (doTestThreadInitAndExit)
    {
        testThreadInitAndExit();
    }

    std::cout<<"******   Running tests   ******"<<std::endl;

    // Global Data or Context
    osgUtx::TestContext ctx;
    osgUtx::TestRunner runner( ctx );
    runner.specify("root");

    osgUtx::TestGraph::instance().root()->accept( runner );

    return 0;
}
