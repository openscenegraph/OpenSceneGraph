#include "ConvertFromPerformer.h"

#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfBillboard.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfSCS.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfMaterial.h>

#include <osg/Registry>

// Performer includes.
#include <Performer/pfdu.h>


extern "C" {

    int pfdStoreFile_osg(pfNode *root, const char *fileName);

};


int pfdStoreFile_osg(pfNode *root, const char *fileName)
{
    ConvertFromPerformer converter;
    osg::Node* osg_root = converter.convert(root);
    if (osg_root)
    {
        osg::Registry::instance()->writeNode(*osg_root,fileName);
        return 1;
    }
    else
    {
        cerr << "error : failed to create any perfomer database from input files."<<endl;
        return 0;
    }
}
