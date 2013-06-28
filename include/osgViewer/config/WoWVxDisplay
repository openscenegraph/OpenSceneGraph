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

#ifndef OSGVIEWER_WoWVxDisplay
#define OSGVIEWER_WoWVxDisplay 1

#include <osgViewer/View>

namespace osgViewer {

/** autostereoscopic Philips WoWvx display.*/
class OSGVIEWER_EXPORT WoWVxDisplay : public ViewConfig
{
    public:
        
        // default to 20" display, type can be 20 to 42.
        WoWVxDisplay(unsigned int type=20, unsigned int screenNum=0):
            _screenNum(screenNum),
            _wow_content(0x02),
            _wow_factor(0x40),
            _wow_offset(0x80),
            _wow_disparity_Zd(0.459813f),
            _wow_disparity_vz(6.180772f),
            _wow_disparity_M(-1586.34f),
            _wow_disparity_C(127.5f) { if (type==42) WoWVx42(); }

        WoWVxDisplay(unsigned int screenNum, unsigned char wow_content, unsigned char wow_factor, unsigned char wow_offset, float wow_disparity_Zd, float wow_disparity_vz, float wow_disparity_M, float wow_disparity_C):
            _screenNum(screenNum),
            _wow_content(wow_content),
            _wow_factor(wow_factor),
            _wow_offset(wow_offset),
            _wow_disparity_Zd(wow_disparity_Zd),
            _wow_disparity_vz(wow_disparity_vz),
            _wow_disparity_M(wow_disparity_M),
            _wow_disparity_C(wow_disparity_C) {}
            
        WoWVxDisplay(const WoWVxDisplay& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
            ViewConfig(rhs, copyop),
            _screenNum(rhs._screenNum),
            _wow_content(rhs._wow_content),
            _wow_factor(rhs._wow_factor),
            _wow_offset(rhs._wow_offset),
            _wow_disparity_Zd(rhs._wow_disparity_Zd),
            _wow_disparity_vz(rhs._wow_disparity_vz),
            _wow_disparity_M(rhs._wow_disparity_M),
            _wow_disparity_C(rhs._wow_disparity_C) {}
        
        META_Object(osgViewer, WoWVxDisplay);
        
        virtual void configure(osgViewer::View& view) const;
        
        void setScreenNum(unsigned int n) { _screenNum = n; }
        unsigned int getScreenNum() const { return _screenNum; }
        
        void WoWVx20()
        {
            _wow_disparity_Zd = 0.459813f;
            _wow_disparity_vz = 6.180772f;
            _wow_disparity_M = -1586.34f;
            _wow_disparity_C = 127.5f;
        }

        void WoWVx42()
        {
            _wow_disparity_Zd = 0.467481f;
            _wow_disparity_vz = 7.655192f;
            _wow_disparity_M = -1960.37f;
            _wow_disparity_C = 127.5f;
        }

        void setContent(unsigned char c) { _wow_content = c; }
        double getContent() const { return _wow_content; }

        void setFactor(unsigned char c) { _wow_factor = c; }
        double getFactor() const { return _wow_factor; }

        void setOffset(unsigned char c) { _wow_offset = c; }
        double getOffset() const { return _wow_offset; }

        void setDisparityZD(float c) { _wow_disparity_Zd = c; }
        float getDisparityZD() const { return _wow_disparity_Zd; }
        
        void setDisparityVZ(float c) { _wow_disparity_vz = c; }
        float getDisparityVZ() const { return _wow_disparity_vz; }
        
        void setDisparityM(float c) { _wow_disparity_M = c; }
        float getDisparityM() const { return _wow_disparity_M; }
        
        void setDisparityC(float c) { _wow_disparity_C = c; }
        float getDisparityC() const { return _wow_disparity_C; }
        
    protected:
    
        unsigned int _screenNum;
        unsigned char _wow_content;
        unsigned char _wow_factor;
        unsigned char _wow_offset;
        float _wow_disparity_Zd;
        float _wow_disparity_vz;
        float _wow_disparity_M;
        float _wow_disparity_C;
};
        
#if 0
/** Configure view with DepthPartition.*/
class OSGVIEWER_EXPORT DepthPartition : public Config
{
    public:
        
        DepthPartition(DepthPartitionSettings* dsp=0);

        DepthPartition(const ViewForWoWVxDisplay& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
        
        META_Object(osgViewer,DepthPartition);

        void setDepthPartionSettings(DepthPartitionSettings* dsp) const { _dps = dps; }
        const DepthPartitionSettings* getDepthPartionSettings() const { return _dps; }

        /** for setting up depth partitioning on the specified camera.*/
        bool setUpDepthPartitionForCamera(osg::Camera* cameraToPartition, DepthPartitionSettings* dps=0);

        virtual void configure(osgViewer::View& view) const;
        
    protected:
};

#endif

}


#endif
