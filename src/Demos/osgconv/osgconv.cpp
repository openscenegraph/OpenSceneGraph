#include <osg/Group>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

int main( int argc, char **argv )
{

    if (argc<2)
    {
        osg::notify(osg::NOTICE)<<endl;
        osg::notify(osg::NOTICE)<<"usage:"<<endl;
        osg::notify(osg::NOTICE)<<"    osgconv [options] infile1 [infile2 ...] outfile"<<endl;
        osg::notify(osg::NOTICE)<<endl;
        osg::notify(osg::NOTICE)<<"options:"<<endl;
        osg::notify(osg::NOTICE)<<"    -l libraryName     - load plugin of name libraryName"<<endl;
        osg::notify(osg::NOTICE)<<"                         i.e. -l osgdb_pfb"<<endl;
        osg::notify(osg::NOTICE)<<"                         Useful for loading reader/writers which can load"<<endl;
        osg::notify(osg::NOTICE)<<"                         other file formats in addition to its extension."<<endl;
        osg::notify(osg::NOTICE)<<"    -e extensionName   - load reader/wrter plugin for file extension"<<endl;
        osg::notify(osg::NOTICE)<<"                         i.e. -e pfb"<<endl;
        osg::notify(osg::NOTICE)<<"                         Useful short hand for specifying full library name as"<<endl;
        osg::notify(osg::NOTICE)<<"                         done with -l above, as it automatically expands to the"<<endl;
        osg::notify(osg::NOTICE)<<"                         full library name appropriate for each platform."<<endl;
        osg::notify(osg::NOTICE)<<endl;

        return 0;
    }

    typedef std::vector<std::string> FileNameList;
    FileNameList fileNames;

    for(int i = 1; i < argc; i++ )
    {

        if (argv[i][0]=='-')
        {
            switch(argv[i][1])
            {
                case('l'):
                    ++i;
                    if (i<argc)
                    {
                        osgDB::Registry::instance()->loadLibrary(argv[i]);
                    }
                    break;
                case('e'):
                    ++i;
                    if (i<argc)
                    {
                        std::string libName = osgDB::Registry::instance()->createLibraryNameForExt(argv[i]);
                        osgDB::Registry::instance()->loadLibrary(libName);
                    }
                    break;
            }
        } else
        {
            fileNames.push_back(argv[i]);
        }

    }

    if (fileNames.empty())
    {
        osg::notify(osg::NOTICE)<<"No files specfied."<<endl;
        return 1;
    }

    if (fileNames.size()==1)
    {
        osg::Node* root = osgDB::readNodeFile(fileNames.front());
        if (root)
        {
            osgDB::writeNodeFile(*root,"converted.osg");
            osg::notify(osg::NOTICE)<<"Data written to 'converted.osg'."<<endl;
        }
        else
        {
            osg::notify(osg::NOTICE)<<"Error no data loaded."<<endl;
            return 1;
        }
    }
    else
    {
        std::string fileNameOut = fileNames.back();
        fileNames.pop_back();

        osg::Group* group = new osg::Group();
        for(FileNameList::iterator itr=fileNames.begin();
            itr<fileNames.end();
            ++itr)
        {
            osg::Node* child = osgDB::readNodeFile(*itr);
            if (child)
            {
                group->addChild(child);
            }
        }

        if (group->getNumChildren()==0)
        {
            osg::notify(osg::NOTICE)<<"Error no data loaded."<<endl;
            return 1;
        }
        else if (group->getNumChildren()==1)
        {
            osgDB::writeNodeFile(*(group->getChild(0)),fileNameOut);
        }
        else
        {
            osgDB::writeNodeFile(*group,fileNameOut);
        }

    }

    return 0;
}
