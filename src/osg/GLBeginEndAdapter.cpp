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
    _normalAssigned(false),
    _normal(0.0f,0.0f,1.0f),
    _colorAssigned(false),
    _color(1.0f,1.0f,1.0f,1.0f),
    _overallNormalAssigned(false),
    _overallColorAssigned(false),
    _primitiveMode(0)
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

void GLBeginEndAdapter::MultiTexCoord4f(GLenum target, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    unsigned int unit = static_cast<unsigned int>(target-GL_TEXTURE0);

    if (unit>=_texCoordAssignedList.size()) _texCoordAssignedList.resize(unit+1, false);
    if (unit>=_texCoordList.size()) _texCoordList.resize(unit+1, osg::Vec4(0.0f,0.0f,0.0f,0.0f));

    _texCoordAssignedList[unit] = true;
    _texCoordList[unit].set(x,y,z,w);
}

void GLBeginEndAdapter::VertexAttrib4f(GLuint unit, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    if (unit>=_vertexAttribAssignedList.size()) _vertexAttribAssignedList.resize(unit+1, false);
    if (unit>=_vertexAttribList.size()) _vertexAttribList.resize(unit+1, osg::Vec4(0.0f,0.0f,0.0f,0.0f));

    _vertexAttribAssignedList[unit] = true;
    _vertexAttribList[unit].set(x,y,z,w);
}

void GLBeginEndAdapter::Vertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    osg::Vec3 vertex(x,y,z);

    if (!_vertices) _vertices = new osg::Vec3Array;

    if (_normalAssigned)
    {
        if (!_normals) _normals = new osg::Vec3Array;
        if (_normals->size()<_vertices->size()) _normals->resize(_vertices->size(), _overallNormal);

        _normals->push_back(_normal);
    }

    if (_colorAssigned)
    {
        if (!_colors) _colors = new osg::Vec4Array;
        if (_colors->size()<_vertices->size()) _colors->resize(_vertices->size(), _overallColor);

        _colors->push_back(_color);
    }

    if (!_texCoordAssignedList.empty())
    {
        for(unsigned int unit=0; unit<_texCoordAssignedList.size(); ++unit)
        {
            if (_texCoordAssignedList[unit])
            {
                if (unit>=_texCoordsList.size()) _texCoordsList.resize(unit+1);
                if (!_texCoordsList[unit]) _texCoordsList[unit] = new osg::Vec4Array;
                if (_texCoordsList[unit]->size()<_vertices->size()) _texCoordsList[unit]->resize(_vertices->size(), osg::Vec4(0.0,0.0f,0.0f,0.0f));

                _texCoordsList[unit]->push_back(_texCoordList[unit]);
            }
        }
    }


    if (!_vertexAttribAssignedList.empty())
    {
        for(unsigned int unit=0; unit<_vertexAttribAssignedList.size(); ++unit)
        {
            if (_vertexAttribAssignedList[unit])
            {
                if (unit>=_vertexAttribsList.size()) _vertexAttribsList.resize(unit+1);
                if (!_vertexAttribsList[unit]) _vertexAttribsList[unit] = new osg::Vec4Array;
                if (_vertexAttribsList[unit]->size()<_vertices->size()) _vertexAttribsList[unit]->resize(_vertices->size(), osg::Vec4(0.0,0.0f,0.0f,0.0f));

                _vertexAttribsList[unit]->push_back(_vertexAttribList[unit]);
            }
        }
    }

    _vertices->push_back(vertex);
}

void GLBeginEndAdapter::Begin(GLenum mode)
{
    _overallNormal = _normal;
    _overallColor = _color;

    // reset geometry
    _primitiveMode = mode;
    if (_vertices.valid()) _vertices->clear();

    _normalAssigned = false;
    if (_normals.valid()) _normals->clear();

    _colorAssigned = false;
    if (_colors.valid()) _colors->clear();

    _texCoordAssignedList.clear();
    _texCoordList.clear();
    for(VertexArrayList::iterator itr = _texCoordsList.begin();
        itr != _texCoordsList.end();
        ++itr)
    {
        if (itr->valid()) (*itr)->clear();
    }

    _vertexAttribAssignedList.clear();
    _vertexAttribList.clear();
}

void GLBeginEndAdapter::End()
{
    if (!_vertices || _vertices->empty()) return;

    if (!_matrixStack.empty())
    {
        const osg::Matrixd& matrix = _matrixStack.back();
        if (_mode==APPLY_LOCAL_MATRICES_TO_VERTICES)
        {
            osg::Matrix inverse;
            inverse.invert(matrix);

            for(Vec3Array::iterator itr = _vertices->begin();
                itr != _vertices->end();
                ++itr)
            {
                *itr = *itr * matrix;
            }

            if (_normalAssigned && _normals.valid())
            {
                for(Vec3Array::iterator itr = _normals->begin();
                    itr != _normals->end();
                    ++itr)
                {
                    *itr = osg::Matrixd::transform3x3(inverse, *itr);
                    (*itr).normalize();
                }
            }
            else
            {
                _overallNormal = osg::Matrixd::transform3x3(inverse, _overallNormal);
                _overallNormal.normalize();
            }
        }
        else
        {
            _state->applyModelViewMatrix(new RefMatrix(matrix));
        }
    }

    _state->lazyDisablingOfVertexAttributes();

    if (_colorAssigned)
    {
        _state->setColorPointer(_colors.get());
    }
    else if (_overallColorAssigned)
    {
        _state->Color(_overallColor.r(), _overallColor.g(), _overallColor.b(), _overallColor.a());
    }

    if (_normalAssigned)
    {
         _state->setNormalPointer(_normals.get());
    }
    else if (_overallNormalAssigned)
    {
        _state->Normal(_overallNormal.x(), _overallNormal.y(), _overallNormal.z());
    }

    for(unsigned int unit=0; unit<_texCoordAssignedList.size(); ++unit)
    {
        if (_texCoordAssignedList[unit] && _texCoordsList[unit].valid())
        {
            _state->setTexCoordPointer(unit, _texCoordsList[unit].get());
        }
    }


    for(unsigned int unit=0; unit<_vertexAttribAssignedList.size(); ++unit)
    {
        if (_vertexAttribAssignedList[unit] && _vertexAttribsList[unit].valid())
        {
            _state->setVertexAttribPointer(unit, _vertexAttribsList[unit].get());
        }
    }

    _state->setVertexPointer(_vertices.get());

    _state->applyDisablingOfVertexAttributes();

    if (_primitiveMode==GL_QUADS)
    {
        _state->drawQuads(0, _vertices->size());
    }
    else if (_primitiveMode==GL_QUAD_STRIP)
    {
        // will the winding be wrong? Do we need to swap it?
        glDrawArrays(GL_TRIANGLE_STRIP, 0, _vertices->size());
    }
    else if (_primitiveMode==GL_POLYGON) glDrawArrays(GL_TRIANGLE_FAN, 0, _vertices->size());
    else glDrawArrays(_primitiveMode, 0, _vertices->size());
}

void GLBeginEndAdapter::reset()
{
    _overallNormalAssigned = false;
    _overallColorAssigned = false;
}
