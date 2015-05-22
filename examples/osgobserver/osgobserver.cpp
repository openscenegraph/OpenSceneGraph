/* OpenSceneGraph example, osgintersection.
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

#include <iostream>
#include <memory>
#include <ostream>

#include <osg/Node>
#include <osg/PositionAttitudeTransform>
  
#include <osgDB/ReadFile>

#include <osgGA/GUIEventAdapter>

#include <osgViewer/Viewer>


class Observer : public osg::Observer
{
public:

    void objectStateChanged(void* obj, osg::ObserverRecord* record)
    {
        if ( auto rec = dynamic_cast<osg::Group::ObserverRecord*>(record) )
        {
            std::pair<unsigned, unsigned> range = rec->getRange();
            std::cout << "Inserted " << (range.second - range.first)
                      << " child(ren) at position " << range.first << std::endl;
        }

        if ( auto rec = dynamic_cast<osg::Transform::ObserverRecord*>(record) )
        {
            (void)rec;
            std::cout << "Position changed" << std::endl;
        }
    }
};


class KeyHandler : public osgGA::GUIEventHandler
{
public:

    KeyHandler(osg::ref_ptr<osg::PositionAttitudeTransform> const& group) : group_(group) {}

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&)
    {
        if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
        {
            switch (ea.getKey())
            {

            case 'i':
                std::cout << "Let's add a node." << std::endl;
                group_->addChild(new osg::Node);
                return true;

            case 'j':
                group_->setPosition( osg::Vec3f(0.0f, 0.0f, 0.0f) );
                return true;

            default:
                return false;

            }
        }

        return false;
    }

private:

    osg::ref_ptr<osg::PositionAttitudeTransform> const& group_;

};


int main(int argc, char **argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
   
    osg::ref_ptr<osg::Node> mesh = osgDB::readNodeFiles(arguments);
   
    if (!mesh) 
    {
        std::cout<<"No model loaded, please specify a valid model on the command line."<<std::endl;
        return 0;
    }

    osg::ref_ptr<osg::PositionAttitudeTransform> mt = new osg::PositionAttitudeTransform;
    mt->addChild(mesh.get());

    std::shared_ptr<Observer> co = std::make_shared<Observer>();
    mt->addObserver(co.get());

    osgViewer::Viewer viewer(arguments);
    viewer.setSceneData(mt.get());
    KeyHandler* keyHandler = new KeyHandler(mt);

    viewer.addEventHandler( keyHandler ); 
    return viewer.run();
}
