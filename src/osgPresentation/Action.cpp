/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2013 Robert Osfield
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

#include <osgPresentation/Action>

#include <osgPresentation/Group>
#include <osgPresentation/Element>
#include <osgPresentation/Text>
#include <osgPresentation/Model>
#include <osgPresentation/Volume>
#include <osgPresentation/Movie>
#include <osgPresentation/Image>
#include <osgPresentation/Section>
#include <osgPresentation/Layer>
#include <osgPresentation/Slide>
#include <osgPresentation/Presentation>

using namespace osgPresentation;

//////////////////////////////////////////////////////////////////////////////////
//
//  Action base class
//
void Action::apply(osgPresentation::Group& group)
{
    OSG_NOTICE<<"LoadAction::apply()"<<std::endl;

    osg::NodeVisitor::apply(group);
}

void Action::apply(osgPresentation::Element& element)
{
    apply(static_cast<osgPresentation::Group&>(element));
}

void Action::apply(osgPresentation::Text& text)
{
    apply(static_cast<osgPresentation::Element&>(text));
}

void Action::apply(osgPresentation::Volume& volume)
{
    apply(static_cast<osgPresentation::Element&>(volume));
}

void Action::apply(osgPresentation::Model& model)
{
    apply(static_cast<osgPresentation::Element&>(model));
}

void Action::apply(osgPresentation::Image& image)
{
    apply(static_cast<osgPresentation::Element&>(image));
}

void Action::apply(osgPresentation::Movie& movie)
{
    apply(static_cast<osgPresentation::Element&>(movie));
}


void Action::apply(osgPresentation::Section& section)
{
    apply(static_cast<osgPresentation::Group&>(section));
}

void Action::apply(osgPresentation::Layer& layer)
{
    apply(static_cast<osgPresentation::Group&>(layer));
}

void Action::apply(osgPresentation::Slide& slide)
{
    apply(static_cast<osgPresentation::Group&>(slide));
}

void Action::apply(osgPresentation::Presentation& presentation)
{
    apply(static_cast<osgPresentation::Group&>(presentation));
}


/////////////////////////////////////////////////////////////////////////
//
// Specific Action implementations
//
void LoadAction::apply(osgPresentation::Element& element)
{
    OSG_NOTICE<<"LoadAction::apply()"<<std::endl;
    element.load();
}

void UnloadAction::apply(osgPresentation::Element& element)
{
    element.unload();
}

void ResetAction::apply(osgPresentation::Element& element)
{
    element.reset();
}

void PauseAction::apply(osgPresentation::Element& element)
{
    element.pause();
}

void PlayAction::apply(osgPresentation::Element& element)
{
    element.play();
}

void PrintSupportedProperties::apply(osgPresentation::Group& group)
{
    _output<<"osgPresentation object : "<<group.className()<<std::endl;
    osgPresentation::PropertyList properties;
    if (group.getSupportedProperties(properties))
    {
        for(osgPresentation::PropertyList::iterator itr = properties.begin();
            itr != properties.end();
            ++itr)
        {
            osgPresentation::ObjectDescription& od = *itr;
            _output<<"    "<<od.first->className()<<" : "<<od.first->getName()<<", description = "<<od.second<<std::endl;

        }
    }
    traverse(group);
}
