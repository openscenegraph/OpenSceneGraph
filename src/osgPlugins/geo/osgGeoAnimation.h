// animation features of the CarbonGraphics .geo format
// This file is required by external simulations to enable the user to
// control the GEO internal, user & external variables.

// The user creates one or two functions (here uvarupdate, extvarupdate).
// These functions are demonstrated in geodemo.cpp.

// Consider a model as being a class (or a subroutine).  The class has a 'draw' method
// supplied by OSG, and the model can be animated (parts of the model 
//	rotate, move, change colour, become invisible etc.).
//
// The model developer attaches 'behaviours' to parts of the model (using the Geo graphical editor)
// and assigns these behaviours to depend on variables.  There are 3 pools of variables:
//	Internal, usually time dependent variables which cannot be modified by the developer.
//	User variables - may be a function of other variables, defined in the editor.
//	External variables - the user written callback function extvarupdate sets these values on each frame of simulation.
//	User & external variables may be defined as a mathematical or logical function of 
// all the variables (external variables, internal variables & user variables).

// as a design rule, you should not normally attach a function to uvarupdate
// these variables should be considered as local variables within a function and not accessed by the program.
// The external variables should call a user written extvarupdate routine which can
// access Ethernet, a data file, shared memory or any other code to model the dynamics of your model.

#ifndef _GEO_ANIM_H_
#define _GEO_ANIM_H_

#include <osg/PositionAttitudeTransform>

class geoHeader: public osg::PositionAttitudeTransform {
	// structure for header of .geo file
	// adds position attitude orientation for not Z up models,
	// plus animation variables.
public:
	geoHeader() { 
		uvarupdate=NULL; extvarupdate=NULL;
	};
	geoHeader(const geoHeader &geo,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY) :
		osg::PositionAttitudeTransform(geo,copyop)
	{ 
	//	const geoHeaderGeo *ghg=static_cast<const geoHeaderGeo *> (&geo);
	}

	~geoHeader() {}
	void setUserUpdate(double (*ufn)(const double time,const double val, const std::string name) )
	{ // pass the address of a user written function in the Update phase.
		uvarupdate=ufn;
	}
	void setExternUpdate(double (*ufn)(const double time,const double val, const std::string name) )
	{ // pass the address of a user written function in the Update phase.
		extvarupdate=ufn;
	}
	double (* uvarupdate)(const double t, const double val, const std::string name); // called when variables are updated, you write this!
	double (* extvarupdate)(const double t, const double val, const std::string name); // called when variables are updated, you write this!
private:
};

#endif
