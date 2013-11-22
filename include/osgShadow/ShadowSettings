/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-20 Robert Osfield
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

#ifndef OSGSHADOW_SHADOWSETTINGS
#define OSGSHADOW_SHADOWSETTINGS 1

#include <osg/Uniform>
#include <osg/CullSettings>
#include <osgShadow/Export>

namespace osgShadow {

/** ShadowSettings provides the parameters that the ShadowTechnique should use as a guide for setting up shadowing.*/
class OSGSHADOW_EXPORT ShadowSettings : public osg::Object
{
    public:
        ShadowSettings();
        ShadowSettings(const ShadowSettings& ss, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Object(osgShadow, ShadowSettings);

        void setReceivesShadowTraversalMask(unsigned int mask) { _receivesShadowTraversalMask = mask; }
        unsigned int getReceivesShadowTraversalMask() const { return _receivesShadowTraversalMask; }

        void setCastsShadowTraversalMask(unsigned int mask) { _castsShadowTraversalMask = mask; }
        unsigned int getCastsShadowTraversalMask() const { return _castsShadowTraversalMask; }

        void setComputeNearFarModeOverride(osg::CullSettings::ComputeNearFarMode cnfn) { _computeNearFearModeOverride = cnfn; }
        osg::CullSettings::ComputeNearFarMode getComputeNearFarModeOverride() const { return _computeNearFearModeOverride; }


        /** Set the LightNum of the light in the scene to assign a shadow for.
          * Default value is -1, which signifies that shadow technique should automatically select an active light
          * to assign a shadow, typically this will be the first active light found. */
        void setLightNum(int lightNum) { _lightNum = lightNum; }
        int getLightNum() const { return _lightNum; }

        void setBaseShadowTextureUnit(unsigned int unit) { _baseShadowTextureUnit = unit; }
        unsigned int getBaseShadowTextureUnit() const { return _baseShadowTextureUnit; }

        /** Set whether to use osg::StateAttribute::OVERRIDE for the shadow map texture.
         * Enabling override will force the shadow map texture to override any texture set on the shadow maps texture unit.*/
        void setUseOverrideForShadowMapTexture(bool useOverride) { _useShadowMapTextureOverride = useOverride; }

        /** Get whether to use osg::StateAttribute::OVERRIDE for the shadow map texture. */
        bool getUseOverrideForShadowMapTexture() const { return _useShadowMapTextureOverride; }


        /** Set the size of the shadow map textures.*/
        void setTextureSize(const osg::Vec2s& textureSize) { _textureSize = textureSize; }

        /** Get the size of the shadow map textures.*/
        const osg::Vec2s& getTextureSize() const { return _textureSize; }

        void setMinimumShadowMapNearFarRatio(double ratio) { _minimumShadowMapNearFarRatio = ratio; }
        double getMinimumShadowMapNearFarRatio() const { return _minimumShadowMapNearFarRatio; }

        void setMaximumShadowMapDistance(double distance) { _maximumShadowMapDistance = distance; }
        double getMaximumShadowMapDistance() const { return _maximumShadowMapDistance; }


        enum ShadowMapProjectionHint
        {
            ORTHOGRAPHIC_SHADOW_MAP,
            PERSPECTIVE_SHADOW_MAP
        };

        void setShadowMapProjectionHint(ShadowMapProjectionHint hint) { _shadowMapProjectionHint = hint; }
        ShadowMapProjectionHint getShadowMapProjectionHint() const { return _shadowMapProjectionHint; }

        /** Set the cut off angle, in degrees, between the light direction and the view direction
         * that determines whether perspective shadow mapping is appropriate, or thar orthographic shadow
         * map should be used instead.  Default is 2 degrees so that for any angle greater than 2 degrees
         * perspective shadow map will be used, and any angle less than 2 degrees orthographic shadow map
         * will be used.  Note, if ShadowMapProjectionHint is set to ORTHOGRAPHIC_SHADOW_MAP then an
         * orthographic shadow map will always be used.*/
        void setPerspectiveShadowMapCutOffAngle(double angle) { _perspectiveShadowMapCutOffAngle = angle; }
        double getPerspectiveShadowMapCutOffAngle() const { return _perspectiveShadowMapCutOffAngle; }


        void setNumShadowMapsPerLight(unsigned int numShadowMaps) { _numShadowMapsPerLight = numShadowMaps; }
        unsigned int getNumShadowMapsPerLight() const { return _numShadowMapsPerLight; }

        enum MultipleShadowMapHint
        {
            PARALLEL_SPLIT,
            CASCADED
        };

        void setMultipleShadowMapHint(MultipleShadowMapHint hint) { _multipleShadowMapHint = hint; }
        MultipleShadowMapHint getMultipleShadowMapHint() const { return _multipleShadowMapHint; }


        enum ShaderHint
        {
            NO_SHADERS,
            PROVIDE_FRAGMENT_SHADER,
            PROVIDE_VERTEX_AND_FRAGMENT_SHADER
        };

        void setShaderHint(ShaderHint shaderHint) { _shaderHint = shaderHint; }
        ShaderHint getShaderHint() const { return _shaderHint; }

        void setDebugDraw(bool debugDraw) { _debugDraw = debugDraw; }
        bool getDebugDraw() const { return _debugDraw; }

    protected:

        virtual ~ShadowSettings();


        unsigned int            _receivesShadowTraversalMask;
        unsigned int            _castsShadowTraversalMask;

        osg::CullSettings::ComputeNearFarMode _computeNearFearModeOverride;

        int                     _lightNum;
        unsigned int            _baseShadowTextureUnit;
        bool                    _useShadowMapTextureOverride;
        osg::Vec2s              _textureSize;

        double                  _minimumShadowMapNearFarRatio;
        double                  _maximumShadowMapDistance;
        ShadowMapProjectionHint _shadowMapProjectionHint;
        double                  _perspectiveShadowMapCutOffAngle;

        unsigned int            _numShadowMapsPerLight;
        MultipleShadowMapHint   _multipleShadowMapHint;

        ShaderHint              _shaderHint;
        bool                    _debugDraw;

};

}

#endif
