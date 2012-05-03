/* ************************
   Copyright Terrain Experts Inc.
   Terrain Experts Inc (TERREX) reserves all rights to this source code
   unless otherwise specified in writing by the President of TERREX.
   This copyright may be updated in the future, in which case that version
   supercedes this one.
   -------------------
   Terrex Experts Inc.
   4400 East Broadway #314
   Tucson, AZ  85711
   info@terrex.com
   Tel: (520) 323-7990
   ************************
   */

/* trpage_light.cpp
    Methods for the trpgLightAttr, trpgLight and trpgLightTable classes.
    This includes read and write methods.
    You should only need to change something in here if you want to modify
     what any of the classes contains.
    */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <trpage_geom.h>
#include <trpage_read.h>

#if defined(_WIN32)
#define ALIGNMENT_WORKAROUND    false
#else
#define ALIGNMENT_WORKAROUND    true
#endif

/******
    Lights Attribute
******/

trpgLightAttr::trpgLightAttr(void)
{
    data.commentStr = NULL;
    Reset();
}

trpgLightAttr::trpgLightAttr(const trpgLightAttr& in):
    trpgReadWriteable(in)
{
    data.commentStr = NULL;
    operator=(in);
}

trpgLightAttr::~trpgLightAttr(void)
{
    if (data.commentStr)
        delete [] data.commentStr;
    data.commentStr = NULL;
}


// Setters
void trpgLightAttr::SetType( trpgLightAttr::LightType in_type )
{
    data.type = in_type;
}
void trpgLightAttr::SetDirectionality( trpgLightAttr::LightDirectionality in_directionality )
{
    data.directionality = in_directionality;
}
void trpgLightAttr::SetFrontColor( trpgColor in_frontColor )
{
    data.frontColor = in_frontColor;
}
void trpgLightAttr::SetFrontIntensity( float64 in_frontIntensity )
{
    data.frontIntensity = in_frontIntensity;
}
void trpgLightAttr::SetBackColor( trpgColor in_backColor )
{
    data.backColor = in_backColor;
}
void trpgLightAttr::SetBackIntensity( float64 in_backIntensity )
{
    data.backIntensity = in_backIntensity;
}
void trpgLightAttr::SetNormal( trpg3dPoint in_normal )
{
    data.normal = in_normal;
}
void trpgLightAttr::SetSMC( int32 in_smc )
{
    data.smc = in_smc;
}
void trpgLightAttr::SetFID( int32 in_fid )
{
    data.fid = in_fid;
}
void trpgLightAttr::SetFlags( int32 in_flags )
{
    data.flags = in_flags;
}
void trpgLightAttr::SetHLobeAngle( float64 in_hLobeAngle )
{
    data.horizontalLobeAngle = in_hLobeAngle;
}
void trpgLightAttr::SetVLobeAngle( float64 in_vLobeAngle )
{
    data.verticalLobeAngle = in_vLobeAngle;
}
void trpgLightAttr::SetLobeRollAngle( float64 in_lobeRollAngle )
{
    data.lobeRollAngle = in_lobeRollAngle;
}
void trpgLightAttr::SetLobeFalloff( float64 in_lobeFalloff )
{
    data.lobeFalloff = in_lobeFalloff;
}
void trpgLightAttr::SetAmbient( float64 in_ambientIntensity )
{
    data.ambientIntensity = in_ambientIntensity;
}
void trpgLightAttr::SetQuality( trpgLightAttr::LightQuality in_quality )
{
    data.quality = in_quality;
}
void trpgLightAttr::SetRascalSignificance( float64 in_rascalSignificance )
{
    data.rascalSignificance = in_rascalSignificance;
}
void trpgLightAttr::SetRandomIntensity( trpgLightAttr::LightQuality in_randomIntensity )
{
    data.randomIntensity = in_randomIntensity;
}
void trpgLightAttr::SetCalligraphicAttr( trpgLightAttr::CalligraphicAttr& in_calligraphicAttr )
{
    data.calligraphicAttr = in_calligraphicAttr;
}
void trpgLightAttr::SetCalligraphicDrawOrder( int32 in_drawOrder )
{
    data.calligraphicAttr.drawOrder = in_drawOrder;
}
void trpgLightAttr::SetCalligraphicMinDefocus( float64 in_minDefocus )
{
    data.calligraphicAttr.minDefocus = in_minDefocus;
}
void trpgLightAttr::SetCalligraphicMaxDefocus( float64 in_maxDefocus )
{
    data.calligraphicAttr.maxDefocus = in_maxDefocus;
}
void trpgLightAttr::SetPerformerAttr( trpgLightAttr::PerformerAttr& in_performerAttr )
{
    data.performerAttr = in_performerAttr;
}
void trpgLightAttr::SetPerformerFlags( int32 in_flags )
{
    data.performerAttr.flags = in_flags & trpgLightAttr::trpg_PerformerMask;
    data.flags |= data.performerAttr.flags;
}
void trpgLightAttr::SetPerformerMinPixelSize( float64 in_minPxSize )
{
    data.performerAttr.minPixelSize = in_minPxSize;
}
void trpgLightAttr::SetPerformerMaxPixelSize( float64 in_maxPxSize )
{
    data.performerAttr.maxPixelSize = in_maxPxSize;
}
void trpgLightAttr::SetPerformerActualSize( float64 in_actualSize )
{
    data.performerAttr.actualSize = in_actualSize;
}
void trpgLightAttr::SetPerformerTpPixelSize( float64 in_tpPixelSize )
{
    data.performerAttr.transparentPixelSize = in_tpPixelSize;
}
void trpgLightAttr::SetPerformerTpFalloffExp( float64 in_tpFalloffExp )
{
    data.performerAttr.transparentFallofExp = in_tpFalloffExp;
}
void trpgLightAttr::SetPerformerTpScale( float64 in_tpScale )
{
    data.performerAttr.transparentScale = in_tpScale;
}
void trpgLightAttr::SetPerformerTpClamp( float64 in_tpClamp )
{
    data.performerAttr.transparentClamp = in_tpClamp;
}
void trpgLightAttr::SetPerformerFogScale( float64 in_fogScale )
{
    data.performerAttr.fogScale = in_fogScale;
}
void trpgLightAttr::SetAnimationAttr( trpgLightAttr::AnimationAttr& in_animationAttr )
{
    data.animationAttr = in_animationAttr;
}
void trpgLightAttr::SetAnimationPeriod( float64 in_period )
{
    data.animationAttr.period = in_period;
}
void trpgLightAttr::SetAnimationPhaseDelay( float64 in_phaseDelay )
{
    data.animationAttr.phaseDelay = in_phaseDelay;
}
void trpgLightAttr::SetAnimationTimeOn( float64 in_timeOn )
{
    data.animationAttr.timeOn = in_timeOn;
}
void trpgLightAttr::SetAnimationVector( trpg3dPoint in_vector )
{
    data.animationAttr.vector = in_vector;
}
void trpgLightAttr::SetAnimationFlags( int32 flags )
{
    data.animationAttr.flags = flags & trpgLightAttr::trpg_AnimationMask;
    data.flags |= data.animationAttr.flags;
}
void trpgLightAttr::SetComment(const char *inStr)
{
    if (!inStr)
        return;

    if (data.commentStr)
        delete [] data.commentStr;
    data.commentStr = new char[strlen(inStr)+1];
    strcpy(data.commentStr,inStr);
}
// Getters
void trpgLightAttr::GetType( trpgLightAttr::LightType& out_type )
{
    out_type = data.type;
}
void trpgLightAttr::GetDirectionality( trpgLightAttr::LightDirectionality& out_directionality )
{
    out_directionality = data.directionality;
}
void trpgLightAttr::GetFrontColor( trpgColor& out_frontColor )
{
    out_frontColor = data.frontColor;
}
void trpgLightAttr::GetFrontIntensity( float64& out_frontIntensity )
{
    out_frontIntensity = data.frontIntensity;
}
void trpgLightAttr::GetBackColor( trpgColor& out_backColor )
{
    out_backColor = data.backColor;
}
void trpgLightAttr::GetBackIntensity( float64& out_backIntensity )
{
    out_backIntensity = data.backIntensity;
}
void trpgLightAttr::GetNormal( trpg3dPoint& out_normal )
{
    out_normal = data.normal;
}
void trpgLightAttr::GetSMC( int32& out_smc )
{
    out_smc = data.smc;
}
void trpgLightAttr::GetFID( int32& out_fid )
{
    out_fid = data.fid;
}
void trpgLightAttr::GetFlags( int32& out_flags )
{
    out_flags = data.flags;
}
void trpgLightAttr::GetHLobeAngle( float64& out_hLobeAngle )
{
    out_hLobeAngle = data.horizontalLobeAngle;
}
void trpgLightAttr::GetVLobeAngle( float64& out_vLobeAngle )
{
    out_vLobeAngle = data.verticalLobeAngle;
}
void trpgLightAttr::GetLobeRollAngle( float64& out_lobeRollAngle )
{
    out_lobeRollAngle = data.lobeRollAngle;
}
void trpgLightAttr::GetLobeFalloff( float64& out_lobeFalloff )
{
    out_lobeFalloff = data.lobeFalloff;
}
void trpgLightAttr::GetAmbient( float64& out_ambientIntensity )
{
    out_ambientIntensity = data.ambientIntensity;
}
void trpgLightAttr::GetQuality( trpgLightAttr::LightQuality& out_quality )
{
    out_quality = data.quality;
}
void trpgLightAttr::GetRascalSignificance( float64& out_rascalSignificance )
{
    out_rascalSignificance = data.rascalSignificance;
}
void trpgLightAttr::GetRandomIntensity( trpgLightAttr::LightQuality& out_randomIntensity )
{
    out_randomIntensity = data.randomIntensity;
}
void trpgLightAttr::GetCalligraphicAttr( trpgLightAttr::CalligraphicAttr& out_calligraphicAttr )
{
    out_calligraphicAttr = data.calligraphicAttr;
}
void trpgLightAttr::GetCalligraphicDrawOrder( int32& out_drawOrder )
{
    out_drawOrder = data.calligraphicAttr.drawOrder;
}
void trpgLightAttr::GetCalligraphicMinDefocus( float64& out_minDefocus )
{
    out_minDefocus = data.calligraphicAttr.minDefocus;
}
void trpgLightAttr::GetCalligraphicMaxDefocus( float64& out_maxDefocus )
{
    out_maxDefocus = data.calligraphicAttr.maxDefocus;
}
void trpgLightAttr::GetPerformerAttr( trpgLightAttr::PerformerAttr& out_performerAttr )
{
    out_performerAttr = data.performerAttr;
}
void trpgLightAttr::GetPerformerFlags( int32& out_flags )
{
    out_flags = data.performerAttr.flags;
}
void trpgLightAttr::GetPerformerMinPixelSize( float64& out_minPxSize )
{
    out_minPxSize = data.performerAttr.minPixelSize;
}
void trpgLightAttr::GetPerformerMaxPixelSize( float64& out_maxPxSize )
{
    out_maxPxSize = data.performerAttr.maxPixelSize;
}
void trpgLightAttr::GetPerformerActualSize( float64& out_actualSize )
{
    out_actualSize = data.performerAttr.actualSize;
}
void trpgLightAttr::GetPerformerTpPixelSize( float64& out_tpPixelSize )
{
    out_tpPixelSize = data.performerAttr.transparentPixelSize;
}
void trpgLightAttr::GetPerformerTpFalloffExp( float64& out_tpFalloffExp )
{
    out_tpFalloffExp = data.performerAttr.transparentFallofExp;
}
void trpgLightAttr::GetPerformerTpScale( float64& out_tpScale )
{
    out_tpScale = data.performerAttr.transparentScale;
}
void trpgLightAttr::GetPerformerTpClamp( float64& out_tpClamp )
{
    out_tpClamp = data.performerAttr.transparentClamp;
}
void trpgLightAttr::GetPerformerFogScale( float64& out_fogScale )
{
    out_fogScale = data.performerAttr.fogScale;
}
void trpgLightAttr::GetAnimationAttr( trpgLightAttr::AnimationAttr& out_animationAttr )
{
    out_animationAttr = data.animationAttr;
}
void trpgLightAttr::GetAnimationPeriod( float64& out_period )
{
    out_period = data.animationAttr.period;
}
void trpgLightAttr::GetAnimationPhaseDelay( float64& out_phaseDelay )
{
    out_phaseDelay = data.animationAttr.phaseDelay;
}
void trpgLightAttr::GetAnimationTimeOn( float64& out_timeOn )
{
    out_timeOn = data.animationAttr.timeOn;
}
void trpgLightAttr::GetAnimationVector( trpg3dPoint& out_vector )
{
    out_vector = data.animationAttr.vector;
}
void trpgLightAttr::GetAnimationFlags( int32& flags )
{
    flags = data.animationAttr.flags;
}
const char *trpgLightAttr::GetComment()
{
    return data.commentStr;
}
// Writes this class to a write buffer
bool trpgLightAttr::Write(trpgWriteBuffer &buf)
{
    buf.Begin(TRPGLIGHTATTR);

    buf.Begin(TRPGLIGHTATTR_BASIC);
    buf.Add((int)data.type);
    buf.Add((int)data.directionality);
    buf.Add(data.frontColor);
    buf.Add(data.frontIntensity);
    buf.Add(data.backColor);
    buf.Add(data.backIntensity);
    buf.Add(data.normal);
    buf.Add(data.smc);
    buf.Add(data.fid);
    buf.Add(data.flags);
    buf.Add(data.horizontalLobeAngle);
    buf.Add(data.verticalLobeAngle);
    buf.Add(data.lobeRollAngle);
    buf.Add(data.lobeFalloff);
    buf.Add(data.ambientIntensity);
    buf.Add((int)data.quality);
    buf.Add((int)data.randomIntensity);
    buf.End();

    buf.Begin(TRPGLIGHTATTR_RASCAL);
    buf.Add(data.rascalSignificance);
    buf.End();

    buf.Begin(TRPGLIGHTATTR_CALLIGRAPHIC);
    buf.Add(data.calligraphicAttr.drawOrder);
    buf.Add(data.calligraphicAttr.minDefocus);
    buf.Add(data.calligraphicAttr.maxDefocus);
    buf.End();

    buf.Begin(TRPGLIGHTATTR_PERFORMER);
    buf.Add(data.performerAttr.actualSize);
    buf.Add(data.performerAttr.fogScale);
    buf.Add(data.performerAttr.minPixelSize);
    buf.Add(data.performerAttr.maxPixelSize);
    buf.Add(data.performerAttr.transparentClamp);
    buf.Add(data.performerAttr.transparentFallofExp);
    buf.Add(data.performerAttr.transparentPixelSize);
    buf.Add(data.performerAttr.transparentScale);
    buf.End();

    buf.Begin(TRPGLIGHTATTR_ANIMATION);
    buf.Add(data.animationAttr.period);
    buf.Add(data.animationAttr.phaseDelay);
    buf.Add(data.animationAttr.timeOn);
    buf.Add(data.animationAttr.vector);
    buf.End();

    if (data.commentStr) {
        buf.Begin(TRPGLIGHTATTR_COMMENT);
        buf.Add(data.commentStr);
        buf.End();
    }

    if(writeHandle) {
        buf.Begin(TRPGLIGHTATTR_HANDLE);
        buf.Add((int)handle);
        buf.End();
    }
    buf.End();

    return true;
}

/* LightAttr CB
    Used to parse tokens for a light attribute.
    */
class lightAttrCB : public trpgr_Callback {
public:
    void * Parse(trpgToken,trpgReadBuffer &);
    trpgLightAttr *lightAttr;
};
void * lightAttrCB::Parse(trpgToken tok,trpgReadBuffer &buf)
{
    int            type_data;
    int            directionality_data;
    trpgColor    color_data;;
    float64        float64_data;
    trpg3dPoint    point_data;;
    int32        int32_data;
    int            quality_data;
    char        commentStr[1024];

    try {
        switch (tok) {
        case TRPGLIGHTATTR_BASIC:
            buf.Get(type_data);
            lightAttr->SetType((trpgLightAttr::LightType)type_data);
            buf.Get(directionality_data);
            lightAttr->SetDirectionality((trpgLightAttr::LightDirectionality)directionality_data);
            buf.Get(color_data);
            lightAttr->SetFrontColor(color_data);
            buf.Get(float64_data);
            lightAttr->SetFrontIntensity(float64_data);
            buf.Get(color_data);
            lightAttr->SetBackColor(color_data);
            buf.Get(float64_data);
            lightAttr->SetBackIntensity(float64_data);
            buf.Get(point_data);
            lightAttr->SetNormal(point_data);
            buf.Get(int32_data);
            lightAttr->SetSMC(int32_data);
            buf.Get(int32_data);
            lightAttr->SetFID(int32_data);
            buf.Get(int32_data);
            lightAttr->SetFlags(int32_data);
            lightAttr->SetPerformerFlags(int32_data);
            lightAttr->SetAnimationFlags(int32_data);
            buf.Get(float64_data);
            lightAttr->SetHLobeAngle(float64_data);
            buf.Get(float64_data);
            lightAttr->SetVLobeAngle(float64_data);
            buf.Get(float64_data);
            lightAttr->SetLobeRollAngle(float64_data);
            buf.Get(float64_data);
            lightAttr->SetLobeFalloff(float64_data);
            buf.Get(float64_data);
            lightAttr->SetAmbient(float64_data);
            buf.Get(quality_data);
            lightAttr->SetQuality((trpgLightAttr::LightQuality)quality_data);
            buf.Get(quality_data);
            lightAttr->SetRandomIntensity((trpgLightAttr::LightQuality)quality_data);
            break;
        case TRPGLIGHTATTR_RASCAL:
            buf.Get(float64_data);
            lightAttr->SetRascalSignificance(float64_data);
            break;
        case TRPGLIGHTATTR_PERFORMER:
            buf.Get(float64_data);
            lightAttr->SetPerformerActualSize(float64_data);
            buf.Get(float64_data);
            lightAttr->SetPerformerFogScale(float64_data);
            buf.Get(float64_data);
            lightAttr->SetPerformerMinPixelSize(float64_data);
            buf.Get(float64_data);
            lightAttr->SetPerformerMaxPixelSize(float64_data);
            buf.Get(float64_data);
            lightAttr->SetPerformerTpClamp(float64_data);
            buf.Get(float64_data);
            lightAttr->SetPerformerTpFalloffExp(float64_data);
            buf.Get(float64_data);
            lightAttr->SetPerformerTpPixelSize(float64_data);
            buf.Get(float64_data);
            lightAttr->SetPerformerTpScale(float64_data);
            break;
        case TRPGLIGHTATTR_CALLIGRAPHIC:
            buf.Get(int32_data);
            lightAttr->SetCalligraphicDrawOrder(int32_data);
            buf.Get(float64_data);
            lightAttr->SetCalligraphicMinDefocus(float64_data);
            buf.Get(float64_data);
            lightAttr->SetCalligraphicMaxDefocus(float64_data);
            break;
        case TRPGLIGHTATTR_ANIMATION:
            buf.Get(float64_data);
            lightAttr->SetAnimationPeriod(float64_data);
            buf.Get(float64_data);
            lightAttr->SetAnimationPhaseDelay(float64_data);
            buf.Get(float64_data);
            lightAttr->SetAnimationTimeOn(float64_data);
            buf.Get(point_data);
            lightAttr->SetAnimationVector(point_data);
            break;
        case TRPGLIGHTATTR_COMMENT:
            buf.Get(commentStr,1024);
            lightAttr->SetComment(commentStr);
            break;
        case TRPGLIGHTATTR_HANDLE:
            int hdl;
            buf.Get(hdl);
            lightAttr->SetHandle(hdl);
            break;
        default:
            break;
        }
    }
    catch (...) {
        return NULL;
    }

    return lightAttr;
}

// Reads this class from a read buffer
bool trpgLightAttr::Read(trpgReadBuffer &buf)
{
    Reset();

    trpgr_Parser parse;
    lightAttrCB lightAttrCb;

    // Light attribute is just a bunch of unordered tokens.
    // Interface to it with a generic parser
    lightAttrCb.lightAttr = this;
    parse.AddCallback(TRPGLIGHTATTR_BASIC,&lightAttrCb,false);
    parse.AddCallback(TRPGLIGHTATTR_PERFORMER,&lightAttrCb,false);
    parse.AddCallback(TRPGLIGHTATTR_RASCAL,&lightAttrCb,false);
    parse.AddCallback(TRPGLIGHTATTR_CALLIGRAPHIC,&lightAttrCb,false);
    parse.AddCallback(TRPGLIGHTATTR_ANIMATION,&lightAttrCb,false);
    parse.AddCallback(TRPGLIGHTATTR_COMMENT,&lightAttrCb,false);
    parse.AddCallback(TRPGLIGHTATTR_HANDLE,&lightAttrCb,false);
    parse.Parse(buf);

    return isValid();
}

bool trpgLightAttr::isValid(void) const
{
    return true;
}

trpgLightAttr& trpgLightAttr::operator = (const trpgLightAttr& in)
{
    data = in.data;
    if (in.data.commentStr) {
        data.commentStr = new char[strlen(in.data.commentStr)+1];
        strcpy(data.commentStr,in.data.commentStr);
    }
    handle = in.handle;
    writeHandle = in.writeHandle;
    return *this;
}

bool trpgLightAttr::operator == (const trpgLightAttr& in)
{
    // this doesn't work, so do it a hard way
    // return memcmp( &data,&in.data,sizeof(data) ) == 0;

    if ( data.type != in.data.type )
        return false;
    if ( data.directionality != in.data.directionality )
        return false;
    if ( data.frontColor != in.data.frontColor )
        return false;
    if ( data.frontIntensity != in.data.frontIntensity )
        return false;
    if ( data.backColor != in.data.backColor )
        return false;
    if ( data.backIntensity != in.data.backIntensity )
        return false;
    if ( data.normal != in.data.normal )
        return false;
    if ( data.smc != in.data.smc )
        return false;
    if ( data.fid != in.data.fid )
        return false;
    if ( data.flags != in.data.flags )
        return false;
    if ( data.horizontalLobeAngle != in.data.horizontalLobeAngle )
        return false;
    if ( data.verticalLobeAngle != in.data.verticalLobeAngle )
        return false;
    if ( data.lobeRollAngle != in.data.lobeRollAngle )
        return false;
    if ( data.lobeFalloff != in.data.lobeFalloff )
        return false;
    if ( data.ambientIntensity != in.data.ambientIntensity )
        return false;
    if ( data.quality != in.data.quality )
        return false;
    if ( data.randomIntensity != in.data.randomIntensity )
        return false;
    if ( data.rascalSignificance != in.data.rascalSignificance )
        return false;
    if ( data.calligraphicAttr.drawOrder != in.data.calligraphicAttr.drawOrder )
        return false;
    if ( data.calligraphicAttr.minDefocus != in.data.calligraphicAttr.minDefocus )
        return false;
    if ( data.calligraphicAttr.maxDefocus != in.data.calligraphicAttr.maxDefocus )
        return false;
    if ( data.performerAttr.flags != in.data.performerAttr.flags )
        return false;
    if ( data.performerAttr.minPixelSize != in.data.performerAttr.minPixelSize )
        return false;
    if ( data.performerAttr.maxPixelSize != in.data.performerAttr.maxPixelSize )
        return false;
    if ( data.performerAttr.actualSize != in.data.performerAttr.actualSize )
        return false;
    if ( data.performerAttr.transparentPixelSize != in.data.performerAttr.transparentPixelSize )
        return false;
    if ( data.performerAttr.transparentFallofExp != in.data.performerAttr.transparentFallofExp )
        return false;
    if ( data.performerAttr.transparentScale != in.data.performerAttr.transparentScale )
        return false;
    if ( data.performerAttr.transparentClamp != in.data.performerAttr.transparentClamp )
        return false;
    if ( data.performerAttr.fogScale != in.data.performerAttr.fogScale )
        return false;
    if ( data.animationAttr.period != in.data.animationAttr.period )
        return false;
    if ( data.animationAttr.phaseDelay != in.data.animationAttr.phaseDelay )
        return false;
    if ( data.animationAttr.timeOn != in.data.animationAttr.timeOn )
        return false;
    if ( data.animationAttr.vector != in.data.animationAttr.vector )
        return false;
    if ( data.animationAttr.flags != in.data.animationAttr.flags )
        return false;
    if ( (data.commentStr && !in.data.commentStr) ||
         (!data.commentStr && in.data.commentStr))
         return false;
    if (data.commentStr && in.data.commentStr && strcmp(data.commentStr,in.data.commentStr))
        return false;
    if (handle != in.handle)
        return false;
    if (writeHandle != in.writeHandle)
        return false;
    return true;
}

bool trpgLightAttr::operator != (const trpgLightAttr& in)
{
    return !operator==(in);
}

void trpgLightAttr::Reset(void)
{
    errMess[0] = '\0';
    data.type = trpg_Raster;
    data.directionality = trpg_Omnidirectional;
    data.frontColor = trpgColor(0,0,0);
    data.frontIntensity = 0;
    data.backColor = trpgColor(0,0,0);
    data.backIntensity = 0;
    data.normal = trpg3dPoint(0,0,1);
    data.smc = 0;
    data.fid = 0;
    data.flags = 0;
    data.horizontalLobeAngle = 0;
    data.verticalLobeAngle = 0;
    data.lobeRollAngle = 0;
    data.lobeFalloff = 0;
    data.ambientIntensity = 0;
    data.quality = trpg_Low;
    data.randomIntensity = trpg_Low;
    data.rascalSignificance = 0;
    data.calligraphicAttr.drawOrder = 0;
    data.calligraphicAttr.minDefocus = 0;
    data.calligraphicAttr.maxDefocus = 0;
    data.performerAttr.flags = 0;
    data.performerAttr.minPixelSize = 0;
    data.performerAttr.maxPixelSize = 0;
    data.performerAttr.actualSize = 0;
    data.performerAttr.transparentPixelSize = 0;
    data.performerAttr.transparentFallofExp = 0;
    data.performerAttr.transparentScale = 0;
    data.performerAttr.transparentClamp = 0;
    data.performerAttr.fogScale = 0;
    data.animationAttr.period = 0;
    data.animationAttr.phaseDelay = 0;
    data.animationAttr.timeOn = 0;
    data.animationAttr.vector = trpg3dPoint(0,0,1);
    data.animationAttr.flags = 0;
    if (data.commentStr)
        delete [] data.commentStr;
    data.commentStr = NULL;
    handle = -1;
    writeHandle = false;
}

/**************
    Light
*/

trpgLight::trpgLight(void)
{
    index = -1;
}

trpgLight::trpgLight(const trpgLight &in):
    trpgReadWriteable(in)
{
    operator=(in);
}

trpgLight::~trpgLight(void)
{
    Reset();
}

// Set the index pointing into the Light Table
void trpgLight::SetAttrIndex(int ix)
{
    index = ix;
}

// Add a new location this light is located at
void trpgLight::AddVertex(trpg3dPoint pt)
{
    lightPoints.push_back(pt);
}

// Returns the number of locations, this light is located at
void trpgLight::GetNumVertices(uint32 &nvertices) const
{
    nvertices = lightPoints.size();
}

// Returns the location at a given index
bool trpgLight::GetVertex(uint32 ix, trpg3dPoint &pt) const
{
    if (ix < lightPoints.size() ) {
        pt = lightPoints[ix];
        return true;
    }
    else
        return false;
}

bool trpgLight::GetVertices(trpg3dPoint *pts) const
{
    unsigned int i;

    if (!isValid()) return false;

    if (lightPoints.size() != 0)
        for (i=0;i<lightPoints.size();i++)
            pts[i] = lightPoints[i];

    return true;
}

bool trpgLight::GetVertices(float64 *fts) const
{
    unsigned int i;
    unsigned int j = 0;

    if (!isValid()) return false;

    if (lightPoints.size() != 0)
        for (i=0;i<lightPoints.size();i++) {
            fts[j++] = lightPoints[i].x;
            fts[j++] = lightPoints[i].y;
            fts[j++] = lightPoints[i].z;
        }

    return true;
}

bool trpgLight::GetVertices(float32 *fts) const
{
    unsigned int i;
    unsigned int j = 0;

    if (!isValid()) return false;

    if (lightPoints.size() != 0)
        for (i=0;i<lightPoints.size();i++) {
            fts[j++] = (float32)lightPoints[i].x;
            fts[j++] = (float32)lightPoints[i].y;
            fts[j++] = (float32)lightPoints[i].z;
        }

    return true;
}


// Returns the index of the Light Attributes in the Light Table
void trpgLight::GetAttrIndex(int &ix) const
{
    ix = index;
}

// Validity check
bool trpgLight::isValid(void) const
{
    return true;
}

// Resets the contents back to empty
void trpgLight::Reset(void)
{
    lightPoints.clear();
    index =-1;
}

// Writes this class to a write buffer
bool trpgLight::Write(trpgWriteBuffer &buf)
{
    int numVertices = lightPoints.size();

    if (!isValid())
        return false;

    buf.Begin(TRPGLIGHT);

    buf.Add(index);
    buf.Add(numVertices);
    for (unsigned int i=0;i<lightPoints.size();i++)
        buf.Add(lightPoints[i]);

    buf.End();

    return true;
}

// Reads this class from a read buffer
bool trpgLight::Read(trpgReadBuffer &buf)
{
    Reset();

    int numVertices;
    buf.Get(index);
    buf.Get(numVertices);
    for ( int i = 0; i < numVertices; i++ ) {
        trpg3dPoint vx;
        buf.Get(vx);
        lightPoints.push_back(vx);
    }

    return isValid();
}

// operator
trpgLight& trpgLight::operator = (const trpgLight &in)
{
    Reset();

    index = in.index;
    for (unsigned int i = 0; i < in.lightPoints.size(); i++ )
        lightPoints.push_back(in.lightPoints[i]);

    return *this;
}


/* Light Attributes Table
    Just a list of light attribs so we can index.
    */

// Constructor
trpgLightTable::trpgLightTable()
{
}

trpgLightTable::trpgLightTable(const trpgLightTable &in):
    trpgReadWriteable(in)
{
    *this = in;
}

// Reset function
void trpgLightTable::Reset()
{
    errMess[0] = '\0';
    //lightList.clear();
    lightMap.clear();
}

// Destructor
trpgLightTable::~trpgLightTable()
{
    Reset();
}

// Validity check
bool trpgLightTable::isValid() const
{
    LightMapType::const_iterator itr = lightMap.begin();
    for (  ; itr != lightMap.end( ); itr++)
    {
        if (!itr->second.isValid())
        {
            if(itr->second.getErrMess())
                strcpy(errMess, itr->second.getErrMess());
            return false;
        }
    }

    return true;
}

// Set functions
int trpgLightTable::AddLightAttr(const trpgLightAttr& inLight)
{
    int handle = inLight.GetHandle();
    if(handle==-1) {
        handle = lightMap.size();
    }
    lightMap[handle] = inLight;
    return handle;
}
int trpgLightTable::FindAddLightAttr(const trpgLightAttr& inLight)
{
    LightMapType::iterator itr = lightMap.begin();
    for (  ; itr != lightMap.end( ); itr++) {
        if(itr->second==inLight)
            return itr->first;
    }
    return AddLightAttr(inLight);
}


// Copy operator
trpgLightTable &trpgLightTable::operator = (const trpgLightTable &in)
{
    Reset();

    LightMapType::const_iterator itr = in.lightMap.begin();
    for (  ; itr != in.lightMap.end( ); itr++) {
        AddLightAttr(itr->second);
    }
    //for (int i=0;i<in.lightList.size();i++)
    //    AddLightAttr(in.lightList[i]);
    return *this;
}

// Write Light table
bool trpgLightTable::Write(trpgWriteBuffer &buf)
{
    int32 numLights;

    if (!isValid())
        return false;

    buf.Begin(TRPGLIGHTTABLE);
    numLights = lightMap.size();
    buf.Add(numLights);
    LightMapType::iterator itr = lightMap.begin();
    for (  ; itr != lightMap.end( ); itr++)
        itr->second.Write(buf);
    //for (unsigned int i=0;i<lightList.size();i++)
    //    lightList[i].Write(buf);
    buf.End();

    return true;
}

/* ***********
    Read Light Table
   ***********
   */
// Get functions
bool trpgLightTable::GetNumLightAttrs(int &no) const
{
    if (!isValid()) return false;
    no = lightMap.size();
    return true;
}

const trpgLightAttr* trpgLightTable::GetLightAttrRef(int id) const
{
    if (id < 0) return NULL;

    LightMapType::const_iterator itr = lightMap.find(id);
    if(itr == lightMap.end())    {
        return NULL;
    }

    return &itr->second;
}

bool trpgLightTable::Read(trpgReadBuffer &buf)
{
    int32        numLights;
    trpgToken    lightTok;
    int32        len;

    try {
        buf.Get(numLights);
        for (int i=0;i<numLights;i++) {
            buf.GetToken(lightTok,len);
            if (lightTok != TRPGLIGHTATTR) throw 1;
            buf.PushLimit(len);
            trpgLightAttr light;// = lightList[i];
            bool status = light.Read(buf);
            buf.PopLimit();
            if (!status) throw 1;
            AddLightAttr(light);
        }
    }
    catch (...) {
        return false;
    }

    return true;
}

