
#ifndef __MDL_ROOT_H_
#define __MDL_ROOT_H_


#include <vector>

#include "BodyPart.h"


namespace mdl
{


class MDLRoot
{
protected:

    typedef std::vector<BodyPart *>    BodyPartList;

    BodyPartList      body_parts;

public:

    MDLRoot();
    virtual ~MDLRoot();

    void          addBodyPart(BodyPart * newPart);
    int           getNumBodyParts();
    BodyPart *    getBodyPart(int partIndex);
};


}

#endif

