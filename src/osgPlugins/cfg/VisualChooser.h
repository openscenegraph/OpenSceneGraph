/* -*-c++-*- Producer - Copyright (C) 2001-2004  Don Burns
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

#ifndef OSGPRODUCER_VISUAL_CHOOSER
#define OSGPRODUCER_VISUAL_CHOOSER 1

#include <vector>

#include <osg/Referenced>


namespace osgProducer {

class VisualChooser : public osg::Referenced
{
    public :
        VisualChooser( void );

        enum AttributeName {
            UseGL,
            BufferSize,
            Level,
            RGBA,
            DoubleBuffer,
            Stereo,
            AuxBuffers,
            RedSize,
            GreenSize,
            BlueSize,
            AlphaSize,
            DepthSize,
            StencilSize,
            AccumRedSize,
            AccumGreenSize,
            AccumBlueSize,
            AccumAlphaSize,
            Samples,
            SampleBuffers
        };
#if 0
        //-------------------------------------------------------------------------
        // Explicitly set the visual info pointer.  This will override the use of
        // glXChooseVisual().  Useful for functions requiring a VisualChooser
        // argument, but not a XVisualInfo.
        void setVisual( VisualInfo *vinfo );
#endif
        //-------------------------------------------------------------------------
        // Chooses a minimal set of parameters
        void setSimpleConfiguration(bool doublebuffer = true);

        //-------------------------------------------------------------------------
        // Clear the list of parameters
        void clear() ;

        //-------------------------------------------------------------------------
        // Generic method for adding an attribute without a parameter
        // (e.g DoubleBuffer )
        void addAttribute( AttributeName attribute );

        //-------------------------------------------------------------------------
        // Generic method for adding an attribute with a parameter
        // (e.g DepthSize, 1 )
        void addAttribute( AttributeName attribute, int parameter );

        //-------------------------------------------------------------------------
        // Generic method for adding an attribute without a parameter
        // (e.g DoubleBuffer )
        void addExtendedAttribute( unsigned int attribute );

        //-------------------------------------------------------------------------
        // Generic method for adding an extended attribute with a parameter
        // (e.g DepthSize, 1 )
        void addExtendedAttribute( unsigned int attribute, int parameter );

        //-------------------------------------------------------------------------
        // The following method returns whether double buffering is being used
        bool isDoubleBuffer() const;

        //-------------------------------------------------------------------------
        // The following methods set attributes explicitly
        //
        void setBufferSize( unsigned int size );

        void setLevel( int level );

        void useRGBA();

        void useDoubleBuffer();

        void useStereo();

        void setAuxBuffers( unsigned int num );

        void setRedSize( unsigned int size );

        void setGreenSize( unsigned int size );

        void setBlueSize( unsigned int size );

        void setAlphaSize( unsigned int size );

        void setDepthSize( unsigned int size );

        void setStencilSize( unsigned int size );

        void setAccumRedSize( unsigned int size );

        void setAccumGreenSize( unsigned int size );

        void setAccumBlueSize( unsigned int size );

        void setAccumAlphaSize( unsigned int size );

        void setSampleBuffers( unsigned int size );

        void setSamples( unsigned int size );

        void setVisualID( unsigned int id );


#if 0
        //-------------------------------------------------------------------------
        // Chooses visual based on previously selected attributes and parameters
        //     dpy              = Connection to Xserver as returned by XOpenDisplay()
        //     screen           = XServer screen (Could be DefaultScreen(dpy))
        //     strict_adherence = If true, return NULL visual info if the set of
        //                        parameters is not matched verbatim.  If set to
        //                        false, choose() will attempt to find a visual that
        //                        matches as much of the attribute list as possible
        //
        //                        Important Note : An attribute is removed from the end
        //                        of the list before each retry, implying that the
        //                        attribute list should be specified in priority order,
        //                        most important attriutes first.
        //

        VisualInfo *choose( Display *dpy, int screen, bool strict_adherence=false);

        unsigned int getVisualID() const;

        bool getStrictAdherence();
        void setStrictAdherence(bool);
#endif

    protected:
        ~VisualChooser(void);

    public :

        struct VisualAttribute
        {
                unsigned int  _attribute;
                bool _has_parameter;
                int  _parameter;
                bool _is_extension;

                VisualAttribute( AttributeName attribute, int parameter ) :
                _attribute(attribute),
                _has_parameter(true),
                _parameter(parameter),
                _is_extension(false) {}

                VisualAttribute( AttributeName attribute ) :
                _attribute(attribute),
                _has_parameter(false),
                _parameter(0),
                _is_extension(false) {}

                VisualAttribute( unsigned int attribute, int parameter ) :
                _attribute(attribute),
                _has_parameter(true),
                _parameter(parameter),
                _is_extension(true) {}

                VisualAttribute( unsigned int attribute ) :
                _attribute(attribute),
                _has_parameter(false),
                _parameter(0),
                _is_extension(true) {}

                unsigned int attribute() const   { return _attribute; }
                bool hasParameter() const        { return _has_parameter; }
                int  parameter() const           { return _parameter; }
                bool isExtension() const         { return _is_extension; }
          };

        void applyAttribute(const VisualAttribute &va, std::vector<int> &attribs);
        void resetVisualInfo();

        std::vector <VisualAttribute> _visual_attributes;
        unsigned int _visual_id;
        bool _strictAdherence;
};

}
#endif
