#ifndef _SHUTTLECALLBACK_H_
#define _SHUTTLECALLBACK_H_

#include <osg/Node>
#include <osgUtil/Export>

// Callback for handling the SoShuttle node
class ShuttleCallback : public osg::NodeCallback
{
    public:

        ShuttleCallback(const osg::Vec3& startPos, const osg::Vec3& endPos, 
                        float frequency);

        virtual void operator() (osg::Node* node, osg::NodeVisitor* nv);
                
    protected:
    
        osg::Vec3 _startPos;
        osg::Vec3 _endPos;
        float _frequency;

        int _previousTraversalNumber;
        double _previousTime;
        float _angle;
};

#endif
