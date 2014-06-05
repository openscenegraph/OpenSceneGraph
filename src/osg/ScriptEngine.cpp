/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2013 Robert Osfield
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

#include <osg/ScriptEngine>
#include <osg/UserDataContainer>

using namespace osg;

ScriptEngine* ScriptNodeCallback::getScriptEngine(osg::NodePath& nodePath)
{
    if (!_script) return 0;

    for(osg::NodePath::reverse_iterator itr = nodePath.rbegin();
        itr != nodePath.rend();
        ++itr)
    {
        osg::Node* node = *itr;
        osg::UserDataContainer* udc = node->getUserDataContainer();
        if (udc)
        {
            ScriptEngine* engine = dynamic_cast<ScriptEngine*>(udc->getUserObject(_script->getLanguage()));
            if (engine) return engine;
        }
    }
    return 0;
}

void ScriptNodeCallback::operator()(Node* node, NodeVisitor* nv)
{
    ScriptEngine* engine = getScriptEngine(nv->getNodePath());
    if (engine && _script.valid())
    {
        // To handle the case where a NodeVisitor is created on the stack and can't be automatically ref counted
        // we take a reference to prevent the inputParameters reference to the NodeVisitor making it's ref count going to zero and causing a delete.
        ref_ptr<NodeVisitor> ref_nv(nv);

        {
            Parameters inputParameters;
            inputParameters.push_back(node);
            inputParameters.push_back(nv);

            // empty outputParameters
            Parameters outputParameters;

            engine->run(_script.get(), _entryPoint, inputParameters, outputParameters);
        }

        // now release the ref_ptr used to protected the NodeVisitor from deletion.
        ref_nv.release();
    }

    // note, callback is responsible for scenegraph traversal so
    // they must call traverse(node,nv) to ensure that the
    // scene graph subtree (and associated callbacks) are traversed.
    traverse(node,nv);
}
