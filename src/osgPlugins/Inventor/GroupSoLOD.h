#ifndef _GROUPSOLOD_H_
#define _GROUPSOLOD_H_

#include <Inventor/nodes/SoLOD.h>
#include <Inventor/nodes/SoSubNode.h>

class GroupSoLOD : public SoLOD 
{
    SO_NODE_HEADER(GroupSoLOD);

public:
    GroupSoLOD();
    static void initClass();

protected:
    virtual void callback(SoCallbackAction *action);

private:
    virtual ~GroupSoLOD();
};

#endif
