/*  -*-c++-*-
 *  Copyright (C) 2008 Cedric Pinson <cedric.pinson@plopbyte.net>
 *  Copyright (C) 2017 Julien Valentin <mp3butcher@hotmail.com>
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#ifndef OSGANIMATION_VERTEX_INFLUENCE
#define OSGANIMATION_VERTEX_INFLUENCE 1

#include <osg/Object>
#include <osgAnimation/Export>
#include <map>
#include <vector>
#include <string>

namespace osgAnimation
{
    class Skeleton;

    // first is bonename, and second the weight
    typedef std::pair<std::string, float> BoneWeight;
    // first is vertex index, and second the weight
    typedef std::pair<unsigned int, float> VertexIndexWeight;
    // list of IndexWeight
    typedef std::vector<VertexIndexWeight> IndexWeightList;
    // list of IndexWeight
    typedef std::vector<BoneWeight> BoneWeightList;
    // list of Index
    typedef std::vector<unsigned int> IndexList;

    //Bone influence list
    class OSGANIMATION_EXPORT VertexInfluence : public IndexWeightList
    {
    public:
        const std::string& getName() const { return _name; }
        void setName(const std::string& name) { _name = name; }
    protected:
        // the name is the bone to link to
        std::string _name;
    };

    class VertexInfluenceMap : public std::map<std::string, VertexInfluence>, public osg::Object
    {
    public:
        META_Object(osgAnimation, VertexInfluenceMap);

        VertexInfluenceMap() {}
        VertexInfluenceMap(const osgAnimation::VertexInfluenceMap& org, const osg::CopyOp& copyop):
            std::map<std::string, VertexInfluence>(org),
            osg::Object(org, copyop) {}

        ///normalize per vertex weights given numvert of the attached mesh
        void normalize(unsigned int numvert);

        ///remove weakest influences in order to fit targeted numbonepervertex
        void cullInfluenceCountPerVertex(unsigned int maxnumbonepervertex, float minweight=0, bool renormalize=true);

        //compute PerVertexInfluenceList
        void computePerVertexInfluenceList(std::vector<BoneWeightList>& perVertexInfluenceList, unsigned int numvert) const;

        /// map a set of boneinfluence to a list of vertex indices sharing this set
        class VertexGroup: public std::pair<BoneWeightList, IndexList>
        {
        public:
            inline const BoneWeightList& getBoneWeights() const { return first; }
            inline void setBoneWeights( BoneWeightList& o ) { first=o; }
            inline IndexList& vertIDs() { return second; }
        };

        /// compute the minimal VertexGroup Set in which vertices shares the same influence set
        void computeMinimalVertexGroupList(std::vector<VertexGroup>&uniqVertexGroupList, unsigned int numvert) const;

        //Experimental removal of unexpressed bone from the skeleton
        void removeUnexpressedBones(Skeleton &skel) const;
    };
}

#endif
