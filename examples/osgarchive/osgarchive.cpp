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

#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>

#include <osgDB/Archive>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <iostream>

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the standard OpenSceneGraph example which loads and visualises 3d models.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
        
    // create DataSet.
    osgDB::Archive::Status status;

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
            files.push_back(arguments[pos]);
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
        archive.create(archiveFilename);
        
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
                osgDB::ReaderWriter::ReadResult result = archive.readObject(*itr);                
                osg::ref_ptr<osg::Object> obj = result.getObject();
                if (obj.valid())
                {
                    if (obj.valid()) osgDB::writeObjectFile(*obj, *itr);
                }
            }
        }
    }

    if (list)
    {        
        std::cout<<"Cannot list at present."<<std::endl;
    }
    
    
    return 0;
}

