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
#include <sstream>

using namespace osgAnimation;


RigTransformHardware::RigTransformHardware()
{
    _needInit = true;
    _bonesPerVertex = 0;
    _nbVertexes = 0;
}

RigTransformHardware::RigTransformHardware(const RigTransformHardware& rth, const osg::CopyOp& copyop):
    RigTransform(rth, copyop),
    _bonesPerVertex(rth._bonesPerVertex),
    _nbVertexes(rth._nbVertexes),
    _bonePalette(rth._bonePalette),
    _boneNameToPalette(rth._boneNameToPalette),
    _boneWeightAttribArrays(rth._boneWeightAttribArrays),
    _uniformMatrixPalette(rth._uniformMatrixPalette),
    _shader(rth._shader),
    _needInit(rth._needInit)
{
}

osg::Vec4Array* RigTransformHardware::getVertexAttrib(unsigned int index)
{
    if (index >=  _boneWeightAttribArrays.size())
        return 0;
    return _boneWeightAttribArrays[index].get();
}

unsigned int RigTransformHardware::getNumVertexAttrib()
{
    return _boneWeightAttribArrays.size();
}

osg::Uniform* RigTransformHardware::getMatrixPaletteUniform()
{
    return _uniformMatrixPalette.get();
}


void RigTransformHardware::computeMatrixPaletteUniform(const osg::Matrix& transformFromSkeletonToGeometry, const osg::Matrix& invTransformFromSkeletonToGeometry)
{
    for (unsigned int i = 0; i <  _bonePalette.size(); i++)
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


unsigned int RigTransformHardware::getNumBonesPerVertex() const { return _bonesPerVertex;}
unsigned int RigTransformHardware::getNumVertexes() const { return _nbVertexes;}

typedef std::vector<std::vector<IndexWeight> > VertexIndexWeightList;
void createVertexAttribList(RigTransformHardware& rig,const VertexIndexWeightList&_vertexIndexMatrixWeightList,RigTransformHardware::BoneWeightAttribList & boneWeightAttribArrays);

//
// create vertex attribute by 2 bones
// vec4(boneIndex0, weight0, boneIndex1, weight1)
// if more bones are needed then other attributes are created
// vec4(boneIndex2, weight2, boneIndex3, weight3)
// the idea is to use this format to have a granularity smaller
// than the 4 bones using two vertex attributes
//

void createVertexAttribList(RigTransformHardware& rig,const VertexIndexWeightList& _vertexIndexMatrixWeightList, RigTransformHardware::BoneWeightAttribList& boneWeightAttribArrays)
{
    unsigned int nbVertices= rig.getNumVertexes();
    unsigned int maxbonepervertex=rig.getNumBonesPerVertex();
    unsigned int nbArray = static_cast<unsigned int>(ceilf( ((float)maxbonepervertex) * 0.5f));
    if (!nbArray)
        return ;

    boneWeightAttribArrays.resize(nbArray);
    for (unsigned int i = 0; i < nbArray; i++)
    {
        osg::ref_ptr<osg::Vec4Array> array = new osg::Vec4Array(osg::Array::BIND_PER_VERTEX);
        boneWeightAttribArrays[i] = array;
        array->resize( nbVertices);
        for (unsigned int j = 0; j < nbVertices; j++)
        {

            for (unsigned int b = 0; b < 2; b++)
            {
                // the granularity is 2 so if we have only one bone
                // it's convenient to init the second with a weight 0
                unsigned int boneIndexInList = i*2 + b;
                unsigned int boneIndexInVec4 = b*2;
                (*array)[j][0 + boneIndexInVec4] = 0;
                (*array)[j][1 + boneIndexInVec4] = 0;
                if (boneIndexInList < maxbonepervertex)
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
    return ;
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

bool RigTransformHardware::prepareData(RigGeometry& rig)
{
    if(!rig.getSkeleton() && !rig.getParents().empty())
    {
        RigGeometry::FindNearestParentSkeleton finder;
        if(rig.getParents().size() > 1)
            osg::notify(osg::WARN) << "A RigGeometry should not have multi parent ( " << rig.getName() << " )" << std::endl;
        rig.getParents()[0]->accept(finder);

        if(!finder._root.valid())
        {
            osg::notify(osg::WARN) << "A RigGeometry did not find a parent skeleton for RigGeometry ( " << rig.getName() << " )" << std::endl;
            return false;
        }
        rig.setSkeleton(finder._root.get());
    }
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

    //grab geom source program and vertex shader if _shader is not setted
    if(!_shader.valid() && (program = (osg::Program*)stateset->getAttribute(osg::StateAttribute::PROGRAM)))
    {
        for(unsigned int i=0; i<program->getNumShaders(); ++i)
            if(program->getShader(i)->getType()==osg::Shader::VERTEX) {
                vertexshader=program->getShader(i);
                program->removeShader(vertexshader);

            }
    } else {
        program = new osg::Program;
        program->setName("HardwareSkinning");
    }
    //set default source if _shader is not user setted
    if (!vertexshader.valid()) {
        if (!_shader.valid())
            vertexshader = osg::Shader::readShaderFile(osg::Shader::VERTEX,"skinning.vert");
        else vertexshader=_shader;
    }


    if (!vertexshader.valid()) {
        OSG_WARN << "RigTransformHardware can't load VertexShader" << std::endl;
        return false;
    }

    // replace max matrix by the value from uniform
    {
        std::string str = vertexshader->getShaderSource();
        std::string toreplace = std::string("MAX_MATRIX");
        std::size_t start = str.find(toreplace);
        if (std::string::npos != start) {
            std::stringstream ss;
            ss << getMatrixPaletteUniform()->getNumElements();
            str.replace(start, toreplace.size(), ss.str());
            vertexshader->setShaderSource(str);
        }
        else
        {
            OSG_INFO<< "MAX_MATRIX not found in Shader! " << str << std::endl;
        }
        OSG_INFO << "Shader " << str << std::endl;
    }

    unsigned int attribIndex = 11;
    unsigned int nbAttribs = getNumVertexAttrib();
    if(nbAttribs==0)
        OSG_WARN << "nbAttribs== " << nbAttribs << std::endl;
    for (unsigned int i = 0; i < nbAttribs; i++)
    {
        std::stringstream ss;
        ss << "boneWeight" << i;
        program->addBindAttribLocation(ss.str(), attribIndex + i);

        if(getVertexAttrib(i)->getNumElements()!=_nbVertexes)
            OSG_WARN << "getVertexAttrib== " << getVertexAttrib(i)->getNumElements() << std::endl;
        rig.setVertexAttribArray(attribIndex + i, getVertexAttrib(i));
        OSG_INFO << "set vertex attrib " << ss.str() << std::endl;
    }


    program->addShader(vertexshader.get());
    stateset->removeUniform("nbBonesPerVertex");
    stateset->addUniform(new osg::Uniform("nbBonesPerVertex",_bonesPerVertex));
    stateset->removeUniform("matrixPalette");
    stateset->addUniform(getMatrixPaletteUniform());

    stateset->removeAttribute(osg::StateAttribute::PROGRAM);
    if(!stateset->getAttribute(osg::StateAttribute::PROGRAM))
        stateset->setAttributeAndModes(program.get());

    _needInit = false;
    return true;
}
void createVertexAttribList(RigTransformHardware& rig,const VertexIndexWeightList&_vertexIndexMatrixWeightList,RigTransformHardware::BoneWeightAttribList & boneWeightAttribArrays);

bool RigTransformHardware::buildPalette(BoneMap&boneMap ,RigGeometry&rig) {

    _nbVertexes = rig.getVertexArray()->getNumElements();
    unsigned int maxBonePerVertex=0;

    typedef std::pair<float,unsigned int> FloatInt;
    std::vector< FloatInt > sums;///stat totalweight nbref
    sums.resize(_nbVertexes);

    typedef std::map<std::string, int> BoneNameCountMap;
    _bonePalette.clear();
    _boneNameToPalette.clear();
    BoneNameCountMap boneNameCountMap;

    VertexInfluenceMap *vertexInfluenceMap=rig.getInfluenceMap();
    BoneNamePaletteIndex::iterator boneName2PaletteIndex;
    _boneWeightAttribArrays.resize(0);

    // init temp vertex attribute data
    VertexIndexWeightList vertexIndexWeight;
    vertexIndexWeight.resize(_nbVertexes);

    for (osgAnimation::VertexInfluenceMap::iterator mapit = vertexInfluenceMap->begin();
            mapit != vertexInfluenceMap->end();
            ++mapit)
    {
        const IndexWeightList& boneinflist = mapit->second;
        const std::string& bonename = mapit->first;
        for(IndexWeightList::const_iterator infit = boneinflist.begin(); infit!=boneinflist.end(); ++infit)
        {
            const IndexWeight& iw = *infit;
            const unsigned int &index = iw.getIndex();
            const float &weight = iw.getWeight();

            FloatInt &sum=sums[index];

            if(fabs(weight) > 1e-4) // don't use bone with weight too small
            {
                if ((boneName2PaletteIndex= _boneNameToPalette.find(bonename)) != _boneNameToPalette.end())
                {
                    boneNameCountMap[bonename]++;
                    vertexIndexWeight[index].push_back(IndexWeight(boneName2PaletteIndex->second,weight));
                }
                else
                {
                    BoneMap::const_iterator bonebyname;
                    if ((bonebyname=boneMap.find(bonename)) == boneMap.end())
                    {
                        OSG_WARN << "RigTransformHardware::createPalette can't find bone " << bonename << "in skeleton bonemap:  skip this influence" << std::endl;
                        continue;
                    }
                    boneNameCountMap[bonename] = 1; // for stats

                    _boneNameToPalette[bonename] = _bonePalette.size() ;
                    vertexIndexWeight[index].push_back(IndexWeight(_bonePalette.size(),weight));
                    _bonePalette.push_back(bonebyname->second);
                    sum.first+=weight;
                    ++sum.second;
                }
            }
            else
            {
                OSG_WARN << "RigTransformHardware::createPalette Bone " << bonename << " has a weight " << weight << " for vertex " << index << " this bone will not be in the palette" << std::endl;
            }
            maxBonePerVertex = osg::maximum(maxBonePerVertex, sum.second);

        }
        OSG_INFO << "RigTransformHardware::createPalette maximum number of bone per vertex is " << maxBonePerVertex << std::endl;
        OSG_INFO << "RigTransformHardware::createPalette matrix palette has " << boneNameCountMap.size() << " entries" << std::endl;

        for (BoneNameCountMap::iterator it = boneNameCountMap.begin(); it != boneNameCountMap.end(); ++it)
        {
            OSG_INFO << "RigTransformHardware::createPalette Bone " << it->first << " is used " << it->second << " times" << std::endl;
        }

        OSG_INFO << "RigTransformHardware::createPalette will use " << boneNameCountMap.size() * 4 << " uniforms" << std::endl;


    }

    _bonesPerVertex = maxBonePerVertex;
    _uniformMatrixPalette = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "matrixPalette", _bonePalette.size());

    createVertexAttribList(*this,vertexIndexWeight,this->_boneWeightAttribArrays);
    // normalize weight per vertex
///..assume not sum=0

    /*    for(BoneWeightAttribList::iterator attribit=_boneWeightAttribArrays.begin();attribit!=_boneWeightAttribArrays.end();++attribit){
                std::vector< std::pair<float,unsigned int> >::iterator countit=sums.begin();
            for(osg::Vec4Array::iterator vert=attribit->get()->begin();vert!=attribit->get()->end();++vert,++countit){
                osg::Vec4& v=*vert;
                v[1]/=countit->first;
                v[3]/=countit->first;
            }

        }
    */
    /* unsigned int vertexID=0;
     for (VertIDToBoneWeightList::iterator it = _vertex2Bones.begin(); it != _vertex2Bones.end(); ++it,++vertexID)
     {
         BoneWeightList& bones = *it;
         int size = bones.size();
         if (sums[vertexID].first < 1e-4)
         {
             OSG_WARN << "VertexInfluenceSet::buildVertex2BoneList warning the vertex " << it->first << " seems to have 0 weight, skip normalize for this vertex" << std::endl;
         }
         else
         {
             float mult = 1.0/sums[vertexID].first ;
             for (int i = 0; i < size; i++)
                 bones[i].setWeight(bones[i].getWeight() * mult);
         }
     }
    */
return true;
}

void RigTransformHardware::operator()(RigGeometry& geom)
{
    if (_needInit)
        if (!prepareData(geom))
            return;
    computeMatrixPaletteUniform(geom.getMatrixFromSkeletonToGeometry(), geom.getInvMatrixFromSkeletonToGeometry());
}
