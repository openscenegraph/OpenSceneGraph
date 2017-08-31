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

#define DEFAULT_FIRST_VERTATTRIB_TARGETTED 11
RigTransformHardware::RigTransformHardware()
{
    _needInit = true;
    _bonesPerVertex = 0;
    _nbVertexes = 0;
    _minAttribIndex = DEFAULT_FIRST_VERTATTRIB_TARGETTED;
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


void createVertexAttribList(RigTransformHardware& rig,const  std::vector<std::vector<VertexIndexWeight> > &perVertexInfluences,RigTransformHardware::BoneWeightAttribList & boneWeightAttribArrays);

//
// create vertex attribute by 2 bones
// vec4(boneIndex0, weight0, boneIndex1, weight1)
// if more bones are needed then other attributes are created
// vec4(boneIndex2, weight2, boneIndex3, weight3)
// the idea is to use this format to have a granularity smaller
// than the 4 bones using two vertex attributes
//

typedef std::vector<std::vector<VertexIndexWeight> > PerVertexInfList;
void createVertexAttribList(RigTransformHardware& rig,
                            const PerVertexInfList & perVertexInfluences,
                            RigTransformHardware::BoneWeightAttribList& boneWeightAttribArrays)
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
                if (boneIndexInList < perVertexInfluences[j].size())
                {
                    float boneIndex = static_cast<float>(perVertexInfluences[j][boneIndexInList].getIndex());
                    float boneWeight = perVertexInfluences[j][boneIndexInList].getWeight();
                    // fill the vec4
                    (*array)[j][0 + boneIndexInVec4] = boneIndex;
                    (*array)[j][1 + boneIndexInVec4] = boneWeight;
                }
            }
        }
    }
    return;
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



    return true;
}

bool RigTransformHardware::buildPalette(const BoneMap&boneMap ,const RigGeometry&rig) {

    typedef std::map<std::string, int> BoneNameCountMap;
    _nbVertexes = rig.getVertexArray()->getNumElements();
    _boneWeightAttribArrays.resize(0);
    _bonePalette.clear();
    _boneNameToPalette.clear();

    IndexWeightList::size_type maxBonePerVertex=0;
    BoneNameCountMap boneNameCountMap;

    const VertexInfluenceMap &vertexInfluenceMap=*rig.getInfluenceMap();
    BoneNamePaletteIndex::iterator boneName2PaletteIndex;

    // init temp vertex attribute data
    std::vector<IndexWeightList >  perVertexInfluences;
    perVertexInfluences.resize(_nbVertexes);

    unsigned int paletteindex;
    for (osgAnimation::VertexInfluenceMap::const_iterator boneinflistit = vertexInfluenceMap.begin();
            boneinflistit != vertexInfluenceMap.end();
            ++boneinflistit)
    {
        const IndexWeightList& boneinflist = boneinflistit->second;
        const std::string& bonename = boneinflistit->first;
        BoneMap::const_iterator bonebyname;
        if ((bonebyname=boneMap.find(bonename)) == boneMap.end())
        {
            OSG_WARN << "RigTransformHardware::buildPalette can't find bone " << bonename << "in skeleton bonemap:  skip this influence" << std::endl;
            continue;
        }
        if ((boneName2PaletteIndex= _boneNameToPalette.find(bonename)) != _boneNameToPalette.end())
        {
            boneNameCountMap[bonename]++;
            paletteindex= boneName2PaletteIndex->second ;
        }
        else
        {
            boneNameCountMap[bonename] = 1; // for stats
            _boneNameToPalette[bonename] = _bonePalette.size() ;
            paletteindex= _bonePalette.size() ;
            _bonePalette.push_back(bonebyname->second);

        }
        for(IndexWeightList::const_iterator infit = boneinflist.begin(); infit!=boneinflist.end(); ++infit)
        {
            const VertexIndexWeight& iw = *infit;
            const unsigned int &index = iw.getIndex();
            const float &weight = iw.getWeight();
            IndexWeightList & iwlist=perVertexInfluences[index];

            if(fabs(weight) > 1e-4) // don't use bone with weight too small
            {
                iwlist.push_back(VertexIndexWeight(paletteindex,weight));
            }
            else
            {
                OSG_WARN << "RigTransformHardware::buildPalette Bone " << bonename << " has a weight " << weight << " for vertex " << index << " this bone will not be in the palette" << std::endl;
            }
            maxBonePerVertex = osg::maximum(maxBonePerVertex, iwlist.size());

        }
        OSG_INFO << "RigTransformHardware::buildPalette maximum number of bone per vertex is " << maxBonePerVertex << std::endl;
        OSG_INFO << "RigTransformHardware::buildPalette matrix palette has " << boneNameCountMap.size() << " entries" << std::endl;

        for (BoneNameCountMap::iterator it = boneNameCountMap.begin(); it != boneNameCountMap.end(); ++it)
        {
            OSG_INFO << "RigTransformHardware::buildPalette Bone " << it->first << " is used " << it->second << " times" << std::endl;
        }

        OSG_INFO << "RigTransformHardware::buildPalette will use " << boneNameCountMap.size() * 4 << " uniforms" << std::endl;


    }

    ///normalize
    unsigned int vertid=0;
    for(PerVertexInfList::iterator vertinfit=perVertexInfluences.begin(); vertinfit != perVertexInfluences.end(); ++vertinfit,++vertid)
    {
        float sum=0;
        for(IndexWeightList::iterator iwit = vertinfit->begin(); iwit != vertinfit->end(); ++iwit)
            sum+=iwit->second;

        if(sum< 1e-4){
            OSG_WARN << "RigTransformHardware::buildPalette Warning: vertex with zero sum weights: " <<vertid<< std::endl;
        }
        else
        {
            sum=1.0f/sum;
            for(IndexWeightList::iterator iwit=vertinfit->begin();iwit!=vertinfit->end();++iwit)
                iwit->second*=sum;
        }
    }

    _bonesPerVertex = maxBonePerVertex;
    _uniformMatrixPalette = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "matrixPalette", _bonePalette.size());

    createVertexAttribList(*this,perVertexInfluences,this->_boneWeightAttribArrays);

return true;
}

bool RigTransformHardware::init(RigGeometry& rig){
    if(_uniformMatrixPalette.valid()){
    ///data seams prepared
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

        unsigned int attribIndex = _minAttribIndex;
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
        return false;
    }
    else prepareData(rig);
    return false;
}
void RigTransformHardware::operator()(RigGeometry& geom)
{
    if (_needInit)
        if (!init(geom))
            return;
    computeMatrixPaletteUniform(geom.getMatrixFromSkeletonToGeometry(), geom.getInvMatrixFromSkeletonToGeometry());
}
