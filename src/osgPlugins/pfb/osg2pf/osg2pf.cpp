#include "ConvertToPerformer.h"

#include <Performer/pf/pfNode.h>

#include <osg/Registry>
#include <osg/FileNameUtils>
#include <osg/Group>

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
        cerr << "Usage :";
        cerr << "\t"<<argv[0]<<" [-si] InputFile [InputFile...] OutputFile"<<endl;
        cerr << "\t"<<"InputFile    - input file to be loaded by OSG."<<endl;
        cerr << "\t"<<"OutputFile   - output file to be saved by Performer."<<endl;
        cerr << "\t"<<"-si          - save of texture images as rgb files"<<endl;
        cerr << "\t"<<"               in the OutputFile's directory"<<endl;
        cerr << endl;
    }


    // create the output file from the command line arguments
    std::string outfile = filelist.back();
    filelist.pop_back();
    std::string outpath = osg::getFilePath(outfile);

    pfInitArenas();
    pfInit();

    FileList::iterator itr;        
    for(itr=filelist.begin();itr!=filelist.end();++itr)
    {
        pfdInitConverter(outfile.c_str());    
    }
    
    pfConfig();

    std::vector<osg::Node*> nodeList;
    for(itr=filelist.begin();itr!=filelist.end();++itr)
    {
        osg::Node* osg_file_root = osg::Registry::instance()->readNode((*itr).c_str());
        if (osg_file_root)
        {
            nodeList.push_back(osg_file_root);
        }
    }


    if (!nodeList.empty())
    {    
        osg::Node* osg_root = NULL;
        if (nodeList.size()==1)
        {
            osg_root = nodeList[0];
        }
        else
        {
            osg::Group* group = new osg::Group();
            for(std::vector<osg::Node*>::iterator ni=nodeList.begin();
                ni!=nodeList.end();
                ++ni)
            {
                group->addChild(*ni);
            }
            osg_root = group;
        }

        ConvertToPerformer converter;
        //converter.setSaveImageDirectory(outpath);
        //converter.setSaveImagesAsRGB(saveImagesAsRGB);

        pfNode* pf_root = converter.convert(osg_root);
        if (pf_root)
        {
            pfdStoreFile(pf_root,outfile.c_str());
        }
        else
        {
            cerr << "error : failed to convert OSG scene graph to Performer."<<endl;
        }

    }
    else
    {
        cerr << "error : failed to convert any OSG database from input files."<<endl;
    }

    pfExit(); 

}
