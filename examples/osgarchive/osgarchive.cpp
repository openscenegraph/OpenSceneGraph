/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osg/Timer>
#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>

#include <osgDB/Archive>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>

#include <iostream>
#include <algorithm>


int main( int argc, char **argv )
{
/*
    
    std::fstream fout("test.data",std::ofstream::out | std::ofstream::binary);
    unsigned int numCharacters = 26;
    char baseCharacter = 'A';
    
    for(unsigned int i=0;i<numCharacters;++i)
    {
        char c = baseCharacter + i;
        fout.write(&c,1);
    }
    fout.close();
    
    fout.open("test.data",std::ofstream::out | std::ofstream::in | std::ofstream::binary);
    
    char offset = 'a'-'A';
    unsigned int start_range = 5;
    unsigned int end_range = 15;
    
    fout.seekp(start_range);
    for(unsigned int i=start_range;i<end_range;++i)
    {
        char c = (baseCharacter + i)+offset ;
        fout.write(&c,1);
    }
    
    fout.close();       

*/
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the standard OpenSceneGraph example which loads and visualises 3d models.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
        
    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    std::string archiveFilename;
    while (arguments.read("-a",archiveFilename) || arguments.read("--archive",archiveFilename))
    {
    }

    bool insert = false;
    while (arguments.read("-i") || arguments.read("--insert"))
    {
        insert = true;
    }
    
    bool extract = false;
    while (arguments.read("-e") || arguments.read("--extract"))
    {
        extract = true;
    }
    
    bool list = false;    
    while (arguments.read("-l") || arguments.read("--list"))
    {
        list = true;
    }

    typedef std::vector<std::string> FileNameList;
    FileNameList files;
    for(int pos=1;pos<arguments.argc();++pos)
    {
        if (!arguments.isOption(pos))
        {
            if (insert)
            {
                osgDB::FileType fileType = osgDB::fileType(arguments[pos]);
                if (fileType==osgDB::REGULAR_FILE)
                {
                    files.push_back(arguments[pos]);
                }
                else if (fileType==osgDB::DIRECTORY)
                {
                    osgDB::DirectoryContents directory = osgDB::getDirectoryContents(arguments[pos]);
                    files.insert(files.end(),directory.begin(),directory.end());
                }
            }
            else
            {
                files.push_back(arguments[pos]);
            }
        }
    }
    
    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
    if (archiveFilename.empty())
    {
        std::cout<<"Please specify an archive name using --archive filename"<<std::endl;
        return 1;
    }

    if (!insert && !extract && !list)
    {
        std::cout<<"Please specify an operation on the archive, either --insert, --extract or --list"<<std::endl;
        return 1;
    }
    
    if (insert && extract)
    {
        std::cout<<"Cannot insert and extract files from the archive at one time, please use either --insert or --extract."<<std::endl;
        return 1;
    }

    osgDB::Archive archive;

    if (insert)
    {
        osgDB::Archive archive;
        archive.open(archiveFilename, osgDB::Archive::WRITE);
        
        for (FileNameList::iterator itr=files.begin();
            itr!=files.end();
            ++itr)
        {
            osg::ref_ptr<osg::Object> obj = osgDB::readObjectFile(*itr);
            if (obj.valid())
            {
                archive.writeObject(*obj, *itr);
            }
        }
    }
    else 
    {
        archive.open(archiveFilename,osgDB::Archive::READ);
        
        if (extract)
        {
            for (FileNameList::iterator itr=files.begin();
                itr!=files.end();
                ++itr)
            {
                osg::Timer_t start = osg::Timer::instance()->tick();
                osgDB::ReaderWriter::ReadResult result = archive.readObject(*itr);                
                osg::ref_ptr<osg::Object> obj = result.getObject();
                std::cout<<"readObejct time = "<<osg::Timer::instance()->delta_m(start,osg::Timer::instance()->tick())<<std::endl;
                if (obj.valid())
                {
                    if (obj.valid()) osgDB::writeObjectFile(*obj, *itr);
                }
            }
        }
    }

    if (list)
    {        
        std::cout<<"List of files in archive:"<<std::endl;
        const osgDB::Archive::FileNamePositionMap& indexMap = archive.getFileNamePositionMap();
        for(osgDB::Archive::FileNamePositionMap::const_iterator itr=indexMap.begin();
            itr!=indexMap.end();
            ++itr)
        {
            std::cout<<"    "<<itr->first<<"\t"<<itr->second.first<<"\t"<<itr->second.second<<std::endl;
        }

    }
    
    return 0;
}

