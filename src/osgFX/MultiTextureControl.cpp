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

MultiTextureControl::MultiTextureControl()
{
}

MultiTextureControl::MultiTextureControl(const MultiTextureControl& copy, const osg::CopyOp& copyop):
    Group(copy,copyop),
    _textureWeightList(copy._textureWeightList)
{
    updateStateSet();
}

void MultiTextureControl::setTextureWeight(unsigned int unit, float weight)
{
    if (unit >= _textureWeightList.size())
    {
        _textureWeightList.resize(unit+1,0.0f);
    }
    _textureWeightList[unit] = weight;
    
    updateStateSet();
}

void MultiTextureControl::updateStateSet()
{
    osg::StateSet* stateset = getOrCreateStateSet();
    
    stateset->clear();
    
    unsigned int numTextureUnitsOn = 0;
    unsigned int unit;
    for(unit=0;unit<_textureWeightList.size();++unit)
    {
        if (_textureWeightList[unit]>0.0f) ++numTextureUnitsOn;
    }
    
    if (numTextureUnitsOn<=1)
    {
        for(unit=0;unit<_textureWeightList.size();++unit)
        {
            if (_textureWeightList[unit]>0.0f) 
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
    else if (_textureWeightList.size()==2)
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

            float r = _textureWeightList[0]/(_textureWeightList[0]+_textureWeightList[1]);
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
    else if (_textureWeightList.size()==3)
    {
        float b = (_textureWeightList[0]+_textureWeightList[1])/(_textureWeightList[0]+_textureWeightList[1]+_textureWeightList[2]);
        float a = _textureWeightList[0]/(_textureWeightList[0]+_textureWeightList[1]);
        
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

    
/*    
    bool firstActiveTextureUnit = true;
    for(unsigned int unit = 0;
        unit < _textureWeightList.size();
        ++unit)
    {
        float r = _textureWeightList[unit];
        
        if (r==0.0f)
        {
            stateset->setTextureMode(unit, GL_TEXTURE_2D, osg::StateAttribute::OFF);
        }
        else 
        {        
            stateset->setTextureMode(unit, GL_TEXTURE_2D, osg::StateAttribute::ON);
            
            if (firstActiveTextureUnit)
            {   
                stateset->setTextureAttribute(unit, new osg::TexEnv);
                firstActiveTextureUnit = false;
            }
            else
            {




                osg::TexEnvCombine* texenv = new osg::TexEnvCombine;
                texenv->setCombine_RGB(osg::TexEnvCombine::INTERPOLATE);
                texenv->setSource0_RGB(osg::TexEnvCombine::PREVIOUS);
                texenv->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
                texenv->setSource1_RGB(osg::TexEnvCombine::TEXTURE);
                texenv->setOperand1_RGB(osg::TexEnvCombine::SRC_COLOR);
                texenv->setSource2_RGB(osg::TexEnvCombine::CONSTANT);
                texenv->setOperand2_RGB(osg::TexEnvCombine::SRC_COLOR);
                
                texenv->setConstantColor(osg::Vec4(r,r,r,r));
 
                stateset->setTextureAttribute(unit, texenv);
            }
            
        }
            
        
    }
*/    
}

