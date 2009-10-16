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
#include <osg/GLBeginEndAdapter>
#include <osg/State>

#include <osg/Notify>
#include <osg/io_utils>

using namespace osg;

GLBeginEndAdapter::GLBeginEndAdapter(State* state):
    _state(state),
    _mode(APPLY_LOCAL_MATRICES_TO_VERTICES),
    _normalSet(false),
    _normal(0.0f,0.0f,1.0f),
    _colorSet(false),
    _color(1.0f,1.0f,1.0f,1.0f),
    _maxNumTexCoordComponents(0),
    _texCoord(0.f,0.0f,0.0f,1.0f)
{
}

void GLBeginEndAdapter::PushMatrix()
{
    if (_matrixStack.empty())
    {
        if (_mode==APPLY_LOCAL_MATRICES_TO_VERTICES) _matrixStack.push_back(Matrixd());
        else _matrixStack.push_back(_state->getModelViewMatrix());
    }
    else _matrixStack.push_back(_matrixStack.back());
}

void GLBeginEndAdapter::PopMatrix()
{
    if (!_matrixStack.empty()) _matrixStack.pop_back();
}


void GLBeginEndAdapter::LoadIdentity()
{
    if (_matrixStack.empty()) _matrixStack.push_back(Matrixd::identity());
    else _matrixStack.back().makeIdentity();
}

void GLBeginEndAdapter::LoadMatrixd(const GLdouble* m)
{
    if (_matrixStack.empty()) _matrixStack.push_back(Matrixd(m));
    else _matrixStack.back().set(m);
}

void GLBeginEndAdapter::MultMatrixd(const GLdouble* m)
{
    if (_matrixStack.empty())
    {
        if (_mode==APPLY_LOCAL_MATRICES_TO_VERTICES) _matrixStack.push_back(Matrixd());
        else _matrixStack.push_back(_state->getModelViewMatrix());
    }
    _matrixStack.back().preMult(Matrixd(m));
}


void GLBeginEndAdapter::Translated(GLdouble x, GLdouble y, GLdouble z)
{
    if (_matrixStack.empty())
    {
        if (_mode==APPLY_LOCAL_MATRICES_TO_VERTICES) _matrixStack.push_back(Matrixd());
        else _matrixStack.push_back(_state->getModelViewMatrix());
    }
    _matrixStack.back().preMultTranslate(Vec3d(x,y,z));
}

void GLBeginEndAdapter::Scaled(GLdouble x, GLdouble y, GLdouble z)
{
    if (_matrixStack.empty())
    {
        if (_mode==APPLY_LOCAL_MATRICES_TO_VERTICES) _matrixStack.push_back(Matrixd());
        else _matrixStack.push_back(_state->getModelViewMatrix());
    }
    _matrixStack.back().preMultScale(Vec3d(x,y,z));
}

void GLBeginEndAdapter::Rotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    if (_matrixStack.empty())
    {
        if (_mode==APPLY_LOCAL_MATRICES_TO_VERTICES) _matrixStack.push_back(Matrixd());
        else _matrixStack.push_back(_state->getModelViewMatrix());
    }
    _matrixStack.back().preMultRotate(Quat(DegreesToRadians(angle), Vec3d(x,y,z)));
}

void GLBeginEndAdapter::Color4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    _normalSet = true;
    _color.set(red,green,blue,alpha);
}

void GLBeginEndAdapter::Normal3f(GLfloat x, GLfloat y, GLfloat z)
{
    _normalSet = true;
    _normal.set(x,y,z);
}

void GLBeginEndAdapter::TexCoord1f(GLfloat x)
{
    _maxNumTexCoordComponents = 1;
    _texCoord.set(x,0.0f,0.0f,1.0f);
}

void GLBeginEndAdapter::TexCoord2f(GLfloat x, GLfloat y)
{
    _maxNumTexCoordComponents = 2;
    _texCoord.set(x,y,0.0f,1.0f);
}

void GLBeginEndAdapter::TexCoord3f(GLfloat x, GLfloat y, GLfloat z)
{
    _maxNumTexCoordComponents = 3;
    _texCoord.set(x,y,z,1.0);
}

void GLBeginEndAdapter::TexCoord4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    _maxNumTexCoordComponents = 4;
    _texCoord.set(x,y,z,w);
}

void GLBeginEndAdapter::Vertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    osg::Vec3 vertex(x,y,z);

    if (_vertices.valid()) _vertices->push_back(vertex);
    if (_normal.valid()) _normals->push_back(_normal);
    if (_colors.valid()) _colors->push_back(_color);
    if (_texCoords.valid()) _texCoords->push_back(_texCoord);
}

void GLBeginEndAdapter::Begin(GLenum mode)
{
    _overallNormal = _normal;
    _overallColor = _color;

    // reset geometry
    _primitiveMode = mode;
    if (!_vertices) _vertices = new osg::Vec3Array;
    else _vertices->clear();

    _normalSet = false;
    if (!_normals) _normals = new osg::Vec3Array;
    else _normals->clear();

    _colorSet = false;
    if (!_colors) _colors = new osg::Vec4Array;
    else _colors->clear();

    _maxNumTexCoordComponents = 0;
    if (!_texCoords) _texCoords = new osg::Vec4Array;
    else _texCoords->clear();

}

void GLBeginEndAdapter::End()
{
    if (!_vertices || _vertices->empty()) return;

    if (!_matrixStack.empty())
    {
        const osg::Matrixd& matrix = _matrixStack.back();
        if (_mode==APPLY_LOCAL_MATRICES_TO_VERTICES)
        {
            for(Vec3Array::iterator itr = _vertices->begin();
                itr != _vertices->end();
                ++itr)
            {
                *itr = *itr * matrix;
            }
        }
        else
        {
            _state->applyModelViewMatrix(new RefMatrix(matrix));
        }
    }

    _state->lazyDisablingOfVertexAttributes();

    _state->setVertexPointer(_vertices.get());

    if (_colorSet)
    {
        _state->setColorPointer(_colors.get());
    }
    else
    {
        glColor4fv(_overallColor.ptr());
    }
    
    if (_normalSet)
    {
         _state->setNormalPointer(_normals.get());
    }
    else
    {
        glNormal3fv(_overallNormal.ptr());
    }

    if (_maxNumTexCoordComponents!=0)
    {
        _state->setTexCoordPointer(0, _texCoords.get());
    }

    _state->applyDisablingOfVertexAttributes();

    glDrawArrays(_primitiveMode, 0, _vertices->size());
}
