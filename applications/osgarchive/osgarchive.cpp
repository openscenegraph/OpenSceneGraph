/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This application is open source and may be redistributed and/or modified
 * freely and without restriction, both in commercial and non commercial applications,
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
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is an application for collecting a set of separate files into a single archive file that can be later read in OSG applications..");
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
                std::string filePath = osgDB::findDataFile(arguments[pos]);
                osgDB::FileType fileType = osgDB::fileType(filePath);
                if (fileType==osgDB::REGULAR_FILE)
                {
                    files.push_back(arguments[pos]);
                }
                else if (fileType==osgDB::DIRECTORY)
                {
                    osgDB::DirectoryContents directory = osgDB::getDirectoryContents(arguments[pos]);
                    osgDB::DirectoryContents::iterator it = directory.begin();
                    while( it != directory.end())
                    {
                        files.push_back(filePath + "/" + (*it));
                        ++it;
                    }
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

    // report any errors if they have occurred when parsing the program arguments.
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

    osg::ref_ptr<osgDB::Archive> archive;

    if (insert)
    {
        archive = osgDB::openArchive(archiveFilename, osgDB::Archive::WRITE);

        if (archive.valid())
        {
            for (FileNameList::iterator itr=files.begin();
                itr!=files.end();
                ++itr)
            {
                std::cout<<"reading "<<*itr<<std::endl;
                osg::ref_ptr<osg::Object> obj = osgDB::readRefObjectFile(*itr);
                if (obj.valid())
                {
                    std::cout<<"  write to archive "<<*itr<<std::endl;
                    osg::Image* image = dynamic_cast<osg::Image*>(obj.get());
                    osg::HeightField* hf = dynamic_cast<osg::HeightField*>(obj.get());
                    osg::Node* node = dynamic_cast<osg::Node*>(obj.get());
                    osg::Shader* shader = dynamic_cast<osg::Shader*>(obj.get());
                    if (image) archive->writeImage(*image, *itr);
                    else if (hf) archive->writeHeightField(*hf, *itr);
                    else if (node) archive->writeNode(*node, *itr);
                    else if (shader) archive->writeShader(*shader, *itr);
                    else archive->writeObject(*obj, *itr);
                }
            }
        }
    }
    else
    {
        archive = osgDB::openArchive(archiveFilename, osgDB::Archive::READ);

        if (extract && archive.valid())
        {
            for (FileNameList::iterator itr=files.begin();
                itr!=files.end();
                ++itr)
            {
                osg::Timer_t start = osg::Timer::instance()->tick();
                osgDB::ReaderWriter::ReadResult result = archive->readObject(*itr);
                osg::ref_ptr<osg::Object> obj = result.getObject();
                std::cout<<"readObejct time = "<<osg::Timer::instance()->delta_m(start,osg::Timer::instance()->tick())<<std::endl;
                if (obj.valid())
                {
                    osgDB::writeObjectFile(*obj, *itr);
                }
            }
        }
    }

    if (list && archive.valid())
    {
        std::cout<<"List of files in archive:"<<std::endl;
        osgDB::Archive::FileNameList fileNames;
        if (archive->getFileNames(fileNames))
        {
            for(osgDB::Archive::FileNameList::const_iterator itr=fileNames.begin();
                itr!=fileNames.end();
                ++itr)
            {
                std::cout<<"    "<<*itr<<std::endl;
            }
        }

        std::cout<<std::endl;
        std::cout<<"Master file "<<archive->getMasterFileName()<<std::endl;
    }

    return 0;
}

