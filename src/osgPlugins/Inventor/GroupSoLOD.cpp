#include <Inventor/nodes/SoGroup.h>
#include <Inventor/actions/SoCallbackAction.h>

#include "GroupSoLOD.h"

SO_NODE_SOURCE(GroupSoLOD);

void GroupSoLOD::initClass()
{
    classTypeId = SoType::overrideType(SoLOD::getClassTypeId(),
                                       createInstance);
    parentFieldData = SoLOD::getFieldDataPtr();
}

GroupSoLOD::GroupSoLOD()
{
    SO_NODE_CONSTRUCTOR(GroupSoLOD);
}

GroupSoLOD::~GroupSoLOD()
{
}

void GroupSoLOD::callback(SoCallbackAction *action)
{
    SoGroup::doAction(action);
}

