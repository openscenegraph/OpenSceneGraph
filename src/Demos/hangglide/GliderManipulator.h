#ifndef HANGGLIDE_GLIDERMANIPULATOR
#define HANGGLIDE_GLIDERMANIPULATOR 1

#include <osgUtil/CameraManipulator>

class GliderManipulator : public osgUtil::CameraManipulator
{
    public:

	GliderManipulator();
	virtual ~GliderManipulator();

        /** Attach a node to the manipulator. 
            Automatically detaches previously attached node.
            setNode(NULL) detaches previously nodes.
            Is ignored by manipulators which do not require a reference model.*/
        virtual void setNode(osg::Node*);

        /** Return node if attached.*/
        virtual const osg::Node* getNode() const;

        /** Move the camera to the default position. 
            May be ignored by manipulators if home functionality is not appropriate.*/
        virtual void home(const osgUtil::GUIEventAdapter& ea,osgUtil::GUIActionAdapter& us);
        
        /** Start/restart the manipulator.*/
        virtual void init(const osgUtil::GUIEventAdapter& ea,osgUtil::GUIActionAdapter& us);

        /** handle events, return true if handled, false otherwise.*/
	virtual bool handle(const osgUtil::GUIEventAdapter& ea,osgUtil::GUIActionAdapter& us);

        enum YawControlMode {
            YAW_AUTOMATICALLY_WHEN_BANKED,
            NO_AUTOMATIC_YAW
        };

        /** Set the yaw control between no yaw and yawing when banked.*/
        void setYawControlMode(YawControlMode ycm) { _yawMode = ycm; }

    private:

        /** Reset the internal GUIEvent stack.*/
        void flushMouseEventStack();
        /** Add the current mouse GUIEvent to internal stack.*/
        void addMouseEvent(const osgUtil::GUIEventAdapter& ea);

        /** For the give mouse movement calculate the movement of the camera.
            Return true is camera has moved and a redraw is required.*/
        bool calcMovement();

        // Internal event stack comprising last three mouse events.
        osg::ref_ptr<const osgUtil::GUIEventAdapter> _ga_t1;
        osg::ref_ptr<const osgUtil::GUIEventAdapter> _ga_t0;

        osg::ref_ptr<osg::Node>       _node;

        float _modelScale;
        float _velocity;
        
        YawControlMode _yawMode;

};

#endif

