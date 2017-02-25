/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2014 Robert Osfield
 *  Copyright (C) 2014 Pawel Ksiezopolski
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
 *
*/

#ifndef AGREGATE_GEOMETRY_VISITOR_H
#define AGREGATE_GEOMETRY_VISITOR_H 1
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Transform>
#include <osg/NodeVisitor>
#include <osg/TriangleIndexFunctor>

// AggregateGeometryVisitor uses ConvertTrianglesOperator to create and fill osg::Geometry
// with data matching users needs
struct ConvertTrianglesOperator : public osg::Referenced
{
    ConvertTrianglesOperator()
        : osg::Referenced()
    {
    }
    virtual void initGeometry( osg::Geometry* outputGeometry ) = 0;
    virtual bool pushNode( osg::Node* /*node*/ )
    {
        return false;
    }
    virtual void popNode() = 0;
    virtual void setGeometryData( const osg::Matrix &matrix, osg::Geometry *inputGeometry, osg::Geometry* outputGeometry, float typeID, float lodNumber ) = 0;
    virtual void operator() ( unsigned int i1, unsigned int i2, unsigned int i3 ) = 0;
};

class GetVec2FromArrayVisitor : public osg::ValueVisitor
{
public:
    GetVec2FromArrayVisitor()
    {
    }
    void apply( GLfloat& value )
    {
        out = osg::Vec2( value, 0.0 );
    }
    void apply( osg::Vec2& value )
    {
        out = osg::Vec2( value.x(), value.y() );
    }
    virtual void apply( osg::Vec2d& value )
    {
        out = osg::Vec2( value.x(), value.y() );
    }
    void apply( osg::Vec3& value )
    {
        out = osg::Vec2( value.x(), value.y() );
    }
    void apply( osg::Vec4& value )
    {
        out = osg::Vec2( value.x(), value.y() );
    }
    void apply( osg::Vec3d& value )
    {
        out = osg::Vec2( value.x(), value.y() );
    }
    void apply( osg::Vec4d& value )
    {
        out = osg::Vec2( value.x(), value.y() );
    }

    osg::Vec2 out;
};


// ConvertTrianglesOperatorClassic is a ConvertTrianglesOperator that creates
// aggregated geometry with standard set of vertex attributes : vertices, normals, color and texCoord0.
// texCoord1 holds additional information about vertex ( typeID, lodNumber, boneIndex )
struct ConvertTrianglesOperatorClassic : public ConvertTrianglesOperator
{
    ConvertTrianglesOperatorClassic()
        : ConvertTrianglesOperator(), _typeID(0.0f), _lodNumber(0.0f)

    {
        _boneIndices.push_back(0.0);
    }
    virtual void initGeometry( osg::Geometry* outputGeometry )
    {
        osg::Vec3Array* vertices    = new osg::Vec3Array; outputGeometry->setVertexArray( vertices );
        osg::Vec4Array* colors      = new osg::Vec4Array; outputGeometry->setColorArray( colors, osg::Array::BIND_PER_VERTEX );
        osg::Vec3Array* normals     = new osg::Vec3Array; outputGeometry->setNormalArray( normals, osg::Array::BIND_PER_VERTEX );
        osg::Vec2Array* texCoords0  = new osg::Vec2Array; outputGeometry->setTexCoordArray( 0, texCoords0 );
        osg::Vec3Array* texCoords1  = new osg::Vec3Array; outputGeometry->setTexCoordArray( 1, texCoords1 );
        outputGeometry->setStateSet(NULL);
    }
    virtual bool pushNode( osg::Node* node )
    {
        std::map<std::string,float>::iterator it = _boneNames.find( node->getName() );
        if(it==_boneNames.end())
            return false;
        _boneIndices.push_back( it->second );
        return true;
    }
    virtual void popNode()
    {
        _boneIndices.pop_back();
    }
    virtual void setGeometryData( const osg::Matrix &matrix, osg::Geometry *inputGeometry, osg::Geometry* outputGeometry, float typeID, float lodNumber )
    {
        _matrix = matrix;

        _inputVertices  = dynamic_cast<osg::Vec3Array *>( inputGeometry->getVertexArray() );
        _inputColors    = dynamic_cast<osg::Vec4Array *>( inputGeometry->getColorArray() );
        _inputNormals   = dynamic_cast<osg::Vec3Array *>( inputGeometry->getNormalArray() );
        _inputTexCoord0 = inputGeometry->getTexCoordArray(0);

        _outputVertices = dynamic_cast<osg::Vec3Array *>( outputGeometry->getVertexArray() );
        _outputColors   = dynamic_cast<osg::Vec4Array *>( outputGeometry->getColorArray() );
        _outputNormals  = dynamic_cast<osg::Vec3Array *>( outputGeometry->getNormalArray() );
        _outputTexCoord0 = dynamic_cast<osg::Vec2Array *>( outputGeometry->getTexCoordArray(0) );
        _outputTexCoord1 = dynamic_cast<osg::Vec3Array *>( outputGeometry->getTexCoordArray(1) );

        _typeID = typeID;
        _lodNumber = lodNumber;
    }
    virtual void operator() ( unsigned int i1, unsigned int i2, unsigned int i3 )
    {
        unsigned int ic1=i1, ic2=i2, ic3=i3, in1=i1, in2=i2, in3=i3, it01=i1, it02=i2, it03=i3;
        if ( _inputColors!=NULL && _inputColors->size() == 1 )
        {
            ic1=0; ic2=0; ic3=0;
        }
        if ( _inputNormals!=NULL && _inputNormals->size() == 1 )
        {
            in1=0; in2=0; in3=0;
        }
        if ( _inputTexCoord0!=NULL && _inputTexCoord0->getNumElements()==1 )
        {
            it01=0; it02=0; it03=0;
        }

        _outputVertices->push_back( _inputVertices->at( i1 ) * _matrix );
        _outputVertices->push_back( _inputVertices->at( i2 ) * _matrix  );
        _outputVertices->push_back( _inputVertices->at( i3 ) * _matrix  );

        if( _inputColors != NULL )
        {
            _outputColors->push_back( _inputColors->at( ic1 ) );
            _outputColors->push_back( _inputColors->at( ic2 ) );
            _outputColors->push_back( _inputColors->at( ic3 ) );
        }
        else
        {
            for(unsigned int i=0; i<3; ++i)
                _outputColors->push_back( osg::Vec4(1.0,1.0,1.0,1.0) );
        }

        if( _inputNormals != NULL )
        {
            _outputNormals->push_back( osg::Matrix::transform3x3( _inputNormals->at( in1 ), _matrix ) );
            _outputNormals->push_back( osg::Matrix::transform3x3( _inputNormals->at( in2 ), _matrix ) );
            _outputNormals->push_back( osg::Matrix::transform3x3( _inputNormals->at( in3 ), _matrix ) );
        }
        else
        {
            for(unsigned int i=0; i<3; ++i)
                _outputNormals->push_back( osg::Vec3( 0.0,0.0,1.0 ) );
        }
        if( _inputTexCoord0 != NULL )
        {
            _inputTexCoord0->accept( it01, _inputTexCoord0Visitor );
            _outputTexCoord0->push_back( _inputTexCoord0Visitor.out );
            _inputTexCoord0->accept( it02, _inputTexCoord0Visitor );
            _outputTexCoord0->push_back( _inputTexCoord0Visitor.out );
            _inputTexCoord0->accept( it03, _inputTexCoord0Visitor );
            _outputTexCoord0->push_back( _inputTexCoord0Visitor.out );
        }
        else
        {
            for(unsigned int i=0; i<3; ++i)
                _outputTexCoord0->push_back( osg::Vec2(0.0,0.0) );
        }

        for(unsigned int i=0; i<3; ++i)
            _outputTexCoord1->push_back( osg::Vec3( _typeID, _lodNumber, _boneIndices.back() ) );
    }
    void registerBoneByName( const std::string& boneName, int boneIndex )
    {
        _boneNames[boneName] = float(boneIndex);
    }

    osg::Matrix _matrix;

    osg::Vec3Array* _inputVertices;
    osg::Vec4Array* _inputColors;
    osg::Vec3Array* _inputNormals;
    osg::Array*     _inputTexCoord0;

    osg::Vec3Array* _outputVertices;
    osg::Vec4Array* _outputColors;
    osg::Vec3Array* _outputNormals;
    osg::Vec2Array* _outputTexCoord0;
    osg::Vec3Array* _outputTexCoord1;

    float _typeID;
    float _lodNumber;
    std::vector<float> _boneIndices;

    std::map<std::string,float> _boneNames;

    GetVec2FromArrayVisitor _inputTexCoord0Visitor;
};



class AggregateGeometryVisitor : public osg::NodeVisitor
{
public:
    AggregateGeometryVisitor( ConvertTrianglesOperator* ctOperator );
    void init();

    // osg::TriangleIndexFunctor uses its template parameter as a base class, so we must use an adapter pattern to hack it
    struct ConvertTrianglesBridge
    {
        inline void setConverter( ConvertTrianglesOperator* cto )
        {
            _converter = cto;
        }
        inline void initGeometry( osg::Geometry* outputGeometry )
        {
            _converter->initGeometry(outputGeometry);
        }
        inline bool pushNode( osg::Node* node )
        {
            return _converter->pushNode( node );
        }
        inline void popNode()
        {
            _converter->popNode();
        }
        inline void setGeometryData( const osg::Matrix &matrix, osg::Geometry *inputGeometry, osg::Geometry* outputGeometry, float typeID, float lodNumber )
        {
            _converter->setGeometryData(matrix,inputGeometry,outputGeometry,typeID, lodNumber);
        }
        inline void operator() ( unsigned int i1, unsigned int i2, unsigned int i3 )
        {
            _converter->operator()(i1,i2,i3);
        }

        osg::ref_ptr<ConvertTrianglesOperator> _converter;
    };


    // struct returning information about added object ( first vertex, vertex count, primitiveset index )
    // used later to create indirect command texture buffers
    struct AddObjectResult
    {
        AddObjectResult( unsigned int f, unsigned int c, unsigned int i )
            : first(f), count(c), index(i)
        {
        }
        unsigned int first;
        unsigned int count;
        unsigned int index;
    };
    AddObjectResult addObject( osg::Node* object, unsigned int typeID, unsigned int lodNumber );

    void apply( osg::Node& node );
    void apply( osg::Transform& transform );
    void apply( osg::Geode& geode );

    inline osg::Geometry* getAggregatedGeometry()
    {
        return _aggregatedGeometry.get();
    }
protected:
    osg::ref_ptr<osg::Geometry> _aggregatedGeometry;
    osg::TriangleIndexFunctor<ConvertTrianglesBridge> _ctOperator;
    std::vector<osg::Matrix>    _matrixStack;

    unsigned int _currentTypeID;
    unsigned int _currentLodNumber;
};

#endif
