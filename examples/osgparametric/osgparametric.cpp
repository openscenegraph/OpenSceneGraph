#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Quat>
#include <osg/Matrix>
#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Texture2D>

#include <osgDB/FileUtils>
#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>

#include <osgProducer/Viewer>

char vertexShaderSource[] = 
    "uniform vec2  xCoeff; \n"
    "uniform vec2  yCoeff; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "\n"
    "    gl_TexCoord[0] = gl_Vertex; \n"
    "    gl_Vertex.z = gl_Vertex.x*xCoeff[0] + gl_Vertex.x*gl_Vertex.x* xCoeff[1] + \n"
    "                  gl_Vertex.y*yCoeff[1] + gl_Vertex.y*gl_Vertex.y* yCoeff[1]; \n"
    "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
    "}\n";
    
char fragmentShaderSource[] = 
    "uniform sampler2D baseTexture; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "    gl_FragColor = texture2D( baseTexture, gl_TexCoord[0].xy); \n"
    "}\n";


class UniformVarying : public osg::Uniform::Callback
{
    virtual void operator () (osg::Uniform* uniform, osg::NodeVisitor* nv)
    {
        const osg::FrameStamp* fs = nv->getFrameStamp();
        float value = sinf(fs->getReferenceTime());
        uniform->set(osg::Vec2(value,-value));
    }
};

osg::Node* createModel()
{
    osg::Geode* geode = new osg::Geode;
    
    osg::Geometry* geom = new osg::Geometry;
    geode->addDrawable(geom);
    
    // dimensions for ~one million triangles :-)
    unsigned int num_x = 708;
    unsigned int num_y = 708;

    osg::Vec3Array* vertices = new osg::Vec3Array( num_x * num_y );
    
    float dx = 1.0f/(float)(num_x-1);
    float dy = 1.0f/(float)(num_y-1);
    osg::Vec3 row(0.0f,0.0f,0.0);
    
    unsigned int vert_no = 0;
    for(unsigned int iy=0; iy<num_y; ++iy)
    {
        osg::Vec3 column = row;
        for(unsigned int ix=0;ix<num_x;++ix)
        {
            (*vertices)[vert_no++] = column;
            column.x() += dx;
        }        
        row.y() += dy;
    }

    geom->setVertexArray(vertices);

    for(unsigned int iy=0; iy<num_y-1; ++iy)
    {
        unsigned int element_no = 0;
        osg::DrawElementsUInt* elements = new osg::DrawElementsUInt(GL_TRIANGLE_STRIP, num_x*2);
        unsigned int index = iy * num_x;
        for(unsigned int ix = 0; ix<num_x; ++ix)
        {
            (*elements)[element_no++] = index + num_x;
            (*elements)[element_no++] = index++;
        }
        geom->addPrimitiveSet(elements);    
    }
    
    geom->setUseVertexBufferObjects(true);
    
    
    osg::StateSet* stateset = geom->getOrCreateStateSet();

    osg::Program* program = new osg::Program;
    stateset->setAttribute(program);

    osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource);
    program->addShader(vertex_shader);
    

    osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource);
    program->addShader(fragment_shader);


    osg::Uniform* xCoeff = new osg::Uniform("xCoeff",osg::Vec2(1.0,-1.0f));
    xCoeff->setUpdateCallback(new UniformVarying);
    stateset->addUniform(xCoeff);

    osg::Uniform* yCoeff = new osg::Uniform("yCoeff",osg::Vec2(-1.0f,1.0f));
    stateset->addUniform(yCoeff);
    
    
    stateset->setTextureAttributeAndModes(0,new osg::Texture2D(osgDB::readImageFile("lz.rgb")));

    osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
    stateset->addUniform(baseTextureSampler);
    
    return geode;

}

int main(int argc, char *argv[])
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrate support for ARB_vertex_program.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
   
    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    // load the nodes from the commandline arguments.
    osg::Node* model = createModel();
    if (!model)
    {
        return 1;
    }

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData(model);
    
    // create the windows and run the threads.
    viewer.realize();

    while( !viewer.done() )
    {
        // wait for all cull and draw threads to complete.
        viewer.sync();

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        viewer.update();
         
        // fire off the cull and draw traversals of the scene.
        viewer.frame();
        
    }
    
    // wait for all cull and draw threads to complete before exit.
    viewer.sync();

    return 0;
}
