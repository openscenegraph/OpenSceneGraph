/*
 * IGES importer for OpenSceneGraph.
 * Copyright (c)2013 VizExperts India Pvt. Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _READERWRITERIGES_H_
#define _READERWRITERIGES_H_

/// \file ReaderWritterOpenCASCADE.h
/// \brief header file for creating osgdb plugin for IGES format
/// \author Abhishek Bansal, Engineer Graphics, vizExperts India Pvt. Ltd. 

#ifdef WIN32
/// \brief preproccessor macro required for compilation with open cascade
/// \todo not sure what it does
#define WNT
#endif

#include <TDF_LabelSequence.hxx>

#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>

#include <TopoDS_Shape.hxx>
#include <gp_Trsf.hxx>

#include <osg/Notify>
#include <osg/Geode>
#include <osg/Geometry>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

/// \class ReaderWritterOpenCASCADE
/// \brief contains implementation of reading IGES models
///        depends on OpenCascade library
///        this code was written with version 6.6.0
/// \todo enabling/disabling Healing can be added as reader writer options

class ReaderWritterOpenCASCADE: public osgDB::ReaderWriter
{
    public:
        /// \brief constructor
        ReaderWritterOpenCASCADE();

        /// \brief returns class name
        virtual const char* className() const { return "STEP/IGES Reader"; }


        virtual ReadResult readObject(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            return readNode(fileName, options);
        }

        virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*) const;

        virtual osgDB::ReaderWriter::WriteResult writeNode(const osg::Node&, const std::string&, const Options* =NULL) const ;

    private:

        /// \brief following class will contain all reading related functionality
        /// \detail this class uses OCT XDE module to read IGES/STEP file. XDE mechanism is needed
        ///         to find out colors and transformation of sub shapes.
        ///         normal OCCTKReader wasn't giving very good results with shapes. Edges weren't sharp enough
        ///         and also there is no way in which we can get color information with that
        /// \Note Go through XDE user guide and IGES User guide supplied with
        /// \todo OSG automatic normal calculation is not working good for few mnodels
        ///       try to get from XDE document only
        class OCCTKReader
        {
            public:

                /// \brief this function is single point of contact for this class.
                ///        it takes path of IGES file and returns an OpenSceneGraph Geode
                ///        which directly can be used anywhere. It calculates normals using osgUtil::smoother
                osg::ref_ptr<osg::Geode> igesToOSGGeode(const std::string& filePath);

            private:

                /// \brief heals a opencascade shape
                /// \detail http://www.opencascade.org/org/forum/thread_12716/?forum=3
                ///         Usually IGES files suffer from precision problems (when transferring from
                ///         one CAD system to another).It might be the case that faces are not sewed
                ///         properly,  or do not have the right precision, and so the tesselator does
                ///         not treat them like "sewed". this needs to be done for sewing
                /// \param[in,out] shape opencascade shape to be healed
                void _healShape(TopoDS_Shape& shape);

                /// \brief recursively traverse opencascade assembly structure and build a osg geode
                ///        this function also finds color for leaf node shapes and calculates transformation from parent
                ///        to leaf
                /// \param[in] shapeTree its a OCT(OpenCascade Technology) XDE document label which might contain children or referred shapes
                /// \param[in] transformation contains transformation matrix to be applied
                void _traverse(const TDF_Label &shapeTree, gp_Trsf& transformation);


                /// \brief takes and OpenCascadeShape and returns OSG geometry(drawable), which further can be added to a geode
                /// \detail it iterates shape and breaks it into faces, builds vertex list, color list and creates geometry
                ///         transformation is applied to each vertex before storing it into vertex list
                ///         all vertices are assigned same color
                /// \param[in] shape shape to be converted in geometry. Not a const because it needs to be modified if healing
                ///        is enabled
                /// \param[in] color color of geometry
                /// \param[in] transformation matrix with which vertex position has to be transformed
                osg::ref_ptr<osg::Geometry> _createGeometryFromShape(TopoDS_Shape& shape, const osg::Vec3& color, gp_Trsf& transformation);

            private:

                /// \bried XDE document color tool it stores all colors in color table
                ///        and used to get color from a label
                Handle(XCAFDoc_ColorTool) _colorTool;

                /// \brief geode to contain full model
                osg::ref_ptr<osg::Geode> _modelGeode;

                /// \brief shape tool instance to deal with shapes(simple shapes), referredShape, children etc
                Handle (XCAFDoc_ShapeTool) _assembly;
        };
};

#endif // _READERWRITERIGES_H_
