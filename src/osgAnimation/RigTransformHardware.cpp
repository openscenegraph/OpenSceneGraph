/*  -*-c++-*-
 *  Copyright (C) 2009 Cedric Pinson <cedric.pinson@plopbyte.net>
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

#include <osgAnimation/RigTransformHardware>
#include <osgAnimation/RigGeometry>
#include <osgAnimation/BoneMapVisitor>
#include <osgDB/ReadFile>
#include <sstream>

using namespace osgAnimation;

RigTransformHardware::RigTransformHardware():
    _bonesPerVertex (0),
    _nbVertices (0),
    _needInit (true),
    _minAttribIndex(RIGTRANSHW_DEFAULT_FIRST_VERTATTRIB_TARGETTED)
{}

RigTransformHardware::RigTransformHardware(const RigTransformHardware& rth, const osg::CopyOp& copyop):
    RigTransform(rth, copyop),
    _bonesPerVertex(rth._bonesPerVertex),
    _nbVertices(rth._nbVertices),
    _bonePalette(rth._bonePalette),
    _boneNameToPalette(rth._boneNameToPalette),
    _boneWeightAttribArrays(rth._boneWeightAttribArrays),
    _uniformMatrixPalette(rth._uniformMatrixPalette),
    _shader(rth._shader),
    _needInit(rth._needInit),
    _minAttribIndex(rth._minAttribIndex)
{
}

osg::Vec4Array* RigTransformHardware::getVertexAttrib(unsigned int index)
{
    if (index >=  _boneWeightAttribArrays.size())
        return 0;
    return _boneWeightAttribArrays[index].get();
}

void RigTransformHardware::computeMatrixPaletteUniform(const osg::Matrix& transformFromSkeletonToGeometry, const osg::Matrix& invTransformFromSkeletonToGeometry)
{
    for (unsigned int i = 0; i <  _bonePalette.size(); ++i)
    {
        osg::ref_ptr<Bone> bone = _bonePalette[i].get();
        const osg::Matrixf& invBindMatrix = bone->getInvBindMatrixInSkeletonSpace();
        const osg::Matrixf& boneMatrix = bone->getMatrixInSkeletonSpace();
        osg::Matrixf resultBoneMatrix = invBindMatrix * boneMatrix;
        osg::Matrixf result =  transformFromSkeletonToGeometry * resultBoneMatrix * invTransformFromSkeletonToGeometry;
        if (!_uniformMatrixPalette->setElement(i, result))
            OSG_WARN << "RigTransformHardware::computeUniformMatrixPalette can't set uniform at " << i << " elements" << std::endl;
    }
}

//
// create vertex attribute by 2 bones
// vec4(boneIndex0, weight0, boneIndex1, weight1)
// if more bones are needed then other attributes are created
// vec4(boneIndex2, weight2, boneIndex3, weight3)
// the idea is to use this format to have a granularity smaller
// than the 4 bones using two vertex attributes
//

typedef std::vector<std::vector<VertexIndexWeight> > PerVertexInfList;

///create normalized a set of Vertex Attribs given a PerVertexInfList and return the max num bone per vertex
unsigned int createVertexAttribList(const PerVertexInfList & perVertexInfluences,
                                    RigTransformHardware::BoneWeightAttribList& boneWeightAttribArrays)
{
    short boneIndexInVec4;
    unsigned int vertid = 0,
                 boneIndexInList;
    IndexWeightList::size_type maxBonePerVertex = 0;
    ///build vertex attrib arrays
    //get maxBonePerVertex
    for(PerVertexInfList::const_iterator vertinfit = perVertexInfluences.begin(); vertinfit != perVertexInfluences.end(); ++vertinfit)
        maxBonePerVertex = osg::maximum(maxBonePerVertex, vertinfit->size());

    OSG_INFO << "RigTransformHardware::createVertexAttribList maximum number of bone per vertex is " << maxBonePerVertex << std::endl;

    unsigned int nbArray = static_cast<unsigned int>(ceilf( ((float)maxBonePerVertex) * 0.5f));

    if (!nbArray)
        return 0;

    ///create vertex attrib arrays
    boneWeightAttribArrays.reserve(nbArray);
    boneWeightAttribArrays.resize(nbArray);
    for(unsigned int j = 0; j< nbArray; ++j)
    {
        osg::Vec4Array* vecattr = new osg::Vec4Array(osg::Array::BIND_PER_VERTEX);
        vecattr->reserve(perVertexInfluences.size());
        vecattr->resize(perVertexInfluences.size());
        boneWeightAttribArrays[j] = vecattr;
    }

    ///populate vertex attrib arrays
    for(PerVertexInfList::const_iterator vertinfit = perVertexInfluences.begin();
            vertinfit != perVertexInfluences.end();
            ++vertinfit, ++vertid)
    {
        //sum for normalization
        float sum = 0;
        for(IndexWeightList::const_iterator iwit = vertinfit->begin(); iwit != vertinfit->end(); ++iwit)
            sum+=iwit->second;

        if(sum< 1e-4)
        {
            OSG_WARN << "RigTransformHardware::createVertexAttribList Warning: vertex with zero sum weights: " <<vertid<< std::endl;
            return false;
        }
        else
        {
            sum = 1.0f/sum;
            for (unsigned int j = 0; j < nbArray; ++j)
            {
                osg::Vec4& dest = (* boneWeightAttribArrays[j])[vertid];
                for (unsigned int b = 0; b < 2; ++b)
                {
                    boneIndexInVec4 = b*2;
                    boneIndexInList = j*2 + b;
                    if (boneIndexInList < (*vertinfit).size())
                    {
                        float boneIndex = static_cast<float>((*vertinfit)[boneIndexInList].first);
                        ///normalization here
                        float boneWeight = (*vertinfit)[boneIndexInList].second*sum;
                        dest[0 + boneIndexInVec4] = boneIndex;
                        dest[1 + boneIndexInVec4] = boneWeight;
                    }
                    else
                    {
                        dest[0 + boneIndexInVec4] = 0;
                        dest[1 + boneIndexInVec4] = 0;
                    }
                }
            }
        }
    }
    return maxBonePerVertex;
}

bool RigTransformHardware::prepareData(RigGeometry& rig)
{
    _nbVertices = rig.getSourceGeometry()->getVertexArray()->getNumElements();
    const VertexInfluenceMap &vertexInfluenceMap = *rig.getInfluenceMap();
    _perVertexInfluences.reserve(_nbVertices);
    _perVertexInfluences.resize(_nbVertices);

    unsigned int localboneid = 0;
    for (VertexInfluenceMap::const_iterator boneinflistit = vertexInfluenceMap.begin();
            boneinflistit != vertexInfluenceMap.end();
            ++boneinflistit, ++localboneid)
    {
        const IndexWeightList& boneinflist = boneinflistit->second;
        const std::string& bonename = boneinflistit->first;

        for(IndexWeightList::const_iterator infit = boneinflist.begin(); infit!=boneinflist.end(); ++infit)
        {
            const VertexIndexWeight& iw = *infit;
            const unsigned int &index = iw.first;
            const float &weight = iw.second;
            IndexWeightList & iwlist = _perVertexInfluences[index];

            if(fabs(weight) > 1e-4) // don't use bone with weight too small
            {
                iwlist.push_back(VertexIndexWeight(localboneid,weight));
            }
            else
            {
                OSG_WARN << "RigTransformHardware::prepareData Bone " << bonename << " has a weight " << weight << " for vertex " << index << " this bone will not be in the palette" << std::endl;
            }
        }
    }
    return true;
}


bool RigTransformHardware::buildPalette(const BoneMap& boneMap, const RigGeometry& rig)
{

    typedef std::map<std::string, int> BoneNameCountMap;
    _boneWeightAttribArrays.resize(0);
    _bonePalette.clear();
    _boneNameToPalette.clear();

    BoneNameCountMap boneNameCountMap;

    const VertexInfluenceMap &vertexInfluenceMap = *rig.getInfluenceMap();
    BoneNamePaletteIndex::iterator boneName2PaletteIndex;

    ///create local boneid to paletteindex
    unsigned int paletteindex;
    std::vector<int> localid2bone;
    localid2bone.reserve(vertexInfluenceMap.size());
    for (osgAnimation::VertexInfluenceMap::const_iterator perBoneinfit = vertexInfluenceMap.begin();
            perBoneinfit != vertexInfluenceMap.end();
            ++perBoneinfit)
    {
        const std::string& bonename = perBoneinfit->first;

        if (bonename.empty())
        {
            OSG_WARN << "RigTransformHardware::VertexInfluenceMap contains unnamed bone IndexWeightList" << std::endl;
        }
        BoneMap::const_iterator bmit = boneMap.find(bonename);
        if (bmit == boneMap.end() )
        {
            OSG_WARN << "RigTransformHardware Bone " << bonename << " not found, skip the influence group " << std::endl;
            localid2bone.push_back(-1);
            continue;
        }
        if ( (boneName2PaletteIndex =  _boneNameToPalette.find(bonename)) != _boneNameToPalette.end())
        {
            boneNameCountMap[bonename]++;
            paletteindex = boneName2PaletteIndex->second ;
        }
        else
        {
            boneNameCountMap[bonename] = 1; // for stats
            _boneNameToPalette[bonename] = _bonePalette.size() ;
            paletteindex = _bonePalette.size() ;
            _bonePalette.push_back(bmit->second);
        }
        localid2bone.push_back(paletteindex);
    }
    OSG_INFO << "RigTransformHardware::buildPalette matrix palette has " << boneNameCountMap.size() << " entries" << std::endl;
    for (BoneNameCountMap::iterator it = boneNameCountMap.begin(); it != boneNameCountMap.end(); ++it)
    {
        OSG_INFO << "RigTransformHardware::buildPalette Bone " << it->first << " is used " << it->second << " times" << std::endl;
    }
    OSG_INFO << "RigTransformHardware::buildPalette will use " << boneNameCountMap.size() * 4 << " uniforms" << std::endl;

    ///set paletteindices
    for( std::vector<IndexWeightList>::iterator idwlistit = _perVertexInfluences.begin(); idwlistit!=_perVertexInfluences.end(); ++idwlistit)
    {
        for( IndexWeightList::iterator idwit = idwlistit->begin(); idwit!=idwlistit->end();)
        {
            if(localid2bone[idwit->first]<0)
                idwit = idwlistit->erase(idwit);
            else
            {
                idwit->first = localid2bone[idwit->first];
                ++idwit;
            }
        }
    }
    if( (_bonesPerVertex = createVertexAttribList(_perVertexInfluences, _boneWeightAttribArrays) ) < 1 )
        return false;

    _uniformMatrixPalette = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "matrixPalette", _bonePalette.size());

    _needInit = true;
    return true;
}

bool RigTransformHardware::init(RigGeometry& rig)
{
    if(_perVertexInfluences.empty())
    {
        prepareData(rig);
        return false;
    }
    if(!rig.getSkeleton())
        return false;

    BoneMapVisitor mapVisitor;
    rig.getSkeleton()->accept(mapVisitor);
    BoneMap boneMap = mapVisitor.getBoneMap();

    if (!buildPalette(boneMap,rig) )
        return false;

    osg::Geometry& source = *rig.getSourceGeometry();
    osg::Vec3Array* positionSrc = dynamic_cast<osg::Vec3Array*>(source.getVertexArray());

    if (!positionSrc)
    {
        OSG_WARN << "RigTransformHardware no vertex array in the geometry " << rig.getName() << std::endl;
        return false;
    }

    // copy shallow from source geometry to rig
    rig.copyFrom(source);

    osg::ref_ptr<osg::Program> program ;
    osg::ref_ptr<osg::Shader> vertexshader;
    osg::ref_ptr<osg::StateSet> stateset = rig.getOrCreateStateSet();

    //grab geom source program and vertex shader if _shader is not set
    if(!_shader.valid() && (program = (osg::Program*)stateset->getAttribute(osg::StateAttribute::PROGRAM)))
    {
        for(unsigned int i = 0; i<program->getNumShaders(); ++i)
            if(program->getShader(i)->getType() == osg::Shader::VERTEX)
            {
                vertexshader = program->getShader(i);
                program->removeShader(vertexshader);
            }
    }
    else
    {
        program = new osg::Program;
        program->setName("HardwareSkinning");
    }
    //set default source if _shader is not user set
    if (!vertexshader.valid())
    {
        if (!_shader.valid()) vertexshader = osgDB::readRefShaderFile(osg::Shader::VERTEX,"skinning.vert");
        else vertexshader = _shader;
    }

    if (!vertexshader.valid())
    {
        OSG_WARN << "RigTransformHardware can't load VertexShader" << std::endl;
        return false;
    }

    // replace max matrix by the value from uniform
    {
        std::string str = vertexshader->getShaderSource();
        std::string toreplace = std::string("MAX_MATRIX");
        std::size_t start = str.find(toreplace);
        if (std::string::npos != start)
        {
            std::stringstream ss;
            ss << getMatrixPaletteUniform()->getNumElements();
            str.replace(start, toreplace.size(), ss.str());
            vertexshader->setShaderSource(str);
        }
        else
        {
            OSG_WARN<< "MAX_MATRIX not found in Shader! " << str << std::endl;
        }
        OSG_INFO << "Shader " << str << std::endl;
    }

    unsigned int nbAttribs = getNumVertexAttrib();
    for (unsigned int i = 0; i < nbAttribs; i++)
    {
        std::stringstream ss;
        ss << "boneWeight" << i;
        program->addBindAttribLocation(ss.str(), _minAttribIndex + i);
        rig.setVertexAttribArray(_minAttribIndex + i, getVertexAttrib(i));
        OSG_INFO << "set vertex attrib " << ss.str() << std::endl;
    }

    program->addShader(vertexshader.get());

    stateset->removeUniform("nbBonesPerVertex");
    stateset->addUniform(new osg::Uniform("nbBonesPerVertex",_bonesPerVertex));

    stateset->removeUniform("matrixPalette");
    stateset->addUniform(_uniformMatrixPalette);

    stateset->setAttribute(program.get());

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
