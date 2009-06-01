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
#include <osgUtil/GLObjectsVisitor>

#include <osg/Drawable>
#include <osg/Notify>
#include <osg/Timer>

namespace osgUtil 
{

/////////////////////////////////////////////////////////////////
//
// GLObjectsVisitor
//
GLObjectsVisitor::GLObjectsVisitor(Mode mode)
{
    setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);

    _mode = mode;

}


void GLObjectsVisitor::apply(osg::Node& node)
{
    if (node.getStateSet())
    {
        apply(*(node.getStateSet()));
    }

    traverse(node);
}

void GLObjectsVisitor::apply(osg::Geode& node)
{
    if (node.getStateSet())
    {
        apply(*(node.getStateSet()));
    }

    for(unsigned int i=0;i<node.getNumDrawables();++i)
    {
        osg::Drawable* drawable = node.getDrawable(i);
        if (drawable)
        {
            apply(*drawable);
            if (drawable->getStateSet())
            {
                apply(*(drawable->getStateSet()));
            }
        }
    }
}

void GLObjectsVisitor::apply(osg::Drawable& drawable)
{
    if (_drawablesAppliedSet.count(&drawable)!=0) return;
    
    _drawablesAppliedSet.insert(&drawable);

    if (_mode&SWITCH_OFF_DISPLAY_LISTS)
    {
        drawable.setUseDisplayList(false);
    }

    if (_mode&SWITCH_ON_DISPLAY_LISTS)
    {
        drawable.setUseDisplayList(true);
    }

    if (_mode&COMPILE_DISPLAY_LISTS && _renderInfo.getState())
    {
        drawable.compileGLObjects(_renderInfo);
    }

    if (_mode&RELEASE_DISPLAY_LISTS)
    {
        drawable.releaseGLObjects(_renderInfo.getState());
    }

    if (_mode&SWITCH_ON_VERTEX_BUFFER_OBJECTS)
    {
        drawable.setUseVertexBufferObjects(true);
    }

    if (_mode&SWITCH_OFF_VERTEX_BUFFER_OBJECTS)
    {
        drawable.setUseVertexBufferObjects(false);
    }
}

void GLObjectsVisitor::apply(osg::StateSet& stateset)
{
    if (_stateSetAppliedSet.count(&stateset)!=0) return;
    
    _stateSetAppliedSet.insert(&stateset);

    if (_mode & COMPILE_STATE_ATTRIBUTES && _renderInfo.getState())
    {
        stateset.compileGLObjects(*_renderInfo.getState());
        
        osg::Program* program = dynamic_cast<osg::Program*>(stateset.getAttribute(osg::StateAttribute::PROGRAM));
        if (program) {
            if( program->isFixedFunction() )
                _lastCompiledProgram = NULL; // It does not make sense to apply uniforms on fixed pipe
            else 
                _lastCompiledProgram = program;
        }

        if (_lastCompiledProgram.valid() && !stateset.getUniformList().empty())
        {
            osg::Program::PerContextProgram* pcp = _lastCompiledProgram->getPCP(_renderInfo.getState()->getContextID());
            if (pcp)
            {
                pcp->useProgram();
                
                _renderInfo.getState()->setLastAppliedProgramObject(pcp);
            
                osg::StateSet::UniformList& ul = stateset.getUniformList();
                for(osg::StateSet::UniformList::iterator itr = ul.begin();
                    itr != ul.end();
                    ++itr)
                {
                    pcp->apply(*(itr->second.first));
                }
            }
        }
        else if(_renderInfo.getState()->getLastAppliedProgramObject()){
                            
            osg::GL2Extensions* extensions = osg::GL2Extensions::Get(_renderInfo.getState()->getContextID(), true);
            extensions->glUseProgram(0);
            _renderInfo.getState()->setLastAppliedProgramObject(0);
        }
        
    }

    if (_mode & RELEASE_STATE_ATTRIBUTES)
    {
        stateset.releaseGLObjects(_renderInfo.getState());
    }
    
    if (_mode & CHECK_BLACK_LISTED_MODES)
    {
        stateset.checkValidityOfAssociatedModes(*_renderInfo.getState());
    }
}

/////////////////////////////////////////////////////////////////
//
// GLObjectsVisitor
//

GLObjectsOperation::GLObjectsOperation(GLObjectsVisitor::Mode mode):
    osg::GraphicsOperation("GLObjectOperation",false),
    _mode(mode)
{
}

GLObjectsOperation::GLObjectsOperation(osg::Node* subgraph, GLObjectsVisitor::Mode mode):
    osg::GraphicsOperation("GLObjectOperation",false),
    _subgraph(subgraph),
    _mode(mode)
{
}

void GLObjectsOperation::operator () (osg::GraphicsContext* context)
{
    GLObjectsVisitor glObjectsVisitor(_mode);
    
    context->getState()->initializeExtensionProcs();

    glObjectsVisitor.setState(context->getState());
    
    // osg::notify(osg::NOTICE)<<"GLObjectsOperation::before <<<<<<<<<<<"<<std::endl;
    if (_subgraph.valid())
    {
        _subgraph->accept(glObjectsVisitor);
    }
    else
    {
        for(osg::GraphicsContext::Cameras::iterator itr = context->getCameras().begin();
            itr != context->getCameras().end();
            ++itr)
        {
            (*itr)->accept(glObjectsVisitor);
        }
    }
    // osg::notify(osg::NOTICE)<<"GLObjectsOperation::after >>>>>>>>>>> "<<std::endl;
}


} // end of namespace osgUtil
