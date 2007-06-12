/* -*-c++-*- 
*
*  OpenSceneGraph example, osgshaders.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

/* file:	examples/osgglsl/GL2Scene.h
 * author:	Mike Weiblen 2005-03-30
 *
 * See http://www.3dlabs.com/opengl2/ for more information regarding
 * the OpenGL Shading Language.
*/

#include <osg/Node>
#include <osg/Referenced>
#include <osg/ref_ptr>

#include <osg/Program>

class GL2Scene : public osg::Referenced
{
    public:
	GL2Scene();

	osg::ref_ptr<osg::Group> getRootNode() { return _rootNode; }
	void reloadShaderSource();
	void toggleShaderEnable();

    protected:
	~GL2Scene();

    private:	/*methods*/
	osg::ref_ptr<osg::Group> buildScene();

    private:	/*data*/
	osg::ref_ptr<osg::Group> _rootNode;
	std::vector< osg::ref_ptr<osg::Program> > _programList;
	bool _shadersEnabled;
};

typedef osg::ref_ptr<GL2Scene> GL2ScenePtr;

/*EOF*/

