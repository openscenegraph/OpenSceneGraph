// MeshRecord.h

#ifndef __FLT_MESH_RECORD_H
#define __FLT_MESH_RECORD_H


#include "FaceRecord.h"


namespace flt {


////////////////////////////////////////////////////////////////////
//
//                          MeshRecord
//
////////////////////////////////////////////////////////////////////


// Note: the enums for FaceRecord are the same for MeshRecord, so we inherit 
// and overload where necessary. I was sort of on the fence with this decision,
// because a MeshRecord isn't really a more specific case of a FaceRecord s
// (which single inheritance typically implies). However, there is too much in 
// FaceRecord that is identical to not take advantage of. This can be changed 
// later if the two records diverge, or if it proved to be a bad idea.

class MeshRecord : public FaceRecord
{
public:

  MeshRecord();

  virtual void          accept ( RecordVisitor &rv ) { rv.apply ( *this ); }

  virtual const char *  className() const { return "MeshRecord"; }
  virtual int           classOpcode() const { return MESH_OP; }
  virtual Record *      clone() const { return new MeshRecord(); }

protected:

  virtual ~MeshRecord();

  virtual void          endian();

  virtual bool          readLocalData ( Input &fr );
};


}; // end namespace flt


#endif // __FLT_MESH_RECORD_H
