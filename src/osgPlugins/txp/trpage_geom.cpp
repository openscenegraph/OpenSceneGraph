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

/* trpage_geom.cpp
    Methods for the trpgGeometry class.
    This includes read and write methods.
    You should only need to change something in here if you want to modify
     what trpgGeometry contains.
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

// Constructor
trpgGeometry::trpgGeometry()
{
    primType = Polygons;
    normBind = Overall;
    numPrim = 0;
}
trpgGeometry::~trpgGeometry()
{
}

// Reset function
void trpgGeometry::Reset()
{
    primType = Polygons;
    numPrim = 0;
    primLength.resize(0);
    materials.resize(0);
    vertDataFloat.resize(0);
    vertDataDouble.resize(0);
    normBind = Overall;
    normDataFloat.resize(0);
    normDataDouble.resize(0);
    colors.resize(0);
    texData.resize(0);
    edgeFlags.resize(0);
}

// Set functions
void trpgGeometry::SetPrimType(PrimType type)
{
    primType = type;
}
void trpgGeometry::SetPrimLengths(int num,const int *len)
{
    if (num < 0)
        return;

    numPrim = num;
    for (int i=0;i<num;i++)
        primLength.push_back(len[i]);
}
void trpgGeometry::AddPrimLength(int len)
{
    if (len < 0)
        return;

    numPrim++;
    primLength.push_back(len);
}
void trpgGeometry::AddPrim()
{
    numPrim++;
}
void trpgGeometry::SetNumPrims(int num)
{
    numPrim = num;
}
void trpgGeometry::SetNumMaterial(int no)
{
    if (no < 0)
        return;

    materials.resize(no,-1);
}
void trpgGeometry::SetMaterial(int which,int mat,bool isLocal)
{
    if (which < 0 || which >= (int)materials.size())
        return;

    materials[which] = (isLocal ? -(mat+1) : mat);
}
void trpgGeometry::SetMaterials(int32 num,const int32 *mat)
{
    materials.resize(num);
    for (int i=0;i<num;i++)
        materials[i] = mat[i];
}
int trpgGeometry::AddMaterial(int mat)
{
    materials.push_back(mat);

    return materials.size()-1;
}

// Geometry/color/normal/etc... set functions
void trpgGeometry::SetVertices(int num,const float32 *data)
{
    if (num < 0)
        return;

    vertDataFloat.resize(0);
    vertDataDouble.resize(0);
    for (int i=0;i<3*num;i++)
        vertDataFloat.push_back(data[i]);
}
void trpgGeometry::SetVertices(int num,const float64 *data)
{
    if (num < 0)
        return;

    vertDataFloat.resize(0);
    vertDataDouble.resize(0);
    for (int i=0;i<3*num;i++)
        vertDataDouble.push_back(data[i]);
}
void trpgGeometry::AddVertex(DataType type,trpg3dPoint &pt)
{
    if (type == FloatData) {
        vertDataFloat.push_back(static_cast<float>(pt.x));
        vertDataFloat.push_back(static_cast<float>(pt.y));
        vertDataFloat.push_back(static_cast<float>(pt.z));
    } else {
        vertDataDouble.push_back(pt.x);
        vertDataDouble.push_back(pt.y);
        vertDataDouble.push_back(pt.z);
    }
}
void trpgGeometry::SetNormals(int num,BindType bind,const float32 *data)
{
    if (num < 0)
        return;

    normBind = bind;
    normDataFloat.resize(0);
    normDataDouble.resize(0);
    for (int i=0;i<3*num;i++)
        normDataFloat.push_back(data[i]);
}
void trpgGeometry::SetNormals(int num,BindType bind,const float64 *data)
{
    if (num <0)
        return;

    normBind = bind;
    normDataFloat.resize(0);
    normDataDouble.resize(0);
    for (int i=0;i<3*num;i++)
        normDataDouble.push_back(data[i]);
}
void trpgGeometry::AddNormal(DataType type,trpg3dPoint &pt)
{
    if (type == FloatData) {
        normDataFloat.push_back(static_cast<float>(pt.x));
        normDataFloat.push_back(static_cast<float>(pt.y));
        normDataFloat.push_back(static_cast<float>(pt.z));
    } else {
        normDataDouble.push_back(pt.x);
        normDataDouble.push_back(pt.y);
        normDataDouble.push_back(pt.z);
    }
}
// Constructor
trpgColorInfo::trpgColorInfo()
{
}
trpgColorInfo::~trpgColorInfo()
{
}
void trpgColorInfo::Reset()
{
    bind = 0;
    type = 0;
    data.resize(0);
}
void trpgGeometry::SetColors(int num,ColorType type,BindType bind,const trpgColor *data)
{
    trpgColorInfo ci;

    if (num < 0)
        return;

    // Set up color list
    ci.type = type;
    ci.bind = bind;
    for (int i=0;i<num;i++)
        ci.data.push_back(data[i]);

    colors.push_back(ci);
}
// Constructor
trpgTexData::trpgTexData()
{
}
trpgTexData::~trpgTexData()
{
}
void trpgTexData::set(int num,int in_bind,const float32 *data)
{
    bind = in_bind;
    floatData.resize(0);
    doubleData.resize(0);
    for (int i=0;i<2*num;i++)
        floatData.push_back(data[i]);
}
void trpgTexData::set(int num,int in_bind,const float64 *data)
{
    bind = in_bind;
    floatData.resize(0);
    doubleData.resize(0);
    for (int i=0;i<2*num;i++)
        doubleData.push_back(data[i]);
}
void trpgTexData::Reset()
{
    bind = 0;
    floatData.resize(0);
    doubleData.resize(0);
}

void trpgGeometry::SetTexCoords(int num,BindType bind,const float32 *data)
{
    if (num < 0)
        return;

    trpgTexData td;
    td.set(num,bind,data);
    texData.push_back(td);
}
void trpgGeometry::SetTexCoords(int num,BindType bind,const float64 *data)
{
    if (num < 0)
        return;

    trpgTexData td;
    td.set(num,bind,data);
    texData.push_back(td);
}
void trpgGeometry::AddTexCoord(DataType type,trpg2dPoint &pt, int n)
{
    if ((n<0) || (n >= (int)texData.size()))
        return;
    trpgTexData *td = &texData[n];

    if (type == FloatData) {
        td->floatData.push_back(static_cast<float>(pt.x));
        td->floatData.push_back(static_cast<float>(pt.y));
    } else {
        td->doubleData.push_back(pt.x);
        td->doubleData.push_back(pt.y);
    }
}
void trpgGeometry::AddTexCoord(DataType type,std::vector<trpg2dPoint> &pts)
{
    if (texData.size() != pts.size())
        return;

    for (unsigned int loop = 0; loop < pts.size(); loop++ ) {
        trpgTexData *td = &texData[loop];

        if (type == FloatData) {
            td->floatData.push_back(static_cast<float>(pts[loop].x));
            td->floatData.push_back(static_cast<float>(pts[loop].y));
        } else {
            td->doubleData.push_back(pts[loop].x);
            td->doubleData.push_back(pts[loop].y);
        }
    }
}
void trpgGeometry::AddTexCoords(BindType bind)
{
    trpgTexData td;
    td.bind = bind;
    texData.push_back(td);
}
void trpgGeometry::SetEdgeFlags(int num,const char *flags)
{
    if (num < 0)
        return;

    edgeFlags.resize(0);
    for (int i=0;i<num;i++)
        edgeFlags.push_back(flags[i]);
}

// Get methods
bool trpgGeometry::GetPrimType(PrimType &t) const
{
    if (!isValid()) return false;
    t = (PrimType)primType;
    return true;
}
bool trpgGeometry::GetNumPrims(int &n) const
{
    if (!isValid()) return false;
    n = numPrim;
    return true;
}
bool trpgGeometry::GetPrimLengths(int *ret) const
{
    if (!isValid()) return false;
    for (int i=0;i<numPrim;i++)
        ret[i] = primLength[i];
    return true;
}
bool trpgGeometry::GetNumMaterial(int &n) const
{
    if (!isValid()) return false;
    n = materials.size();
    return true;
}
bool trpgGeometry::GetMaterial(int id,int32 &m,bool &isLocal) const
{
    isLocal = false;
    if (!isValid() || id < 0 || id >= (int)materials.size()) return false;
    m = materials[id];
    if (m < 0) {
        m = -m - 1;
        isLocal = true;
    }
    return true;
}
bool trpgGeometry::GetNumVertex(int &v) const
{
    if (!isValid()) return false;
    int nvf = vertDataFloat.size();
    int nvd = vertDataDouble.size();
    v = MAX(nvf,nvd);
    v = v / 3;
    return true;
}
bool trpgGeometry::GetVertices(float32 *v) const
{
    unsigned int i;

    if (!isValid()) return false;
    if (vertDataFloat.size() != 0)
        for (i=0;i<vertDataFloat.size();i++)
            v[i] = vertDataFloat[i];
    else
        for (i=0;i<vertDataDouble.size();i++)
            v[i] = static_cast<float32>(vertDataDouble[i]);
    return true;
}
bool trpgGeometry::GetVertices(float64 *v) const
{
    unsigned int i;

    if (!isValid()) return false;
    if (vertDataFloat.size() != 0)
        for (i=0;i<vertDataFloat.size();i++)
            v[i] = vertDataFloat[i];
    else
        for (i=0;i<vertDataDouble.size();i++)
            v[i] = vertDataDouble[i];
    return true;
}
bool trpgGeometry::GetVertex(int n,trpg3dPoint &pt) const
{
    int id = 3*n;
    int idMax = 3*n+2;
    if (id < 0 || (idMax >= (int)vertDataFloat.size() && idMax >= (int)vertDataDouble.size()))
        return false;
    if (vertDataFloat.size() > vertDataDouble.size()) {
        pt.x = vertDataFloat[id];
        pt.y = vertDataFloat[id+1];
        pt.z = vertDataFloat[id+2];
    } else {
        pt.x = vertDataDouble[id];
        pt.y = vertDataDouble[id+1];
        pt.z = vertDataDouble[id+2];
    }
    return true;
}
bool trpgGeometry::GetNumNormal(int32 &n) const
{
    if (!isValid()) return false;
    if (normDataFloat.size() != 0)
        n = normDataFloat.size();
    if (normDataDouble.size() != 0)
        n = normDataDouble.size();
    n = n / 3;
    return true;
}
bool trpgGeometry::GetNormals(float32 *v) const
{
    unsigned int i;

    if (!isValid()) return false;
    if (normDataFloat.size() != 0)
        for (i=0;i<normDataFloat.size();i++)
            v[i] = normDataFloat[i];
    else
        for (i=0;i<normDataDouble.size();i++)
            v[i] = static_cast<float32>(normDataDouble[i]);
    return true;
}
bool trpgGeometry::GetNormals(float64 *v) const
{
    unsigned int i;

    if (!isValid()) return false;
    if (normDataFloat.size() != 0)
        for (i=0;i<normDataFloat.size();i++)
            v[i] = normDataFloat[i];
    else
        for (i=0;i<normDataDouble.size();i++)
            v[i] = normDataDouble[i];
    return true;
}
bool trpgGeometry::GetNumColorSets(int &n) const
{
    if (!isValid()) return false;
    n = colors.size();
    return true;
}
bool trpgGeometry::GetColorSet(int id,trpgColorInfo *ci) const
{
    if (!isValid() || id < 0 || id >= (int)colors.size()) return false;
    *ci = colors[id];
    return true;
}
bool trpgGeometry::GetNumTexCoordSets(int &n) const
{
    if (!isValid()) return false;
    n = texData.size();
    return true;
}
bool trpgGeometry::GetTexCoordSet(int id,trpgTexData *tx) const
{
    if (!isValid() || id < 0 || id >= (int)texData.size()) return false;
    *tx = texData[id];
    return true;
}
const trpgTexData *trpgGeometry::GetTexCoordSet(int id) const
{
    if (!isValid() || id < 0 || id >= (int)texData.size()) return 0;
    return &(texData[id]);
}
bool trpgGeometry::GetNumEdgeFlag(int &n) const
{
    if (!isValid()) return false;
    n = edgeFlags.size();
    return true;
}
bool trpgGeometry::GetEdgeFlags(char *e) const
{
    if (!isValid()) return false;
    for (unsigned int i=0;i<edgeFlags.size();i++)
        e[i] = edgeFlags[i];
    return true;
}

// Validity check
// Note: maybe I should do this sometime
bool trpgGeometry::isValid() const
{
    return true;
}

// Write geometry fields.
// Order doesn't matter very much for this
bool trpgGeometry::Write(trpgWriteBuffer &buf)
{
    unsigned int i,j;

    if (!isValid())
        return false;

    buf.Begin(TRPG_GEOMETRY);
    /* Primitive info
        Primitive Type
         Number of primitives
        Primitive array lengths
        */
    buf.Begin(TRPG_GEOM_PRIM);
     buf.Add(primType);
    buf.Add(numPrim);
    if (primLength.size() != 0) {
        buf.Add((uint8)1);
        for (i=0;i<(unsigned int)numPrim;i++)
            buf.Add(primLength[i]);
    } else
        buf.Add((uint8)0);
    buf.End();

    /* Material info
        Num materials
        Material indicies
        */
    if (materials.size() > 0) {
        buf.Begin(TRPG_GEOM_MATERIAL);
        buf.Add((int32)materials.size());
        for (i=0;i<materials.size();i++)
            buf.Add(materials[i]);
        buf.End();
    }

    /* Vertices
        Float and Double should never both be here
        Num vertex
        Vertex data
        */
    if (vertDataFloat.size() > 0) {
        buf.Begin(TRPG_GEOM_VERT32);
        int32 num = vertDataFloat.size()/3;
        buf.Add(num);
        for (i=0;i<(unsigned int)3*num;i++)
            buf.Add(vertDataFloat[i]);
        buf.End();
    }
    if (vertDataDouble.size() > 0) {
        buf.Begin(TRPG_GEOM_VERT64);
        int32 num = vertDataDouble.size()/3;
        buf.Add(num);
        for (i=0;i<(unsigned int)3*num;i++)
            buf.Add(vertDataDouble[i]);
        buf.End();
    }

    /* Normals
        Normal binding
        Num normals
        Normal data
        */
    if (normDataFloat.size() > 0) {
        buf.Begin(TRPG_GEOM_NORM32);
        buf.Add((int32)normBind);
        int32 num = normDataFloat.size()/3;
        buf.Add(num);
        for (i=0;i<(unsigned int)3*num;i++)
            buf.Add(normDataFloat[i]);
        buf.End();
    }
    if (normDataDouble.size() > 0) {
        buf.Begin(TRPG_GEOM_NORM64);
        buf.Add((int32)normBind);
        int32 num = normDataDouble.size()/3;
        buf.Add(num);
        for (i=0;i<(unsigned int)3*num;i++)
            buf.Add(normDataDouble[i]);
        buf.End();
    }

    /* Colors
        Color binding
        Num colors
        Colors
           */
    if (colors.size() > 0) {
        for (i=0;i<colors.size();i++) {
            trpgColorInfo &ci = colors[i];
            if (ci.data.size()) {
                buf.Begin(TRPG_GEOM_COLOR);
                buf.Add((int32)ci.type);
                buf.Add((int32)ci.bind);
                buf.Add((int32)ci.data.size());
                for (j=0;j<ci.data.size();j++)
                    buf.Add(ci.data[j]);
                buf.End();
            }
        }
    }

    /* Texture coordinates
        Binding
        Num coords
        Texture coords
        */
    for (i=0;i<texData.size();i++) {
        trpgTexData &td = texData[i];
        if (td.floatData.size()) {
            buf.Begin(TRPG_GEOM_TEX32);
            buf.Add((int32)td.bind);
            int32 num = td.floatData.size()/2;
            buf.Add(num);
            for (j=0;j<(unsigned int)num*2;j++)
                buf.Add(td.floatData[j]);
            buf.End();
        }
        if (td.doubleData.size()) {
            buf.Begin(TRPG_GEOM_TEX64);
            buf.Add((int32)td.bind);
            int32 num = td.doubleData.size()/2;
            buf.Add(num);
            for (j=0;j<(unsigned int)num*2;j++)
                buf.Add(td.doubleData[j]);
            buf.End();
        }
    }

    // Edge flags (for triangle strips, etc..)
    if (edgeFlags.size() > 0) {
        buf.Begin(TRPG_GEOM_EFLAG);
        buf.Add((int32)edgeFlags.size());
        for (i=0;i<edgeFlags.size();i++)
            buf.Add(edgeFlags[i]);
        buf.End();
    }

    buf.End();

    return true;
}

// Geometry class is made up of individual tokens.
class geomCB : public trpgr_Callback {
public:
    void *Parse(trpgToken,trpgReadBuffer &buf);
    trpgGeometry *geom;
};

void *geomCB::Parse(trpgToken tok,trpgReadBuffer &buf)
{
    int32 *iData;
    int32 num,primType,bind,type;
    float32 *fData;
    float64 *dData;
    trpgColor *cData;
    char *charData;
    uint8 hasPrimLen;

    try {
        switch (tok) {
        case TRPG_GEOM_PRIM:
            buf.Get(primType);
            geom->SetPrimType((trpgGeometry::PrimType)primType);
            buf.Get(num);
            if (num < 0) throw 1;
            geom->SetNumPrims(num);
            buf.Get(hasPrimLen);
            if (hasPrimLen) {
                buf.GetArray(num,&iData);
                if (ALIGNMENT_WORKAROUND)
                {
                    int32 *aligned;
                    aligned = (int32 *)calloc (num, sizeof(int32));
                    memcpy (aligned, iData, num * sizeof(int32));
                    geom->SetPrimLengths(num, aligned);
                    free (aligned);
                }
                else
                    geom->SetPrimLengths(num,iData);
            }
            break;
        case TRPG_GEOM_MATERIAL:
            buf.Get(num);
            if (num < 0) throw 1;
            buf.GetArray(num,&iData);
            if (ALIGNMENT_WORKAROUND)
            {
                int32 *aligned;
                aligned = (int32 *)calloc (num, sizeof(int32));
                memcpy (aligned, iData, num * sizeof(int32));
                geom->SetMaterials(num,aligned);
                free (aligned);
            }
            else
                geom->SetMaterials(num,iData);
            break;
        case TRPG_GEOM_VERT32:
            buf.Get(num);
            if (num < 0) throw 1;
            buf.GetArray(3*num,&fData);
            if (ALIGNMENT_WORKAROUND)
            {
                float32 *aligned;
                aligned = (float32 *)calloc (3*num, sizeof(float32));
                memcpy (aligned, fData, 3*num * sizeof(float32));
                geom->SetVertices(num,aligned);
                free (aligned);
            }
            else
                geom->SetVertices(num,fData);
            break;
        case TRPG_GEOM_VERT64:
            buf.Get(num);
            if (num < 0) throw 1;
            buf.GetArray(3*num,&dData);
            if (ALIGNMENT_WORKAROUND)
            {
                float64 *aligned;
                aligned = (float64 *)calloc (3*num, sizeof(float64));
                memcpy (aligned, dData, 3*num * sizeof(float64));
                geom->SetVertices(num,aligned);
                free (aligned);
            }
            else
                geom->SetVertices(num,dData);
            break;
        case TRPG_GEOM_NORM32:
            buf.Get(bind);
            buf.Get(num);
            if (num < 0) throw 1;
            buf.GetArray(3*num,&fData);
            if (ALIGNMENT_WORKAROUND)
            {
                float32 *aligned;
                aligned = (float32 *)calloc (3*num, sizeof(float32));
                memcpy (aligned, fData, 3*num * sizeof(float32));
                geom->SetNormals(num,(trpgGeometry::BindType)bind,aligned);
                free (aligned);
            }
            else
                geom->SetNormals(num,(trpgGeometry::BindType)bind,fData);
            break;
        case TRPG_GEOM_NORM64:
            buf.Get(bind);
            buf.Get(num);
            if (num < 0) throw 1;
            buf.GetArray(3*num,&dData);
            if (ALIGNMENT_WORKAROUND)
            {
                float64 *aligned;
                aligned = (float64 *)calloc (3*num, sizeof(float64));
                memcpy (aligned, dData, 3*num * sizeof(float64));
                geom->SetNormals(num,(trpgGeometry::BindType)bind,aligned);
                free (aligned);
            }
            else
                geom->SetNormals(num,(trpgGeometry::BindType)bind,dData);
            break;
        case TRPG_GEOM_COLOR:
            buf.Get(num);
            if (num < 0) throw 1;
            buf.Get(type);
            buf.Get(bind);
            buf.GetArray(num,&cData);
            if (ALIGNMENT_WORKAROUND)
            {
                trpgColor *aligned;
                aligned = (trpgColor *)calloc (num, sizeof(trpgColor));
                memcpy (aligned, cData, num * sizeof(trpgColor));
                geom->SetColors(num,(trpgGeometry::ColorType)type,(trpgGeometry::BindType)bind,aligned);
                free (aligned);
            }
            else
                geom->SetColors(num,(trpgGeometry::ColorType)type,(trpgGeometry::BindType)bind,cData);
            break;
        case TRPG_GEOM_TEX32:
            buf.Get(bind);
            buf.Get(num);
            if (num < 0) throw 1;
            buf.GetArray(2*num,&fData);
            if (ALIGNMENT_WORKAROUND)
            {
                float32 *aligned;
                aligned = (float32 *)calloc (2*num, sizeof(float32));
                memcpy (aligned, fData, 2*num * sizeof(float32));
                geom->SetTexCoords(num,(trpgGeometry::BindType)bind,aligned);
                free (aligned);
            }
            else
                geom->SetTexCoords(num,(trpgGeometry::BindType)bind,fData);
            break;
        case TRPG_GEOM_TEX64:
            buf.Get(bind);
            buf.Get(num);
            if (num < 0) throw 1;
            buf.GetArray(2*num,&dData);
            if (ALIGNMENT_WORKAROUND)
            {
                float64 *aligned;
                aligned = (float64 *)calloc (2*num, sizeof(float64));
                memcpy (aligned, dData, 2*num * sizeof(float64));
                geom->SetTexCoords(num,(trpgGeometry::BindType)bind,aligned);
                free (aligned);
            }
            else
                geom->SetTexCoords(num,(trpgGeometry::BindType)bind,dData);
            break;
        case TRPG_GEOM_EFLAG:
            buf.Get(num);
            if (num < 0) throw 1;
            buf.GetArray(num,&charData);
            geom->SetEdgeFlags(num,charData);
            break;
        default:
            // Skip
            break;
        }
    }
    catch (...) {
        return NULL;
    }

    return geom;
}

// Read Geometry
bool trpgGeometry::Read(trpgReadBuffer &buf)
{
    trpgr_Parser parse;
    geomCB gcb;

    gcb.geom = this;
    parse.AddCallback(TRPG_GEOM_PRIM,&gcb,false);
    parse.AddCallback(TRPG_GEOM_MATERIAL,&gcb,false);
    parse.AddCallback(TRPG_GEOM_VERT32,&gcb,false);
    parse.AddCallback(TRPG_GEOM_VERT64,&gcb,false);
    parse.AddCallback(TRPG_GEOM_NORM32,&gcb,false);
    parse.AddCallback(TRPG_GEOM_NORM64,&gcb,false);
    parse.AddCallback(TRPG_GEOM_COLOR,&gcb,false);
    parse.AddCallback(TRPG_GEOM_TEX32,&gcb,false);
    parse.AddCallback(TRPG_GEOM_TEX64,&gcb,false);
    parse.AddCallback(TRPG_GEOM_EFLAG,&gcb,false);
    parse.Parse(buf);

    return isValid();
}
