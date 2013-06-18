/*  -*-c++-*-
 *  Copyright (C) 2009 Cedric Pinson <cedric.pinson@plopbyte.net>
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

#include <osgAnimation/RigTransformHardware>
#include <osgAnimation/RigGeometry>
#include <osgAnimation/BoneMapVisitor>
#include <sstream>

using namespace osgAnimation;


RigTransformHardware::RigTransformHardware()
{
    _needInit = true;
    _bonesPerVertex = 0;
    _nbVertexes = 0;
}

osg::Vec4Array* RigTransformHardware::getVertexAttrib(int index)
{
    if (index >= (int)_boneWeightAttribArrays.size())
        return 0;
    return _boneWeightAttribArrays[index].get();
}

int RigTransformHardware::getNumVertexAttrib()
{
    return _boneWeightAttribArrays.size();
}

osg::Uniform* RigTransformHardware::getMatrixPaletteUniform()
{
    return _uniformMatrixPalette.get();
}


void RigTransformHardware::computeMatrixPaletteUniform(const osg::Matrix& transformFromSkeletonToGeometry, const osg::Matrix& invTransformFromSkeletonToGeometry)
{
    for (int i = 0; i < (int)_bonePalette.size(); i++)
    {
        osg::ref_ptr<Bone> bone = _bonePalette[i].get();
        const osg::Matrix& invBindMatrix = bone->getInvBindMatrixInSkeletonSpace();
        const osg::Matrix& boneMatrix = bone->getMatrixInSkeletonSpace();
        osg::Matrix resultBoneMatrix = invBindMatrix * boneMatrix;
        osg::Matrix result =  transformFromSkeletonToGeometry * resultBoneMatrix * invTransformFromSkeletonToGeometry;
        if (!_uniformMatrixPalette->setElement(i, result))
            OSG_WARN << "RigTransformHardware::computeUniformMatrixPalette can't set uniform at " << i << " elements" << std::endl;
    }
}


int RigTransformHardware::getNumBonesPerVertex() const { return _bonesPerVertex;}
int RigTransformHardware::getNumVertexes() const { return _nbVertexes;}

bool RigTransformHardware::createPalette(int nbVertexes, BoneMap boneMap, const VertexInfluenceSet::VertexIndexToBoneWeightMap& vertexIndexToBoneWeightMap)
{
    typedef std::map<std::string, int> BoneNameCountMap;
    typedef std::map<std::string, int> BoneNamePaletteIndex;
    BoneNamePaletteIndex bname2palette;
    BonePalette palette;
    BoneNameCountMap boneNameCountMap;

    // init vertex attribute data
    VertexIndexWeightList vertexIndexWeight;
    vertexIndexWeight.resize(nbVertexes);

    int maxBonePerVertex = 0;
    for (VertexInfluenceSet::VertexIndexToBoneWeightMap::const_iterator it = vertexIndexToBoneWeightMap.begin(); it != vertexIndexToBoneWeightMap.end(); ++it)
    {
        int vertexIndex = it->first;
        const VertexInfluenceSet::BoneWeightList& boneWeightList = it->second;
        int bonesForThisVertex = 0;
        for (VertexInfluenceSet::BoneWeightList::const_iterator it = boneWeightList.begin(); it != boneWeightList.end(); ++it)
        {
            const VertexInfluenceSet::BoneWeight& bw = *it;
            if (boneNameCountMap.find(bw.getBoneName()) != boneNameCountMap.end())
            {
                boneNameCountMap[bw.getBoneName()]++;
                bonesForThisVertex++; // count max number of bones per vertexes
                vertexIndexWeight[vertexIndex].push_back(IndexWeightEntry(bname2palette[bw.getBoneName()],bw.getWeight()));
            }
            else if (fabs(bw.getWeight()) > 1e-2) // dont use bone with weight too small
            {
                if (boneMap.find(bw.getBoneName()) == boneMap.end())
                {
                    OSG_INFO << "RigTransformHardware::createPalette can't find bone " << bw.getBoneName() << " skip this influence" << std::endl;
                    continue;
                }
                boneNameCountMap[bw.getBoneName()] = 1; // for stats
                bonesForThisVertex++;
                palette.push_back(boneMap[bw.getBoneName()]);
                bname2palette[bw.getBoneName()] = palette.size()-1;
                vertexIndexWeight[vertexIndex].push_back(IndexWeightEntry(bname2palette[bw.getBoneName()],bw.getWeight()));
            }
            else
            {
                OSG_WARN << "RigTransformHardware::createPalette Bone " << bw.getBoneName() << " has a weight " << bw.getWeight() << " for vertex " << vertexIndex << " this bone will not be in the palette" << std::endl;
            }
        }
        maxBonePerVertex = osg::maximum(maxBonePerVertex, bonesForThisVertex);
    }
    OSG_INFO << "RigTransformHardware::createPalette maximum number of bone per vertex is " << maxBonePerVertex << std::endl;
    OSG_INFO << "RigTransformHardware::createPalette matrix palette has " << boneNameCountMap.size() << " entries" << std::endl;

    for (BoneNameCountMap::iterator it = boneNameCountMap.begin(); it != boneNameCountMap.end(); ++it)
    {
        OSG_INFO << "RigTransformHardware::createPalette Bone " << it->first << " is used " << it->second << " times" << std::endl;
    }

    OSG_INFO << "RigTransformHardware::createPalette will use " << boneNameCountMap.size() * 4 << " uniforms" << std::endl;


    for (int i = 0 ; i < (int)vertexIndexWeight.size(); i++)
        vertexIndexWeight[i].resize(maxBonePerVertex);

    _nbVertexes = nbVertexes;
    _bonesPerVertex = maxBonePerVertex;
    _bonePalette = palette;
    _vertexIndexMatrixWeightList = vertexIndexWeight;
    _uniformMatrixPalette = createVertexUniform();
    _boneWeightAttribArrays = createVertexAttribList();
    return true;
}


//
// create vertex attribute by 2 bones
// vec4(boneIndex0, weight0, boneIndex1, weight1)
// if more bones are needed then other attributes are created
// vec4(boneIndex2, weight2, boneIndex3, weight3)
// the idea is to use this format to have a granularity smaller
// than the 4 bones using two vertex attributes
//
RigTransformHardware::BoneWeightAttribList RigTransformHardware::createVertexAttribList()
{
    BoneWeightAttribList arrayList;
    int nbArray = static_cast<int>(ceilf(getNumBonesPerVertex() * 0.5));
    if (!nbArray)
        return arrayList;

    arrayList.resize(nbArray);
    for (int i = 0; i < nbArray; i++)
    {
        osg::ref_ptr<osg::Vec4Array> array = new osg::Vec4Array;
        arrayList[i] = array;
        int nbVertexes = getNumVertexes();
        array->resize(nbVertexes);
        for (int j = 0; j < nbVertexes; j++)
        {
            for (int b = 0; b < 2; b++)
            {
                // the granularity is 2 so if we have only one bone
                // it's convenient to init the second with a weight 0
                int boneIndexInList = i*2 + b;
                int boneIndexInVec4 = b*2;
                (*array)[j][0 + boneIndexInVec4] = 0;
                (*array)[j][1 + boneIndexInVec4] = 0;
                if (boneIndexInList < getNumBonesPerVertex())
                {
                    float boneIndex = static_cast<float>(_vertexIndexMatrixWeightList[j][boneIndexInList].getIndex());
                    float boneWeight = _vertexIndexMatrixWeightList[j][boneIndexInList].getWeight();
                    // fill the vec4
                    (*array)[j][0 + boneIndexInVec4] = boneIndex;
                    (*array)[j][1 + boneIndexInVec4] = boneWeight;
                }
            }
        }
    }
    return arrayList;
}


osg::Uniform* RigTransformHardware::createVertexUniform()
{
    osg::Uniform* uniform = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "matrixPalette", _bonePalette.size());
    return uniform;
}


void RigTransformHardware::setShader(osg::Shader* shader)
{
    _shader = shader;
}

bool RigTransformHardware::init(RigGeometry& geom)
{
    osg::Geometry& source = *geom.getSourceGeometry();
    osg::Vec3Array* positionSrc = dynamic_cast<osg::Vec3Array*>(source.getVertexArray());
    if (!positionSrc)
    {
        OSG_WARN << "RigTransformHardware no vertex array in the geometry " << geom.getName() << std::endl;
        return false;
    }

    if (!geom.getSkeleton())
    {
        OSG_WARN << "RigTransformHardware no skeleton set in geometry " << geom.getName() << std::endl;
        return false;
    }


    // copy shallow from source geometry to rig
    geom.copyFrom(source);


    BoneMapVisitor mapVisitor;
    geom.getSkeleton()->accept(mapVisitor);
    BoneMap bm = mapVisitor.getBoneMap();

    if (!createPalette(positionSrc->size(),bm, geom.getVertexInfluenceSet().getVertexToBoneList()))
        return false;

    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->setName("HardwareSkinning");
    if (!_shader.valid())
        _shader = osg::Shader::readShaderFile(osg::Shader::VERTEX,"skinning.vert");

    if (!_shader.valid()) {
        OSG_WARN << "RigTransformHardware can't load VertexShader" << std::endl;
        return false;
    }

    // replace max matrix by the value from uniform
    {
    std::string str = _shader->getShaderSource();
    std::string toreplace = std::string("MAX_MATRIX");
    std::size_t start = str.find(toreplace);
    std::stringstream ss;
    ss << getMatrixPaletteUniform()->getNumElements();
    str.replace(start, toreplace.size(), ss.str());
    _shader->setShaderSource(str);
    OSG_INFO << "Shader " << str << std::endl;
    }

    int attribIndex = 11;
    int nbAttribs = getNumVertexAttrib();
    for (int i = 0; i < nbAttribs; i++)
    {
        std::stringstream ss;
        ss << "boneWeight" << i;
        program->addBindAttribLocation(ss.str(), attribIndex + i);
        geom.setVertexAttribArray(attribIndex + i, getVertexAttrib(i));
        OSG_INFO << "set vertex attrib " << ss.str() << std::endl;
    }
    program->addShader(_shader.get());

    osg::ref_ptr<osg::StateSet> ss = geom.getOrCreateStateSet();
    ss->addUniform(getMatrixPaletteUniform());
    ss->addUniform(new osg::Uniform("nbBonesPerVertex", getNumBonesPerVertex()));
    ss->setAttributeAndModes(program.get());

    _needInit = false;
    return true;
}
void RigTransformHardware::operator()(RigGeometry& geom)
{
    if (_needInit)
        if (!init(geom))
            return;
    computeMatrixPaletteUniform(geom.getMatrixFromSkeletonToGeometry(), geom.getInvMatrixFromSkeletonToGeometry());
}
