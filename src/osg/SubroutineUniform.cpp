/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 * Copyright (C) 2016 Julien Valentin
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

#include <osg/SubroutineUniform>
#include <osg/State>

using namespace osg;

void SubroutineUniform::apply(State& state) const
{
    GLExtensions* extensions = state.get<GLExtensions>();
    GLuint contextID = state.getContextID();

    if(!_subroutineNames.empty()&&_program.valid())
    {
        Program::PerContextProgram* pcp = _program->getPCP( state );

        if( pcp->needsLink() ) _program->compileGLObjects( state );
        if( pcp->isLinked() )
        {
            if(pcp != state.getLastAppliedProgramObject()){///IMPORTANT that why I didn't use program->apply
                if( osg::isNotifyEnabled(osg::INFO) )
                    pcp->validateProgram();

                pcp->useProgram();
                state.setLastAppliedProgramObject(pcp);
            }
            ///else NOTHING: AVOID THE COST OF BINDING PROGRAM (OTHERWISE THERE NO GAIN IN USING SUBROUTINES)

            if(_indicesSetted[contextID]==0)
            {
                ///update uniform index (assume program is the LastAppliedProgramObject)
                _indices[contextID].resize(_subroutineNames.size());

                {
                    GLuint GLpo = pcp->getHandle();
                    std::vector<GLuint>::iterator percontextroutineindexit=_indices[contextID].begin();
                    for(std::vector<std::string>::const_iterator it=_subroutineNames.begin(); it!=_subroutineNames.end(); it++,percontextroutineindexit++)
                    {
                        *percontextroutineindexit=
                            extensions->glGetSubroutineIndex(GLpo,_shadertype,it->c_str());
                    }
                    _indicesSetted[contextID]=1;
                }
            }

            extensions->glUniformSubroutinesuiv(_shadertype,_subroutineNames.size(),&_indices[contextID].front());
        }
        else
        {
            // program not usable, fallback to fixed function.
            extensions->glUseProgram( 0 );
            state.setLastAppliedProgramObject(0);
        }
    }
}
bool SubroutineUniform::setShaderType( Shader::Type shadertype)
{
    if(shadertype == _shadertype) return true;

    ReassignToParents needToReassingToParentsWhenMemberValueChanges(this);

    _shadertype=shadertype;
    return true;
}

SubroutineUniform::~SubroutineUniform() {}
