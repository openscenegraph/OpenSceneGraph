#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include "osg/ConvexPlanarOccluder"
#include "osg/Notify"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool ConvexPlanarOccluder_readLocalData(Object& obj, Input& fr);
bool ConvexPlanarOccluder_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(ConvexPlanarOccluder)
(
    new osg::ConvexPlanarOccluder,
    "ConvexPlanarOccluder",
    "Object ConvexPlanarOccluder",
    &ConvexPlanarOccluder_readLocalData,
    &ConvexPlanarOccluder_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool ConvexPlanarOccluder_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    ConvexPlanarOccluder& cpo = static_cast<ConvexPlanarOccluder&>(obj);


    bool matchFirst;
    if ((matchFirst=fr.matchSequence("Occluder {")) || fr.matchSequence("Occluder %i {"))
    {

        ConvexPlanarPolygon& cpp = cpo.getOccluder();
        ConvexPlanarPolygon::VertexList& vertexList = cpp.getVertexList();

        // set up coordinates.
        int entry = fr[0].getNoNestedBrackets();

        if (matchFirst)
        {
            fr += 2;
        }
        else
        {
            int capacity;
            fr[1].getInt(capacity);

            vertexList.reserve(capacity);

            fr += 3;
        }

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            Vec3 v;
            if (fr[0].getFloat(v.x()) && fr[1].getFloat(v.y()) && fr[2].getFloat(v.z()))
            {
                fr += 3;
                vertexList.push_back(v);
            }
            else
            {
                ++fr;
            }
        }
        iteratorAdvanced = true;
        ++fr;

    }

    ConvexPlanarOccluder::HoleList& holeList = cpo.getHoleList();

    while ((matchFirst=fr.matchSequence("Hole {")) || fr.matchSequence("Hole %i {"))
    {
        holeList.push_back(ConvexPlanarPolygon());

        ConvexPlanarPolygon& cpp = holeList.back();
        ConvexPlanarPolygon::VertexList& vertexList = cpp.getVertexList();

        // set up coordinates.
        int entry = fr[0].getNoNestedBrackets();

        if (matchFirst)
        {
            fr += 2;
        }
        else
        {
            int capacity;
            fr[1].getInt(capacity);

            vertexList.reserve(capacity);

            fr += 3;
        }

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            Vec3 v;
            if (fr[0].getFloat(v.x()) && fr[1].getFloat(v.y()) && fr[2].getFloat(v.z()))
            {
                fr += 3;
                vertexList.push_back(v);
            }
            else
            {
                ++fr;
            }
        }
        iteratorAdvanced = true;
        ++fr;

    }

    return iteratorAdvanced;
}


bool ConvexPlanarOccluder_writeLocalData(const Object& obj, Output& fw)
{
    const ConvexPlanarOccluder& cpo = static_cast<const ConvexPlanarOccluder&>(obj);

    // write out the occluder polygon.
    {
        const ConvexPlanarPolygon::VertexList& vertexList = cpo.getOccluder().getVertexList();

        fw.indent() << "Occluder " << vertexList.size()<< "{"<< std::endl;
        fw.moveIn();

        for(ConvexPlanarPolygon::VertexList::const_iterator itr=vertexList.begin();
            itr!=vertexList.end();
            ++itr)
        {
            fw.indent() << (*itr)[0] << ' ' << (*itr)[1] << ' ' << (*itr)[2] << std::endl;
        }

        fw.moveOut();
        fw.indent()<<"}"<< std::endl;
    }

    // write out any holes.
    const ConvexPlanarOccluder::HoleList& holeList = cpo.getHoleList();
    for(ConvexPlanarOccluder::HoleList::const_iterator holeItr=holeList.begin();
        holeItr!=holeList.end();
        ++holeItr)
    {
        const ConvexPlanarPolygon::VertexList& vertexList = holeItr->getVertexList();

        fw.indent() << "Hole " << vertexList.size() << "{"<< std::endl;
        fw.moveIn();

        for(ConvexPlanarPolygon::VertexList::const_iterator itr=vertexList.begin();
            itr!=vertexList.end();
            ++itr)
        {
            fw.indent() << (*itr)[0] << ' ' << (*itr)[1] << ' ' << (*itr)[2] << std::endl;
        }

        fw.moveOut();
        fw.indent()<<"}"<< std::endl;
    }


    return true;
}
