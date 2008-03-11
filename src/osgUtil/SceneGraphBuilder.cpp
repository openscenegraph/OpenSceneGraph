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

#include <osg/Timer>
#include <osg/GLExtensions>
#include <osg/GLObjects>
#include <osg/Notify>
#include <osg/Texture>
#include <osg/AlphaFunc>
#include <osg/TexEnv>
#include <osg/ColorMatrix>
#include <osg/LightModel>
#include <osg/CollectOccludersVisitor>

#include <osg/GLU>

using namespace osgUtil;

SceneGraphBuilder::SceneGraphBuilder():
    _normal(0.0f,0.0f,1.0f),
    _color(1.0f,1.0f,1.0f,1.0f),
    _texCoord(0.f,0.0f,0.0f,0.0f)
{
}

void SceneGraphBuilder::glPushMatrix()
{
    if (_matrixStack.empty()) _matrixStack.push_back(osg::Matrixd());
    else _matrixStack.push_back(_matrixStack.back());
}

void SceneGraphBuilder::glPopMatrix()
{
    if (!_matrixStack.empty()) _matrixStack.pop_back();
}

void SceneGraphBuilder::glLoadIdentity()
{
    if (_matrixStack.empty()) _matrixStack.push_back(osg::Matrixd());
    
    _matrixStack.back().makeIdentity();
}

void SceneGraphBuilder::glLoadMatrixd(const GLdouble* m)
{
    
}

void SceneGraphBuilder::glMultMatrixd(const GLdouble* m)
{
}

void SceneGraphBuilder::glTranslated(GLdouble x, GLdouble y, GLdouble z)
{
}

void SceneGraphBuilder::glScaled(GLdouble x, GLdouble y, GLdouble z)
{
}

void SceneGraphBuilder::glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
}
void SceneGraphBuilder::glBlendFunc(GLenum srcFactor, GLenum dstFactor)
{
}

void SceneGraphBuilder::glCullFace(GLenum mode)
{
}

void SceneGraphBuilder::glDepthFunc(GLenum mode)
{
}

void SceneGraphBuilder::glFrontFace(GLenum mode)
{
}

void SceneGraphBuilder::glLineStipple(GLint factor, GLushort pattern)
{
}

void SceneGraphBuilder::glLineWidth(GLfloat lineWidth)
{
}

void SceneGraphBuilder::glPointSize(GLfloat pointSize)
{
}

void SceneGraphBuilder::glPolygonMode(GLenum face, GLenum mode)
{
}

void SceneGraphBuilder::glPolygonOffset(GLfloat factor, GLfloat units)
{
}

void SceneGraphBuilder::glPolygonStipple(GLubyte* mask)
{
}

void SceneGraphBuilder::glShadeModel(GLenum mode)
{
}
void SceneGraphBuilder::glEnable(GLenum mode)
{
}

void SceneGraphBuilder::glDisable(GLenum mode)
{
}
void SceneGraphBuilder::glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
}

void SceneGraphBuilder::glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
}

void SceneGraphBuilder::glNormal3f(GLfloat x, GLfloat y, GLfloat z)
{
}
void SceneGraphBuilder::glTexCoord1f(GLfloat x)
{
}

void SceneGraphBuilder::glTexCoord2f(GLfloat x, GLfloat y)
{
}

void SceneGraphBuilder::glTexCoord3f(GLfloat x, GLfloat y, GLfloat z)
{
}

void SceneGraphBuilder::glTexCoord4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
}
void SceneGraphBuilder::glBegin(GLenum mode)
{
}

void SceneGraphBuilder::glEnd()
{
}

osg::Node* SceneGraphBuilder::getScene()
{
}

osg::Node* SceneGraphBuilder::takeScene()
{
}
