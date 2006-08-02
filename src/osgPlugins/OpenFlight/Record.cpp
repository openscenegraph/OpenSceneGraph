//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#include <stdexcept>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include "Record.h"
#include "Document.h"

using namespace flt;


Record::Record()
{
}

Record::~Record()
{
}

void Record::setParent(PrimaryRecord* parent)
{
    _parent = parent;
}

//PrimaryRecord& Record::parent()
//{
//    if (!_parent)
//        throw std::runtime_error("Record::parent(): invalid pointer to parent exception.");
//
//    return *_parent;
//}


void Record::read(RecordInputStream& in, Document& document)
{
    setParent(document.getCurrentPrimaryRecord());

    // Read record body.
    readRecord(in,document);
}


void Record::readRecord(RecordInputStream& /*in*/, Document& /*document*/)
{
}


PrimaryRecord::PrimaryRecord() :
    _numberOfReplications(0)
{
}


void PrimaryRecord::read(RecordInputStream& in, Document& document)
{
    setParent(document.getTopOfLevelStack());

    // Update primary record.
    document.setCurrentPrimaryRecord(this);

    // Read record body.
    readRecord(in,document);
}

///////////////////////////////////////////////////////////////////////////////////
// Helper methods

// Insert matrix-tranform above node.
// Return transform.
osg::ref_ptr<osg::MatrixTransform> flt::insertMatrixTransform(osg::Node& node, const osg::Matrix& matrix)
{
    osg::ref_ptr<osg::Node> ref = &node;
    osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform(matrix);
    transform->setDataVariance(osg::Object::STATIC);

    // Replace parent
    osg::Node::ParentList parents = node.getParents();
    for (osg::Node::ParentList::iterator itr=parents.begin();
        itr!=parents.end();
        ++itr)
    {
        (*itr)->replaceChild(&node,transform.get());
    }

    // Make primary a child of matrix transform.
    transform->addChild(&node);

    return transform;
}


///////////////////////////////////////////////////////////////////////////////////


osg::Vec3Array* flt::getOrCreateVertexArray(osg::Geometry& geometry)
{
    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry.getVertexArray());
    if (!vertices)
    {
        vertices = new osg::Vec3Array;
        geometry.setVertexArray(vertices);
    }
    return vertices;
}

osg::Vec3Array* flt::getOrCreateNormalArray(osg::Geometry& geometry)
{
    osg::Vec3Array* normals = dynamic_cast<osg::Vec3Array*>(geometry.getNormalArray());
    if (!normals)
    {
        normals = new osg::Vec3Array;
        geometry.setNormalArray(normals);
    }
    return normals;
}

osg::Vec4Array* flt::getOrCreateColorArray(osg::Geometry& geometry)
{
    osg::Vec4Array* colors = dynamic_cast<osg::Vec4Array*>(geometry.getColorArray());
    if (!colors)
    {
        colors = new osg::Vec4Array;
        geometry.setColorArray(colors);
    }
    return colors;
}


osg::Vec2Array* flt::getOrCreateTextureArray(osg::Geometry& geometry, int unit)
{
    osg::Vec2Array* UVs = dynamic_cast<osg::Vec2Array*>(geometry.getTexCoordArray(unit));
    if (!UVs)
    {
        UVs = new osg::Vec2Array;
        geometry.setTexCoordArray(unit,UVs);
    }
    return UVs;
}
