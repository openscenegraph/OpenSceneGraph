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
#include <osg/FileNameUtils>
#include <osg/Notify>

// Performer includes.
#include <Performer/pfdu.h>

#ifdef OSG_USE_IO_DOT_H
#include <iostream.h>
#else
#include <iostream>
using namespace std;
#endif


int main( int argc, const char **argv )
{
    
    // create the list of input files from the command line arguments
    typedef std::vector<std::string> FileList;
    FileList filelist;
    
    bool saveImagesAsRGB = false;
    for(int i=1;i<argc;++i)
    {
        if (strcmp(argv[i],"-si")==0) saveImagesAsRGB = true;
        else filelist.push_back(argv[i]);    
    }

    if (filelist.size()<2)
    {
        osg::notify(osg::NOTICE) << "Usage :";
        osg::notify(osg::NOTICE) << "\t"<<argv[0]<<" [-si] InputFile [InputFile...] OutputFile"<<endl;
        osg::notify(osg::NOTICE) << "\t"<<"InputFile    - input file to be loaded by Performer."<<endl;
        osg::notify(osg::NOTICE) << "\t"<<"OutputFile   - output file to be saved by OSG."<<endl;
        osg::notify(osg::NOTICE) << "\t"<<"-si          - save of texture images as rgb files"<<endl;
        osg::notify(osg::NOTICE) << "\t"<<"               in the OutputFile's directory"<<endl;
        osg::notify(osg::NOTICE) << endl;
    }


    // create the output file from the command line arguments
    std::string outfile = filelist.back();
    filelist.pop_back();
    std::string outpath = osg::getFilePath(outfile);

    pfInit();

    FileList::iterator itr;        
    for(itr=filelist.begin();itr!=filelist.end();++itr)
    {
        pfdInitConverter((*itr).c_str());    
    }
    
    pfConfig();

    std::vector<pfNode*> nodeList;
    for(itr=filelist.begin();itr!=filelist.end();++itr)
    {
        pfNode* pf_file_root = pfdLoadFile((*itr).c_str());
        if (pf_file_root)
        {
            nodeList.push_back(pf_file_root);
        }
    }


    if (!nodeList.empty())
    {    
        pfNode* pf_root = NULL;
        if (nodeList.size()==1)
        {
            pf_root = nodeList[0];
        }
        else
        {
            pfGroup* group = new pfGroup;
            for(std::vector<pfNode*>::iterator ni=nodeList.begin();
                ni!=nodeList.end();
                ++ni)
            {
                group->addChild(*ni);
            }
            pf_root = (pfNode*) group;
        }

        ConvertFromPerformer converter;
        converter.setSaveImageDirectory(outpath);
        converter.setSaveImagesAsRGB(saveImagesAsRGB);

        osg::Node* osg_root = converter.convert(pf_root);
        if (osg_root)
        {
            osg::Registry::instance()->writeNode(*osg_root,outfile.c_str());
        }
        else
        {
            osg::notify(osg::NOTICE) << "error : failed to convert perfomer scene graph to OSG."<<endl;
        }

    }
    else
    {
        osg::notify(osg::NOTICE) << "error : failed to create any perfomer database from input files."<<endl;
    }

    pfExit(); 

}
