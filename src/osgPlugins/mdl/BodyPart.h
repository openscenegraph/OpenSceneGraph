
#ifndef __BODY_PART_H_
#define __BODY_PART_H_


#include <vector>

#include "Model.h"


namespace mdl
{


struct MDLBodyPart
{
    int    mdl_name_index;
    int    num_models;
    int    body_part_base;
    int    model_offset;
};


class BodyPart
{
protected:

    typedef std::vector<Model *>    ModelList;

    MDLBodyPart *    my_body_part;

    ModelList        part_models;

public:

    BodyPart(MDLBodyPart * myPart);
    virtual ~BodyPart();

    MDLBodyPart *    getBodyPart();

    void             addModel(Model * newModel);
    int              getNumModels();
    Model *          getModel(int modelIndex);
};


}

#endif

