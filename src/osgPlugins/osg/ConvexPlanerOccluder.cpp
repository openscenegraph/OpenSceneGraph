#if defined(_MSC_VER)
	#pragma warning( disable : 4786 )
#endif

#include "osg/ConvexPlanerOccluder"
#include "osg/Types"
#include "osg/Notify"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool ConvexPlanerOccluder_readLocalData(Object& obj, Input& fr);
bool ConvexPlanerOccluder_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_ConvexPlanerOccluderFuncProxy
(
    osgNew osg::ConvexPlanerOccluder,
    "ConvexPlanerOccluder",
    "Object ConvexPlanerOccluder",
    &ConvexPlanerOccluder_readLocalData,
    &ConvexPlanerOccluder_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool ConvexPlanerOccluder_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    ConvexPlanerOccluder& cpo = static_cast<ConvexPlanerOccluder&>(obj);


    bool matchFirst;
    if ((matchFirst=fr.matchSequence("Occluder {")) || fr.matchSequence("Occluder %i {"))
    {

        ConvexPlanerPolygon& cpp = cpo.getOccluder();
        ConvexPlanerPolygon::VertexList& vertexList = cpp.getVertexList();

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

    ConvexPlanerOccluder::HoleList& holeList = cpo.getHoleList();

    while ((matchFirst=fr.matchSequence("Hole {")) || fr.matchSequence("Hole %i {"))
    {
        holeList.push_back(ConvexPlanerPolygon());
        
        ConvexPlanerPolygon& cpp = holeList.back();
        ConvexPlanerPolygon::VertexList& vertexList = cpp.getVertexList();

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


bool ConvexPlanerOccluder_writeLocalData(const Object& obj, Output& fw)
{
    const ConvexPlanerOccluder& cpo = static_cast<const ConvexPlanerOccluder&>(obj);

    // write out the occluder polygon.
    {
        const ConvexPlanerPolygon::VertexList& vertexList = cpo.getOccluder().getVertexList();

        fw.indent() << "Occluder " << vertexList.size()<< "{"<< std::endl;
        fw.moveIn();

        for(ConvexPlanerPolygon::VertexList::const_iterator itr=vertexList.begin();
            itr!=vertexList.end();
            ++itr)
        {
            fw.indent() << (*itr)[0] << ' ' << (*itr)[1] << ' ' << (*itr)[2] << std::endl;
        }
        
        fw.moveOut();
        fw.indent()<<"}"<< std::endl;
    }
    
    // write out any holes.
    const ConvexPlanerOccluder::HoleList& holeList = cpo.getHoleList();
    for(ConvexPlanerOccluder::HoleList::const_iterator holeItr=holeList.begin();
        holeItr!=holeList.end();
        ++holeItr)
    {
        const ConvexPlanerPolygon::VertexList& vertexList = holeItr->getVertexList();

        fw.indent() << "Hole " << vertexList.size() << "{"<< std::endl;
        fw.moveIn();
        
        for(ConvexPlanerPolygon::VertexList::const_iterator itr=vertexList.begin();
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
