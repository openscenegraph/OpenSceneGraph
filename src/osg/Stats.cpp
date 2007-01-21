/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2007 Robert Osfield 
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

#include <osg/Stats>
#include <osg/Notify>

using namespace osg;

Stats::Stats(const std::string& name, unsigned int numberOfFrames):
    _name(name)
{
    allocate(numberOfFrames);
}

void Stats::allocate(unsigned int numberOfFrames)
{
    _baseFrameNumber = 0;
    _latestFrameNumber  = 0;
    _attributeMapList.clear();
    _attributeMapList.resize(numberOfFrames);
}


bool Stats::setAttribute(int frameNumber, const std::string& attributeName, double value)
{
    if (frameNumber<getEarliestFrameNumber()) return false;
    if (frameNumber>_latestFrameNumber)
    {
        // need to advance 
        
        // first clear the entries up to and including the new frameNumber
        for(int i = _latestFrameNumber+1; i<= frameNumber; ++i)
        {
            int index = (i - _baseFrameNumber) % _attributeMapList.size();
            _attributeMapList[index].clear();
        }
        
        if ( (frameNumber-_baseFrameNumber) >= static_cast<int>(_attributeMapList.size()))
        {
            _baseFrameNumber = (frameNumber/_attributeMapList.size())*_attributeMapList.size();
        }
        
        _latestFrameNumber = frameNumber;
        
    }

    int index = getIndex(frameNumber);
    if (index<0) 
    {
        osg::notify(osg::NOTICE)<<"Failed to assing valid index for Stats::setAttribute("<<frameNumber<<","<<attributeName<<","<<value<<")"<<std::endl;
        return false;
    }
    
    AttributeMap& attributeMap = _attributeMapList[index];
    attributeMap[attributeName] = value;

    return true;
}

bool Stats::getAttribute(int frameNumber, const std::string& attributeName, double& value) const
{
    int index = getIndex(frameNumber);
    if (index<0) return false;
    
    const AttributeMap& attributeMap = _attributeMapList[index];
    AttributeMap::const_iterator itr = attributeMap.find(attributeName);
    if (itr == attributeMap.end()) return false;

    value = itr->second;    
    return true;
}

Stats::AttributeMap& Stats::getAttributeMap(int frameNumber)
{
    int index = getIndex(frameNumber);
    if (index<0) return _invalidAttributeMap;
    
    return _attributeMapList[index];
}

const Stats::AttributeMap& Stats::getAttributeMap(int frameNumber) const
{
    int index = getIndex(frameNumber);
    if (index<0) return _invalidAttributeMap;
    
    return _attributeMapList[index];
}

void Stats::report(std::ostream& out)
{
    out<<"Stats "<<_name<<std::endl;
    for(int i = getEarliestFrameNumber(); i<= getLatestFrameNumber(); ++i)
    {
        out<<" FrameNumber "<<i<<std::endl;
        osg::Stats::AttributeMap& attributes = getAttributeMap(i);
        for(osg::Stats::AttributeMap::iterator itr = attributes.begin();
            itr != attributes.end();
            ++itr)
        {
            out<<"    "<<itr->first<<"\t"<<itr->second<<std::endl;
        }

    }
}
