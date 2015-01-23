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

#ifndef OSGFX_MULTITEXTURECONTROL
#define OSGFX_MULTITEXTURECONTROL

#include <osg/Group>

#include <osgFX/Export>

namespace osgFX
{
    /**
      This node provides control over the which texture units are active and the
      blending weighting between them.
     */
    class OSGFX_EXPORT MultiTextureControl: public osg::Group {
    public:

        MultiTextureControl();
        MultiTextureControl(const MultiTextureControl& copy, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

        META_Node(osgFX, MultiTextureControl);

        typedef osg::FloatArray TextureWeights;

        void setTextureWeights(TextureWeights* twl) { _textureWeights = twl; }
        TextureWeights* getTextureWeights() { return _textureWeights.get(); }
        const TextureWeights* getTextureWeights() const { return _textureWeights.get(); }

        void setTextureWeight(unsigned int unit, float weight);
        float getTextureWeight(unsigned int unit) const { return (unit<_textureWeights->size()) ?  (*_textureWeights)[unit] : 0.0f; }
        unsigned int getNumTextureWeights() const { return _textureWeights->size(); }

        void setUseTexEnvCombine(bool flag) { _useTexEnvCombine = flag; }
        bool getUseTexEnvCombine() const { return _useTexEnvCombine; }

        void setUseTextureWeightsUniform(bool flag) { _useTextureWeightsUniform = flag; }
        bool getUseTextureWeightsUniform() const { return _useTextureWeightsUniform; }

    protected:

        virtual ~MultiTextureControl() {}
        MultiTextureControl& operator = (const MultiTextureControl&) { return *this; }

        void updateStateSet();

        osg::ref_ptr<TextureWeights> _textureWeights;

        bool _useTexEnvCombine;
        bool _useTextureWeightsUniform;

    };

}

#endif
