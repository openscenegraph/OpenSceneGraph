/* -*-c++-*- Present3D - Copyright (C) 1999-2006 Robert Osfield
 *
 * This software is open source and may be redistributed and/or modified under
 * the terms of the GNU General Public License (GPL) version 2.0.
 * The full license is in LICENSE.txt file included with this distribution,.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * include LICENSE.txt for more details.
*/

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <iostream>

#include "SpellChecker.h"

using namespace p3d;

SpellChecker::SpellChecker()
{
}

void SpellChecker::checkP3dXml(const std::string& filename) const
{
    std::string foundFileName = osgDB::findDataFile( filename );
    if (foundFileName.empty()) return;

    std::ifstream fin(foundFileName.c_str());

    osgDB::XmlNode::Input input;
    input.attach(fin);
    input.readAllDataIntoBuffer();

    osg::ref_ptr<osgDB::XmlNode> doc = new osgDB::XmlNode;
    doc->read(input);

    if (!doc) return;

    checkXml(doc.get());
}

void SpellChecker::checkXml(osgDB::XmlNode* node) const
{
    if (node->name=="page") checkWords(node->contents);
    else if (node->name=="paragraph") checkWords(node->contents);
    else if (node->name=="bullet") checkWords(node->contents);

    for(osgDB::XmlNode::Children::iterator itr = node->children.begin();
        itr != node->children.end();
        ++itr)
    {
        checkXml(itr->get());
    }
}

void SpellChecker::checkWords(const std::string& words) const
{
    OSG_NOTICE<<"--"<<std::endl<<words<<std::endl;


#if 0
    const char alpha[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string::size_type start = words.find_first_of(alpha);
    while(start != std::string::npos)
    {
        std::string::size_type end = words.find_first_not_of(alpha, start+1);

        std::string word = words.substr(start, end-start);
        if (!isCorrect(word))
        {
            OSG_NOTICE<<"Error : "<<word<<std::endl;
        }

        start = (end!=std::string::npos) ? words.find_first_of(alpha, end+1) : std::string::npos;
    }
#endif
}

bool SpellChecker::isCorrect(const std::string& word) const
{
    OSG_NOTICE<<"SpellChecker::isCorrect("<<word<<")"<<std::endl;
    return true;
}

SpellChecker::WordList SpellChecker::suggest(const std::string& word) const
{
    return WordList();
}
