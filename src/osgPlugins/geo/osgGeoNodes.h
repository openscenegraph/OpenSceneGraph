// special geo nodes
#ifndef _GEO_NODES_H_
#define _GEO_NODES_H_

#include <osg/Timer>

class geoValue {
public:
	geoValue() {
		token=0; fid=0; val.d=0; name="";
		vmin=0; vmax=0;
		constrained=false;
	}
	geoValue(const unsigned int tok, const unsigned int fident) {
		token=tok; fid=fident; val.d=0; name="";
		vmin=0; vmax=0;
		constrained=false;
	}
	~geoValue() {}
	inline unsigned int getToken() const { return token;}
	inline unsigned int getFID() const { return fid;}
	inline double *getVar() { 
		return &(val.d);} // address of variable
	inline const double getVal() const { return (val.d);}
	void setVal(double v) { val.d=v;
	if (constrained) {
		if (v>vmax) val.d=vmax;
		if (v<vmin) val.d=vmin;
	}
	}
	const std::string getName(void) const { return name;}
	void setName(const char *nm) { name=nm;	}
	void setMinRange(const float f) { vmin=f;}
	void setMaxRange(const float f) { vmax=f; }
	void setConstrained(bool onoff=true) {	constrained=onoff;}
private:
	union {
		double d;
		float f;
		int i;
		unsigned int ui;
	} val;
	unsigned int token; // type of field
	unsigned int fid; // field identifier
	float vmin, vmax;
	std::string name;
	bool constrained; // are values limited by min,max
}; // a numerical value, may be one of several types

class internalVars { // holds internal variables for whole model
public:
	internalVars() {   }
	internalVars(const internalVars &iv) {
		vars=iv.vars; }
	~internalVars() { 
		}
	void addInternalVars(const georecord &gr);
	void update(const osg::FrameStamp *_frameStamp);
	double *getVar(const unsigned fid) { 
		for (std::vector<geoValue>::iterator itr=vars.begin();
		itr!=vars.end();
		++itr)
		{// for each field
			if ((*itr).getFID() == fid) {
				return ((*itr).getVar());
			}
		}
		return NULL;
	}
	const geoValue *getGeoVar(const unsigned fid) const {
		for (std::vector<geoValue>::const_iterator itr=vars.begin();
		itr!=vars.end();
		++itr)
		{// for each field
			if ((*itr).getFID() == fid) {
				return (&(*itr));
			}
		}
		return NULL;
	}
private:
	std::vector<geoValue> vars; // these fields define internal vars
};

class userVars {
public:
	userVars() {}
	userVars(const userVars &iv)
        {
		vars=iv.vars;
        }
	~userVars() {}
	unsigned int number() { return vars.size();}
	std::vector<geoValue> *getvars() { return &vars;}
	double *getVar(const unsigned fid)  { 
		for (std::vector<geoValue>::iterator itr=vars.begin(); itr<vars.end(); itr++) {
			if (itr->getFID() == fid) return (itr->getVar());
		}
		return NULL;
	}
	const geoValue *getGeoVar(const unsigned fid) const { 
		for (std::vector<geoValue>::const_iterator itr=vars.begin(); itr<vars.end(); itr++) {
			if (itr->getFID() == fid) return (&(*itr));
		}
		return NULL;
	}
	void addUserVar(const georecord &gr);
private:
	std::vector<geoValue> vars;
};

class pack_colour { // holds the top colour of each colour ramp
public:
    pack_colour() { cr=cg=cb=0; ca=1;}
    ~pack_colour() {}
    pack_colour(const unsigned char col[4]) { cr= col[0]; cg= col[1];cb= col[2];; ca= col[2];}
    void get(unsigned char col[4]) const { col[0]=cr; col[1]=cg; col[2]=cb; col[3]=ca; }
    friend inline std::ostream& operator << (std::ostream& output, const pack_colour& pc)
    {
        output << " cpalette: " <<(int)pc.cr << " " <<(int)pc.cg << " " <<(int)pc.cb << " " <<(int)pc.ca;
        return output;     // to enable cascading.. 
    }
private:
    unsigned char cr, cg, cb, ca;
};

typedef std::vector< pack_colour > colourPalette;

class geoHeaderGeo: public geoHeader {
	// detailed structure for header of .geo file,
	// including animation variables.
public:
	geoHeaderGeo();
	~geoHeaderGeo() { color_palette->clear();	}
	geoHeaderGeo(const geoHeaderGeo &geo,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY) :
		geoHeader(geo,copyop){ 
			intVars=new internalVars(*geo.intVars); useVars=new userVars(*geo.useVars);
			extVars=new userVars(*geo.extVars);
		}
	void addInternalVars(const georecord &gr) { intVars->addInternalVars(gr);}
	internalVars *getInternalVars(void) const { return intVars;}
	const std::string getVarname(const unsigned fid) const {
		const geoValue *gv=getGeoVar(fid);
		return gv->getName();
	}
	const geoValue *getGeoVar(const unsigned fid) const;
	double *getVar(const unsigned fid) const;
	void addUserVar(const georecord &gr);
	//== handler for updating internal variables
	void update(const osg::FrameStamp *);
	inline void getPalette(uint icp, float cll[4]) const { // get color from palette
        uint maxcol=icp/128; // the maximum intensity index
		float frac = (float)(icp-maxcol*128)/128.0f;

        if (maxcol < color_palette->size()) {
			unsigned char col[4];
			(*color_palette)[maxcol].get(col);
			for (int i=0; i<4; i++) {
				col[i]=(unsigned char)(col[i]*frac); // prevents warning under gcc from *=frac with frac=real
				cll[i]=col[i]/255.0f;
			}
		} else {
			unsigned char col[4];
			col[0]=(icp & 0xff000000)>> 24;
			col[1]=(icp & 0xff0000)  >> 16;
			col[2]=(icp & 0xff00)    >>  8;
			col[3]= icp & 0xff;
			for (int i=0; i<4; i++) {
				cll[i]=col[i]/255.0f;
			}
			cll[0]=cll[1]=cll[2]=1.0f;
		}
		cll[3]=1.0f; // default alpha {0-1}
	}
	void addColour(unsigned char *cpal) {(*color_palette).push_back(cpal);}
	inline colourPalette *getColorPalette() const { return color_palette;}
private:
	osg::Timer_t _lastFrameTick,_initialTick;
	osg::Timer   _timer;
	internalVars *intVars;
	userVars *useVars;
	userVars *extVars;
	void moveit(const double t);
    colourPalette *color_palette; // the colour palette - used in colour animations
};

#endif
