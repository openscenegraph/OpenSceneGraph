/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osg/Notify>

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

SpellChecker::WordList SpellChecker::suggest(const std::string& /*word*/) const
{
    return WordList();
}

//////////////////////////////////////////////////////////////////////////////////////////
XmlPatcher::XmlPatcher()
{
}

void XmlPatcher::stripP3dXml(const std::string& filename, std::ostream& fout) const
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

    stripXml(doc.get(), fout);
}

void XmlPatcher::stripXml(osgDB::XmlNode* node, std::ostream& fout) const
{
    if (node->name=="presentation" ||
        node->name=="slide" ||
        node->name=="layer" ||
        node->name=="page" ||
        node->name=="paragraph" ||
        node->name=="bullet")
    {
        if (!(node->children.empty()))
        {
            fout<<"<"<<node->name<<">"<<std::endl;
            for(osgDB::XmlNode::Children::iterator itr = node->children.begin();
                itr != node->children.end();
                ++itr)
            {
                stripXml(itr->get(), fout);
            }
            fout<<"</"<<node->name<<">"<<std::endl;
        }
        else
        {
            fout<<"<"<<node->name<<">"<<node->contents<<"</"<<node->name<<">"<<std::endl;
        }
    }
    else
    {
        for(osgDB::XmlNode::Children::iterator itr = node->children.begin();
            itr != node->children.end();
            ++itr)
        {
            stripXml(itr->get(), fout);
        }
    }
}

osgDB::XmlNode* XmlPatcher::simplifyP3dXml(const std::string& filename) const
{
    std::string foundFileName = osgDB::findDataFile( filename );
    if (foundFileName.empty()) return 0;

    std::ifstream fin(foundFileName.c_str());

    osgDB::XmlNode::Input input;
    input.attach(fin);
    input.readAllDataIntoBuffer();

    osg::ref_ptr<osgDB::XmlNode> doc = new osgDB::XmlNode;
    doc->read(input);

    if (!doc) return 0;

    return simplifyXml(doc.get());
}

osgDB::XmlNode* XmlPatcher::simplifyXml(osgDB::XmlNode* node) const
{
    if (node->name.empty() ||
        node->name=="presentation" ||
        node->name=="slide" ||
        node->name=="layer" ||
        node->name=="page" ||
        node->name=="paragraph" ||
        node->name=="bullet")
    {
        osgDB::XmlNode* newNode = new osgDB::XmlNode;
        newNode->type = node->type;
        newNode->name = node->name;
        newNode->contents = node->contents;
        for(osgDB::XmlNode::Children::iterator itr = node->children.begin();
            itr != node->children.end();
            ++itr)
        {
            osgDB::XmlNode* child = simplifyXml(itr->get());
            if (child)  newNode->children.push_back(child);
        }
        return newNode;
    }
    else
    {
        return 0;
    }
}
osgDB::XmlNode* XmlPatcher::mergeP3dXml(const std::string& lhs_filename, const std::string& rhs_filename) const
{
    std::string lhs_foundFileName = osgDB::findDataFile( lhs_filename );
    if (lhs_foundFileName.empty()) return 0;

    std::string rhs_foundFileName = osgDB::findDataFile( rhs_filename );
    if (rhs_foundFileName.empty()) return 0;

    osg::ref_ptr<osgDB::XmlNode> lhs_doc = new osgDB::XmlNode;
    osg::ref_ptr<osgDB::XmlNode> rhs_doc = new osgDB::XmlNode;

    {
        std::ifstream fin(lhs_foundFileName.c_str());

        osgDB::XmlNode::Input input;
        input.attach(fin);
        input.readAllDataIntoBuffer();

        lhs_doc->read(input);
        if (!lhs_doc) return 0;
    }

    {
        std::ifstream fin(rhs_foundFileName.c_str());

        osgDB::XmlNode::Input input;
        input.attach(fin);
        input.readAllDataIntoBuffer();

        rhs_doc->read(input);
        if (!rhs_doc) return 0;
    }

    lhs_doc = mergeXml(lhs_doc.get(), rhs_doc.get());
    return lhs_doc.release();
}

osgDB::XmlNode* XmlPatcher::mergeXml(osgDB::XmlNode* lhs_node, osgDB::XmlNode* rhs_node) const
{
    if (lhs_node->name == rhs_node->name)
    {
        lhs_node->contents = rhs_node->contents;
        osgDB::XmlNode::Children::iterator rhs_itr = rhs_node->children.begin();
        for(osgDB::XmlNode::Children::iterator lhs_itr = lhs_node->children.begin();
            lhs_itr != lhs_node->children.end() && rhs_itr != rhs_node->children.end();
            ++lhs_itr)
        {
            if ((*lhs_itr)->name == (*rhs_itr)->name)
            {
                mergeXml(lhs_itr->get(), rhs_itr->get());
                ++rhs_itr;
            }
        }
    }
    return lhs_node;
}
