// animation features of the CarbonGraphics .geo format
// require access to the internal Vars & External Vars.
#ifndef _GEO_ANIM_H_
#define _GEO_ANIM_H_

#include <osg/Timer>
#include <osg/PositionAttitudeTransform>

class georecord; // You don't need to know how I read a geo record, 
//	but objects of this class are passed to some of the parsing routines.
// The values are defined in osgGeoStructs.h which is distributed with OSG.

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
	inline double *getValue() { return &(val.d);}
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
	~internalVars() { 
		}
	void addInternalVars(const georecord &gr);
	void update(osg::Timer  &_timer,osg::FrameStamp &_frameStamp);
	double *getVar(const unsigned fid) { 
		int iord=0;
		for (std::vector<geoValue>::iterator itr=vars.begin();
		itr!=vars.end();
		++itr, iord++)
		{// for each field
			if ((*itr).getFID() == fid) {
				return ((*itr).getValue());
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
	~userVars() {}
	unsigned int number() { return vars.size();}
	std::vector<geoValue> *getvars() { return &vars;}
	double *getVar(const unsigned fid)  { 
		for (std::vector<geoValue>::iterator itr=vars.begin(); itr<vars.end(); itr++) {
			if (itr->getFID() == fid) return (itr->getValue());
		}
		return NULL;
	}
	void addUserVar(const georecord &gr);
	void update() {
		for (std::vector<geoValue>::iterator itr=vars.begin();
			itr!=vars.end();
			++itr)
		{// for each user var
			//	itr->setVal((*itr->getValue())+0.01);
		}
	}
private:
	std::vector<geoValue> vars;
};

class geoHeader: public osg::PositionAttitudeTransform {
	// structure for header of .geo file
	// adds position attitude orientation for not Z up models,
	// plus animation variables.
public:
	geoHeader() { intVars=new internalVars; useVars=new userVars;
		extVars=new userVars;
		_frameStamp.setFrameNumber(0);  //  vars=NULL;
		tstart=_frameStamp.getReferenceTime();_initialTick = _timer.tick();
		ucb=NULL;
	};
	void setUserUpdate(void (*ufn)(const double time, userVars *locVars,userVars *extVars) )
	{ // pass the address of a user written function in the App process.
		ucb=ufn;
	}
	void addInternalVars(const georecord &gr) { intVars->addInternalVars(gr);}
	internalVars *getInternalVars(void) const { return intVars;}
	double *getVar(const unsigned fid) { 
		double *dv=NULL;
		dv=intVars->getVar(fid);
		if (!dv) {
			dv=useVars->getVar(fid);
			if (!dv) {
				dv=extVars->getVar(fid);
			}
		}
		return dv;
	}
	void addUserVar(const georecord &gr)
	{ // this georecord defines a single variable of type<>
		useVars->addUserVar(gr);
	}
	void update() {
		osg::Timer_t _frameTick = _timer.tick();;
		_lastFrameTick=_frameTick;
		
		_frameStamp.setFrameNumber(_frameStamp.getFrameNumber()+1);
		_frameStamp.setReferenceTime(_timer.delta_s(_initialTick,_frameTick));
		double time = _frameStamp.getReferenceTime();
		intVars->update(_timer, _frameStamp);
		useVars->update();
		extVars->update();
		if (ucb) ucb(time,useVars, extVars);
	}
	userVars *getLocalVars() const { return useVars;}
	userVars *getExternalVars() const { return extVars;}

protected:

	virtual ~geoHeader() {}

	osg::Timer_t _lastFrameTick,_initialTick;
	osg::Timer   _timer;
	double tstart; // start time
	osg::FrameStamp _frameStamp ; // time utilities
	internalVars *intVars;
	userVars *useVars;
	userVars *extVars;
	void (* ucb)(const double t, userVars *l,userVars *e); // called when variables are updated, you write this!
};

#endif
