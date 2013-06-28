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
        input.readAllDataIntoBuffer();

        if (!input)
        {
            OSG_NOTICE<<"Could not open XML file: "<<filename<<std::endl;
            return 0;
        }

        osg::ref_ptr<XmlNode> root = new XmlNode;
        root->read(input);

        return root.release();
    }
    else
    {
        OSG_NOTICE<<"Could not find XML file: "<<filename<<std::endl;
        return 0;
    }
}

std::string osgDB::trimEnclosingSpaces(const std::string& str)
{
    if (str.empty()) return str;

    std::string::size_type start = str.find_first_not_of(' ');
    if (start==std::string::npos) return std::string();

    std::string::size_type end = str.find_last_not_of(' ');
    if (end==std::string::npos) return std::string();

    return std::string(str, start, (end-start)+1);
}


XmlNode* osgDB::readXmlStream(std::istream& fin)
{
    XmlNode::Input input;
    input.attach(fin);
    input.readAllDataIntoBuffer();

    if (!input)
    {
        OSG_NOTICE<<"Could not attach to XML stream."<<std::endl;
        return 0;
    }

    osg::ref_ptr<XmlNode> root = new XmlNode;
    root->read(input);

    return root.release();
}

XmlNode::ControlMap::ControlMap()
{
    setUpControlMappings();
}

void XmlNode::ControlMap::addControlToCharacter(const std::string& control, int c)
{
    _controlToCharacterMap[control] = c;
    _characterToControlMap[c] = control;
}

void XmlNode::ControlMap::setUpControlMappings()
{
    addControlToCharacter("&amp;",'&');
    addControlToCharacter("&lt;",'<');
    addControlToCharacter("&gt;",'>');
    addControlToCharacter("&quot;",'"');
    addControlToCharacter("&apos;",'\'');
}

XmlNode::Input::Input():
    _currentPos(0)
{
}

XmlNode::Input::Input(const Input&):
    ControlMap(),
    _currentPos(0)
{
}

XmlNode::Input::~Input()
{
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
    while(_currentPos<_buffer.size() && (_buffer[_currentPos]==' ' || _buffer[_currentPos]=='\t' || _buffer[_currentPos]=='\n' || _buffer[_currentPos]=='\r'))
    {
        //OSG_NOTICE<<"_currentPos="<<_currentPos<<"_buffer.size()="<<_buffer.size()<<" v="<<int(_buffer[_currentPos])<<std::endl;
        ++_currentPos;
    }
    //OSG_NOTICE<<"done"<<std::endl;
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
                OSG_INFO<<"Valid Comment record ["<<commentNode->contents<<"]"<<std::endl;
                input += (end+3);
            }
            else
            {
                OSG_NOTICE<<"Error: Unclosed Comment record ["<<commentNode->contents<<"]"<<std::endl;
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
                OSG_INFO<<"Valid end tag ["<<comment<<"]"<<std::endl;
                input += (end+1);
            }
            else
            {
                OSG_NOTICE<<"Error: Unclosed end tag ["<<comment<<"]"<<std::endl;
                input += end;
            }

            if (comment==name) { OSG_INFO<<"end tag is matched correctly"<<std::endl; }
            else { OSG_NOTICE<<"Error: end tag is not matched correctly"<<std::endl; }

            return true;
        }
        else if (input.match("<!DOCTYPE"))
        {
            XmlNode* commentNode = new XmlNode;
            commentNode->type = XmlNode::INFORMATION;
            children.push_back(commentNode);

            ++input;
            XmlNode::Input::size_type end = input.find(">");
            commentNode->contents = input.substr(0, end);
            if (end!=std::string::npos)
            {
                OSG_INFO<<"Valid infomation record ["<<commentNode->contents<<"]"<<std::endl;
                input += (end+2);
            }
            else
            {
                OSG_NOTICE<<"Error: Unclosed infomation record ["<<commentNode->contents<<"]"<<std::endl;
                input += end;
            }
        }
        else if (input.match("<![CDATA["))
        {
            XmlNode* commentNode = new XmlNode;
            commentNode->type = XmlNode::INFORMATION;
            children.push_back(commentNode);

            input += 9;
            XmlNode::Input::size_type end = input.find("]]>");
            commentNode->contents = input.substr(0, end);
            if (end!=std::string::npos)
            {
                OSG_INFO<<"Valid infomation record ["<<commentNode->contents<<"]"<<std::endl;
                input += (end+2);
            }
            else
            {
                OSG_NOTICE<<"Error: Unclosed infomation record ["<<commentNode->contents<<"]"<<std::endl;
                input += end;
            }
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
                OSG_INFO<<"Valid infomation record ["<<commentNode->contents<<"]"<<std::endl;
                input += (end+2);
            }
            else
            {
                OSG_NOTICE<<"Error: Unclosed infomation record ["<<commentNode->contents<<"]"<<std::endl;
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
            while ((c=input[0])>=0 && c!=' ' && c!='\n' && c!='\r' && c!='>' && c!='/')
            {
                childNode->name.push_back(c);
                ++input;
            }

            while ((c=input[0])>=0 && c!='>' && c!='/')
            {
                Input::size_type prev_pos = input.currentPosition();

                input.skipWhiteSpace();
                std::string option;
                std::string value;

                if (input[0]=='"')
                {
                    option.push_back(input[0]);
                    ++input;
                    while((c=input[0])>=0 && c!='"')
                    {
                        if (c=='&')
                            readAndReplaceControl(option, input);
                        else
                        {
                            option.push_back(c);
                            ++input;
                        }
                    }
                    option.push_back(input[0]);
                    ++input;
                }
                else
                {
                    while((c=input[0])>=0 && c!='>' && c!='/' && c!='"' && c!='\'' && c!='=' && c!=' ' && c!='\n' && c!='\r')
                    {
                        option.push_back(c);
                        ++input;
                    }
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
                            if (c=='&')
                                readAndReplaceControl(value, input);
                            else
                            {
                                value.push_back(c);
                                ++input;
                            }
                        }
                        ++input;
                    }
                    else if (input[0]=='\'')
                    {
                        ++input;
                        while((c=input[0])>=0 && c!='\'')
                        {
                            if (c=='&')
                                readAndReplaceControl(value, input);
                            else
                            {
                                value.push_back(c);
                                ++input;
                            }
                        }
                        ++input;
                    }
                    else
                    {
                        ++input;
                        while((c=input[0])>=0 && c!=' ' && c!='\n' && c!='\r' && c!='"' && c!='\'' && c!='>')
                        {
                            value.push_back(c);
                            ++input;
                        }
                    }
                }

                if (prev_pos == input.currentPosition())
                {
                    OSG_NOTICE<<"Error, parser iterator not advanced, position: "<<input.substr(0,50)<<std::endl;
                    ++input;
                }

                if (!option.empty())
                {
                    OSG_INFO<<"Assigning option "<<option<<" with value "<<value<<std::endl;
                    childNode->properties[option] = value;
                }
            }

            if ((c=input[0])>=0 && (c=='>' || c=='/'))
            {
                ++input;

                OSG_INFO<<"Valid tag ["<<childNode->name<<"]"<<std::endl;

                if (c=='/')
                {
                    if ((c=input[0])>=0 && c=='>')
                    {
                        ++input;
                        OSG_INFO<<"tag is closed correctly"<<std::endl;
                        childNode->type = ATOM;
                    }
                    else
                        OSG_NOTICE<<"Error: tag is not closed correctly"<<std::endl;
                }
                else
                {
                    bool result = childNode->read(input);
                    if (!result) return false;
                }

                if (type==NODE && !children.empty()) type = GROUP;
            }
            else
            {
                OSG_NOTICE<<"Unclosed tag ["<<childNode->name<<"]"<<std::endl;
                return false;
            }

        }
        else
        {
            int c = input[0];

            if (c=='&')
            {
                readAndReplaceControl(contents, input);
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

bool XmlNode::write(std::ostream& fout, const std::string& indent) const
{
    ControlMap controlMap;
    return write(controlMap, fout, indent);
}

bool XmlNode::write(const ControlMap& controlMap, std::ostream& fout, const std::string& indent) const
{
    switch(type)
    {
        case(UNASSIGNED):
            OSG_NOTICE<<"UNASSIGNED"<<std::endl;
            return false;
        case(ATOM):
        {
            fout<<indent<<"<"<<name;
            writeProperties(controlMap, fout);
            fout<<" />"<<std::endl;
            return true;
        }
        case(ROOT):
        {
            writeChildren(controlMap, fout, indent);
            return true;
        }
        case(NODE):
            fout<<indent<<"<"<<name;
            writeProperties(controlMap,fout);
            fout<<">"; writeString(controlMap, fout, contents); fout<<"</"<<name<<">"<<std::endl;
            return true;
        case(GROUP):
        {
            fout<<indent<<"<"<<name;
            writeProperties(controlMap,fout);
            fout<<">"<<std::endl;

            writeChildren(controlMap, fout, indent + "  ");

            fout<<indent<<"</"<<name<<">"<<std::endl;
            return true;
        }
        case(COMMENT):
        {
            fout<<indent<<"<!--"<<contents<<"-->"<<std::endl;
            return true;
        }
        case(INFORMATION):
        {
            fout<<indent<<"<?"<<contents<<"?>"<<std::endl;
            return true;
        }
    }
    return false;
}

bool XmlNode::writeString(const ControlMap& controlMap, std::ostream& fout, const std::string& str) const
{
    for(std::string::const_iterator itr = str.begin();
        itr != str.end();
        ++itr)
    {
        int c = *itr;
        ControlMap::CharacterToControlMap::const_iterator citr = controlMap._characterToControlMap.find(c);
        if (citr != controlMap._characterToControlMap.end()) fout << citr->second;
        else fout.put(c);
    }
    return true;
}

bool XmlNode::writeChildren(const ControlMap& /*controlMap*/, std::ostream& fout, const std::string& indent) const
{
    for(Children::const_iterator citr = children.begin();
        citr != children.end();
        ++citr)
    {
        if (!(*citr)->write(fout, indent))
            return false;
    }

    return true;
}

bool XmlNode::writeProperties(const ControlMap& controlMap, std::ostream& fout) const
{
    for(Properties::const_iterator oitr = properties.begin();
        oitr != properties.end();
        ++oitr)
    {
        fout<<" "<<oitr->first<<"=\"";
        if (!writeString(controlMap,fout,oitr->second))
            return false;
        fout<<"\"";
    }

    return true;
}

bool XmlNode::readAndReplaceControl(std::string& contents, XmlNode::Input& input)
{
    int c = 0;
    std::string value;
    while(input && (c=input.get())!=';') { value.push_back(c); }
    value.push_back(c);

    if (input._controlToCharacterMap.count(value)!=0)
    {
        c = input._controlToCharacterMap[value];
        OSG_INFO<<"Read control character "<<value<<" converted to "<<char(c)<<std::endl;
        contents.push_back(c);
        return true;
    }
    else
    {
        OSG_NOTICE<<"Warning: read control character "<<value<<", but have no mapping to convert it to."<<std::endl;
        return false;
    }
}
