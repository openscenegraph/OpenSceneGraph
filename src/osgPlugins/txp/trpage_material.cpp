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
	matTables.resize(0);
}

// Validity check
bool trpgMatTable::isValid() const
{
	if (numTable <= 0 || numMat <= 0)
		return false;

	for (int i=0;i<matTables.size();i++)
		if (!matTables[i].isValid())
			return false;

	return true;
}

// Set functions
void trpgMatTable::SetNumTable(int no)
{
	if ((no < 0) || (no==numTable))
		return;
	numTable = no;
	matTables.resize(numTable*numMat);
}
void trpgMatTable::SetNumMaterial(int no)
{
	if (no < 0)
		return;

	// Note: Doesn't preserve the order of what was there
	// It has to, if we're going to have multiple material subtables.

	// Perhaps the manner of organizing these should be revisited.  This is
	// pretty inefficient, but it will hopefully only amount to extra
	// minutes per build at most.

	matTables.resize(no*numTable);
	if ((no<=numMat) || (numTable==1)) {
		// If they're shrinking the table, they had better be starting over.
		// If there's only one table, no problems.
		numMat = no;
	} else {
		// For each table, determine the last 'used' element.
		// Then, move them all, starting with the last one, and
		// reset the others.  Slow, slow, slow.
		int loop;
		for (int tableIter=numTable-1;tableIter>=0;tableIter--) {
                        int lastEntry;
			for (lastEntry=0;lastEntry<numMat;lastEntry++) {
				if (matTables[tableIter*numMat+lastEntry].shadeModel==999) break;
			}
			// move them
			if (tableIter && lastEntry) { // no need to move the first subtable
				for (loop=lastEntry-1;loop>=0;loop--)
					matTables[tableIter*no+loop]=matTables[tableIter*numMat+loop];
			}
			// reset the others
			for (loop=lastEntry;loop<no;loop++) matTables[tableIter*no+loop].Reset();
		}
		numMat = no;
	}
}
void trpgMatTable::SetMaterial(int nt,int nm,const trpgMaterial &mat)
{
	if (nt < 0 || nt >= numTable)
		return;
	if (nm < 0 || nm >= numMat)
		return;

	matTables[nt*numMat+nm] = mat;
	// ensure that this entry isn't re-used
	if (mat.shadeModel>100) matTables[nt*numMat+nm].shadeModel=trpgMaterial::Smooth;
}
void trpgMatTable::SetMaterial(int nm,const trpgMaterial &mat)
{
	if (nm < 0 || nm >= numMat)
		return;

	for (int i=0;i<numTable;i++) {
		matTables[i*numMat+nm] = mat;
		// ensure that this entry isn't re-used
		if (mat.shadeModel>100) matTables[i*numMat+nm].shadeModel=trpgMaterial::Smooth;
	}
}

#define CEQ(ca,cb) (ca.red == cb.red && ca.green == cb.green && ca.blue == cb.blue)

int trpgMatTable::AddMaterial(const trpgMaterial &mat,bool lookForExisting)
{
	return AddMaterialInSubtable(mat, 1, lookForExisting);
}

int trpgMatTable::AddMaterialInSubtable(const trpgMaterial &mat,int table, bool lookForExisting)
{
	// Doesn't work if there's more than one table
	//if (numTable != 1)
	//	return -1;

	trpgMaterial cmat = mat; // necessary?
	
	// having a shadeModel of 999 indicates that the entry is free.  I thought this would
	// work fine, until I realized that evidently most of the time the shademodel isn't set
	// at all.  Now my kludge takes so much work it's almost worth doing it right.

	if (cmat.shadeModel>100) cmat.shadeModel=trpgMaterial::Smooth;
	int baseMat=numMat;
	int offset=(table-1)*numMat+baseMat;

	if (table>numTable) {
		// Add another table.  But we don't have to shuffle indices or reset entries
		SetNumTable(table);
		offset=(table-1)*numMat;
		matTables[offset] = cmat;
		return 0;
	}

	bool spaceInTable=false;
	if (lookForExisting) {
		// Look for a matching base material minus the textures
		for (baseMat = 0;baseMat < numMat;baseMat++) {
			offset=(table-1)*numMat+baseMat;

			trpgMaterial &bm = matTables[offset];
			if (bm.shadeModel==999) {
				// this is an 'empty' entry.  Means we won't find it, either.
				spaceInTable=true;
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
				int i;
				for (i=0;i<cmat.texEnvs.size();i++) {
					trpgTextureEnv &e1 = cmat.texEnvs[i];
					trpgTextureEnv &e2 = bm.texEnvs[i];
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
	if (spaceInTable) {
		matTables[offset]=cmat;
		return baseMat;
	} else {
		SetNumMaterial(baseMat+1); // shifts values, changes numMat
		matTables[table*numMat-1]=cmat;
		return baseMat;
	}
}

// Write out material table
bool trpgMatTable::Write(trpgWriteBuffer &buf)
{
	int i;

	if (!isValid())
		return false;

	buf.Begin(TRPGMATTABLE);

	// Total number of materials
	buf.Add((int32)numTable);
	buf.Add((int32)numMat);

	// Write the materials
	for (i=0;i<matTables.size();i++)
		matTables[i].Write(buf);

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
	mat = matTables[nt*numMat+nm];
	
	return true;
}
const trpgMaterial *trpgMatTable::GetMaterialRef(int nt,int nm) const
{
	if (nt < 0 || nt >= numTable || nm < 0 || nm >= numMat)
		return false;
	return const_cast<trpgMaterial *>(&matTables[nt*numMat+nm]);
}


bool trpgMatTable::Read(trpgReadBuffer &buf)
{
	trpgMaterial mat;
	trpgToken matTok;
	int32 len;
	bool status;
	int i,j;

	try {
		buf.Get(numTable);
		buf.Get(numMat);
		if (numTable <= 0 || numMat < 0) throw 1;
		// Read the materials
		matTables.resize(numTable*numMat);
		for (i=0;i<numTable;i++)
			for (j=0;j<numMat;j++) {
				buf.GetToken(matTok,len);
				if (matTok != TRPGMATERIAL) throw 1;
				buf.PushLimit(len);
				mat.Reset();
				status = mat.Read(buf);
				buf.PopLimit();
				if (!status) throw 1;
				matTables[i*numMat+j] = mat;
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
	buf.Add(autoNormal);
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
	buf.Add(isBump);
	buf.End();

	// Attributes (e.g. fid, smc)
	buf.Begin(TRPGMAT_ATTR);
	buf.Add(attrSet.fid);
	buf.Add(attrSet.smc);
	buf.Add(attrSet.stp);
	buf.Add(attrSet.swc);
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
		case TRPGMAT_BUMP:
		    {
			int tmp;
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
	org = trpg_RGBX_Neither;
	name = NULL;
	useCount = 0;
	sizeX = sizeY = -1;
	addr.file = 0;
	addr.offset = 0;
	isMipmap = false;
}

// Copy construction
trpgTexture::trpgTexture(const trpgTexture &in)
{
	mode = in.mode;
	type = in.type;
	numLayer = in.numLayer; // RGBX
	org = in.org; // RGBX
	name = NULL;
	SetName(in.name);
	useCount = in.useCount;
	sizeX = in.sizeX;  sizeY = in.sizeY;
	addr.file = in.addr.file;
	addr.offset = in.addr.offset;
	isMipmap = in.isMipmap;
	// storageSize + levelOffset
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
	org = trpg_RGBX_Neither;
	if (name)
		delete [] name;
	name = NULL;
	useCount = 0;
	sizeX = sizeY = -1;
	addr.file = 0;
	addr.offset = 0;
	isMipmap = false;
	storageSize.clear();
	levelOffset.clear();
}

// Valid if we've got a name
bool trpgTexture::isValid() const
{
	switch (mode) {
	case External:
		return (name != NULL);
		break;
	case Local:
		return (type != trpg_Unknown && sizeX != -1 && sizeY != -1);
		break;
	case Global:
		return (type != trpg_Unknown);
		break;
	case Template:
		return (type != trpg_Unknown && sizeX != -1 && sizeY != -1);
		break;
	default:
		return false;
	}

	return false;
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
	if (!isValid()) return false;

	int len = (name) ? strlen(name) : 0;
	strncpy(outName,name,MIN(len,outLen)+1);

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

void trpgTexture::SetImageOrganization(ImageOrg inOrg)
{
	org = inOrg;
}

bool trpgTexture::GetImageOrganization(ImageOrg &outOrg) const
{
	outOrg = org;

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
	case trpg_DDS:
		depth = 3;
		break;
	case trpg_DXT1:
		depth = 3;
		break;
	case trpg_DXT3:
		depth = 4;
		break;
	case trpg_DXT5:
		depth = 4;
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
	if (!isValid()) return false;
	layers = numLayer;
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
    mode = in.mode;
    type = in.type;

    if (in.name)
	SetName(in.name);

    useCount = in.useCount;
    
    sizeX = in.sizeX;
    sizeY = in.sizeY;

	// RGBX
	numLayer = in.numLayer;
	org = in.org;

    isMipmap = in.isMipmap;
    addr = in.addr;

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
		numLayer == in.numLayer && org == in.org)
	    return 1;
	break;
    case Global:
    case Template:
	if (type == in.type && sizeX == in.sizeX && sizeY == in.sizeY &&
	    isMipmap == in.isMipmap && numLayer == in.numLayer && org == in.org)
	    return 1;
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
	for (int i=0;i<storageSize.size();i++)
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
	buf.Add(isMipmap);
	// More for MCMs
	buf.Add(numLayer);
	buf.Add((unsigned char)org);
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
		buf.Get(bval);	type = (trpgTexture::ImageType)bval;
		buf.Get(sizeX);
		buf.Get(sizeY);
		buf.Get(addr.file);
		buf.Get(addr.offset);
		int ival;
		buf.Get(ival);  
		isMipmap = (ival) ? true : false;
		if ( !buf.isEmpty() ) {
			buf.Get(numLayer);
			buf.Get(bval);  org = (trpgTexture::ImageOrg)bval;
		}
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

}
trpgTexTable::trpgTexTable(const trpgTexTable &in)
{
	*this = in;
}

// Reset function
void trpgTexTable::Reset()
{
	errMess[0] = '\0';
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
	if (!texList.size())
	{
		strcpy(errMess, "Texture table list is empty");
		return false;
	}

	for (unsigned int i=0;i<texList.size();i++)
	{
		if (!texList[i].isValid())
		{
			strcpy(errMess, "A texture in the texture table is invalid");
			return false;
		}
	}

	return true;
}


// Set functions
void trpgTexTable::SetNumTextures(int no)
{
	texList.resize(no);
}
int trpgTexTable::AddTexture(const trpgTexture &inTex)
{
	texList.resize(texList.size()+1);
	texList[texList.size()-1] = inTex;
	return texList.size()-1;
}
int trpgTexTable::FindAddTexture(const trpgTexture &inTex)
{
	for (int i=0;i<texList.size();i++)
		if (texList[i] == inTex)
			return i;

	return AddTexture(inTex);
}
void trpgTexTable::SetTexture(int id,const trpgTexture &inTex)
{
	if (id < 0 || (unsigned int)id >= texList.size())
		return;

	texList[id] = inTex;
}

// Copy operator
trpgTexTable &trpgTexTable::operator = (const trpgTexTable &in)
{
	Reset();
	for (int i=0;i<in.texList.size();i++)
		AddTexture(in.texList[i]);

	return *this;
}

// Write Texture table
bool trpgTexTable::Write(trpgWriteBuffer &buf)
{
	int32 numTex;

	if (!isValid())
		return false;

	buf.Begin(TRPGTEXTABLE2);
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
	no = texList.size();
	if (!isValid()) return false;
	return true;
}
bool trpgTexTable::GetTexture(int id,trpgTexture &ret) const
{
	if (!isValid()) return false;
	if (id < 0 || id >= texList.size()) return false;

	ret = texList[id];
	return true;
}
const trpgTexture *trpgTexTable::GetTextureRef(int id) const
{
	if (id < 0 || id >= texList.size()) return NULL;
	return &texList[id];
}

bool trpgTexTable::Read(trpgReadBuffer &buf)
{
	int32 numTex;
	trpgToken texTok;
	int32 len;

	try {
		buf.Get(numTex);
		texList.resize(numTex);
		for (unsigned int i=0;i<numTex;i++) {
			buf.GetToken(texTok,len);
			if (texTok != TRPGTEXTURE) throw 1;
			buf.PushLimit(len);
			trpgTexture &tex = texList[i];
			bool status = tex.Read(buf);
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
	baseMat = -1;
	sx = sy = ex = ey = destWidth = destHeight = 0;
	addr.resize(1);
	addr[0].file = 0;
	addr[0].offset = 0;
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
//	storageSize.resize(0);
	addr.resize(1);
	addr[0].file = 0;
	addr[0].offset = 0;
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
	buf.Add(numAddrs-1); // write even if its zero for reading purposes
	if (numAddrs>1) {
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
				}
			}
		}
	}
	catch (...) {
		return false;
	}

	return isValid();
}

