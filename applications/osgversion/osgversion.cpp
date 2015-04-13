// The majority of the application is dedicated to building the
// current contributors list by parsing the ChangeLog, it just takes
// one line in the main itself to report the version number.

#include <set>
#include <vector>
#include <iostream>

#include <OpenThreads/Version>

#include <osg/Notify>
#include <osg/Version>
#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>

#include <osg/Matrix>
#include <osg/Plane>
#include <osg/BoundingBox>
#include <osg/BoundingSphere>

#ifdef BUILD_CONTRIBUTORS
extern void printContributors(const std::string& changeLog, bool printNumEntries);
#endif

using namespace std;

int main( int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName() + " [options]");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help",                   "Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--version-number",               "Print out version number only");
    arguments.getApplicationUsage()->addCommandLineOption("--major-number",                 "Print out major version number only");
    arguments.getApplicationUsage()->addCommandLineOption("--minor-number",                 "Print out minor version number only");
    arguments.getApplicationUsage()->addCommandLineOption("--patch-number",                 "Print out patch version number only");
    arguments.getApplicationUsage()->addCommandLineOption("--so-number ",                   "Print out shared object version number only");
    arguments.getApplicationUsage()->addCommandLineOption("--openthreads-version-number",   "Print out version number for OpenThreads only");
    arguments.getApplicationUsage()->addCommandLineOption("--openthreads-soversion-number", "Print out shared object version number for OpenThreads only");
    arguments.getApplicationUsage()->addCommandLineOption("Matrix::value_type",             "Print the value of Matrix::value_type");
    arguments.getApplicationUsage()->addCommandLineOption("Plane::value_type",              "Print the value of Plane::value_type");
    arguments.getApplicationUsage()->addCommandLineOption("BoundingSphere::value_type",     "Print the value of BoundingSphere::value_type");
    arguments.getApplicationUsage()->addCommandLineOption("BoundingBox::value_type",        "Print the value of BoundingBox::value_type");

#ifdef BUILD_CONTRIBUTORS
    arguments.getApplicationUsage()->addCommandLineOption("-r <file> or --read <file>",     "Read the ChangeLog to generate an estimated contributors list.");
    arguments.getApplicationUsage()->addCommandLineOption("--entries",                      "Print out number of entries into the ChangeLog file for each contributor.");
#endif

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        cout << arguments.getApplicationUsage()->getCommandLineUsage() << endl;
        arguments.getApplicationUsage()->write(cout, arguments.getApplicationUsage()->getCommandLineOptions());
        return 1;
    }

    if (arguments.read("--version-number"))
    {
        cout << osgGetVersion() << endl;
        return 0;
    }

    if (arguments.read("--major-number"))
    {
        cout << OPENSCENEGRAPH_MAJOR_VERSION << endl;
        return 0;
    }

    if (arguments.read("--minor-number"))
    {
        cout << OPENSCENEGRAPH_MINOR_VERSION << endl;
        return 0;
    }

    if (arguments.read("--patch-number"))
    {
        cout << OPENSCENEGRAPH_PATCH_VERSION << endl;
        return 0;
    }

    if (arguments.read("--soversion-number") || arguments.read("--so-number") )
    {
        cout << osgGetSOVersion() << endl;
        return 0;
    }

    if (arguments.read("--openthreads-version-number"))
    {
        cout << OpenThreadsGetVersion() << endl;
        return 0;
    }

    if (arguments.read("--openthreads-major-number"))
    {
        cout << OPENTHREADS_MAJOR_VERSION << endl;
        return 0;
    }

    if (arguments.read("--openthreads-minor-number"))
    {
        cout << OPENTHREADS_MINOR_VERSION << endl;
        return 0;
    }

    if (arguments.read("--openthreads-patch-number"))
    {
        cout << OPENTHREADS_PATCH_VERSION << endl;
        return 0;
    }

    if (arguments.read("--openthreads-soversion-number"))
    {
        cout << OpenThreadsGetSOVersion() << endl;
        return 0;
    }


    if (arguments.read("Matrix::value_type"))
    {
        cout << ((sizeof(osg::Matrix::value_type) == 4) ? "float" : "double") << endl;
        return 0;
    }

    if (arguments.read("Plane::value_type"))
    {
        cout << ((sizeof(osg::Plane::value_type) == 4) ? "float" : "double") << endl;
        return 0;
    }

    if (arguments.read("BoundingSphere::value_type"))
    {
        cout << ((sizeof(osg::BoundingSphere::value_type) == 4) ? "float" : "double") << endl;
        return 0;
    }

    if (arguments.read("BoundingBox::value_type"))
    {
        cout << ((sizeof(osg::BoundingBox::value_type) == 4) ? "float" : "double") << endl;
        return 0;
    }

    cout << osgGetLibraryName() << " " << osgGetVersion() << endl << endl;

#ifdef BUILD_CONTRIBUTORS
    string changeLog;
    while (arguments.read("-r",     changeLog) ||
           arguments.read("--read", changeLog))
    {
        printContributors(changeLog, arguments.read("--entries"));
    }
#endif

    return 0;
}
