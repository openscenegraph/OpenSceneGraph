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
    _normal(0.0f,0.0f,1.0f),
    _color(1.0f,1.0f,1.0f,1.0f),
    _texCoord(0.f,0.0f,0.0f,0.0f)
{
}


///////////////////////////////////////////////////////////////////////////////////////////
//
// OpenGL 1.0 building methods
//
void SceneGraphBuilder::glPushMatrix()
{
    if (_matrixStack.empty()) _matrixStack.push_back(osg::Matrixd());
    else _matrixStack.push_back(_matrixStack.back());
}

void SceneGraphBuilder::glPopMatrix()
{
    if (!_matrixStack.empty()) _matrixStack.pop_back();
    
    matrixChanged();
}

void SceneGraphBuilder::glLoadIdentity()
{
    if (_matrixStack.empty()) _matrixStack.push_back(osg::Matrixd());
    _matrixStack.back().makeIdentity();

    matrixChanged();
}

void SceneGraphBuilder::glLoadMatrixd(const GLdouble* m)
{
    if (_matrixStack.empty()) _matrixStack.push_back(osg::Matrixd());
    _matrixStack.back().set(m);

    matrixChanged();
}

void SceneGraphBuilder::glMultMatrixd(const GLdouble* m)
{
    if (_matrixStack.empty()) _matrixStack.push_back(osg::Matrixd());
    _matrixStack.back().preMult(osg::Matrixd(m));

    matrixChanged();
}

void SceneGraphBuilder::glTranslated(GLdouble x, GLdouble y, GLdouble z)
{
    if (_matrixStack.empty()) _matrixStack.push_back(osg::Matrixd());
    _matrixStack.back().preMult(osg::Matrixd::translate(x,y,z));

    matrixChanged();
}

void SceneGraphBuilder::glScaled(GLdouble x, GLdouble y, GLdouble z)
{
    if (_matrixStack.empty()) _matrixStack.push_back(osg::Matrixd());
    _matrixStack.back().preMult(osg::Matrixd::scale(x,y,z));

    matrixChanged();
}

void SceneGraphBuilder::glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    if (_matrixStack.empty()) _matrixStack.push_back(osg::Matrixd());
    _matrixStack.back().preMult(osg::Matrixd::rotate(osg::inDegrees(angle),x,y,z));

    matrixChanged();
}

void SceneGraphBuilder::glBlendFunc(GLenum srcFactor, GLenum dstFactor)
{
    addAttribute(new osg::BlendFunc(srcFactor, dstFactor));
}

void SceneGraphBuilder::glCullFace(GLenum mode)
{
    addAttribute(new osg::CullFace(osg::CullFace::Mode(mode)));
}

void SceneGraphBuilder::glDepthFunc(GLenum mode)
{
    addAttribute(new osg::Depth(osg::Depth::Function(mode)));
}

void SceneGraphBuilder::glFrontFace(GLenum mode)
{
    addAttribute(new osg::FrontFace(osg::FrontFace::Mode(mode)));
}

void SceneGraphBuilder::glLineStipple(GLint factor, GLushort pattern)
{
    addAttribute(new osg::LineStipple(factor, pattern));
}

void SceneGraphBuilder::glLineWidth(GLfloat lineWidth)
{
    addAttribute(new osg::LineWidth(lineWidth));
}

void SceneGraphBuilder::glPointSize(GLfloat pointSize)
{
    addAttribute(new osg::Point(pointSize));
}

void SceneGraphBuilder::glPolygonMode(GLenum face, GLenum mode)
{
    addAttribute(new osg::PolygonMode(osg::PolygonMode::Face(face),osg::PolygonMode::Mode(mode)));
}

void SceneGraphBuilder::glPolygonOffset(GLfloat factor, GLfloat units)
{
    addAttribute(new osg::PolygonOffset(factor,units));
}

void SceneGraphBuilder::glPolygonStipple(const GLubyte* mask)
{
    addAttribute(new osg::PolygonStipple(mask));
}

void SceneGraphBuilder::glShadeModel(GLenum mode)
{
    addAttribute(new osg::ShadeModel(osg::ShadeModel::Mode(mode)));
}
void SceneGraphBuilder::glEnable(GLenum mode)
{
    addMode(mode, true);
}

void SceneGraphBuilder::glDisable(GLenum mode)
{
    addMode(mode, false);
}

void SceneGraphBuilder::glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    _normalSet = true;
    _color.set(red,green,blue,alpha);
}

void SceneGraphBuilder::glNormal3f(GLfloat x, GLfloat y, GLfloat z)
{
    _normalSet = true;
    _normal.set(x,y,z);
}

void SceneGraphBuilder::glTexCoord1f(GLfloat x)
{
    _maxNumTexCoordComponents = 1;
    _texCoord.set(x,0.0f,0.0f,1.0f);
}

void SceneGraphBuilder::glTexCoord2f(GLfloat x, GLfloat y)
{
    _maxNumTexCoordComponents = 2;
    _texCoord.set(x,y,0.0f,1.0f);
}

void SceneGraphBuilder::glTexCoord3f(GLfloat x, GLfloat y, GLfloat z)
{
    _maxNumTexCoordComponents = 3;
    _texCoord.set(x,y,z,1.0);
}

void SceneGraphBuilder::glTexCoord4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    _maxNumTexCoordComponents = 4;
    _texCoord.set(x,y,z,w);
}

void SceneGraphBuilder::glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    osg::Vec3 vertex(x,y,z);
    
    vertex = vertex * _matrixStack.back();
    
    if (_vertices.valid()) _vertices->push_back(vertex);
    if (_normal.valid()) _normals->push_back(_normal);
    if (_colors.valid()) _colors->push_back(_color);
    if (_texCoords.valid()) _texCoords->push_back(_texCoord);
}

void SceneGraphBuilder::glBegin(GLenum mode)
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

void SceneGraphBuilder::glEnd()
{
    allocateGeometry();

    _geometry->setVertexArray(_vertices.get());
    
    if (_colorSet)
    {
         _geometry->setColorArray(_colors.get());
         _geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    }
    else
    {
         osg::Vec4Array* colors = new osg::Vec4Array;
         colors->push_back(_color);
         
         _geometry->setColorArray(colors);
         _geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    }
    
    if (_normalSet)
    {
         _geometry->setNormalArray(_normals.get());
         _geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    }
    else
    {
         _geometry->setNormalBinding(osg::Geometry::BIND_OFF);
    }
    
    if (_maxNumTexCoordComponents>0)
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
void SceneGraphBuilder::gluQuadricDrawStyle(GLenum aDrawStyle)
{
    _quadricState._drawStyle = aDrawStyle;
}

void SceneGraphBuilder::gluQuadricNormals(GLenum aNormals)
{
    _quadricState._normals = aNormals;
}

void SceneGraphBuilder::gluQuadricOrientation(GLenum aOrientation)
{
    _quadricState._orientation = aOrientation;
}

void SceneGraphBuilder::gluQuadricTexture(GLboolean aTexture)
{
    _quadricState._texture = aTexture;
}

void SceneGraphBuilder::gluCylinder(GLfloat        aBase,
                             GLfloat        aTop,
                             GLfloat        aHeight,
                             GLint          aSlices,
                             GLint          aStacks)
{
    osg::notify(osg::NOTICE)<<"SceneGraphBuilder::gluCylinder("<<aBase<<", "<<aTop<<", "<<aHeight<<", "<<aSlices<<", "<<aStacks<<")"<<std::endl;
    osg::notify(osg::NOTICE)<<"   quadric("<<_quadricState._drawStyle<<", "<<_quadricState._normals<<", "<<_quadricState._orientation<<", "<<_quadricState._texture<<std::endl;
}

void SceneGraphBuilder::gluDisk(GLfloat        aInner,
                         GLfloat        aOuter,
                         GLint          aSlices,
                         GLint          aLoops)
{
    osg::notify(osg::NOTICE)<<"SceneGraphBuilder::gluDisk("<<aInner<<", "<<aOuter<<", "<<aSlices<<", "<<aLoops<<")"<<std::endl;
    osg::notify(osg::NOTICE)<<"   quadric("<<_quadricState._drawStyle<<", "<<_quadricState._normals<<", "<<_quadricState._orientation<<", "<<_quadricState._texture<<std::endl;
}

void SceneGraphBuilder::gluPartialDisk(GLfloat        aInner,
                                GLfloat        aOuter,
                                GLint          aSlices,
                                GLint          aLoops,
                                GLfloat        aStart,
                                GLfloat        aSweep)
{
    osg::notify(osg::NOTICE)<<"SceneGraphBuilder::gluPartialDisk("<<aInner<<", "<<aOuter<<", "<<aSlices<<", "<<aLoops<<", "<<aStart<<", "<<aSweep<<")"<<std::endl;
    osg::notify(osg::NOTICE)<<"   quadric("<<_quadricState._drawStyle<<", "<<_quadricState._normals<<", "<<_quadricState._orientation<<", "<<_quadricState._texture<<std::endl;
}

void SceneGraphBuilder::gluSphere(GLfloat        aRadius,
                           GLint          aSlices,
                           GLint          aStacks)
{
    addShape(new osg::Sphere(osg::Vec3(0.0f,0.0f,0.0f), aRadius));
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

    osg::notify(osg::NOTICE)<<"SceneGraphBuilder::addAttribute("<<attribute->className()<<")"<<std::endl;
}

void SceneGraphBuilder::addMode(GLenum mode, bool enabled)
{
    osg::notify(osg::NOTICE)<<"SceneGraphBuilder::addMode("<<mode<<","<<enabled<<")"<<std::endl;

    allocateStateSet();
    _stateset->setMode(mode, enabled ? osg::StateAttribute::ON : osg::StateAttribute::OFF);
}


void SceneGraphBuilder::addTextureAttribute(unsigned int unit, osg::StateAttribute* attribute)
{
    osg::notify(osg::NOTICE)<<"SceneGraphBuilder::addAttribute("<<attribute->className()<<")"<<std::endl;

    allocateStateSet();
    _stateset->setTextureAttribute(unit, attribute);
}

void SceneGraphBuilder::addTextureMode(unsigned int unit, GLenum mode, bool enabled)
{
    osg::notify(osg::NOTICE)<<"SceneGraphBuilder::addTextureMode("<<mode<<","<<enabled<<")"<<std::endl;

    allocateStateSet();
    _stateset->setTextureMode(unit, mode, enabled ? osg::StateAttribute::ON : osg::StateAttribute::OFF);
}

void SceneGraphBuilder::addShape(osg::Shape* shape)
{
    osg::notify(osg::NOTICE)<<"SceneGraphBuilder::addShape("<<shape->className()<<std::endl;
    osg::ShapeDrawable* sd = new osg::ShapeDrawable(shape);
    sd->setColor(_color);
    
    addDrawable(sd);
}

void SceneGraphBuilder::addDrawable(osg::Drawable* drawable)
{
    if (!_geode) _geode = new osg::Geode;
    
    _geode->addDrawable(drawable);
}

void SceneGraphBuilder::allocateStateSet()
{
    if (_geometry.valid())
    {
        completeGeometry();
        
        _stateset = 0;
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
