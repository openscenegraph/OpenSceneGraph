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
#include <osgUtil/TriStripVisitor>

#include <osgViewer/Viewer>

#include <iostream>

// for the grid data..
#include "../osghangglide/terrain_coords.h"

///////////////////////////////////////////////////////////////////
// vertex shader using just Vec4 coefficients
char vertexShaderSource_simple[] = 
    "uniform vec4 coeff; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "\n"
    "    gl_TexCoord[0] = gl_Vertex; \n"
    "    vec4 vert = gl_Vertex; \n"
    "    vert.z = gl_Vertex.x*coeff[0] + gl_Vertex.x*gl_Vertex.x* coeff[1] + \n"
    "             gl_Vertex.y*coeff[2] + gl_Vertex.y*gl_Vertex.y* coeff[3]; \n"
    "    gl_Position = gl_ModelViewProjectionMatrix * vert;\n"
    "}\n";
  
  
//////////////////////////////////////////////////////////////////
// vertex shader using full Matrix4 coefficients
char vertexShaderSource_matrix[] = 
    "uniform vec4  origin; \n"
    "uniform mat4  coeffMatrix; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "\n"
    "    gl_TexCoord[0] = gl_Vertex; \n"
    "    vec4 v = vec4(gl_Vertex.x, gl_Vertex.x*gl_Vertex.x, gl_Vertex.y, gl_Vertex.y*gl_Vertex.y ); \n"
    "    gl_Position = gl_ModelViewProjectionMatrix * (origin + coeffMatrix * v);\n"
    "}\n";

//////////////////////////////////////////////////////////////////
// vertex shader using texture read
char vertexShaderSource_texture[] = 
    "uniform sampler2D vertexTexture; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "\n"
    "    gl_TexCoord[0] = gl_Vertex; \n"
    "    vec4 vert = gl_Vertex; \n"
    "    vert.z = texture2D( vertexTexture, gl_TexCoord[0].xy).x*0.1; \n"
    "    gl_Position = gl_ModelViewProjectionMatrix * vert;\n"
    "}\n";

//////////////////////////////////////////////////////////////////
// vertex shader using texture read
char vertexShaderSource_texture2[] = 
    "uniform sampler2D vertexTexture; \n"
    "uniform sampler2D vertexTexture2; \n"
    "uniform float osg_FrameTime;\n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "\n"
    "    gl_TexCoord[0] = gl_Vertex; \n"
    "    vec4 vert = gl_Vertex; \n"
    "    float r = 1.0 + 0.5 * sin(osg_FrameTime);\n"
    "    vert.z = (texture2D( vertexTexture, gl_TexCoord[0].xy).x * (1-r) + texture2D( vertexTexture2, gl_TexCoord[0].xy).x * r)*0.1; \n"
    "    gl_Position = gl_ModelViewProjectionMatrix * vert;\n"
    "}\n";

//////////////////////////////////////////////////////////////////
// fragment shader
//
char fragmentShaderSource[] = 
    "uniform sampler2D baseTexture; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "    gl_FragColor = texture2D( baseTexture, gl_TexCoord[0].xy); \n"
    "}\n";

#if 0
char fragmentShaderNormalSource[] = 
    "uniform sampler2D baseTexture; \n"
    "uniform sampler2D vertexTexture; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "    const float dx = 0.0010;  \n"
    "    const float dy = 0.0010;  \n"
    "    float dz_dx = texture2D( vertexTexture, gl_TexCoord[0].xy + vec2(dx,0.0)).x - texture2D( vertexTexture, gl_TexCoord[0].xy + vec2(-dx,0.0)).x; \n"
    "    float dz_dy = texture2D( vertexTexture, gl_TexCoord[0].xy + vec2(0.0,dy)).x - texture2D( vertexTexture, gl_TexCoord[0].xy + vec2(0.0,-dy)).x; \n"
    "    vec3 normal = normalize(vec3(-dz_dx, -dz_dy, dx*50));\n"
    " \n"
    "    gl_FragColor = vec4(normal.z,normal.z,normal.z,1.0); \n"
    "}\n";
#else
char fragmentShaderNormalSource[] = 
    "uniform sampler2D baseTexture; \n"
    "uniform sampler2D vertexTexture; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "    vec3 normal = normalize(texture2D( baseTexture, gl_TexCoord[0].xy).xyz);\n"
    " \n"
    "    gl_FragColor = vec4(normal.z,normal.z,normal.z,1.0); \n"
    "}\n";
#endif

class UniformVarying : public osg::Uniform::Callback
{
    virtual void operator () (osg::Uniform* uniform, osg::NodeVisitor* nv)
    {
        const osg::FrameStamp* fs = nv->getFrameStamp();
        float value = sinf(fs->getReferenceTime());
        uniform->set(osg::Vec4(value,-value,-value,value));
    }
};

osg::Node* createModel(const std::string& shader, const std::string& textureFileName, const std::string& terrainFileName, const std::string& terrainFileName2, unsigned int cacheSize, unsigned int maxSize, bool joinStrips, const std::string& mesh)
{
    osg::Geode* geode = new osg::Geode;
    
    osg::Geometry* geom = new osg::Geometry;
    geode->addDrawable(geom);
    
    // dimensions for ~one million triangles :-)
    unsigned int num_x = 708;
    unsigned int num_y = 708;

    // set up state
    {
    
        osg::StateSet* stateset = geom->getOrCreateStateSet();

        osg::Program* program = new osg::Program;
        stateset->setAttribute(program);

        if (shader=="simple")
        {
            osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource_simple);
            program->addShader(vertex_shader);

            osg::Uniform* coeff = new osg::Uniform("coeff",osg::Vec4(1.0,-1.0f,-1.0f,1.0f));
            coeff->setUpdateCallback(new UniformVarying);
            stateset->addUniform(coeff);
        }
        else if (shader=="matrix")
        {
            osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource_matrix);
            program->addShader(vertex_shader);

            osg::Uniform* origin = new osg::Uniform("origin",osg::Vec4(0.0f,0.0f,0.0f,1.0f));
            stateset->addUniform(origin);

            osg::Uniform* coeffMatrix = new osg::Uniform("coeffMatrix",
                osg::Matrix(1.0f,0.0f,1.0f,0.0f,
                            0.0f,0.0f,-1.0f,0.0f,
                            0.0f,1.0f,-1.0f,0.0f,
                            0.0f,0.0f,1.0f,0.0f));

            stateset->addUniform(coeffMatrix);
        }
        else if (shader=="texture")
        {

            osg::Image* image = 0;

            if (terrainFileName.empty())
            {
                image = new osg::Image;
                unsigned int tx = 38;
                unsigned int ty = 39;
                image->allocateImage(tx,ty,1,GL_LUMINANCE,GL_FLOAT,1);
                for(unsigned int r=0;r<ty;++r)
                {
                    for(unsigned int c=0;c<tx;++c)
                    {
                        *((float*)image->data(c,r)) = vertex[r+c*39][2]*0.1;
                    }
                }

                num_x = tx;
                num_y = tx;
            }
            else
            {
                image = osgDB::readImageFile(terrainFileName);

                num_x = image->s();
                num_y = image->t();
            }

            osg::Texture2D* vertexTexture = new osg::Texture2D(image);


            vertexTexture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::NEAREST);
            vertexTexture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::NEAREST);
            
            vertexTexture->setInternalFormat(GL_LUMINANCE_FLOAT32_ATI);
            
            vertexTexture->setResizeNonPowerOfTwoHint(false);
            stateset->setTextureAttributeAndModes(1,vertexTexture);

            osg::Uniform* vertexTextureSampler = new osg::Uniform("vertexTexture",1);
            stateset->addUniform(vertexTextureSampler);

            if (!terrainFileName2.empty())
            {
                osg::Image* image2 = osgDB::readImageFile(terrainFileName2);
                osg::Texture2D* vertexTexture2 = new osg::Texture2D(image2);


                vertexTexture2->setFilter(osg::Texture::MIN_FILTER,osg::Texture::NEAREST);
                vertexTexture2->setFilter(osg::Texture::MAG_FILTER,osg::Texture::NEAREST);

                vertexTexture2->setInternalFormat(GL_LUMINANCE_FLOAT32_ATI);

                vertexTexture2->setResizeNonPowerOfTwoHint(false);
                stateset->setTextureAttributeAndModes(2,vertexTexture2);

                osg::Uniform* vertexTextureSampler2 = new osg::Uniform("vertexTexture2",2);
                stateset->addUniform(vertexTextureSampler2);

            }

        }

        if (terrainFileName2.empty())
        {
            osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderNormalSource);
            program->addShader(fragment_shader);

            osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource_texture);
            program->addShader(vertex_shader);
        }
        else
        {

            osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource);
            program->addShader(fragment_shader);

            osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource_texture2);
            program->addShader(vertex_shader);
        }
        
        osg::Texture2D* texture = new osg::Texture2D(osgDB::readImageFile(textureFileName));
        stateset->setTextureAttributeAndModes(0,texture);

        osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
        stateset->addUniform(baseTextureSampler);

    }


    // set up geometry data.

    osg::Vec3Array* vertices = new osg::Vec3Array( num_x * num_y );
    
    float dx = 1.0f/(float)(num_x-1);
    float dy = 1.0f/(float)(num_y-1);
    osg::Vec3 row(0.0f,0.0f,0.0);
    
    unsigned int vert_no = 0;
    unsigned int iy;
    for(iy=0; iy<num_y; ++iy)
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

    unsigned int totalIndices = 0;
    if (mesh=="triangles" || mesh=="tristrip")
    {
        if (cacheSize)
        {
            unsigned int index=0;
            osg::DrawElementsUInt* elements = new osg::DrawElementsUInt(GL_TRIANGLES);
            geom->addPrimitiveSet(elements);    
            elements->dirty();
            
            for(unsigned int is=0; is<num_x; is += cacheSize-1)
            {
            
                for(iy=0; iy<num_y-1; ++iy)
                {
                    unsigned int num_in_stripe = osg::minimum(cacheSize, num_x-is);
                    
                    bool rightToLeft = true;

                    if (elements->size() > maxSize)
                    {
                        osg::notify(osg::NOTICE)<<"elements->size() = "<<elements->size()<<std::endl;
                        totalIndices += elements->size();

                        index=0;
                        elements = new osg::DrawElementsUInt(GL_TRIANGLES);
                        geom->addPrimitiveSet(elements);    
                        elements->dirty();

                    }

                    if (rightToLeft)
                    {

                        index = iy * num_x + is;

                        for(unsigned int ix = 0; ix<num_in_stripe-1; ++ix)
                        {
                            (*elements).push_back(index + num_x);
                            (*elements).push_back(index);
                            (*elements).push_back(index+1);

                            (*elements).push_back(index + num_x);
                            (*elements).push_back(index+1);
                            (*elements).push_back(index + num_x + 1);

                            ++index;
                        }
                    }
                    else
                    {
                        index = iy * num_x + is + (num_x-2);

                        for(unsigned int ix = 0; ix<num_in_stripe-1; ++ix)
                        {
                            (*elements).push_back(index + num_x);
                            (*elements).push_back(index);
                            (*elements).push_back(index+1);

                            (*elements).push_back(index + num_x);
                            (*elements).push_back(index+1);
                            (*elements).push_back(index + num_x + 1);

                            --index;
                        }
                    }
                    
                    rightToLeft = !rightToLeft;                    
                    
                }
                
            }
            totalIndices += elements->size();
            osg::notify(osg::NOTICE)<<"elements->size() = "<<elements->size()<<std::endl;
        }
        else
        {
            osg::DrawElementsUInt* elements = new osg::DrawElementsUInt(GL_TRIANGLES, (num_x-1)*(num_y-1)*6);
            elements->dirty();
            unsigned int element_no = 0;
            for(iy=0; iy<num_y-1; ++iy)
            {
                unsigned int index = iy * num_x;
                for(unsigned int ix = 0; ix<num_x-1; ++ix)
                {
                    (*elements)[element_no++] = index + num_x;
                    (*elements)[element_no++] = index;
                    (*elements)[element_no++] = index+1;

                    (*elements)[element_no++] = index + num_x;
                    (*elements)[element_no++] = index+1;
                    (*elements)[element_no++] = index + num_x + 1;
                    
                    ++index;
                }
            }
            geom->addPrimitiveSet(elements);    
            osg::notify(osg::NOTICE)<<"elements->size() = "<<elements->size()<<std::endl;
        }
    }
    else
    {
    
        if (cacheSize)
        {
            bool needToJoin = false;
            unsigned int index=0;
            osg::DrawElementsUInt* elements = new osg::DrawElementsUInt(GL_TRIANGLE_STRIP);
            geom->addPrimitiveSet(elements);    
            elements->dirty();

            for(unsigned int is=0; is<num_x; is += cacheSize-1)
            {
            
                for(iy=0; iy<num_y-1; ++iy)
                {
                    unsigned int num_in_stripe = osg::minimum(cacheSize, num_x-is);
                    
                    bool rightToLeft = true;

                    if (elements->size() > maxSize)
                    {
                        osg::notify(osg::NOTICE)<<"elements->size() = "<<elements->size()<<std::endl;

                        totalIndices += elements->size();

                        needToJoin = false;
                        index=0;
                        elements = new osg::DrawElementsUInt(GL_TRIANGLE_STRIP);
                        geom->addPrimitiveSet(elements);    
                        elements->dirty();

                    }

                    if (needToJoin) (*elements).push_back(elements->back());
                    
                    if (rightToLeft)
                    {

                        index = iy * num_x + is;

                        if (needToJoin) (*elements).push_back(index + num_x);

                        for(unsigned int ix = 0; ix<num_in_stripe; ++ix)
                        {
                            (*elements).push_back(index + num_x);
                            (*elements).push_back(index++);
                        }
                    }
                    else
                    {
                        index = iy * num_x + is + (num_x-1);

                        if (needToJoin) (*elements).push_back(index);

                        for(unsigned int ix = 0; ix<num_in_stripe; ++ix)
                        {
                            (*elements).push_back(index);
                            (*elements).push_back(index-- + num_x);
                        }
                    }
                    
                    rightToLeft = !rightToLeft;                    
                    
                    needToJoin = true;
                }
                
            }
            totalIndices += elements->size();
        }
        else
        {
            osg::DrawElementsUInt* elements = new osg::DrawElementsUInt(GL_TRIANGLE_STRIP);
            elements->dirty();
            geom->addPrimitiveSet(elements);    
            bool needToJoin = false;
            unsigned int index=0;
            for(iy=0; iy<num_y-1; ++iy)
            {

                if (needToJoin) (*elements).push_back(index-1);

                index = iy * num_x;

                if (needToJoin) (*elements).push_back(index + num_x);

                for(unsigned int ix = 0; ix<num_x; ++ix)
                {
                    (*elements).push_back(index + num_x);
                    (*elements).push_back(index++);
                }
                needToJoin = true;

            }
        }
    }

    osg::notify(osg::NOTICE)<<"totalIndices = "<<totalIndices<<std::endl;

    if (mesh=="tristrip")
    {   osgUtil::TriStripVisitor stripper;
        stripper.stripify(*geom);
    }

#if 0
    geom->setUseVertexBufferObjects(true);
#else
    geom->setUseVertexBufferObjects(false);
#endif

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
    osgViewer::Viewer viewer;

    std::string shader("simple");
    while(arguments.read("-s",shader)) {}
    
    std::string textureFileName("Images/lz.rgb");
    while(arguments.read("-t",textureFileName)) {}

    std::string terrainFileName("");
    while(arguments.read("-d",terrainFileName)) {}

    std::string terrainFileName2("");
    while(arguments.read("-d2",terrainFileName2)) {}

    unsigned int cacheSize = 0;
    while(arguments.read("--cache",cacheSize)) {}

    unsigned int maxSize = 2048;
    while(arguments.read("--max",maxSize)) {}

    std::string mesh("strip");
    while(arguments.read("--mesh",mesh)) {}

    bool joinStrips = false;
    while(arguments.read("--join")) { joinStrips = true; }

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // load the nodes from the commandline arguments.
    osg::Node* model = createModel(shader,textureFileName,terrainFileName,terrainFileName2, cacheSize, maxSize, joinStrips,mesh);
    if (!model)
    {
        return 1;
    }
    
    viewer.setSceneData(model);

    return viewer.run();
}
