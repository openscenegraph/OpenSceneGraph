// geo actions header

#ifndef _GEO_ACTIONS_H_
#define _GEO_ACTIONS_H_

using namespace osg;
using namespace osgDB;

class georecord; // You don't need to know how I read a geo record, 
//	but objects of this class are passed to some of the parsing routines.
// The values are defined in osgGeoStructs.h which is distributed with OSG.

class geoBehaviour { // base class for action & math functions where var out = f(var in)
public:
    geoBehaviour() { }
    virtual ~geoBehaviour() { }
    virtual void doaction(void) const
    { // do math or action operation 
    }
protected:
};

class geoBehaviourDrawableCB: public osg::Drawable::AppCallback {
public:
    geoBehaviourDrawableCB() { }
    ~geoBehaviourDrawableCB() { }
    void addBehaviour(geoBehaviour *gb) {gblist.push_back(gb);}
    void app(osg::NodeVisitor *,osg::Drawable *dr);
private:
    std::vector<geoBehaviour *> gblist;
};

class geoMathBehaviour : public geoBehaviour { // base class for math functions where var out = f(var in)
public:
    geoMathBehaviour() { in=out=NULL; }
    virtual ~geoMathBehaviour() { }
    virtual void setInVar(const double *indvar) {in=indvar;}
    virtual void setOutVar(double *outdvar) {out=outdvar;}
    virtual void doaction(void) const
    { // do math operation eg *out=*in or =f(*in).
    }
protected:
    const double *in; // address of input variable
    double *out; // address of output
};

// in these functions, var1 is the input value x, var2 (& 3) are constants.
inline double DEG2RAD(const double var) { return var*0.0174532925199432957692369076848861; }
inline double addv(const double var1,const double var2) { return var1+var2; }
inline double subv(const double var1,const double var2) { return var1-var2; }
inline double mulv(const double var1,const double var2) { return var1*var2; }
inline double divv(const double var1,const double var2) { return var1/var2; }
inline double equa(const double var1,const double var2) { return var1; }
inline double linear(const double var1,const double var2,const double var3) { return var2*var1+var3; }
inline double lininv(const double var1,const double var2,const double var3) { return var2/var1+var3; }
inline double linmod(const double var1,const double var2,const double var3) { return var2*fmod(var1,var3); }
inline double linsqr(const double var1,const double var2,const double var3) { return var2*sqrt(var1)+var3; }
inline double linabs(const double var1,const double var2,const double var3) { return var1*fmod(var2,var3); }
inline double trunc(const double var1,const double var2,const double var3) { return ((int)(var1/var2)+var3); }
inline double trigsin(const double var1,const double var2,const double var3) { return var2*sin(DEG2RAD(var1*var3)); }
inline double trigcos(const double var1,const double var2,const double var3) { return var2*cos(DEG2RAD(var1*var3)); }
inline double trigtan(const double var1,const double var2,const double var3) { return var2*tan(DEG2RAD(var1*var3)); }
inline double trigasin(const double var1,const double var2,const double var3) { return var2*asin(var1*var3); }
inline double trigacos(const double var1,const double var2,const double var3) { return var2*acos(var1*var3); }
inline double trigatan(const double var1,const double var2,const double var3) { return var2*atan(var1*var3); }
inline double trigatan2(const double var1,const double var2,const double var3) { return var2*atan2(var1,var3); }
inline double trighypot(const double var1,const double var2,const double var3) { return var2*hypot(var1,var3); }
inline double period_1(const double var1,const double var2,const double var3) { return var2*fmod(var1,var3); }
inline double period_2(const double var1,const double var2,const double var3) { return fmod(var1*var2,var3); }
inline double ifelse(const double var1,const double var2,const double var3) { return ((var1<1.0 && var1>-1.0) ? var2:var3); }

class geoArithConstant { // a constant can be a fixed double OR address of variable
public:
	geoArithConstant(const float v=0) { constant=v; varop=NULL; }
	virtual ~geoArithConstant() {}
	void set(const float v) { constant=v; varop=NULL; }
	bool set(const double *v) { varop=v; return (v!=NULL);}
	inline double get(void) const { return varop? *varop : constant;}
private:
    float constant;
    const double *varop; // if null use constant value in maths; else 
};

class geoArithBehaviour : public geoMathBehaviour {
public:
    geoArithBehaviour() { op=NULL; }
    virtual ~geoArithBehaviour() { }
    void setType(uint iop);
    bool setVariable(const double *varvar) { return acon.set(varvar);}
	void setConstant(float v) {acon.set(v); }
	inline double getconstant(void) { return acon.get();}
    void doaction(void) const; // do math operation
    bool makeBehave(const georecord *grec, geoHeaderGeo *theHeader);
private:
	double (* op)(const double d1, const double v2);
	geoArithConstant acon;
};

class geoAr3Behaviour : public geoArithBehaviour { // extended to 3 constants, out=f(a,b,c)
public:
    geoAr3Behaviour() { op=NULL; }
    virtual ~geoAr3Behaviour() { }

    void setType(uint iact);
    void setTrigType(int iop);
    void setPeriodicType(int iop);
	
    void doaction(void); // do math operation
    bool makeBehave(const georecord *grec, geoHeaderGeo *theHeader, const uint act);
private:
    geoArithConstant bcon;
	double (* op)(const double d1, const double v2, const double v3);
};
class geoCompareBehaviour : public geoMathBehaviour {
public:
    geoCompareBehaviour() { constant=0; oper=UNKNOWN; varop=NULL;}
    virtual ~geoCompareBehaviour() { }
    enum optype{UNKNOWN, GREATER, GREATOREQ, LESS, LESSOREQ, EQUALTO};
//    void setConstant(const float v) { constant=v;}
    void setType(uint iop);
    void setVariable(const double *varvar) { varop=varvar;}

    void doaction(void) const; // do compare operation
    bool makeBehave(const georecord *grec, geoHeaderGeo *theHeader);
private:
    float constant;
    optype oper;
    const double *varop; // if null use constant value in maths; else 
};

class geoRangeBehaviour :public geoMathBehaviour {
    // output = outmin + frac*(outmax-min) where frac = (in-min)/(max-min)
public:
    geoRangeBehaviour() { inmin=outmin=(float)-1.e32; inmax=outmax=(float)1.e32; in=out=NULL;}
    ~geoRangeBehaviour() { }
    void setInMax(const double v) { inmax=v;}
    void setInMin(const double v) { inmin=v;}
    void setOutMax(const double v) { outmax=v;}
    void setOutMin(const double v) { outmin=v;}
    void doaction(void) const; // do math operation
    bool makeBehave(const georecord *grec, geoHeaderGeo *theHeader);
private:
    float inmin,inmax;
    float outmin,outmax;
};
class geoClampBehaviour :public geoMathBehaviour {
public:
    geoClampBehaviour() { min=(float)-1.e32; max=(float)1.e32; in=out=NULL;}
    ~geoClampBehaviour() { }
    void setMax(const double v) { max=v;}
    void setMin(const double v) { min=v;}
    void doaction(void) const; // do math operation
    bool makeBehave(const georecord *grec, geoHeaderGeo *theHeader);
private:
    float min,max;
};
class geoRange { // discrete range -- min,max,output
public:
	geoRange() { min.set(0.0); max.set(0.0); val.set(0.0);}
	virtual ~geoRange() {}
	inline void setMin(const float v) { min.set(v);}
	inline void setMax(const float v) { max.set(v);}
	inline void setVal(const float v) { val.set(v);}
	const double getMin(void) const { return min.get();}
	const double getMax(void) const { return max.get();}
	const double getVal(void) const { return val.get();}
private:
	geoArithConstant min,max,val;
};
class geoDiscreteBehaviour : public geoMathBehaviour { // discrete action -- output= 1 of several
public:
	geoDiscreteBehaviour() {nrange=1;	}
	virtual ~geoDiscreteBehaviour() {}
    void doaction(void) const; // do math operation
    bool makeBehave(const georecord *grec, geoHeaderGeo *theHeader);

private:
	int nrange;
	std::vector<geoRange> rangelist;
};


class geoActionBehaviour : public geoBehaviour { // base class for any scenegraph changes
public:
    geoActionBehaviour() { var=NULL; type=0;}
    virtual ~geoActionBehaviour() { 
        var=NULL;}
    void setType(const unsigned int t) { type=t; }
    void setVar(const double *v) { var=v;}
    inline unsigned int getType(void) const { return type;}
    inline const double *getVar(void) const { return var;}
    inline double getValue(void) const { return *var;}
    virtual void doaction(osg::Node *node) const {
    }

    virtual bool makeBehave(const georecord *grec, geoHeaderGeo *theHeader, const uint act) {
		return true;
    }
private:
    // for fast transform behaviours
    unsigned int type; // eg GEO_DB_ROTATE_ACTION_INPUT_VAR, translate etc
    const double *var; // variable controls this behaviour
};
class geoMoveBehaviour : public geoActionBehaviour { // class of rotate & translate actions
public:
    geoMoveBehaviour() { axis.set(0,0,1); centre.set(0,0,0);}
    virtual ~geoMoveBehaviour() { }
    void setCentre(const Vec3 v) { centre=v;}
    void setAxis(const Vec3 v) { axis=v;}
	inline Vec3 getAxis() { return axis;}
	inline Vec3 getCentre() { return centre;}
    void doaction(osg::Node *node);

    bool makeBehave(const georecord *grec, const geoHeaderGeo *theHeader, const uint act);
private:
    // for fast transform behaviours
    osg::Vec3 axis; // axis of rotation or translate or scale
    osg::Vec3 centre; // centre of rotation or scale
};
class geoMoveVertexBehaviour : public geoActionBehaviour { // class of rotate & translate vertex actions
public:
    geoMoveVertexBehaviour() { index=0; pos.set(0,0,0);}
    virtual ~geoMoveVertexBehaviour() { }
	inline Vec3 getpos() { return pos;}
	void setpos(const osg::Vec3 p) { pos=p;}
	void setindx(const int idx) { index=idx;}
	inline int getindex(void) const { return index;}
    void doaction(Matrix &mtr,osg::Drawable *dr);

    bool makeBehave(const georecord *grec, const geoHeaderGeo *theHeader, const uint act);
private:
    // for fast transform behaviours
	int index; // which index in the geometry
	Vec3 pos; // raw position of the vertex
	geoMoveBehaviour movb;
};

class geoVisibBehaviour : public geoActionBehaviour { // visibility action -- sets node mask
public:
    geoVisibBehaviour() { }
    virtual ~geoVisibBehaviour() { }

    bool makeBehave(const georecord *grec, geoHeaderGeo *theHeader, const uint act);
    void doaction(osg::Node *node) const;
private:
};
class geoColourBehaviour : public geoActionBehaviour { // colour action
    // sets content of case DB_DSK_COLOR_RAMP_ACTION
public:
    geoColourBehaviour() { topcindx=4096; botcindx=0; numramps=1; type=UNKNOWN; colours=NULL;}
    virtual ~geoColourBehaviour() { }
	enum cacts {UNKNOWN, PALETTE, RAMP};
    bool makeBehave(const georecord *grec, const geoHeaderGeo *theHeader);
    void doaction(osg::Drawable *dr) const;
	void setVertIndices(const uint ns, const uint n) { nstart=ns; nend=ns+n;}
	void setColorPalette(const colourPalette *color_palette) {colours=color_palette;}
private:
	uint numramps;
	uint topcindx,botcindx; // top & bottom colour indices
	cacts type; //
	uint nstart,nend; // start index in the colours array, and number of indices
	const colourPalette *colours; // where the colours come from - actually held in the geoHeader structure
};

class geoStrContentBehaviour : public geoActionBehaviour { // string content actions
    // sets content of a string...
public:
    geoStrContentBehaviour() {format=NULL;PADDING_TYPE=0;
        PAD_FOR_SIGN=0; vt=UNKNOWN; }
    virtual ~geoStrContentBehaviour() { delete [] format;}
    void doaction(osg::Drawable *node); // do new text
    bool makeBehave(const georecord *grec, geoHeaderGeo *theHeader);
    enum valuetype {UNKNOWN, INT, FLOAT, DOUBLE, CHAR};
private:
    char *format;
    uint PADDING_TYPE;
    uint PAD_FOR_SIGN;
    valuetype vt;
};

class geoBehaviourCB: public osg::NodeCallback {
public:
    geoBehaviourCB() { }
    ~geoBehaviourCB() { }
    void addBehaviour(geoBehaviour *gb) {gblist.push_back(gb);}
    virtual void geoBehaviourCB::operator() (osg::Node *node, osg::NodeVisitor* nv);
private:
    std::vector<geoBehaviour *> gblist;
};


#endif
