/* ************************
   Copyright Terrain Experts Inc.
   Terrain Experts Inc (TERREX) reserves all rights to this source code
   unless otherwise specified in writing by the Chief Operating Officer
   of TERREX.
   This copyright may be updated in the future, in which case that version
   supercedes this one.
   -------------------
   Terrex Experts Inc.
   84 West Santa Clara St., Suite 380
   San Jose, CA 95113
   info@terrex.com
   Tel: (408) 293-9977
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

#include "trpage_geom.h"
#include "trpage_read.h"

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
	baseMats.resize(0);
	matTables.resize(0);
}

// Validity check
bool trpgMatTable::isValid() const
{
	if (numTable <= 0 || numMat <= 0)
		return false;

	for (unsigned int i=0;i<baseMats.size();i++)
		if (!baseMats[i].isValid())
			return false;

	return true;
}

// Set functions
void trpgMatTable::SetNumTable(int no)
{
	if (no < 0)
		return;
	numTable = no;
	matTables.resize(numTable*numMat);
}
void trpgMatTable::SetNumMaterial(int no)
{
	if (no < 0)
		return;

	// Note: Doesn't preserve the order of what was there
	numMat = no;
	matTables.resize(numMat*numTable);
}
void trpgMatTable::SetMaterial(int nt,int nm,const trpgMaterial &mat)
{
	if (nt < 0 || nt >= numTable)
		return;
	if (nm < 0 || nm >= numMat)
		return;
#ifdef FIX
	tables[nt*numMat+nm] = mat;
#endif
}
void trpgMatTable::SetMaterial(int nm,const trpgMaterial &mat)
{
	if (nm < 0 || nm >= numMat)
		return;

#ifdef FIX
	for (int i=0;i<numTable;i++)
		tables[i*numMat+nm] = mat;
#endif
}
#define CEQ(ca,cb) (ca.red == cb.red && ca.green == cb.green && ca.blue == cb.blue)
int trpgMatTable::AddMaterial(const trpgMaterial &mat)
{
	// Doesn't work if there's more than one table
	if (numTable != 1)
		return -1;

	// Look for a matching base material minus the textures
	trpgMaterial cmat = mat;
	cmat.texids.resize(0);
	uint32 baseMat;
	for (baseMat = 0;baseMat < baseMats.size();baseMat++) {
		trpgMaterial &bm = baseMats[baseMat];
		// Compare structures
		if (CEQ(cmat.color,bm.color) && CEQ(cmat.ambient,bm.ambient) &&
			CEQ(cmat.diffuse,bm.diffuse) && CEQ(cmat.specular,bm.specular) &&
			CEQ(cmat.emission,bm.emission) && cmat.shininess == bm.shininess &&
			cmat.shadeModel == bm.shadeModel && cmat.pointSize == bm.pointSize &&
			cmat.lineWidth == bm.lineWidth && cmat.cullMode == bm.cullMode &&
			cmat.alphaFunc == bm.alphaFunc && cmat.alphaRef == bm.alphaRef &&
			cmat.autoNormal == bm.autoNormal && cmat.texEnvs.size() == bm.texEnvs.size()) {
			// Test the texture envs
			bool isSame=true;
			for (unsigned int i=0;i<cmat.texEnvs.size();i++) {
				trpgTextureEnv &e1 = cmat.texEnvs[i];
				trpgTextureEnv &e2 = bm.texEnvs[i];
				if (e1.envMode != e2.envMode ||
					e1.minFilter != e2.minFilter ||
					e1.magFilter != e2.magFilter ||
					e1.wrapS != e2.wrapS || e1.wrapT != e2.wrapT ||
					!CEQ(e1.borderCol,e2.borderCol))
					isSame = false;
			}
			if (isSame)
				break;
		}
	}
	// Didn't find it.  Add it
	if (baseMat >= baseMats.size())
		baseMats.push_back(cmat);

	// Add a short material
	trpgShortMaterial smat;
	smat.baseMat = baseMat;
	smat.texids = mat.texids;
	matTables.push_back(smat);

	numMat++;

	return numMat-1;
}

// Write out material table
bool trpgMatTable::Write(trpgWriteBuffer &buf)
{
	int i,j;

	if (!isValid())
		return false;

	buf.Begin(TRPGMATTABLE2);

	// Total number of materials
	buf.Add((int32)numTable);
	buf.Add((int32)numMat);

	// Dump the short table in first
	// These are small materials.  They reference the base materials
	buf.Begin(TRPGSHORTMATTABLE);
	for (i=0;i<numTable;i++)
		for (j=0;j<numMat;j++) {
			trpgShortMaterial &smat = matTables[i*numMat+j];
			buf.Add((int32)smat.baseMat);
			buf.Add((int32)smat.texids.size());
			for (unsigned int k=0;k<smat.texids.size();k++)
				buf.Add((int32)smat.texids[k]);
		}
	buf.End();

	// Write the base materials
	buf.Add((int32)baseMats.size());
	for (i=0;i<(int)baseMats.size();i++)
		baseMats[i].Write(buf);
	buf.End();

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
	if (!isValid()) return false;
	no = numTable;
	return true;
}
bool trpgMatTable::GetNumMaterial(int &no) const
{
	if (!isValid()) return false;
	no = numMat;
	return true;
}
bool trpgMatTable::GetMaterial(int nt,int nm,trpgMaterial &mat) const
{
	if (!isValid()) return false;
	if (nt < 0 || nt >= numTable || nm < 0 || nm >= numMat)
		return false;
	const trpgShortMaterial *smat = &matTables[nt*numMat+nm];
	mat = baseMats[smat->baseMat];
	// Now do the overrides for the short material
	mat.texids = smat->texids;

	return true;
}
trpgMaterial *trpgMatTable::GetMaterialRef(int nt,int nm)
{
	if (nt < 0 || nt >= numTable || nm < 0 || nm >= numMat)
		return 0;
	GetMaterial(nt,nm,matRef);
	return &matRef;
}

// This parses both MATTABLE and MATTABLE2
bool trpgMatTable::Read(trpgReadBuffer &buf)
{
	trpgMaterial mat;
	trpgToken matTok;
	int32 len;
	bool status;
	int i,j,k;

	try {
		buf.Get(numTable);
		buf.Get(numMat);
		if (numTable <= 0 || numMat < 0) throw 1;

		// Short material tables are always full size
		matTables.resize(numTable*numMat);

		// Look for short material table
		buf.GetToken(matTok,len);
		if (matTok == TRPGSHORTMATTABLE) {
			int32 numTex,texId;
			buf.PushLimit(len);
			for (i=0;i<numTable;i++)
				for (j=0;j<numMat;j++) {
					trpgShortMaterial &smat = matTables[i*numMat+j];
					buf.Get(smat.baseMat);
					buf.Get(numTex);
					for (k=0;k<numTex;k++) {
						buf.Get(texId);
						smat.texids.push_back(texId);
					}
				}
			buf.PopLimit();

			// Now read the base materials
			int32 numBaseMat;
			buf.Get(numBaseMat);
			if (numBaseMat < 0) throw 1;
			baseMats.resize(numBaseMat);
			for (i=0;i<numBaseMat;i++) {
				buf.GetToken(matTok,len);
				if (matTok != TRPGMATERIAL) throw 1;
				buf.PushLimit(len);
				mat.Reset();
				status = mat.Read(buf);
				buf.PopLimit();
				if (!status) throw 1;
				baseMats[i] = mat;
			}
		} else {
			// Didn't find it.  Read the old style
			baseMats.resize(numTable*numMat);
			for (i=0;i<numTable;i++)
				for (j=0;j<numMat;j++) {
					if (i != 0 && j != 0)
						buf.GetToken(matTok,len);
					if (matTok != TRPGMATERIAL) throw 1;
					buf.PushLimit(len);
					mat.Reset();
					status = mat.Read(buf);
					buf.PopLimit();
					if (!status) throw 1;
					baseMats[i*numMat+j] = mat;
					// Set up fake short material for old style material table
					trpgShortMaterial &smat = matTables[i*numMat+j];
					smat.baseMat = i*numMat+j;
					smat.texids = baseMats[i*numMat+j].texids;
				}
		}
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
	shadeModel = Smooth;
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
	if (no < 0)	return;
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
	buf.Add(autoNormal);
	buf.End();

	buf.Begin(TRPGMAT_TEXTURE);
	buf.Add(numTex);
	for (int i=0;i<numTex;i++) {
		buf.Add(texids[i]);
		texEnvs[i].Write(buf);
	}
	buf.End();

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
				int tmp;
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
	name = NULL;
	useCount = 0;
}

// Copy construction
trpgTexture::trpgTexture(const trpgTexture &in)
{
	name = NULL;
	useCount = 0;
	*this = in;
}

// Destruction
trpgTexture::~trpgTexture()
{
	Reset();
}

// Reset
void trpgTexture::Reset()
{
	if (name)
		delete name;
	name = NULL;
	useCount = 0;
}

// Valid if we've got a name
bool trpgTexture::isValid() const
{
	if (name)
		return true;

	return false;
}

// Set Name
void trpgTexture::SetName(const char *inName)
{
	if (name)
		delete name;
	if (!inName || strlen(inName) < 1)
		return;

	name = new char[strlen(inName)+1];
	strcpy(name,inName);
}

// Get Name
bool trpgTexture::GetName(char *outName,int outLen) const
{
	if (!isValid()) return false;

	int len = (name) ? strlen(name) : 0;
	strncpy(outName,name,MIN(len,outLen)+1);

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
	if (!isValid()) return false;
	num = useCount;
	return true;
}

// Copy operator
trpgTexture &trpgTexture::operator = (const trpgTexture &in)
{
	SetName(in.name);
	useCount = in.useCount;

	return *this;
}

// Equality operator
int trpgTexture::operator == (const trpgTexture &in) const
{
	if (!in.name && !name)
		return 1;
	if (!in.name || !name)
		return 0;
	return (!strcmp(in.name,name));
}

// Write function
bool trpgTexture::Write(trpgWriteBuffer &buf)
{
	if (!isValid()) return false;

	buf.Add(name);
	buf.Add(useCount);

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
	}
	catch (...) {
		return false;
	}

	if (!isValid()) return false;
	return true;
}

/* Texture Table
	Just a list of texture names so we can index.
	*/

// Constructor
trpgTexTable::trpgTexTable()
{
}
trpgTexTable::trpgTexTable(const trpgTexTable &in)
{
	*this = in;
}

// Reset function
void trpgTexTable::Reset()
{
	texList.resize(0);
}

// Destructor
trpgTexTable::~trpgTexTable()
{
	Reset();
}

// Validity check
bool trpgTexTable::isValid() const
{
	for (unsigned int i=0;i<texList.size();i++)
		if (!texList[i].isValid())
			return false;

	return true;
}

// Set functions
void trpgTexTable::SetNumTextures(int no)
{
	texList.resize(no);
}
int trpgTexTable::AddTexture(const trpgTexture &inTex)
{
	texList.push_back(inTex);
	return texList.size()-1;
}
int trpgTexTable::FindAddTexture(const trpgTexture &inTex)
{
	for (unsigned int i=0;i<texList.size();i++)
		if (texList[i] == inTex)
			return (int)i;

	return AddTexture(inTex);
}
void trpgTexTable::SetTexture(int id,const trpgTexture &inTex)
{
	if (id < 0 || id >= (int)texList.size())
		return;

	texList[id] = inTex;
}

// Copy operator
trpgTexTable &trpgTexTable::operator = (const trpgTexTable &in)
{
	for (unsigned int i=0;i<in.texList.size();i++)
		AddTexture(in.texList[i]);

	return *this;
}

// Write Texture table
bool trpgTexTable::Write(trpgWriteBuffer &buf)
{
	int32 numTex;

	if (!isValid())
		return false;

	buf.Begin(TRPGTEXTABLE);
	numTex = texList.size();
	buf.Add(numTex);
	for (unsigned int i=0;i<texList.size();i++)
		texList[i].Write(buf);
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
	if (!isValid()) return false;
	no = texList.size();
	return true;
}
bool trpgTexTable::GetTexture(int id,trpgTexture &ret) const
{
	if (!isValid()) return false;
	if (id < 0 || id >= (int)texList.size()) return false;

	ret = texList[id];
	return true;
}
trpgTexture *trpgTexTable::GetTextureRef(int id)
{
	if (id < 0 || id >= (int)texList.size()) return NULL;
	return &texList[id];
}

bool trpgTexTable::Read(trpgReadBuffer &buf)
{
	int32 numTex;
	trpgTexture tex;

	try {
		buf.Get(numTex);
		for (int i=0;i<numTex;i++) {
			tex.Read(buf);
			texList.push_back(tex);
		}
	}
	catch (...) {
		return false;
	}

	return true;
}
