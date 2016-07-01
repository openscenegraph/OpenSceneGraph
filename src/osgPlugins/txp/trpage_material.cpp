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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* trpage_material.cpp
   This source file contains the methods for trpgMatTable, trpgTextureEnv,
   trpgMaterial, and trpgTexTable.
   You should only modify this code if you want to add data to these classes.
*/

#include <trpage_geom.h>
#include <trpage_read.h>

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

/* Write Material Table class
   Keeps track of the materials that have been added.
*/

// Constructor
trpgMatTable::trpgMatTable()
{
    numTable = numMat = 0;
}
trpgMatTable::~trpgMatTable()
{
}

// Reset function
void trpgMatTable::Reset()
{
    numTable = 0;
    numMat = 0;
    materialMap.clear();
}

// Validity check
bool trpgMatTable::isValid() const
{
    if(materialMap.size()==0)
    return false;
    // get an iterator for the materialMap
    MaterialMapType::const_iterator itr = materialMap.begin();
    for (  ; itr != materialMap.end( ); itr++) {
        if(!(*itr).second.isValid()) {
            return false;
        }
    }

    return true;
}

// Set functions
void trpgMatTable::SetNumTable(int no)
{
    if ((no < 0) || (no==numTable))
        return;
    numTable = no;
}
void trpgMatTable::SetNumMaterial(int /*no*/)
{
    // This method is obsolete since we're using the project handle
    // and a map to store the materials, instead of a vector
}


void trpgMatTable::SetMaterial(int nm,const trpgMaterial &mat)
{
    materialMap[nm] = mat;
    numMat = materialMap.size();
}

#define CEQ(ca,cb) (ca.red == cb.red && ca.green == cb.green && ca.blue == cb.blue)

int trpgMatTable::AddMaterial(const trpgMaterial &mat,bool lookForExisting)
{
    trpgMaterial cmat = mat; // necessary?

    // having a shadeModel of 999 indicates that the entry is free.  I thought this would
    // work fine, until I realized that evidently most of the time the shademodel isn't set
    // at all.  Now my kludge takes so much work it's almost worth doing it right.

    if (cmat.shadeModel>100) cmat.shadeModel=trpgMaterial::Smooth;

    int baseMat=0;
    //bool spaceInTable=false;
    //int offset=baseMat;

    if (lookForExisting) {
        // Look for a matching base material minus the textures
        //for (baseMat = 0;baseMat < numMat;baseMat++) {
        MaterialMapType::const_iterator itr = materialMap.begin();
        for (  ; itr != materialMap.end( ); itr++) {
            baseMat = itr->first;
            const trpgMaterial &bm = itr->second;
            if (bm.shadeModel==999) {
                // this is an 'empty' entry.  Means we won't find it, either.
                //spaceInTable=true;
                break;
            }

            // Compare structures
            if (CEQ(cmat.color,bm.color) && CEQ(cmat.ambient,bm.ambient) &&
                CEQ(cmat.diffuse,bm.diffuse) && CEQ(cmat.specular,bm.specular) &&
                CEQ(cmat.emission,bm.emission) && cmat.shininess == bm.shininess &&
                cmat.shadeModel == bm.shadeModel && cmat.pointSize == bm.pointSize &&
                cmat.lineWidth == bm.lineWidth && cmat.cullMode == bm.cullMode &&
                cmat.alphaFunc == bm.alphaFunc && cmat.alphaRef == bm.alphaRef &&
                cmat.attrSet.fid == bm.attrSet.fid && cmat.attrSet.smc == bm.attrSet.smc &&
                cmat.attrSet.stp == bm.attrSet.stp && cmat.attrSet.swc == bm.attrSet.swc &&
                cmat.autoNormal == bm.autoNormal && cmat.texEnvs.size() == bm.texEnvs.size()) {
                // Test the texture envs
                bool isSame=true;
                unsigned int i;
                for (i=0;i<cmat.texEnvs.size();i++) {
                            const trpgTextureEnv &e1 = cmat.texEnvs[i];
                    const trpgTextureEnv &e2 = bm.texEnvs[i];
                    if (e1.envMode != e2.envMode ||
                    e1.minFilter != e2.minFilter ||
                    e1.magFilter != e2.magFilter ||
                    e1.wrapS != e2.wrapS || e1.wrapT != e2.wrapT ||
                    !CEQ(e1.borderCol,e2.borderCol))
                    isSame = false;
                }
                // Test the texture IDs
                for (i=0;i<cmat.texids.size();i++) {
                    if (cmat.texids[i] != bm.texids[i])
                    isSame = false;
                }
                if (isSame)
                    return baseMat;
            }
        }
    }

    // Didn't find it.  Add it
    int idx;
    if(cmat.writeHandle)
        idx = cmat.GetHandle();
    else
        idx = numMat;
    materialMap[idx] = cmat;
    numMat = materialMap.size();
    return idx;
}

// Write out material table
bool trpgMatTable::Write(trpgWriteBuffer &buf)
{
    if (!isValid())
        return false;

    buf.Begin(TRPGMATTABLE);

    // Total number of materials
    buf.Add((int32)numTable);
    buf.Add((int32)numMat);

    // Write the materials
    MaterialMapType::const_iterator itr = materialMap.begin();
    for (  ; itr != materialMap.end( ); itr++) {
        ((trpgMaterial)(*itr).second).Write(buf);
    }
    buf.End();

    return true;
}

/* ************
    Material Table Read
   ***********
   */

// Get functions
bool trpgMatTable::GetNumTable(int &no) const
{
    if (!isValid()) {
        no = 0; // otherwise this causes errors because it is uninitialized.
        return false;
    }
    no = numTable;
    return true;
}
bool trpgMatTable::GetNumMaterial(int &no) const
{
    if (!isValid()) {
        no = 0;
        return false;
    }
    no = numMat;
    return true;
}

bool trpgMatTable::GetMaterial(int nt,int nm,trpgMaterial &mat) const
{
    if (!isValid()) return false;

    MaterialMapType::const_iterator itr = materialMap.find((nt*numMat)+nm);
    if(itr == materialMap.end())
        return false;

    mat = (*itr).second;
    return true;
}

const trpgMaterial *trpgMatTable::GetMaterialRef(int nt,int nm) const
{
    MaterialMapType::const_iterator itr = materialMap.find((nt*numMat)+nm);
    if(itr == materialMap.end())
        return 0;
    return const_cast<trpgMaterial *>(&(*itr).second);
}


bool trpgMatTable::Read(trpgReadBuffer &buf)
{
    trpgMaterial mat;
    trpgToken matTok;
    int32 len;
    bool status;
    int i,j;
    int nMat,nTable;

    try {
        buf.Get(nTable);
        buf.Get(nMat);
        if (nTable <= 0 || nMat < 0) throw 1;
        // Read the materials
        for (i=0;i<nTable;i++)
            for (j=0;j<nMat;j++) {
                buf.GetToken(matTok,len);
                if (matTok != TRPGMATERIAL) throw 1;
                buf.PushLimit(len);
                mat.Reset();
                status = mat.Read(buf);
                buf.PopLimit();
                if (!status) throw 1;
                AddMaterial(mat,false);
            }
            numTable += nTable;
            numMat = materialMap.size();
    }
    catch (...) {
        return false;
    }

    return isValid();
}

/* Write Texture Environment class
    Used to specify how a texture is applied.
    Associated with materials.
    */

// Constructor
trpgTextureEnv::trpgTextureEnv()
{
    Reset();
}
trpgTextureEnv::~trpgTextureEnv()
{
}

// Reset function
void trpgTextureEnv::Reset()
{
    envMode = Decal;
    minFilter = Linear;
    magFilter = MipmapBilinear;
    wrapS = wrapT = Repeat;
    borderCol = trpgColor(0,0,0);
}

bool trpgTextureEnv::isValid() const
{
    return true;
}

// Set functions
void trpgTextureEnv::SetEnvMode(int mode)
{
    envMode = mode;
}
void trpgTextureEnv::SetMinFilter(int fl)
{
    minFilter = fl;
}
void trpgTextureEnv::SetMagFilter(int fl)
{
    magFilter = fl;
}
void trpgTextureEnv::SetWrap(int s,int t)
{
    wrapS = s;
    wrapT = t;
}
void trpgTextureEnv::SetBorderColor(const trpgColor &col)
{
    borderCol = col;
}

// Write function
bool trpgTextureEnv::Write(trpgWriteBuffer &buf)
{
    if (!isValid())
    return false;

    buf.Begin(TRPGMAT_TEXENV);

    buf.Begin(TRPGMAT_TXENV_MODE);
    buf.Add(envMode);
    buf.End();

    buf.Begin(TRPGMAT_TXENV_FILTER);
    buf.Add(minFilter);
    buf.Add(magFilter);
    buf.End();

    buf.Begin(TRPGMAT_TXENV_WRAP);
    buf.Add(wrapS);
    buf.Add(wrapT);
    buf.End();

    buf.Begin(TRPGMAT_TXENV_BORDER);
    buf.Add(borderCol);
    buf.End();

    buf.End();

    return true;
}

/* **************
   Texture Env Read support
   **************
   */
// Get functions
bool trpgTextureEnv::GetEnvMode(int32 &ret) const
{
    if (!isValid()) return false;
    ret = envMode;
    return true;
}
bool trpgTextureEnv::GetMinFilter(int32 &ret) const
{
    if (!isValid()) return false;
    ret = minFilter;
    return true;
}
bool trpgTextureEnv::GetMagFilter(int32 &ret) const
{
    if (!isValid()) return false;
    ret = magFilter;
    return true;
}
bool trpgTextureEnv::GetWrap(int &S,int &T) const
{
    if (!isValid()) return false;
    S = wrapS;
    T = wrapT;
    return true;
}
bool trpgTextureEnv::GetBorderColor(trpgColor &col) const
{
    if (!isValid()) return false;
    col = borderCol;
    return true;
}

/* Texture Env CB
   Used to parse tokens for a texture Env.
*/
class textureEnvCB : public trpgr_Callback {
public:
    void *Parse(trpgToken,trpgReadBuffer &);
    trpgTextureEnv *tenv;
};

void * textureEnvCB::Parse(trpgToken tok,trpgReadBuffer &buf)
{
    int envMode;
    int minFilter;
    int magFilter;
    int wrapS,wrapT;
    trpgColor borderCol;

    try {
        switch (tok) {
        case TRPGMAT_TXENV_MODE:
            buf.Get(envMode);
            tenv->SetEnvMode(envMode);
            break;
        case TRPGMAT_TXENV_FILTER:
            buf.Get(minFilter);
            buf.Get(magFilter);
            tenv->SetMinFilter(minFilter);
            tenv->SetMagFilter(magFilter);
            break;
        case TRPGMAT_TXENV_WRAP:
            buf.Get(wrapS);
            buf.Get(wrapT);
            tenv->SetWrap(wrapS,wrapT);
            break;
        case TRPGMAT_TXENV_BORDER:
            buf.Get(borderCol);
            tenv->SetBorderColor(borderCol);
            break;
        default:
            // Don't know this token.  Skip
            break;
        }
    }
    catch (...) {
    return NULL;
    }

    return tenv;
}

bool trpgTextureEnv::Read(trpgReadBuffer &buf)
{
    trpgr_Parser parse;
    textureEnvCB teCb;

    // Texture environment is a bunch of tokens in random order
    // Interface to it with a parser
    teCb.tenv = this;
    parse.AddCallback(TRPGMAT_TXENV_MODE,&teCb,false);
    parse.AddCallback(TRPGMAT_TXENV_FILTER,&teCb,false);
    parse.AddCallback(TRPGMAT_TXENV_WRAP,&teCb,false);
    parse.AddCallback(TRPGMAT_TXENV_BORDER,&teCb,false);
    parse.Parse(buf);

    return isValid();
}

/* Write Material class
   Material representation.
*/

// Constructor
trpgMaterial::trpgMaterial()
{
    Reset();
}
trpgMaterial::~trpgMaterial()
{
}

// Reset function
void trpgMaterial::Reset()
{
    color = trpgColor(1,1,1);
    ambient = trpgColor(0,0,0);
    diffuse = trpgColor(1,1,1);
    specular = trpgColor(0,0,0);
    emission = trpgColor(0,0,0);
    shininess = 0;
    shadeModel = 999; // kludge to identify 'empty' material table entries
    pointSize = 1;
    lineWidth = 1;
    cullMode = Back;
    alphaFunc = GreaterThan;
    alphaRef = 0;
    alpha = 1.0;
    autoNormal = false;
    numTex = 0;
    texids.resize(0);
    texEnvs.resize(0);
    numTile = 0;
    isBump = false;
    attrSet.fid = -1;
    attrSet.smc = -1;
    attrSet.stp = -1;
    attrSet.swc = -1;
    handle = -1;
    writeHandle = false;
}

// Validity check
bool trpgMaterial::isValid() const
{
    // Only thing we really care about is texture
    if (numTex < 0)
    return false;

    for (int i=0;i<numTex;i++)
    if (!texEnvs[i].isValid())
        return false;

    return true;
}



// Set functions
void trpgMaterial::SetColor(const trpgColor &col)
{
    color = col;
}
void trpgMaterial::SetAmbient(const trpgColor &col)
{
    ambient = col;
}
void trpgMaterial::SetDiffuse(const trpgColor &col)
{
    diffuse = col;
}
void trpgMaterial::SetSpecular(const trpgColor &col)
{
    specular = col;
}
void trpgMaterial::SetEmission(const trpgColor &col)
{
    emission = col;
}
void trpgMaterial::SetShininess(float64 val)
{
    shininess = val;
}
void trpgMaterial::SetShadeModel(int val)
{
    shadeModel = val;
}
void trpgMaterial::SetPointSize(float64 val)
{
    pointSize = val;
}
void trpgMaterial::SetLineWidth(float64 val)
{
    lineWidth = val;
}
void trpgMaterial::SetCullMode(int val)
{
    cullMode = val;
}
void trpgMaterial::SetAlphaFunc(int val)
{
    alphaFunc = val;
}
void trpgMaterial::SetAlphaRef(float64 val)
{
    alphaRef = val;
}
void trpgMaterial::SetAlpha(float64 val)
{
    alpha = val;
}
void trpgMaterial::SetAutoNormal(bool val)
{
    autoNormal = val;
}
void trpgMaterial::SetNumTexture(int no)
{
    if (no < 0)    return;
    numTex = no;
    texids.resize(no);
    texEnvs.resize(no);
}
void trpgMaterial::SetTexture(int no,int id,const trpgTextureEnv &env)
{
    if (no < 0 || (unsigned int)no >= texids.size())
    return;

    texids[no] = id;
    texEnvs[no] = env;
}
int trpgMaterial::AddTexture(int id,const trpgTextureEnv &env)
{
    texids.push_back(id);
    texEnvs.push_back(env);
    numTex++;

    return numTex-1;
}
void trpgMaterial::SetNumTiles(int no)
{
    numTile = no;
}
int trpgMaterial::AddTile()
{
    return(++numTile);
}
void trpgMaterial::SetIsBumpMap(bool val)
{
    isBump = val;
}
void trpgMaterial::SetAttr(int attrCode,int val)
{
    switch (attrCode) {
    case TR_FID:
        attrSet.fid = val;
        break;
    case TR_SMC:
        attrSet.smc = val;
        break;
    case TR_STP:
        attrSet.stp = val;
        break;
    case TR_SWC:
        attrSet.swc = val;
        break;
    }

    return;
}

// Write to buffer
bool trpgMaterial::Write(trpgWriteBuffer &buf)
{
    if (!isValid())
        return false;

    buf.Begin(TRPGMATERIAL);

    // Bundle the basic material parameters together
    buf.Begin(TRPGMAT_BASIC);
    buf.Add(color);
    buf.Add(ambient);
    buf.Add(diffuse);
    buf.Add(specular);
    buf.Add(emission);
    buf.Add(shininess);
    buf.Add(numTile);
    buf.End();

    // Most everything else is a single token.
    // This is inefficient, but expandable
    buf.Begin(TRPGMAT_SHADE);
    buf.Add(shadeModel);
    buf.End();

    buf.Begin(TRPGMAT_SIZES);
    buf.Add(pointSize);
    buf.Add(lineWidth);
    buf.End();

    buf.Begin(TRPGMAT_CULL);
    buf.Add(cullMode);
    buf.End();

    buf.Begin(TRPGMAT_ALPHA);
    buf.Add(alphaFunc);
    buf.Add(alphaRef);
    buf.Add(alpha);
    buf.End();

    buf.Begin(TRPGMAT_NORMAL);
    buf.Add((int32)autoNormal);
    buf.End();

    buf.Begin(TRPGMAT_TEXTURE);
    buf.Add(numTex);
    for (int i=0;i<numTex;i++) {
        buf.Add(texids[i]);
        texEnvs[i].Write(buf);
    }
    buf.End();

    // Bump mapping properties
    buf.Begin(TRPGMAT_BUMP);
    buf.Add((int32)isBump);
    buf.End();

    // Attributes (e.g. fid, smc)
    buf.Begin(TRPGMAT_ATTR);
    buf.Add(attrSet.fid);
    buf.Add(attrSet.smc);
    buf.Add(attrSet.stp);
    buf.Add(attrSet.swc);
    buf.End();

    /**
     * If the terrapage version is >= 2.3, handle will be set to a unique identifier.
     **/
    if(writeHandle) {
        buf.Begin(TRPGMAT_HANDLE);
        buf.Add((int)handle);
        buf.End();
    }

    buf.End();

    return true;
}


/* ***************
   Material Read methods
   ***************
   */
// Get methods
bool trpgMaterial::GetColor(trpgColor &col) const
{
    if (!isValid()) return false;
    col = color;
    return true;
}
bool trpgMaterial::GetAmbient(trpgColor &col) const
{
    if (!isValid()) return false;
    col = ambient;
    return true;
}
bool trpgMaterial::GetDiffuse(trpgColor &col) const
{
    if (!isValid()) return false;
    col = diffuse;
    return true;
}
bool trpgMaterial::GetSpecular(trpgColor &col) const
{
    if (!isValid()) return false;
    col = specular;
    return true;
}
bool trpgMaterial::GetEmission(trpgColor &col) const
{
    if (!isValid()) return false;
    col = emission;
    return true;
}
bool trpgMaterial::GetShininess(float64 &shin) const
{
    if (!isValid()) return false;
    shin = shininess;
    return true;
}
bool trpgMaterial::GetShadeModel(int &sm) const
{
    if (!isValid()) return false;
    sm = shadeModel;
    return true;
}
bool trpgMaterial::GetPointSize(float64 &ps) const
{
    if (!isValid()) return false;
    ps = pointSize;
    return true;
}
bool trpgMaterial::GetLineWidth(float64 &lw) const
{
    if (!isValid()) return false;
    lw = lineWidth;
    return true;
}
bool trpgMaterial::GetCullMode(int32 &cull) const
{
    if (!isValid()) return false;
    cull = cullMode;
    return true;
}
bool trpgMaterial::GetAlphaFunc(int &af) const
{
    if (!isValid()) return false;
    af = alphaFunc;
    return true;
}
bool trpgMaterial::GetAlphaRef(float64 &ar) const
{
    if (!isValid()) return false;
    ar = alphaRef;
    return true;
}
bool trpgMaterial::GetAlpha(float64 &a) const
{
    if (!isValid()) return false;
    a = alpha;
    return true;
}
bool trpgMaterial::GetAutoNormal(bool &an) const
{
    if (!isValid()) return false;
    an = autoNormal;
    return true;
}
bool trpgMaterial::GetNumTexture(int &no) const
{
    if (!isValid()) return false;
    no = numTex;
    return true;
}
bool trpgMaterial::GetTexture(int no,int &id,trpgTextureEnv &te) const
{
    if (!isValid() || no < 0  || no >= numTex)
        return false;
    id = texids[no];
    te = texEnvs[no];
    return true;
}
bool trpgMaterial::GetNumTile(int &no) const
{
    if (!isValid()) return false;
    no = numTile;
    return true;
}
bool trpgMaterial::GetIsBumpMap(bool &ret) const
{
    if (!isValid()) return false;
    ret = isBump;
    return true;
}
bool trpgMaterial::GetAttr(int attrCode,int &ret) const
{
    switch (attrCode) {
    case TR_FID:
        ret = attrSet.fid;
        break;
    case TR_SMC:
        ret = attrSet.smc;
        break;
    case TR_STP:
        ret = attrSet.stp;
        break;
    case TR_SWC:
        ret = attrSet.swc;
        break;
    default:
        return false;
    }

    return true;
}


/* Material CB
   Used to parse tokens for a material.
*/
class materialCB : public trpgr_Callback {
public:
    void * Parse(trpgToken,trpgReadBuffer &);
    trpgMaterial *mat;
};
void * materialCB::Parse(trpgToken tok,trpgReadBuffer &buf)
{
    trpgColor color;
    float64 shininess;
    int shadeModel;
    float64 size;
    int cullMode;
    int alphaFunc;
    float64 alphaRef,alpha;
    bool autoNormal;
    int numTex,texId;
    trpgToken envTok;
    trpgTextureEnv texEnv;
    int32 len,numtile;
    bool status;
    int i;

    try {
        switch (tok) {
        case TRPGMAT_BASIC:
            buf.Get(color);
            mat->SetColor(color);
            buf.Get(color);
            mat->SetAmbient(color);
            buf.Get(color);
            mat->SetDiffuse(color);
            buf.Get(color);
            mat->SetSpecular(color);
            buf.Get(color);
            mat->SetEmission(color);
            buf.Get(shininess);
            mat->SetShininess(shininess);
            buf.Get(numtile);
            mat->SetNumTiles(numtile);
            break;
        case TRPGMAT_SHADE:
            buf.Get(shadeModel);
            mat->SetShadeModel(shadeModel);
            break;
        case TRPGMAT_SIZES:
            buf.Get(size);
            mat->SetPointSize(size);
            buf.Get(size);
            mat->SetLineWidth(size);
            break;
        case TRPGMAT_CULL:
            buf.Get(cullMode);
            mat->SetCullMode(cullMode);
            break;
        case TRPGMAT_ALPHA:
            buf.Get(alphaFunc);
            buf.Get(alphaRef);
            buf.Get(alpha);
            mat->SetAlphaFunc(alphaFunc);
            mat->SetAlphaRef(alphaRef);
            mat->SetAlpha(alpha);
            break;
        case TRPGMAT_NORMAL:
        {
            int32 tmp;
            buf.Get(tmp);
            if (tmp)
            autoNormal = true;
            else
            autoNormal = false;
            mat->SetAutoNormal(autoNormal);
        }
        break;
        case TRPGMAT_TEXTURE:
            buf.Get(numTex);
            for (i=0;i<numTex;i++) {
                buf.Get(texId);
                // Parse the texture Env
                buf.GetToken(envTok,len);
                if (envTok != TRPGMAT_TEXENV)  throw 1;
                buf.PushLimit(len);
                status = texEnv.Read(buf);
                buf.PopLimit();
                if (!status) throw 1;

                mat->AddTexture(texId,texEnv);
            }
            break;
        case TRPGMAT_BUMP:
        {
            int32 tmp;
            buf.Get(tmp);
            bool isBump = (tmp) ? true : false;
            mat->SetIsBumpMap(isBump);
        }
        break;
        case TRPGMAT_ATTR:
        {
            int tmp;
            buf.Get(tmp);
            mat->SetAttr(trpgMaterial::TR_FID,tmp);
            buf.Get(tmp);
            mat->SetAttr(trpgMaterial::TR_SMC,tmp);
            buf.Get(tmp);
            mat->SetAttr(trpgMaterial::TR_STP,tmp);
            buf.Get(tmp);
            mat->SetAttr(trpgMaterial::TR_SWC,tmp);
        }
        break;
        case TRPGMAT_HANDLE:
        {
            int hdl;
            buf.Get(hdl);
            mat->SetHandle(hdl);
        }
        break;
        default:
            break;
        }
    }
    catch (...) {
        return NULL;
    }

    return mat;
}

bool trpgMaterial::Read(trpgReadBuffer &buf)
{
    trpgr_Parser parse;
    materialCB matCb;

    // Material is just a bunch of unordered tokens.
    // Interface to it with a generic parser
    matCb.mat = this;
    parse.AddCallback(TRPGMAT_BASIC,&matCb,false);
    parse.AddCallback(TRPGMAT_SHADE,&matCb,false);
    parse.AddCallback(TRPGMAT_SIZES,&matCb,false);
    parse.AddCallback(TRPGMAT_CULL,&matCb,false);
    parse.AddCallback(TRPGMAT_ALPHA,&matCb,false);
    parse.AddCallback(TRPGMAT_NORMAL,&matCb,false);
    parse.AddCallback(TRPGMAT_TEXTURE,&matCb,false);
    parse.AddCallback(TRPGMAT_BUMP,&matCb,false);
    parse.AddCallback(TRPGMAT_ATTR,&matCb,false);
    parse.AddCallback(TRPGMAT_HANDLE,&matCb,false);
    parse.Parse(buf);

    return isValid();
}

/* Texture
   Really just a container for a texture name and use count.
   The use count is used for paging.
*/

// Constructor
trpgTexture::trpgTexture()
{
    mode = External;
    type = trpg_Unknown;
    numLayer = -1;
    name = NULL;
    useCount = 0;
    sizeX = sizeY = -1;
    addr.file = 0;
    addr.offset = 0;
    addr.col = -1;
    addr.row = -1;
    isMipmap = false;
    numMipMap = 0;
    writeHandle = false;
    handle = -1;
}

// Copy construction
trpgTexture::trpgTexture(const trpgTexture &in):
    trpgReadWriteable(in)
{
    mode = in.mode;
    type = in.type;
    numLayer = in.numLayer; // RGBX
    name = NULL;
    SetName(in.name);
    useCount = in.useCount;
    sizeX = in.sizeX;  sizeY = in.sizeY;
    addr.file = in.addr.file;
    addr.offset = in.addr.offset;
    addr.row = in.addr.row;
    addr.col = in.addr.col;
    isMipmap = in.isMipmap;
    numMipMap = in.numMipMap;
    // storageSize + levelOffset
    handle = in.handle;
    writeHandle = in.writeHandle;
}

// Destruction
trpgTexture::~trpgTexture()
{
    Reset();
}

// Reset
void trpgTexture::Reset()
{
    mode = External;
    type = trpg_Unknown;
    numLayer = -1;
    if (name)
        delete [] name;
    name = NULL;
    useCount = 0;
    sizeX = sizeY = -1;
    addr.file = 0;
    addr.offset = 0;
    addr.row = -1;
    addr.col = -1;
    isMipmap = false;
    storageSize.clear();
    levelOffset.clear();
    handle = -1;
    writeHandle = false;
}




// Valid if we've got a name
bool trpgTexture::isValid() const
{
    switch (mode) {
    case External:
        return (name != NULL);
    case Local:
        return (type != trpg_Unknown && sizeX != -1 && sizeY != -1);
    case Global:
        return (type != trpg_Unknown);
    case Template:
        return (type != trpg_Unknown && sizeX != -1 && sizeY != -1);
    default:
        return false;
    }
}


// Set Name
void trpgTexture::SetName(const char *inName)
{
    if (name)
        delete [] name;
    name = NULL;

    if (!inName)
        return;

    name = new char[strlen(inName)+1];
    strcpy(name,inName);
}

// Get Name
bool trpgTexture::GetName(char *outName,int outLen) const
{
    if (!isValid())
        return false;

    if (!outName)
        return false;

    if (name)
    {
        int len = strlen(name);
        strncpy(outName,name,MIN(len,outLen)+1);
    }
    else
    {
        outName[0] = '\0';
    }
    return true;
}

void trpgTexture::SetImageMode(ImageMode inMode)
{
    mode = inMode;
}
bool trpgTexture::GetImageMode(ImageMode &outMode) const
{
    outMode = mode;

    return true;
}
void trpgTexture::SetImageType(ImageType inType)
{
    type = inType;
}
bool trpgTexture::GetImageType(ImageType &outType) const
{
    outType = type;

    return true;
}

void trpgTexture::SetImageSize(const trpg2iPoint &inSize)
{
    sizeX = inSize.x;
    sizeY = inSize.y;
}
bool trpgTexture::GetImageSize(trpg2iPoint &outSize) const
{
    if (mode != Local && mode != Template)
        return false;
    outSize.x = sizeX;
    outSize.y = sizeY;

    return true;
}
void trpgTexture::SetIsMipmap(bool val)
{
    isMipmap = val;
}
bool trpgTexture::GetIsMipmap(bool &ret) const
{
    ret = isMipmap;
    return true;
}
bool trpgTexture::GetImageAddr(trpgwAppAddress &outAddr) const
{
    if (mode != Local)
        return false;

    outAddr = addr;

    return true;
}
void trpgTexture::SetImageAddr(const trpgwAppAddress &inAddr)
{
    addr = inAddr;
}
bool trpgTexture::GetImageDepth(int32 &depth) const
{
    switch (type) {
    case trpg_RGB8:
        depth = 3;
        break;
    case trpg_RGBA8:
        depth = 4;
        break;
    case trpg_INT8:
        depth = 1;
        break;
    case trpg_INTA8:
        depth = 2;
        break;
    case trpg_FXT1:
        depth = 3;
        break;
    case trpg_RGBX:
        depth = numLayer;
        break;
    case trpg_DXT1:
        depth = 3;
        break;
    case trpg_DXT3:
        depth = 3;
        break;
    case trpg_DXT5:
        depth = 3;
        break;
    case trpg_MCM5:
        depth = 5;
        break;
    case trpg_MCM6R:
    case trpg_MCM6A:
        depth = 6;
        break;
    case trpg_MCM7RA:
    case trpg_MCM7AR:
        depth = 7;
        break;
    default:
        depth = -1;
        break;
    }

    return true;
}

void trpgTexture::SetNumLayer(int layers)
{
    numLayer = layers;
}

bool trpgTexture::GetNumLayer(int &layers) const
{
    if (!isValid())
        return false;
    GetImageDepth(layers);
    return true;
}

// Use count management
void trpgTexture::SetNumTile(int num)
{
    useCount = num;
}
void trpgTexture::AddTile()
{
    useCount++;
}
bool trpgTexture::GetNumTile(int &num) const
{
    if (!isValid())
        return false;
    num = useCount;
    return true;
}

// Copy operator
trpgTexture &trpgTexture::operator = (const trpgTexture &in)
{
    mode = in.mode;
    type = in.type;

    if (in.name)
        SetName(in.name);

    useCount = in.useCount;

    sizeX = in.sizeX;
    sizeY = in.sizeY;

    // RGBX
    numLayer = in.numLayer;

    isMipmap = in.isMipmap;
    addr = in.addr;

    writeHandle = in.writeHandle;
    handle = in.handle;

    return *this;
}

// Equality operator
int trpgTexture::operator == (const trpgTexture &in) const
{
    if (mode != in.mode)
        return 0;

    switch (mode) {
    case External:
        if (!in.name && !name)
            return 1;
        if (!in.name || !name)
            return 0;
        return (!strcmp(in.name,name));
        break;
    case Local:
        if (type == in.type && sizeX == in.sizeX && sizeY == in.sizeY &&
            isMipmap == in.isMipmap &&
            addr.file == in.addr.file && addr.offset == in.addr.offset &&
            addr.row == in.addr.row && addr.col==in.addr.col )
            return 1;
        break;
    case Global:
    case Template:
        if (type == in.type && sizeX == in.sizeX && sizeY == in.sizeY &&
            isMipmap == in.isMipmap)
            return 1;
        break;
    }

    return 0;
}

// Utility functions

int32 trpgTexture::CalcNumMipmaps() const
{
    // We're going to assume these are powers of two.
    // If not, then the writer's a moron.

    // :))) The comment line above made me really loughing, Steve. - Nick
    int bval = MAX(sizeX,sizeY);

    // Now look for the highest bit
    int p2;
    for (p2=0;p2<32;p2++)
        if ((1<<p2) & bval)
            break;

    return p2+1;
}


// Note: replacing this with explicit sizes
int32 trpgTexture::CalcTotalSize() const
{
    (const_cast<trpgTexture *>(this))->CalcMipLevelSizes();

    int totSize = 0;
    for (unsigned int i=0;i<storageSize.size();i++)
        totSize += storageSize[i];

    return totSize;
}

// Calculate the size of a given mip level
int32 trpgTexture::MipLevelSize(int miplevel)
{

    if ( miplevel >= 0 && miplevel < CalcNumMipmaps() ) {
        if ( !storageSize.size() )
            CalcMipLevelSizes();
        return storageSize[miplevel];
    }

    return 0;
}

int32 trpgTexture::MipLevelOffset(int miplevel)
{
    if ( miplevel > 0 && miplevel < CalcNumMipmaps() ) {
        if ( !levelOffset.size() )
            CalcMipLevelSizes();
        return levelOffset[miplevel];
    }

    return 0;
}


// Write function
bool trpgTexture::Write(trpgWriteBuffer &buf)
{
    if (!isValid()) return false;

    buf.Begin(TRPGTEXTURE);

    buf.Add(name);
    buf.Add(useCount);
    // New in 2.0 from here down
    buf.Add((unsigned char)mode);
    buf.Add((unsigned char)type);
    buf.Add(sizeX);
    buf.Add(sizeY);
    buf.Add(addr.file);
    buf.Add(addr.offset);
    buf.Add((int32)isMipmap);
    if(writeHandle) {
        buf.Add((int32)handle);
    }
    buf.End();

    return true;
}

// Read function
bool trpgTexture::Read(trpgReadBuffer &buf)
{
    char texName[1024];

    try {
        buf.Get(texName,1023);
        SetName(texName);
        buf.Get(useCount);

        mode = External;
        // New in 2.0 from here down
        unsigned char bval;
        buf.Get(bval);  mode = (trpgTexture::ImageMode)bval;
        buf.Get(bval);    type = (trpgTexture::ImageType)bval;
        GetImageDepth(numLayer); // heh
        buf.Get(sizeX);
        buf.Get(sizeY);
        buf.Get(addr.file);
        buf.Get(addr.offset);
        int32 ival;
        buf.Get(ival);
        // Read the handle if we can..
        try {
            int32 tempHandle;
            if(buf.Get(tempHandle))
            {
                writeHandle = true;
                handle = tempHandle;
            }
            else
            {
                handle = -1;
            }
        }
        catch (...) {
            handle = -1;
        }
        isMipmap = (ival) ? true : false;
    }
    catch (...) {
        return false;
    }

    if (!isValid()) return false;

    // calculate the mip level sizes
    CalcMipLevelSizes();

    return true;
}

void trpgTexture::CalcMipLevelSizes()
{
    int num_miplevels = (isMipmap ? CalcNumMipmaps() : 1);
    int level_size = 0;
    int level_offset = 0;
    int block_size = 0;
    int pixel_size = 0;
    int pad_size = 0;
    bool isDXT = false;
    bool isFXT = false;

    switch (type) {
    case trpg_DXT1:
        isDXT = true;
        block_size = 8;
        break;
    case trpg_DXT3:
    case trpg_DXT5:
        isDXT = true;
        block_size = 16;
        break;
    case trpg_RGB8:
        pad_size = 4;
        pixel_size = 3;
        break;
    case trpg_RGBA8:
        pad_size = 4;
        pixel_size = 4;
        break;
    case trpg_RGBX:
        pad_size = 4;
        pixel_size = numLayer;
        break;
    case trpg_MCM5:
        pad_size = 4;
        pixel_size = 5;
        break;
    case trpg_MCM6R:
    case trpg_MCM6A:
        pad_size = 4;
        pixel_size = 6;
        break;
    case trpg_MCM7RA:
    case trpg_MCM7AR:
        pad_size = 4;
        pixel_size = 7;
        break;
    case trpg_INT8:
        pad_size = 4;
        pixel_size = 1;
        break;
    case trpg_INTA8:
        pad_size = 4;
        pixel_size = 2;
        break;
    case trpg_FXT1:
        isFXT = true;
        break;
    default:
        break;
    }


    levelOffset.clear();
    storageSize.clear();

    levelOffset.push_back(level_offset);



    if ( isDXT ) { // DXT compressed
        int num_x_blocks = ((sizeX/4)+(sizeX%4?1:0));
        int num_y_blocks = ((sizeY/4)+(sizeY%4?1:0));

        level_size = num_x_blocks * num_y_blocks * block_size;
        storageSize.push_back(level_size);

        for ( int i = 1; i < num_miplevels; i++ ) {
            level_offset += level_size;
            levelOffset.push_back(level_offset);

            num_x_blocks /= 2;
            num_y_blocks /= 2;
            num_x_blocks = MAX(1,num_x_blocks);
            num_y_blocks = MAX(1,num_y_blocks);

            level_size = num_x_blocks * num_y_blocks * block_size;
            storageSize.push_back(level_size);
        }

        return;
    }
    if ( isFXT) {
        // bits per pixel and size
        int bpp = 4;
        int x = sizeX;
        int y = sizeY;

        int nummiplevels = (isMipmap ? CalcNumMipmaps() : 1);
        for (int i = 0; i < nummiplevels; i++) {
            if (i > 0)
            levelOffset.push_back(level_offset);

            x = ( x + 0x7 ) & ~0x7;
            y = ( y + 0x3 ) & ~0x3;

            // Number of bytes
            level_size = ( x * y * bpp ) >> 3;
            storageSize.push_back(level_size);
            level_offset += level_size;

            if (x > 1)  x /= 2;
            if (y > 1)  y /= 2;
        }

        return;
    }

    {
        int x_size = sizeX;
        int y_size = sizeY;

        // Pad to a given size, if necessary
        int row_size = x_size * pixel_size;
        if (pad_size > 0) {
            int left = row_size%pad_size;
            if (left)
                row_size += pad_size - left;
        }

        level_size = row_size * y_size;
        storageSize.push_back(level_size);
        for ( int i = 1; i < num_miplevels; i++ ) {
            level_offset += level_size;
            levelOffset.push_back(level_offset);

            x_size /= 2;
            y_size /= 2;
            x_size = MAX(1,x_size);
            y_size = MAX(1,y_size);

            row_size = x_size * pixel_size;
            if (pad_size > 0) {
                int left = row_size%pad_size;
                if (left)
                    row_size += pad_size - left;
            }
            level_size = row_size * y_size;
            storageSize.push_back(level_size);
        }
    }
}

/* Texture Table
   Just a list of texture names so we can index.
*/

// Constructor
trpgTexTable::trpgTexTable()
{
    currentRow = -1;
    currentCol = -1;
}
trpgTexTable::trpgTexTable(const trpgTexTable &in):
    trpgReadWriteable(in)
{
    *this = in;
}

// Reset function
void trpgTexTable::Reset()
{
    errMess[0] = '\0';
    textureMap.clear();
    currentRow = -1;
    currentCol = -1;
}

// Destructor
trpgTexTable::~trpgTexTable()
{
    Reset();
}

// Validity check
bool trpgTexTable::isValid() const
{
    if (!textureMap.size())
    {
        errMess.assign("Texture table list is empty");
        return false;
    }

    TextureMapType::const_iterator itr = textureMap.begin();
    for (  ; itr != textureMap.end( ); itr++) {
        if(!itr->second.isValid()) {
            errMess.assign("A texture in the texture table is invalid");
            return false;
        }
    }
    return true;
}


// Set functions
void trpgTexTable::SetNumTextures(int /*no*/)
{
    // obsolete. This method doesn't need to do anything since we're using a map instead of a vector.
    //    texList.resize(no);
}
int trpgTexTable::AddTexture(const trpgTexture &inTex)
{

    TeAttrHdl hdl = inTex.GetHandle();
    if(hdl==-1) {
        // if no handle is specified, we will use an index as the handle (just like before 2.3)
        hdl = textureMap.size();
    }
    TextureMapType::iterator itr = textureMap.find(hdl);
    // Don't overwrite the texture if it was already there
    if(itr==textureMap.end())
        textureMap[hdl] = inTex;
    return hdl;
}
int trpgTexTable::FindAddTexture(const trpgTexture &inTex)
{
    TextureMapType::iterator itr = textureMap.begin();
    for (  ; itr != textureMap.end( ); itr++) {
        trpgTexture tx = itr->second;
        if(tx == inTex) {
            return itr->first;
        }
    }
    return AddTexture(inTex);
}
void trpgTexTable::SetTexture(int id,const trpgTexture &inTex)
{
    if (id < 0)
        return;
    textureMap[id] = inTex;
}

// Copy operator
trpgTexTable &trpgTexTable::operator = (const trpgTexTable &in)
{
    Reset();
    TextureMapType::const_iterator itr = in.textureMap.begin();
    for (  ; itr != in.textureMap.end( ); itr++) {
        trpgTexture tex = itr->second;
        in.GetTexture(itr->first,tex);
        AddTexture(tex);
    }

    return *this;
}

// Write Texture table
bool trpgTexTable::Write(trpgWriteBuffer &buf)
{
    int32 numTex;

    if (!isValid())
        return false;

    buf.Begin(TRPGTEXTABLE2);
    numTex = textureMap.size();
    buf.Add(numTex);
    TextureMapType::iterator itr = textureMap.begin();
    for (  ; itr != textureMap.end( ); itr++) {
        itr->second.Write(buf);
    }
    buf.End();

    return true;
}

/* ***********
   Read Texture Table
   ***********
   */
// Get functions
bool trpgTexTable::GetNumTextures(int &no) const
{
    no = textureMap.size();
    if (!isValid()) return false;
    return true;
}
bool trpgTexTable::GetTexture(int id,trpgTexture &ret) const
{
    if (!isValid())
        return false;
    if (id < 0)
        return false;
    TextureMapType::const_iterator itr = textureMap.find(id);
    if(itr == textureMap.end()) {
        return false;
    }
    ret = itr->second;

    return true;
}
const trpgTexture *trpgTexTable::GetTextureRef(int id) const
{
    if (id < 0)
        return 0;
    TextureMapType::const_iterator itr = textureMap.find(id);
    if(itr == textureMap.end()) {
        return 0;
    }
    const trpgTexture *ret = &(itr->second);
    return ret;
}

const trpgTexture *trpgTexTable::FindByName(const char *name, int &texid) const
{
    TextureMapType::const_iterator itr = textureMap.begin();
    for (  ; itr != textureMap.end( ); itr++) {
        char thisName[1024];
        thisName[0] = '\0';
        itr->second.GetName(thisName,1023);
        if(strcasecmp(thisName,name)==0)
        {
            texid = itr->first;
            return &(itr->second);
        }
    }
    return 0;

}

bool trpgTexTable::Read(trpgReadBuffer &buf)
{
    int32 numTex;
    trpgToken texTok;
    int32 len;

    try {
        buf.Get(numTex);

        for (int i=0;i<numTex;i++) {
            buf.GetToken(texTok,len);
            if (texTok != TRPGTEXTURE) throw 1;
            buf.PushLimit(len);
            trpgTexture tex;
            bool status = tex.Read(buf);
            //set the block for multi-archive archives (version>=2.3)
            if((currentRow!=-1)&&(currentCol!=-1)) {
                trpgwAppAddress taddr;
                tex.GetImageAddr(taddr);
                taddr.col = currentCol;
                taddr.row = currentRow;
                tex.SetImageAddr(taddr);
            }
            AddTexture(tex);
            buf.PopLimit();
            if (!status) throw 1;
        }
    }
    catch (...) {
        return false;
    }

    return true;
}


/* **************
   Local Material
   **************
   */

trpgLocalMaterial::trpgLocalMaterial()
{
    baseMatTable = -1;
    baseMat = -1;
    sx = sy = ex = ey = destWidth = destHeight = 0;
    addr.resize(1);
    addr[0].file = 0;
    addr[0].offset = 0;
    addr[0].col = -1;
    addr[0].row = -1;
}

trpgLocalMaterial::~trpgLocalMaterial()
{
}

void trpgLocalMaterial::SetBaseMaterial(int32 subTable,int32 inMatID)
{
    baseMatTable = subTable;
    baseMat = inMatID;
}

void trpgLocalMaterial::Reset()
{
    baseMat = -1;
    sx = sy = ex = ey = destWidth = destHeight = 0;
//    storageSize.resize(0);
    addr.resize(1);
    addr[0].file = 0;
    addr[0].offset = 0;
    addr[0].col = -1;
    addr[0].row = -1;
}

bool trpgLocalMaterial::GetBaseMaterial(int32 &subTable,int32 &matID) const
{
    if (!isValid()) return false;

    subTable = baseMatTable;
    matID = baseMat;
    return true;
}

void trpgLocalMaterial::SetSubImageInfo(const SubImageInfo &info)
{
    sx = info.sx;
    sy = info.sy;
    ex = info.ex;
    ey = info.ey;
    destWidth = info.destWidth;
    destHeight = info.destHeight;
}

bool trpgLocalMaterial::GetSubImageInfo(SubImageInfo &info) const
{
    if (!isValid()) return false;

    info.sx = sx;
    info.sy = sy;
    info.ex = ex;
    info.ey = ey;
    info.destWidth = destWidth;
    info.destHeight = destHeight;

    return true;
}

void trpgLocalMaterial::SetAddr(const trpgwAppAddress &inAddr)
{
    addr[0] = inAddr;
}

void trpgLocalMaterial::SetNthAddr(unsigned int subtable, const trpgwAppAddress &inAddr)
{
    if (addr.size()<=subtable) addr.resize(subtable+1);
    addr[subtable] = inAddr;
}

bool trpgLocalMaterial::GetAddr(trpgwAppAddress &inAddr) const
{
    if (!isValid()) return false;

    inAddr = addr[0];
    return true;
}

bool trpgLocalMaterial::GetNthAddr(unsigned int subtable, trpgwAppAddress &inAddr) const
{
    if (!isValid()) return false;
    if (addr.size()<=subtable) return false;
    inAddr = addr[subtable];
    return true;
}

bool trpgLocalMaterial::GetNumLocals(int &numLocals) const
{
    if (!isValid()) return false;
    // not checking for tile local, that's up to you.
    numLocals=addr.size();
    return true;
}

/*bool trpgLocalMaterial::SetStorageSizes(int level,vector<int> *inSizes)
  {
  storageSize.resize(inSizes->size());
  for (int i=0;i<inSizes->size();i++)
  storageSize[i] = (*inSizes)[i];

  return true;
  }*/

/*bool trpgLocalMaterial::GetStorageSizes(const vector<int> *retSize)
  {
  if (!isValid()) return false;

  retSize = storageSize;
  return true;
  }*/

bool trpgLocalMaterial::isValid() const
{
    if (baseMat < 0) return false;

    return true;
}

// Write method

bool trpgLocalMaterial::Write(trpgWriteBuffer &buf)
{
    if (!isValid())
        return false;

    buf.Begin(TRPGLOCALMATERIAL);

    // Write the data
    buf.Add(baseMatTable);
    buf.Add(baseMat);
    buf.Add(sx);
    buf.Add(sy);
    buf.Add(ex);
    buf.Add(ey);
    buf.Add(destWidth);
    buf.Add(destHeight);
    buf.Add(addr[0].file);
    buf.Add(addr[0].offset);
    // and in case there's more...
    int numAddrs=(int)(addr.size());
    if (numAddrs>1) {
        buf.Add(numAddrs-1); // suppressed due to breaking old readers.
        for (int i=1;i<numAddrs;i++) {
            buf.Add(addr[i].file);
            buf.Add(addr[i].offset);
        }
    }
    buf.End();

    return true;
}

// Read method

bool trpgLocalMaterial::Read(trpgReadBuffer &buf)
{
    try {
        buf.Get(baseMatTable);
        buf.Get(baseMat);
        buf.Get(sx);
        buf.Get(sy);
        buf.Get(ex);
        buf.Get(ey);
        buf.Get(destWidth);
        buf.Get(destHeight);
        buf.Get(addr[0].file);
        buf.Get(addr[0].offset);
        if ( !buf.isEmpty() ) {
            // there might be more
            int extraAddrs;
            buf.Get(extraAddrs);
            if (extraAddrs!=0) {
                addr.resize(extraAddrs+1);
                for (int i=1;i<=extraAddrs;i++) {
                    buf.Get(addr[i].file);
                    buf.Get(addr[i].offset);
                    addr[i].col = -1;
                    addr[i].row = -1;
                }
            }
        }
    }
    catch (...) {
        return false;
    }

    return isValid();
}
