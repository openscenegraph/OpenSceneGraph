#include <osg/Shape>
#include <osg/Notify>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/ParameterOutput>

using namespace osg;
using namespace osgDB;


//////////////////////////////////////////////////////////////////////////////

// forward declare functions to use later.
bool Sphere_readLocalData(Object& obj, Input& fr);
bool Sphere_writeLocalData(const Object& obj, Output& fw);

//register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_SphereFuncProxy
(
    new osg::Sphere,
    "Sphere",
    "Object Sphere",
    &Sphere_readLocalData,
    &Sphere_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool Sphere_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Sphere& sphere = static_cast<Sphere&>(obj);

    if (fr.matchSequence("Center %f %f %f"))
    {
        osg::Vec3 center;
        fr[1].getFloat(center.x());
        fr[2].getFloat(center.y());
        fr[3].getFloat(center.z());
        sphere.setCenter(center);
        fr+=4;
        iteratorAdvanced = true;
    }
    
    if (fr.matchSequence("Radius %f"))
    {
        float radius;
        fr[1].getFloat(radius);
        sphere.setRadius(radius);
        fr+=2;
        iteratorAdvanced = true;
    }
    

    return iteratorAdvanced;
}

bool Sphere_writeLocalData(const Object& obj, Output& fw)
{
    const Sphere& sphere = static_cast<const Sphere&>(obj);
    
    fw.indent()<<"Center "<<sphere.getCenter()<<std::endl;
    fw.indent()<<"Radius "<<sphere.getRadius()<<std::endl;

    return true;
}


//////////////////////////////////////////////////////////////////////////////
// forward declare functions to use later.
bool Box_readLocalData(Object& obj, Input& fr);
bool Box_writeLocalData(const Object& obj, Output& fw);

//register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_BoxFuncProxy
(
    new osg::Box,
    "Box",
    "Object Box",
    &Box_readLocalData,
    &Box_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool Box_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Box& box = static_cast<Box&>(obj);

    if (fr.matchSequence("Center %f %f %f"))
    {
        osg::Vec3 center;
        fr[1].getFloat(center.x());
        fr[2].getFloat(center.y());
        fr[3].getFloat(center.z());
        box.setCenter(center);
        fr+=4;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("HalfLengths %f %f %f"))
    {
        osg::Vec3 lenghts;
        fr[1].getFloat(lenghts.x());
        fr[2].getFloat(lenghts.y());
        fr[3].getFloat(lenghts.z());
        box.setHalfLengths(lenghts);
        fr+=4;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("Rotation %f %f %f %f"))
    {
        osg::Quat rotation;
        fr[1].getFloat(rotation.x());
        fr[2].getFloat(rotation.y());
        fr[3].getFloat(rotation.z());
        fr[4].getFloat(rotation.w());
        box.setRotation(rotation);
        fr+=5;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool Box_writeLocalData(const Object& obj, Output& fw)
{
    const Box& box = static_cast<const Box&>(obj);

    fw.indent()<<"Center "<<box.getCenter()<<std::endl;
    fw.indent()<<"HalfLengths "<<box.getHalfLengths()<<std::endl;
    fw.indent()<<"Rotation "<<box.getRotation()<<std::endl;

    return true;
}


//////////////////////////////////////////////////////////////////////////////
// forward declare functions to use later.
bool Cone_readLocalData(Object& obj, Input& fr);
bool Cone_writeLocalData(const Object& obj, Output& fw);

//register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_ConeFuncProxy
(
    new osg::Cone,
    "Cone",
    "Object Cone",
    &Cone_readLocalData,
    &Cone_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool Cone_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Cone& cone = static_cast<Cone&>(obj);

    if (fr.matchSequence("Center %f %f %f"))
    {
        osg::Vec3 center;
        fr[1].getFloat(center.x());
        fr[2].getFloat(center.y());
        fr[3].getFloat(center.z());
        cone.setCenter(center);
        fr+=4;
        iteratorAdvanced = true;
    }
    
    if (fr.matchSequence("Radius %f"))
    {
        float radius;
        fr[1].getFloat(radius);
        cone.setRadius(radius);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("Height %f"))
    {
        float height;
        fr[1].getFloat(height);
        cone.setHeight(height);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("Rotation %f %f %f %f"))
    {
        osg::Quat rotation;
        fr[1].getFloat(rotation.x());
        fr[2].getFloat(rotation.y());
        fr[3].getFloat(rotation.z());
        fr[4].getFloat(rotation.w());
        cone.setRotation(rotation);
        fr+=5;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool Cone_writeLocalData(const Object& obj, Output& fw)
{
    const Cone& cone = static_cast<const Cone&>(obj);

    fw.indent()<<"Center "<<cone.getCenter()<<std::endl;
    fw.indent()<<"Radius "<<cone.getRadius()<<std::endl;
    fw.indent()<<"Height "<<cone.getHeight()<<std::endl;
    fw.indent()<<"Rotation "<<cone.getRotation()<<std::endl;

    return true;
}



//////////////////////////////////////////////////////////////////////////////
// forward declare functions to use later.
bool Cylinder_readLocalData(Object& obj, Input& fr);
bool Cylinder_writeLocalData(const Object& obj, Output& fw);

//register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_CylinderFuncProxy
(
    new osg::Cylinder,
    "Cylinder",
    "Object Cylinder",
    &Cylinder_readLocalData,
    &Cylinder_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool Cylinder_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Cylinder& cylinder = static_cast<Cylinder&>(obj);

    if (fr.matchSequence("Center %f %f %f"))
    {
        osg::Vec3 center;
        fr[1].getFloat(center.x());
        fr[2].getFloat(center.y());
        fr[3].getFloat(center.z());
        cylinder.setCenter(center);
        fr+=4;
        iteratorAdvanced = true;
    }
    
    if (fr.matchSequence("Radius %f"))
    {
        float radius;
        fr[1].getFloat(radius);
        cylinder.setRadius(radius);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("Height %f"))
    {
        float height;
        fr[1].getFloat(height);
        cylinder.setHeight(height);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("Rotation %f %f %f %f"))
    {
        osg::Quat rotation;
        fr[1].getFloat(rotation.x());
        fr[2].getFloat(rotation.y());
        fr[3].getFloat(rotation.z());
        fr[4].getFloat(rotation.w());
        cylinder.setRotation(rotation);
        fr+=5;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool Cylinder_writeLocalData(const Object& obj, Output& fw)
{
    const Cylinder& cylinder = static_cast<const Cylinder&>(obj);

    fw.indent()<<"Center "<<cylinder.getCenter()<<std::endl;
    fw.indent()<<"Radius "<<cylinder.getRadius()<<std::endl;
    fw.indent()<<"Height "<<cylinder.getHeight()<<std::endl;
    fw.indent()<<"Rotation "<<cylinder.getRotation()<<std::endl;

    return true;
}


//////////////////////////////////////////////////////////////////////////////
// forward declare functions to use later.
bool Capsule_readLocalData(Object& obj, Input& fr);
bool Capsule_writeLocalData(const Object& obj, Output& fw);

//register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_CapsuleFuncProxy
(
    new osg::Capsule,
    "Capsule",
    "Object Capsule",
    &Capsule_readLocalData,
    &Capsule_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool Capsule_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Capsule& capsule = static_cast<Capsule&>(obj);

    if (fr.matchSequence("Center %f %f %f"))
    {
        osg::Vec3 center;
        fr[1].getFloat(center.x());
        fr[2].getFloat(center.y());
        fr[3].getFloat(center.z());
        capsule.setCenter(center);
        fr+=4;
        iteratorAdvanced = true;
    }
    
    if (fr.matchSequence("Radius %f"))
    {
        float radius;
        fr[1].getFloat(radius);
        capsule.setRadius(radius);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("Height %f"))
    {
        float height;
        fr[1].getFloat(height);
        capsule.setHeight(height);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("Rotation %f %f %f %f"))
    {
        osg::Quat rotation;
        fr[1].getFloat(rotation.x());
        fr[2].getFloat(rotation.y());
        fr[3].getFloat(rotation.z());
        fr[4].getFloat(rotation.w());
        capsule.setRotation(rotation);
        fr+=5;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}

bool Capsule_writeLocalData(const Object& obj, Output& fw)
{
    const Capsule& capsule = static_cast<const Capsule&>(obj);

    fw.indent()<<"Center "<<capsule.getCenter()<<std::endl;
    fw.indent()<<"Radius "<<capsule.getRadius()<<std::endl;
    fw.indent()<<"Height "<<capsule.getHeight()<<std::endl;
    fw.indent()<<"Rotation "<<capsule.getRotation()<<std::endl;

    return true;
}


//////////////////////////////////////////////////////////////////////////////
// forward declare functions to use later.
bool HeightField_readLocalData(Object& obj, Input& fr);
bool HeightField_writeLocalData(const Object& obj, Output& fw);

//register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_HeightFieldFuncProxy
(
    new osg::HeightField,
    "HeightField",
    "Object HeightField",
    &HeightField_readLocalData,
    &HeightField_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

//register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_GridFuncProxy
(
    new osg::HeightField,
    "Grid",
    "Object HeightField",
    0,
    0,
    DotOsgWrapper::READ_AND_WRITE
);

bool HeightField_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    HeightField& heightfield = static_cast<HeightField&>(obj);

    if (fr.matchSequence("Origin %f %f %f"))
    {
        osg::Vec3 origin;
        fr[1].getFloat(origin.x());
        fr[2].getFloat(origin.y());
        fr[3].getFloat(origin.z());
        heightfield.setOrigin(origin);
        fr+=4;
    }
    
    if (fr.matchSequence("XInterval %f"))
    {
        float interval;
        fr[1].getFloat(interval);
        heightfield.setXInterval(interval);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("YInterval %f"))
    {
        float interval;
        fr[1].getFloat(interval);
        heightfield.setYInterval(interval);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("SkirtHeight %f"))
    {
        float height;
        fr[1].getFloat(height);
        heightfield.setSkirtHeight(height);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("BorderWidth %i"))
    {
        unsigned int width;
        fr[1].getUInt(width);
        heightfield.setBorderWidth(width);
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("Rotation %f %f %f %f"))
    {
        osg::Quat rotation;
        fr[1].getFloat(rotation.x());
        fr[2].getFloat(rotation.y());
        fr[3].getFloat(rotation.z());
        fr[4].getFloat(rotation.w());
        heightfield.setRotation(rotation);
        fr+=5;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("NumColumnsAndRows %i %i"))
    {
        int numcolumns,numrows;
        fr[1].getInt(numcolumns);
        fr[2].getInt(numrows);
        heightfield.allocate(numcolumns,numrows);
        fr+=3;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("Heights {"))
    {

        int entry = fr[0].getNoNestedBrackets();

        fr += 2;

        float height;
        unsigned int row = 0;
        unsigned int column = 0;

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            if (fr.readSequence(height))
            {
                heightfield.setHeight(column,row,height);
                ++column;
                if (column>=heightfield.getNumColumns())
                {
                    column = 0;
                    ++row;
                }
            }
            else
            {
                ++fr;
            }
        }

        iteratorAdvanced = true;
        ++fr;

    }

    return iteratorAdvanced;
}

bool HeightField_writeLocalData(const Object& obj, Output& fw)
{
    const HeightField& heightfield = static_cast<const HeightField&>(obj);

    int prec = fw.precision();
    fw.precision(15);
    fw.indent()<<"Origin "<<heightfield.getOrigin().x()<<" "<<heightfield.getOrigin().y()<<" "<<heightfield.getOrigin().z()<<std::endl;
    fw.indent()<<"XInterval "<<heightfield.getXInterval()<<std::endl;
    fw.indent()<<"YInterval "<<heightfield.getYInterval()<<std::endl;
    fw.indent()<<"SkirtHeight "<<heightfield.getSkirtHeight()<<std::endl;
    fw.indent()<<"BorderWidth "<<heightfield.getBorderWidth()<<std::endl;
    fw.indent()<<"Rotation "<<heightfield.getRotation()<<std::endl;
    fw.precision(prec);

    fw.indent()<<"NumColumnsAndRows "<<heightfield.getNumColumns()<<" "<<heightfield.getNumRows()<<std::endl;

    fw.indent()<<"Heights"<<std::endl;
    
    ParameterOutput po(fw);
    po.begin();
    for(unsigned int row=0;row<heightfield.getNumRows();++row)
    {
        for(unsigned int column=0;column<heightfield.getNumColumns();++column)
        {
            po.write(heightfield.getHeight(column,row));         
        }
        po.newLine();
    }
    po.end();

    return true;
}

//////////////////////////////////////////////////////////////////////////////
// forward declare functions to use later.
bool CompositeShape_readLocalData(Object& obj, Input& fr);
bool CompositeShape_writeLocalData(const Object& obj, Output& fw);

//register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_CompositeShapeFuncProxy
(
    new osg::CompositeShape,
    "CompositeShape",
    "Object CompositeShape",
    &CompositeShape_readLocalData,
    &CompositeShape_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool CompositeShape_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    CompositeShape& composite = static_cast<CompositeShape&>(obj);

    ref_ptr<Object> readObject;
    if (fr[0].matchWord("Shape"))
    {
        readObject = fr.readObject();
        if (readObject.valid())
        {
            osg::Shape* shape = dynamic_cast<osg::Shape*>(readObject.get());
            if (shape) composite.setShape(shape);
            else notify(WARN)<<"Warning:: "<<readObject->className()<<" loaded but cannot not be attached to Drawable."<<std::endl;
            iteratorAdvanced = true;
        }
    }
    
    while((readObject=fr.readObjectOfType(type_wrapper<osg::Shape>())).valid())
    {
        osg::Shape* shape = static_cast<osg::Shape*>(readObject.get());
        composite.addChild(shape);
        iteratorAdvanced = true;
    }
    
    return iteratorAdvanced;
}

bool CompositeShape_writeLocalData(const Object& obj, Output& fw)
{
    const CompositeShape& composite = static_cast<const CompositeShape&>(obj);

    if (composite.getShape())
    {
        fw.indent() << "Shape ";
        fw.writeObject(*composite.getShape());
    }
    
    for(unsigned int i=0;i<composite.getNumChildren();++i)
    {
        fw.writeObject(*composite.getChild(i));
    }

    return true;
}


// 
// 
// //////////////////////////////////////////////////////////////////////////////
// // forward declare functions to use later.
// bool InfinitePlane_readLocalData(Object& obj, Input& fr);
// bool InfinitePlane_writeLocalData(const Object& obj, Output& fw);
// 
// //register the read and write functions with the osgDB::Registry.
// RegisterDotOsgWrapperProxy g_InfinitePlaneFuncProxy
// (
//     new osg::InfinitePlane,
//     "InfinitePlane",
//     "Object InfinitePlane",
//     &InfinitePlane_readLocalData,
//     &InfinitePlane_writeLocalData,
//     DotOsgWrapper::READ_AND_WRITE
// );
// 
// bool InfinitePlane_readLocalData(Object& obj, Input& fr)
// {
//     bool iteratorAdvanced = false;
// 
//     //InfinitePlane& infplane = static_cast<InfinitePlane&>(obj);
// 
//     return iteratorAdvanced;
// }
// 
// bool InfinitePlane_writeLocalData(const Object& obj, Output& fw)
// {
//     //const InfinitePlane& infplane = static_cast<const InfinitePlane&>(obj);
// 
//     return true;
// }
// 
// 
// //////////////////////////////////////////////////////////////////////////////
// 
// // forward declare functions to use later.
// bool TriangleMesh_readLocalData(Object& obj, Input& fr);
// bool TriangleMesh_writeLocalData(const Object& obj, Output& fw);
// 
// //register the read and write functions with the osgDB::Registry.
// RegisterDotOsgWrapperProxy g_TriangleMeshFuncProxy
// (
//     new osg::TriangleMesh,
//     "TriangleMesh",
//     "Object ",
//     &TriangleMesh_readLocalData,
//     &TriangleMesh_writeLocalData,
//     DotOsgWrapper::READ_AND_WRITE
// );
// 
// bool TriangleMesh_readLocalData(Object& obj, Input& fr)
// {
//     bool iteratorAdvanced = false;
// 
// //    TriangleMesh& mesh = static_cast<TriangleMesh&>(obj);
// 
//     return iteratorAdvanced;
// }
// 
// bool TriangleMesh_writeLocalData(const Object& obj, Output& fw)
// {
// //    const TriangleMesh& mesh = static_cast<const TriangleMesh&>(obj);
// 
//     return true;
// }
// 
// 
// //////////////////////////////////////////////////////////////////////////////
// // forward declare functions to use later.
// bool ConvexHull_readLocalData(Object& obj, Input& fr);
// bool ConvexHull_writeLocalData(const Object& obj, Output& fw);
// 
// //register the read and write functions with the osgDB::Registry.
// RegisterDotOsgWrapperProxy g_ConvexHullFuncProxy
// (
//     new osg::ConvexHull,
//     "ConvexHull",
//     "Object ",
//     &ConvexHull_readLocalData,
//     &ConvexHull_writeLocalData,
//     DotOsgWrapper::READ_AND_WRITE
// );
// 
// bool ConvexHull_readLocalData(Object& obj, Input& fr)
// {
//     bool iteratorAdvanced = false;
// 
// //    ConvexHull& geom = static_cast<ConvexHull&>(obj);
// 
//     return iteratorAdvanced;
// }
// 
// bool ConvexHull_writeLocalData(const Object& obj, Output& fw)
// {
// //    const ConvexHull& geom = static_cast<const ConvexHull&>(obj);
// 
//     return true;
// }
// 
// 

