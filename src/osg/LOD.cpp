#include "osg/LOD"
#include "osg/Input"
#include "osg/Output"
#include "osg/Registry"

#include <algorithm>

using namespace osg;

RegisterObjectProxy<LOD> g_LODProxy;

void LOD::traverse(NodeVisitor& nv)
{
    switch(nv.getTraverseMode())
    {
    case(NodeVisitor::TRAVERSE_ALL_CHILDREN):
        std::for_each(_children.begin(),_children.end(),NodeAcceptOp(nv));
        break;
    case(NodeVisitor::TRAVERSE_ACTIVE_CHILDREN):
        if (_children.size()!=0) _children.front()->accept(nv);
        break;
    default:
        break;
    }
}

void LOD::setRange(unsigned int index, float range)
{
    if (index<_rangeList.size()) _rangeList[index] = range;
    else while (index>=_rangeList.size()) _rangeList.push_back(range);

    if (index<_rangeList2.size()) _rangeList2[index] = range*range;
    else while (index>=_rangeList2.size()) _rangeList2.push_back(range*range);
}

int LOD::evaluate(const Vec3& eye_local, float bias)
{
	// For cache coherency, use _rangeList2 exclusively
    if (_rangeList2.size()==0) return -1;

	// Test distance-squared against the stored array of squared ranges
    float LODRange = (eye_local-_center).length2()*bias;
    if (LODRange<_rangeList2[0]) return -1;
    
    for(unsigned int i=0;i<_rangeList2.size()-1;++i)
    {
        if (_rangeList2[i]<=LODRange && LODRange<_rangeList2[i+1]) {
            return i;
        }
    }
    return -1;
}

bool LOD::readLocalData(Input& fr)
{
    bool iteratorAdvanced = false;

    if (fr.matchSequence("Center %f %f %f"))
    {

        fr[1].getFloat(_center[0]);
        fr[2].getFloat(_center[1]);
        fr[3].getFloat(_center[2]);

        iteratorAdvanced = true;
        fr+=3;
    }

    bool matchFirst = false;
    if ((matchFirst=fr.matchSequence("Ranges {")) || fr.matchSequence("Ranges %i {"))
    {

        // set up coordinates.
        int entry = fr[0].getNoNestedBrackets();

        if (matchFirst)
        {
            fr += 2;
        }
        else
        {
            //_rangeList.(capacity);
            fr += 3;
        }

        float range;
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            if (fr[0].getFloat(range))
            {
                ++fr;
                _rangeList.push_back(range);
                _rangeList2.push_back(range*range);
            }
            else
            {
                ++fr;
            }
        }

        iteratorAdvanced = true;
        ++fr;

    }

    if (Group::readLocalData(fr)) iteratorAdvanced = true;

    return iteratorAdvanced;
}


bool LOD::writeLocalData(Output& fw)
{
    fw.indent() << "Center "<<_center[0] << " "<<_center[1] << " "<<_center[2] <<endl;

    fw.indent() << "Ranges {"<<endl;
    fw.moveIn();
    for(RangeList::iterator riter = _rangeList.begin();
                            riter != _rangeList.end();
                            ++riter)
    {
        fw.indent() << (*riter) <<endl;
    }
    fw.moveOut();
    fw.indent() << "}"<<endl;

    Group::writeLocalData(fw);

    return true;
}
