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

#ifndef OSGGA_GliderMANIPULATOR
#define OSGGA_GliderMANIPULATOR 1

#include <osgGA/MatrixManipulator>
#include <osg/Quat>

/**
GliderManipulator is a MatrixManipulator which provides Glider simulator-like
updating of the camera position & orientation. By default, the left mouse
button accelerates, the right mouse button decelerates, and the middle mouse
button (or left and right simultaneously) stops dead.
*/

class GliderManipulator : public osgGA::MatrixManipulator
{
    public:

        GliderManipulator();

        virtual const char* className() const { return "Glider"; }

        /** set the position of the matrix manipulator using a 4x4 Matrix.*/
        virtual void setByMatrix(const osg::Matrixd& matrix);

        /** set the position of the matrix manipulator using a 4x4 Matrix.*/
        virtual void setByInverseMatrix(const osg::Matrixd& matrix) { setByMatrix(osg::Matrixd::inverse(matrix)); }

        /** get the position of the manipulator as 4x4 Matrix.*/
        virtual osg::Matrixd getMatrix() const;

        /** get the position of the manipulator as a inverse matrix of the manipulator, typically used as a model view matrix.*/
        virtual osg::Matrixd getInverseMatrix() const;


        virtual void setNode(osg::Node*);

        virtual const osg::Node* getNode() const;

        virtual osg::Node* getNode();

        virtual void home(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us);

        virtual void init(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us);

        virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us);

        /** Get the keyboard and mouse usage of this manipulator.*/
        virtual void getUsage(osg::ApplicationUsage& usage) const;

        enum YawControlMode {
                YAW_AUTOMATICALLY_WHEN_BANKED,
                NO_AUTOMATIC_YAW
        };

        /**        Configure the Yaw control for the Glider model.        */
        void setYawControlMode(YawControlMode ycm) { _yawMode = ycm; }

    protected:

        virtual ~GliderManipulator();

        /** Reset the internal GUIEvent stack.*/
        void flushMouseEventStack();
        /** Add the current mouse GUIEvent to internal stack.*/
        void addMouseEvent(const osgGA::GUIEventAdapter& ea);

        void computePosition(const osg::Vec3& eye,const osg::Vec3& lv,const osg::Vec3& up);

        /** For the give mouse movement calculate the movement of the camera.
            Return true is camera has moved and a redraw is required.*/
        bool calcMovement();


        // Internal event stack comprising last three mouse events.
        osg::ref_ptr<const osgGA::GUIEventAdapter> _ga_t1;
        osg::ref_ptr<const osgGA::GUIEventAdapter> _ga_t0;

        osg::ref_ptr<osg::Node>       _node;

        float _modelScale;
        float _velocity;

        YawControlMode _yawMode;
        
        osg::Vec3   _eye;
        osg::Quat   _rotation;
        float       _distance;

};

#endif

