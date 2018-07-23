/*  -*-c++-*-
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

#include <osgAnimation/MorphTransformHardware>
#include <osgAnimation/MorphGeometry>
#include <osgAnimation/BoneMapVisitor>
#include <osg/TextureBuffer>
#include <osgDB/ReadFile>
#include <sstream>

using namespace osgAnimation;

MorphTransformHardware::MorphTransformHardware():
    _needInit(true),
    _reservedTextureUnit(MORPHTRANSHW_DEFAULTMORPHTEXTUREUNIT)
{
}

MorphTransformHardware::MorphTransformHardware(const MorphTransformHardware& rth, const osg::CopyOp& copyop):
    MorphTransform(rth, copyop),
    _uniformTargetsWeight(rth._uniformTargetsWeight),
    _shader(rth._shader),
    _needInit(rth._needInit),
    _reservedTextureUnit(rth._reservedTextureUnit)
{
}

bool MorphTransformHardware::init(MorphGeometry& morphGeometry)
{
    osg::Vec3Array* pos = dynamic_cast<osg::Vec3Array*>(morphGeometry.getVertexArray());

    osg::Vec3Array * vertexSource = (morphGeometry.getVertexSource());
    osg::Vec3Array * normalSource = (morphGeometry.getNormalSource());
    morphGeometry.setDataVariance(osg::Object::STATIC);
    ///check for correct morph configuration
    ///(blender osgexport doesn't set sources so assume morphgeom arrays are sources)
    if(pos)
    {
        pos->setDataVariance(osg::Object::STATIC);
        ///check if source is set correctly
        if (!vertexSource|| vertexSource->size() != pos->size())
        {
            vertexSource =(static_cast<osg::Vec3Array*>( pos->clone(osg::CopyOp::DEEP_COPY_ARRAYS)));
        }
        osg::Vec3Array* normal = dynamic_cast<osg::Vec3Array*>(morphGeometry.getNormalArray());
        bool normalmorphable = morphGeometry.getMorphNormals() && normal&&(normal->getBinding()==osg::Array::BIND_PER_VERTEX);
        if(!normalmorphable)
        {
            OSG_WARN << "MorphTransformHardware::morph geometry "<<morphGeometry.getName()<<" without per vertex normal : HWmorphing not supported! "  << std::endl;
            return false;
        }
        normal->setDataVariance(osg::Object::STATIC);
        if (normalmorphable && (!normalSource || normalSource->size() != normal->size()))
        {
            normalSource =(static_cast<osg::Vec3Array*>( normal->clone(osg::CopyOp::DEEP_COPY_ARRAYS)));
        }
    }
    
    ///end check
    morphGeometry.setVertexArray(morphGeometry.getVertexSource());
    morphGeometry.setNormalArray(morphGeometry.getNormalSource(),osg::Array::BIND_PER_VERTEX);
    morphGeometry.setDataVariance(osg::Object::STATIC);

    //create one TBO for all morphtargets (pack vertex/normal)
    osg::Vec3Array *  morphTargets=new osg::Vec3Array ;
    MorphGeometry::MorphTargetList & morphlist=morphGeometry.getMorphTargetList();
    for(MorphGeometry::MorphTargetList::const_iterator curmorph=morphlist.begin(); curmorph!=morphlist.end(); ++curmorph)
    {
        const osg::Geometry * morphtargetgeom=                curmorph->getGeometry() ;
        const osg::Vec3Array *varray=(osg::Vec3Array*)morphtargetgeom->getVertexArray();
        const osg::Vec3Array *narray=(osg::Vec3Array*)morphtargetgeom->getNormalArray();
        if(morphGeometry.getMethod()==MorphGeometry::RELATIVE)
        {
            for(unsigned int i=0; i<morphGeometry.getVertexArray()->getNumElements(); ++i)
            {
                morphTargets->push_back( (*varray)[i]);
                morphTargets->push_back( (*narray)[i]);
            }
        }
        else
        {
            //convert to RELATIVE as it involve less math in the VS than NORMALIZED
            const osg::Vec3Array *ovarray=(osg::Vec3Array*)morphGeometry.getVertexArray();
            const osg::Vec3Array *onarray=(osg::Vec3Array*)morphGeometry.getNormalArray();
            for(unsigned int i=0; i<morphGeometry.getVertexArray()->getNumElements(); ++i)
            {
                morphTargets->push_back( (*varray)[i]- (*ovarray)[i] );
                morphTargets->push_back( (*narray)[i]- (*onarray)[i] );
            }
        }
    }
    
    osg::ref_ptr<osg::TextureBuffer> morphTargetsTBO=new osg::TextureBuffer();
    morphTargetsTBO->setBufferData(morphTargets);
    morphTargetsTBO->setInternalFormat( GL_RGB32F_ARB );

    //create TBO Texture handle
    osg::ref_ptr<osg::Uniform> morphTBOHandle=new osg::Uniform(osg::Uniform::SAMPLER_BUFFER,"morphTargets");
    morphTBOHandle->set((int)_reservedTextureUnit);

    //create dynamic uniform for morphtargets animation weights
    _uniformTargetsWeight=new osg::Uniform(osg::Uniform::FLOAT,"morphWeights",morphlist.size());


    osg::ref_ptr<osg::Program> program ;
    osg::ref_ptr<osg::Shader> vertexshader;
    osg::ref_ptr<osg::StateSet> stateset = morphGeometry.getOrCreateStateSet();
    //grab geom source program and vertex shader if _shader is not set
    if(!_shader.valid() && (program = (osg::Program*)stateset->getAttribute(osg::StateAttribute::PROGRAM)))
    {
        for(unsigned int i=0;i<program->getNumShaders();++i)
        {
            if(program->getShader(i)->getType()==osg::Shader::VERTEX)
            {
               // vertexshader=program->getShader(i);
                program->removeShader(vertexshader);
            }
        }
    }
    
    if (!program)
    {
        program = new osg::Program;
    }
    
    program->setName("HardwareMorphing");
    //set default source if _shader is not user set
    if (!vertexshader.valid())
    {
        if (!_shader.valid()) vertexshader = osgDB::readRefShaderFile(osg::Shader::VERTEX,"morphing.vert");
        else vertexshader=_shader;
    }

    if (!vertexshader.valid())
    {
        OSG_WARN << "RigTransformHardware can't load VertexShader" << std::endl;
        return false;
    }

    // replace max matrix by the value from uniform
    {
        std::string str = vertexshader->getShaderSource();
        std::string toreplace = std::string("MAX_MORPHWEIGHT");
        std::size_t start = str.find(toreplace);
        if (std::string::npos == start)
        {
            // perhaps remanance from previous init (if saved after init) so reload shader
            vertexshader = osgDB::readRefShaderFile(osg::Shader::VERTEX,"morphing.vert");
            if (!vertexshader.valid())
            {
                OSG_WARN << "RigTransformHardware can't load VertexShader" << std::endl;
                return false;
            }
            str = vertexshader->getShaderSource();
            start = str.find(toreplace);
        }
        if (std::string::npos != start)
        {
            std::stringstream ss;
            ss << _uniformTargetsWeight->getNumElements();
            str.replace(start, toreplace.size(), ss.str());
            vertexshader->setShaderSource(str);
        }
        else
        {
            OSG_WARN << "MAX_MORPHWEIGHT not found in Shader! " << str << std::endl;
        }
        OSG_INFO << "Shader " << str << std::endl;
    }

    program->addShader(vertexshader.get());
    //morphGeometry.setStateSet((osg::StateSet *) osg::CopyOp()(source.getOrCreateStateSet()));

    osg::ref_ptr<osg::StateSet> ss = morphGeometry.getOrCreateStateSet();
    ss->addUniform(_uniformTargetsWeight);
    ss->setTextureAttribute(_reservedTextureUnit,morphTargetsTBO);
    ss->addUniform( morphTBOHandle);
    ss->addUniform(new osg::Uniform("nbMorphVertex", morphGeometry.getVertexArray()->getNumElements()));

    ss->setAttributeAndModes(program.get());
    _needInit = false;
    return true;
}

void MorphTransformHardware::operator()(MorphGeometry& geom)
{
    if (_needInit && !init(geom)) return;
    
    if (geom.isDirty())
    {
        ///upload new morph weights each update via uniform
        int curimorph=0;
        MorphGeometry::MorphTargetList & morphlist=geom.getMorphTargetList();
        for(MorphGeometry::MorphTargetList::const_iterator curmorph=morphlist.begin(); curmorph!=morphlist.end(); ++curmorph)
        {
            _uniformTargetsWeight->setElement(curimorph++, curmorph->getWeight());
        }
        _uniformTargetsWeight->dirty();
        geom.dirty(false);
    }
}
