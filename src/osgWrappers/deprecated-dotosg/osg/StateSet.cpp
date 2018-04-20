#include <osg/StateSet>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/TextureCubeMap>
#include <osg/TextureRectangle>
#include <osg/TexGen>
#include <osg/PolygonOffset>
#include <osg/LineStipple>
#include <osg/Light>
#include <osg/ClipPlane>
#include <osg/AlphaFunc>
#include <osg/Point>
#include <osg/Material>
#include <osg/Fog>
#include <osg/GL2Extensions>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>

#include <set>
#include <string.h>

using namespace osg;
using namespace osgDB;
using namespace std;

// forward declare functions to use later.
bool GeoState_readLocalData(Object& obj, Input& fr);

bool StateSet_readLocalData(Object& obj, Input& fr);
bool StateSet_writeLocalData(const Object& obj, Output& fw);

bool StateSet_matchModeStr(const char* str,StateAttribute::GLModeValue& mode);
const char* StateSet_getModeStr(StateAttribute::GLModeValue mode);

bool StateSet_matchRenderBinModeStr(const char* str,StateSet::RenderBinMode& mode);
const char* StateSet_getRenderBinModeStr(StateSet::RenderBinMode mode);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(StateSet)
(
    new osg::StateSet,
    "StateSet",
    "Object StateSet",
    &StateSet_readLocalData,
    &StateSet_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(GeoState)
(
    new osg::StateSet,
    "GeoState",
    "Object GeoState",
    &GeoState_readLocalData,
    NULL,
    DotOsgWrapper::READ_ONLY
);

//
// Set up the maps from name to GLMode and visa versa.
//

#define ADD_NAME(name,mode) _GLNameToGLModeMap[name]=mode; _GLModeToGLNameMap[mode]=name;

struct ModesAndNames
{
    ModesAndNames();

    typedef std::map<std::string,StateAttribute::GLMode>    GLNameToGLModeMap;
    typedef std::map<StateAttribute::GLMode,std::string>    GLModeToGLNameMap;
    typedef std::set<StateAttribute::GLMode>                TextureGLModeSet;

    inline bool isTextureMode(int mode) const
    {
        return _TextureGLModeSet.find(mode)!=_TextureGLModeSet.end();
    }

    inline bool getGLModeForName(const std::string& str, osg::StateAttribute::GLMode& mode) const
    {
        GLNameToGLModeMap::const_iterator nitr = _GLNameToGLModeMap.find(str);
        if (nitr!=_GLNameToGLModeMap.end())
        {
            mode = nitr->second;
            return true;
        }
        else
        {
            return false;
        }

    }

    inline bool getNameForGLMode(const osg::StateAttribute::GLMode& mode, std::string& str) const
    {
        GLModeToGLNameMap::const_iterator nitr = _GLModeToGLNameMap.find(mode);
        if (nitr!=_GLModeToGLNameMap.end())
        {
            str = nitr->second;
            return true;
        }
        else
        {
            return false;
        }

    }

    GLNameToGLModeMap _GLNameToGLModeMap;
    GLModeToGLNameMap _GLModeToGLNameMap;
    TextureGLModeSet _TextureGLModeSet;
};

static ModesAndNames s_ModesAndNames;

ModesAndNames::ModesAndNames()
{
    ADD_NAME("GL_ALPHA_TEST",GL_ALPHA_TEST)
    ADD_NAME("GL_BLEND",GL_BLEND)
    ADD_NAME("GL_COLOR_MATERIAL",GL_COLOR_MATERIAL)
    ADD_NAME("GL_CULL_FACE",GL_CULL_FACE)
    ADD_NAME("GL_DEPTH_TEST",GL_DEPTH_TEST)
    ADD_NAME("GL_FOG",GL_FOG)
    ADD_NAME("GL_LIGHTING",GL_LIGHTING)
    ADD_NAME("GL_POINT_SMOOTH",GL_POINT_SMOOTH)
    ADD_NAME("GL_LINE_STIPPLE",GL_LINE_STIPPLE)
    ADD_NAME("GL_POLYGON_OFFSET_FILL",GL_POLYGON_OFFSET_FILL)
    ADD_NAME("GL_POLYGON_OFFSET_LINE",GL_POLYGON_OFFSET_LINE)
    ADD_NAME("GL_POLYGON_OFFSET_POINT",GL_POLYGON_OFFSET_POINT)
    ADD_NAME("GL_COLOR_SUM",GL_COLOR_SUM);
    ADD_NAME("GL_NORMALIZE",GL_NORMALIZE);
    ADD_NAME("GL_RESCALE_NORMAL",GL_RESCALE_NORMAL);

    ADD_NAME("GL_TEXTURE_1D",GL_TEXTURE_1D)
    ADD_NAME("GL_TEXTURE_2D",GL_TEXTURE_2D)
    ADD_NAME("GL_TEXTURE_3D",GL_TEXTURE_3D)

    ADD_NAME("GL_TEXTURE_CUBE_MAP",GL_TEXTURE_CUBE_MAP);
    ADD_NAME("GL_TEXTURE_RECTANGLE",GL_TEXTURE_RECTANGLE);

    ADD_NAME("GL_TEXTURE_GEN_Q",GL_TEXTURE_GEN_Q)
    ADD_NAME("GL_TEXTURE_GEN_R",GL_TEXTURE_GEN_R)
    ADD_NAME("GL_TEXTURE_GEN_S",GL_TEXTURE_GEN_S)
    ADD_NAME("GL_TEXTURE_GEN_T",GL_TEXTURE_GEN_T)

    ADD_NAME("GL_STENCIL_TEST",GL_STENCIL_TEST)

    ADD_NAME("GL_CLIP_PLANE0",GL_CLIP_PLANE0);
    ADD_NAME("GL_CLIP_PLANE1",GL_CLIP_PLANE1);
    ADD_NAME("GL_CLIP_PLANE2",GL_CLIP_PLANE2);
    ADD_NAME("GL_CLIP_PLANE3",GL_CLIP_PLANE3);
    ADD_NAME("GL_CLIP_PLANE4",GL_CLIP_PLANE4);
    ADD_NAME("GL_CLIP_PLANE5",GL_CLIP_PLANE5);

    ADD_NAME("GL_LIGHT0",GL_LIGHT0);
    ADD_NAME("GL_LIGHT1",GL_LIGHT1);
    ADD_NAME("GL_LIGHT2",GL_LIGHT2);
    ADD_NAME("GL_LIGHT3",GL_LIGHT3);
    ADD_NAME("GL_LIGHT4",GL_LIGHT4);
    ADD_NAME("GL_LIGHT5",GL_LIGHT5);
    ADD_NAME("GL_LIGHT6",GL_LIGHT6);
    ADD_NAME("GL_LIGHT7",GL_LIGHT7);

#if defined(OSG_GL3_AVAILABLE)
    #define GL_VERTEX_PROGRAM_POINT_SIZE      0x8642
    #define GL_VERTEX_PROGRAM_TWO_SIDE        0x8643
#endif
    ADD_NAME("GL_VERTEX_PROGRAM_POINT_SIZE", GL_VERTEX_PROGRAM_POINT_SIZE)
    ADD_NAME("GL_VERTEX_PROGRAM_TWO_SIDE", GL_VERTEX_PROGRAM_TWO_SIDE)

    _TextureGLModeSet.insert(GL_TEXTURE_1D);
    _TextureGLModeSet.insert(GL_TEXTURE_2D);
    _TextureGLModeSet.insert(GL_TEXTURE_3D);

    _TextureGLModeSet.insert(GL_TEXTURE_CUBE_MAP);
    _TextureGLModeSet.insert(GL_TEXTURE_RECTANGLE);

    _TextureGLModeSet.insert(GL_TEXTURE_GEN_Q);
    _TextureGLModeSet.insert(GL_TEXTURE_GEN_R);
    _TextureGLModeSet.insert(GL_TEXTURE_GEN_S);
    _TextureGLModeSet.insert(GL_TEXTURE_GEN_T);


//     for(GLNameToGLModeMap::iterator itr=s_GLNameToGLModeMap.begin();
//         itr!=s_GLNameToGLModeMap.end();
//         ++itr)
//     {
//         cout << "Name ["<<itr->first<<","<<itr->second<<"]"<< std::endl;
//     }
}


//////////////////////////////////////////////////////////////////////

bool GeoState_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    // note, StateSet replaced GeoState April 2001.
    StateSet& statset = static_cast<StateSet&>(obj);

    statset.setRenderingHint(StateSet::OPAQUE_BIN);

    StateAttribute::GLModeValue mode;
    if (fr[0].matchWord("transparency") && StateSet_matchModeStr(fr[1].getStr(),mode))
    {
        if (mode&StateAttribute::ON)
        {
            statset.setRenderingHint(StateSet::TRANSPARENT_BIN);
        }
        statset.setMode(GL_BLEND,mode);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("antialiasing") && StateSet_matchModeStr(fr[1].getStr(),mode))
    {
        // what is the OpenGL modes for antialissing, need to look up.
        // statset.setMode(GeoState::ANTIALIAS,mode);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("face_culling") && StateSet_matchModeStr(fr[1].getStr(),mode))
    {
        statset.setMode(GL_CULL_FACE,mode);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("lighting") && StateSet_matchModeStr(fr[1].getStr(),mode))
    {
        statset.setMode(GL_LIGHTING,mode);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("texturing") && StateSet_matchModeStr(fr[1].getStr(),mode))
    {
        statset.setTextureMode(0,GL_TEXTURE_2D,mode);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("fogging") && StateSet_matchModeStr(fr[1].getStr(),mode))
    {
        statset.setMode(GL_FOG,mode);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("colortable") && StateSet_matchModeStr(fr[1].getStr(),mode))
    {
        // what is the OpenGL modes for colortable, need to look up...
        // statset.setMode(GeoState::COLORTABLE,mode);
        fr+=2;
        iteratorAdvanced = true;
    }

    StateAttribute::GLModeValue texgening = StateAttribute::OFF;
    if (fr[0].matchWord("texgening") && StateSet_matchModeStr(fr[1].getStr(),mode))
    {
        // leave up to a tex gen object to set modes associated with TexGen
        // as there are multiple modes associated with TexGen.  See below
        // attribute reading code.
        texgening = mode;
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("point_smoothing") && StateSet_matchModeStr(fr[1].getStr(),mode))
    {
        statset.setMode(GL_POINT_SMOOTH,mode);
        fr+=2;
        iteratorAdvanced = true;
    }


    if (fr[0].matchWord("polygon_offset") && StateSet_matchModeStr(fr[1].getStr(),mode))
    {
        // no GL mode associated with polygon offset so commenting out.
        // statset.setMode(GeoState::POLYGON_OFFSET,mode);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("alpha_test") && StateSet_matchModeStr(fr[1].getStr(),mode))
    {
        statset.setMode(GL_ALPHA_TEST,mode);
        fr+=2;
        iteratorAdvanced = true;
    }


    // new code using osg::Registry's list of prototypes to loaded attributes.
    StateAttribute* attribute = NULL;
    while((attribute=fr.readStateAttribute())!=NULL)
    {
        if (attribute->isTextureAttribute())
        {
            // remap to be a texture attribute
            statset.setTextureAttribute(0,attribute);
        }
        else
        {
            statset.setAttribute(attribute);
        }

        if (attribute->getType()==StateAttribute::TEXGEN)
            statset.setAssociatedModes(attribute,texgening);

        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool StateSet_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    // note, StateSet replaced GeoState April 2001.
    StateSet& stateset = static_cast<StateSet&>(obj);

    // read the rendering hint value.
    if (fr[0].matchWord("rendering_hint"))
    {
        if (fr[1].matchWord("DEFAULT_BIN"))
        {
            stateset.setRenderingHint(StateSet::DEFAULT_BIN);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("OPAQUE_BIN"))
        {
            stateset.setRenderingHint(StateSet::OPAQUE_BIN);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("TRANSPARENT_BIN"))
        {
            stateset.setRenderingHint(StateSet::TRANSPARENT_BIN);
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].isInt())
        {
            int value;
            fr[1].getInt(value);
            stateset.setRenderingHint(value);
            fr+=2;
            iteratorAdvanced = true;
        }
    }

    bool setRenderBinDetails=false;
    StateSet::RenderBinMode rbmode = stateset.getRenderBinMode();
    if (fr[0].matchWord("renderBinMode") && StateSet_matchRenderBinModeStr(fr[1].getStr(),rbmode))
    {
        setRenderBinDetails=true;
        fr+=2;
        iteratorAdvanced = true;
    }

    int binNumber = stateset.getBinNumber();
    if (fr[0].matchWord("binNumber") && fr[1].getInt(binNumber))
    {
        setRenderBinDetails=true;
        fr+=2;
        iteratorAdvanced = true;
    }

    std::string binName = stateset.getBinName();
    if (fr[0].matchWord("binName"))
    {
        setRenderBinDetails=true;
        binName = fr[1].getStr();

        fr+=2;
        iteratorAdvanced = true;
    }

    if (setRenderBinDetails)
    {
        stateset.setRenderBinDetails(binNumber,binName,rbmode);
    }

    while (fr.matchSequence("UpdateCallback {"))
    {
        // int entry = fr[0].getNoNestedBrackets();
        fr += 2;
        StateSet::Callback* callback = fr.readObjectOfType<StateSet::Callback>();
        if (callback) {
            stateset.setUpdateCallback(callback);
        }
        iteratorAdvanced = true;
    }

    while (fr.matchSequence("EventCallback {"))
    {
        //int entry = fr[0].getNoNestedBrackets();
        fr += 2;
        StateSet::Callback* callback = fr.readObjectOfType<StateSet::Callback>();
        if (callback) {
            stateset.setEventCallback(callback);
        }
        iteratorAdvanced = true;
    }

    bool readingGLMode = true;
    while (readingGLMode)
    {
        StateAttribute::GLModeValue value;
        readingGLMode=false;
        if (fr[0].isInt())
        {
            if (StateSet_matchModeStr(fr[1].getStr(),value))
            {
                int mode;
                fr[0].getInt(mode);

                if (s_ModesAndNames.isTextureMode(mode))
                {
                    // remap to a texture unit.
                    stateset.setTextureMode(0,(StateAttribute::GLMode)mode,value);
                }
                else
                {
                    stateset.setMode((StateAttribute::GLMode)mode,value);
                }
                fr+=2;
                iteratorAdvanced = true;
                readingGLMode=true;
            }
        }
        else
        if (fr[0].getStr())
        {
            if (StateSet_matchModeStr(fr[1].getStr(),value))
            {
                StateAttribute::GLMode mode=0;
                if (s_ModesAndNames.getGLModeForName(fr[0].getStr(), mode))
                {
                    if (s_ModesAndNames.isTextureMode(mode))
                    {
                        // remap to a texture unit.
                        stateset.setTextureMode(0,mode,value);
                    }
                    else
                    {
                        stateset.setMode(mode,value);
                    }
                    fr+=2;
                    iteratorAdvanced = true;
                    readingGLMode=true;
                }
            }
        }
    }

    // new code using osg::Registry's list of prototypes to loaded attributes.
    Uniform* uniform = NULL;
    while((uniform=fr.readUniform())!=NULL)
    {
        stateset.addUniform(uniform);
        iteratorAdvanced = true;
    }


    // new code using osg::Registry's list of prototypes to loaded attributes.
    StateAttribute* stateAttribute = NULL;
    while((stateAttribute=fr.readStateAttribute())!=NULL)
    {
        if (stateAttribute->isTextureAttribute())
        {
            // remap to be a texture attribute
            stateset.setTextureAttribute(0,stateAttribute);
        }
        else
        {
            stateset.setAttribute(stateAttribute);
        }
        iteratorAdvanced = true;
    }

    while(fr.matchSequence("textureUnit %i {"))
    {
        int entry = fr[0].getNoNestedBrackets();

        unsigned int unit=0;
        fr[1].getUInt(unit);
        fr+=3;

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            bool localIteratorAdvanced = false;

            bool readingMode = true;
            StateAttribute::GLModeValue value;
            while (readingMode)
            {
                readingMode=false;
                if (fr[0].isInt())
                {
                    if (StateSet_matchModeStr(fr[1].getStr(),value))
                    {
                        int mode;
                        fr[0].getInt(mode);
                        stateset.setTextureMode(unit,(StateAttribute::GLMode)mode,value);
                        fr+=2;
                        localIteratorAdvanced = true;
                        readingMode=true;
                    }
                }
                else
                if (fr[0].getStr())
                {
                    if (StateSet_matchModeStr(fr[1].getStr(),value))
                    {
                        StateAttribute::GLMode mode=0;
                        if (s_ModesAndNames.getGLModeForName(fr[0].getStr(), mode))
                        {
                            stateset.setTextureMode(unit,mode,value);
                            fr+=2;
                            localIteratorAdvanced = true;
                            readingMode=true;
                        }
                    }
                }
            }

            StateAttribute* attribute = NULL;
            while((attribute=fr.readStateAttribute())!=NULL)
            {
                stateset.setTextureAttribute(unit,attribute);
                localIteratorAdvanced = true;
            }

            if (!localIteratorAdvanced)
                fr.advanceOverCurrentFieldOrBlock();
        }

        // skip over trailing '}'
        ++fr;

        iteratorAdvanced = true;

    }




    return iteratorAdvanced;
}

// visual studio 6.0 doesn't appear to define std::max?!? So do our own here..
template<class T>
T mymax(const T& a,const T& b)
{
    return (((a) > (b)) ? (a) : (b));
}

bool StateSet_writeLocalData(const Object& obj, Output& fw)
{

    const StateSet& stateset = static_cast<const StateSet&>(obj);

    // write the rendering hint value.
    fw.indent()<<"rendering_hint ";
    switch(stateset.getRenderingHint())
    {
    case(StateSet::DEFAULT_BIN):
        fw<<"DEFAULT_BIN"<< std::endl;
        break;
    case(StateSet::OPAQUE_BIN):
        fw<<"OPAQUE_BIN"<< std::endl;
        break;
    case(StateSet::TRANSPARENT_BIN):
        fw<<"TRANSPARENT_BIN"<< std::endl;
        break;
    default:
        fw<<stateset.getRenderingHint()<< std::endl;
        break;
    }

    fw.indent()<<"renderBinMode "<<StateSet_getRenderBinModeStr(stateset.getRenderBinMode())<< std::endl;
    if (stateset.getRenderBinMode()!=StateSet::INHERIT_RENDERBIN_DETAILS)
    {
        fw.indent()<<"binNumber "<<stateset.getBinNumber()<< std::endl;
        fw.indent()<<"binName "<<stateset.getBinName()<< std::endl;
    }


  const StateSet::ModeList& ss_ml = stateset.getModeList();
  for(StateSet::ModeList::const_iterator mitr=ss_ml.begin();
        mitr!=ss_ml.end();
        ++mitr)
     {
         std::string str;
         if (s_ModesAndNames.getNameForGLMode(mitr->first, str))
         {
             fw.indent() << str << " " << StateSet_getModeStr(mitr->second) << std::endl;
         }
         else
         {
            // no name defined for GLMode so just pass its value to fw.
             fw.indent() << "0x" << hex << (unsigned int)mitr->first << dec <<" " << StateSet_getModeStr(mitr->second) << std::endl;
         }
    }

    const StateSet::UniformList& ul = stateset.getUniformList();
    for(StateSet::UniformList::const_iterator uitr=ul.begin();
        uitr!=ul.end();
        ++uitr)
    {
        fw.writeObject(*(uitr->second.first));
    }

    const StateSet::AttributeList& ss_sl = stateset.getAttributeList();
    for(StateSet::AttributeList::const_iterator sitr=ss_sl.begin();
        sitr!=ss_sl.end();
        ++sitr)
    {
        fw.writeObject(*(sitr->second.first));
    }


    const StateSet::TextureModeList& tml = stateset.getTextureModeList();
    const StateSet::TextureAttributeList& tal = stateset.getTextureAttributeList();
    unsigned int maxUnit = mymax(tml.size(),tal.size());
    for(unsigned int unit=0;unit<maxUnit;++unit)
    {
        fw.indent()<<"textureUnit "<<unit<<" {"<< std::endl;
        fw.moveIn();

        if (unit<tml.size())
        {
            const StateSet::ModeList& ml = tml[unit];
            for(StateSet::ModeList::const_iterator mitr=ml.begin();
                mitr!=ml.end();
                ++mitr)
            {
                std::string str;
                if (s_ModesAndNames.getNameForGLMode(mitr->first, str))
                {
                    fw.indent() << str << " " << StateSet_getModeStr(mitr->second) << std::endl;
                }
                else
                {
                    // no name defined for GLMode so just pass its value to fw.
                    fw.indent() << "0x" << hex << (unsigned int)mitr->first << dec <<" " << StateSet_getModeStr(mitr->second) << std::endl;
                }
            }
        }

        if (unit<tal.size())
        {
            const StateSet::AttributeList& sl = tal[unit];
            for(StateSet::AttributeList::const_iterator sitr=sl.begin();
                sitr!=sl.end();
                ++sitr)
            {
                fw.writeObject(*(sitr->second.first));
            }
        }

        fw.moveOut();
        fw.indent()<<"}"<< std::endl;
    }

    if (stateset.getUpdateCallback())
    {
        fw.indent() << "UpdateCallback {" << std::endl;
        fw.moveIn();
        fw.writeObject(*stateset.getUpdateCallback());
        fw.moveOut();
        fw.indent() << "}" << std::endl;
    }

    if (stateset.getEventCallback())
    {
        fw.indent() << "EventCallback {" << std::endl;
        fw.moveIn();
        fw.writeObject(*stateset.getEventCallback());
        fw.moveOut();
        fw.indent() << "}" << std::endl;
    }

    return true;
}


bool StateSet_matchModeStr(const char* str,StateAttribute::GLModeValue& mode)
{
    if (strcmp(str,"INHERIT")==0) mode = StateAttribute::INHERIT;
    else if (strcmp(str,"ON")==0) mode = StateAttribute::ON;
    else if (strcmp(str,"OFF")==0) mode = StateAttribute::OFF;
    else if (strcmp(str,"OVERRIDE_ON")==0) mode = StateAttribute::OVERRIDE|StateAttribute::ON;
    else if (strcmp(str,"OVERRIDE_OFF")==0) mode = StateAttribute::OVERRIDE|StateAttribute::OFF;
    else if (strcmp(str,"OVERRIDE|ON")==0) mode = StateAttribute::OVERRIDE|StateAttribute::ON;
    else if (strcmp(str,"OVERRIDE|OFF")==0) mode = StateAttribute::OVERRIDE|StateAttribute::OFF;
    else if (strcmp(str,"PROTECTED|ON")==0) mode = StateAttribute::PROTECTED|StateAttribute::ON;
    else if (strcmp(str,"PROTECTED|OFF")==0) mode = StateAttribute::PROTECTED|StateAttribute::OFF;
    else if (strcmp(str,"PROTECTED|OVERRIDE|ON")==0) mode = StateAttribute::PROTECTED|StateAttribute::OVERRIDE|StateAttribute::ON;
    else if (strcmp(str,"PROTECTED|OVERRIDE|OFF")==0) mode = StateAttribute::PROTECTED|StateAttribute::OVERRIDE|StateAttribute::OFF;
    else return false;
    return true;
}


const char* StateSet_getModeStr(StateAttribute::GLModeValue value)
{
    switch(value)
    {
        case(StateAttribute::INHERIT): return "INHERIT";
        case(StateAttribute::ON): return "ON";
        case(StateAttribute::OFF): return "OFF";
        case(StateAttribute::OVERRIDE|StateAttribute::ON): return "OVERRIDE|ON";
        case(StateAttribute::OVERRIDE|StateAttribute::OFF): return "OVERRIDE|OFF";
        case(StateAttribute::PROTECTED|StateAttribute::ON): return "PROTECTED|ON";
        case(StateAttribute::PROTECTED|StateAttribute::OFF): return "PROTECTED|OFF";
        case(StateAttribute::PROTECTED|StateAttribute::OVERRIDE|StateAttribute::ON): return "PROTECTED|OVERRIDE|ON";
        case(StateAttribute::PROTECTED|StateAttribute::OVERRIDE|StateAttribute::OFF): return "PROTECTED|OVERRIDE|OFF";
    }
    return "";
}

bool StateSet_matchRenderBinModeStr(const char* str,StateSet::RenderBinMode& mode)
{
    if (strcmp(str,"INHERIT")==0) mode = StateSet::INHERIT_RENDERBIN_DETAILS;
    else if (strcmp(str,"USE")==0) mode = StateSet::USE_RENDERBIN_DETAILS;
    else if (strcmp(str,"OVERRIDE")==0) mode = StateSet::OVERRIDE_RENDERBIN_DETAILS;
    else if (strcmp(str,"ENCLOSE")==0) mode = StateSet::USE_RENDERBIN_DETAILS;
    else if (strcmp(str,"PROTECTED")==0) mode = StateSet::PROTECTED_RENDERBIN_DETAILS;
    else if (strcmp(str,"OVERRIDE|PROTECTED")==0) mode = StateSet::OVERRIDE_PROTECTED_RENDERBIN_DETAILS;
    else return false;
    return true;
}

const char* StateSet_getRenderBinModeStr(StateSet::RenderBinMode mode)
{
    switch(mode)
    {
        case(StateSet::INHERIT_RENDERBIN_DETAILS):  return "INHERIT";
        case(StateSet::USE_RENDERBIN_DETAILS):      return "USE";
        case(StateSet::OVERRIDE_RENDERBIN_DETAILS): return "OVERRIDE";
        case(StateSet::PROTECTED_RENDERBIN_DETAILS):return "PROTECTED";
        case(StateSet::OVERRIDE_PROTECTED_RENDERBIN_DETAILS): return "OVERRIDE|PROTECTED";
    }
    return "";
}
