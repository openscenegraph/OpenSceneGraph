/* -*-c++-*-
*
*  OpenSceneGraph example, osgunittests.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osg/Notify>
#include <osg/ArgumentParser>
#include <osgDB/FileNameUtils>
#include <list>

void runFileNameUtilsTest(osg::ArgumentParser&)
{
typedef std::list<std::string> Strings;
    Strings strings;
    strings.push_back(std::string(""));
    strings.push_back(std::string("myfile"));
    strings.push_back(std::string(".osgt"));
    strings.push_back(std::string("myfile.osgt"));
    strings.push_back(std::string("/myfile.osgt"));
    strings.push_back(std::string("home/robert/myfile.osgt"));
    strings.push_back(std::string("/home/robert/myfile.osgt"));
    strings.push_back(std::string("\\myfile.osgt"));
    strings.push_back(std::string("home\\robert\\myfile.osgt"));
    strings.push_back(std::string("\\home\\robert\\myfile.osgt"));
    strings.push_back(std::string("\\home/robert\\myfile.osgt"));
    strings.push_back(std::string("\\home\\robert/myfile.osgt"));
    strings.push_back(std::string("home/robert/"));
    strings.push_back(std::string("\\home\\robert\\"));
    strings.push_back(std::string("home/robert/myfile"));
    strings.push_back(std::string("\\home\\robert\\myfile"));
    strings.push_back(std::string("home/robert/.osgt"));
    strings.push_back(std::string("\\home\\robert\\.osgt"));
    strings.push_back(std::string("home/robert/myfile.ext.osgt"));
    strings.push_back(std::string("home\\robert\\myfile.ext.osgt"));

    for(Strings::iterator itr = strings.begin();
        itr != strings.end();
        ++itr)
    {
        std::string& str = *itr;
        OSG_NOTICE<<"string="<<str;
        OSG_NOTICE<<"\n\tosgDB::getFilePath(str)="<<osgDB::getFilePath(str);
        OSG_NOTICE<<"\n\tosgDB::getSimpleFileName(str)="<<osgDB::getSimpleFileName(str);
        OSG_NOTICE<<"\n\tosgDB::getStrippedName(str)="<<osgDB::getStrippedName(str);
        OSG_NOTICE<<"\n\tosgDB::getFileExtension(str)="<<osgDB::getFileExtension(str);
        OSG_NOTICE<<"\n\tosgDB::getFileExtensionIncludingDot(str)="<<osgDB::getFileExtensionIncludingDot(str);
        OSG_NOTICE<<"\n\tosgDB::getNameLessExtension(str)="<<osgDB::getNameLessExtension(str);
        OSG_NOTICE<<"\n\tosgDB::getNameLessAllExtensions(str)="<<osgDB::getNameLessAllExtensions(str);
        OSG_NOTICE<<std::endl<<std::endl;
    }
}
