/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
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

#include <osg/TexEnv>
#include <osg/TexEnvCombine>

#include <osgFX/MultiTextureControl>

using namespace osg;
using namespace osgFX;

MultiTextureControl::MultiTextureControl():
    _useTexEnvCombine(true),
    _useTextureWeightsUniform(true)
{
    _textureWeights = new TextureWeights;
}

MultiTextureControl::MultiTextureControl(const MultiTextureControl& copy, const osg::CopyOp& copyop):
    Group(copy,copyop),
    _textureWeights(osg::clone(copy._textureWeights.get(), osg::CopyOp::DEEP_COPY_ALL)),
    _useTexEnvCombine(copy._useTexEnvCombine),
    _useTextureWeightsUniform(copy._useTextureWeightsUniform)
{
    updateStateSet();
}

void MultiTextureControl::setTextureWeight(unsigned int unit, float weight)
{
    if (unit >= _textureWeights->size())
    {
        _textureWeights->resize(unit+1,0.0f);
    }
    (*_textureWeights)[unit] = weight;

    updateStateSet();
}

void MultiTextureControl::updateStateSet()
{
    osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;

    if (_useTexEnvCombine)
    {
        unsigned int numTextureUnitsOn = 0;
        unsigned int unit;
        for(unit=0;unit<_textureWeights->size();++unit)
        {
            if ((*_textureWeights)[unit]>0.0f) ++numTextureUnitsOn;
        }

        if (numTextureUnitsOn<=1)
        {
            for(unit=0;unit<_textureWeights->size();++unit)
            {
                if ((*_textureWeights)[unit]>0.0f)
                {
                    osg::TexEnv* texenv = new osg::TexEnv(osg::TexEnv::MODULATE);
                    stateset->setTextureAttribute(unit, texenv);
                    stateset->setTextureMode(unit, GL_TEXTURE_2D, osg::StateAttribute::ON);
                }
                else
                {
                    stateset->setTextureMode(unit, GL_TEXTURE_2D, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
                }
            }

        }
        else if (_textureWeights->size()==2)
        {
            {
                osg::TexEnvCombine* texenv = new osg::TexEnvCombine;
                texenv->setCombine_RGB(osg::TexEnvCombine::INTERPOLATE);
                texenv->setSource0_RGB(osg::TexEnvCombine::TEXTURE0+0);
                texenv->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
                texenv->setSource1_RGB(osg::TexEnvCombine::TEXTURE0+1);
                texenv->setOperand1_RGB(osg::TexEnvCombine::SRC_COLOR);
                texenv->setSource2_RGB(osg::TexEnvCombine::CONSTANT);
                texenv->setOperand2_RGB(osg::TexEnvCombine::SRC_COLOR);

                float r = (*_textureWeights)[0]/((*_textureWeights)[0]+(*_textureWeights)[1]);
                texenv->setConstantColor(osg::Vec4(r,r,r,r));

                stateset->setTextureAttribute(0, texenv);
            }

            {
                osg::TexEnvCombine* texenv = new osg::TexEnvCombine;
                texenv->setCombine_RGB(osg::TexEnvCombine::MODULATE);
                texenv->setSource0_RGB(osg::TexEnvCombine::PREVIOUS);
                texenv->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
                texenv->setSource1_RGB(osg::TexEnvCombine::PRIMARY_COLOR);
                texenv->setOperand1_RGB(osg::TexEnvCombine::SRC_COLOR);

                stateset->setTextureAttribute(1, texenv);
            }
        }
        else if (_textureWeights->size()==3)
        {
            float b = ((*_textureWeights)[0]+(*_textureWeights)[1])/((*_textureWeights)[0]+(*_textureWeights)[1]+(*_textureWeights)[2]);
            float a = (*_textureWeights)[0]/((*_textureWeights)[0]+(*_textureWeights)[1]);

            {
                osg::TexEnvCombine* texenv = new osg::TexEnvCombine;
                texenv->setCombine_RGB(osg::TexEnvCombine::INTERPOLATE);
                texenv->setSource0_RGB(osg::TexEnvCombine::TEXTURE0+0);
                texenv->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
                texenv->setSource1_RGB(osg::TexEnvCombine::TEXTURE0+1);
                texenv->setOperand1_RGB(osg::TexEnvCombine::SRC_COLOR);
                texenv->setSource2_RGB(osg::TexEnvCombine::CONSTANT);
                texenv->setOperand2_RGB(osg::TexEnvCombine::SRC_COLOR);

                texenv->setConstantColor(osg::Vec4(a,a,a,a));

                stateset->setTextureAttribute(0, texenv);
            }

            {
                osg::TexEnvCombine* texenv = new osg::TexEnvCombine;
                texenv->setCombine_RGB(osg::TexEnvCombine::INTERPOLATE);
                texenv->setSource0_RGB(osg::TexEnvCombine::PREVIOUS);
                texenv->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
                texenv->setSource1_RGB(osg::TexEnvCombine::TEXTURE0+2);
                texenv->setOperand1_RGB(osg::TexEnvCombine::SRC_COLOR);
                texenv->setSource2_RGB(osg::TexEnvCombine::CONSTANT);
                texenv->setOperand2_RGB(osg::TexEnvCombine::SRC_COLOR);

                texenv->setConstantColor(osg::Vec4(b,b,b,b));

                stateset->setTextureAttribute(1, texenv);
            }

            {
                osg::TexEnvCombine* texenv = new osg::TexEnvCombine;
                texenv->setCombine_RGB(osg::TexEnvCombine::MODULATE);
                texenv->setSource0_RGB(osg::TexEnvCombine::PREVIOUS);
                texenv->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
                texenv->setSource1_RGB(osg::TexEnvCombine::PRIMARY_COLOR);
                texenv->setOperand1_RGB(osg::TexEnvCombine::SRC_COLOR);

                stateset->setTextureAttribute(2, texenv);
            }
        }
    }

    if (_useTextureWeightsUniform && _textureWeights->size()>0)
    {
        osg::ref_ptr<osg::Uniform> uniform = new osg::Uniform(osg::Uniform::FLOAT, "TextureWeights", _textureWeights->size());
#if 1
        uniform->setArray(_textureWeights.get());
#else
        for(unsigned int i=0; i<_textureWeights->size(); ++i)
        {
            uniform->setElement(i, (*_textureWeights)[i]);
        }
#endif
        stateset->addUniform(uniform.get());
        stateset->setDefine("TEXTURE_WEIGHTS");
    }

    setStateSet(stateset.get());
}

