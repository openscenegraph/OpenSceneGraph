
#include "MDLRoot.h"


using namespace mdl;


MDLRoot::MDLRoot()
{
}


MDLRoot::~MDLRoot()
{
}


void MDLRoot::addBodyPart(BodyPart * newPart)
{
    // Add the new part to our list
    body_parts.push_back(newPart);
}


int MDLRoot::getNumBodyParts()
{
    return body_parts.size();
}


BodyPart * MDLRoot::getBodyPart(int partIndex)
{
    if ((partIndex < 0) || (partIndex >= static_cast<int>(body_parts.size())))
        return NULL;
    else
        return body_parts[partIndex];
}

