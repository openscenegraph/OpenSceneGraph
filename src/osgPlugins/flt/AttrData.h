#ifndef __ATTR_DATA_H
#define __ATTR_DATA_H

#if defined(WIN32) && !defined(__CYGWIN__)
#pragma warning( disable : 4786 )
#endif

#include <osg/StateSet>
#include <osg/Referenced>

// Detail Texture as defined in Creator v2.5.1 got lot's of parameters that I don't know 
// how they work, so I have implemented only the things I need, and if someone needs 
// something else it's posible to use other parameters using the new flt::AttrData class
// Now the implemented parameters are:
//        - txDetail_m, txDetail_n : Control how new texcoord will be generated.
//        - modulateDetail         : If set to true, be use scale_rgb and scale_alpha values from 
//                                 osg::TexEnvCombine to make a lighted image.
//
// I implement detail texture as a new TexEnvCombine attribute in texture unit 1
// that is added to the stateset of the geometry.
//
// flt::AttrData class is used to store Texture Attribute Data as defined in Creator
// so we can pass this info to others loader object an use it where needed.
//
// Julian Ortiz, June 18th 2003.

namespace flt {

 typedef signed long     int32;

 class AttrData : public osg::Object { 
  public:
    osg::StateSet *stateset;

    int32   useDetail;              // TRUE if using next 5 integers for detail texture
    int32   txDetail_j;             // J argument for TX_DETAIL
    int32   txDetail_k;             // K argument for TX_DETAIL
    int32   txDetail_m;             // M argument for TX_DETAIL
    int32   txDetail_n;             // N argument for TX_DETAIL
    int32   txDetail_s;             // Scramble argument for TX_DETAIL
    bool    modulateDetail;            // True if Magnification filter type is MODULATE_DETAIL
    AttrData() {};
    AttrData(const AttrData& attr,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):osg::Object(attr,copyop) {};
    virtual osg::Object* cloneType() const { return new AttrData(); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new AttrData(*this,copyop); }    
    virtual const char* libraryName() const { return "osg"; }
    virtual const char* className() const { return "AttrData"; }
 };
}

#endif // __ATTR_DATA_H
