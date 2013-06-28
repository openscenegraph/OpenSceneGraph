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
#include <osgUtil/SceneGraphBuilder>

#include <osg/Notify>
#include <osg/io_utils>

#include <osg/PrimitiveSet>
#include <osg/Texture>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/LineStipple>
#include <osg/Depth>
#include <osg/CullFace>
#include <osg/FrontFace>
#include <osg/LineWidth>
#include <osg/Point>
#include <osg/PolygonMode>
#include <osg/PolygonOffset>
#include <osg/PolygonStipple>
#include <osg/ShadeModel>
#include <osg/ShapeDrawable>

using namespace osgUtil;

SceneGraphBuilder::SceneGraphBuilder():
    _statesetAssigned(false),
    _normalSet(false),
    _normal(0.0f,0.0f,1.0f),
    _colorSet(false),
    _color(1.0f,1.0f,1.0f,1.0f),
    _maxNumTexCoordComponents(0),
    _texCoord(0.f,0.0f,0.0f,1.0f),
    _primitiveMode(0)
{
}


///////////////////////////////////////////////////////////////////////////////////////////
//
// OpenGL 1.0 building methods
//
void SceneGraphBuilder::PushMatrix()
{
    if (_matrixStack.empty()) _matrixStack.push_back(osg::Matrixd());
    else _matrixStack.push_back(_matrixStack.back());
}

void SceneGraphBuilder::PopMatrix()
{
    if (!_matrixStack.empty()) _matrixStack.pop_back();

    matrixChanged();
}

void SceneGraphBuilder::LoadIdentity()
{
    if (_matrixStack.empty()) _matrixStack.push_back(osg::Matrixd());
    _matrixStack.back().makeIdentity();

    matrixChanged();
}

void SceneGraphBuilder::LoadMatrixd(const GLdouble* m)
{
    if (_matrixStack.empty()) _matrixStack.push_back(osg::Matrixd());
    _matrixStack.back().set(m);

    matrixChanged();
}

void SceneGraphBuilder::MultMatrixd(const GLdouble* m)
{
    if (_matrixStack.empty()) _matrixStack.push_back(osg::Matrixd());
    _matrixStack.back().preMult(osg::Matrixd(m));

    matrixChanged();
}

void SceneGraphBuilder::Translated(GLdouble x, GLdouble y, GLdouble z)
{
    if (_matrixStack.empty()) _matrixStack.push_back(osg::Matrixd());
    _matrixStack.back().preMultTranslate(osg::Vec3d(x,y,z));

    matrixChanged();
}

void SceneGraphBuilder::Scaled(GLdouble x, GLdouble y, GLdouble z)
{
    if (_matrixStack.empty()) _matrixStack.push_back(osg::Matrixd());
    _matrixStack.back().preMultScale(osg::Vec3d(x,y,z));

    matrixChanged();
}

void SceneGraphBuilder::Rotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    if (_matrixStack.empty()) _matrixStack.push_back(osg::Matrixd());
    _matrixStack.back().preMultRotate(osg::Quat(osg::inDegrees(angle),osg::Vec3d(x,y,z)));

    matrixChanged();
}

void SceneGraphBuilder::BlendFunc(GLenum srcFactor, GLenum dstFactor)
{
    addAttribute(new osg::BlendFunc(srcFactor, dstFactor));
}

void SceneGraphBuilder::CullFace(GLenum mode)
{
    addAttribute(new osg::CullFace(osg::CullFace::Mode(mode)));
}

void SceneGraphBuilder::DepthFunc(GLenum mode)
{
    addAttribute(new osg::Depth(osg::Depth::Function(mode)));
}

void SceneGraphBuilder::FrontFace(GLenum mode)
{
    addAttribute(new osg::FrontFace(osg::FrontFace::Mode(mode)));
}

void SceneGraphBuilder::LineStipple(GLint factor, GLushort pattern)
{
    addAttribute(new osg::LineStipple(factor, pattern));
}

void SceneGraphBuilder::LineWidth(GLfloat lineWidth)
{
    addAttribute(new osg::LineWidth(lineWidth));
}

void SceneGraphBuilder::PointSize(GLfloat pointSize)
{
    addAttribute(new osg::Point(pointSize));
}

void SceneGraphBuilder::PolygonMode(GLenum face, GLenum mode)
{
    addAttribute(new osg::PolygonMode(osg::PolygonMode::Face(face),osg::PolygonMode::Mode(mode)));
}

void SceneGraphBuilder::PolygonOffset(GLfloat factor, GLfloat units)
{
    addAttribute(new osg::PolygonOffset(factor,units));
}

void SceneGraphBuilder::PolygonStipple(const GLubyte* mask)
{
    addAttribute(new osg::PolygonStipple(mask));
}

void SceneGraphBuilder::ShadeModel(GLenum mode)
{
    addAttribute(new osg::ShadeModel(osg::ShadeModel::Mode(mode)));
}
void SceneGraphBuilder::Enable(GLenum mode)
{
    addMode(mode, true);
}

void SceneGraphBuilder::Disable(GLenum mode)
{
    addMode(mode, false);
}

void SceneGraphBuilder::Color4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    _normalSet = true;
    _color.set(red,green,blue,alpha);
}

void SceneGraphBuilder::Normal3f(GLfloat x, GLfloat y, GLfloat z)
{
    _normalSet = true;
    _normal.set(x,y,z);
}

void SceneGraphBuilder::TexCoord1f(GLfloat x)
{
    _maxNumTexCoordComponents = 1;
    _texCoord.set(x,0.0f,0.0f,1.0f);
}

void SceneGraphBuilder::TexCoord2f(GLfloat x, GLfloat y)
{
    _maxNumTexCoordComponents = 2;
    _texCoord.set(x,y,0.0f,1.0f);
}

void SceneGraphBuilder::TexCoord3f(GLfloat x, GLfloat y, GLfloat z)
{
    _maxNumTexCoordComponents = 3;
    _texCoord.set(x,y,z,1.0);
}

void SceneGraphBuilder::TexCoord4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    _maxNumTexCoordComponents = 4;
    _texCoord.set(x,y,z,w);
}

void SceneGraphBuilder::Vertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    osg::Vec3 vertex(x,y,z);

    vertex = vertex * _matrixStack.back();

    if (_vertices.valid()) _vertices->push_back(vertex);
    if (_normal.valid()) _normals->push_back(_normal);
    if (_colors.valid()) _colors->push_back(_color);
    if (_texCoords.valid()) _texCoords->push_back(_texCoord);
}

void SceneGraphBuilder::Begin(GLenum mode)
{
    // reset geometry
    _primitiveMode = mode;
    _vertices = new osg::Vec3Array;

    _normalSet = false;
    _normals = new osg::Vec3Array;

    _colorSet = false;
    _colors = new osg::Vec4Array;

    _maxNumTexCoordComponents = 0;
    _texCoords = new osg::Vec4Array;

}

void SceneGraphBuilder::End()
{
    allocateGeometry();

    _geometry->setVertexArray(_vertices.get());

    if (_colorSet)
    {
         _geometry->setColorArray(_colors.get(), osg::Array::BIND_PER_VERTEX);
    }
    else
    {
         osg::Vec4Array* colors = new osg::Vec4Array;
         colors->push_back(_color);

         _geometry->setColorArray(colors, osg::Array::BIND_OVERALL);
    }

    if (_normalSet)
    {
         _geometry->setNormalArray(_normals.get(), osg::Array::BIND_PER_VERTEX);
    }
    else
    {
         _geometry->setNormalArray(NULL, osg::Array::BIND_OFF);
    }

    if (_maxNumTexCoordComponents==1)
    {
        // convert Vec4Array into FloatArray
        osg::FloatArray* texCoords = new osg::FloatArray;
        for(osg::Vec4Array::iterator itr = _texCoords->begin();
            itr != _texCoords->end();
            ++itr)
        {
            texCoords->push_back(itr->x());
        }
         _geometry->setTexCoordArray(0, texCoords);
    }
    if (_maxNumTexCoordComponents==2)
    {
        // convert Vec4Array into FloatArray
        osg::Vec2Array* texCoords = new osg::Vec2Array;
        for(osg::Vec4Array::iterator itr = _texCoords->begin();
            itr != _texCoords->end();
            ++itr)
        {
            texCoords->push_back(osg::Vec2(itr->x(),itr->y()));
        }
         _geometry->setTexCoordArray(0, texCoords);
    }
    if (_maxNumTexCoordComponents==3)
    {
        // convert Vec4Array into FloatArray
        osg::Vec3Array* texCoords = new osg::Vec3Array;
        for(osg::Vec4Array::iterator itr = _texCoords->begin();
            itr != _texCoords->end();
            ++itr)
        {
            texCoords->push_back(osg::Vec3(itr->x(),itr->y(), itr->z()));
        }
         _geometry->setTexCoordArray(0, texCoords);
    }
    else if (_maxNumTexCoordComponents==4)
    {
         _geometry->setTexCoordArray(0, _texCoords.get());
    }

    _geometry->addPrimitiveSet(new osg::DrawArrays(_primitiveMode, 0, _vertices->size()));

    completeGeometry();
}

///////////////////////////////////////////////////////////////////////////////////////////
//
//  GLU style building methods
//
void SceneGraphBuilder::QuadricDrawStyle(GLenum aDrawStyle)
{
    _quadricState._drawStyle = aDrawStyle;
}

void SceneGraphBuilder::QuadricNormals(GLenum aNormals)
{
    _quadricState._normals = aNormals;
}

void SceneGraphBuilder::QuadricOrientation(GLenum aOrientation)
{
    _quadricState._orientation = aOrientation;
}

void SceneGraphBuilder::QuadricTexture(GLboolean aTexture)
{
    _quadricState._texture = aTexture;
}

void SceneGraphBuilder::Cylinder(GLfloat        aBase,
                             GLfloat        aTop,
                             GLfloat        aHeight,
                             GLint          aSlices,
                             GLint          aStacks)
{
    OSG_NOTICE<<"SceneGraphBuilder::Cylinder("<<aBase<<", "<<aTop<<", "<<aHeight<<", "<<aSlices<<", "<<aStacks<<") not implemented yet"<<std::endl;
}

void SceneGraphBuilder::Disk(GLfloat        /*inner*/,
                             GLfloat        outer,
                             GLint          slices,
                             GLint          /*loops*/)
{
    double angle = 0.0;
    double delta = 2.0*osg::PI/double(slices-1);

    if (_quadricState._normals!=GLU_NONE) Normal3f(0.0f,0.0f,1.0f);

    switch(_quadricState._drawStyle)
    {
        case(GLU_POINT):
        {
            Begin(GL_POINTS);
            if (_quadricState._texture) TexCoord2f(0.5f,0.5f);
            Vertex3f(0.0f, 0.0f, 0.0f);
            for(GLint i=0; i<slices; ++i, angle += delta)
            {
                if (_quadricState._texture) TexCoord2f(GLfloat(sin(angle)*0.5+0.5), GLfloat(cos(angle)*0.5+0.5));
                Vertex3f(outer*GLfloat(sin(angle)), outer*GLfloat(cos(angle)), 0.0f);
            }
            End();
            break;
        }
        case(GLU_LINE):
        {
            Begin(GL_LINE_LOOP);
            for(GLint i=0; i<slices; ++i, angle += delta)
            {
                if (_quadricState._texture) TexCoord2f(GLfloat(sin(angle)*0.5+0.5), GLfloat(cos(angle)*0.5+0.5));
                Vertex3f(outer*GLfloat(sin(angle)), outer*GLfloat(cos(angle)), 0.0f);
            }
            End();
            break;
        }
        case(GLU_FILL):
        {
            Begin(GL_TRIANGLE_FAN);
            if (_quadricState._texture) TexCoord2f(0.5f,0.5f);
            Vertex3f(0.0f, 0.0f, 0.0f);
            for(GLint i=0; i<slices; ++i, angle += delta)
            {
                if (_quadricState._texture) TexCoord2f(GLfloat(sin(angle)*0.5+0.5), GLfloat(cos(angle)*0.5+0.5));
                Vertex3f(outer*GLfloat(sin(angle)), outer*GLfloat(cos(angle)), 0.0f);
            }
            End();
            break;
        }
        case(GLU_SILHOUETTE):
        {
            Begin(GL_LINE_LOOP);
            for(GLint i=0; i<slices; ++i, angle += delta)
            {
                if (_quadricState._texture) TexCoord2f(GLfloat(sin(angle)*0.5+0.5), GLfloat(cos(angle)*0.5+0.5));
                Vertex3f(outer*GLfloat(sin(angle)), outer*GLfloat(cos(angle)), 0.0f);
            }
            End();
            break;
        }
    }
}

void SceneGraphBuilder::PartialDisk(GLfloat        inner,
                                    GLfloat        outer,
                                    GLint          slices,
                                    GLint          loops,
                                    GLfloat        start,
                                    GLfloat        sweep)
{
    OSG_NOTICE<<"SceneGraphBuilder::PartialDisk("<<inner<<", "<<outer<<", "<<slices<<", "<<loops<<", "<<start<<", "<<sweep<<") not implemented yet."<<std::endl;
    OSG_NOTICE<<"   quadric("<<_quadricState._drawStyle<<", "<<_quadricState._normals<<", "<<_quadricState._orientation<<", "<<_quadricState._texture<<std::endl;
}

void SceneGraphBuilder::Sphere(GLfloat        radius,
                               GLint          /*slices*/,
                               GLint          /*stacks*/)
{
    addShape(new osg::Sphere(osg::Vec3(0.0f,0.0f,0.0f), radius));
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//  General scene graph building methods
//

osg::Node* SceneGraphBuilder::getScene()
{
    if (_group.valid() && _group->getNumChildren()>0) return _group.get();
    else if (_transform.valid() && _transform->getNumChildren()>0) return _transform.get();
    else if (_geode.valid() && _geode->getNumDrawables()>0) return _geode.get();

    return 0;
}

osg::Node* SceneGraphBuilder::takeScene()
{
    osg::ref_ptr<osg::Node> node;

    if (_group.valid() && _group->getNumChildren()>0) node = _group.get();
    else if (_transform.valid() && _transform->getNumChildren()>0) node = _transform.get();
    else if (_geode.valid() && _geode->getNumDrawables()>0) node = _geode.get();

    // reset all the pointers to properly release the scene graph
    _geometry = 0;
    _geode = 0;
    _transform = 0;
    _group = 0;

    return node.release();
}

void SceneGraphBuilder::matrixChanged()
{
}

void SceneGraphBuilder::addAttribute(osg::StateAttribute* attribute)
{
    allocateStateSet();
    _stateset->setAttribute(attribute);
}

void SceneGraphBuilder::addMode(GLenum mode, bool enabled)
{
    allocateStateSet();
    _stateset->setMode(mode, enabled ? osg::StateAttribute::ON : osg::StateAttribute::OFF);
}


void SceneGraphBuilder::addTextureAttribute(unsigned int unit, osg::StateAttribute* attribute)
{
    allocateStateSet();
    _stateset->setTextureAttribute(unit, attribute);
}

void SceneGraphBuilder::addTextureMode(unsigned int unit, GLenum mode, bool enabled)
{
    allocateStateSet();
    _stateset->setTextureMode(unit, mode, enabled ? osg::StateAttribute::ON : osg::StateAttribute::OFF);
}

void SceneGraphBuilder::addShape(osg::Shape* shape)
{
    osg::ShapeDrawable* sd = new osg::ShapeDrawable(shape);
    sd->setColor(_color);

    addDrawable(sd);
}

void SceneGraphBuilder::addDrawable(osg::Drawable* drawable)
{
    if (!_geode) _geode = new osg::Geode;

    if (_stateset.valid())
    {
        drawable->setStateSet(_stateset.get());
        _statesetAssigned = true;
    }

    _geode->addDrawable(drawable);
}

void SceneGraphBuilder::allocateStateSet()
{
    if (_statesetAssigned)
    {
        _stateset = dynamic_cast<osg::StateSet*>(_stateset->clone(osg::CopyOp::SHALLOW_COPY));
        _statesetAssigned = false;
    }

    if (!_stateset) _stateset = new osg::StateSet;
}

void SceneGraphBuilder::allocateGeometry()
{
    if (!_geometry)
    {
        _geometry = new osg::Geometry;
    }
}

void SceneGraphBuilder::completeGeometry()
{
    if (_geometry.valid()) addDrawable(_geometry.get());
    _geometry = 0;
}
