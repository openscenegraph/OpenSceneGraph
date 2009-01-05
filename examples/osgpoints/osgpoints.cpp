/* OpenSceneGraph example, osgpoints.
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

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgViewer/Viewer>

#include <osg/Point>
#include <osg/BlendFunc>
#include <osg/Texture2D>
#include <osg/PointSprite>
#include <osg/PolygonMode>

#include <iostream>

class KeyboardEventHandler : public osgGA::GUIEventHandler
{
public:
    
        KeyboardEventHandler(osg::StateSet* stateset):
            _stateset(stateset)
        {
            _point = new osg::Point;
            _point->setDistanceAttenuation(osg::Vec3(0.0,0.0000,0.05f));
            _stateset->setAttribute(_point.get());
        }
    
        virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
        {
            switch(ea.getEventType())
            {
                case(osgGA::GUIEventAdapter::KEYDOWN):
                {
                    if (ea.getKey()=='+' || ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Add)
                    {
                       changePointSize(1.0f);
                       return true;
                    }
                    else if (ea.getKey()=='-' || ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Subtract)
                    {
                       changePointSize(-1.0f);
                       return true;
                    }
                    else if (ea.getKey()=='<')
                    {
                       changePointAttenuation(1.1f);
                       return true;
                    }
                    else if (ea.getKey()=='>')
                    {
                       changePointAttenuation(1.0f/1.1f);
                       return true;
                    }
                    break;
                }
                default:
                    break;
            }
            return false;
        }
        
        
        float getPointSize() const
        {
            return _point->getSize();
        }
        
        void setPointSize(float psize)
        {
            if (psize>0.0)
            {
                _point->setSize(psize);
            }
            std::cout<<"Point size "<<psize<<std::endl;
        }

        void changePointSize(float delta)
        {
            setPointSize(getPointSize()+delta);
        }

        void changePointAttenuation(float scale)
        {
            _point->setDistanceAttenuation(_point->getDistanceAttenuation()*scale);
        }
        
        osg::ref_ptr<osg::StateSet> _stateset;
        osg::ref_ptr<osg::Point>    _point;
        
};

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" example provides an interactive viewer for visualising point clouds..");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--sprites","Point sprites.");
    arguments.getApplicationUsage()->addCommandLineOption("--points","Sets the polygon mode to GL_POINT for front and back faces.");


    // construct the viewer.
    osgViewer::Viewer viewer;

    bool shader = false;
    while (arguments.read("--shader")) shader = true;

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }
    
    bool usePointSprites = false;
    while (arguments.read("--sprites")) { usePointSprites = true; };

    bool forcePointMode = false;
    while (arguments.read("--points")) { forcePointMode = true; };

    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

    // if no model has been successfully loaded report failure.
    if (!loadedModel) 
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel.get());


    // set the scene to render
    viewer.setSceneData(loadedModel.get());
    

    osg::StateSet* stateset = loadedModel->getOrCreateStateSet();
    if (usePointSprites)    
    {
        /// Setup cool blending
        osg::BlendFunc *fn = new osg::BlendFunc();
        stateset->setAttributeAndModes(fn, osg::StateAttribute::ON);

        /// Setup the point sprites
        osg::PointSprite *sprite = new osg::PointSprite();
        stateset->setTextureAttributeAndModes(0, sprite, osg::StateAttribute::ON);

        /// The texture for the sprites
        osg::Texture2D *tex = new osg::Texture2D();
        tex->setImage(osgDB::readImageFile("Images/particle.rgb"));
        stateset->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON);
    }

    if( forcePointMode )
    {
        /// Set polygon mode to GL_POINT
        osg::PolygonMode *pm = new osg::PolygonMode(
            osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::POINT );
        stateset->setAttributeAndModes( pm, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    }
    

    // register the handler for modifying the point size
    viewer.addEventHandler(new KeyboardEventHandler(viewer.getCamera()->getOrCreateStateSet()));


    if (shader)
    {
        osg::StateSet* stateset = loadedModel->getOrCreateStateSet();
    
        ///////////////////////////////////////////////////////////////////
        // vertex shader using just Vec4 coefficients
        char vertexShaderSource[] = 
            "void main(void) \n"
            "{ \n"
            "\n"
            "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
            "}\n";



        osg::Program* program = new osg::Program;
        stateset->setAttribute(program);

        osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource);
        program->addShader(vertex_shader);

#if 0
        //////////////////////////////////////////////////////////////////
        // fragment shader
        //
        char fragmentShaderSource[] = 
            "void main(void) \n"
            "{ \n"
            "    gl_FragColor = gl_Color; \n"
            "}\n";

        osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource);
        program->addShader(fragment_shader);
#endif
    }

    return viewer.run();
}

