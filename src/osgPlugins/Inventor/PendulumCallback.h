#ifndef _PENDULUMCALLBACK_H_
#define _PENDULUMCALLBACK_H_

#include <osg/Node>
#include <osgUtil/Export>

// Callback for handling the SoPendulum node
class PendulumCallback : public osg::NodeCallback
{
    public:

        PendulumCallback(const osg::Vec3& axis, float startAngle,
                         float endAngle, float frequency);

        virtual void operator() (osg::Node* node, osg::NodeVisitor* nv);

    protected:

        float _startAngle;
        float _endAngle;
        float _frequency;
        osg::Vec3 _axis;

        unsigned int _previousTraversalNumber;
        double _previousTime;
        float _angle;
};

#endif
