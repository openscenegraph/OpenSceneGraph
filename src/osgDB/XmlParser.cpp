/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2009 Robert Osfield
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

#include <osgDB/XmlParser>
#include <osgDB/FileUtils>

#include <osg/Notify>

using namespace osgDB;

XmlNode* osgDB::readXmlFile(const std::string& filename,const Options* options)
{
    std::string foundFile = osgDB::findDataFile(filename, options);
    if (!foundFile.empty())
    {
        XmlNode::Input input;
        input.open(foundFile);
        if (!input)
        {
            osg::notify(osg::NOTICE)<<"Could not open XML file: "<<filename<<std::endl;
            return 0;
        }

        input.readAllDataIntoBuffer();

        osg::ref_ptr<XmlNode> root = new XmlNode;
        root->read(input);

        return root.release();
    }
    else
    {
        osg::notify(osg::NOTICE)<<"Could not find XML file: "<<filename<<std::endl;
        return 0;
    }
}

XmlNode* osgDB::readXmlStream(std::istream& fin)
{
    XmlNode::Input input;
    input.attach(fin);
    if (!input)
    {
        osg::notify(osg::NOTICE)<<"Could not attach to XML stream."<<std::endl;
        return 0;
    }

    input.readAllDataIntoBuffer();

    osg::ref_ptr<XmlNode> root = new XmlNode;
    root->read(input);

    return root.release();
}


XmlNode::Input::Input():
    _currentPos(0)
{
    setUpControlMappings();
}

XmlNode::Input::Input(const Input&):
    _currentPos(0)
{
    setUpControlMappings();
}

XmlNode::Input::~Input()
{
}

void XmlNode::Input::setUpControlMappings()
{
    addControlToCharacter("&amp;",'&');
    addControlToCharacter("&lt;",'<');
    addControlToCharacter("&gt;",'>');
    addControlToCharacter("&quot;",'"');
    addControlToCharacter("&apos;",'\'');
}

void XmlNode::Input::addControlToCharacter(const std::string& control, int c)
{
    _controlToCharacterMap[control] = c;
    _characterToControlMap[c] = control;
}

void XmlNode::Input::open(const std::string& filename)
{
    _fin.open(filename.c_str());
}

void XmlNode::Input::attach(std::istream& fin)
{
    std::ios &fios = _fin;
    fios.rdbuf(fin.rdbuf());
}

void XmlNode::Input::readAllDataIntoBuffer()
{
    while(_fin)
    {
        int c = _fin.get();
        if (c>=0 && c<=255)
        {
            _buffer.push_back(c);
        }
    }
}

void XmlNode::Input::skipWhiteSpace()
{
    while(_currentPos<_buffer.size() && _buffer[_currentPos]==' ' || _buffer[_currentPos]=='\t' )
    {
        // osg::notify(osg::NOTICE)<<"_currentPos="<<_currentPos<<"_buffer.size()="<<_buffer.size()<<" v="<<_buffer[_currentPos]<<std::endl;
        ++_currentPos;
    }
    //osg::notify(osg::NOTICE)<<"done"<<std::endl;
}

XmlNode::XmlNode()
{
    type = UNASSIGNED;
}

bool XmlNode::read(Input& input)
{
    if (type == UNASSIGNED) type = ROOT;

    while(input)
    {
        //input.skipWhiteSpace();
        if (input.match("<!--"))
        {
            XmlNode* commentNode = new XmlNode;
            commentNode->type = XmlNode::COMMENT;
            children.push_back(commentNode);

            input += 4;
            XmlNode::Input::size_type end = input.find("-->");
            commentNode->contents = input.substr(0, end);
            if (end!=std::string::npos)
            {
                osg::notify(osg::INFO)<<"Valid Comment record ["<<commentNode->contents<<"]"<<std::endl;
                input += (end+3);
            }
            else
            {
                osg::notify(osg::NOTICE)<<"Error: Unclosed Comment record ["<<commentNode->contents<<"]"<<std::endl;
                input += end;
            }
        }
        else if (input.match("</"))
        {
            input += 2;
            XmlNode::Input::size_type end = input.find(">");
            std::string comment = input.substr(0, end);
            if (end!=std::string::npos)
            {
                osg::notify(osg::INFO)<<"Valid end tag ["<<comment<<"]"<<std::endl;
                input += (end+1);
            }
            else
            {
                osg::notify(osg::NOTICE)<<"Error: Unclosed end tag ["<<comment<<"]"<<std::endl;
                input += end;
            }

            if (comment==name) osg::notify(osg::INFO)<<"end tag is matched correctly"<<std::endl;
            else osg::notify(osg::NOTICE)<<"Error: end tag is not matched correctly"<<std::endl;

            return true;
        }
        else if (input.match("<?"))
        {
            XmlNode* commentNode = new XmlNode;
            commentNode->type = XmlNode::INFORMATION;
            children.push_back(commentNode);

            input += 2;
            XmlNode::Input::size_type end = input.find("?>");
            commentNode->contents = input.substr(0, end);
            if (end!=std::string::npos)
            {
                osg::notify(osg::INFO)<<"Valid infomation record ["<<commentNode->contents<<"]"<<std::endl;
                input += (end+2);
            }
            else
            {
                osg::notify(osg::NOTICE)<<"Error: Unclosed infomation record ["<<commentNode->contents<<"]"<<std::endl;
                input += end;
            }
        }
        else if (input.match("<"))
        {
            XmlNode* childNode = new XmlNode;
            childNode->type = XmlNode::NODE;
            children.push_back(childNode);

            input += 1;

            input.skipWhiteSpace();

            int c = 0;
            while ((c=input[0])>=0 && c!=' ' && c!='>' )
            {
                childNode->name.push_back(c);
                ++input;
            }

            while ((c=input[0])>=0 && c!='>')
            {
                Input::size_type prev_pos = input.currentPosition();

                input.skipWhiteSpace();
                std::string option;
                std::string value;
                while((c=input[0])>=0 && c!='>' && c!='"' && c!='\'' && c!='=' && c!=' ')
                {
                    option.push_back(c);
                    ++input;
                }
                input.skipWhiteSpace();
                if (input[0]=='=')
                {
                    ++input;

                    input.skipWhiteSpace();

                    if (input[0]=='"')
                    {
                        ++input;
                        while((c=input[0])>=0 && c!='"')
                        {
                            value.push_back(c);
                            ++input;
                        }
                        ++input;
                    }
                    else if (input[0]=='\'')
                    {
                        ++input;
                        while((c=input[0])>=0 && c!='\'')
                        {
                            value.push_back(c);
                            ++input;
                        }
                        ++input;
                    }
                    else
                    {
                        ++input;
                        while((c=input[0])>=0 && c!=' ' && c!='"' && c!='\'' && c!='>')
                        {
                            value.push_back(c);
                            ++input;
                        }
                    }
                }

                if (prev_pos == input.currentPosition())
                {
                    osg::notify(osg::NOTICE)<<"Error, parser iterator note advanced, position: "<<input.substr(0,50)<<std::endl;
                    ++input;
                }

                if (!option.empty())
                {
                    osg::notify(osg::NOTICE)<<"Assigning option "<<option<<" with value "<<value<<std::endl;
                    childNode->properties[option] = value;
                }
            }

            if ((c=input[0])>=0 && c=='>' )
            {
                ++input;

                osg::notify(osg::INFO)<<"Valid tag ["<<childNode->name<<"]"<<std::endl;

                bool result = childNode->read(input);
                if (!result) return false;

                if (type==NODE && !children.empty()) type = GROUP;
            }
            else
            {
                osg::notify(osg::NOTICE)<<"Unclosed tag ["<<childNode->name<<"]"<<std::endl;
                return false;
            }

        }
        else
        {
            int c = input[0];

            if (c=='&')
            {
                std::string value;
                while(input && (c=input.get())!=';') { value.push_back(c); }
                value.push_back(c);

                if (input._controlToCharacterMap.count(value)!=0)
                {
                    c = input._controlToCharacterMap[value];
                    osg::notify(osg::INFO)<<"Read control character "<<value<<" converted to "<<char(c)<<std::endl;
                    contents.push_back(c);
                }
                else
                {
                    osg::notify(osg::NOTICE)<<"Warning: read control character "<<value<<", but have no mapping to convert it to."<<std::endl;
                }
            }
            else
            {
                contents.push_back( c );
                ++input;
            }

        }
    }

    if (type==NODE && !children.empty()) type = GROUP;
    return false;
}

bool XmlNode::write(std::ostream& fout) const
{
    switch(type)
    {
        case(UNASSIGNED):
            return false;
        case(ATOM):
        {
            fout<<"<"<<name;
            for(Properties::const_iterator oitr = properties.begin();
                oitr != properties.end();
                ++oitr)
            {
                fout<<oitr->first<<"\"";
                writeString(fout,oitr->second);
                fout<<"\""<<std::endl;
            }
            return true;
        }
        case(ROOT):
        {
            for(Children::const_iterator citr = children.begin();
                citr != children.end();
                ++citr)
            {
                (*citr)->write(fout);
            }
            return true;
        }
        case(NODE):
        {
            fout<<"<"<<name;
            for(Properties::const_iterator oitr = properties.begin();
                oitr != properties.end();
                ++oitr)
            {
                fout<<" "<<oitr->first<<"=\"";
                writeString(fout,oitr->second);
                fout<<"\"";
            }
            fout<<">";

            for(Children::const_iterator citr = children.begin();
                citr != children.end();
                ++citr)
            {
                (*citr)->write(fout);
            }

            if (!contents.empty()) writeString(fout,contents);

            fout<<"</"<<name<<">"<<std::endl;
            return true;
        }
        case(GROUP):
        {
            fout<<"<"<<name;
            for(Properties::const_iterator oitr = properties.begin();
                oitr != properties.end();
                ++oitr)
            {
                fout<<" "<<oitr->first<<"=\"";
                writeString(fout,oitr->second);
                fout<<"\"";
            }
            fout<<">"<<std::endl;

            for(Children::const_iterator citr = children.begin();
                citr != children.end();
                ++citr)
            {
                (*citr)->write(fout);
            }

            fout<<"</"<<name<<">"<<std::endl;
            return true;
        }
        case(COMMENT):
        {
            fout<<"<!--"<<contents<<"-->"<<std::endl;
            return true;
        }
        case(INFORMATION):
        {
            fout<<"<?"<<contents<<"?>"<<std::endl;
            return true;
        }
    }
    return false;
}

bool XmlNode::writeString(std::ostream& fout, const std::string& str) const
{
    fout<<str;
    return true;
}
