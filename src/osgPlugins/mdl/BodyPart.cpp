
#include "BodyPart.h"


using namespace mdl;


BodyPart::BodyPart(MDLBodyPart * myPart)
{
    // Save the body part information
    my_body_part = myPart;
}


BodyPart::~BodyPart()
{
    // Clean up the associated data
    delete my_body_part;
}


MDLBodyPart * BodyPart::getBodyPart()
{
    return my_body_part;
}


void BodyPart::addModel(Model * newModel)
{
    // Add the new node to our list
    part_models.push_back(newModel);
}


int BodyPart::getNumModels()
{
    return part_models.size();
}


Model * BodyPart::getModel(int partIndex)
{
    if ((partIndex < 0) || (partIndex >= static_cast<int>(part_models.size())))
        return NULL;
    else
        return part_models[partIndex];
}

