#include <iostream>

#include <osg/Array>
#include <osg/BoundingSphere>
#include <osg/BufferIndexBinding>
#include <osg/BufferObject>
#include <osg/Group>
#include <osg/Math>
#include <osg/MatrixTransform>
#include <osg/Program>
#include <osg/Shader>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgUtil/Optimizer>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

using namespace std;
using namespace osg;

// This example is based on the sample code in the
// ARB_uniform_buffer_object extension specification.

GLfloat colors1[] = {
    // block
    0.45,0.45,1,1,
    0.45,0.45,1,1,
    0.75,0.75,0.75,1,
    0.0,0.0,1.0,1,
    0.0,1.0,0.0,1
};
GLfloat colors2[] = {
    // block
    0.45,0.45,1,1,
    0.45,0.45,1,1,
    0.75,0.75,0.75,1,
    1.0,0.0,0.0,1,
    0.0,1.0,0.0,1,
};

char vertexShaderSource[] =
    "// Vertex shader for Gooch shading\n"
    "// Author: Randi Rost\n"
    "// Copyright (c) 2002-2006 3Dlabs Inc. Ltd.\n"
    "// See 3Dlabs-License.txt for license information\n"

    "vec3 LightPosition = vec3(0.0, 10.0, 4.0); \n"

    "varying float NdotL;\n"
    "varying vec3  ReflectVec;\n"
    "varying vec3  ViewVec;\n"

    "void main(void)\n"
    "{\n"
        "vec3 ecPos      = vec3 (gl_ModelViewMatrix * gl_Vertex);\n"
        "vec3 tnorm      = normalize(gl_NormalMatrix * gl_Normal);\n"
        "vec3 lightVec   = normalize(LightPosition - ecPos);\n"
        "ReflectVec      = normalize(reflect(-lightVec, tnorm));\n"
        "ViewVec         = normalize(-ecPos);\n"
        "NdotL           = (dot(lightVec, tnorm) + 1.0) * 0.5;\n"
        "gl_Position     = ftransform();\n"
    "}\n";

char fragmentShaderSource[] =
    "// Fragment shader for Gooch shading, adapted for ARB_uniform_buffer_object\n"

    "#extension GL_ARB_uniform_buffer_object : enable\n"

    "layout(std140) uniform colors0\n"
    "{\n"
        "float DiffuseCool;\n"
        "float DiffuseWarm;\n"
        "vec3  SurfaceColor;\n"
        "vec3  WarmColor;\n"
        "vec3  CoolColor;\n"
    "};\n"

    "varying float NdotL;\n"
    "varying vec3  ReflectVec;\n"
    "varying vec3  ViewVec;\n"

    "void main (void)\n"
    "{\n"
        "vec3 kcool    = min(CoolColor + DiffuseCool * SurfaceColor, 1.0);\n"
        "vec3 kwarm    = min(WarmColor + DiffuseWarm * SurfaceColor, 1.0); \n"
        "vec3 kfinal   = mix(kcool, kwarm, NdotL);\n"

        "vec3 nreflect = normalize(ReflectVec);\n"
        "vec3 nview    = normalize(ViewVec);\n"

        "float spec    = max(dot(nreflect, nview), 0.0);\n"
        "spec          = pow(spec, 32.0);\n"

        "gl_FragColor = vec4 (min(kfinal + spec, 1.0), 1.0);\n"
    "}\n";

// Callback for animating the WarmColor

class UniformCallback : public StateAttributeCallback
{
public:
    void operator() (StateAttribute* attr, NodeVisitor* nv)
    {
        UniformBufferBinding* ubb = static_cast<UniformBufferBinding*>(attr);
        UniformBufferObject* ubo
            = static_cast<UniformBufferObject*>(ubb->getBufferObject());
        FloatArray* array = static_cast<FloatArray*>(ubo->getBufferData(0));
        double time = nv->getFrameStamp()->getSimulationTime();
        double frac = fmod(time, 1.0);
        Vec4f warmColor = (Vec4f(0.0, 0.0, 1.0 ,1) * frac
                           +  Vec4f(1.0, 0.0, 0.0, 1) * (1 - frac));
        // Since we're using the std140 layout, we know where the
        // warmColor variable is located in the buffer.
        for (int i = 0; i < 4; ++i)
            (*array)[12 + i] = warmColor[i];
        array->dirty();
    }
};
    
int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc,argv);
    osgViewer::Viewer viewer(arguments);
    
    if (arguments.argc() <= 1) {
        cerr << "Need a scene.\n";
        return 1;
    }

    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    if (!loadedModel) {
        cerr << "couldn't load " << argv[1] << "\n";
        return 1;
    }
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel.get());
    const BoundingSphere bound = loadedModel->getBound();
    const float displacement = 2.25 * bound.radius();
    Group* scene = new Group;
    StateSet* rootSS = scene->getOrCreateStateSet();

    Shader* vertexShader = new Shader(Shader::VERTEX);
    vertexShader->setShaderSource(vertexShaderSource);
    Shader* fragmentShader = new Shader(Shader::FRAGMENT);
    fragmentShader->setShaderSource(fragmentShaderSource);
    Program* prog = new Program;
    prog->addShader(vertexShader);
    prog->addShader(fragmentShader);
    prog->addBindUniformBlock("colors0", 0);
    rootSS->setAttributeAndModes(prog, StateAttribute::ON);
    // Place 3 instances of the loaded model with different uniform
    // blocks for each.
    //
    // The blocksize is known because of the std140 format.
    const unsigned blockSize = 20 * sizeof(GLfloat);
    ref_ptr<FloatArray> colorArray
        = new FloatArray(&colors1[0],
                         &colors1[sizeof(colors1) / sizeof(GLfloat)]);
    ref_ptr<UniformBufferObject> ubo = new UniformBufferObject;
    colorArray->setBufferObject(ubo.get());
    Group* group1 = new Group;
    StateSet* ss1 = group1->getOrCreateStateSet();
    group1->addChild(loadedModel.get());
    scene->addChild(group1);
    ref_ptr<UniformBufferBinding> ubb1
        = new UniformBufferBinding(0, ubo.get(), 0, blockSize);
    ss1->setAttributeAndModes(ubb1.get(), StateAttribute::ON);
    
    ref_ptr<FloatArray> colorArray2
        = new FloatArray(&colors2[0],
                         &colors2[sizeof(colors2) / sizeof(GLfloat)]);
    ref_ptr<UniformBufferObject> ubo2 = new UniformBufferObject;
    colorArray2->setBufferObject(ubo2.get());
    MatrixTransform* group2 = new MatrixTransform;
    Matrix mat2 = Matrix::translate(-displacement, 0.0, 0.0);
    group2->setMatrix(mat2);
    StateSet* ss2 = group2->getOrCreateStateSet();
    group2->addChild(loadedModel.get());
    scene->addChild(group2);
    ref_ptr<UniformBufferBinding> ubb2
        = new UniformBufferBinding(0, ubo2.get(), 0, blockSize);
    ss2->setAttributeAndModes(ubb2.get(), StateAttribute::ON);

    ref_ptr<FloatArray> colorArray3
        = new FloatArray(&colors2[0],
                         &colors2[sizeof(colors2) / sizeof(GLfloat)]);
    ref_ptr<UniformBufferObject> ubo3 = new UniformBufferObject;
    colorArray3->setBufferObject(ubo3.get());
    MatrixTransform* group3 = new MatrixTransform;
    Matrix mat3 = Matrix::translate(displacement, 0.0, 0.0);
    group3->setMatrix(mat3);
    StateSet* ss3 = group3->getOrCreateStateSet();
    group3->addChild(loadedModel.get());
    scene->addChild(group3);    
    ref_ptr<UniformBufferBinding> ubb3
        = new UniformBufferBinding(0, ubo3.get(), 0, blockSize);
    ubb3->setUpdateCallback(new UniformCallback);
    ubb3->setDataVariance(Object::DYNAMIC);
    ss3->setAttributeAndModes(ubb3.get(), StateAttribute::ON);
    
    viewer.setSceneData(scene);
    viewer.realize();
    return viewer.run();
}
